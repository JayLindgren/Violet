// Copyright 2006 by Jay Lindgren. All Rights Reserved.
//

#define _CRT_SECURE_NO_WARNINGS
#include "Definitions.h"
#include "Functions.h"
#include "Structures.h"
#include "MoveGen.h" // For PEXT-based attack generation
#include "nnue.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

Evaluator gsEvaluator;

// Forward declarations for helper functions
int GetUseMobility();
int CountPiecesAndEvaluatePosition( struct Board *argsBoard, struct GeneralMove *argsGeneralMoves, int iScore );
int PawnMisc( struct Board *argsBoard, struct GeneralMove *argsGeneralMoves, int argiSquareIndex );
int RookMobility( struct Board *argsBoard, struct GeneralMove *argsGeneralMoves, int argiSquareIndex );
int BishopMobility( struct Board *argsBoard, struct GeneralMove *argsGeneralMoves, int argiSquareIndex );
int KnightMobility( struct Board *argsBoard, struct GeneralMove *argsGeneralMoves, int argiSquareIndex );
int QueenMobility( struct Board *argsBoard, struct GeneralMove *argsGeneralMoves, int argiSquareIndex );

// Evaluate a position.
//
// Note that the default is to score the position from the white point of view
// and then flip the sign.
//
int EvaluateBoard( struct Board       *argsBoard,
                   struct GeneralMove *argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Allocate the initialize the score.
   int iScore = 0;

// See if we are using NNUE or the old method.
#if dUseNNUE

   // NNUE returns score relative to side to move, so no sign flip needed.
   return EvaluateBoardDirectNNUE( argsBoard, argsGeneralMoves );

#else

   // Count the pieces and value their positions.
   iScore = CountPiecesAndEvaluatePosition( argsBoard, argsGeneralMoves, iScore );

   // Return the standard score.
   if ( argsBoard->siColorToMove == dBlack )
   {

      iScore = -iScore;
   }

   return iScore;
#endif
}

//
//
//---------------------------------------------------------------------
//
//

