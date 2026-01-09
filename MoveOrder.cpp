// Copyright 2006 by Jay Lindgren. All Rights Reserved.
//
#define _CRT_SECURE_NO_WARNINGS

// #include "MoveOrder.h"
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
static const int s_viPieceValues[ 13 ] =
    {
        0,                   // 0: dEmpty
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
int g_PlacementScore[ 13 ][ 64 ][ 64 ];

//
//
//---------------------------------------------------------------------
//
//
// Sort the moves based on a variety of heuristics, including MVV-LVA for captures.
// This is the BASELINE version (A)
void SortMovesHashA( int                   *argvsiMoveOrder,
                     Move                  *argvsMoveList,
                     int                    argsiNumberOfMoves,
                     Board                 *argsBoard,
                     GeneralMove           *argsGeneralMoves,
                     int                   *viScore,
                     const HashQueryResult &argsHashResult,
                     SearchParameters      *argsSearchParameters )
{
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves > 0 );
   assert( argsiNumberOfMoves >= 0 );
   assert( argsiNumberOfMoves <= dNumberOfMoves );
   assert( argvsMoveList >= 0 );

   const int siCaptureBonus = 1000000;

   for ( int iMoveCount = 0; iMoveCount < argsiNumberOfMoves; iMoveCount++ )
   {
      argvsiMoveOrder[ iMoveCount ] = iMoveCount;
      Move &sCurrentMove            = argvsMoveList[ iMoveCount ];
      int   iScore                  = 0;

      if ( sCurrentMove.iMoveType == dCapture ||
           sCurrentMove.iMoveType == dCaptureAndPromote ||
           sCurrentMove.iMoveType == dEnPassant )
      {
         int iAggressorType = argsBoard->mBoard[ sCurrentMove.iFromSquare ];
         int iVictimType    = ( sCurrentMove.iMoveType == dEnPassant ) ? ( ( argsBoard->siColorToMove == dWhite ) ? dBlackPawn : dWhitePawn ) : argsBoard->mBoard[ sCurrentMove.iToSquare ];

         if ( iAggressorType >= 1 && iAggressorType <= 12 && iVictimType >= 1 && iVictimType <= 12 )
         {
            iScore = ( s_viPieceValues[ iVictimType ] * 100 ) - s_viPieceValues[ iAggressorType ];
         }

         iScore += siCaptureBonus;
      }
      else
      {
         iScore = sCurrentMove.iScore;
      }

      if ( sCurrentMove.iMoveType == dPromote || sCurrentMove.iMoveType == dCaptureAndPromote )
      {
         iScore += s_viPieceValues[ sCurrentMove.iPiece ] * 10;
      }

      viScore[ iMoveCount ] = iScore;
      viScore[ iMoveCount ] += UpdateScoreHH( &argvsMoveList[ iMoveCount ], argsGeneralMoves, argsSearchParameters );
      viScore[ iMoveCount ] += UpdateScoreKillerMoves( argsBoard, &argvsMoveList[ iMoveCount ], argsGeneralMoves, argsSearchParameters );
      viScore[ iMoveCount ] += g_PlacementScore[ argvsMoveList[ iMoveCount ].iPiece ][ argvsMoveList[ iMoveCount ].iFromSquare ][ argvsMoveList[ iMoveCount ].iToSquare ];

      if ( argsBoard->iNumberOfPlys + 1 < argsBoard->iMaxPlys - 1 )
      {
         if ( ( argsBoard->vmPrincipalVariation[ 0 ][ argsBoard->iNumberOfPlys + 1 ].iFromSquare == argvsMoveList[ iMoveCount ].iFromSquare ) &&
              ( argsBoard->vmPrincipalVariation[ 0 ][ argsBoard->iNumberOfPlys + 1 ].iToSquare == argvsMoveList[ iMoveCount ].iToSquare ) )
         {
            viScore[ iMoveCount ] += argsGeneralMoves->msPVMove;
         }
      }
   }

#if defined( dUseHash )
   if ( GetUseHashTable() && argsHashResult.iQueryState && ( argsHashResult.iBestMove < 128 ) )
   {
      viScore[ argsHashResult.iBestMove ] += argsGeneralMoves->msBestMove;
   }
#endif

   // Use std::sort with a lambda for efficient O(N log N) sorting
   std::sort( argvsiMoveOrder, argvsiMoveOrder + argsiNumberOfMoves,
              [ &viScore ]( int iIndexA, int iIndexB )
              {
                 return viScore[ iIndexA ] > viScore[ iIndexB ];
              } );
}

