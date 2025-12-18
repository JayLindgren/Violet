// Copyright 2006 by Jay Lindgren. All Rights Reserved.
//
#define _CRT_SECURE_NO_WARNINGS

//#include "MoveOrder.h"
#include "Functions.h"
#include "Definitions.h"
#include "Structures.h"
#include "SEE.h"

#include <algorithm> // For std::sort
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

// This array is used by SortMovesHash for MVV-LVA scoring.
// It's better placed here as it's directly related to move ordering scores.
static const int pieceValues[13] = {
    0,   // 0: dEmpty
    dValuePawnOpening,   // 1: dWhitePawn
    dValueKnightOpening, // 2: dWhiteKnight
    dValueBishopOpening, // 3: dWhiteBishop
    dValueRookOpening,   // 4: dWhiteRook
    dValueQueenOpening,  // 5: dWhiteQueen
    dValueKingOpening,   // 6: dWhiteKing
    dValuePawnOpening,   // 7: dBlackPawn
    dValueKnightOpening, // 8: dBlackKnight
    dValueBishopOpening, // 9: dBlackBishop
    dValueRookOpening,   // 10: dBlackRook
    dValueQueenOpening,  // 11: dBlackQueen
    dValueKingOpening    // 12: dBlackKing
};

// Global score array removed (was unused)
// int g_viScore[dNumberOfMoves];

// Global array to store pre-calculated placement score differences.
// Dimensions: [PieceType][FromSquare][ToSquare]
int g_PlacementScore[13][64][64];

// Sort the moves based on a variety of heuristics, including MVV-LVA for captures.
// This is the BASELINE version (A)
void SortMovesHashA( int * argvsiMoveOrder,
                    Move * argvsMoveList,
                    int argiNumberOfMoves,
                    Board * argsBoard,
                    GeneralMove * argsGeneralMoves,
                    int * viScore,
                    const HashQueryResult& hashResult,
                    SearchParameters * argsSearchParameters )
{
   assert(argsBoard >= 0);
   assert(argsGeneralMoves > 0);
   assert(argiNumberOfMoves >= 0);
   assert(argiNumberOfMoves <= dNumberOfMoves);
   assert(argvsMoveList >= 0);

   const int CAPTURE_BONUS = 1000000;

   for (int iMoveCount = 0; iMoveCount < argiNumberOfMoves; iMoveCount++)
   {
      argvsiMoveOrder[ iMoveCount ] = iMoveCount;
      Move& currentMove = argvsMoveList[ iMoveCount ];
      int score = 0;

      if (currentMove.iMoveType == dCapture || currentMove.iMoveType == dCaptureAndPromote || currentMove.iMoveType == dEnPassant) {
         int aggressorType = argsBoard->mBoard[currentMove.iFromSquare];
         int victimType = (currentMove.iMoveType == dEnPassant) ?
            ((argsBoard->siColorToMove == dWhite) ? dBlackPawn : dWhitePawn) :
            argsBoard->mBoard[currentMove.iToSquare];

         if (aggressorType >= 1 && aggressorType <= 12 && victimType >= 1 && victimType <= 12) {
            score = ( pieceValues[ victimType ] * 100 ) - pieceValues[ aggressorType ];
         }
         score += CAPTURE_BONUS;
      }
      else {
         score = currentMove.iScore;
      }

      if (currentMove.iMoveType == dPromote || currentMove.iMoveType == dCaptureAndPromote) {
         score += pieceValues[ currentMove.iPiece ] * 10;
      }
      viScore[ iMoveCount ] = score;
      viScore[ iMoveCount ] += UpdateScoreHH( &argvsMoveList[ iMoveCount ], argsGeneralMoves, argsSearchParameters );
      viScore[ iMoveCount ] += UpdateScoreKillerMoves( argsBoard, &argvsMoveList[ iMoveCount ], argsGeneralMoves, argsSearchParameters );
      viScore[ iMoveCount ] += g_PlacementScore[ argvsMoveList[ iMoveCount ].iPiece ][ argvsMoveList[ iMoveCount ].iFromSquare ][ argvsMoveList[ iMoveCount ].iToSquare ];

      if (argsBoard->iNumberOfPlys + 1 < argsBoard->iMaxPlys - 1)
      {
         if ( ( argsBoard->vmPrincipalVariation[ 0 ][ argsBoard->iNumberOfPlys + 1 ].iFromSquare == argvsMoveList[ iMoveCount ].iFromSquare ) &&
            ( argsBoard->vmPrincipalVariation[ 0 ][ argsBoard->iNumberOfPlys + 1 ].iToSquare == argvsMoveList[ iMoveCount ].iToSquare ) )
         {
            viScore[ iMoveCount ] += argsGeneralMoves->msPVMove;
         }
      }
   }

# if defined( dUseHash )
   if (GetUseHashTable() && hashResult.iQueryState && (hashResult.iBestMove < 128))
   {
      viScore[hashResult.iBestMove] += argsGeneralMoves->msBestMove;
   }
# endif

   // Use std::sort with a lambda for efficient O(N log N) sorting
   std::sort(argvsiMoveOrder, argvsiMoveOrder + argiNumberOfMoves,
      [&viScore](int index_a, int index_b) {
         return viScore[ index_a ] > viScore[ index_b ];
      });
}