int CountPiecesAndEvaluatePosition( struct Board       *argsBoard,
                                    struct GeneralMove *argsGeneralMoves,
                                    int                 iScore )
{

   // Debug the inpput.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );
   assert( CheckBoard( argsBoard ) );
   assert( argsBoard->bbWhiteKing != 0 );
   assert( argsBoard->bbBlackKing != 0 );

   // Count the pieces and evaluate their position.
   int iScoreOpening = 0;
   int iScoreEndGame = 0;
   int iSquareIndex;
   int iWhiteKingSquare  = 0;
   int iBlackKingSquare  = 0;
   int iWhiteBishopCount = 0;
   int iBlackBishopCount = 0;
   int iPhaseOfGame      = 0;
   int iScoreOldOpening  = 0; // used for debugging.
   int iScoreOldEndGame  = 0; // used for debugging.

   // Find the king squares for king safety calculations.
   Find( argsBoard->bbWhiteKing, &iWhiteKingSquare, argsGeneralMoves );
   Find( argsBoard->bbBlackKing, &iBlackKingSquare, argsGeneralMoves );

   // Loop over the board.
   for ( iSquareIndex = 0; iSquareIndex < 64; iSquareIndex++ )
   {

      // Switch depending on the piece on the square.
      switch ( argsBoard->mBoard[ iSquareIndex ] )
      {

      case dEmpty:
      {

         break;
      }
      case dWhitePawn:
      {

         // Update the phase.
         iPhaseOfGame += dPhasePawn;

         // Value the piece.
         iScoreOpening += dValuePawnOpening;
         iScoreEndGame += dValuePawnEndGame;

         // Value the placement.
         iScoreOpening += gsEvaluator.viPlacementPawnWhite[ iSquareIndex ];
         iScoreEndGame += gsEvaluator.viPlacementPawnWhite[ iSquareIndex ];

         // Add the attacks on the king.
         if ( ( argsGeneralMoves->bbWPAttack[ iSquareIndex ] &
                argsGeneralMoves->bbKingBubble[ iBlackKingSquare ] ) > 0 )
         {

            iScoreOpening += dKingOpaqueAttack; // A little King safety.
            iScoreEndGame += dKingOpaqueAttack; // A little King safety.
         }

         // Add a score for a passed pawns
         int iPawnMisc = PawnMisc( argsBoard, argsGeneralMoves, iSquareIndex );
         iScoreOpening += iPawnMisc;
         iScoreEndGame += iPawnMisc;

         break;
      }
      case dBlackPawn:
      {

         // Update the phase.
         iPhaseOfGame += dPhasePawn;

         // Value the piece.
         iScoreOpening -= dValuePawnOpening;
         iScoreEndGame -= dValuePawnEndGame;

         // Value the placement.
         iScoreOpening -=
             gsEvaluator.viPlacementPawnBlack[ gsEvaluator.viReverse[ iSquareIndex ] ];
         iScoreEndGame -=
             gsEvaluator.viPlacementPawnBlack[ gsEvaluator.viReverse[ iSquareIndex ] ];

         // Add the attacks on the king.
         if ( ( argsGeneralMoves->bbBPAttack[ iSquareIndex ] &
                argsGeneralMoves->bbKingBubble[ iWhiteKingSquare ] ) > 0 )
         {

            iScoreOpening -= dKingOpaqueAttack; // A little King safety.
            iScoreEndGame -= dKingOpaqueAttack; // A little King safety.
         }

         // Add a score for a passed pawns
         int iPawnMisc = PawnMisc( argsBoard, argsGeneralMoves, iSquareIndex );
         iScoreOpening -= iPawnMisc;
         iScoreEndGame -= iPawnMisc;

         break;
      }
      case dWhiteRook:
      {

         // Update the phase.
         iPhaseOfGame += dPhaseRook;

         // Value the piece.
         iScoreOpening += dValueRookOpening;
         iScoreEndGame += dValueRookEndGame;

         // Add in the rook placement.
         int iRookPlacement = gsEvaluator.viPlacementRook[ iSquareIndex ];
         iScoreOpening += iRookPlacement;
         iScoreEndGame += iRookPlacement;

         // Do the rook file thing.
         // Look for a semi open file.
         if ( !( argsGeneralMoves->vbbRow[ iSquareIndex ] & argsBoard->bbBlackPawn ) )
         {

            // Look for a completely open file.
            if ( !( argsGeneralMoves->vbbRow[ iSquareIndex ] &
                    argsBoard->bbWhitePawn ) )
            {

               // Update the scores for open files.
               iScoreOpening += dRookOnOpenFileOpening;
               iScoreEndGame += dRookOnOpenFileEndGame;

               // Add in a potential open file king attack bonus.
               if ( argsGeneralMoves->vbbRow[ iSquareIndex ] & argsBoard->bbBlackKing )
               {

                  iScoreOpening += dRookOnOpenKingFileOpening;
                  iScoreEndGame += dRookOnOpenKingFileEndGame;
               }
            }
            else
            {

               // Update the scores for semiopen files.
               iScoreOpening += dRookOnSemiOpenFileOpening;
               iScoreEndGame += dRookOnSemiOpenFileEndGame;

               // Add in a potential semiopen king attack bonus.
               if ( argsGeneralMoves->vbbRow[ iSquareIndex ] & argsBoard->bbBlackKing )
               {

                  iScoreOpening += dRookOnSemiOpenKingFileOpening;
                  iScoreEndGame += dRookOnSemiOpenKingFileEndGame;
               }
            }
         }

         // Value a king attack (using PEXT)
         if ( ( GetRookAttacksPEXT( iSquareIndex, argsBoard->bbAllPieces ) &
                argsGeneralMoves->bbKingBubble[ iBlackKingSquare ] ) > 0 )
         {

            iScoreOpening += dKingOpaqueAttack; // A little King safety.
            iScoreEndGame += dKingOpaqueAttack; // A little King safety.
         }

         // Add mobility.
         if ( GetUseMobility() == dYes )
         {

            // Calculate the rook mobility
            int iRookMobility =
                RookMobility( argsBoard, argsGeneralMoves, iSquareIndex );

            // Weight the mobility and put it in the score.
            iScoreOpening += iRookMobility * dMobilityRookOpening;
            iScoreOpening += iRookMobility * dMobilityRookEndGame;
         }

         break;
      }
      case dBlackRook:
      {

         // Update the phase.
         iPhaseOfGame += dPhaseRook;

         // Value the piece.
         iScoreOpening -= dValueRookOpening;
         iScoreEndGame -= dValueRookEndGame;

         // Value the rook placement
         int iRookPlacement =
             gsEvaluator.viPlacementRook[ gsEvaluator.viReverse[ iSquareIndex ] ];
         iScoreOpening -= iRookPlacement;
         iScoreEndGame -= iRookPlacement;

         // Do the rook file thing.
         // Look for a semi open file.
         if ( !( argsGeneralMoves->vbbRow[ iSquareIndex ] & argsBoard->bbWhitePawn ) )
         {

            // Look for a completely open file.
            if ( !( argsGeneralMoves->vbbRow[ iSquareIndex ] &
                    argsBoard->bbBlackPawn ) )
            {

               // Update the scores for open files.
               iScoreOpening -= dRookOnOpenFileOpening;
               iScoreEndGame -= dRookOnOpenFileEndGame;

               // Add in a potential open file king attack bonus.
               if ( argsGeneralMoves->vbbRow[ iSquareIndex ] & argsBoard->bbWhiteKing )
               {

                  iScoreOpening -= dRookOnOpenKingFileOpening;
                  iScoreEndGame -= dRookOnOpenKingFileEndGame;
               }
            }
            else
            {

               // Update the scores for semiopen files.
               iScoreOpening -= dRookOnSemiOpenFileOpening;
               iScoreEndGame -= dRookOnSemiOpenFileEndGame;

               // Add in a potential semiopen king attack bonus.
               if ( argsGeneralMoves->vbbRow[ iSquareIndex ] & argsBoard->bbWhiteKing )
               {

                  iScoreOpening -= dRookOnSemiOpenKingFileOpening;
                  iScoreEndGame -= dRookOnSemiOpenKingFileEndGame;
               }
            }
         }

         // Value a king attack (using PEXT)
         if ( ( GetRookAttacksPEXT( iSquareIndex, argsBoard->bbAllPieces ) &
                argsGeneralMoves->bbKingBubble[ iWhiteKingSquare ] ) > 0 )
         {

            iScoreOpening -= dKingOpaqueAttack; // A little King safety.
            iScoreEndGame -= dKingOpaqueAttack; // A little King safety.
         }

         // Add mobility.
         if ( GetUseMobility() == dYes )
         {

            // Calculate the rook mobility
            int iRookMobility =
                RookMobility( argsBoard, argsGeneralMoves, iSquareIndex );

            // Weight the mobility and put it in the score.
            iScoreOpening -= iRookMobility * dMobilityRookOpening;
            iScoreOpening -= iRookMobility * dMobilityRookEndGame;
         }

         break;
      }
      case dWhiteKnight:
      {

         // Update the phase.
         iPhaseOfGame += dPhaseKnight;

         // Value the piece.
         iScoreOpening += dValueKnightOpening;
         iScoreEndGame += dValueKnightEndGame;

         // Add in the Knight placement.
         int iKnightPlacement = gsEvaluator.viPlacementKnight[ iSquareIndex ];
         iScoreOpening += iKnightPlacement;
         iScoreEndGame += iKnightPlacement;

         // Value a king attack
         if ( ( argsGeneralMoves->bbNMove[ iSquareIndex ] &
                argsGeneralMoves->bbKingBubble[ iBlackKingSquare ] ) > 0 )
         {

            iScoreOpening += dKingOpaqueAttack; // A little King safety.
            iScoreEndGame += dKingOpaqueAttack; // A little King safety.
         }

         // Add mobility.
         if ( GetUseMobility() == dYes )
         {

            // Calculate the Knight mobility
            int iKnightMobility =
                KnightMobility( argsBoard, argsGeneralMoves, iSquareIndex );

            // Weight the mobility and put it in the score.
            iScoreOpening += iKnightMobility * dMobilityKnightOpening;
            iScoreOpening += iKnightMobility * dMobilityKnightEndGame;
         }

         break;
      }
      case dBlackKnight:
      {

         // Update the phase.
         iPhaseOfGame += dPhaseKnight;

         // Value the piece.
         iScoreOpening -= dValueKnightOpening;
         iScoreEndGame -= dValueKnightEndGame;

         // Value the Knight placement
         int iKnightPlacement =
             gsEvaluator.viPlacementKnight[ gsEvaluator.viReverse[ iSquareIndex ] ];
         iScoreOpening -= iKnightPlacement;
         iScoreEndGame -= iKnightPlacement;

         // Value a king attack
         if ( ( argsGeneralMoves->bbNMove[ iSquareIndex ] &
                argsGeneralMoves->bbKingBubble[ iWhiteKingSquare ] ) > 0 )
         {

            iScoreOpening -= dKingOpaqueAttack; // A little King safety.
            iScoreEndGame -= dKingOpaqueAttack; // A little King safety.
         }

         // Add mobility.
         if ( GetUseMobility() == dYes )
         {

            // Calculate the Knight mobility
            int iKnightMobility =
                KnightMobility( argsBoard, argsGeneralMoves, iSquareIndex );

            // Weight the mobility and put it in the score.
            iScoreOpening -= iKnightMobility * dMobilityKnightOpening;
            iScoreOpening -= iKnightMobility * dMobilityKnightEndGame;
         }

         break;
      }
      case dWhiteBishop:
      {

         // Update the phase.
         iPhaseOfGame += dPhaseBishop;

         // Value the piece.
         iScoreOpening += dValueBishopOpening;
         iScoreEndGame += dValueBishopEndGame;

         // Add in the Bishop placement.
         int iBishopPlacement = gsEvaluator.viPlacementBishop[ iSquareIndex ];
         iScoreOpening += iBishopPlacement;
         iScoreEndGame += iBishopPlacement;

         // Value a king attack (using PEXT)
         if ( ( GetBishopAttacksPEXT( iSquareIndex, argsBoard->bbAllPieces ) &
                argsGeneralMoves->bbKingBubble[ iBlackKingSquare ] ) > 0 )
         {

            iScoreOpening += dKingOpaqueAttack; // A little King safety.
            iScoreEndGame += dKingOpaqueAttack; // A little King safety.
         }

         // Add mobility.
         if ( GetUseMobility() == dYes )
         {

            // Calculate the Bishop mobility
            int iBishopMobility =
                BishopMobility( argsBoard, argsGeneralMoves, iSquareIndex );

            // Weight the mobility and put it in the score.
            iScoreOpening += iBishopMobility * dMobilityBishopOpening;
            iScoreOpening += iBishopMobility * dMobilityBishopEndGame;
         }

         break;
      }
      case dBlackBishop:
      {

         // Update the phase.
         iPhaseOfGame += dPhaseBishop;

         // Value the piece.
         iScoreOpening -= dValueBishopOpening;
         iScoreEndGame -= dValueBishopEndGame;

         // Value the Bishop placement
         int iBishopPlacement =
             gsEvaluator.viPlacementBishop[ gsEvaluator.viReverse[ iSquareIndex ] ];
         iScoreOpening -= iBishopPlacement;
         iScoreEndGame -= iBishopPlacement;

         // Value a king attack (using PEXT)
         if ( ( GetBishopAttacksPEXT( iSquareIndex, argsBoard->bbAllPieces ) &
                argsGeneralMoves->bbKingBubble[ iWhiteKingSquare ] ) > 0 )
         {

            iScoreOpening -= dKingOpaqueAttack; // A little King safety.
            iScoreEndGame -= dKingOpaqueAttack; // A little King safety.
         }

         // Add mobility.
         if ( GetUseMobility() == dYes )
         {

            // Calculate the Bishop mobility
            int iBishopMobility =
                BishopMobility( argsBoard, argsGeneralMoves, iSquareIndex );

            // Weight the mobility and put it in the score.
            iScoreOpening -= iBishopMobility * dMobilityBishopOpening;
            iScoreOpening -= iBishopMobility * dMobilityBishopEndGame;
         }

         break;
      }
      case dWhiteQueen:
      {

         // Update the phase.
         iPhaseOfGame += dPhaseQueen;

         // Value the piece.
         iScoreOpening += dValueQueenOpening;
         iScoreEndGame += dValueQueenEndGame;

         // Add in the Queen placement.
         int iQueenPlacement = gsEvaluator.viPlacementQueen[ iSquareIndex ];
         iScoreOpening += iQueenPlacement;
         // iScoreEndGame += iQueenPlacement;  // Taken out because I don't want it
         // in the end game.

         // Value a king attack (using PEXT)
         if ( ( GetQueenAttacksPEXT( iSquareIndex, argsBoard->bbAllPieces ) &
                argsGeneralMoves->bbKingBubble[ iBlackKingSquare ] ) > 0 )
         {

            iScoreOpening += dKingOpaqueAttack; // A little King safety.
            iScoreEndGame += dKingOpaqueAttack; // A little King safety.
         }

         // Add mobility.
         if ( GetUseMobility() == dYes )
         {

            // Calculate the Queen mobility
            int iQueenMobility =
                QueenMobility( argsBoard, argsGeneralMoves, iSquareIndex );

            // Weight the mobility and put it in the score.
            iScoreOpening += iQueenMobility * dMobilityQueenOpening;
            iScoreEndGame += iQueenMobility * dMobilityQueenEndGame;
         }

         break;
      }
      case dBlackQueen:
      {

         // Update the phase.
         iPhaseOfGame += dPhaseQueen;

         // Value the piece.
         iScoreOpening -= dValueQueenOpening;
         iScoreEndGame -= dValueQueenEndGame;

         // Value the Queen placement
         int iQueenPlacement =
             gsEvaluator.viPlacementQueen[ gsEvaluator.viReverse[ iSquareIndex ] ];
         iScoreOpening -= iQueenPlacement;
         // iScoreEndGame -= iQueenPlacement; // Taken out because I don't want it
         // in the end game.

         // Value a king attack (using PEXT)
         if ( ( GetQueenAttacksPEXT( iSquareIndex, argsBoard->bbAllPieces ) &
                argsGeneralMoves->bbKingBubble[ iWhiteKingSquare ] ) > 0 )
         {

            iScoreOpening -= dKingOpaqueAttack; // A little King safety.
            iScoreEndGame -= dKingOpaqueAttack; // A little King safety.
         }

         // Add mobility.
         if ( GetUseMobility() == dYes )
         {

            // Calculate the Queen mobility
            int iQueenMobility =
                QueenMobility( argsBoard, argsGeneralMoves, iSquareIndex );

            // Weight the mobility and put it in the score.
            iScoreOpening -= iQueenMobility * dMobilityQueenOpening;
            iScoreEndGame -= iQueenMobility * dMobilityQueenEndGame;
         }

         break;
      }
      case dWhiteKing:
      {

         // Value the piece.
         iScoreOpening += dValueKingOpening;
         iScoreEndGame += dValueKingEndGame;

         // Add the score of the king's position
         iScoreOpening += gsEvaluator.viPlacementKingWhite[ iSquareIndex ];

         // Add the value of a pawn fence, but only for the opening.
         // Only give credit for the fence after both sides have castled.
         BitBoard bbFence =
             argsBoard->bbWhitePawn & gsEvaluator.vbbKingFence[ iSquareIndex ];
         if ( Count( bbFence ) == 3 && argsBoard->bbCastle == 0 )
         {

            iScoreOpening += dKingFence;
         }

         break;
      }
      case dBlackKing:
      {

         // Value the piece.
         iScoreOpening -= dValueKingOpening;
         iScoreEndGame -= dValueKingEndGame;

         // Add the score of the king's position
         iScoreOpening -= gsEvaluator.viPlacementKingBlack[ iSquareIndex ];

         // Add the value of a pawn fence, but only for the opening.
         // Only give credit for the fence after both sides have castled.
         BitBoard bbFence =
             argsBoard->bbBlackPawn & gsEvaluator.vbbKingFence[ iSquareIndex ];
         if ( Count( bbFence ) == 3 && argsBoard->bbCastle == 0 )
         {

            iScoreOpening -= dKingFence;
         }

         break;
      }
      }

      /*
            // Count some square and scores.
            cout << "Row = " << argsGeneralMoves->viRow[ iSquareIndex ] << " Col =
         " << argsGeneralMoves->viCol[ iSquareIndex ]
                 << " OpenDiff = " << iScoreOpening - iScoreOldOpening << "
         EndDiff = " << iScoreEndGame - iScoreOldEndGame << endl; iScoreOldOpening
         = iScoreOpening; iScoreOldEndGame = iScoreEndGame;
      */
   }

   // cout << endl << "Score at the end of the squares = " << iScoreOpening << "
   // " << iScoreEndGame << endl;

   // Addd a bonus for having the bishop pair.
   if ( iWhiteBishopCount >= 2 )
   {

      iScoreOpening += dBishopPair;
      iScoreEndGame += dBishopPair;
   }
   if ( iBlackBishopCount >= 2 )
   {

      iScoreOpening -= dBishopPair;
      iScoreEndGame -= dBishopPair;
   }

   /*
         // Count some square and scores.
         cout << "Bishop Pair " << " OpenDiff = " << iScoreOpening -
      iScoreOldOpening << " EndDiff = " << iScoreEndGame - iScoreOldEndGame <<
      endl; iScoreOldOpening = iScoreOpening; iScoreOldEndGame = iScoreEndGame;
   */

   // bit 1 - white king side castle.
   // bit 2 - black king side castle.
   // bit 3 - white queen side castle.
   // bit 4 - black queen side castle.
   // Define the penality for no casteling
   // Add penalties for not casteling.
   if ( argsBoard->iHasWhiteCastled == dNo )
   {

      iScoreOpening -= dNoCastlePenalty;
   }
   if ( argsBoard->iHasBlackCastled == dNo )
   {

      iScoreOpening += dNoCastlePenalty;
   }

   /*
         // Count some square and scores.
         cout << "Casteling " << " OpenDiff = " << iScoreOpening -
      iScoreOldOpening << " EndDiff = " << iScoreEndGame - iScoreOldEndGame <<
      endl; iScoreOldOpening = iScoreOpening; iScoreOldEndGame = iScoreEndGame;
   */

   // Weight the score:
   iScore = ( ( dMaxPhase - iPhaseOfGame ) * iScoreEndGame +
              iPhaseOfGame * iScoreOpening ) /
            dMaxPhase;

   return iScore;
}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
int RookMobility( struct Board *argsBoard, struct GeneralMove *argsGeneralMoves,
                  int argiSquareIndex )
{
   // Calculate rook attacks using PEXT
   BitBoard rookAttacks = GetRookAttacksPEXT( argiSquareIndex, argsBoard->bbAllPieces );

   // Count mobility (squares attacked excluding own pieces)
   BitBoard mobility;
   if ( argsBoard->siColorToMove == dWhite )
      mobility = rookAttacks & ~argsBoard->bbWhitePieces;
   else
      mobility = rookAttacks & ~argsBoard->bbBlackPieces;

   return (int)Count( mobility );
}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
int BishopMobility( struct Board       *argsBoard,
                    struct GeneralMove *argsGeneralMoves, int argiSquareIndex )
{
   // Calculate bishop attacks using PEXT
   BitBoard bishopAttacks = GetBishopAttacksPEXT( argiSquareIndex, argsBoard->bbAllPieces );