//
//
//---------------------------------------------------------------------
//
//
// Sort the moves based on a variety of heuristics, including MVV-LVA for captures.
// This is the EXPERIMENTAL version (B) - Uses SEE to filter bad captures
void SortMovesHashB( int                   *argvsiMoveOrder,
                     Move                  *argvsMoveList,
                     int                    argsiNumberOfMoves,
                     Board                 *argsBoard,
                     GeneralMove           *argsGeneralMoves,
                     int                   *viScore,
                     const HashQueryResult &argsHashResult,
                     SearchParameters      *argsSearchParameters )
{
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves > 0 );
   assert( argsiNumberOfMoves >= 0 );
   assert( argsiNumberOfMoves <= dNumberOfMoves );
   assert( argvsMoveList >= 0 );

   for ( int iMoveCount = 0; iMoveCount < argsiNumberOfMoves; iMoveCount++ )
   {
      argvsiMoveOrder[ iMoveCount ] = iMoveCount;
      Move &sCurrentMove            = argvsMoveList[ iMoveCount ];
      int   iScore                  = 0;

      if ( sCurrentMove.iMoveType == dCapture ||
           sCurrentMove.iMoveType == dCaptureAndPromote ||
           sCurrentMove.iMoveType == dEnPassant )
      {
         int iAggressorType = argsBoard->mBoard[ sCurrentMove.iFromSquare ];
         int iVictimType    = ( sCurrentMove.iMoveType == dEnPassant ) ? ( ( argsBoard->siColorToMove == dWhite ) ? dBlackPawn : dWhitePawn ) : argsBoard->mBoard[ sCurrentMove.iToSquare ];

         if ( iAggressorType >= 1 && iAggressorType <= 12 && iVictimType >= 1 && iVictimType <= 12 )
         {
            iScore = ( s_viPieceValues[ iVictimType ] * 100 ) - s_viPieceValues[ iAggressorType ];
         }

         // SEE Check: Only give GOOD_CAPTURE_BONUS if SEE >= 0
         // Optimization: If victim value > attacker value (e.g. PxQ), it's almost always good.
         // We can skip SEE in these cases to save time.
         bool bGoodCapture = false;
         int  iSeeScore    = 0;

         if ( s_viPieceValues[ iVictimType ] > s_viPieceValues[ iAggressorType ] )
         {
            bGoodCapture = true;
         }
         else
         {
            iSeeScore = See( argsBoard, argsGeneralMoves, &sCurrentMove );

            if ( iSeeScore >= 0 )
            {
               bGoodCapture = true;
            }
         }

         if ( bGoodCapture )
         {
            // Good capture: High base score + SEE value weighted + MVV/LVA
            iScore += dsGoodCaptureBase;

            // If we calculated SEE, incorporate it to differentiate between good captures
            // Otherwise, the MVV-LVA score already computed will serve as the differentiator
            if ( iSeeScore > 0 )
            {
               iScore += ( iSeeScore * 100 ); // Scale SEE to ranking level
            }
         }
         else
         {
            // Bad capture: Heavily penalized, searched after all quiet moves
            // Use the already computed SEE score
            iScore = dsBadCaptureBase + iSeeScore; // Negative base + small recovery
         }
      }
      else
      {
         iScore = sCurrentMove.iScore;
      }

      if ( sCurrentMove.iMoveType == dPromote || sCurrentMove.iMoveType == dCaptureAndPromote )
      {
         iScore += s_viPieceValues[ sCurrentMove.iPiece ] * 10;
      }

      viScore[ iMoveCount ] = iScore;
      viScore[ iMoveCount ] += UpdateScoreHH( &argvsMoveList[ iMoveCount ], argsGeneralMoves, argsSearchParameters );
      viScore[ iMoveCount ] += UpdateScoreKillerMoves( argsBoard, &argvsMoveList[ iMoveCount ], argsGeneralMoves, argsSearchParameters );
      viScore[ iMoveCount ] += g_PlacementScore[ argvsMoveList[ iMoveCount ].iPiece ][ argvsMoveList[ iMoveCount ].iFromSquare ][ argvsMoveList[ iMoveCount ].iToSquare ];

      if ( argsBoard->iNumberOfPlys + 1 < argsBoard->iMaxPlys - 1 )
      {
         if ( ( argsBoard->vmPrincipalVariation[ 0 ][ argsBoard->iNumberOfPlys + 1 ].iFromSquare == argvsMoveList[ iMoveCount ].iFromSquare ) &&
              ( argsBoard->vmPrincipalVariation[ 0 ][ argsBoard->iNumberOfPlys + 1 ].iToSquare == argvsMoveList[ iMoveCount ].iToSquare ) )
         {
            viScore[ iMoveCount ] += argsGeneralMoves->msPVMove;
         }
      }
   }