// Sort the moves based on a variety of heuristics, including MVV-LVA for captures.
// This is the EXPERIMENTAL version (B) - Uses SEE to filter bad captures
void SortMovesHashB( int * argvsiMoveOrder,
                    Move * argvsMoveList,
                    int argiNumberOfMoves,
                    Board * argsBoard,
                    GeneralMove * argsGeneralMoves,
                    int * viScore,
                    const HashQueryResult& hashResult,
                    SearchParameters * argsSearchParameters )
{
   assert(argsBoard >= 0);
   assert(argsGeneralMoves > 0);
   assert(argiNumberOfMoves >= 0);
   assert(argiNumberOfMoves <= dNumberOfMoves);
   assert(argvsMoveList >= 0);

   const int CAPTURE_BONUS = 1000000;

   for (int iMoveCount = 0; iMoveCount < argiNumberOfMoves; iMoveCount++)
   {
      argvsiMoveOrder[ iMoveCount ] = iMoveCount;
      Move& currentMove = argvsMoveList[ iMoveCount ];
      int score = 0;

      if (currentMove.iMoveType == dCapture || currentMove.iMoveType == dCaptureAndPromote || currentMove.iMoveType == dEnPassant) {
         int aggressorType = argsBoard->mBoard[currentMove.iFromSquare];
         int victimType = (currentMove.iMoveType == dEnPassant) ?
            ((argsBoard->siColorToMove == dWhite) ? dBlackPawn : dWhitePawn) :
            argsBoard->mBoard[currentMove.iToSquare];

         if (aggressorType >= 1 && aggressorType <= 12 && victimType >= 1 && victimType <= 12) {
            score = ( pieceValues[ victimType ] * 100 ) - pieceValues[ aggressorType ];
         }
         
         // SEE Check: Only give CAPTURE_BONUS if SEE >= 0
         // Optimization: If victim value > attacker value (e.g. PxQ), it's almost always good.
         // We can skip SEE in these cases to save time.
         bool bGoodCapture = false;
         if ( pieceValues[ victimType ] > pieceValues[ aggressorType ] ) {
             bGoodCapture = true;
         } else {
             int seeScore = See(argsBoard, argsGeneralMoves, &currentMove);
             if (seeScore >= 0) {
                 bGoodCapture = true;
             }
         }

         if (bGoodCapture) {
             score += CAPTURE_BONUS;
         } else {
             // Bad capture: Treat as quiet move or worse. 
             // Current score is small positive/negative from MVV-LVA.
             // We leave it as is, so it will be sorted by MVV-LVA but without the huge bonus.
             // This places it below killers (1500) and history.
         }
      }
      else {
         score = currentMove.iScore;
      }

      if (currentMove.iMoveType == dPromote || currentMove.iMoveType == dCaptureAndPromote) {
         score += pieceValues[ currentMove.iPiece ] * 10;
      }
      viScore[ iMoveCount ] = score;
      viScore[ iMoveCount ] += UpdateScoreHH( &argvsMoveList[ iMoveCount ], argsGeneralMoves, argsSearchParameters );
      viScore[ iMoveCount ] += UpdateScoreKillerMoves( argsBoard, &argvsMoveList[ iMoveCount ], argsGeneralMoves, argsSearchParameters );
      viScore[ iMoveCount ] += g_PlacementScore[ argvsMoveList[ iMoveCount ].iPiece ][ argvsMoveList[ iMoveCount ].iFromSquare ][ argvsMoveList[ iMoveCount ].iToSquare ];

      if (argsBoard->iNumberOfPlys + 1 < argsBoard->iMaxPlys - 1)
      {
         if ( ( argsBoard->vmPrincipalVariation[ 0 ][ argsBoard->iNumberOfPlys + 1 ].iFromSquare == argvsMoveList[ iMoveCount ].iFromSquare ) &&
            ( argsBoard->vmPrincipalVariation[ 0 ][ argsBoard->iNumberOfPlys + 1 ].iToSquare == argvsMoveList[ iMoveCount ].iToSquare ) )
         {
            viScore[ iMoveCount ] += argsGeneralMoves->msPVMove;
         }
      }
   }

# if defined( dUseHash )
   if (GetUseHashTable() && hashResult.iQueryState && (hashResult.iBestMove < 128))
   {
      // Prioritize Hash Move above all else (including captures)
      // CAPTURE_BONUS is 1,000,000. We use 2,000,000 to ensure it's first.
      viScore[ hashResult.iBestMove ] += 2000000;
   }
# endif

   // Use std::sort with a lambda for efficient O(N log N) sorting
   std::sort(argvsiMoveOrder, argvsiMoveOrder + argiNumberOfMoves,
      [&viScore](int index_a, int index_b) {
         return viScore[ index_a ] > viScore[ index_b ];
      });
}

