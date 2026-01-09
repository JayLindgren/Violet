#include "SEE.h"
#include "Definitions.h"
#include "Functions.h"
#include "MoveGen.h" // For PEXT-based attack generation
#include <algorithm>
#include <iostream>

using namespace std;

// Piece values for SEE (using standard values or slightly modified for SEE)
// dEmpty, dWhitePawn, dWhiteRook, dWhiteKnight, dWhiteBishop, dWhiteQueen, dWhiteKing
// dBlackPawn, dBlackRook, dBlackKnight, dBlackBishop, dBlackQueen, dBlackKing
static const int vSeePieceValues[ 13 ] =
    {
        0,     // Empty
        100,   // White Pawn
        500,   // White Rook
        325,   // White Knight
        325,   // White Bishop
        900,   // White Queen
        20000, // White King (should not be captured, but high value for logic)
        100,   // Black Pawn
        500,   // Black Rook
        325,   // Black Knight
        325,   // Black Bishop
        900,   // Black Queen
        20000  // Black King
};

//
//
//---------------------------------------------------------------------
//
//
int GetPieceValue( int iPiece )
{
   if ( iPiece >= 0 && iPiece <= 12 )
   {

      return vSeePieceValues[ iPiece ];
   }

   return 0;
}

//
//
//---------------------------------------------------------------------
//
//
// Get the smallest attacker for 'side' attacking 'square'
// Returns the piece type of the attacker, or 0 if no attacker found.
// Updates attackerSq with the square of the attacker.
int GetSmallestAttacker( struct Board       *argsBoard,
                         struct GeneralMove *argsGeneralMoves,
                         int                 iSquare,
                         int                 siSide,
                         BitBoard            bbOccupied,
                         int                &iAttackerSq,
                         int                &iAttackerPiece )
{

   // 1. Pawns
   if ( siSide == dWhite )
   {

      // White Pawn Candidates
      // Candidate 1: square - 9 (South-West)
      // Valid if square col > 0 (not A file) and square row > 0
      if ( dCol( iSquare ) > 0 && dRow( iSquare ) > 0 )
      {
         // Row 0 is rank 1.
         int iCand = iSquare - 9;
         if ( iCand >= 0 && ( bbOccupied & argsBoard->bbWhitePawn & ( 1ULL << iCand ) ) )
         {

            iAttackerSq    = iCand;
            iAttackerPiece = dWhitePawn;
            return dWhitePawn;
         }
      }

      // Candidate 2: square - 7 (South-East)
      // Valid if square col < 7 (not H file) and square row > 0
      if ( dCol( iSquare ) < 7 && dRow( iSquare ) > 0 )
      {

         int iCand = iSquare - 7;
         if ( iCand >= 0 && ( bbOccupied & argsBoard->bbWhitePawn & ( 1ULL << iCand ) ) )
         {

            iAttackerSq    = iCand;
            iAttackerPiece = dWhitePawn;
            return dWhitePawn;
         }
      }
   }
   else
   {

      // Black Pawns
      // Attack from North-West (sq+7) and North-East (sq+9)
      // Candidate 1: square + 9 (North-East)
      // Valid if square col < 7 and square row < 7
      if ( dCol( iSquare ) < 7 && dRow( iSquare ) < 7 )
      {

         int iCand = iSquare + 9;
         if ( iCand < 64 && ( bbOccupied & argsBoard->bbBlackPawn & ( 1ULL << iCand ) ) )
         {

            iAttackerSq    = iCand;
            iAttackerPiece = dBlackPawn;
            return dBlackPawn;
         }
      }

      // Candidate 2: square + 7 (North-West)
      // Valid if square col > 0 and square row < 7
      if ( dCol( iSquare ) > 0 && dRow( iSquare ) < 7 )
      {

         int iCand = iSquare + 7;
         if ( iCand < 64 && ( bbOccupied & argsBoard->bbBlackPawn & ( 1ULL << iCand ) ) )
         {

            iAttackerSq    = iCand;
            iAttackerPiece = dBlackPawn;
            return dBlackPawn;
         }
      }
   }

   // 2. Knights
   BitBoard bbKnights = ( siSide == dWhite ) ? argsBoard->bbWhiteKnight : argsBoard->bbBlackKnight;
   bbKnights &= bbOccupied; // Only consider pieces currently on board (in case of X-ray logic, though knights don't X-ray)

   if ( bbKnights )
   {

      // We can use the pre-calculated knight moves for the 'square'.
      // If a knight is at 'square', it attacks X, Y, Z.
      // If a knight is at X, it attacks 'square'.
      // So we can check generalMoves->bbNMove[square] & knights.
      BitBoard bbAttackers = argsGeneralMoves->bbNMove[ iSquare ] & bbKnights;
      if ( bbAttackers )
      {

         // Find the first bit
         for ( int i = 0; i < 64; ++i )
         {

            if ( ( bbAttackers >> i ) & 1 )
            {

               iAttackerSq    = i;
               iAttackerPiece = ( siSide == dWhite ) ? dWhiteKnight : dBlackKnight;
               return iAttackerPiece;
            }
         }
      }
   }

   // 3. Bishops (and Queens diagonal)
   BitBoard bbBishops = ( siSide == dWhite ) ? ( argsBoard->bbWhiteBishop | argsBoard->bbWhiteQueen ) : ( argsBoard->bbBlackBishop | argsBoard->bbBlackQueen );
   bbBishops &= bbOccupied;

   // We need to check if we found a queen, but we should prefer a bishop if available.
   int iFoundQueenSq    = -1;
   int iFoundQueenPiece = 0;

   if ( bbBishops )
   {
      // Use PEXT to get all diagonal attackers in one operation
      BitBoard bbDiagonalAttacks   = GetBishopAttacksPEXT( iSquare, bbOccupied );
      BitBoard bbDiagonalAttackers = bbDiagonalAttacks & bbBishops;

      if ( bbDiagonalAttackers )
      {
         // Find closest attacker (prefer bishops over queens)
         int iClosestBishop = -1;
         int iClosestQueen  = -1;
         int iMinDistBishop = 999;
         int iMinDistQueen  = 999;

         while ( bbDiagonalAttackers )
         {
            unsigned long index;
            _BitScanForward64( &index, bbDiagonalAttackers );
            int iSq = (int)index;
            bbDiagonalAttackers &= bbDiagonalAttackers - 1; // Clear LSB

            int iPiece = argsBoard->mBoard[ iSq ];
            int iDist  = abs( dRow( iSq ) - dRow( iSquare ) ) + abs( dCol( iSq ) - dCol( iSquare ) );

            if ( iPiece == dWhiteBishop || iPiece == dBlackBishop )
            {
               if ( iDist < iMinDistBishop )
               {
                  iMinDistBishop = iDist;
                  iClosestBishop = iSq;
               }
            }
            else // Queen
            {
               if ( iDist < iMinDistQueen )
               {
                  iMinDistQueen = iDist;
                  iClosestQueen = iSq;
               }
            }
         }

         // Prefer bishop over queen
         if ( iClosestBishop != -1 )
         {
            iAttackerSq    = iClosestBishop;
            iAttackerPiece = argsBoard->mBoard[ iClosestBishop ];
            return iAttackerPiece;
         }
         else if ( iClosestQueen != -1 )
         {
            iFoundQueenSq    = iClosestQueen;
            iFoundQueenPiece = argsBoard->mBoard[ iClosestQueen ];
         }
      }
   }

   // 4. Rooks (and Queens orthogonal)
   BitBoard bbRooks = ( siSide == dWhite ) ? ( argsBoard->bbWhiteRook | argsBoard->bbWhiteQueen ) : ( argsBoard->bbBlackRook | argsBoard->bbBlackQueen );
   bbRooks &= bbOccupied;

   if ( bbRooks )
   {
      // Use PEXT to get all orthogonal attackers in one operation
      BitBoard bbOrthogonalAttacks   = GetRookAttacksPEXT( iSquare, bbOccupied );
      BitBoard bbOrthogonalAttackers = bbOrthogonalAttacks & bbRooks;

      if ( bbOrthogonalAttackers )
      {
         // Find closest attacker (prefer rooks over queens)
         int iClosestRook  = -1;
         int iClosestQueen = -1;
         int iMinDistRook  = 999;
         int iMinDistQueen = 999;

         while ( bbOrthogonalAttackers )
         {
            unsigned long index;
            _BitScanForward64( &index, bbOrthogonalAttackers );
            int iSq = (int)index;
            bbOrthogonalAttackers &= bbOrthogonalAttackers - 1; // Clear LSB

            int iPiece = argsBoard->mBoard[ iSq ];
            int iDist  = abs( dRow( iSq ) - dRow( iSquare ) ) + abs( dCol( iSq ) - dCol( iSquare ) );

            if ( iPiece == dWhiteRook || iPiece == dBlackRook )
            {
               if ( iDist < iMinDistRook )
               {
                  iMinDistRook = iDist;
                  iClosestRook = iSq;
               }
            }
            else // Queen
            {
               if ( iDist < iMinDistQueen )
               {
                  iMinDistQueen = iDist;
                  iClosestQueen = iSq;
               }
            }
         }

         // Prefer rook over queen
         if ( iClosestRook != -1 )
         {
            iAttackerSq    = iClosestRook;
            iAttackerPiece = argsBoard->mBoard[ iClosestRook ];
            return iAttackerPiece;
         }
         else if ( iClosestQueen != -1 )
         {
            // Store queen info but continue searching
            if ( iFoundQueenSq == -1 || iMinDistQueen < abs( dRow( iFoundQueenSq ) - dRow( iSquare ) ) + abs( dCol( iFoundQueenSq ) - dCol( iSquare ) ) )
            {
               iFoundQueenSq    = iClosestQueen;
               iFoundQueenPiece = argsBoard->mBoard[ iClosestQueen ];
            }
         }
      }
   }

   // If we found a Queen but no minor pieces (Bishop/Rook), return the Queen.
   if ( iFoundQueenSq != -1 )
   {
      iAttackerSq    = iFoundQueenSq;
      iAttackerPiece = iFoundQueenPiece;
      return iFoundQueenPiece;
   }

   // 5. King
   BitBoard bbKing = ( siSide == dWhite ) ? argsBoard->bbWhiteKing : argsBoard->bbBlackKing;
   bbKing &= bbOccupied;

   if ( bbKing )
   {
      BitBoard bbAttackers = argsGeneralMoves->bbKMove[ iSquare ] & bbKing;
      if ( bbAttackers )
      {
         for ( int i = 0; i < 64; ++i )
         {
            if ( ( bbAttackers >> i ) & 1 )
            {
               iAttackerSq    = i;
               iAttackerPiece = ( siSide == dWhite ) ? dWhiteKing : dBlackKing;
               return iAttackerPiece;
            }
         }
      }
   }

   return 0; // No attacker found
}