#if defined( dUseHash )
   if ( GetUseHashTable() && argsHashResult.iQueryState && ( argsHashResult.iBestMove < 128 ) )
   {
      // Prioritize Hash Move above all else (including captures)
      viScore[ argsHashResult.iBestMove ] += dsHashMove;
   }
#endif

   // Use std::sort with a lambda for efficient O(N log N) sorting
   std::sort( argvsiMoveOrder, argvsiMoveOrder + argsiNumberOfMoves,
              [ &viScore ]( int iIndexA, int iIndexB )
              {
                 return viScore[ iIndexA ] > viScore[ iIndexB ];
              } );
}

//
//
//---------------------------------------------------------------------
//
//
// Wrapper function to dispatch to the correct move ordering function
void SortMovesHash( int                   *argvsiMoveOrder,
                    Move                  *argvsMoveList,
                    int                    argsiNumberOfMoves,
                    Board                 *argsBoard,
                    GeneralMove           *argsGeneralMoves,
                    int                   *viScore,
                    const HashQueryResult &argsHashResult,
                    SearchParameters      *argsSearchParameters )
{
   if ( gsTempus.giMoveOrderingMode == dMoveOrderingB )
   {
      SortMovesHashB( argvsiMoveOrder, argvsMoveList, argsiNumberOfMoves, argsBoard, argsGeneralMoves, viScore, argsHashResult, argsSearchParameters );
   }
   else
   {
      SortMovesHashA( argvsiMoveOrder, argvsMoveList, argsiNumberOfMoves, argsBoard, argsGeneralMoves, viScore, argsHashResult, argsSearchParameters );
   }
}

//
//
//---------------------------------------------------------------------
//
//
void SortMoves( int  *argvsiMoveOrder,
                Move *argvsMoveList,
                int   argsiNumberOfMoves )
{
   assert( argvsiMoveOrder >= 0 );
   assert( argvsMoveList > 0 );
   assert( argsiNumberOfMoves >= 0 );
   assert( argsiNumberOfMoves <= dNumberOfMoves );

   int viScore[ dNumberOfMoves ]; // Local array

   for ( int iMoveIndex = 0; iMoveIndex < argsiNumberOfMoves; iMoveIndex++ )
   {
      viScore[ iMoveIndex ]         = argvsMoveList[ iMoveIndex ].iScore;
      argvsiMoveOrder[ iMoveIndex ] = iMoveIndex;
   }

   // Use insertion sort for small N, as it's often faster than std::sort
   // due to less overhead.
   for ( int i = 1; i < argsiNumberOfMoves; i++ )
   {
      int iCurrentMoveIndex = argvsiMoveOrder[ i ];
      int iCurrentScore     = viScore[ iCurrentMoveIndex ];
      int j                 = i - 1;

      while ( j >= 0 && viScore[ argvsiMoveOrder[ j ] ] < iCurrentScore )
      {
         argvsiMoveOrder[ j + 1 ] = argvsiMoveOrder[ j ];
         j--;
      }

      argvsiMoveOrder[ j + 1 ] = iCurrentMoveIndex;
   }
}