// Wrapper function to dispatch to the correct move ordering function
void SortMovesHash( int * argvsiMoveOrder,
                    Move * argvsMoveList,
                    int argiNumberOfMoves,
                    Board * argsBoard,
                    GeneralMove * argsGeneralMoves,
                    int * viScore,
                    const HashQueryResult& hashResult,
                    SearchParameters * argsSearchParameters )
{
    if (gsTempus.giMoveOrderingMode == dMoveOrderingB) {
        SortMovesHashB(argvsiMoveOrder, argvsMoveList, argiNumberOfMoves, argsBoard, argsGeneralMoves, viScore, hashResult, argsSearchParameters);
    } else {
        SortMovesHashA(argvsiMoveOrder, argvsMoveList, argiNumberOfMoves, argsBoard, argsGeneralMoves, viScore, hashResult, argsSearchParameters);
    }
}

//
//------------------------------------------------------------------------------------------------------------
//
void SortMoves( int * argvsiMoveOrder, 
                Move * argvsMoveList, 
                int argNumberOfMoves )
{
   assert(argvsiMoveOrder >= 0);
   assert(argvsMoveList > 0);
   assert(argNumberOfMoves >= 0);
   assert(argNumberOfMoves <= dNumberOfMoves);

   int viScore[dNumberOfMoves]; // Local array

   for (int iMoveIndex = 0; iMoveIndex < argNumberOfMoves; iMoveIndex++)
   {
      viScore[ iMoveIndex ] = argvsMoveList[ iMoveIndex ].iScore;
      argvsiMoveOrder[ iMoveIndex ] = iMoveIndex;
   }

   // Use insertion sort for small N, as it's often faster than std::sort
   // due to less overhead.
   for (int i = 1; i < argNumberOfMoves; i++)
   {
      int currentMoveIndex = argvsiMoveOrder[ i ];
      int currentScore = viScore[ currentMoveIndex ];
      int j = i - 1;

      while ( j >= 0 && viScore[ argvsiMoveOrder[ j ] ] < currentScore )
      {
         argvsiMoveOrder[ j + 1 ] = argvsiMoveOrder[ j ];
         j--;
      }
      argvsiMoveOrder[ j + 1 ] = currentMoveIndex;
   }
}