//
//
//---------------------------------------------------------------------
//
//
int SeeCapture( struct Board       *argsBoard,
                struct GeneralMove *argsGeneralMoves,
                int                 iFrom,
                int                 iTo,
                int                 siSide,
                int                 iCapturedPieceType )
{
   int iValue = 0;

   // Virtual occupied board
   BitBoard bbOccupied = argsBoard->bbWhitePieces | argsBoard->bbBlackPieces;

   // Initial capture
   int iPiece    = argsBoard->mBoard[ iFrom ]; // The piece making the capture
   int iCaptured = iCapturedPieceType;

   // Make the first capture on the bitboard (virtual)
   bbOccupied &= ~( (BitBoard)1 << iFrom ); // Remove attacker from origin
   bbOccupied |= ( (BitBoard)1 << iTo );    // Place attacker on target (overwriting victim)

   // The value of the first capture is the value of the captured piece
   int vGain[ 32 ]; // Stack of gains
   int iD      = 0;
   vGain[ iD ] = GetPieceValue( iCaptured );

   int iAttackerSq    = iFrom;
   int iAttackerPiece = iPiece;
   int siCurrentSide  = ( siSide == dWhite ) ? dBlack : dWhite; // Next side to move

   // Iterative SEE
   while ( true )
   {

      iD++;

      // Find the smallest attacker for the current side
      int iNextAttackerSq    = -1;
      int iNextAttackerPiece = 0;
      int iAttackerType      = GetSmallestAttacker( argsBoard,
                                                    argsGeneralMoves,
                                                    iTo,
                                                    siCurrentSide,
                                                    bbOccupied,
                                                    iNextAttackerSq,
                                                    iNextAttackerPiece );

      if ( iAttackerType == 0 )
      {

         break; // No more attackers
      }

      // Add the value of the piece that was just on the square (the previous attacker)
      // If we capture back, we gain the value of the piece that just captured.
      vGain[ iD ] = GetPieceValue( iAttackerPiece ) - vGain[ iD - 1 ];

      if ( max( -vGain[ iD - 1 ], vGain[ iD ] ) < 0 )
      {

         // Pruning optimization could go here
      }

      // Update occupied bitboard
      bbOccupied &= ~( (BitBoard)1 << iNextAttackerSq ); // Remove new attacker from origin
      bbOccupied |= ( (BitBoard)1 << iTo );              // Place new attacker on target

      // Update state
      iAttackerSq    = iNextAttackerSq;
      iAttackerPiece = iNextAttackerPiece;
      siCurrentSide  = ( siCurrentSide == dWhite ) ? dBlack : dWhite;
   }

   // Propagate scores back (Negamax)
   while ( --iD > 0 )
   {

      vGain[ iD - 1 ] = -max( -vGain[ iD - 1 ], vGain[ iD ] );
   }

   return vGain[ 0 ];
}

