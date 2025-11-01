// Copyright 2006 by Jay Lindgren. All Rights Reserved.
//

   //#define _CRTDBG_MAP_ALLOC
   #include <stdlib.h>
   //#include <crtdbg.h>


#include <iostream>
#include <fstream>
#include "Definitions.h"
#include "Structures.h"
#include "Functions.h"

using namespace std;

void GenerateGeneralMove( struct GeneralMove * sGeneralMove )
{
// This function pre calculates all possible piece moves.
//
//
// Some Board notes.
// Cols ---->
// R
// o a1 a2 a3 a4 a5 a6 a7 a8
// w b1 b2 b3 b4 b5 b6 b7 b8
// s c1 c2 c3 c4 c5 c6 c7 c8
// | d1 d2 d3 d4 d5 d6 d7 d8
// | e1 e2 e3 e4 e5 e6 e7 e8
// V f1 f2 f3 f4 f5 f6 f7 f8
//   g1 g2 g3 g4 g5 g6 g7 g8
//   h1 h2 3h h4 h5 h6 h7 h8
//
// North is increasing cols
// South is decreasing cols
// East is increasing rows
// West is decreasing rows
//

   // Debug inputs
   assert( sGeneralMove >= 0 );

  // Declarations.
  int rowIndex;
  int colIndex;
  int iRow = 0;
  int iCol = 1;

   // Calcualte the Selectors.
   CalculateSelectors( sGeneralMove );

   // The Pawns are done all at once.
   CalculatePawns( sGeneralMove );

   // Do the pieces.
   for ( rowIndex = 0; rowIndex < 8; rowIndex++ )
   {
       
      for ( colIndex = 0; colIndex < 8; colIndex++ )
      {

         // Calculate the square.
         int square = rowIndex + ( colIndex ) * 8;

         // Set all the bit boards to zero.
         sGeneralMove->bbKingBubble[ square ] = 0;

         sGeneralMove->bbRMove[ square ] = 0;
         sGeneralMove->bbRMoveN[ square ] = 0;
         sGeneralMove->bbRMoveW[ square ] = 0;
         sGeneralMove->bbRMoveE[ square ] = 0;
         sGeneralMove->bbRMoveS[ square ] = 0;
         sGeneralMove->bbQMove[ square ] = 0;
         sGeneralMove->bbQMoveNE[ square ] = 0;
         sGeneralMove->bbQMoveNW[ square ] = 0;
         sGeneralMove->bbQMoveSE[ square ] = 0;
         sGeneralMove->bbQMoveSW[ square ] = 0;
         sGeneralMove->bbQMoveN[ square ] = 0;
         sGeneralMove->bbQMoveW[ square ] = 0;
         sGeneralMove->bbQMoveS[ square ] = 0;
         sGeneralMove->bbQMoveE[ square ] = 0;
         sGeneralMove->bbBMove[ square ] = 0;
         sGeneralMove->bbBMoveNE[ square ] = 0;
         sGeneralMove->bbBMoveNW[ square ] = 0;
         sGeneralMove->bbBMoveSE[ square ] = 0;
         sGeneralMove->bbBMoveSW[ square ] = 0;

         // Set up the pieces.
         CalculateRooks(   sGeneralMove, rowIndex, colIndex, square );
         CalculateKnights( sGeneralMove, rowIndex, colIndex, square );
         CalculateBishops( sGeneralMove, rowIndex, colIndex, square );
         CalculateQueen(   sGeneralMove, rowIndex, colIndex, square );
         CalculateKing(    sGeneralMove, rowIndex, colIndex, square );
         
      }
      
   }

   // Set the promotion piece vector.
   sGeneralMove->vPromotionPieces[ 0 ] = dWhiteRook;
   sGeneralMove->vPromotionPieces[ 1 ] = dWhiteKnight;
   sGeneralMove->vPromotionPieces[ 2 ] = dWhiteBishop;
   sGeneralMove->vPromotionPieces[ 3 ] = dWhiteQueen;

   sGeneralMove->bbCol1 = 0x00000000000000FF;
   sGeneralMove->bbCol2 = 0x000000000000FF00;
   sGeneralMove->bbCol3 = 0x0000000000FF0000;
   sGeneralMove->bbCol4 = 0x00000000FF000000;
   sGeneralMove->bbCol5 = 0x000000FF00000000;
   sGeneralMove->bbCol6 = 0x0000FF0000000000;
   sGeneralMove->bbCol7 = 0x00FF000000000000;
   sGeneralMove->bbCol8 = 0xFF00000000000000;
   sGeneralMove->bbRow8 = 0x8080808080808080;
   sGeneralMove->bbRow7 = 0x4040404040404040;
   sGeneralMove->bbRow6 = 0x2020202020202020;
   sGeneralMove->bbRow5 = 0x1010101010101010;
   sGeneralMove->bbRow4 = 0x0808080808080808;
   sGeneralMove->bbRow3 = 0x0404040404040404;
   sGeneralMove->bbRow2 = 0x0202020202020202;
   sGeneralMove->bbRow1 = 0x0101010101010101;
   sGeneralMove->bbUpperHalf     = 0xFFFFFFFF00000000;
   sGeneralMove->bbLowerHalf     = 0x00000000FFFFFFFF;
   sGeneralMove->bbFourthQuarter = 0xFFFF000000000000;
   sGeneralMove->bbThirdQuarter  = 0x0000FFFF00000000;
   sGeneralMove->bbSecondQuarter = 0x00000000FFFF0000;
   sGeneralMove->bbFirstQuarter  = 0x000000000000FFFF;
   sGeneralMove->bbLeftHalf      = 0x0F0F0F0F0F0F0F0F;
   sGeneralMove->bbRightHalf     = 0xF0F0F0F0F0F0F0F0;
   sGeneralMove->bbNearLeft      = 0x0303030303030303;
   sGeneralMove->bbNearRight     = 0x3030303030303030;
   sGeneralMove->bbFarLeft       = 0x0C0C0C0C0C0C0C0C;
   sGeneralMove->bbFarRight      = 0xC0C0C0C0C0C0C0C0;
  
   // Define the masks.
   sGeneralMove->bbScore       = 0x000000000001FFFF; // First 17 bits, cum 17.
   sGeneralMove->bbScoreSign   = 0x0000000000020000; // The 18th bit contains the sign of the score. cum 18
   sGeneralMove->bbDepth       = 0x00000001FFFC0000; // Next  15 bits, cum 32.
   sGeneralMove->bbBestMove    = 0x001FFFFE00000000; // Next  21 bits, cum 53. 6 - start square, 6 end square, 4 promotion, RNBQ
   sGeneralMove->bbDanger      = 0x0020000000000000; // Next   1 bits, cum 54.
   sGeneralMove->bbTypeOfScore = 0x00C0000000000000; // Next   2 bits, cum 56.
   sGeneralMove->bbAge         = 0x0700000000000000; // Next   3 bits, cum 59.

   // Define the opening book masks. - the number of games should not exceed 1048576
   sGeneralMove->bbWhiteScore = 0x00000000000FFFFF; // The first 20 bits are for the white score
   sGeneralMove->bbBlackScore = 0x000000FFFFF00000; // The next 20 are the black score
   sGeneralMove->bbDrawScore  = 0x0FFFFF0000000000; // The next 20 are the draws.
   sGeneralMove->bbVerified   = 0x1000000000000000; // The next 20 are the draws.

   // Define the shifts.
   sGeneralMove->iScoreShift       = 0;
   sGeneralMove->iSignShift        = 17;
   sGeneralMove->iDepthShift       = 18;
   sGeneralMove->iBestMoveShift    = 33;
   sGeneralMove->iDangerShift      = 53;
   sGeneralMove->iTypeOfScoreShift = 54;
   sGeneralMove->iAgeShift         = 56;

   // Define the shifts for the opening book.
   sGeneralMove->iWhiteScoreShift   = 0;
   sGeneralMove->iBlackScoreShift   = dNumberOfBitsPerScore;
   sGeneralMove->iDrawScoreShift    = dNumberOfBitsPerScore + dNumberOfBitsPerScore;
   sGeneralMove->iOpenBookMoveShift = 63;

   // Set the masks for forward looking passed pawns.
   sGeneralMove->vbbWhitePPBoardMask[ 0 ] = 0;
   sGeneralMove->vbbWhitePPBoardMask[ 1 ] = 0xFFFFFFFFFFFF0000;
   sGeneralMove->vbbWhitePPBoardMask[ 2 ] = 0xFFFFFFFFFF000000;
   sGeneralMove->vbbWhitePPBoardMask[ 3 ] = 0xFFFFFFFF00000000;
   sGeneralMove->vbbWhitePPBoardMask[ 4 ] = 0xFFFFFF0000000000;
   sGeneralMove->vbbWhitePPBoardMask[ 5 ] = 0xFFFF000000000000;
   sGeneralMove->vbbWhitePPBoardMask[ 6 ] = 0xFF00000000000000;
   sGeneralMove->vbbWhitePPBoardMask[ 7 ] = 0;
   sGeneralMove->vbbBlackPPBoardMask[ 0 ] = 0;
   sGeneralMove->vbbBlackPPBoardMask[ 1 ] = 0x00000000000000FF;
   sGeneralMove->vbbBlackPPBoardMask[ 2 ] = 0x000000000000FFFF;
   sGeneralMove->vbbBlackPPBoardMask[ 3 ] = 0x0000000000FFFFFF;
   sGeneralMove->vbbBlackPPBoardMask[ 4 ] = 0x00000000FFFFFFFF;
   sGeneralMove->vbbBlackPPBoardMask[ 5 ] = 0x000000FFFFFFFFFF;
   sGeneralMove->vbbBlackPPBoardMask[ 6 ] = 0x0000FFFFFFFFFFFF;
   sGeneralMove->vbbBlackPPBoardMask[ 7 ] = 0x00FFFFFFFFFFFFFF;
   
   // Calculate the rows and cols for a given square.
   for( int iSquare = 0; iSquare < 64; iSquare++ )
   {

      // Calculate the row and cols
      iRow++;
      if ( iRow > 8 )
      {

         iRow = 1;
         iCol++;

      }

      // Set the rows and cols
      sGeneralMove->viCol[ iSquare ] = iCol;
      sGeneralMove->viRow[ iSquare ] = iRow;

      // Switch on the rows and columns to create the bit boards.
      switch ( sGeneralMove->viCol[ iSquare ] )
      {
         case 1 :
         {
            sGeneralMove->vbbCol[ iSquare ] = sGeneralMove->bbCol1;
            break;
         }
         case 2 :
         {
            sGeneralMove->vbbCol[ iSquare ] = sGeneralMove->bbCol2;
            break;
         }
         case 3 :
         {
            sGeneralMove->vbbCol[ iSquare ] = sGeneralMove->bbCol3;
            break;
         }
         case 4 :
         {
            sGeneralMove->vbbCol[ iSquare ] = sGeneralMove->bbCol4;
            break;
         }
         case 5 :
         {
            sGeneralMove->vbbCol[ iSquare ] = sGeneralMove->bbCol5;
            break;
         }
         case 6 :
         {
            sGeneralMove->vbbCol[ iSquare ] = sGeneralMove->bbCol6;
            break;
         }
         case 7 :
         {
            sGeneralMove->vbbCol[ iSquare ] = sGeneralMove->bbCol7;
            break;
         }
         case 8 :
         {
            sGeneralMove->vbbCol[ iSquare ] = sGeneralMove->bbCol8;
            break;
         }
      }

      // Switch on the rows and Rowumns to create the bit boards.
      switch ( sGeneralMove->viRow[ iSquare ] )
      {
         case 1 :
         {
            sGeneralMove->vbbRow[ iSquare ] = sGeneralMove->bbRow1;
            break;
         }
         case 2 :
         {
            sGeneralMove->vbbRow[ iSquare ] = sGeneralMove->bbRow2;
            break;
         }
         case 3 :
         {
            sGeneralMove->vbbRow[ iSquare ] = sGeneralMove->bbRow3;
            break;
         }
         case 4 :
         {
            sGeneralMove->vbbRow[ iSquare ] = sGeneralMove->bbRow4;
            break;
         }
         case 5 :
         {
            sGeneralMove->vbbRow[ iSquare ] = sGeneralMove->bbRow5;
            break;
         }
         case 6 :
         {
            sGeneralMove->vbbRow[ iSquare ] = sGeneralMove->bbRow6;
            break;
         }
         case 7 :
         {
            sGeneralMove->vbbRow[ iSquare ] = sGeneralMove->bbRow7;
            break;
         }
         case 8 :
         {
            sGeneralMove->vbbRow[ iSquare ] = sGeneralMove->bbRow8;
            break;
         }

      }


   }

   // Set up the passed pawn vectors.
   for( int iSquare = 0; iSquare < 64; iSquare++ )
   {

      // Extract the rows and cols.
      iCol = sGeneralMove->viCol[ iSquare ];
      iRow = sGeneralMove->viRow[ iSquare ];

      // Calculate the pass pawn vector.
      // Note that cols and rows are messed up!!!!!
      sGeneralMove->vbbWhitePPVector[ iSquare ] = sGeneralMove->vbbRow[ iSquare ] & sGeneralMove->vbbWhitePPBoardMask[ iCol - 1 ];
      sGeneralMove->vbbBlackPPVector[ iSquare ] = sGeneralMove->vbbRow[ iSquare ] & sGeneralMove->vbbBlackPPBoardMask[ iCol - 1 ];

      // Calculate the wide vectors
      // Set the lower and higher col.
      if ( iRow == 1 )
      {

         sGeneralMove->vbbWhitePPWideVector[ iSquare ] = sGeneralMove->vbbWhitePPVector[ iSquare ] |
                        ( sGeneralMove->vbbRow[ iSquare + 1 ] & sGeneralMove->vbbWhitePPBoardMask[ iCol - 1 ] );;
         sGeneralMove->vbbBlackPPWideVector[ iSquare ] = sGeneralMove->vbbBlackPPVector[ iSquare ] |
                                                         ( sGeneralMove->vbbRow[ iSquare + 1 ] & sGeneralMove->vbbBlackPPBoardMask[ iCol - 1 ] );


      }
      else if ( iRow == 8 )
      {

         sGeneralMove->vbbWhitePPWideVector[ iSquare ] = sGeneralMove->vbbWhitePPVector[ iSquare ] | 
                                                         ( sGeneralMove->vbbRow[ iSquare - 1 ] & sGeneralMove->vbbWhitePPBoardMask[ iCol - 1 ] );
         sGeneralMove->vbbBlackPPWideVector[ iSquare ] = sGeneralMove->vbbBlackPPVector[ iSquare ] |
                                                         ( sGeneralMove->vbbRow[ iSquare - 1 ] & sGeneralMove->vbbBlackPPBoardMask[ iCol - 1 ] );

      }      
      else
      {
         sGeneralMove->vbbWhitePPWideVector[ iSquare ] = sGeneralMove->vbbWhitePPVector[ iSquare ] | 
                                                         ( sGeneralMove->vbbRow[ iSquare - 1 ] & sGeneralMove->vbbWhitePPBoardMask[ iCol - 1 ] ) | 
                                                         ( sGeneralMove->vbbRow[ iSquare + 1 ] & sGeneralMove->vbbWhitePPBoardMask[ iCol - 1 ] );
         sGeneralMove->vbbBlackPPWideVector[ iSquare ] = sGeneralMove->vbbBlackPPVector[ iSquare ] |
                                                         ( sGeneralMove->vbbRow[ iSquare - 1 ] & sGeneralMove->vbbBlackPPBoardMask[ iCol - 1 ] ) |
                                                         ( sGeneralMove->vbbRow[ iSquare + 1 ] & sGeneralMove->vbbBlackPPBoardMask[ iCol - 1 ] );

      }      

   }

   // Assign the specific move scores.
   // Heuristic move scores
   sGeneralMove->msRegular     = dsRegular;
   sGeneralMove->msPawnTwo     = dsPawnTwo;
   sGeneralMove->msPiece       = dsPiece;
   sGeneralMove->msQueen       = dsQueen;
   sGeneralMove->msBishop      = dsBishop;
   sGeneralMove->msRook        = dsRook;
   sGeneralMove->msKnight      = dsKnight;
   sGeneralMove->msKing        = dsKing;  
   sGeneralMove->msCaptureDown = dsCaptureDown;
   sGeneralMove->msCaptureSide = dsCaptureSide;
   sGeneralMove->msCaptureUp   = dsCaptureUp;
   sGeneralMove->msCheck       = dsCheck;
   sGeneralMove->msPromotion   = dsPromotion;
   sGeneralMove->msCastle      = dsCastle;
   sGeneralMove->msKillerMove  = dsKillerMove;
   sGeneralMove->msHH          = dsHH;
   sGeneralMove->msKingCapture = dsKingCapture;
   sGeneralMove->msBestMove    = dsBestMove;
   sGeneralMove->msPVMove      = dsPVMove;

   // Redefine the captures.
   // Specific move scores.
   sGeneralMove->msPawnTakesQueen    = dsPawnTakesQueen;
   sGeneralMove->msPawnTakesRook     = dsPawnTakesRook;
   sGeneralMove->msPawnTakesBishop   = dsPawnTakesBishop;
   sGeneralMove->msPawnTakesKnight   = dsPawnTakesKnight;
   sGeneralMove->msPawnTakesPawn     = dsPawnTakesPawn;
   sGeneralMove->msKnightTakesPawn   = dsKnightTakesPawn;
   sGeneralMove->msKnightTakesKnight = dsKnightTakesKnight;
   sGeneralMove->msKnightTakesBishop = dsKnightTakesBishop;
   sGeneralMove->msKnightTakesRook   = dsKnightTakesRook;
   sGeneralMove->msKnightTakesQueen  = dsKnightTakesQueen;
   sGeneralMove->msBishopTakesPawn   = dsBishopTakesPawn;
   sGeneralMove->msBishopTakesKnight = dsBishopTakesKnight;
   sGeneralMove->msBishopTakesBishop = dsBishopTakesBishop;
   sGeneralMove->msBishopTakesRook   = dsBishopTakesRook;
   sGeneralMove->msBishopTakesQueen  = dsBishopTakesQueen;
   sGeneralMove->msRookTakesPawn     = dsRookTakesPawn;
   sGeneralMove->msRookTakesKnight   = dsRookTakesKnight;
   sGeneralMove->msRookTakesBishop   = dsRookTakesBishop;
   sGeneralMove->msRookTakesBishop   = dsRookTakesBishop;
   sGeneralMove->msRookTakesRook     = dsRookTakesRook;
   sGeneralMove->msRookTakesQueen    = dsRookTakesQueen;
   sGeneralMove->msQueenTakesPawn    = dsQueenTakesPawn;
   sGeneralMove->msQueenTakesKnight  = dsQueenTakesKnight;
   sGeneralMove->msQueenTakesBishop  = dsQueenTakesBishop;
   sGeneralMove->msQueenTakesRook    = dsQueenTakesRook;
   sGeneralMove->msQueenTakesQueen   = dsQueenTakesQueen;

}