//
//
//---------------------------------------------------------------------
//
//
// Q-search sorting that only keeps captures
int QSortMoves( int   *argvsiMoveOrder,
                Move  *argvsMoveList,
                int    argsiNumberOfMoves,
                Board *argsBoard )
{
   assert( argvsiMoveOrder >= 0 );
   assert( argvsMoveList >= 0 );
   assert( argsiNumberOfMoves <= dNumberOfMoves );
   assert( argsBoard != nullptr );

   int viScore[ dNumberOfMoves ]; // Local array
   int iCaptureCount = 0;

   for ( int iMoveIndex = 0; iMoveIndex < argsiNumberOfMoves; iMoveIndex++ )
   {
      Move &sCurrentMove = argvsMoveList[ iMoveIndex ];

      if ( sCurrentMove.iMoveType == dCapture ||
           sCurrentMove.iMoveType == dCaptureAndPromote ||
           sCurrentMove.iMoveType == dEnPassant )
      {
         int iScore         = 0;
         int iAggressorType = argsBoard->mBoard[ sCurrentMove.iFromSquare ];
         int iVictimType    = ( sCurrentMove.iMoveType == dEnPassant ) ? ( ( argsBoard->siColorToMove == dWhite ) ? dBlackPawn : dWhitePawn ) : argsBoard->mBoard[ sCurrentMove.iToSquare ];

         if ( iAggressorType >= 1 && iAggressorType <= 12 && iVictimType >= 1 && iVictimType <= 12 )
         {
            iScore = ( s_viPieceValues[ iVictimType ] * 100 ) - s_viPieceValues[ iAggressorType ];
         }

         if ( sCurrentMove.iMoveType == dCaptureAndPromote )
         {
            iScore += s_viPieceValues[ sCurrentMove.iPiece ] * 10;
         }

         viScore[ iMoveIndex ]            = iScore;
         argvsiMoveOrder[ iCaptureCount ] = iMoveIndex;
         iCaptureCount++;
      }
   }

   if ( iCaptureCount > 1 )
   {
      // Use insertion sort for a small number of captures. It's often faster than std::sort
      // for small N due to less overhead.
      for ( int i = 1; i < iCaptureCount; i++ )
      {
         int iCurrentMoveIndex = argvsiMoveOrder[ i ];
         int iCurrentScore     = viScore[ iCurrentMoveIndex ];
         int j                 = i - 1;

         while ( j >= 0 && viScore[ argvsiMoveOrder[ j ] ] < iCurrentScore )
         {
            argvsiMoveOrder[ j + 1 ] = argvsiMoveOrder[ j ];
            j--;
         }

         argvsiMoveOrder[ j + 1 ] = iCurrentMoveIndex;
      }
   }

   return iCaptureCount;
}

//
//
//---------------------------------------------------------------------
//
//
//  Reset the history heuristic tables
void ResetHistoryHeuristic( SearchParameters *argsSearchParameters )
{
   gSharedHistory.Reset();
}

