// Copyright 2006 by Jay Lindgren. All Rights Reserved.
//
#define _CRT_SECURE_NO_WARNINGS

//#include "MoveOrder.h"
#include "Functions.h"
#include "Definitions.h"

#include <algorithm> // For std::sort

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

// Global score array to avoid stack allocation in SortMoves and QSortMoves
int g_viScore[dNumberOfMoves];

// Global array to store pre-calculated placement score differences.
// Dimensions: [PieceType][FromSquare][ToSquare]
int g_PlacementScore[13][64][64];

// Sort the moves based on a variety of heuristics, including MVV-LVA for captures.
void SortMovesHash( int* argvsiMoveOrder,
                    Move* argvsMoveList,
                    int argiNumberOfMoves,
                    Board* argsBoard,
                    GeneralMove* argsGeneralMoves,
                    int* viScore)
{
   assert(argsBoard >= 0);
   assert(argsGeneralMoves > 0);
   assert(argiNumberOfMoves >= 0);
   assert(argiNumberOfMoves <= dNumberOfMoves);
   assert(argvsMoveList >= 0);

   const int CAPTURE_BONUS = 1000000;

   for (int iMoveCount = 0; iMoveCount < argiNumberOfMoves; iMoveCount++)
   {
      argvsiMoveOrder[iMoveCount] = iMoveCount;
      Move& currentMove = argvsMoveList[iMoveCount];
      int score = 0;

      if (currentMove.iMoveType == dCapture || currentMove.iMoveType == dCaptureAndPromote || currentMove.iMoveType == dEnPassant) {
         int aggressorType = argsBoard->mBoard[currentMove.iFromSquare];
         int victimType = (currentMove.iMoveType == dEnPassant) ?
            ((argsBoard->siColorToMove == dWhite) ? dBlackPawn : dWhitePawn) :
            argsBoard->mBoard[currentMove.iToSquare];

         if (aggressorType >= 1 && aggressorType <= 12 && victimType >= 1 && victimType <= 12) {
            score = (pieceValues[victimType] * 100) - pieceValues[aggressorType];
         }
         score += CAPTURE_BONUS;
      }
      else {
         score = currentMove.iScore;
      }

      if (currentMove.iMoveType == dPromote || currentMove.iMoveType == dCaptureAndPromote) {
         score += pieceValues[currentMove.iPiece] * 10;
      }
      viScore[iMoveCount] = score;
      viScore[iMoveCount] += UpdateScoreHH(&argvsMoveList[iMoveCount], argsGeneralMoves);
      viScore[iMoveCount] += UpdateScoreKillerMoves(argsBoard, &argvsMoveList[iMoveCount], argsGeneralMoves);
      viScore[iMoveCount] += g_PlacementScore[argvsMoveList[iMoveCount].iPiece][argvsMoveList[iMoveCount].iFromSquare][argvsMoveList[iMoveCount].iToSquare];

      if (argsBoard->iNumberOfPlys + 1 < argsBoard->iMaxPlys - 1)
      {
         if ((argsBoard->vmPrincipalVariation[0][argsBoard->iNumberOfPlys + 1].iFromSquare == argvsMoveList[iMoveCount].iFromSquare) &&
            (argsBoard->vmPrincipalVariation[0][argsBoard->iNumberOfPlys + 1].iToSquare == argvsMoveList[iMoveCount].iToSquare))
         {
            viScore[iMoveCount] += argsGeneralMoves->msPVMove;
         }
      }
   }

# if defined( dUseHash )
   if (GetUseHashTable() && GetQueryState() && (GetBestMove() < 128))
   {
      viScore[GetBestMove()] += argsGeneralMoves->msBestMove;
   }
# endif

   // Use std::sort with a lambda for efficient O(N log N) sorting
   std::sort(argvsiMoveOrder, argvsiMoveOrder + argiNumberOfMoves,
      [&viScore](int index_a, int index_b) {
         return viScore[index_a] > viScore[index_b];
      });
}

//
//------------------------------------------------------------------------------------------------------------
//
void SortMoves(int* argvsiMoveOrder, Move* argvsMoveList, int argNumberOfMoves)
{
   assert(argvsiMoveOrder >= 0);
   assert(argvsMoveList > 0);
   assert(argNumberOfMoves >= 0);
   assert(argNumberOfMoves <= dNumberOfMoves);

   for (int iMoveIndex = 0; iMoveIndex < argNumberOfMoves; iMoveIndex++)
   {
      g_viScore[iMoveIndex] = argvsMoveList[iMoveIndex].iScore;
      argvsiMoveOrder[iMoveIndex] = iMoveIndex;
   }

   // Use insertion sort for small N, as it's often faster than std::sort
   // due to less overhead.
   for (int i = 1; i < argNumberOfMoves; i++)
   {
      int currentMoveIndex = argvsiMoveOrder[i];
      int currentScore = g_viScore[currentMoveIndex];
      int j = i - 1;

      while (j >= 0 && g_viScore[argvsiMoveOrder[j]] < currentScore)
      {
         argvsiMoveOrder[j + 1] = argvsiMoveOrder[j];
         j--;
      }
      argvsiMoveOrder[j + 1] = currentMoveIndex;
   }
}