//
////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateKing( GeneralMove * sGeneralMove, 
                    int rowIndex, 
                    int colIndex, 
                    int square )
//
{
     
   // Set up the new squares.
   int vNewRow[ 8 ];
   int vNewCol[ 8 ];
   vNewRow[ 0 ] = rowIndex + 1;
   vNewCol[ 0 ] = colIndex + 1;
   vNewRow[ 1 ] = rowIndex + 1;
   vNewCol[ 1 ] = colIndex + 0;
   vNewRow[ 2 ] = rowIndex + 1;
   vNewCol[ 2 ] = colIndex - 1;
   vNewRow[ 3 ] = rowIndex + 0;
   vNewCol[ 3 ] = colIndex + 1;
   vNewRow[ 4 ] = rowIndex + 0;
   vNewCol[ 4 ] = colIndex - 1;
   vNewRow[ 5 ] = rowIndex - 1;
   vNewCol[ 5 ] = colIndex + 1;
   vNewRow[ 6 ] = rowIndex - 1;
   vNewCol[ 6 ] = colIndex + 0;
   vNewRow[ 7 ] = rowIndex - 1;
   vNewCol[ 7 ] = colIndex - 1;

   // Make the assignments.
   int length = 8;
   int vMove[ 8 ];
   int index;

   CalculateSquares( vNewRow, 
                     vNewCol, 
                     vMove, 
                     &length );
                     
   sGeneralMove->vKNOM[ square ] = length;
   
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mKMove[ square ][ index ] = vMove[ index ];
      
   }
   
   // Set the bit board.
   sGeneralMove->bbKMove[ square ] = 0;
   
   for ( int moveIndex = 0; moveIndex < length; moveIndex++ )
   {
       
      sGeneralMove->bbKMove[ square ] = SetBitToOne( sGeneralMove->bbKMove[ square ],
                                                     sGeneralMove->mKMove[ square ][ moveIndex ] );
                                                     
   }

   // Calculate a Bubble around the king to for use in calculating king safety.
   for ( index = 0; index < 64; index++ )
   {

      sGeneralMove->bbKingBubble[ index ] = 0;

      // Set the Kings square to 1. and move around the king if possible.
      sGeneralMove->bbKingBubble[ index ] = SetBitToOne( sGeneralMove->bbKingBubble[ index ], 
                                                         index );

      // Use the square the king can move to to calculate the bubble.
      sGeneralMove->bbKingBubble[ index ] |= sGeneralMove->bbKMove[ index ];

   }

}