//
//
//---------------------------------------------------------------------
//
//
// Update the history heuristic tables
void UpdateHH( Board            *argsBoard,
               Move             *argsMove,
               SearchParameters *argsSearchParameters )
{
   assert( argsBoard >= 0 );
   assert( argsMove > 0 );

   if ( gsTempus.giMoveOrderingMode == dMoveOrderingB )
   {
      // Mode B: Linear Depth Bonus (depth * depth was too aggressive)
      int iDepth = argsBoard->iMaxPlys - argsBoard->iNumberOfPlys;

      if ( iDepth < 0 )
      {
         iDepth = 0;
      }

      int iBonus = iDepth * gsTempus.iHistoryBonusMultiplier;

      // Cap bonus to prevent excessive values if needed, but int is large enough.
      // We might want to cap the table entries eventually.

      if ( argsMove->iPiece < dBlackPawn )
      {
         gSharedHistory.mWhiteHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ] += iBonus;
      }
      else
      {
         gSharedHistory.mBlackHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ] += iBonus;
      }
   }
   else
   {
      // Mode A: Original Counter
      if ( argsMove->iPiece < dBlackPawn )
      {
         gSharedHistory.iWhiteHHCount++;
         gSharedHistory.mWhiteHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ]++;
      }
      else
      {
         gSharedHistory.iBlackHHCount++;
         gSharedHistory.mBlackHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ]++;
      }
   }
}

//
//
//---------------------------------------------------------------------
//
//
// Update the score based on the history heuristic
int UpdateScoreHH( Move             *argsMove,
                   GeneralMove      *argsGeneralMoves,
                   SearchParameters *argsSearchParameters )
{
   int iScore = 0;

   if ( gsTempus.giMoveOrderingMode == dMoveOrderingB )
   {
      // Mode B: Raw History Score
      if ( argsMove->iPiece < dBlackPawn )
      {
         iScore = gSharedHistory.mWhiteHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ];
      }
      else
      {
         iScore = gSharedHistory.mBlackHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ];
      }

      // Cap at msBestMove - 3 to avoid interfering with Hash/Killer if they are in similar range
      // But Hash/Killer are usually handled separately or have higher bonuses.
      // Current logic caps at msBestMove - 2.
      // Update: Cap at msKillerMove - 100 to ensure Killer Moves are searched first.
      if ( iScore > gsTempus.iHistoryScoreCap )
      {
         iScore = gsTempus.iHistoryScoreCap;
      }

      return iScore;
   }

   // Mode A: Original Logic
   if ( argsMove->iPiece < dBlackPawn && gSharedHistory.iWhiteHHCount > 0 )
   {
      iScore = int( double( argsGeneralMoves->msKillerMove ) * ( double( gSharedHistory.mWhiteHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ] ) / double( gSharedHistory.iWhiteHHCount ) ) );
   }
   else if ( gSharedHistory.iBlackHHCount > 0 )
   {
      iScore = int( double( argsGeneralMoves->msKillerMove ) * ( double( gSharedHistory.mBlackHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ] ) / double( gSharedHistory.iBlackHHCount ) ) );
   }

   assert( iScore >= 0 );
   assert( iScore <= argsGeneralMoves->msKillerMove );

   if ( iScore > argsGeneralMoves->msBestMove )
   {
      iScore = argsGeneralMoves->msBestMove - 2;
   }

   return iScore;
}

//
//
//---------------------------------------------------------------------
//
//
// Reset the killer move tables
void ResetKillerMoves( SearchParameters *argsSearchParameters )
{
   // Reset Killer Moves (2 slots per ply)
   for ( int i = 0; i < dNumberOfPlys; ++i )
   {
      argsSearchParameters->mKillerMoves[ i ][ 0 ].iScore      = 0;
      argsSearchParameters->mKillerMoves[ i ][ 0 ].iFromSquare = 0;
      argsSearchParameters->mKillerMoves[ i ][ 0 ].iToSquare   = 0;
      argsSearchParameters->mKillerMoves[ i ][ 1 ].iScore      = 0;
      argsSearchParameters->mKillerMoves[ i ][ 1 ].iFromSquare = 0;
      argsSearchParameters->mKillerMoves[ i ][ 1 ].iToSquare   = 0;
   }
}