   // Count mobility (squares attacked excluding own pieces)
   BitBoard mobility;
   if ( argsBoard->siColorToMove == dWhite )
      mobility = bishopAttacks & ~argsBoard->bbWhitePieces;
   else
      mobility = bishopAttacks & ~argsBoard->bbBlackPieces;

   return (int)Count( mobility );
}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
int QueenMobility( struct Board *argsBoard, struct GeneralMove *argsGeneralMoves,
                   int argiSquareIndex )
{
   // Calculate queen attacks using PEXT (rook + bishop)
   BitBoard queenAttacks = GetQueenAttacksPEXT( argiSquareIndex, argsBoard->bbAllPieces );

   // Count mobility (squares attacked excluding own pieces)
   BitBoard mobility;
   if ( argsBoard->siColorToMove == dWhite )
      mobility = queenAttacks & ~argsBoard->bbWhitePieces;
   else
      mobility = queenAttacks & ~argsBoard->bbBlackPieces;

   return (int)Count( mobility );
}

//
//
//---------------------------------------------------------------------
//
//
int SliderMobility( struct Board *argsBoard, int *pMoves,
                    int numberOfGeneralMoves, int startSquare )
{

   // This evaluation is color specific.
   // Attacks are included in the count.

   int moveIndex = 0;

   // Find the slider moves.
   if ( numberOfGeneralMoves <= 0 )
   {

      return moveIndex;
   }

   // Start the slice
   int jumpOut = 1; // this is placed lower in the program for efficiency.
   while ( jumpOut )
   {

      // Extract the square occupant.
      int squareOccupant = argsBoard->mBoard[ pMoves[ moveIndex ] ];

      if ( squareOccupant == dEmpty )
      {

         moveIndex++;
      }
      else if ( squareOccupant < dBlackPawn )
      {

         if ( argsBoard->siColorToMove == dWhite )
         {

            // Piece is blocked
            return moveIndex;
         }
         else
         {

            // Capture
            return moveIndex++;
         }
      }
      else if ( squareOccupant > dWhiteKing )
      {

         if ( argsBoard->siColorToMove == dBlack )
         {

            // Piece is blocked
            return moveIndex;
         }
         else
         {

            // Capture
            return moveIndex++;
         }
      }

      // Look for the end of the line.
      if ( moveIndex >= numberOfGeneralMoves )
      {

         return moveIndex;
      }
   }

   // This point should not be reached, but included for completeness.
   if ( moveIndex > 8 )
   {
      moveIndex = 0;
   }
   return moveIndex;
}
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
int KnightMobility( struct Board       *argsBoard,
                    struct GeneralMove *argsGeneralMoves, int argiSquareIndex )
{