//
////////////////////////////////////////////////////////////////////////////
//
void CalculateQueen( GeneralMove * sGeneralMove, 
                                    int rowIndex, 
                     int colIndex, 
                     int square )
{
//

  // Declarations.
  int index;

   // Create a slider vector.
   int vSlide[ 7 ];
   int vRows[ 7 ];
   int vCols[ 7 ];
   
   for ( index = 0; index < 7; index++ )
   {
       
      vSlide[ index ] = index + 1;
      
    }

   // Make the assignments.
   int length = 7;
   int vMove[ 7 ];

   // Do North East.
   length = 7;
   for ( index = 0; index < 7; index++ )
   {
       
     vRows[ index ]  = rowIndex + vSlide[ index ];
     vCols[ index ]  = colIndex + vSlide[ index ];
     
   }
   
   CalculateSquares( vRows, 
                     vCols, 
                     vMove, 
                     &length );
                     
   sGeneralMove->vQNOMNE[ square ] = length;
   
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mQNE[ square ][ index ] = vMove[ index ];
      sGeneralMove->bbQMove[ square ] = SetBitToOne( sGeneralMove->bbQMove[ square ],
                                                     sGeneralMove->mQNE[ square ][ index ] );
      sGeneralMove->bbQMoveNE[ square ] = SetBitToOne( sGeneralMove->bbQMoveNE[ square ],
                                                      sGeneralMove->mQNE[ square ][ index ] );
                                                      
   }

   // Do North West.
   length = 7;
   for ( index = 0; index < 7; index++ )
   {
       
     vRows[ index ]  = rowIndex - vSlide[ index ];
     vCols[ index ]  = colIndex + vSlide[ index ];
     
   }
   
   CalculateSquares( vRows, vCols, vMove, &length );
   sGeneralMove->vQNOMNW[ square ] = length;
   
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mQNW[ square ][ index ] = vMove[ index ];
      sGeneralMove->bbQMove[ square ] = SetBitToOne( sGeneralMove->bbQMove[ square ],
                                                     sGeneralMove->mQNW[ square ][ index ] );
      sGeneralMove->bbQMoveNW[ square ] = SetBitToOne( sGeneralMove->bbQMoveNW[ square ],
                                                      sGeneralMove->mQNW[ square ][ index ] );
                                                      
   }

   // Do South East.
   length = 7;
   for ( index = 0; index < 7; index++ )
   {
       
     vRows[ index ]  = rowIndex + vSlide[ index ];
     vCols[ index ]  = colIndex - vSlide[ index ];
     
   }
   
   CalculateSquares( vRows, vCols, vMove, &length );
   sGeneralMove->vQNOMSE[ square ] = length;
   
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mQSE[ square ][ index ] = vMove[ index ];
      sGeneralMove->bbQMove[ square ] = SetBitToOne( sGeneralMove->bbQMove[ square ],
                                                     sGeneralMove->mQSE[ square ][ index ] );
      sGeneralMove->bbQMoveSE[ square ] = SetBitToOne( sGeneralMove->bbQMoveSE[ square ],
                                                      sGeneralMove->mQSE[ square ][ index ] );
                                                      
   }

   // Do South West.
   length = 7;
   for ( index = 0; index < 7; index++ )
   {
       
     vRows[ index ]  = rowIndex - vSlide[ index ];
     vCols[ index ]  = colIndex - vSlide[ index ];
     
   }
   
   CalculateSquares( vRows, vCols, vMove, &length );
   sGeneralMove->vQNOMSW[ square ] = length;
   
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mQSW[ square ][ index ] = vMove[ index ];
      sGeneralMove->bbQMove[ square ] = SetBitToOne( sGeneralMove->bbQMove[ square ],
                                                     sGeneralMove->mQSW[ square ][ index ] );
      sGeneralMove->bbQMoveSW[ square ] = SetBitToOne( sGeneralMove->bbQMoveSW[ square ],
                                                       sGeneralMove->mQSW[ square ][ index ] );
   }

   // Do North.
   length = 7;
   for ( index = 0; index < 7; index++ )
   {
       
     vRows[ index ]  = rowIndex;
     vCols[ index ]  = colIndex + vSlide[ index ];
     
   }
   
   CalculateSquares( vRows, vCols, vMove, &length );
   sGeneralMove->vQNOMN[ square ] = length;
   
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mQN[ square ][ index ] = vMove[ index ];
      sGeneralMove->bbQMove[ square ] = SetBitToOne( sGeneralMove->bbQMove[ square ],
                                                     sGeneralMove->mQN[ square ][ index ] );
      sGeneralMove->bbQMoveN[ square ] = SetBitToOne( sGeneralMove->bbQMoveN[ square ],
                                                      sGeneralMove->mQN[ square ][ index ] );
                                                      
   }

   // Do West.
   length = 7;
   for ( index = 0; index < 7; index++ )
   {
       
     vRows[ index ]  = rowIndex - vSlide[ index ];
     vCols[ index ]  = colIndex;
     
   }
   
   CalculateSquares( vRows, vCols, vMove, &length );
   sGeneralMove->vQNOMW[ square ] = length;
   
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mQW[ square ][ index ] = vMove[ index ];
      sGeneralMove->bbQMove[ square ] = SetBitToOne( sGeneralMove->bbQMove[ square ],
                                                     sGeneralMove->mQW[ square ][ index ] );
      sGeneralMove->bbQMoveW[ square ] = SetBitToOne( sGeneralMove->bbQMoveW[ square ],
                                                      sGeneralMove->mQW[ square ][ index ] );
                                                      
   }

   // Do East.
   length = 7;
   for ( index = 0; index < 7; index++ )
   {
       
     vRows[ index ]  = rowIndex + vSlide[ index ];
     vCols[ index ]  = colIndex;
     
   }
   
   CalculateSquares( vRows, vCols, vMove, &length );
   sGeneralMove->vQNOME[ square ] = length;
   
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mQE[ square ][ index ] = vMove[ index ];
      sGeneralMove->bbQMove[ square ] = SetBitToOne( sGeneralMove->bbQMove[ square ],
                                                     sGeneralMove->mQE[ square ][ index ] );
      sGeneralMove->bbQMoveE[ square ] = SetBitToOne( sGeneralMove->bbQMoveE[ square ],
                                                      sGeneralMove->mQE[ square ][ index ] );
                                                       
   }

   // Do South.
   length = 7;
   for ( index = 0; index < 7; index++ )
   {
       
     vRows[ index ]  = rowIndex;
     vCols[ index ]  = colIndex - vSlide[ index ];
     
   }
   
   CalculateSquares( vRows, vCols, vMove, &length );
   sGeneralMove->vQNOMS[ square ] = length;
   
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mQS[ square ][ index ] = vMove[ index ];
      sGeneralMove->bbQMove[ square ] = SetBitToOne( sGeneralMove->bbQMove[ square ],
                                                     sGeneralMove->mQS[ square ][ index ] );
      sGeneralMove->bbQMoveS[ square ] = SetBitToOne( sGeneralMove->bbQMoveS[ square ],
                                                      sGeneralMove->mQS[ square ][ index ] );
                                                       
   }

}

