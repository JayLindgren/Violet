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

// Sort the moves based on a variety of heuristics, including MVV-LVA for captures.
void SortMovesHash(int* argvsiMoveOrder,
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

      if (argsBoard->iNumberOfPlys + 1 < argsBoard->iMaxPlys - 1)
      {
         if ((argsBoard->vmPrincipalVariation[0][argsBoard->iNumberOfPlys + 1].iFromSquare == argvsMoveList[iMoveCount].iFromSquare) &&
            (argsBoard->vmPrincipalVariation[0][argsBoard->iNumberOfPlys + 1].iToSquare == argvsMoveList[iMoveCount].iToSquare))
         {
            viScore[iMoveCount] += argsGeneralMoves->msPVMove;
         }
      }

      switch (argvsMoveList[iMoveCount].iPiece)
      {
      case dWhitePawn:   viScore[iMoveCount] += GetWhitePawnPlacementScore(argvsMoveList[iMoveCount].iToSquare) - GetWhitePawnPlacementScore(argvsMoveList[iMoveCount].iFromSquare);   break;
      case dBlackPawn:   viScore[iMoveCount] += GetBlackPawnPlacementScore(argvsMoveList[iMoveCount].iToSquare) - GetBlackPawnPlacementScore(argvsMoveList[iMoveCount].iFromSquare);   break;
      case dWhiteRook:   viScore[iMoveCount] += GetWhiteRookPlacementScore(argvsMoveList[iMoveCount].iToSquare) - GetWhiteRookPlacementScore(argvsMoveList[iMoveCount].iFromSquare);   break;
      case dBlackRook:   viScore[iMoveCount] += GetBlackRookPlacementScore(argvsMoveList[iMoveCount].iToSquare) - GetBlackRookPlacementScore(argvsMoveList[iMoveCount].iFromSquare);   break;
      case dWhiteKnight: viScore[iMoveCount] += GetWhiteKnightPlacementScore(argvsMoveList[iMoveCount].iToSquare) - GetWhiteKnightPlacementScore(argvsMoveList[iMoveCount].iFromSquare); break;
      case dBlackKnight: viScore[iMoveCount] += GetBlackKnightPlacementScore(argvsMoveList[iMoveCount].iToSquare) - GetBlackKnightPlacementScore(argvsMoveList[iMoveCount].iFromSquare); break;
      case dWhiteBishop: viScore[iMoveCount] += GetWhiteBishopPlacementScore(argvsMoveList[iMoveCount].iToSquare) - GetWhiteBishopPlacementScore(argvsMoveList[iMoveCount].iFromSquare); break;
      case dBlackBishop: viScore[iMoveCount] += GetBlackBishopPlacementScore(argvsMoveList[iMoveCount].iToSquare) - GetBlackBishopPlacementScore(argvsMoveList[iMoveCount].iFromSquare); break;
      case dWhiteQueen:  viScore[iMoveCount] += GetWhiteQueenPlacementScore(argvsMoveList[iMoveCount].iToSquare) - GetWhiteQueenPlacementScore(argvsMoveList[iMoveCount].iFromSquare);  break;
      case dBlackQueen:  viScore[iMoveCount] += GetBlackQueenPlacementScore(argvsMoveList[iMoveCount].iToSquare) - GetBlackQueenPlacementScore(argvsMoveList[iMoveCount].iFromSquare);  break;
      case dWhiteKing:   viScore[iMoveCount] += GetWhiteKingPlacementScore(argvsMoveList[iMoveCount].iToSquare) - GetWhiteKingPlacementScore(argvsMoveList[iMoveCount].iFromSquare);   break;
      case dBlackKing:   viScore[iMoveCount] += GetBlackKingPlacementScore(argvsMoveList[iMoveCount].iToSquare) - GetBlackKingPlacementScore(argvsMoveList[iMoveCount].iFromSquare);   break;
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

void SortMoves(int* argvsiMoveOrder, Move* argvsMoveList, int argNumberOfMoves)
{
   assert(argvsiMoveOrder >= 0);
   assert(argvsMoveList > 0);
   assert(argNumberOfMoves >= 0);
   assert(argNumberOfMoves <= dNumberOfMoves);

   int viScore[dNumberOfMoves];
   for (int iMoveIndex = 0; iMoveIndex < argNumberOfMoves; iMoveIndex++)
   {
      viScore[iMoveIndex] = argvsMoveList[iMoveIndex].iScore;
      argvsiMoveOrder[iMoveIndex] = iMoveIndex;
   }

   // Use std::sort instead of bubble sort for efficiency
   std::sort(argvsiMoveOrder, argvsiMoveOrder + argNumberOfMoves,
      [&viScore](int a, int b) {
         return viScore[a] > viScore[b];
      });
}

int QSortMoves(int* argvsiMoveOrder, Move* argvsMoveList, int argNumberOfMoves)
{
   assert(argvsiMoveOrder >= 0);
   assert(argvsMoveList >= 0);
   assert(argNumberOfMoves <= dNumberOfMoves);

   int iCaptureCount = 0;
   for (int iMoveIndex = 0; iMoveIndex < argNumberOfMoves; iMoveIndex++)
   {
      if (argvsMoveList[iMoveIndex].iMoveType == dCapture)
      {
         argvsiMoveOrder[iCaptureCount] = iMoveIndex;
         iCaptureCount++;
      }
   }
   return iCaptureCount;
}

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

void UpdateKillerMoves(Board* argsBoard, Move* argsMove)
{
   assert(argsBoard >= 0);
   assert(argsMove > 0);
   gsSearchParameters.mKillerMove[argsBoard->iNumberOfPlys][argsMove->iFromSquare][argsMove->iToSquare]++;
   gsSearchParameters.vKillerMoveCount[argsBoard->iNumberOfPlys]++;
}

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