   int iNumberOfMoves = 0;

   // Look for the good moves..
   BitBoard bbMoves =
       argsGeneralMoves->bbNMove[ argiSquareIndex ] & ( ~argsBoard->bbAllPieces );

   // Look for Attacks
   BitBoard bbAttacks = 0;
   if ( argsBoard->siColorToMove == dWhite )
   {

      BitBoard bbAttacks =
          argsGeneralMoves->bbNMove[ argiSquareIndex ] & ( argsBoard->bbBlackPieces );
   }
   else
   {

      BitBoard bbAttacks =
          argsGeneralMoves->bbNMove[ argiSquareIndex ] & ( argsBoard->bbWhitePieces );
   }

   // Count the totals
   iNumberOfMoves = (int)Count( bbMoves );
   iNumberOfMoves += (int)Count( bbAttacks );

   return iNumberOfMoves;
}

//
//
//---------------------------------------------------------------------
//
//

//
//
//---------------------------------------------------------------------
//
//
int LookForDraw( struct Board *argsBoard, struct GeneralMove *argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Look for a three fold repetition.
   // Start with 1 because the current position is the first occurrence.
   int iDrawCount = 1;
   int iPlyCount;

   // Get the current hash.
   BitBoard bbCurrentHash = argsBoard->bbHash;

   // Check the initial position (not in history stack)
   if ( GetHashInitial() == bbCurrentHash )
   {
      iDrawCount++;
   }

   // Loop over the history.
   // We only need to check back to the last irreversible move (capture or pawn
   // move), which is tracked by iFiftyMoveCounter. We stop at iMoveHistory - 1
   // because we are comparing past positions to the current one.
   for ( iPlyCount = argsBoard->iMoveHistory - argsBoard->iFiftyMoveCounter;
         iPlyCount < argsBoard->iMoveHistory; iPlyCount++ )
   {

      // Check for hash match first (fastest)
      if ( argsBoard->sHistoryStack[ iPlyCount ].bbHash == bbCurrentHash )
      {

         // Strict check for collisions:
         // 1. Castling rights must match
         // 2. En Passant square must match
         if ( argsBoard->sHistoryStack[ iPlyCount ].bbCastle == argsBoard->bbCastle &&
              argsBoard->sHistoryStack[ iPlyCount ].bbEPSquare == argsBoard->bbEP )
         {
            iDrawCount++;
         }
      }
   }

   if ( iDrawCount >= 3 )
   {

      // PrintBoard( argsBoard->mBoard );

      // Return that a draw has been found.
      return 1;
   }
   else
   {

      // Return that a draw has not been found.
      return 0;
   }
}