//
////////////////////////////////////////////////////////////////////////////
//
void CalculateBishops( GeneralMove * sGeneralMove, 
                       int rowIndex, 
                       int colIndex, 
                       int square )
{
//

   // Create a slider vector.
   int vSlide[ 7 ];
   int vRows[ 7 ];
   int vCols[ 7 ];
   int index;
   
   for ( index = 0; index < 7; index++ )
   {
       
      vSlide[ index ] = index + 1;
      
    }

   // Make the assignments.
   int length = 7;
   int vMove[ 7 ];

   // Do North East.
   length = 7;
   for ( index = 0; index < 7; index++ )
   {
       
     vRows[ index ]  = rowIndex + vSlide[ index ];
     vCols[ index ]  = colIndex + vSlide[ index ];
     
   }
   
   CalculateSquares( vRows, vCols, vMove, &length );
   sGeneralMove->vBNOMNE[ square ] = length;
   
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mBNE[ square ][ index ] = vMove[ index ];
      sGeneralMove->bbBMove[ square ] = SetBitToOne( sGeneralMove->bbBMove[ square ],
                                                     sGeneralMove->mBNE[ square ][ index ] );
      sGeneralMove->bbBMoveNE[ square ] = SetBitToOne( sGeneralMove->bbBMoveNE[ square ],
                                                       sGeneralMove->mBNE[ square ][ index ] );
                                                      
   }

   // Do North West.
   length = 7;
   for ( index = 0; index < 7; index++ )
   {
       
     vRows[ index ]  = rowIndex - vSlide[ index ];
     vCols[ index ]  = colIndex + vSlide[ index ];
     
   }
   
   CalculateSquares( vRows, vCols, vMove, &length );
   sGeneralMove->vBNOMNW[ square ] = length;
   
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mBNW[ square ][ index ] = vMove[ index ];
      sGeneralMove->bbBMove[ square ] = SetBitToOne( sGeneralMove->bbBMove[ square ],
                                                     sGeneralMove->mBNW[ square ][ index ] );
      sGeneralMove->bbBMoveNW[ square ] = SetBitToOne( sGeneralMove->bbBMoveNW[ square ],
                                                       sGeneralMove->mBNW[ square ][ index ] );
                                                      
   }

   // Do South East.
   length = 7;
   for ( int index = 0; index < 7; index++ )
   {
       
     vRows[ index ]  = rowIndex + vSlide[ index ];
     vCols[ index ]  = colIndex - vSlide[ index ];
     
   }
   
   CalculateSquares( vRows, vCols, vMove, &length );
   sGeneralMove->vBNOMSE[ square ] = length;
   
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mBSE[ square ][ index ] = vMove[ index ];
      sGeneralMove->bbBMove[ square ] = SetBitToOne( sGeneralMove->bbBMove[ square ],
                                                     sGeneralMove->mBSE[ square ][ index ] );
      sGeneralMove->bbBMoveSE[ square ] = SetBitToOne( sGeneralMove->bbBMoveSE[ square ],
                                                       sGeneralMove->mBSE[ square ][ index ] );
                                                       
   }

   // Do South West.
   length = 7;
   for ( index = 0; index < 7; index++ )
   {
       
     vRows[ index ]  = rowIndex - vSlide[ index ];
     vCols[ index ]  = colIndex - vSlide[ index ];
     
   }
   
   CalculateSquares( vRows, vCols, vMove, &length );
   sGeneralMove->vBNOMSW[ square ] = length;
   
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mBSW[ square ][ index ] = vMove[ index ];
      sGeneralMove->bbBMove[ square ] = SetBitToOne( sGeneralMove->bbBMove[ square ],
                                                     sGeneralMove->mBSW[ square ][ index ] );
      sGeneralMove->bbBMoveSW[ square ] = SetBitToOne( sGeneralMove->bbBMoveSW[ square ],
                                                       sGeneralMove->mBSW[ square ][ index ] );
                                                      
   }
   
}