//
//
//---------------------------------------------------------------------
//
//
// Update the killer move tables
void UpdateKillerMoves( Board            *argsBoard,
                        Move             *argsMove,
                        SearchParameters *argsSearchParameters )
{
   assert( argsBoard >= 0 );
   assert( argsMove > 0 );

   // 2-slot Killer Moves
   // Only store quiet moves (non-captures)
   if ( argsMove->iCapture != dEmpty )
   {
      return;
   }

   int iPly = argsBoard->iNumberOfPlys;

   if ( iPly >= 0 && iPly < dNumberOfPlys )
   {
      // Check if move is already Killer 1
      if ( argsSearchParameters->mKillerMoves[ iPly ][ 0 ].iFromSquare == argsMove->iFromSquare &&
           argsSearchParameters->mKillerMoves[ iPly ][ 0 ].iToSquare == argsMove->iToSquare )
      {
         return; // Already top killer
      }

      // Shift Killer 1 to Killer 2
      argsSearchParameters->mKillerMoves[ iPly ][ 1 ] = argsSearchParameters->mKillerMoves[ iPly ][ 0 ];

      // Store new move as Killer 1
      argsSearchParameters->mKillerMoves[ iPly ][ 0 ] = *argsMove;
   }
}

//
//
//---------------------------------------------------------------------
//
//
// Update the score based on the killer move heuristic
int UpdateScoreKillerMoves( Board            *argsBoard,
                            Move             *argsMove,
                            GeneralMove      *argsGeneralMoves,
                            SearchParameters *argsSearchParameters )
{
   assert( argsBoard >= 0 );
   assert( argsMove > 0 );

   // 2-slot Killer Moves Scoring
   int iPly = argsBoard->iNumberOfPlys;

   if ( iPly >= 0 && iPly < dNumberOfPlys )
   {
      if ( argsSearchParameters->mKillerMoves[ iPly ][ 0 ].iFromSquare == argsMove->iFromSquare &&
           argsSearchParameters->mKillerMoves[ iPly ][ 0 ].iToSquare == argsMove->iToSquare )
      {
         return gsTempus.iKillerScore1; // Killer 1 Score
      }

      if ( argsSearchParameters->mKillerMoves[ iPly ][ 1 ].iFromSquare == argsMove->iFromSquare &&
           argsSearchParameters->mKillerMoves[ iPly ][ 1 ].iToSquare == argsMove->iToSquare )
      {
         return gsTempus.iKillerScore2; // Killer 2 Score
      }
   }

   return 0;
}

//
//
//---------------------------------------------------------------------
//
//
// Initializes the g_PlacementScore table with score differences
// for every possible piece move on the board.
void InitializePlacementScores()
{
   for ( int iFrom = 0; iFrom < 64; ++iFrom )
   {
      for ( int iTo = 0; iTo < 64; ++iTo )
      {
         g_PlacementScore[ dWhitePawn ][ iFrom ][ iTo ]   = GetWhitePawnPlacementScore( iTo ) - GetWhitePawnPlacementScore( iFrom );
         g_PlacementScore[ dBlackPawn ][ iFrom ][ iTo ]   = GetBlackPawnPlacementScore( iTo ) - GetBlackPawnPlacementScore( iFrom );
         g_PlacementScore[ dWhiteKnight ][ iFrom ][ iTo ] = GetWhiteKnightPlacementScore( iTo ) - GetWhiteKnightPlacementScore( iFrom );
         g_PlacementScore[ dBlackKnight ][ iFrom ][ iTo ] = GetBlackKnightPlacementScore( iTo ) - GetBlackKnightPlacementScore( iFrom );
         g_PlacementScore[ dWhiteBishop ][ iFrom ][ iTo ] = GetWhiteBishopPlacementScore( iTo ) - GetWhiteBishopPlacementScore( iFrom );
         g_PlacementScore[ dBlackBishop ][ iFrom ][ iTo ] = GetBlackBishopPlacementScore( iTo ) - GetBlackBishopPlacementScore( iFrom );
         g_PlacementScore[ dWhiteRook ][ iFrom ][ iTo ]   = GetWhiteRookPlacementScore( iTo ) - GetWhiteRookPlacementScore( iFrom );
         g_PlacementScore[ dBlackRook ][ iFrom ][ iTo ]   = GetBlackRookPlacementScore( iTo ) - GetBlackRookPlacementScore( iFrom );
         g_PlacementScore[ dWhiteQueen ][ iFrom ][ iTo ]  = GetWhiteQueenPlacementScore( iTo ) - GetWhiteQueenPlacementScore( iFrom );
         g_PlacementScore[ dBlackQueen ][ iFrom ][ iTo ]  = GetBlackQueenPlacementScore( iTo ) - GetBlackQueenPlacementScore( iFrom );
         g_PlacementScore[ dWhiteKing ][ iFrom ][ iTo ]   = GetWhiteKingPlacementScore( iTo ) - GetWhiteKingPlacementScore( iFrom );
         g_PlacementScore[ dBlackKing ][ iFrom ][ iTo ]   = GetBlackKingPlacementScore( iTo ) - GetBlackKingPlacementScore( iFrom );
      }
   }
}