//
//
//---------------------------------------------------------------------
//
//
int GetValueOfDraw()
{
   return dDrawValue;
}

//
//
//---------------------------------------------------------------------
//
//
void InitializeEvaluator()
{

   // Initialize the evaluator with score information.

   // Set up the reverse the scoring for black from white square definitions.
   int iNumberOfSquares = 64;
   for ( int iIndex = 0; iIndex < 64; iIndex++ )
   {

      iNumberOfSquares--;
      gsEvaluator.viReverse[ iIndex ] = iNumberOfSquares;
   }

   int viPlacementPawnWhite[ 64 ] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       1, 2, 3, 6, 6, 3, 2, 1, 2, 4, 6, 10, 10, 6, 4, 2,
       3, 6, 9, 12, 12, 9, 6, 3, 4, 8, 12, 16, 16, 12, 8, 4,
       5, 10, 15, 20, 20, 15, 10, 5, 0, 0, 0, 0, 0, 0, 0, 0 };

   int viPlacementPawnBlack[ 64 ] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       1, 2, 3, 6, 6, 3, 2, 1, 2, 4, 6, 10, 10, 6, 4, 2,
       3, 6, 9, 12, 12, 9, 6, 3, 4, 8, 12, 16, 16, 12, 8, 4,
       5, 10, 15, 20, 20, 15, 10, 5, 0, 0, 0, 0, 0, 0, 0, 0 };

   // #pragma omp parallel for schedule( dynamic, 1 )
   for ( int iIndex = 0; iIndex < 64; iIndex++ )
   {

      gsEvaluator.viPlacementPawnWhite[ iIndex ] = viPlacementPawnWhite[ iIndex ];
      gsEvaluator.viPlacementPawnBlack[ iIndex ] = viPlacementPawnBlack[ iIndex ];
   }

   int viPlacementRook[ 64 ] = { -1, 5, 7, 10, 10, 7, 5, -1, 0, 0, 0, 10, 10,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 11, 11, 15,
                                 15, 11, 11, 11, 3, 4, 5, 15, 15, 5, 4, 3 };

   for ( int iIndex = 0; iIndex < 64; iIndex++ )
   {

      gsEvaluator.viPlacementRook[ iIndex ] = viPlacementRook[ iIndex ];
   }

   int viPlacementKnight[ 64 ] = {
       -10, -15, -10, -10, -10, -10, -15, -10, -10, 0, 0, 0, 0,
       0, 0, -10, -10, 0, 5, 5, 5, 5, 0, -10, -10, 0,
       5, 10, 10, 5, 0, -10, -10, 0, 5, 10, 10, 5, 0,
       -10, -10, 0, 6, 6, 6, 6, 0, -10, -10, 0, 0, 0,
       0, 0, 0, -10, -10, -15, -10, -10, -10, -10, -15, -10 };

   for ( int iIndex = 0; iIndex < 64; iIndex++ )
   {

      gsEvaluator.viPlacementKnight[ iIndex ] = viPlacementKnight[ iIndex ];
   }

   int viPlacementBishop[ 64 ] = {
       -10, -10, -15, -10, -10, -15, -10, -10, -10, 1, 1, 1, 1,
       1, 1, -10, -10, 1, 5, 5, 5, 5, 1, -10, -10, 1,
       5, 10, 10, 5, 1, -10, -10, 1, 5, 10, 10, 5, 1,
       -10, -10, 1, 5, 5, 5, 5, 1, -10, -10, 1, 1, 1,
       1, 1, 1, -10, -10, -10, -10, -10, -10, -10, -10, -10 };

   for ( int iIndex = 0; iIndex < 64; iIndex++ )
   {

      gsEvaluator.viPlacementBishop[ iIndex ] = viPlacementBishop[ iIndex ];
   }

   int viPlacementQueen[ 64 ] = {
       -10, -10, -10, 0, 0, -10, -10, -10, -10, 0, 0, 0, 0,
       0, 0, -10, -10, 0, -1, -1, -1, -1, 0, -10, -10, 0,
       -1, -2, -2, -1, 0, -10, -10, 0, -1, -2, -2, -1, 0,
       -10, -10, 0, -1, -1, -1, -1, 0, -10, 10, 10, 10, 10,
       10, 10, 10, 10, -10, -10, -10, -11, -11, -10, -10, -10 };

   for ( int iIndex = 0; iIndex < 64; iIndex++ )
   {

      gsEvaluator.viPlacementQueen[ iIndex ] = viPlacementQueen[ iIndex ];
   }

   int viPlacementKingWhite[ 64 ] = {
       2, 2, 4, -2, -2, 2, 4, 2, -2, -2, -2, -2, -2, -2, -2, -2,
       -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
       -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
       -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4 };

   for ( int iIndex = 0; iIndex < 64; iIndex++ )
   {

      gsEvaluator.viPlacementKingWhite[ iIndex ] = viPlacementKingWhite[ iIndex ];
   }

   int viPlacementKingBlack[ 64 ] = {
       -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
       -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
       -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
       -2, -2, -2, -2, -2, -2, -2, -2, 2, 2, 4, -2, -2, 2, 4, 2 };

   for ( int iIndex = 0; iIndex < 64; iIndex++ )
   {

      gsEvaluator.viPlacementKingBlack[ iIndex ] = viPlacementKingBlack[ iIndex ];
   }

   int viReverse[ 64 ] = { dH8, dG8, dF8, dE8, dD8, dC8, dB8, dA8, dH7, dG7, dF7,
                           dE7, dD7, dC7, dB7, dA7, dH6, dG6, dF6, dE6, dD6, dC6,
                           dB6, dA6, dH5, dG5, dF5, dE5, dD5, dC5, dB5, dA5, dH4,
                           dG4, dF4, dE4, dD4, dC4, dB4, dA4, dH3, dG3, dF3, dE3,
                           dD3, dC3, dB3, dA3, dH2, dG2, dF2, dE2, dD2, dC2, dB2,
                           dA2, dH1, dG1, dF1, dE1, dD1, dC1, dB1, dA1 };

   for ( int iIndex = 0; iIndex < 64; iIndex++ )
   {

      gsEvaluator.viReverse[ iIndex ] = viReverse[ iIndex ];
   }

   // Set up a pawn fence in front of the king.  In the future, this will include
   // a pawn hash table.
   for ( int iIndex = 0; iIndex < 64; iIndex++ )
   {

      gsEvaluator.vbbKingFence[ iIndex ] = 0;
   }

   // Set up the fence for several key positions.
   int iSquare = dA1;
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dA2 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dA3 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dB2 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dB3 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dC2 );

   iSquare = dB1;
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dA2 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dA3 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dB2 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dB3 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dC2 );

   iSquare = dC1;
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dA2 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dA3 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dB2 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dB3 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dC2 );

   // Allow for a h3 move.
   iSquare = dF1;
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dF2 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dG2 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dG3 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dH2 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dH3 );

   iSquare = dG1;
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dF2 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dG2 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dG3 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dH2 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dH3 );

   iSquare = dH1;
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dF2 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dG2 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dG3 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dH2 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dH3 );

   // Set up the black fence
   iSquare = dA8;
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dA7 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dA6 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dB7 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dB6 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dC7 );

   iSquare = dB8;
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dA7 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dA6 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dB7 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dB6 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dC7 );

   iSquare = dC8;
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dA7 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dA6 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dB7 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dB6 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dC7 );

   // Allow for a h3 move.
   iSquare = dF8;
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dF7 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dG7 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dG6 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dH7 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dH6 );

   iSquare = dG8;
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dF7 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dG7 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dG6 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dH7 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dH6 );

   iSquare = dH8;
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dF7 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dG7 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dG6 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dH7 );
   gsEvaluator.vbbKingFence[ iSquare ] =
       SetBitToOne( gsEvaluator.vbbKingFence[ iSquare ], dH6 );

   // Seed the random number generator.
   srand( (int)time( NULL ) );
}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Add a score for a passed pawns
int PawnMisc( struct Board *argsBoard, struct GeneralMove *argsGeneralMoves,
              int argiSquareIndex )
{
   // Note, all scores in this routine are positive and signs are assigned in the
   // call routine.

   // Allocate the score
   int iPassedScore = 0;

   // Switch on the color of the pawn.
   if ( argsBoard->mBoard[ argiSquareIndex ] == dWhitePawn )
   {

      /////////////////////////////////////////////
      // Look for a passed pawn
      if ( !( argsGeneralMoves->vbbWhitePPWideVector[ argiSquareIndex ] &
              argsBoard->bbBlackPawn ) )
      {

         // This score increases with increasing(decreasing) rank
         // Note that rows and cols are messed up.
         iPassedScore +=
             dPassedPawnIntercept +
             argsGeneralMoves->viCol[ argiSquareIndex ] * dPassedPawnSlope;

         // Look for another piece blocking the progress.
         if ( ( argsGeneralMoves->vbbWhitePPVector[ argiSquareIndex ] &
                argsBoard->bbBlackRook ) |
              ( argsGeneralMoves->vbbWhitePPVector[ argiSquareIndex ] &
                argsBoard->bbBlackKnight ) |
              ( argsGeneralMoves->vbbWhitePPVector[ argiSquareIndex ] &
                argsBoard->bbBlackBishop ) |
              ( argsGeneralMoves->vbbWhitePPVector[ argiSquareIndex ] &
                argsBoard->bbBlackQueen ) |
              ( argsGeneralMoves->vbbWhitePPVector[ argiSquareIndex ] &
                argsBoard->bbBlackKing ) )
         {

            iPassedScore -= iPassedScore / 2;
         }
      }

      /////////////////////////////////////////////
      // Look for an isolated pawn
      if ( argsGeneralMoves->viRow[ argiSquareIndex ] == 1 )
      {

         // Look for an isolated pawn on A
         if ( !( argsGeneralMoves->vbbRow[ argiSquareIndex + 1 ] &
                 argsBoard->bbWhitePawn ) )
         {

            iPassedScore -= dIsolatedPawn;
         }
      }
      else if ( argsGeneralMoves->viRow[ argiSquareIndex ] == 8 )
      {

         // Look for an isolated pawn on H
         if ( !( argsGeneralMoves->vbbRow[ argiSquareIndex - 1 ] &
                 argsBoard->bbWhitePawn ) )
         {

            iPassedScore -= dIsolatedPawn;
         }
      }
      else
      {

         // Look for an isolated pawn on in the middle.
         if ( ( !( argsGeneralMoves->vbbRow[ argiSquareIndex - 1 ] &
                   argsBoard->bbWhitePawn ) ) &&
              ( !( argsGeneralMoves->vbbRow[ argiSquareIndex + 1 ] &
                   argsBoard->bbWhitePawn ) ) )
         {

            iPassedScore -= dIsolatedPawn;
         }
      }

      /////////////////////////////////////////////
      // Look for a doubled or trippled pawn.
      // The minus one is because we know there is already one pawn on the row.
      BitBoard bbNumberOfPawns = Count( argsGeneralMoves->vbbRow[ argiSquareIndex ] &
                                        argsBoard->bbWhitePawn ) -
                                 1;

      // Some debugging
      assert( bbNumberOfPawns >= 0 );

      // Do the if to avoid the multiplication if possible.
      if ( bbNumberOfPawns > 0 )
      {

         // Update the score, not the minus sign as this is a penality
         iPassedScore -= (int)bbNumberOfPawns * dDoubleCol1;
      }
   }
   // Do a black pawn.
   else if ( argsBoard->mBoard[ argiSquareIndex ] == dBlackPawn )
   {

      /////////////////////////////////////////////
      // Look for a passed pawn
      if ( !( argsGeneralMoves->vbbBlackPPWideVector[ argiSquareIndex ] &
              argsBoard->bbWhitePawn ) )
      {

         // This score increases with increasing(decreasing) rank
         // Note that rows and cols are messed up but working
         // The sign is handled in the main routine.
         iPassedScore +=
             dPassedPawnIntercept +
             ( 9 - argsGeneralMoves->viCol[ argiSquareIndex ] ) * dPassedPawnSlope;

         // Look for another piece blocking the progress.
         if ( ( argsGeneralMoves->vbbBlackPPVector[ argiSquareIndex ] &
                argsBoard->bbWhiteRook ) |
              ( argsGeneralMoves->vbbBlackPPVector[ argiSquareIndex ] &
                argsBoard->bbWhiteKnight ) |
              ( argsGeneralMoves->vbbBlackPPVector[ argiSquareIndex ] &
                argsBoard->bbWhiteBishop ) |
              ( argsGeneralMoves->vbbBlackPPVector[ argiSquareIndex ] &
                argsBoard->bbWhiteQueen ) |
              ( argsGeneralMoves->vbbBlackPPVector[ argiSquareIndex ] &
                argsBoard->bbWhiteKing ) )
         {

            // The score is handled in the main routine.
            iPassedScore -= iPassedScore / 2;
         }
      }

      /////////////////////////////////////////////
      // Look for an isolated pawn
      if ( argsGeneralMoves->viRow[ argiSquareIndex ] == 1 )
      {

         // Look for an isolated pawn on A.
         if ( !( argsGeneralMoves->vbbRow[ argiSquareIndex + 1 ] &
                 argsBoard->bbBlackPawn ) )
         {

            iPassedScore -= dIsolatedPawn;
         }
      }
      else if ( argsGeneralMoves->viRow[ argiSquareIndex ] == 8 )
      {

         // Look for an isolated pawn on H.
         if ( !( argsGeneralMoves->vbbRow[ argiSquareIndex - 1 ] &
                 argsBoard->bbBlackPawn ) )
         {

            iPassedScore -= dIsolatedPawn;
         }
      }
      else
      {

         // Look for an isolated pawn on in the middle.
         if ( ( !( argsGeneralMoves->vbbRow[ argiSquareIndex - 1 ] &
                   argsBoard->bbBlackPawn ) ) &&
              ( !( argsGeneralMoves->vbbRow[ argiSquareIndex + 1 ] &
                   argsBoard->bbBlackPawn ) ) )
         {

            iPassedScore -= dIsolatedPawn;
         }
      }

      /////////////////////////////////////////////
      // Look for a doubled or trippled pawn.
      // The minus one is because we know there is already one pawn on the row.
      BitBoard bbNumberOfPawns = Count( argsGeneralMoves->vbbRow[ argiSquareIndex ] &
                                        argsBoard->bbBlackPawn ) -
                                 1;

      // Some debugging
      assert( bbNumberOfPawns >= 0 );

      // Do the if to avoid the multiplication if possible.
      if ( bbNumberOfPawns > 0 )
      {

         // Update the score, not the minus sign as this is a penality
         iPassedScore -= (int)bbNumberOfPawns * dDoubleCol1;
      }
   }
   else
   {

      // This point should never be reached.
      assert( 0 );
   }

   // Return the score
   return iPassedScore;
}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
int GetWhitePawnPlacementScore( int iSquare )
{
   return gsEvaluator.viPlacementPawnWhite[ iSquare ];
}
int GetBlackPawnPlacementScore( int iSquare )
{
   return gsEvaluator.viPlacementPawnWhite[ gsEvaluator.viReverse[ iSquare ] ];
}
int GetWhiteRookPlacementScore( int iSquare )
{
   return gsEvaluator.viPlacementRook[ iSquare ];
}
int GetBlackRookPlacementScore( int iSquare )
{
   return gsEvaluator.viPlacementRook[ gsEvaluator.viReverse[ iSquare ] ];
}
int GetWhiteKnightPlacementScore( int iSquare )
{
   return gsEvaluator.viPlacementKnight[ iSquare ];
}
int GetBlackKnightPlacementScore( int iSquare )
{
   return gsEvaluator.viPlacementKnight[ gsEvaluator.viReverse[ iSquare ] ];
}
int GetWhiteBishopPlacementScore( int iSquare )
{
   return gsEvaluator.viPlacementBishop[ iSquare ];
}
int GetBlackBishopPlacementScore( int iSquare )
{
   return gsEvaluator.viPlacementBishop[ gsEvaluator.viReverse[ iSquare ] ];
}
int GetWhiteQueenPlacementScore( int iSquare )
{
   return gsEvaluator.viPlacementQueen[ iSquare ];
}
int GetBlackQueenPlacementScore( int iSquare )
{
   return gsEvaluator.viPlacementQueen[ gsEvaluator.viReverse[ iSquare ] ];
}
int GetWhiteKingPlacementScore( int iSquare )
{
   return gsEvaluator.viPlacementKingWhite[ iSquare ];
}
int GetBlackKingPlacementScore( int iSquare )
{
   return gsEvaluator.viPlacementKingBlack[ iSquare ];
}
int GetUseMobility()
{
   return gsEvaluator.iUseMobility;
}