//
//------------------------------------------------------------------------------------------------------------
// Q-search sorting that only keeps captures
int QSortMoves( int * argvsiMoveOrder, 
                Move * argvsMoveList, 
                int argNumberOfMoves, 
                Board * argsBoard )
{
   assert(argvsiMoveOrder >= 0);
   assert(argvsMoveList >= 0);
   assert(argNumberOfMoves <= dNumberOfMoves);
   assert(argsBoard != nullptr);

   int viScore[dNumberOfMoves]; // Local array
   int iCaptureCount = 0;

   for (int iMoveIndex = 0; iMoveIndex < argNumberOfMoves; iMoveIndex++)
   {
      Move& currentMove = argvsMoveList[ iMoveIndex ];
      if (currentMove.iMoveType == dCapture || currentMove.iMoveType == dCaptureAndPromote || currentMove.iMoveType == dEnPassant)
      {
         int score = 0;
         int aggressorType = argsBoard->mBoard[currentMove.iFromSquare];
         int victimType = (currentMove.iMoveType == dEnPassant) ?
            ((argsBoard->siColorToMove == dWhite) ? dBlackPawn : dWhitePawn) :
            argsBoard->mBoard[currentMove.iToSquare];

         if (aggressorType >= 1 && aggressorType <= 12 && victimType >= 1 && victimType <= 12) {
            score = ( pieceValues[ victimType ] * 100 ) - pieceValues[ aggressorType ];
         }

         if (currentMove.iMoveType == dCaptureAndPromote) {
            score += pieceValues[ currentMove.iPiece ] * 10;
         }

         viScore[ iMoveIndex ] = score;
         argvsiMoveOrder[ iCaptureCount ] = iMoveIndex;
         iCaptureCount++;
      }
   }

   if (iCaptureCount > 1)
   {
      // Use insertion sort for a small number of captures. It's often faster than std::sort
      // for small N due to less overhead.
      for (int i = 1; i < iCaptureCount; i++)
      {
         int currentMoveIndex = argvsiMoveOrder[ i ];
         int currentScore = viScore[ currentMoveIndex ];
         int j = i - 1;

         while ( j >= 0 && viScore[ argvsiMoveOrder[ j ] ] < currentScore )
         {
            argvsiMoveOrder[ j + 1 ] = argvsiMoveOrder[ j ];
            j--;
         }
         argvsiMoveOrder[ j + 1 ] = currentMoveIndex;
      }
   }

   return iCaptureCount;
}

//
//------------------------------------------------------------------------------------------------------------
//  Reset the history heuristic tables
void ResetHistoryHeuristic( SearchParameters * argsSearchParameters )
{
    gSharedHistory.Reset();
}