//
//------------------------------------------------------------------------------------------------------------
// Q-search sorting that only keeps captures
int QSortMoves(int* argvsiMoveOrder, Move* argvsMoveList, int argNumberOfMoves, Board* argsBoard)
{
   assert(argvsiMoveOrder >= 0);
   assert(argvsMoveList >= 0);
   assert(argNumberOfMoves <= dNumberOfMoves);
   assert(argsBoard != nullptr);

   int iCaptureCount = 0;

   for (int iMoveIndex = 0; iMoveIndex < argNumberOfMoves; iMoveIndex++)
   {
      Move& currentMove = argvsMoveList[iMoveIndex];
      if (currentMove.iMoveType == dCapture || currentMove.iMoveType == dCaptureAndPromote || currentMove.iMoveType == dEnPassant)
      {
         int score = 0;
         int aggressorType = argsBoard->mBoard[currentMove.iFromSquare];
         int victimType = (currentMove.iMoveType == dEnPassant) ?
            ((argsBoard->siColorToMove == dWhite) ? dBlackPawn : dWhitePawn) :
            argsBoard->mBoard[currentMove.iToSquare];

         if (aggressorType >= 1 && aggressorType <= 12 && victimType >= 1 && victimType <= 12) {
            score = (pieceValues[victimType] * 100) - pieceValues[aggressorType];
         }

         if (currentMove.iMoveType == dCaptureAndPromote) {
            score += pieceValues[currentMove.iPiece] * 10;
         }

         g_viScore[iMoveIndex] = score;
         argvsiMoveOrder[iCaptureCount] = iMoveIndex;
         iCaptureCount++;
      }
   }

   if (iCaptureCount > 1)
   {
      // Use insertion sort for a small number of captures. It's often faster than std::sort
      // for small N due to less overhead.
      for (int i = 1; i < iCaptureCount; i++)
      {
         int currentMoveIndex = argvsiMoveOrder[i];
         int currentScore = g_viScore[currentMoveIndex];
         int j = i - 1;

         while (j >= 0 && g_viScore[argvsiMoveOrder[j]] < currentScore)
         {
            argvsiMoveOrder[j + 1] = argvsiMoveOrder[j];
            j--;
         }
         argvsiMoveOrder[j + 1] = currentMoveIndex;
      }
   }

   return iCaptureCount;
}

//
//------------------------------------------------------------------------------------------------------------
//  Reset the history heuristic tables
void ResetHistoryHeuristic()
{
   gsSearchParameters.iWhiteHHCount = 1;
   gsSearchParameters.iBlackHHCount = 1;
   for (int i = 0; i < 64; i++) {
      for (int j = 0; j < 64; j++) {
         gsSearchParameters.mWhiteHistoryHeuristic[i][j] = 0;
         gsSearchParameters.mBlackHistoryHeuristic[i][j] = 0;
      }
   }
}

//
//------------------------------------------------------------------------------------------------------------
// Update the history heuristic tables
void UpdateHH(Board* argsBoard, Move* argsMove)
{
   assert(argsBoard >= 0);
   assert(argsMove > 0);

   if (argsMove->iPiece < dBlackPawn) {
      gsSearchParameters.iWhiteHHCount++;
      gsSearchParameters.mWhiteHistoryHeuristic[argsMove->iFromSquare][argsMove->iToSquare]++;
   }
   else {
      gsSearchParameters.iBlackHHCount++;
      gsSearchParameters.mBlackHistoryHeuristic[argsMove->iFromSquare][argsMove->iToSquare]++;
   }
}

//
//------------------------------------------------------------------------------------------------------------
// Update the score based on the history heuristic
int UpdateScoreHH(Move* argsMove, GeneralMove* argsGeneralMoves)
{
   int iScore = 0;
   if (argsMove->iPiece < dBlackPawn && gsSearchParameters.iWhiteHHCount > 0) {
      iScore = int(double(argsGeneralMoves->msKillerMove) * (double(gsSearchParameters.mWhiteHistoryHeuristic[argsMove->iFromSquare][argsMove->iToSquare]) / double(gsSearchParameters.iWhiteHHCount)));
   }
   else if (gsSearchParameters.iBlackHHCount > 0) {
      iScore = int(double(argsGeneralMoves->msKillerMove) * (double(gsSearchParameters.mBlackHistoryHeuristic[argsMove->iFromSquare][argsMove->iToSquare]) / double(gsSearchParameters.iBlackHHCount)));
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
void ResetKillerMoves()
{
   for (int iPlyCount = 0; iPlyCount < dNumberOfMoves; iPlyCount++)
   {
      gsSearchParameters.vKillerMoveCount[iPlyCount] = 1;
      for (int i = 0; i < 64; i++) {
         for (int j = 0; j < 64; j++) {
            gsSearchParameters.mKillerMove[iPlyCount][i][j] = 0;
         }
      }
   }
}

//
//------------------------------------------------------------------------------------------------------------
// Update the killer move tables
void UpdateKillerMoves(Board* argsBoard, Move* argsMove)
{
   assert(argsBoard >= 0);
   assert(argsMove > 0);
   gsSearchParameters.mKillerMove[argsBoard->iNumberOfPlys][argsMove->iFromSquare][argsMove->iToSquare]++;
   gsSearchParameters.vKillerMoveCount[argsBoard->iNumberOfPlys]++;
}

//
//------------------------------------------------------------------------------------------------------------
// Update the score based on the killer move heuristic
int UpdateScoreKillerMoves(Board* argsBoard, Move* argsMove, GeneralMove* argsGeneralMoves)
{
   assert(argsBoard >= 0);
   assert(argsMove > 0);

   if (gsSearchParameters.vKillerMoveCount[argsBoard->iNumberOfPlys] == 0) {
      return 0;
   }

   int iScore = int(double(argsGeneralMoves->msKillerMove) * (double(gsSearchParameters.mKillerMove[argsBoard->iNumberOfPlys][argsMove->iFromSquare][argsMove->iToSquare]) / double(gsSearchParameters.vKillerMoveCount[argsBoard->iNumberOfPlys])));

   assert(iScore >= 0);
   assert(iScore <= sdKillerMove);

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