//
////////////////////////////////////////////////////////////////////////////////////////////////////
//
void SetUseMobility( int iMobility )
{
   gsEvaluator.iUseMobility = iMobility;
}

//
//
//---------------------------------------------------------------------
//
//
// Evaluate the board for a potential null search.
//
// Note that the default is to score the position from the white point of view
// and then flip the sign.
//
int EvaluateNull( struct Board       *argsBoard,
                  struct GeneralMove *argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Allocate the initialize the score.
   int iScore = 0;

   // Add the pieces as appropriate
   iScore += (int)Count( argsBoard->bbWhitePawn ) * dValuePawnOpening;
   iScore += (int)Count( argsBoard->bbWhiteRook ) * dValueRookOpening;
   iScore += (int)Count( argsBoard->bbWhiteKnight ) * dValueKnightOpening;
   iScore += (int)Count( argsBoard->bbWhiteBishop ) * dValueBishopOpening;
   iScore += (int)Count( argsBoard->bbWhiteQueen ) * dValueQueenOpening;

   iScore -= (int)Count( argsBoard->bbBlackPawn ) * dValuePawnOpening;
   iScore -= (int)Count( argsBoard->bbBlackRook ) * dValueRookOpening;
   iScore -= (int)Count( argsBoard->bbBlackKnight ) * dValueKnightOpening;
   iScore -= (int)Count( argsBoard->bbBlackBishop ) * dValueBishopOpening;
   iScore -= (int)Count( argsBoard->bbBlackQueen ) * dValueQueenOpening;

   // Return the standard score.
   if ( argsBoard->siColorToMove == dBlack )
   {

      iScore = -iScore;
   }

   return iScore;
}