//
//------------------------------------------------------------------------------------------------------------
// Update the history heuristic tables
void UpdateHH( Board * argsBoard, 
               Move * argsMove,
               SearchParameters * argsSearchParameters )
{
   assert(argsBoard >= 0);
   assert(argsMove > 0);

   if (gsTempus.giMoveOrderingMode == dMoveOrderingB) {
       // Mode B: Linear Depth Bonus (depth * depth was too aggressive)
       int depth = argsBoard->iMaxPlys - argsBoard->iNumberOfPlys;
       if (depth < 0) depth = 0;
       int bonus = depth * gsTempus.iHistoryBonusMultiplier; 
       
       // Cap bonus to prevent excessive values if needed, but int is large enough.
       // We might want to cap the table entries eventually.
       
       if (argsMove->iPiece < dBlackPawn) {
          gSharedHistory.mWhiteHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ] += bonus;
       }
       else {
          gSharedHistory.mBlackHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ] += bonus;
       }
   } else {
       // Mode A: Original Counter
       if (argsMove->iPiece < dBlackPawn) {
          gSharedHistory.iWhiteHHCount++;
          gSharedHistory.mWhiteHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ]++;
       }
       else {
          gSharedHistory.iBlackHHCount++;
          gSharedHistory.mBlackHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ]++;
       }
   }
}

//
//------------------------------------------------------------------------------------------------------------
// Update the score based on the history heuristic
int UpdateScoreHH( Move * argsMove, 
                   GeneralMove * argsGeneralMoves,
                   SearchParameters * argsSearchParameters )
{
   int iScore = 0;
   
   if (gsTempus.giMoveOrderingMode == dMoveOrderingB) {
       // Mode B: Raw History Score
       if (argsMove->iPiece < dBlackPawn) {
          iScore = gSharedHistory.mWhiteHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ];
       } else {
          iScore = gSharedHistory.mBlackHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ];
       }
       
       // Cap at msBestMove - 3 to avoid interfering with Hash/Killer if they are in similar range
       // But Hash/Killer are usually handled separately or have higher bonuses.
       // Current logic caps at msBestMove - 2.
       // Update: Cap at msKillerMove - 100 to ensure Killer Moves are searched first.
       if (iScore > gsTempus.iHistoryScoreCap) {
          iScore = gsTempus.iHistoryScoreCap;
       }
       return iScore;
   }

   // Mode A: Original Logic
   if (argsMove->iPiece < dBlackPawn && gSharedHistory.iWhiteHHCount > 0) {
      iScore = int( double( argsGeneralMoves->msKillerMove ) * ( double( gSharedHistory.mWhiteHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ] ) / double( gSharedHistory.iWhiteHHCount ) ) );
   }
   else if (gSharedHistory.iBlackHHCount > 0) {
      iScore = int( double( argsGeneralMoves->msKillerMove ) * ( double( gSharedHistory.mBlackHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ] ) / double( gSharedHistory.iBlackHHCount ) ) );
   }

   assert(iScore >= 0);
   assert(iScore <= argsGeneralMoves->msKillerMove);

   if (iScore > argsGeneralMoves->msBestMove) {
      iScore = argsGeneralMoves->msBestMove - 2;
   }
   return iScore;
}

//
//------------------------------------------------------------------------------------------------------------
// Reset the killer move tables
void ResetKillerMoves( SearchParameters * argsSearchParameters )
{
   for (int iPlyCount = 0; iPlyCount < dNumberOfMoves; iPlyCount++)
   {
      argsSearchParameters->vKillerMoveCount[ iPlyCount ] = 1;
      for (int i = 0; i < 64; i++) {
         for (int j = 0; j < 64; j++) {
            argsSearchParameters->mKillerMove[ iPlyCount ][ i ][ j ] = 0;
         }
      }
   }
   
   // Reset Mode B Killer Moves
   for (int i = 0; i < dNumberOfPlys; ++i) {
       argsSearchParameters->mKillerMoves[ i ][ 0 ].iScore = 0;
       argsSearchParameters->mKillerMoves[ i ][ 0 ].iFromSquare = 0;
       argsSearchParameters->mKillerMoves[ i ][ 0 ].iToSquare = 0;
       argsSearchParameters->mKillerMoves[ i ][ 1 ].iScore = 0;
       argsSearchParameters->mKillerMoves[ i ][ 1 ].iFromSquare = 0;
       argsSearchParameters->mKillerMoves[ i ][ 1 ].iToSquare = 0;
   }
}