//
//
//---------------------------------------------------------------------
//
//
// Test Move Ordering Performance
//
#include <fstream>
#include <sstream>

// Helper to load baseline results
bool LoadBaseline( const std::string              &strFilename,
                   const std::vector<std::string> &vFens,
                   std::vector<long long>         &vNodes,
                   std::vector<double>            &vTimes )
{
   std::ifstream ifsInfile( strFilename );

   if ( !ifsInfile.good() )
   {
      return false;
   }

   std::string strLine;
   int         iIndex = 0;

   while ( std::getline( ifsInfile, strLine ) && iIndex < vFens.size() )
   {
      std::stringstream ss( strLine );
      std::string       strFen;
      long long         siNodeCount;
      double            dTimeVal;

      if ( !( ss >> siNodeCount >> dTimeVal ) )
      {
         return false;
      }

      vNodes.push_back( siNodeCount );
      vTimes.push_back( dTimeVal );
      iIndex++;
   }

   return iIndex == vFens.size();
}

// Helper to save baseline results
void SaveBaseline( const std::string            &strFilename,
                   const std::vector<long long> &vNodes,
                   const std::vector<double>    &vTimes )
{
   std::ofstream ofsOutfile( strFilename );

   for ( size_t i = 0; i < vNodes.size(); ++i )
   {
      ofsOutfile << vNodes[ i ] << " " << vTimes[ i ] << std::endl;
   }
}