// Direct NNUE evaluation without FEN conversion
// This function converts Board structure directly to NNUE format
int EvaluateBoardDirectNNUE( struct Board       *argsBoard,
                             struct GeneralMove *argsGeneralMoves )
{

   // Prepare the arrays for NNUE
   // pieces[0] = white king, pieces[1] = black king, then all other pieces
   // The arrays are terminated with 0
   int pieces[ 33 ];  // Max 32 pieces + terminator
   int squares[ 33 ]; // Corresponding squares
   int index = 2;     // Start at index 2 (0=white king, 1=black king)

   // Initialize arrays
   pieces[ 0 ]  = 0;
   pieces[ 1 ]  = 0;
   squares[ 0 ] = 0;
   squares[ 1 ] = 0;

   // Map Violet piece codes to NNUE piece codes
   // Violet: dWhitePawn=1, dWhiteRook=2, dWhiteKnight=3, dWhiteBishop=4,
   // dWhiteQueen=5, dWhiteKing=6
   //         dBlackPawn=7, dBlackRook=8, dBlackKnight=9, dBlackBishop=10,
   //         dBlackQueen=11, dBlackKing=12
   // NNUE:   wking=1, wqueen=2, wrook=3, wbishop=4, wknight=5, wpawn=6,
   //         bking=7, bqueen=8, brook=9, bbishop=10, bknight=11, bpawn=12
   const int violetToNNUE[ 13 ] = {
       0,  // dEmpty -> 0
       6,  // dWhitePawn -> wpawn
       3,  // dWhiteRook -> wrook
       5,  // dWhiteKnight -> wknight
       4,  // dWhiteBishop -> wbishop
       2,  // dWhiteQueen -> wqueen
       1,  // dWhiteKing -> wking
       12, // dBlackPawn -> bpawn
       9,  // dBlackRook -> brook
       11, // dBlackKnight -> bknight
       10, // dBlackBishop -> bbishop
       8,  // dBlackQueen -> bqueen
       7   // dBlackKing -> bking
   };