//
//------------------------------------------------------------------------------------------------------------
// Update the killer move tables
void UpdateKillerMoves( Board * argsBoard, 
                        Move * argsMove,
                        SearchParameters * argsSearchParameters )
{
   assert(argsBoard >= 0);
   assert(argsMove > 0);
   
   if (gsTempus.giMoveOrderingMode == dMoveOrderingB) {
       // Mode B: 2-slot Killer Moves
       // Only store quiet moves (non-captures)
       if (argsMove->iCapture != dEmpty) {
           return;
       }
       
       int ply = argsBoard->iNumberOfPlys;
       if (ply >= 0 && ply < dNumberOfPlys) {
           // Check if move is already Killer 1
           if (argsSearchParameters->mKillerMoves[ply][0].iFromSquare == argsMove->iFromSquare &&
               argsSearchParameters->mKillerMoves[ply][0].iToSquare == argsMove->iToSquare) {
               return; // Already top killer
           }
           
           // Shift Killer 1 to Killer 2
           argsSearchParameters->mKillerMoves[ply][1] = argsSearchParameters->mKillerMoves[ply][0];
           
           // Store new move as Killer 1
           argsSearchParameters->mKillerMoves[ply][0] = *argsMove;
       }
   } else {
       // Mode A: Original Frequency Table
       argsSearchParameters->mKillerMove[argsBoard->iNumberOfPlys][argsMove->iFromSquare][argsMove->iToSquare]++;
       argsSearchParameters->vKillerMoveCount[argsBoard->iNumberOfPlys]++;
   }
}

//
//------------------------------------------------------------------------------------------------------------
// Update the score based on the killer move heuristic
int UpdateScoreKillerMoves( Board * argsBoard, 
                            Move * argsMove, 
                            GeneralMove * argsGeneralMoves,
                            SearchParameters * argsSearchParameters )
{
   assert(argsBoard >= 0);
   assert(argsMove > 0);

   if (gsTempus.giMoveOrderingMode == dMoveOrderingB) {
       // Mode B: 2-slot Killer Moves Scoring
       int ply = argsBoard->iNumberOfPlys;
       if (ply >= 0 && ply < dNumberOfPlys) {
           if (argsSearchParameters->mKillerMoves[ply][0].iFromSquare == argsMove->iFromSquare &&
               argsSearchParameters->mKillerMoves[ply][0].iToSquare == argsMove->iToSquare) {
               return gsTempus.iKillerScore1; // Killer 1 Score
           }
           if (argsSearchParameters->mKillerMoves[ply][1].iFromSquare == argsMove->iFromSquare &&
               argsSearchParameters->mKillerMoves[ply][1].iToSquare == argsMove->iToSquare) {
               return gsTempus.iKillerScore2; // Killer 2 Score
           }
       }
       return 0;
   }

   // Mode A: Original Frequency Table
   if (argsSearchParameters->vKillerMoveCount[argsBoard->iNumberOfPlys] == 0) {
      return 0;
   }

   int iScore = int(double(argsGeneralMoves->msKillerMove) * (double(argsSearchParameters->mKillerMove[argsBoard->iNumberOfPlys][argsMove->iFromSquare][argsMove->iToSquare]) / double(argsSearchParameters->vKillerMoveCount[argsBoard->iNumberOfPlys])));

   assert(iScore >= 0);
   assert(iScore <= argsGeneralMoves->msKillerMove); // Fixed sdKillerMove typo to msKillerMove

   if (iScore > argsGeneralMoves->msBestMove) {
      iScore = argsGeneralMoves->msBestMove - 1;
   }
   return iScore;
}