long long TestMoveOrdering( Board       *argsBoard,
                            GeneralMove *argsGeneralMoves,
                            bool         bQuiet,
                            bool         bForceBaseline )
{
   std::cout << "Starting Move Ordering Performance Test..." << std::endl;

   std::vector<std::string> vTestPositions =
       {
           "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
           "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
           "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
           "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
           "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10" };

   int       iDepth        = 6; // Fixed depth for testing
   long long siTotalNodesA = 0;
   long long siTotalNodesB = 0;
   double    dTotalTimeA   = 0;
   double    dTotalTimeB   = 0;

   // Baseline Results Containers
   std::vector<long long> vBaselineNodes;
   std::vector<double>    vBaselineTimes;
   bool                   bUseCachedBaseline = false;
   const std::string      strBaselineFile    = "baseline_results.txt";

   if ( !bQuiet )
   {
      if ( !bForceBaseline && LoadBaseline( strBaselineFile, vTestPositions, vBaselineNodes, vBaselineTimes ) )
      {
         bUseCachedBaseline = true;
         std::cout << "Using cached baseline results from " << strBaselineFile << std::endl;
      }
      else
      {
         std::cout << "Running full baseline search..." << std::endl;
      }

      std::cout << std::left << std::setw( 50 ) << "Position"
                << std::setw( 15 ) << "Nodes A"
                << std::setw( 15 ) << "Time A (s)"
                << std::setw( 15 ) << "Nodes B"
                << std::setw( 15 ) << "Time B (s)"
                << std::setw( 15 ) << "Diff (B/A)" << std::endl;
      std::cout << std::string( 125, '-' ) << std::endl;
   }

   for ( size_t i = 0; i < vTestPositions.size(); ++i )
   {
      const auto &strFen = vTestPositions[ i ];

      long long siNodesAPos = 0;
      double    dTimeAPos   = 0;

      if ( !bQuiet )
      {
         if ( bUseCachedBaseline )
         {
            siNodesAPos = vBaselineNodes[ i ];
            dTimeAPos   = vBaselineTimes[ i ];
         }
         else
         {
            // Test Mode A (Baseline)
            gsTempus.giMoveOrderingMode = dMoveOrderingA;
            ReadFEN( strFen.c_str(), argsBoard, argsGeneralMoves, 2 );
            SetSearchDepth( iDepth );
            SetSearchTimeInMiliSeconds( dInfiniteTime );

            ClearHashTable();
            clock_t clockStartA = clock();
            StartSearch( argsBoard, argsGeneralMoves, dAlpha, dBeta );
            clock_t clockEndA = clock();

            siNodesAPos = (long long)GetNumberOfNodesSearched();
            dTimeAPos   = (double)( clockEndA - clockStartA ) / CLOCKS_PER_SEC;

            vBaselineNodes.push_back( siNodesAPos );
            vBaselineTimes.push_back( dTimeAPos );
         }

         siTotalNodesA += siNodesAPos;
         dTotalTimeA += dTimeAPos;
      }

      // Test Mode B
      gsTempus.giMoveOrderingMode = dMoveOrderingB;
      ReadFEN( strFen.c_str(), argsBoard, argsGeneralMoves, 2 );
      SetSearchDepth( iDepth );
      SetSearchTimeInMiliSeconds( dInfiniteTime );

      ClearHashTable();
      clock_t clockStartB = clock();
      StartSearch( argsBoard, argsGeneralMoves, dAlpha, dBeta );
      clock_t clockEndB = clock();

      long long siNodesB = (long long)GetNumberOfNodesSearched();
      double    dTimeB   = (double)( clockEndB - clockStartB ) / CLOCKS_PER_SEC;
      siTotalNodesB += siNodesB;
      dTotalTimeB += dTimeB;

      if ( !bQuiet )
      {
         std::cout << std::left << std::setw( 50 ) << strFen.substr( 0, 45 ) + ( strFen.length() > 45 ? "..." : "" )
                   << std::setw( 15 ) << siNodesAPos
                   << std::setw( 15 ) << std::fixed << std::setprecision( 3 ) << dTimeAPos
                   << std::setw( 15 ) << siNodesB
                   << std::setw( 15 ) << dTimeB
                   << std::setw( 15 ) << (double)siNodesB / ( siNodesAPos > 0 ? siNodesAPos : 1 ) << std::endl;
      }
   }

   if ( !bQuiet )
   {
      if ( !bUseCachedBaseline )
      {
         SaveBaseline( strBaselineFile, vBaselineNodes, vBaselineTimes );
         std::cout << "Baseline results saved to " << strBaselineFile << std::endl;
      }

      std::cout << std::string( 125, '-' ) << std::endl;
      std::cout << "Total Nodes A: " << siTotalNodesA << " Time A: " << dTotalTimeA << "s" << std::endl;
      std::cout << "Total Nodes B: " << siTotalNodesB << " Time B: " << dTotalTimeB << "s" << std::endl;
      std::cout << "Overall Ratio (B/A): " << (double)siTotalNodesB / ( siTotalNodesA > 0 ? siTotalNodesA : 1 ) << std::endl;
   }

   // Restore default mode
   gsTempus.giMoveOrderingMode = dMoveOrderingA;

   return siTotalNodesB;
}