   // Scan the board and populate the arrays
   for ( int sq = 0; sq < 64; sq++ )
   {
      int violetPiece = argsBoard->mBoard[ sq ];

      if ( violetPiece != dEmpty )
      {
         int nnuePiece = violetToNNUE[ violetPiece ];

         // Kings go in positions 0 and 1
         if ( nnuePiece == 1 ) // White king
         {
            pieces[ 0 ]  = nnuePiece;
            squares[ 0 ] = sq;
         }
         else if ( nnuePiece == 7 ) // Black king
         {
            pieces[ 1 ]  = nnuePiece;
            squares[ 1 ] = sq;
         }
         else // All other pieces
         {
            pieces[ index ]  = nnuePiece;
            squares[ index ] = sq;
            index++;
         }
      }
   }

   // Terminate the arrays
   pieces[ index ]  = 0;
   squares[ index ] = 0;

   // Determine player to move (0=white, 1=black)
   int player = ( argsBoard->siColorToMove == dWhite ) ? 0 : 1;

   // Prepare NNUE data pointers
   NNUEdata *nnue_data[ 3 ];
   int       currentHistoryIndex = argsBoard->iMoveHistory;

   nnue_data[ 0 ] = &argsBoard->sNNUEHistory[ currentHistoryIndex ];

   if ( currentHistoryIndex > 0 )
   {
      nnue_data[ 1 ] = &argsBoard->sNNUEHistory[ currentHistoryIndex - 1 ];
   }
   else
   {
      nnue_data[ 1 ] = NULL;
   }

   if ( currentHistoryIndex > 1 )
   {
      nnue_data[ 2 ] = &argsBoard->sNNUEHistory[ currentHistoryIndex - 2 ];
   }
   else
   {
      nnue_data[ 2 ] = NULL;
   }

   // Call NNUE evaluation incremental
   int iScore = nnue_evaluate_incremental( player, pieces, squares, nnue_data );

   return iScore;
}

void PrefetchNNUE( struct Board *argsBoard )
{
   NNUEdata *nnue_data[ 3 ];
   int       currentHistoryIndex = argsBoard->iMoveHistory;

   nnue_data[ 0 ] = &argsBoard->sNNUEHistory[ currentHistoryIndex ];

   if ( currentHistoryIndex > 0 )
   {
      nnue_data[ 1 ] = &argsBoard->sNNUEHistory[ currentHistoryIndex - 1 ];
   }
   else
   {
      nnue_data[ 1 ] = NULL;
   }

   if ( currentHistoryIndex > 1 )
   {
      nnue_data[ 2 ] = &argsBoard->sNNUEHistory[ currentHistoryIndex - 2 ];
   }
   else
   {
      nnue_data[ 2 ] = NULL;
   }

   // Extract King squares
   int kingSquares[ 2 ];

   // Find White King
   if ( argsBoard->bbWhiteKing )
   {
      kingSquares[ 0 ] = __builtin_ctzll( argsBoard->bbWhiteKing );
   }
   else
   {
      kingSquares[ 0 ] = 0;
   }

   // Find Black King
   if ( argsBoard->bbBlackKing )
   {
      kingSquares[ 1 ] = __builtin_ctzll( argsBoard->bbBlackKing );
   }
   else
   {
      kingSquares[ 1 ] = 0;
   }

   nnue_prefetch( nnue_data, kingSquares );
}