//
//--------------------------------------------------------------------------------------------
// Initializes the g_PlacementScore table with score differences
// for every possible piece move on the board.
void InitializePlacementScores()
{
   for (int from = 0; from < 64; ++from)
   {
      for (int to = 0; to < 64; ++to)
      {
         g_PlacementScore[dWhitePawn][from][to]   = GetWhitePawnPlacementScore(to)   - GetWhitePawnPlacementScore(from);
         g_PlacementScore[dBlackPawn][from][to]   = GetBlackPawnPlacementScore(to)   - GetBlackPawnPlacementScore(from);
         g_PlacementScore[dWhiteKnight][from][to] = GetWhiteKnightPlacementScore(to) - GetWhiteKnightPlacementScore(from);
         g_PlacementScore[dBlackKnight][from][to] = GetBlackKnightPlacementScore(to) - GetBlackKnightPlacementScore(from);
         g_PlacementScore[dWhiteBishop][from][to] = GetWhiteBishopPlacementScore(to) - GetWhiteBishopPlacementScore(from);
         g_PlacementScore[dBlackBishop][from][to] = GetBlackBishopPlacementScore(to) - GetBlackBishopPlacementScore(from);
         g_PlacementScore[dWhiteRook][from][to]   = GetWhiteRookPlacementScore(to)   - GetWhiteRookPlacementScore(from);
         g_PlacementScore[dBlackRook][from][to]   = GetBlackRookPlacementScore(to)   - GetBlackRookPlacementScore(from);
         g_PlacementScore[dWhiteQueen][from][to]  = GetWhiteQueenPlacementScore(to)  - GetWhiteQueenPlacementScore(from);
         g_PlacementScore[dBlackQueen][from][to]  = GetBlackQueenPlacementScore(to)  - GetBlackQueenPlacementScore(from);
         g_PlacementScore[dWhiteKing][from][to]   = GetWhiteKingPlacementScore(to)   - GetWhiteKingPlacementScore(from);
         g_PlacementScore[dBlackKing][from][to]   = GetBlackKingPlacementScore(to)   - GetBlackKingPlacementScore(from);
      }
   }
}

//
//--------------------------------------------------------------------------------------------
// Test Move Ordering Performance
//
#include <fstream>
#include <sstream>

// Helper to load baseline results
bool LoadBaseline( const std::string & filename, 
                   const std::vector<std::string> & fens, 
                   std::vector<long long> & nodes, 
                   std::vector<double> & times ) {
    std::ifstream infile(filename);
    if (!infile.good()) return false;

    std::string line;
    int index = 0;
    while (std::getline(infile, line) && index < fens.size()) {
        std::stringstream ss(line);
        std::string fen;
        long long nodeCount;
        double timeVal;
        
        // Read FEN (might contain spaces, so read until last two tokens)
        // Format: FEN NODES TIME
        // Actually, simpler format: NODES TIME (assuming FEN order is constant)
        // Let's stick to strict order.
        
        if (!(ss >> nodeCount >> timeVal)) return false;
        
        nodes.push_back(nodeCount);
        times.push_back(timeVal);
        index++;
    }
    return index == fens.size();
}

// Helper to save baseline results
void SaveBaseline( const std::string & filename, 
                   const std::vector<long long> & nodes, 
                   const std::vector<double> & times ) {
    std::ofstream outfile(filename);
    for (size_t i = 0; i < nodes.size(); ++i) {
        outfile << nodes[i] << " " << times[i] << std::endl;
    }
}