//
////////////////////////////////////////////////////////////////////////////
//
void CalculateRooks( GeneralMove * sGeneralMove, 
                     int rowIndex, 
                     int colIndex, 
                     int square )
{
//

   // Create a slider vector.
   int vSlide[ 7 ];
   int vRows[ 7 ];
   int vCols[ 7 ];
   int index;
   
   for ( index = 0; index < 7; index++ )
   {
       
      vSlide[ index ] = index + 1;
      
    }

   // Make the assignments.
   int length = 7;
   int vMove[ 7 ];

   // Do North.
   length = 7;
   for ( index = 0; index < 7; index++ )
   {
       
     vRows[ index ]  = rowIndex;
     vCols[ index ]  = colIndex + vSlide[ index ];
     
   }
   
   CalculateSquares( vRows, vCols, vMove, &length );
   sGeneralMove->vRNOMN[ square ] = length;
   
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mRN[ square ][ index ] = vMove[ index ];
      sGeneralMove->bbRMove[ square ] = SetBitToOne( sGeneralMove->bbRMove[ square ],
                                                     sGeneralMove->mRN[ square ][ index ] );
      sGeneralMove->bbRMoveN[ square ] = SetBitToOne( sGeneralMove->bbRMoveN[ square ],
                                                      sGeneralMove->mRN[ square ][ index ] );
                                                      
   }

   // Do West.
   length = 7;
   for ( index = 0; index < 7; index++ )
   {
       
     vRows[ index ]  = rowIndex - vSlide[ index ];
     vCols[ index ]  = colIndex;
     
   }
   
   CalculateSquares( vRows, vCols, vMove, &length );
   sGeneralMove->vRNOMW[ square ] = length;
   
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mRW[ square ][ index ] = vMove[ index ];
      sGeneralMove->bbRMove[ square ] = SetBitToOne( sGeneralMove->bbRMove[ square ],
                                                     sGeneralMove->mRW[ square ][ index ] );
      sGeneralMove->bbRMoveW[ square ] = SetBitToOne( sGeneralMove->bbRMoveW[ square ],
                                                      sGeneralMove->mRW[ square ][ index ] );
                                                      
   }

   // Do East.
   length = 7;
   for ( index = 0; index < 7; index++ )
   {
       
     vRows[ index ]  = rowIndex + vSlide[ index ];
     vCols[ index ]  = colIndex;
   }
   
   CalculateSquares( vRows, vCols, vMove, &length );
   sGeneralMove->vRNOME[ square ] = length;
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mRE[ square ][ index ] = vMove[ index ];
      sGeneralMove->bbRMove[ square ] = SetBitToOne( sGeneralMove->bbRMove[ square ],
                                                      sGeneralMove->mRE[ square ][ index ] );
      sGeneralMove->bbRMoveE[ square ] = SetBitToOne( sGeneralMove->bbRMoveE[ square ],
                                                      sGeneralMove->mRE[ square ][ index ] );
                                                       
   }

   // Do South.
   length = 7;
   for ( index = 0; index < 7; index++ )
   {
       
     vRows[ index ]  = rowIndex;
     vCols[ index ]  = colIndex - vSlide[ index ];
     
   }
   CalculateSquares( vRows, vCols, vMove, &length );
   sGeneralMove->vRNOMS[ square ] = length;
   
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mRS[ square ][ index ] = vMove[ index ];
      sGeneralMove->bbRMove[ square ] = SetBitToOne( sGeneralMove->bbRMove[ square ],
                                                     sGeneralMove->mRS[ square ][ index ] );
      sGeneralMove->bbRMoveS[ square ] = SetBitToOne( sGeneralMove->bbRMoveS[ square ],
                                                      sGeneralMove->mRS[ square ][ index ] );
                                                       
   }
   
}