//
//
//---------------------------------------------------------------------
//
//
int See( struct Board       *argsBoard,
         struct GeneralMove *argsGeneralMoves,
         struct Move        *argsMove )
{
   // Only evaluate captures or promotions for now
   if ( argsMove->iMoveType == dRegular ||
        argsMove->iMoveType == dTwoSquare ||
        argsMove->iMoveType == dCastle ||
        argsMove->iMoveType == dWhiteKingSideCastle ||
        argsMove->iMoveType == dWhiteQueenSideCastle ||
        argsMove->iMoveType == dBlackKingSideCastle ||
        argsMove->iMoveType == dBlackQueenSideCastle )
   {

      return 0;
   }

   int iCapturedPiece = 0;
   if ( argsMove->iMoveType == dEnPassant )
   {

      iCapturedPiece = dWhitePawn; // Value is same for black/white pawn
   }
   else if ( argsMove->iCapture != dEmpty )
   {

      iCapturedPiece = argsMove->iCapture;
   }
   else
   {

      // Fallback: Look at the board
      if ( argsMove->iMoveType == dCapture || argsMove->iMoveType == dCaptureAndPromote )
      {

         iCapturedPiece = argsBoard->mBoard[ argsMove->iToSquare ];
      }

      if ( iCapturedPiece == dEmpty )
      {

         // Promotion without capture?
         if ( argsMove->iMoveType == dPromote )
         {

            // Value is value of promoted piece - value of pawn
            return GetPieceValue( argsMove->iPiece ) - GetPieceValue( dWhitePawn );
         }

         return 0;
      }
   }

   return SeeCapture( argsBoard,
                      argsGeneralMoves,
                      argsMove->iFromSquare,
                      argsMove->iToSquare,
                      argsBoard->siColorToMove,
                      iCapturedPiece );
}