long long TestMoveOrdering( Board * argsBoard, 
                            GeneralMove * argsGeneralMoves, 
                            bool quiet, 
                            bool forceBaseline ) {
    std::cout << "Starting Move Ordering Performance Test..." << std::endl;
    // Define a set of test positions (FEN strings)
    // Using a small subset for faster tuning if needed, or the full set.
    // For now, use the same set.
    std::vector<std::string> testPositions = {
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
    };

    int depth = 6; // Fixed depth for testing
    long long totalNodesA = 0;
    long long totalNodesB = 0;
    double totalTimeA = 0;
    double totalTimeB = 0;

    // Baseline Results Containers
    std::vector<long long> baselineNodes;
    std::vector<double> baselineTimes;
    bool useCachedBaseline = false;
    const std::string baselineFile = "baseline_results.txt";

    if (!quiet) {
        if (!forceBaseline && LoadBaseline(baselineFile, testPositions, baselineNodes, baselineTimes)) {
            useCachedBaseline = true;
            std::cout << "Using cached baseline results from " << baselineFile << std::endl;
        } else {
            std::cout << "Running full baseline search..." << std::endl;
        }

        std::cout << std::left << std::setw(50) << "Position" 
                  << std::setw(15) << "Nodes A" 
                  << std::setw(15) << "Time A (s)" 
                  << std::setw(15) << "Nodes B" 
                  << std::setw(15) << "Time B (s)" 
                  << std::setw(15) << "Diff (B/A)" << std::endl;
        std::cout << std::string(125, '-') << std::endl;
    }

    for (size_t i = 0; i < testPositions.size(); ++i) {
        const auto& fen = testPositions[i];
        
        long long nodesA_pos = 0;
        double timeA_pos = 0;

        if (!quiet) {
            if (useCachedBaseline) {
                nodesA_pos = baselineNodes[i];
                timeA_pos = baselineTimes[i];
            } else {
                // Test Mode A (Baseline)
                gsTempus.giMoveOrderingMode = dMoveOrderingA;
                ReadFEN(fen.c_str(), argsBoard, argsGeneralMoves, 2);
                SetSearchDepth(depth);
                SetSearchTimeInMiliSeconds(dInfiniteTime);
                
                ClearHashTable();
                clock_t startA = clock();
                StartSearch(argsBoard, argsGeneralMoves, dAlpha, dBeta);
                clock_t endA = clock();
                
                nodesA_pos = (long long)GetNumberOfNodesSearched();
                timeA_pos = (double)(endA - startA) / CLOCKS_PER_SEC;
                
                baselineNodes.push_back(nodesA_pos);
                baselineTimes.push_back(timeA_pos);
            }
            totalNodesA += nodesA_pos;
            totalTimeA += timeA_pos;
        }

        // Test Mode B
        gsTempus.giMoveOrderingMode = dMoveOrderingB;
        ReadFEN(fen.c_str(), argsBoard, argsGeneralMoves, 2);
        SetSearchDepth(depth);
        SetSearchTimeInMiliSeconds(dInfiniteTime);
        
        ClearHashTable();
        clock_t startB = clock();
        StartSearch(argsBoard, argsGeneralMoves, dAlpha, dBeta);
        clock_t endB = clock();

        long long nodesB = (long long)GetNumberOfNodesSearched();
        double timeB = (double)(endB - startB) / CLOCKS_PER_SEC;
        totalNodesB += nodesB;
        totalTimeB += timeB;

        if (!quiet) {
            std::cout << std::left << std::setw(50) << fen.substr(0, 45) + (fen.length() > 45 ? "..." : "")
                      << std::setw(15) << nodesA_pos
                      << std::setw(15) << std::fixed << std::setprecision(3) << timeA_pos
                      << std::setw(15) << nodesB 
                      << std::setw(15) << timeB 
                      << std::setw(15) << (double)nodesB / (nodesA_pos > 0 ? nodesA_pos : 1) << std::endl;
        }
    }

    if (!quiet) {
        if (!useCachedBaseline) {
            SaveBaseline(baselineFile, baselineNodes, baselineTimes);
            std::cout << "Baseline results saved to " << baselineFile << std::endl;
        }

        std::cout << std::string(125, '-') << std::endl;
        std::cout << "Total Nodes A: " << totalNodesA << " Time A: " << totalTimeA << "s" << std::endl;
        std::cout << "Total Nodes B: " << totalNodesB << " Time B: " << totalTimeB << "s" << std::endl;
        std::cout << "Overall Ratio (B/A): " << (double)totalNodesB / (totalNodesA > 0 ? totalNodesA : 1) << std::endl;
    }
    
    // Restore default mode
    gsTempus.giMoveOrderingMode = dMoveOrderingA;
    
    return totalNodesB;
}