//
////////////////////////////////////////////////////////////////////////////
//
void CalculateKnights( GeneralMove * sGeneralMove, 
                       int rowIndex, 
                       int colIndex, 
                       int square )
//
{
                       
   // Set up the new squares.
   int vNewRow[ 8 ];
   int vNewCol[ 8 ];
   vNewRow[ 0 ] = rowIndex + 2;
   vNewCol[ 0 ] = colIndex + 1;
   vNewRow[ 1 ] = rowIndex + 2;
   vNewCol[ 1 ] = colIndex - 1;
   vNewRow[ 2 ] = rowIndex - 2;
   vNewCol[ 2 ] = colIndex + 1;
   vNewRow[ 3 ] = rowIndex - 2;
   vNewCol[ 3 ] = colIndex - 1;
   vNewRow[ 4 ] = rowIndex + 1;
   vNewCol[ 4 ] = colIndex + 2;
   vNewRow[ 5 ] = rowIndex - 1;
   vNewCol[ 5 ] = colIndex + 2;
   vNewRow[ 6 ] = rowIndex + 1;
   vNewCol[ 6 ] = colIndex - 2;
   vNewRow[ 7 ] = rowIndex - 1;
   vNewCol[ 7 ] = colIndex - 2;

   // Make the assignments.
   int length = 8;
   int vMove[ 8 ];
   int index;

   CalculateSquares( vNewRow, vNewCol, vMove, &length );
   sGeneralMove->vNNOM[ square ] = length;
   
   for ( index = 0; index < length; index++ )
   {
       
      sGeneralMove->mNMove[ square ][ index ] = vMove[ index ];
      
   }
   
   // Set the bit board.
   sGeneralMove->bbNMove[ square ] = 0;
   for ( int moveIndex = 0; moveIndex < length; moveIndex++ )
   {
       
      sGeneralMove->bbNMove[ square ] = SetBitToOne( sGeneralMove->bbNMove[ square ],
                                                     sGeneralMove->mNMove[ square ][ moveIndex ] );
                                                     
   }
   
}

//
////////////////////////////////////////////////////////////////////////////
//
void CalculatePawns( GeneralMove * sGeneralMove )
{
//
// This funciton calcualtes the pawns attackes and moves.
//

   // Loop over the squares.
   for ( int colIndex = 0; colIndex < 8; colIndex++ )
   {
       
      for ( int rowIndex = 0; rowIndex < 8; rowIndex++ )
      {
          
         // Calculate the square.
         int square = rowIndex + 8 * colIndex;

         // Initialize the bit boards.
         sGeneralMove->bbWPMove[ square ]   = 0;
         sGeneralMove->bbBPMove[ square ]   = 0;
         sGeneralMove->bbWPAttack[ square ] = 0;
         sGeneralMove->bbBPAttack[ square ] = 0;

         // The white pawn moves.
         if ( ( colIndex > 1 ) &
              ( colIndex < 7 ) )
         {
              
            sGeneralMove->bbWPMove[ square ] = SetBitToOne( sGeneralMove->bbWPMove[ square ], 
                                                            ( square + 8 ) );
                                                           
         }
         
         if ( colIndex == 1 )
         {
              
            sGeneralMove->bbWPMove[ square ] = SetBitToOne( sGeneralMove->bbWPMove[ square ],
                                                            ( square + 8 ) );
            sGeneralMove->bbWPMove[ square ] = SetBitToOne( sGeneralMove->bbWPMove[ square ],
                                                            ( square + 16 ) );
                                                           
         }
         
         if ( ( colIndex == 0 ) |
              ( colIndex == 7 ) )
         {
              
            sGeneralMove->bbWPMove[ square  ] = 0;
            
         }

         // The black pawn moves.
         if ( ( colIndex < 7 ) &
              ( colIndex > 0 ) )
         {
              
            sGeneralMove->bbBPMove[ square ] = SetBitToOne( sGeneralMove->bbBPMove[ square ],
                                                            ( square - 8 ) );
                                                           
         }
         
         if ( colIndex == 6 )
         {
              
            sGeneralMove->bbBPMove[ square ] = SetBitToOne( sGeneralMove->bbBPMove[ square ], 
                                                            ( square - 8 ) );
            sGeneralMove->bbBPMove[ square ] = SetBitToOne( sGeneralMove->bbBPMove[ square ], 
                                                            ( square - 16 ) );
                                                            
         }
         
         if ( ( colIndex == 0 ) |
              ( colIndex == 7 ) )
         {
              
            sGeneralMove->bbBPMove[ square  ] = 0;
            
         }
 
         // The white pawn Attack.
         if ( ( colIndex > 0 ) &
              ( colIndex < 7 ) )
         {
              
            BitBoard bbLeft  = 0;
            BitBoard bbRight = 0;
            if ( rowIndex == 0 )
            {
                 
               bbRight = 0;
               bbRight = SetBitToOne( bbRight,
                                     ( square + 9 ) );
                                     
            }
            
            if ( rowIndex == 7 )
            {
                 
               bbLeft = 0;
               bbLeft = SetBitToOne( bbLeft, ( square + 7 ) );
               
            }
            
            if ( ( rowIndex < 7 ) &&
                 ( rowIndex > 0 ) )
            {
                 
               bbRight = 0;
               bbLeft  = 0;
               bbRight = SetBitToOne( bbRight, ( square + 7 ) );
               bbLeft  = SetBitToOne( bbLeft,  ( square + 9 ) );
               
            }

            sGeneralMove->bbWPAttack[ square ] = sGeneralMove->bbWPAttack[ square ] |
                                                 bbRight |
                                                 bbLeft;
                                                 
         }

         // The black pawn Attack.
         if ( ( colIndex > 0 ) &
              ( colIndex < 7 ) )
         {
              
            BitBoard bbLeft  = 0;
            BitBoard bbRight = 0;
            if ( rowIndex == 0 )
            {
                 
               bbRight = 0;
               bbRight = SetBitToOne( bbRight, ( square - 7 ) );
               
            }
            
            if ( rowIndex == 7 )
            {
                 
               bbLeft = 0;
               bbLeft = SetBitToOne( bbLeft, ( square - 9 ) );
               
            }
            
            if ( ( rowIndex < 7 ) &&
                 ( rowIndex > 0 ) )
            {
                 
               bbRight = 0;
               bbLeft  = 0;
               bbRight = SetBitToOne( bbRight, ( square - 7 ) );
               bbLeft  = SetBitToOne( bbLeft,  ( square - 9 ) );
               
            }

            sGeneralMove->bbBPAttack[ square ] = sGeneralMove->bbBPAttack[ square ] |
                                                  bbRight |
                                                  bbLeft;
                                                  
         }
         
      }
      
   }
   
}

//
//-----------------------------------------------------------------
//
void CalculateSelectors( GeneralMove * sGeneralMove )
{
     
   // This function calcualates useful selectors for later use.
   sGeneralMove->bbColZero                  = 0;
   sGeneralMove->bbColOne                   = 0;
   sGeneralMove->bbColSix                   = 0;
   sGeneralMove->bbColSeven                 = 0;
   sGeneralMove->bbRowZero                  = 0;
   sGeneralMove->bbRowSeven                 = 0;
   sGeneralMove->bbKingSideWhite            = 0;
   sGeneralMove->bbKingSideBlack            = 0;
   sGeneralMove->bbQueenSideWhite           = 0;
   sGeneralMove->bbQueenSideBlack           = 0;
   sGeneralMove->bbKingSideWhiteCastleArea  = 0;
   sGeneralMove->bbKingSideBlackCastleArea  = 0;
   sGeneralMove->bbQueenSideWhiteCastleArea = 0;
   sGeneralMove->bbQueenSideBlackCastleArea = 0;

   // Do the row and column selectors.
   sGeneralMove->bbColZero  = 0x00000000000000FF;
   sGeneralMove->bbColOne   = 0x000000000000FF00;
   sGeneralMove->bbColSix   = 0x00FF000000000000;
   sGeneralMove->bbColSeven = 0xFF00000000000000;
   sGeneralMove->bbRowZero  = 0x8080808080808080;
   sGeneralMove->bbRowSeven = 0x0101010101010101;
      

   // Do the side selectors.
   sGeneralMove->bbKingSideWhite  = SetBitToOne( sGeneralMove->bbKingSideWhite,  dF1 );
   sGeneralMove->bbKingSideWhite  = SetBitToOne( sGeneralMove->bbKingSideWhite,  dG1 );
   sGeneralMove->bbQueenSideWhite = SetBitToOne( sGeneralMove->bbQueenSideWhite, dB1 );
   sGeneralMove->bbQueenSideWhite = SetBitToOne( sGeneralMove->bbQueenSideWhite, dC1 );
   sGeneralMove->bbQueenSideWhite = SetBitToOne( sGeneralMove->bbQueenSideWhite, dD1 );
   sGeneralMove->bbKingSideBlack  = SetBitToOne( sGeneralMove->bbKingSideBlack,  dF8 );
   sGeneralMove->bbKingSideBlack  = SetBitToOne( sGeneralMove->bbKingSideBlack,  dG8 );
   sGeneralMove->bbQueenSideBlack = SetBitToOne( sGeneralMove->bbQueenSideBlack, dB8 );
   sGeneralMove->bbQueenSideBlack = SetBitToOne( sGeneralMove->bbQueenSideBlack, dC8 );
   sGeneralMove->bbQueenSideBlack = SetBitToOne( sGeneralMove->bbQueenSideBlack, dD8 );

   // Define the area the king would castle throught.                     
   sGeneralMove->bbKingSideWhiteCastleArea  = SetBitToOne( sGeneralMove->bbKingSideWhiteCastleArea,   dE1 );
   sGeneralMove->bbKingSideWhiteCastleArea  = SetBitToOne( sGeneralMove->bbKingSideWhiteCastleArea,   dF1 );
   sGeneralMove->bbKingSideWhiteCastleArea  = SetBitToOne( sGeneralMove->bbKingSideWhiteCastleArea,  dG1 );
   sGeneralMove->bbQueenSideWhiteCastleArea = SetBitToOne( sGeneralMove->bbQueenSideWhiteCastleArea, dC1 );
   sGeneralMove->bbQueenSideWhiteCastleArea = SetBitToOne( sGeneralMove->bbQueenSideWhiteCastleArea, dD1 );
   sGeneralMove->bbQueenSideWhiteCastleArea = SetBitToOne( sGeneralMove->bbQueenSideWhiteCastleArea, dE1 );
   sGeneralMove->bbKingSideBlackCastleArea  = SetBitToOne( sGeneralMove->bbKingSideBlackCastleArea,  dE8 );
   sGeneralMove->bbKingSideBlackCastleArea  = SetBitToOne( sGeneralMove->bbKingSideBlackCastleArea,  dF8 );
   sGeneralMove->bbKingSideBlackCastleArea  = SetBitToOne( sGeneralMove->bbKingSideBlackCastleArea,  dG8 );
   sGeneralMove->bbQueenSideBlackCastleArea = SetBitToOne( sGeneralMove->bbQueenSideBlackCastleArea, dC8 );
   sGeneralMove->bbQueenSideBlackCastleArea = SetBitToOne( sGeneralMove->bbQueenSideBlackCastleArea, dD8 );
   sGeneralMove->bbQueenSideBlackCastleArea = SetBitToOne( sGeneralMove->bbQueenSideBlackCastleArea, dE8 );
   
}

//
//-------------------------------------------------------------------------------------------------
//
void CreateRandomKeyFile()
{

// This function is used to create the random hash keys that will be persistant.
//

   // Delcare some variables.
   //BitBoard bbRandom;
   //BitBoard bbData;


   // Read the opening book.
   ofstream ofKeys( "RandomKeys.txt" );
   if ( ofKeys.fail() )
   {
   
      cout << "Output RandomKeys.txt file failed to open." << endl;
      
   }

   // Set up the hash keys.
   int iSquareIndex = 0;
   for ( iSquareIndex = 0; iSquareIndex < 64; iSquareIndex++ )
   {

      for( int iPieceIndex = 0; iPieceIndex < 12; iPieceIndex++ )
      {

        ofKeys << RandomBB() << endl;

      }

      ofKeys << RandomBB() << endl;

   }

   for ( int iStateIndex = 0; iStateIndex < 6; iStateIndex++ )
   {

      ofKeys << RandomBB() << endl;

   }

   ofKeys << RandomBB() << endl;
   ofKeys << RandomBB() << endl;
   ofKeys << RandomBB() << endl;
   ofKeys << RandomBB() << endl;

   ofKeys << RandomBB() << endl;
   ofKeys << RandomBB() << endl;

   ofKeys << RandomBB() << endl;

   // Close the file.
   ofKeys.close();

}
