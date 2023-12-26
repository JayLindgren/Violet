// Copyright 2006 by Jay Lindgren. All Rights Reserved.
//
// Calculate the moves for a given board
// This function calculates it for both the white and the black pieces.
// Both moves are needed in order to calculate casteling, check and checkmate.
//

#include <iostream>
#include <iomanip>
#include <string.h>
#include <malloc.h>
#include <fstream>
#include "Definitions.h"
#include "Structures.h"
#include "Functions.h"

using namespace std;

// Create a global file for debugging.
ofstream gofDebugMoves;

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateMoves( struct Move        * argsMoves, 
                     struct Board       * argsBoard, 
                     struct GeneralMove * argsGeneralMoves )
{
// This is a controlling function.
//

   // Debug the inputs.
   assert( argsMoves        >= 0 );
   assert( argsGeneralMoves >= 0 );
   assert( argsMoves        >= 0 );

   // Initialize the counters.
   argsBoard->siNumberOfMoves    = 0;
   argsBoard->bbWhiteAttack      = 0;
   argsBoard->bbWhiteMove        = 0;
   argsBoard->bbBlackAttack      = 0;
   argsBoard->bbBlackMove        = 0;
   argsBoard->bbWhiteKingMove    = 0;
   argsBoard->bbWhiteKingAttack  = 0;
   argsBoard->bbBlackKingMove    = 0;
   argsBoard->bbBlackKingAttack  = 0;

   // Calculate the moves for the pieces.

   // Calculate for the white pieces.
   if ( argsBoard->siColorToMove == dWhite )
   {
   
      if ( argsBoard->bbWhitePawn > 0 )
      {

         CalculateWhitePawns( argsMoves, 
                              argsBoard, 
                              argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteKnight > 0 )
      {

         CalculateWhiteKnights( argsMoves, 
                                argsBoard, 
                                argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteBishop > 0 )
      {

         CalculateWhiteBishops( argsMoves, 
                                argsBoard, 
                                argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteRook > 0 )
      {

         CalculateWhiteRooks( argsMoves, 
                              argsBoard, 
                              argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteQueen > 0 )
      {

         CalculateWhiteQueens( argsMoves, 
                               argsBoard, 
                               argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteKing > 0 )
      {

         CalculateWhiteKing( argsMoves, 
                             argsBoard, 
                             argsGeneralMoves );

      }

      if ( argsBoard->bbCastle > 0 )
      {

         CastleWhite( argsMoves, 
                      argsBoard, 
                      argsGeneralMoves );

      }
      
   }

   if ( argsBoard->siColorToMove == dBlack )
   {

      if ( argsBoard->bbBlackPawn > 0 )
      {

         CalculateBlackPawns( argsMoves, 
                              argsBoard, 
                              argsGeneralMoves );

      }

      if ( argsBoard->bbBlackKnight > 0 )
      {

         CalculateBlackKnights( argsMoves, 
                                argsBoard, 
                                argsGeneralMoves );

      }

      if ( argsBoard->bbBlackBishop > 0 )
      {

         CalculateBlackBishops( argsMoves, 
                                argsBoard, 
                                argsGeneralMoves );

      }

      if ( argsBoard->bbBlackRook > 0 )
      {

         CalculateBlackRooks( argsMoves, 
                              argsBoard, 
                              argsGeneralMoves );

      }

      if ( argsBoard->bbBlackQueen > 0 )
      {

         CalculateBlackQueens( argsMoves, 
                               argsBoard, 
                               argsGeneralMoves );

      }

      if ( argsBoard->bbBlackKing > 0 )
      {

         CalculateBlackKing( argsMoves, 
                             argsBoard, 
                             argsGeneralMoves );

      }

      if ( argsBoard->bbCastle > 0 )
      {

         CastleBlack( argsMoves, 
                      argsBoard, 
                      argsGeneralMoves );

      }
      
   }

   // Calculate Check.
   CalculateCheck( argsMoves, 
                   argsBoard, 
                   argsGeneralMoves );

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateWhitePawns( struct Move * argsMoves, 
                          struct Board * argsBoard, 
                          struct GeneralMove * argsGeneralMoves )
{
//
// This function looks at the white pawn moves.
//

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbWhitePawn == 0 )
   {
   
      return;

   }

   // Do the one step moves.
   BitBoard bbOneStep = ( argsBoard->bbWhitePawn << 8 ) & ( ~ argsBoard->bbAllPieces );

   // Do the two step moves.
   BitBoard bbElegibleMoves = argsBoard->bbWhitePawn & argsGeneralMoves->bbColOne;
   bbElegibleMoves          = bbElegibleMoves & ( bbOneStep >> 8 );
   bbElegibleMoves          = bbElegibleMoves << 16;
   BitBoard bbTwoStep       = ( ~ argsBoard->bbAllPieces ) & bbElegibleMoves;

   // Do the attacks to the right.
   BitBoard bbAttacksRight = ( argsBoard->bbWhitePawn << 9 );
   bbAttacksRight          = bbAttacksRight & argsBoard->bbBlackPieces;
   bbAttacksRight          = bbAttacksRight & ( ~ argsGeneralMoves->bbRowSeven );

   //Do the Attacks to the left.
   BitBoard bbAttacksLeft = ( argsBoard->bbWhitePawn << 7 );
   bbAttacksLeft          = bbAttacksLeft & argsBoard->bbBlackPieces;
   bbAttacksLeft          = bbAttacksLeft & ( ~ argsGeneralMoves->bbRowZero );

   // Combine the attacks and moves.
   argsBoard->bbWhiteAttack = argsBoard->bbWhiteAttack | bbAttacksRight | bbAttacksLeft;
   argsBoard->bbWhiteMove   = argsBoard->bbWhiteMove   | bbTwoStep      | bbOneStep;

   // Take out the promotion things.
   //Find the promotions
   BitBoard bbPromotionsOne   = ( argsGeneralMoves->bbColSeven & bbOneStep );
   BitBoard bbPromotionsRight = ( argsGeneralMoves->bbColSeven & bbAttacksRight );
   BitBoard bbPromotionsLeft  = ( argsGeneralMoves->bbColSeven & bbAttacksLeft );

   // Remove the pormotions from other moves.
   bbOneStep      = bbOneStep & ~argsGeneralMoves->bbColSeven;
   bbAttacksRight = bbAttacksRight & ~argsGeneralMoves->bbColSeven;
   bbAttacksLeft  = bbAttacksLeft & ~argsGeneralMoves->bbColSeven;

   // Find the moves and attacks.
   int vMovesOneStep[    8 ];
   int vMovesTwoStep[    8 ];
   int vAttacksLeft[     8 ];
   int vAttacksRight[    8 ];
   int vPromotionsOne[   8 ];
   int vPromotionsRight[ 8 ];
   int vPromotionsLeft[  8 ];
   int siNumberOfMovesOneStep  = Find( bbOneStep,         vMovesOneStep,    argsGeneralMoves );
   int siNumberOfMovesTwoStep  = Find( bbTwoStep,         vMovesTwoStep,    argsGeneralMoves );
   int numberOfAttacksRight    = Find( bbAttacksRight,    vAttacksRight,    argsGeneralMoves );
   int numberOfAttacksLeft     = Find( bbAttacksLeft,     vAttacksLeft,     argsGeneralMoves );
   int numberOfPromotionsOne   = Find( bbPromotionsOne,   vPromotionsOne,   argsGeneralMoves );
   int numberOfPromotionsRight = Find( bbPromotionsRight, vPromotionsRight, argsGeneralMoves );
   int numberOfPromotionsLeft  = Find( bbPromotionsLeft,  vPromotionsLeft,  argsGeneralMoves );

   // Enter the one step moves.
   for ( int moveIndex = 0; moveIndex < siNumberOfMovesOneStep; moveIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );

      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vMovesOneStep[ moveIndex ] - 8 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                              vMovesOneStep[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vMovesOneStep[ moveIndex ] - 8;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vMovesOneStep[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dOneSquare;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhitePawn;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msRegular;

   }

   // Enter the two step moves.
   for ( int moveIndex = 0; moveIndex < siNumberOfMovesTwoStep; moveIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vMovesTwoStep[ moveIndex ] - 16 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vMovesTwoStep[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vMovesTwoStep[ moveIndex ] - 16;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vMovesTwoStep[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dTwoSquare;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhitePawn;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbEPSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbEPSquare,
                                                                              vMovesTwoStep[ moveIndex ] - 8 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPawnTwo;

   }
   	
   // Enter the right attacks.
   for ( int moveIndex = 0; moveIndex < numberOfAttacksRight; moveIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vAttacksRight[ moveIndex ] - 9 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vAttacksRight[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vAttacksRight[ moveIndex ] - 9;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vAttacksRight[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhitePawn;

      // Score the moves.
      switch ( argsBoard->mBoard[  argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare ] ) 
      {

         case dBlackPawn:
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesPawn;
            break;
      
         }
         case dBlackKnight :
         {   
              
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesKnight;
            break;

         }
         case dBlackBishop :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesBishop;
            break;
   
         }
         case dBlackRook :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesRook;
            break;
   
         }
         case dBlackQueen :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesQueen;
            break;
   
         }
         case dBlackKing :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
            break;
   
         }

      }

   }
   		
   // Enter the left attacks.
   for ( int moveIndex = 0; moveIndex < numberOfAttacksLeft; moveIndex++ )
   {
       
      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare, 
    		                                                                     vAttacksLeft[ moveIndex ] - 7 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vAttacksLeft[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vAttacksLeft[ moveIndex ] - 7;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vAttacksLeft[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhitePawn;

      // Score the moves.
      switch ( argsBoard->mBoard[  argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare ] ) 
      {

         case dBlackPawn:
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesPawn;
            break;
      
         }
         case dBlackKnight :
         {   
              
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesKnight;
            break;

         }
         case dBlackBishop :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesBishop;
            break;
   
         }
         case dBlackRook :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesRook;
            break;
   
         }
         case dBlackQueen :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesQueen;
            break;
   
         }
         case dBlackKing :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
            break;
   
         }

      }


   }

   // Enter the promotions one.
   for ( int moveIndex = 0; moveIndex < numberOfPromotionsOne; moveIndex++ )
   {

      for ( int pieceIndex = 0; pieceIndex < 4; pieceIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                              vPromotionsOne[ moveIndex ] - 8 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                                 vPromotionsOne[ moveIndex ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPromotionsOne[ moveIndex ] - 8;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vPromotionsOne[ moveIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dPromote;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = argsGeneralMoves->vPromotionPieces[ pieceIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPromotion;

      }

   }

   // Enter the promotions right.
   for ( int moveIndex = 0; moveIndex < numberOfPromotionsRight; moveIndex++ )
   {

      for ( int pieceIndex = 0; pieceIndex < 4; pieceIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                              vPromotionsRight[ moveIndex ] - 9 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                                 vPromotionsRight[ moveIndex ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPromotionsRight[ moveIndex ] - 9;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vPromotionsRight[ moveIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCaptureAndPromote;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = argsGeneralMoves->vPromotionPieces[ pieceIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iCapture     = argsBoard->mBoard[ vPromotionsRight[ moveIndex ] ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPromotion;

      }

   }

   // Enter the promotions Left.
   for ( int moveIndex = 0; moveIndex < numberOfPromotionsLeft; moveIndex++ )
   {

      for ( int pieceIndex = 0; pieceIndex < 4; pieceIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                              vPromotionsLeft[ moveIndex ] - 7 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                                 vPromotionsLeft[ moveIndex ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPromotionsLeft[ moveIndex ] - 7;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vPromotionsLeft[ moveIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCaptureAndPromote;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = argsGeneralMoves->vPromotionPieces[ pieceIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iCapture     = argsBoard->mBoard[ vPromotionsLeft[ moveIndex ] ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPromotion;

      }

   }
   		
   // Do that EP thing!
   if ( argsBoard->iMoveHistory > 0 )
   {

      if ( argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare > 0 )
      {
      
         // Do the attacks to the right.
         BitBoard bbAttacksRight = ( argsBoard->bbWhitePawn << 9 );
         bbAttacksRight = bbAttacksRight & ( ~ argsGeneralMoves->bbRowSeven );
         bbAttacksRight = bbAttacksRight & 
                          argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare &
                          ~argsBoard->bbAllPieces;

         // See if we found one.
         if ( bbAttacksRight > 0 )
         {

            int viVector[ 1 ];
            int iSquare = Find( bbAttacksRight, 
                                viVector, 
                                argsGeneralMoves );
            argsBoard->siNumberOfMoves++;
            InitializeMoves( argsMoves,
                             argsBoard->siNumberOfMoves - 1 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare, 
    		                                                                           viVector[ 0 ] - 9 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                                 viVector[ 0 ] );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = viVector[ 0 ] - 9;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = viVector[ 0 ];
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dEnPassant;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhitePawn;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msCaptureSide;

         }

         // Do the attacks to the Left.
         BitBoard bbAttacksLeft = ( argsBoard->bbWhitePawn << 7 );
         bbAttacksLeft = bbAttacksLeft & ( ~ argsGeneralMoves->bbRowZero );
         bbAttacksLeft = bbAttacksLeft & 
                         argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare &
                         ~argsBoard->bbAllPieces;

         // See if we found one.
         if ( bbAttacksLeft > 0 )
         {

            int viVector[ 1 ];
            int iSquare = Find( bbAttacksLeft, viVector, argsGeneralMoves );
            argsBoard->siNumberOfMoves++;
            InitializeMoves( argsMoves,
                             argsBoard->siNumberOfMoves - 1 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare, 
    		                                                                           viVector[ 0 ] - 7 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                                 viVector[ 0 ] );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = viVector[ 0 ] - 7;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = viVector[ 0 ];
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dEnPassant;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhitePawn;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msCaptureSide;

         }

      }

   }

}   

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateWhiteRooks( struct Move * argsMoves, 
                          struct Board * argsBoard, 
                          struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbWhiteRook == 0 )
   {
   
      return;

   }

   // Do the Rooks.
   int numberOfRooks = 0;
   int vWhiteRooks[ 10 ];
   numberOfRooks = Find( argsBoard->bbWhiteRook, 
                         vWhiteRooks, 
                         argsGeneralMoves );

   // Loop over the rooks.
   for ( int rookIndex = 0; rookIndex < numberOfRooks; rookIndex++ )
   {
      
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mRN[ vWhiteRooks[ rookIndex ] ], 
              argsGeneralMoves->vQNOMN[ vWhiteRooks[ rookIndex ] ], 
               vWhiteRooks[ rookIndex ], 
              dWhiteRook);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mRE[ vWhiteRooks[ rookIndex ] ], 
              argsGeneralMoves->vQNOME[ vWhiteRooks[ rookIndex ] ], 
              vWhiteRooks[ rookIndex ], 
              dWhiteRook);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mRS[ vWhiteRooks[ rookIndex ] ], 
              argsGeneralMoves->vQNOMS[ vWhiteRooks[ rookIndex ] ], 
              vWhiteRooks[ rookIndex ], 
              dWhiteRook);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mRW[ vWhiteRooks[ rookIndex ] ], 
              argsGeneralMoves->vQNOMW[ vWhiteRooks[ rookIndex ] ], 
              vWhiteRooks[ rookIndex ], 
              dWhiteRook);

   }
   
}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateWhiteKnights( struct Move * argsMoves, 
                            struct Board * argsBoard, 
                            struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbWhiteKnight == 0 )
   {
   
      return;

   }

   // Do the Knights.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbWhiteKnight, 
                          vPieces, 
                          argsGeneralMoves );

   int vMoves[ 8 ];
   int vAttacks[ 8 ];

   // Loop over the knights.
   for ( int index = 0; index < numberOfPieces; index++ )
   {

      // Look for the good moves..
      BitBoard bbMoves = argsGeneralMoves->bbNMove[ vPieces[ index ] ] &
                         ( ~ argsBoard->bbAllPieces );
      BitBoard bbAttacks = argsGeneralMoves->bbNMove[ vPieces[ index ] ] &
                            (  argsBoard->bbBlackPieces );

      // Enter the moves.
      int siNumberOfMoves   = Find( bbMoves, 
                                       vMoves, 
                                       argsGeneralMoves );
      int numberOfAttacks = Find( bbAttacks, 
                                       vAttacks, 
                                       argsGeneralMoves );
      argsBoard->bbWhiteMove   = argsBoard->bbWhiteMove   | bbMoves;
      argsBoard->bbWhiteAttack = argsBoard->bbWhiteAttack | bbAttacks;

      // Enter the attacks.
      for ( int attackIndex = 0; attackIndex < numberOfAttacks; attackIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
		                                                                           vPieces[ index ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
             			                                                            vAttacks[ attackIndex ] );
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPieces[  index ];
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vAttacks[ attackIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
		   argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhiteKnight;

         // Score the moves.
         switch ( argsBoard->mBoard[  argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare ] ) 
         {

            case dBlackPawn:
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesPawn;
               break;
         
            }
            case dBlackKnight :
            {   
                 
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesKnight;
               break;

            }
            case dBlackBishop :
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesBishop;
               break;
      
            }
            case dBlackRook :
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesRook;
               break;
      
            }
            case dBlackQueen :
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesQueen;
               break;
      
            }
            case dBlackKing :
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
               break;
      
            }

         }

      }

      // Enter the moves.
      for ( int moveIndex = 0; moveIndex < siNumberOfMoves; moveIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
		                                                                           vPieces[ index ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
             			                                                            vMoves[ moveIndex ] );
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPieces[ index ];
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vMoves[ moveIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dRegular;
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhiteKnight;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPiece;

      }

   }

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateWhiteBishops( struct Move * argsMoves, 
                            struct Board * argsBoard, 
                            struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbWhiteBishop == 0 )
   {
   
      return;

   }


   // Do the Bishops.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbWhiteBishop, 
                          vPieces, 
                          argsGeneralMoves );

   // Loop over the Bishops.
   for ( int pieceIndex = 0; pieceIndex < numberOfPieces; pieceIndex++ )
   {
      
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mBNE[    vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMNE[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dWhiteBishop);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mBSE[    vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMSE[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dWhiteBishop);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mBSW[    vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMSW[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dWhiteBishop);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mBNW[    vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMNW[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dWhiteBishop);

   }

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateWhiteQueens(  struct Move * argsMoves, 
                            struct Board * argsBoard, 
                            struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbWhiteQueen == 0 )
   {
   
      return;

   }

   // Do the Queens.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbWhiteQueen, 
                          vPieces, 
                          argsGeneralMoves );

   // Loop over the Queens.
   for ( int pieceIndex = 0; pieceIndex < numberOfPieces; pieceIndex++ )
   {

      
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mQN[    vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMN[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dWhiteQueen);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mQNE[    vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMNE[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dWhiteQueen);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mQE[    vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOME[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dWhiteQueen);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mQSE[    vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMSE[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dWhiteQueen);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mQS[    vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMS[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dWhiteQueen);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mQSW[    vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMSW[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dWhiteQueen);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mQW[    vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMW[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dWhiteQueen);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mQNW[    vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMNW[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dWhiteQueen);

   }

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateWhiteKing( struct Move * argsMoves, 
                         struct Board * argsBoard, 
                         struct GeneralMove * argsGeneralMoves )
{
   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbWhiteKing == 0 )
   {
   
      return;

   }


   // Do the King.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbWhiteKing, 
                          vPieces, 
                          argsGeneralMoves );
   int index = 0;

   int vMoves[ 8 ];
   int vAttacks[ 8 ];

   // Look for the good moves..
   BitBoard bbMoves   = argsGeneralMoves->bbKMove[ vPieces[ index ] ] &
                        ( ~ argsBoard->bbAllPieces );
   BitBoard bbAttacks = argsGeneralMoves->bbKMove[ vPieces[ index ] ] &
                        ( argsBoard->bbBlackPieces );

   argsBoard->bbWhiteMove   = argsBoard->bbWhiteMove   | bbMoves;
   argsBoard->bbWhiteAttack = argsBoard->bbWhiteAttack | bbAttacks;

   // Enter the moves.
   int siNumberOfMoves = Find( bbMoves,   vMoves,   argsGeneralMoves );
   int numberOfAttacks = Find( bbAttacks, vAttacks, argsGeneralMoves );

   // Enter the attacks.
   for ( int attackIndex = 0; attackIndex < numberOfAttacks; attackIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vPieces[ index ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vAttacks[ attackIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPieces[ index ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vAttacks[ attackIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
	   argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhiteKing;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msCaptureDown;

   }

   // Enter the moves.
   for ( int moveIndex = 0; moveIndex < siNumberOfMoves; moveIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vPieces[ index ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vMoves[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPieces[ index ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vMoves[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dRegular;
	   argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhiteKing;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msRegular;
            
   }

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CastleWhite( struct Move        * argsMoves,
                  struct Board       * argsBoard,
                  struct GeneralMove * argsGeneralMoves )
{
//
// This function is used to calculate if the King can castle.
//

   // Debug the inputs.
   assert( argsMoves >= 0       );
   assert( argsBoard >= 0        );
   assert( argsGeneralMoves >= 0 );

   // Check to see what casteling is available.
   int iNumberOfBits = 0;
   int vCastle[ 4 ];

   // Extract the castle state.
   iNumberOfBits = Find( argsBoard->bbCastle, 
                         vCastle, 
                         argsGeneralMoves );

   // Look for the king side castle.
   if ( ( argsBoard->bbCastle & 1 ) &&
        ( ( argsGeneralMoves->bbKingSideWhite & argsBoard->bbAllPieces ) == 0 ) &&
        ( ( argsGeneralMoves->bbKingSideWhite & argsBoard->bbBlackMove ) == 0 ) &&
        ( ( argsBoard->bbWhiteKing & argsBoard->bbBlackAttack )          == 0 ) &&
        ( argsBoard->mBoard[ dH1 ] == dWhiteRook ) )
   {

      // Enter the castle move.
      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
                                                                              dE1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                              dG1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = dE1;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = dG1;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dWhiteKingSideCastle;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhiteKing;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msCastle ;

   }

   // Look for the queen side castle.
   if ( ( ( argsBoard->bbCastle & 4 ) > 0 ) &&
        ( ( argsGeneralMoves->bbQueenSideWhite & argsBoard->bbAllPieces ) == 0 ) &&
        ( ( argsGeneralMoves->bbQueenSideWhite & argsBoard->bbBlackMove ) == 0 ) &&
        ( ( argsBoard->bbWhiteKing & argsBoard->bbBlackAttack )           == 0 ) &&
        ( argsBoard->mBoard[ dA1 ] == dWhiteRook ) )
   {

      // Enter the castle move.
      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
                                                                              dE1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                              dC1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = dE1;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = dC1;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dWhiteQueenSideCastle;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhiteKing;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msCastle;

   }

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateBlackPawns( struct Move        * argsMoves, 
                          struct Board       * argsBoard, 
                          struct GeneralMove * argsGeneralMoves )
{
//
// This function looks at the Black pawn moves.
//

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbBlackPawn == 0 )
   {
   
      return;

   }

   // Do the one step moves.
   BitBoard bbOneStep = ( argsBoard->bbBlackPawn >> 8 ) & ( ~ argsBoard->bbAllPieces );

   // Do the two step moves.
   BitBoard bbElegibleMoves = argsBoard->bbBlackPawn & argsGeneralMoves->bbColSix;
   bbElegibleMoves          = bbElegibleMoves & ( bbOneStep << 8 );  
   BitBoard bbTwoStep         = ( bbElegibleMoves >> 16 ) & ( ~ argsBoard->bbAllPieces );

   // Do the attacks to the right.
   BitBoard bbAttacksRight = ( argsBoard->bbBlackPawn >> 9 );
   bbAttacksRight          = bbAttacksRight & ( ~ argsGeneralMoves->bbRowZero );
   bbAttacksRight          = bbAttacksRight & argsBoard->bbWhitePieces;
   bbAttacksRight          = bbAttacksRight & ( ~ argsBoard->bbBlackPieces );

   //Do the Attacks to the left.
   BitBoard bbAttacksLeft = ( argsBoard->bbBlackPawn >> 7 );
   bbAttacksLeft          = bbAttacksLeft & argsBoard->bbWhitePieces;
   bbAttacksLeft          = bbAttacksLeft & ( ~ argsGeneralMoves->bbRowSeven );
   bbAttacksLeft          = bbAttacksLeft & ( ~ argsBoard->bbBlackPieces );

   // Combine the attacks and moves.
   argsBoard->bbBlackAttack = argsBoard->bbBlackAttack | bbAttacksRight | bbAttacksLeft;
   argsBoard->bbBlackMove   = argsBoard->bbBlackMove   | bbTwoStep      | bbOneStep;

   // Take out the promotion things.
   //Find the promotions
   BitBoard bbPromotionsOne   = ( argsGeneralMoves->bbColZero & bbOneStep );
   BitBoard bbPromotionsRight = ( argsGeneralMoves->bbColZero & bbAttacksRight );
   BitBoard bbPromotionsLeft  = ( argsGeneralMoves->bbColZero & bbAttacksLeft );

   // Remove the pormotions from other moves.
   bbOneStep      = bbOneStep & ~argsGeneralMoves->bbColZero;
   bbAttacksRight = bbAttacksRight & ~argsGeneralMoves->bbColZero;
   bbAttacksLeft  = bbAttacksLeft & ~argsGeneralMoves->bbColZero;

   // Find the moves and attacks.
   int vMovesOneStep[    8 ];
   int vMovesTwoStep[    8 ];
   int vAttacksLeft[     8 ];
   int vAttacksRight[    8 ];
   int vPromotionsOne[   8 ];
   int vPromotionsRight[ 8 ];
   int vPromotionsLeft[  8 ];
   int siNumberOfMovesOneStep  = Find( bbOneStep,         vMovesOneStep,    argsGeneralMoves );
   int siNumberOfMovesTwoStep  = Find( bbTwoStep,         vMovesTwoStep,    argsGeneralMoves );
   int numberOfAttacksRight    = Find( bbAttacksRight,    vAttacksRight,    argsGeneralMoves );
   int numberOfAttacksLeft     = Find( bbAttacksLeft,     vAttacksLeft,     argsGeneralMoves );
   int numberOfPromotionsOne   = Find( bbPromotionsOne,   vPromotionsOne,   argsGeneralMoves );
   int numberOfPromotionsRight = Find( bbPromotionsRight, vPromotionsRight, argsGeneralMoves );
   int numberOfPromotionsLeft  = Find( bbPromotionsLeft,  vPromotionsLeft,  argsGeneralMoves );

   // Enter the one step moves.
   for ( int moveIndex = 0; moveIndex < siNumberOfMovesOneStep; moveIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare, 
	                                                                           vMovesOneStep[ moveIndex ] + 8 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vMovesOneStep[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vMovesOneStep[ moveIndex ] + 8;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vMovesOneStep[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dOneSquare;
	   argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackPawn;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msRegular;

   }

   // Enter the two step moves.
   for ( int moveIndex = 0; moveIndex < siNumberOfMovesTwoStep; moveIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vMovesTwoStep[ moveIndex ] +16 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vMovesTwoStep[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vMovesTwoStep[ moveIndex ] +16;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vMovesTwoStep[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dTwoSquare;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackPawn;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbEPSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbEPSquare,
	                                                                           vMovesTwoStep[ moveIndex ] + 8 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPawnTwo;

   }

   // Enter the right attacks.
   for ( int moveIndex = 0; moveIndex < numberOfAttacksRight; moveIndex++ )
   {
      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vAttacksRight[ moveIndex ] + 9 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vAttacksRight[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vAttacksRight[ moveIndex ] + 9;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vAttacksRight[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
	   argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackPawn;
 
      // Score the moves.
      switch ( argsBoard->mBoard[  argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare ] ) 
      {

         case dWhitePawn:
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesPawn;
            break;
      
         }
         case dWhiteKnight :
         {   
              
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesKnight;
            break;

         }
         case dWhiteBishop :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesBishop;
            break;
   
         }
         case dWhiteRook :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesRook;
            break;
   
         }
         case dWhiteQueen :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesQueen;
            break;
   
         }
         case dWhiteKing :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
            break;
   
         }

      }

   }

   // Enter the left attacks.
   for ( int moveIndex = 0; moveIndex < numberOfAttacksLeft; moveIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vAttacksLeft[ moveIndex ] +7 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vAttacksLeft[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vAttacksLeft[ moveIndex ] + 7;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vAttacksLeft[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
	   argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackPawn;

      // Score the moves.
      switch ( argsBoard->mBoard[  argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare ] ) 
      {

         case dWhitePawn:
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesPawn;
            break;
      
         }
         case dWhiteKnight :
         {   
              
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesKnight;
            break;

         }
         case dWhiteBishop :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesBishop;
            break;
   
         }
         case dWhiteRook :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesRook;
            break;
   
         }
         case dWhiteQueen :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesQueen;
            break;
   
         }
         case dWhiteKing :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
            break;
   
         }

      }

   }

   // Enter the promotions one.
   for ( int moveIndex = 0; moveIndex < numberOfPromotionsOne; moveIndex++ )
   {

      for ( int pieceIndex = 0; pieceIndex < 4; pieceIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );

         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                              vPromotionsOne[ moveIndex ] + 8 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                                 vPromotionsOne[ moveIndex ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPromotionsOne[ moveIndex ] + 8;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vPromotionsOne[ moveIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dPromote;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = argsGeneralMoves->vPromotionPieces[ pieceIndex ] + 6;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPromotion;

      }

   }

   // Enter the promotions right.
   for ( int moveIndex = 0; moveIndex < numberOfPromotionsRight; moveIndex++ )
   {

      for ( int pieceIndex = 0; pieceIndex < 4; pieceIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );

         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                              vPromotionsRight[ moveIndex ] + 9 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                                 vPromotionsRight[ moveIndex ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPromotionsRight[ moveIndex ] + 9;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vPromotionsRight[ moveIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCaptureAndPromote;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = argsGeneralMoves->vPromotionPieces[ pieceIndex ] + 6;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iCapture     = argsBoard->mBoard[ vPromotionsRight[ moveIndex ] ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPromotion;

      }

   }

   // Enter the promotions Left.
   for ( int moveIndex = 0; moveIndex < numberOfPromotionsLeft; moveIndex++ )
   {

      for ( int pieceIndex = 0; pieceIndex < 4; pieceIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );

         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                              vPromotionsLeft[ moveIndex ] + 7 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                                 vPromotionsLeft[ moveIndex ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPromotionsLeft[ moveIndex ] + 7;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vPromotionsLeft[ moveIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCaptureAndPromote;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = argsGeneralMoves->vPromotionPieces[ pieceIndex ] + 6;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iCapture     = argsBoard->mBoard[ vPromotionsLeft[ moveIndex ] ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPromotion;

      }

   }

   // Do that EP thing!
   if ( argsBoard->iMoveHistory > 0 )
   {

      if ( argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare > 0 )
      {
      
         // Do the attacks to the right.
         BitBoard bbAttacksRight = ( argsBoard->bbBlackPawn >> 9 );
         bbAttacksRight = bbAttacksRight & ( ~ argsGeneralMoves->bbRowZero );
         bbAttacksRight = bbAttacksRight & 
                          argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare;
//         bbAttacksRight = bbAttacksRight & 
//                          argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare &
//                          ~argsBoard->bbAllPieces;

         // See if we found one.
         if ( ( bbAttacksRight > 0 ) )
         {

            int viVector[ 1 ];
            int iSquare = Find( bbAttacksRight, 
                                viVector, 
                                argsGeneralMoves );
             
            argsBoard->siNumberOfMoves++;
            InitializeMoves( argsMoves,
                             argsBoard->siNumberOfMoves - 1 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare, 
    		                                                                           viVector[ 0 ] + 9 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                                 viVector[ 0 ] );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = viVector[ 0 ] + 9;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = viVector[ 0 ];
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dEnPassant;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackPawn;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msCaptureSide;

         }

         // Do the attacks to the Left.
         bbAttacksLeft = ( argsBoard->bbBlackPawn >> 7 );
         bbAttacksLeft = bbAttacksLeft & ( ~ argsGeneralMoves->bbRowSeven );
         bbAttacksLeft = bbAttacksLeft & 
                         argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare;
//         bbAttacksLeft = bbAttacksLeft & 
//                         argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare &
//                         ~argsBoard->bbAllPieces;

         // See if we found one.
         if ( ( bbAttacksLeft > 0 ) )
         {

            int viVector[ 1 ];
            int iSquare = Find( bbAttacksLeft, 
                                viVector, 
                                argsGeneralMoves );
             
            argsBoard->siNumberOfMoves++;
            InitializeMoves( argsMoves,
                             argsBoard->siNumberOfMoves - 1 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare, 
    		                                                                           viVector[ 0 ] + 7 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                                 viVector[ 0 ] );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = viVector[ 0 ] + 7;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = viVector[ 0 ];
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dEnPassant;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackPawn;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msCaptureSide;

         }

      }

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateBlackRooks( struct Move * argsMoves, 
                          struct Board * argsBoard, 
                          struct GeneralMove * argsGeneralMoves )
{
   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbBlackRook == 0 )
   {
   
      return;

   }

   // Do the Rooks.
   int numberOfRooks = 0;
   int vBlackRooks[ 10 ];
   numberOfRooks = Find( argsBoard->bbBlackRook, 
                         vBlackRooks, 
                         argsGeneralMoves );

   // Loop over the rooks.
   for ( int rookIndex = 0; rookIndex < numberOfRooks; rookIndex++ )
   {
      
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mRN[ vBlackRooks[ rookIndex ] ], 
              argsGeneralMoves->vQNOMN[ vBlackRooks[ rookIndex ] ], 
              vBlackRooks[ rookIndex ], 
              dBlackRook);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mRE[ vBlackRooks[ rookIndex ] ], 
              argsGeneralMoves->vQNOME[ vBlackRooks[ rookIndex ] ], 
              vBlackRooks[ rookIndex ], 
              dBlackRook);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mRS[ vBlackRooks[ rookIndex ] ], 
              argsGeneralMoves->vQNOMS[ vBlackRooks[ rookIndex ] ], 
              vBlackRooks[ rookIndex ], 
              dBlackRook);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mRW[ vBlackRooks[ rookIndex ] ], 
              argsGeneralMoves->vQNOMW[ vBlackRooks[ rookIndex ] ], 
              vBlackRooks[ rookIndex ], 
              dBlackRook);

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateBlackKnights( struct Move * argsMoves, 
                            struct Board * argsBoard, 
                            struct GeneralMove * argsGeneralMoves )
{
   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbBlackKnight == 0 )
   {
   
      return;

   }

   // Do the Knights.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbBlackKnight, 
                          vPieces, 
                          argsGeneralMoves );

   int vMoves[   8 ];
   int vAttacks[ 8 ];

   // Loop over the knights.
   for ( int index = 0; index < numberOfPieces; index++ )
   {
      // Look for the good moves..
      BitBoard bbMoves = argsGeneralMoves->bbNMove[ vPieces[ index ] ] &
                         ( ~ argsBoard->bbAllPieces );
      BitBoard bbAttacks = argsGeneralMoves->bbNMove[ vPieces[ index ] ] &
                           ( argsBoard->bbWhitePieces );

      argsBoard->bbBlackMove   = argsBoard->bbBlackMove   | bbMoves;
      argsBoard->bbBlackAttack = argsBoard->bbBlackAttack | bbAttacks;

      // Enter the moves.
      int siNumberOfMoves   = Find( bbMoves,  vMoves,    argsGeneralMoves );
      int numberOfAttacks = Find( bbAttacks, vAttacks, argsGeneralMoves );

      // Enter the attacks.
      for ( int attackIndex = 0; attackIndex < numberOfAttacks; attackIndex++ )
      {
          
         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
		                                                                           vPieces[ index ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
             			                                                            vAttacks[ attackIndex ] );
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPieces[ index ];
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vAttacks[ attackIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackKnight;

         // Score the moves.
         switch ( argsBoard->mBoard[  argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare ] ) 
         {

            case dWhitePawn:
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesPawn;
               break;
         
            }
            case dWhiteKnight :
            {   
                 
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesKnight;
               break;

            }
            case dWhiteBishop :
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesBishop;
               break;
      
            }
            case dWhiteRook :
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesRook;
               break;
      
            }
            case dWhiteQueen :
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesQueen;
               break;
      
            }
            case dWhiteKing :
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
               break;
      
            }

         }

      }

      // Enter the moves.
      for ( int moveIndex = 0; moveIndex < siNumberOfMoves; moveIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );
   	   argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
 		                                                                           vPieces[ index ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
      		                                                                     vMoves[ moveIndex ] );
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPieces[ index ];
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vMoves[ moveIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dRegular;
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackKnight;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPiece;

      }

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateBlackBishops( struct Move * argsMoves, 
                            struct Board * argsBoard, 
                            struct GeneralMove * argsGeneralMoves )
{
   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbBlackBishop == 0 )
   {
   
      return;

   }

   // Do the Bishops.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbBlackBishop, 
                          vPieces, 
                          argsGeneralMoves );

   // Loop over the Bishops.
   for ( int pieceIndex = 0; pieceIndex < numberOfPieces; pieceIndex++ )
   {
      
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves, 
              argsGeneralMoves->mBNE[ vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMNE[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dBlackBishop);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves, 
              argsGeneralMoves->mBSE[ vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMSE[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dBlackBishop);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves, 
              argsGeneralMoves->mBSW[ vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMSW[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dBlackBishop);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves, 
              argsGeneralMoves->mBNW[ vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMNW[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dBlackBishop);

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateBlackQueens(  struct Move * argsMoves, 
                            struct Board * argsBoard, 
                            struct GeneralMove * argsGeneralMoves )
{
   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbBlackQueen == 0 )
   {
   
      return;

   }


   // Do the Queens.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbBlackQueen, 
                          vPieces, 
                          argsGeneralMoves );

   // Loop over the Queens.
   for ( int pieceIndex = 0; pieceIndex < numberOfPieces; pieceIndex++ )
   {
      
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mQN[ vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMN[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dBlackQueen);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mQNE[ vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMNE[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dBlackQueen);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mQE[ vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOME[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dBlackQueen);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mQSE[ vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMSE[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dBlackQueen);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mQS[ vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMS[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dBlackQueen);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mQSW[ vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMSW[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dBlackQueen);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mQW[ vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMW[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dBlackQueen);
      Slider( argsMoves, 
              argsBoard, 
              argsGeneralMoves,
              argsGeneralMoves->mQNW[ vPieces[ pieceIndex ] ], 
              argsGeneralMoves->vQNOMNW[ vPieces[ pieceIndex ] ], 
              vPieces[ pieceIndex ], 
              dBlackQueen);

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateBlackKing(    struct Move * argsMoves, 
                            struct Board * argsBoard, 
                            struct GeneralMove * argsGeneralMoves )
{
   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbBlackKing == 0 )
   {
   
      return;

   }

   // Do the King.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbBlackKing, 
                          vPieces, 
                          argsGeneralMoves );
   int index = 0;

   int vMoves[   8 ];
   int vAttacks[ 8 ];

   // Look for the good moves..
   BitBoard bbMoves = argsGeneralMoves->bbKMove[ vPieces[ index ] ] &
                      ( ~ argsBoard->bbAllPieces );
   BitBoard bbAttacks = argsGeneralMoves->bbKMove[ vPieces[ index ] ] &
                        ( argsBoard->bbWhitePieces );


   argsBoard->bbBlackMove   = argsBoard->bbBlackMove   | bbMoves;
   argsBoard->bbBlackAttack = argsBoard->bbBlackAttack | bbAttacks;

   // Enter the moves.
   int siNumberOfMoves = Find( bbMoves,   
                               vMoves,   
                               argsGeneralMoves );
   int numberOfAttacks = Find( bbAttacks, 
                               vAttacks, 
                               argsGeneralMoves );

   // Enter the attacks.
   for ( int attackIndex = 0; attackIndex < numberOfAttacks; attackIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vPieces[ index ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vAttacks[ attackIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPieces[ index ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vAttacks[ attackIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
	   argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackKing;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msCaptureDown;

   }

   // Enter the moves.
   for ( int moveIndex = 0; moveIndex < siNumberOfMoves; moveIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vPieces[ index ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vMoves[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPieces[ index ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vMoves[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dRegular;
	   argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackKing;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPiece;

   }

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CastleBlack( struct Move * argsMoves,
                  struct Board * argsBoard,
                  struct GeneralMove * argsGeneralMoves )
{
//
// This function is used to calculate if the King can castle.
//
   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Check to see what casteling is available.
   int iNumberOfBits = 0;
   int vCastle[ 4 ];

   // Extract the castle state.
   iNumberOfBits = Find( argsBoard->bbCastle, 
                         vCastle, 
                         argsGeneralMoves );

   // Look for the king side castle.
   if ( ( argsBoard->bbCastle & 2 ) &&
        ( ( argsGeneralMoves->bbKingSideBlack & argsBoard->bbAllPieces ) == 0 ) &&
        ( ( argsGeneralMoves->bbKingSideBlack & argsBoard->bbWhiteMove ) == 0 ) &&
        ( ( argsBoard->bbBlackKing & argsBoard->bbWhiteAttack ) == 0 ) &&
        ( argsBoard->mBoard[ dH8 ] == dBlackRook ) )
   {

      // Enter the castle move.
      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
                                                                              dE8 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                              dG8 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = dE8;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = dG8;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dBlackKingSideCastle;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackKing;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msCastle;

   }

   // Look for the queen side castle.
   if ( ( argsBoard->bbCastle & 8                                              ) &&
        ( ( argsGeneralMoves->bbQueenSideBlack & argsBoard->bbAllPieces ) == 0 ) &&
        ( ( argsGeneralMoves->bbQueenSideBlack & argsBoard->bbWhiteMove ) == 0 ) &&
        ( ( argsBoard->bbBlackKing & argsBoard->bbWhiteAttack )           == 0 ) &&
        ( argsBoard->mBoard[ dA8 ] == dBlackRook )                             )
   {

      // Enter the castle move.
      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
                                                                              dE8 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                              dC8 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = dE8;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = dC8;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dBlackQueenSideCastle;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackKing;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msCastle;

   }

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateCheck( struct Move * argsMoves, 
                     struct Board * argsBoard,
                     struct GeneralMove * argsGeneralMoves )
{
   // Check to see if we are in check or checkmate.
   
   // Debugging
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Set the check state to zero.
   argsBoard->bbCheck = 0;

   // Look for white giving check.
   if ( argsBoard->bbWhiteAttack & argsBoard->bbBlackKing )
   {

      // Set the appropriate bit to one.
      argsBoard->bbCheck = SetBitToOne( argsBoard->bbCheck, 1 );

   }

   // Look for black giving check.
   if ( argsBoard->bbBlackAttack & argsBoard->bbWhiteKing )
   {

      // Set the appropriate bit to one.
      argsBoard->bbCheck = SetBitToOne( argsBoard->bbCheck, 0 );

   }

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void PrintMoves( struct Move * argsMoves,
                 int argiNumberOfMoves,
				 	  struct Board * argsBoard)
{
   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argiNumberOfMoves >= 0 );

   // Print out the moves.
   cout << endl;
   cout << "Number of Moves = " << argiNumberOfMoves << endl;
   cout << "Here are the moves " << endl;
   
   for ( int moveIndex = 0; moveIndex < argiNumberOfMoves; moveIndex++ )
   {
      cout << "MoveIndex = " << moveIndex << endl;
      cout << " Piece = " << argsMoves[ moveIndex ].iPiece << endl;
      cout << " FromSquare = " << argsMoves[ moveIndex ].iFromSquare << endl;
      cout << " EndSquare = " << argsMoves[ moveIndex ].iToSquare << endl;
      cout << " MoveType = " << argsMoves[ moveIndex ].iMoveType << endl;

     // Print out the combinded moves.
     BitBoard bbCombinedMoves = argsMoves[ moveIndex ].bbFromSquare | 
                                argsMoves[ moveIndex ].bbToSquare;
     PrintBitBoard( bbCombinedMoves );

    }

}


//
//-----------------------------------------------------------------------
//
void Slider( struct Move * argsMoves, 
             struct Board * argsBoard,
             struct GeneralMove * argsGeneralMoves,
             int * pMoves,
             int numberOfGeneralMoves,
             int startSquare,
             int pieceType )
{

   // Debut the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( pMoves >= 0 );
   assert( numberOfGeneralMoves >= 0 );
   assert( numberOfGeneralMoves <= 64 );
   assert( startSquare >= 0 );
   assert( startSquare <= 64 );
   assert( pieceType >= 0 );
   assert( pieceType <= 32 );

   // Find the slider moves.
   int jumpOut = 1;
   if ( numberOfGeneralMoves <= 0 )
   {

      return;

   }

   int moveIndex = 0;
   while ( jumpOut )
   {

      // Extract the square occupant.
      int squareOccupant = argsBoard->mBoard[ pMoves[ moveIndex ] ];

      // Jump out if the piece is blocked.
      // White piece blocked by a white piece.
      if ( ( ( squareOccupant < dBlackPawn ) &&
             ( squareOccupant > dEmpty ) ) &&
             ( pieceType < dBlackPawn ) )
      {

         return;

      }

      // Jump out if the square is blocked.
      // Black piece blocked by a black piece.
      if ( ( squareOccupant > dWhiteKing ) && ( pieceType > dWhiteKing ) )
      {

         return;

      }

      // Enter a normal move.
      if ( squareOccupant == 0 )
      {

         if ( ( pieceType > 0 ) && ( pieceType < dBlackPawn ) )
         {

            argsBoard->bbWhiteMove = SetBitToOne( argsBoard->bbWhiteMove, pMoves[ moveIndex ] );

         }

         if ( pieceType > dWhiteKing )
         {

            argsBoard->bbBlackMove   = SetBitToOne( argsBoard->bbBlackMove, pMoves[ moveIndex ] );
            argsBoard->bbBlackAttack = argsBoard->bbBlackAttack | argsBoard->bbBlackMove;

         }

         // Enter the move if appropriate.
         if ( ( ( ( ( pieceType > 0 )            && 
                    ( pieceType < dBlackPawn ) ) && 
                    ( argsBoard->siColorToMove == dWhite ) ) ||
                  ( ( pieceType > dWhiteKing )   && 
                    ( argsBoard->siColorToMove == dBlack ) ) ) )
         {

            argsBoard->siNumberOfMoves++;
            InitializeMoves( argsMoves,
                             argsBoard->siNumberOfMoves - 1 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
		                                                                              startSquare );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
		                                                                              pMoves[ moveIndex ] );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = startSquare;
	         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = pMoves[ moveIndex ];
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dRegular;
	         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = pieceType;
	         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPiece;

         }

         // Increment the move indes.
         moveIndex++;

      }

      // Enter a White capture.
      if ( ( squareOccupant > dWhiteKing ) &&
         ( ( pieceType > 0 )               && 
           ( pieceType < dBlackPawn ) ) )
      {

         argsBoard->bbWhiteAttack = SetBitToOne( argsBoard->bbWhiteAttack, pMoves[ moveIndex ] );

         // Enter the move if appropriate.
         if ( ( argsBoard->siColorToMove == dWhite ) )
         {

            argsBoard->siNumberOfMoves++;
            InitializeMoves( argsMoves,
                             argsBoard->siNumberOfMoves - 1 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
		                                                                              startSquare );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
		                                                                              pMoves[ moveIndex ] );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = startSquare;
	         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = pMoves[ moveIndex ];
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
	         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = pieceType;

            // Score the move
            switch( pieceType )
            {
            
               case dWhiteBishop : 
               {

                  switch( argsBoard->mBoard[ pMoves[ moveIndex ] ] )
                  {

                     case dBlackPawn :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesPawn;
                        break;

                     }
                     case dBlackKnight :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesKnight;
                        break;

                     }
                     case dBlackBishop :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesBishop;
                        break;

                     }
                     case dBlackRook :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesRook;
                        break;

                     }
                     case dBlackQueen :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesQueen;
                        break;

                     }
                     case dBlackKing :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
                        break;

                     }
                     default :
                     {

                        cout << "Here's the rub." << endl;
                        break;

                     }

                  }
                  
                  break;

               }
               case dWhiteRook : 
               {
               
                  switch( argsBoard->mBoard[ pMoves[ moveIndex ] ] )
                  {

                     case dBlackPawn :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesPawn;
                        break;

                     }
                     case dBlackKnight :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesKnight;
                        break;

                     }
                     case dBlackBishop :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesBishop;
                        break;

                     }
                     case dBlackRook :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesRook;
                        break;

                     }
                     case dBlackQueen :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesQueen;
                        break;

                     }
                     case dBlackKing :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
                        break;

                     }
                     default :
                     {

                        cout << "Here's the rub." << endl;
                        break;

                     }

                  }

                  break;
                  
               }
               case dWhiteQueen : 
               {
               
                  switch( argsBoard->mBoard[ pMoves[ moveIndex ] ] )
                  {

                     case dBlackPawn :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesPawn;
                        break;

                     }
                     case dBlackKnight :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesKnight;
                        break;

                     }
                     case dBlackBishop :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesBishop;
                        break;

                     }
                     case dBlackRook :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesRook;
                        break;

                     }
                     case dBlackQueen :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesQueen;
                        break;

                     }
                     case dBlackKing :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
                        break;

                     }
                     default :
                     {

                        cout << "Here's the rub." << endl;
                        break;

                     }

                  }

                  break;
                  
               }
               default :
               {

                  cout << "Here's the rub." << endl;
                  break;
                  
               }

            }

         }

         return;

      }

      // Enter a black capture.
      if ( ( ( squareOccupant > 0 )            &&
             ( squareOccupant < dBlackPawn ) ) && 
             ( pieceType > dWhiteKing ) )
      {

         argsBoard->bbBlackAttack = SetBitToOne( argsBoard->bbBlackAttack, pMoves[ moveIndex ] );

         // Enter the move if appropriate.
         if ( ( argsBoard->siColorToMove == dBlack ) )
         {

            argsBoard->siNumberOfMoves++;
            InitializeMoves( argsMoves,
                             argsBoard->siNumberOfMoves - 1 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
		                                                                              startSquare );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
		                                                                              pMoves[ moveIndex ] );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = startSquare;
	         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = pMoves[ moveIndex ];
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
	         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = pieceType;

            // Score the move
            switch( pieceType )
            {
            
               case dBlackBishop : 
               {

                  switch( argsBoard->mBoard[ pMoves[ moveIndex ] ] )
                  {

                     case dWhitePawn :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesPawn;
                        break;

                     }
                     case dWhiteKnight :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesKnight;
                        break;

                     }
                     case dWhiteBishop :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesBishop;
                        break;

                     }
                     case dWhiteRook :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesRook;
                        break;

                     }
                     case dWhiteQueen :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesQueen;
                        break;

                     }
                     case dWhiteKing :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
                        break;

                     }
                     default :
                     {

                        cout << "Here's the rub." << endl;
                        break;

                     }

                  }
                  
                  break;

               }
               case dBlackRook : 
               {
               
                  switch( argsBoard->mBoard[ pMoves[ moveIndex ] ] )
                  {

                     case dWhitePawn :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesPawn;
                        break;

                     }
                     case dWhiteKnight :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesKnight;
                        break;

                     }
                     case dWhiteBishop :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesBishop;
                        break;

                     }
                     case dWhiteRook :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesRook;
                        break;

                     }
                     case dWhiteQueen :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesQueen;
                        break;

                     }
                     case dWhiteKing :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
                        break;

                     }
                     default :
                     {

                        cout << "Here's the rub." << endl;
                        break;

                     }

                  }

                  break;
                  
               }
               case dBlackQueen : 
               {
               
                  switch( argsBoard->mBoard[ pMoves[ moveIndex ] ] )
                  {

                     case dWhitePawn :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesPawn;
                        break;

                     }
                     case dWhiteKnight :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesKnight;
                        break;

                     }
                     case dWhiteBishop :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesBishop;
                        break;

                     }
                     case dWhiteRook :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesRook;
                        break;

                     }
                     case dWhiteQueen :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesQueen;
                        break;

                     }
                     case dWhiteKing :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
                        break;

                     }
                     default :
                     {

                        cout << "Here's the rub." << endl;
                        break;

                     }

                  }

                  break;
                  
               }
               default :
               {

                  cout << "Here's the rub." << endl;
                  break;
                  
               }

            }

         }

         return;

      }
         
      // Look for the end of the line.
      if ( moveIndex == numberOfGeneralMoves )
      {

         return;

      }

   }

}

//
//-----------------------------------------------------------------------
//
void SliderAttacks( struct Move * argsMoves, 
                    struct Board * argsBoard,
                    struct GeneralMove * argsGeneralMoves,
                    int * pMoves,
                    int numberOfGeneralMoves,
                    int startSquare,
                    int pieceType )
{

   // Debut the inputs.
   assert( argsMoves            >=  0 );
   assert( argsBoard            >=  0 );
   assert( pMoves               >=  0 );
   assert( numberOfGeneralMoves >=  0 );
   assert( numberOfGeneralMoves <= 64 );
   assert( startSquare          >=  0 );
   assert( startSquare          <= 64 );
   assert( pieceType            >=  0 );
   assert( pieceType            <= 32 );

   // Find the slider moves.
   int jumpOut = 1;
   if ( numberOfGeneralMoves <= 0 )
   {

      return;

   }

   int moveIndex = 0;
   while ( jumpOut )
   {

      // Extract the square occupant.
      int squareOccupant = argsBoard->mBoard[ pMoves[ moveIndex ] ];

      // Jump out if the piece is blocked.
      // White piece blocked by a white piece.
      if ( ( ( squareOccupant < dBlackPawn ) &&
             ( squareOccupant > dEmpty ) ) &&
             ( pieceType < dBlackPawn ) )
      {

         return;

      }

      // Jump out if the square is blocked.
      // Black piece blocked by a black piece.
      if ( ( squareOccupant > dWhiteKing ) && 
           ( pieceType > dWhiteKing ) )
      {

         return;

      }

      // Enter a White capture.
      if ( ( squareOccupant > dWhiteKing ) && 
           ( ( pieceType > 0 )             && 
             ( pieceType < dBlackPawn ) ) )
      {

         argsBoard->bbWhiteAttack = SetBitToOne( argsBoard->bbWhiteAttack, pMoves[ moveIndex ] );

         // Enter the move if appropriate.
         if ( ( argsBoard->siColorToMove == dWhite ) )
         {

            argsBoard->siNumberOfMoves++;
            InitializeMoves( argsMoves,
                             argsBoard->siNumberOfMoves - 1 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
		                                                                              startSquare );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
		                                                                              pMoves[ moveIndex ] );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = startSquare;
	         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = pMoves[ moveIndex ];
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
	         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = pieceType;

            // Score the move
            switch( pieceType )
            {
            
               case dWhiteBishop : 
               {

                  switch( argsBoard->mBoard[ pMoves[ moveIndex ] ] )
                  {

                     case dBlackPawn :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesPawn;
                        break;

                     }
                     case dBlackKnight :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesKnight;
                        break;

                     }
                     case dBlackBishop :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesBishop;
                        break;

                     }
                     case dBlackRook :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesRook;
                        break;

                     }
                     case dBlackQueen :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesQueen;
                        break;

                     }
                     case dBlackKing :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
                        break;

                     }
                     default :
                     {

                        cout << "Here's the rub." << endl;
                        break;

                     }

                  }
                  
                  break;

               }
               case dWhiteRook : 
               {
               
                  switch( argsBoard->mBoard[ pMoves[ moveIndex ] ] )
                  {

                     case dBlackPawn :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesPawn;
                        break;

                     }
                     case dBlackKnight :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesKnight;
                        break;

                     }
                     case dBlackBishop :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesBishop;
                        break;

                     }
                     case dBlackRook :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesRook;
                        break;

                     }
                     case dBlackQueen :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesQueen;
                        break;

                     }
                     case dBlackKing :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
                        break;

                     }
                     default :
                     {

                        cout << "Here's the rub." << endl;
                        break;

                     }

                  }

                  break;
                  
               }
               case dWhiteQueen : 
               {
               
                  switch( argsBoard->mBoard[ pMoves[ moveIndex ] ] )
                  {

                     case dBlackPawn :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesPawn;
                        break;

                     }
                     case dBlackKnight :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesKnight;
                        break;

                     }
                     case dBlackBishop :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesBishop;
                        break;

                     }
                     case dBlackRook :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesRook;
                        break;

                     }
                     case dBlackQueen :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesQueen;
                        break;

                     }
                     case dBlackKing :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
                        break;

                     }
                     default :
                     {

                        cout << "Here's the rub." << endl;
                        break;

                     }

                  }

                  break;
                  
               }
               default :
               {

                  cout << "Here's the rub." << endl;
                  break;
                  
               }

            }

         }

         return;

      }

      // Enter a black capture.
      if ( ( ( squareOccupant > 0 )            && 
             ( squareOccupant < dBlackPawn ) ) && 
             ( pieceType > dWhiteKing ) )
      {

         argsBoard->bbBlackAttack = SetBitToOne( argsBoard->bbBlackAttack, pMoves[ moveIndex ] );

         // Enter the move if appropriate.
         if ( ( argsBoard->siColorToMove == dBlack ) )
         {

            argsBoard->siNumberOfMoves++;
            InitializeMoves( argsMoves,
                             argsBoard->siNumberOfMoves - 1 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
		                                                                              startSquare );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
		                                                                              pMoves[ moveIndex ] );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = startSquare;
	         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = pMoves[ moveIndex ];
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
	         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = pieceType;

            // Score the move
            switch( pieceType )
            {
            
               case dBlackBishop : 
               {

                  switch( argsBoard->mBoard[ pMoves[ moveIndex ] ] )
                  {

                     case dWhitePawn :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesPawn;
                        break;

                     }
                     case dWhiteKnight :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesKnight;
                        break;

                     }
                     case dWhiteBishop :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesBishop;
                        break;

                     }
                     case dWhiteRook :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesRook;
                        break;

                     }
                     case dWhiteQueen :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msBishopTakesQueen;
                        break;

                     }
                     case dWhiteKing :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
                        break;

                     }
                     default :
                     {

                        cout << "Here's the rub." << endl;
                        break;

                     }

                  }
                  
                  break;

               }
               case dBlackRook : 
               {
               
                  switch( argsBoard->mBoard[ pMoves[ moveIndex ] ] )
                  {

                     case dWhitePawn :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesPawn;
                        break;

                     }
                     case dWhiteKnight :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesKnight;
                        break;

                     }
                     case dWhiteBishop :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesBishop;
                        break;

                     }
                     case dWhiteRook :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesRook;
                        break;

                     }
                     case dWhiteQueen :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msRookTakesQueen;
                        break;

                     }
                     case dWhiteKing :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
                        break;

                     }
                     default :
                     {

                        cout << "Here's the rub." << endl;
                        break;

                     }

                  }

                  break;
                  
               }
               case dBlackQueen : 
               {
               
                  switch( argsBoard->mBoard[ pMoves[ moveIndex ] ] )
                  {

                     case dWhitePawn :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesPawn;
                        break;

                     }
                     case dWhiteKnight :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesKnight;
                        break;

                     }
                     case dWhiteBishop :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesBishop;
                        break;

                     }
                     case dWhiteRook :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesRook;
                        break;

                     }
                     case dWhiteQueen :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msQueenTakesQueen;
                        break;

                     }
                     case dWhiteKing :
                     {

                        argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
                        break;

                     }
                     default :
                     {

                        cout << "Here's the rub." << endl;
                        break;

                     }

                  }

                  break;
                  
               }
               default :
               {

                  cout << "Here's the rub." << endl;
                  break;
                  
               }

            }

         }

         return;

      }

      // Increment the move index.
      moveIndex++;

      // Look for the end of the line.
      if ( moveIndex == numberOfGeneralMoves )
      {

         return;

      }

   }

}

//
//----------------------------------------------------------------------------------------------------------------------------------------------------
//
void InitializeMoves( struct Move * argsMoveList,
                      int iMoveIndex )
{
// This function initializes the move variables to zero.

   // Debug the inputs.
   assert( argsMoveList >= 0 );
   assert( iMoveIndex >= 0 );
   assert( iMoveIndex <= dNumberOfMoves );

   argsMoveList[ iMoveIndex ].bbFromSquare    = 0;
   argsMoveList[ iMoveIndex ].bbToSquare      = 0;
   argsMoveList[ iMoveIndex ].bbEPSquare      = 0;
   argsMoveList[ iMoveIndex ].iPiece          = 0;
   argsMoveList[ iMoveIndex ].iFromSquare     = 0;
   argsMoveList[ iMoveIndex ].iToSquare       = 0;
   argsMoveList[ iMoveIndex ].iPromote        = 0;
   argsMoveList[ iMoveIndex ].iMoveType       = 0;
   argsMoveList[ iMoveIndex ].bbCastle        = 0;
   argsMoveList[ iMoveIndex ].iCapture        = 0;
   argsMoveList[ iMoveIndex ].iScore          = 0;

}

//
//-----------------------------------------------------------------------
//
void SliderQuiets( struct Move * argsMoves, 
                   struct Board * argsBoard,
                   struct GeneralMove * argsGeneralMoves,
                   int * pMoves,
                   int numberOfGeneralMoves,
                   int startSquare,
                   int pieceType )
{

   // Debut the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( pMoves >= 0 );
   assert( numberOfGeneralMoves >= 0 );
   assert( numberOfGeneralMoves <= 64 );
   assert( startSquare >= 0 );
   assert( startSquare <= 64 );
   assert( pieceType >= 0 );
   assert( pieceType <= 32 );

   // Find the slider moves.
   int jumpOut = 1;
   if ( numberOfGeneralMoves <= 0 )
   {

      return;

   }

   int moveIndex = 0;
   while ( jumpOut )
   {

      // Extract the square occupant.
      int squareOccupant = argsBoard->mBoard[ pMoves[ moveIndex ] ];

      // Jump out if the square is not empty or off the board.
      if ( squareOccupant != 0 )
      {
         
         return;

      }
      
      // Make sure we are not off the board.
      //if ( 

/*
      // Extract the square occupant.
      int squareOccupant = argsBoard->mBoard[ pMoves[ moveIndex ] ];

      // Jump out if the piece is blocked.
      // White piece blocked by a white piece.
      if ( ( ( squareOccupant < dBlackPawn ) &&
             ( squareOccupant > dEmpty ) ) &&
             ( pieceType < dBlackPawn ) )
      {

         return;

      }

      // Jump out if the square is blocked.
      // Black piece blocked by a black piece.
      if ( ( squareOccupant > dWhiteKing ) && ( pieceType > dWhiteKing ) )
      {

         return;

      }
//*/

      // Enter a normal move.
      if ( squareOccupant == 0 )
      {

         if ( ( pieceType > 0 ) && ( pieceType < dBlackPawn ) )
         {

            argsBoard->bbWhiteMove = SetBitToOne( argsBoard->bbWhiteMove, pMoves[ moveIndex ] );

         }

         if ( pieceType > dWhiteKing )
         {

            argsBoard->bbBlackMove   = SetBitToOne( argsBoard->bbBlackMove, pMoves[ moveIndex ] );
            argsBoard->bbBlackAttack = argsBoard->bbBlackAttack | argsBoard->bbBlackMove;

         }

         // Enter the move if appropriate.
         if ( ( ( ( ( pieceType > 0 )            && 
                    ( pieceType < dBlackPawn ) ) && 
                    ( argsBoard->siColorToMove == dWhite ) ) ||
                  ( ( pieceType > dWhiteKing )   && 
                    ( argsBoard->siColorToMove == dBlack ) ) ) )
         {

            argsBoard->siNumberOfMoves++;
            InitializeMoves( argsMoves,
                             argsBoard->siNumberOfMoves - 1 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
		                                                                              startSquare );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
		                                                                              pMoves[ moveIndex ] );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = startSquare;
	         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = pMoves[ moveIndex ];
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dRegular;
	         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = pieceType;
	         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPiece;

         }

         // Increment the move indes.
         moveIndex++;

      }
         
      // Look for the end of the line.
      if ( moveIndex == numberOfGeneralMoves )
      {

         return;

      }

   }

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void InputUserMove( struct Board * argsBoard,
                    struct GeneralMove * argsGeneralMoves )
{

   // Debugging the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

// This function assume a notation of the form e2e4.
//
   int iMoveIndex;
   int iMoveNumber;
   char strCommand[ 10 ];
   int iInputFlag = 1;
   int iNumberOfMoves;
   char strMove[ 10 ];
   char strUniverseOfMoves[ dNumberOfMoves][ 10 ];
   int iNumberOfCharacters;
   int iNumberInUniverse;
   int iCharacterIndex;
   int iFoundMove;
   int iStringLength;
   int iMatch;
   int viLegalMove[ dNumberOfMoves ];

   // Initialize the number of characters.
   iNumberOfCharacters = 0;

   // Create moves and pseudo moves.
   Move vsMoveList[      dNumberOfMoves ];
   Move vsDummyMoveList[ dNumberOfMoves ];

   CalculateMoves( vsMoveList, 
                   argsBoard, 
                   argsGeneralMoves );
                   
   iNumberOfMoves = argsBoard->siNumberOfMoves;
   
   // If in book, write out the stats.
   if ( GetIsInBook() == dYes )
   {

      if ( GetUseOpeningBook() )
      {

         PrintOpeningBookMoveStatistics( argsBoard,
                                         argsGeneralMoves );

      }

   }

   // Loop over the moves and store the appropriate moves in strings.
   iNumberInUniverse = 0;
   for ( iMoveNumber = 0; iMoveNumber < iNumberOfMoves; iMoveNumber++ )
   {


      // Check to see if the move is legal.
      MakeMove( vsMoveList,
                argsBoard,
                argsGeneralMoves,
                iMoveNumber );

      // Generate all of the legal moves.
      CalculateMoves( vsDummyMoveList, 
                      argsBoard, 
                      argsGeneralMoves );

      // Check the state of the board.
      LegalState( argsBoard,
                  argsGeneralMoves );

      // Record the legality.
      viLegalMove[ iMoveNumber ] = argsBoard->siLegalMove;

      // Undo the move.
      UndoMove( argsBoard,
                argsGeneralMoves );

      // Clear the string.
      strcpy( strMove, "         " );
         
      // Print out the move.
      iNumberOfCharacters = PrintMove( argsBoard,
                                       argsGeneralMoves,
                                       & vsMoveList[ iMoveNumber ],
                                       strMove );

      // Put the move in storage.
      for ( iCharacterIndex = 0; iCharacterIndex < iNumberOfCharacters; iCharacterIndex++ )
      {
   
         strncpy( & strUniverseOfMoves[ iMoveNumber ][ iCharacterIndex ], & strMove[ iCharacterIndex ], 1 );
         
      }
      
   }
      
   // Get the user input.                
   while( iInputFlag )
   {

      if ( GetInterfaceMode() == dConsole )
      {

         // Write out the potential moves to the screen.
         cout << endl;

         // Input the user move.
         cout << "Input move = ";

      }

      cin >> strMove;

      // See if the input was a numeric character.
      if ( cin.fail() )
      {
      
         // Clear the input.
         cin.clear();
         char c[ 64 ];
         cin >> c;


      }

      // See if we should quit.
      if ( !strcmp( strMove, "quit" ) )
      {

         Destroy();
         exit( 1 );

      }
      
      // See if the move is in the move list.
      iFoundMove = 0;
      iStringLength = strlen( strMove );
      for ( iMoveNumber = 0; iMoveNumber < iNumberOfMoves; iMoveNumber++ )
      {
      
         // Loop over the length of the string.
         iMatch = 1;

         // Only look at legal moves.
         if ( viLegalMove[ iMoveNumber ] == 1 )
         {

            for ( iCharacterIndex = 0; iCharacterIndex < iStringLength; iCharacterIndex++ )
            {
            
               // Look for a mismatch. 
               if ( strncmp( & strMove[ iCharacterIndex ], 
                             & strUniverseOfMoves[ iMoveNumber ][ iCharacterIndex ],
                             1 ) )          
               {
                  
                  // Assign the mismatch.
                  iMatch = 0;
                  break;
            
               }
            
            }

         }
         else
         {
            
            // In this case we have an illegal move and there is not a match.
            iMatch = 0;

         }
         
         // Look for a perfect match.
         if ( iMatch == 1 )
         {
         
            iFoundMove = 1;
            iMoveIndex = iMoveNumber;
            break;
            
         }
         
      }
      
      // If the move was not found, try to get another.
      if ( iFoundMove == 0 )
      {
      
         if ( GetInterfaceMode() == dConsole )
         {

            cout << "Move was not found, try again and look for degeneracy." << endl;

         }

         InputUserMove( argsBoard,
                        argsGeneralMoves );
         return;
            
      }

      // If the move was found make it move on with life.
      MakeMove( vsMoveList, 
                argsBoard,
                argsGeneralMoves,
                iMoveNumber );

      if ( GetInterfaceMode() == dConsole )
      {

         PrintBoard( argsBoard->mBoard );

         cout << "Confirm move (y/n) = ";

      }

      cin >> strCommand;

      if ( GetInterfaceMode() == dConsole )
      {

         cout << endl;

      }

      if ( !strncmp( strCommand, "y", 1 ) ||
           !strncmp( strCommand, "Y", 1 ) )
      {

         iInputFlag = 0;

      }
      else
      {

         UndoMove( argsBoard,
                   argsGeneralMoves );

         CalculateMoves( vsMoveList, 
                         argsBoard, 
                         argsGeneralMoves );
                         
         if ( GetInterfaceMode() == dConsole )
         {

            PrintBoard( argsBoard->mBoard );

         }

      }
      
   }

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
int PrintMove( struct Board * argsBoard,
               struct GeneralMove * argsGeneralMoves,
               Move   * argsMove,
               char   * argstrMove )
{

   // Debug the inputs.
   assert( argsBoard        >= 0 );
   assert( argsGeneralMoves >= 0 );
   assert( argsMove         >= 0 );
   assert( argstrMove       >= 0 );

// Write out a move in algebraic notation.
//

   int iRowFrom;
   int iColFrom;
   int iRowTo;
   int iColTo;
   int iPieceFrom;
   int iPieceTo;
   char strStartSquareCol[ 2 ];
   char strStartSquareRow[ 2 ];
   char strEndSquareCol[   2 ];
   char strEndSquareRow[   2 ];
   Move vsMoveList[      dNumberOfMoves ];
   Move vsMoveList2[     dNumberOfMoves ];
   Move vsMoveListDummy[ dNumberOfMoves ];
   int viLegalMove[      dNumberOfMoves ];
   int iDegeneracy;
   int iRowDegeneracy;
   int iColDegeneracy;
   int iNumberOfCharacters = 0;
   int siNumberOfMoves     = 0;
   int iMoveCount;
   int iMoveCountDummy;

   // Keep an old board around for scoring.
   struct Board * sBoard;
   sBoard = (Board * ) malloc( 1 * sizeof( Board ) );
   * sBoard = * argsBoard;

   // Clear the string.
   * ( argstrMove ) = 0;
   //strncpy( argstrMove, " ", 1 );         

   // Assume no degeneracy.
   iDegeneracy    = 0;
   iRowDegeneracy = 0;
   iColDegeneracy = 0;
   
   // Generate all of the legal moves.
   //PrintBoard( sBoard->mBoard );
   CalculateMoves( vsMoveList, 
                   sBoard, 
                   argsGeneralMoves );

   // Extract the number of moves to live locally.
   siNumberOfMoves = sBoard->siNumberOfMoves;

   // Check to see if the moves are legal.
   for ( iMoveCount = 0; iMoveCount < siNumberOfMoves; iMoveCount++ )
   {

      MakeMove( vsMoveList,
                sBoard,
                argsGeneralMoves,
                iMoveCount );

      CalculateMoves( vsMoveListDummy,
                      sBoard,
                      argsGeneralMoves );

      LegalState( sBoard,
                  argsGeneralMoves );

      viLegalMove[ iMoveCount ] = sBoard->siLegalMove;

      if ( sBoard->siLegalMove == 0 )
      {

         // Remove the move from the list.
         for ( iMoveCountDummy = iMoveCount; iMoveCountDummy < siNumberOfMoves - 1; iMoveCountDummy++ )
         {

            // Move the mooves down one count.
            vsMoveList[ iMoveCountDummy ] = vsMoveList[ iMoveCountDummy + 1 ];

         }

         // Decrement the number of moves.
         siNumberOfMoves--;

         // Decrement the counter.
         iMoveCount--;
         
      }

      UndoMove( sBoard,
                argsGeneralMoves );

   }
   
   // Reset the board.
   * sBoard = * argsBoard;

   // Calculate the from row and column.
   //PrintBoard( sBoard->mBoard );
   iRowFrom          = dRow( argsMove->iFromSquare );
   iColFrom          = dCol( argsMove->iFromSquare );
   iPieceFrom        = sBoard->mBoard[ argsMove->iFromSquare ];
   ConvertIntToRow( iRowFrom, 
                    & strStartSquareRow[ 0 ] );
   ConvertIntToCol( iColFrom, 
                    & strStartSquareCol[ 0 ] );

   // Calculate the to row and column.
   iRowTo          = dRow( argsMove->iToSquare );
   iColTo          = dCol( argsMove->iToSquare );
   iPieceTo        = sBoard->mBoard[ argsMove->iToSquare ];
   ConvertIntToRow( iRowTo, 
                    & strEndSquareRow[ 0 ] );
   ConvertIntToCol( iColTo, 
                    & strEndSquareCol[ 0 ] );

   // Calculate the attacks and see if casteling is allowed.
   CalculateAttacks( vsMoveList2,
                     sBoard,
                     argsGeneralMoves );
      
   // Look for a degeneracy.
   for ( int iMoveIndex = 0; iMoveIndex < siNumberOfMoves; iMoveIndex++ )
   {
      
      // Look for landing on the same square
      // Make sure it is not from a promotion
      if ( ( vsMoveList[ iMoveIndex ].iToSquare     == argsMove->iToSquare   ) &&
           ( vsMoveList[ iMoveIndex ].iFromSquare   != argsMove->iFromSquare ) &&
           ( vsMoveList[ iMoveIndex ].iPiece        == argsMove->iPiece      ) && 
           ( ( ( argsMove->iMoveType                == dPromote              ) ||
               ( argsMove->iMoveType                == dCaptureAndPromote    ) ||
               ( vsMoveList[ iMoveIndex ].iMoveType == dPromote              ) ||
               ( vsMoveList[ iMoveIndex ].iMoveType == dCaptureAndPromote ) )  == 0 ) )
      {	
      
         // Show that the move is degenerate.
         iDegeneracy = 1;
         
         // Look for column and row degeneracy.
         if ( iRowFrom == dRow( vsMoveList[ iMoveIndex ].iFromSquare ) )
         {
         
            // Set a degenerate row.
            iRowDegeneracy = 1;
            
         }
         if ( iColFrom == dCol( vsMoveList[ iMoveIndex ].iFromSquare ) )
         {
         
            // Set a degenerate row.
            iColDegeneracy = 1;
            
         }
         // Look for a complete mismatch.
         if ( ( iColDegeneracy == 0 ) &&
              ( iRowDegeneracy == 0 ) )
         {
         
            // The row takes precidence over the column 
            iColDegeneracy = 1;
            
         }
         
                                              
      }
      
   }
   
   // Write out a king side castle.
   if ( ( ( argsMove->iMoveType == dWhiteKingSideCastle ) &&
          ( sBoard->bbCheck == 0 ) )                   ||
        ( ( argsMove->iMoveType == dBlackKingSideCastle ) &&
          ( sBoard->bbCheck == 0 ) ) )
   {
   
      
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "O", 1 );
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "-", 1 );
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "O", 1 );      
      
   }

   // Write out a queen side castle.
   if ( ( ( argsMove->iMoveType == dWhiteQueenSideCastle ) &&
          ( sBoard->bbCheck == 0 ) )                    ||
        ( ( argsMove->iMoveType == dBlackQueenSideCastle ) &&
          ( sBoard->bbCheck == 0 ) ) )
   {

      
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "O", 1 );      
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "-", 1 );            
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "O", 1 );            
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "-", 1 );            
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "O", 1 );
      
   }

   // Write out a rook move
   if ( ( iPieceFrom == dWhiteRook ) ||
        ( iPieceFrom == dBlackRook ) )
   {
   
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "R", 1 );
      
   }

   // Write out a knight move.
   if ( ( iPieceFrom == dWhiteKnight ) ||
        ( iPieceFrom == dBlackKnight ) )
   {
   
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "N", 1 );
      
   }
   if ( ( iPieceFrom == dWhiteBishop ) ||
        ( iPieceFrom == dBlackBishop ) )
   {
   
      
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "B", 1 );      
      
   }
   if ( ( iPieceFrom == dWhiteQueen ) ||
        ( iPieceFrom == dBlackQueen ) )
   {
   
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "Q", 1 );
      
   }
   if ( ( iPieceFrom == dWhiteKing ) ||
        ( iPieceFrom == dBlackKing ) )
   {
      
      if ( ( argsMove->iMoveType != dWhiteKingSideCastle ) &&
           ( argsMove->iMoveType != dBlackKingSideCastle ) &&
           ( argsMove->iMoveType != dWhiteQueenSideCastle ) &&
           ( argsMove->iMoveType != dBlackQueenSideCastle )  )
      {
         
         strncpy( & argstrMove[ iNumberOfCharacters++ ], "K", 1 );

      }
      
   }
   
   // Write out any degenerate information.
   if ( ( argsMove->iPiece != dWhitePawn ) &&
        ( argsMove->iPiece != dBlackPawn ) )
   {
      
      if ( iRowDegeneracy == 1 )
      {
   
         strncpy( & argstrMove[ iNumberOfCharacters++ ], & strStartSquareCol[ 0 ], 1 );
         
      }
      if ( iColDegeneracy == 1 )
      {
   
         strncpy( & argstrMove[ iNumberOfCharacters++ ], & strStartSquareRow[ 0 ], 1 );
         
      }
      
   }
      
   // Look for a capture.
   if ( ( argsMove->iMoveType == dCapture   ) ||
        ( argsMove->iMoveType == dEnPassant ) ||
        ( argsMove->iMoveType == dCaptureAndPromote ) )
   {
   
      if ( ( iPieceFrom == dWhitePawn ) ||
           ( iPieceFrom == dBlackPawn ) )
      {
            
         strncpy( & argstrMove[ iNumberOfCharacters++ ], & strStartSquareRow[ 0 ], 1 );
         
      }
      
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "x", 1 );
      
   }

   // Write out the final square if required.
   if ( ( argsMove->iMoveType == dCapture )           ||
        ( argsMove->iMoveType == dCaptureAndPromote ) ||
        ( argsMove->iMoveType == dEnPassant )         ||
        ( argsMove->iMoveType == dRegular )           ||
        ( argsMove->iMoveType == dTwoSquare )         ||
        ( argsMove->iMoveType == dOneSquare )         ||
        ( argsMove->iMoveType == dPromote ) )
   {
   
      // Write out the final square.
      strncpy( &argstrMove[ iNumberOfCharacters++ ], &strEndSquareRow[ 0 ], 1 );
      strncpy( &argstrMove[ iNumberOfCharacters++ ], &strEndSquareCol[ 0 ], 1 );
       
   }
   
   // Take care of promotions.
   if ( ( argsMove->iMoveType == dPromote ) ||
        ( argsMove->iMoveType == dCaptureAndPromote ) )
   {
      
     strncpy( & argstrMove[ iNumberOfCharacters++ ], "=", 1 );
      
      if ( ( argsMove->iPiece == dWhiteQueen ) ||
           ( argsMove->iPiece == dBlackQueen ) )
      {
      
         strncpy( & argstrMove[ iNumberOfCharacters++ ], "Q", 1 );
         
      } 
      if ( ( argsMove->iPiece == dWhiteBishop ) ||
           ( argsMove->iPiece == dBlackBishop ) )
      {
      
         strncpy( & argstrMove[ iNumberOfCharacters++ ], "B", 1 );
         
      } 
      if ( ( argsMove->iPiece == dWhiteKnight ) ||
           ( argsMove->iPiece == dBlackKnight ) )
      {
      
         strncpy( & argstrMove[ iNumberOfCharacters++ ], "N", 1 );
         
      } 
      if ( ( argsMove->iPiece == dWhiteRook ) ||
           ( argsMove->iPiece == dBlackRook ) )
      {
      
         strncpy( & argstrMove[ iNumberOfCharacters++ ], "R", 1 );
         
      } 
      
   }

   // Look for check
   // Flip the color to move.
   if ( sBoard->siColorToMove == dWhite )
   {
      
      sBoard->siColorToMove = dBlack;

   }
   else
   {

      sBoard->siColorToMove = dWhite;

   }
   
   // Look for check.  First make the move.       
   MakeMove( argsMove, 
             sBoard, 
             argsGeneralMoves, 
             0 );

   // Calculate the moves
   CalculateAttacks( vsMoveList2,
                     sBoard,
                     argsGeneralMoves );
                           
   // Look to see if the king is in check.                        
   int iCheck = sBoard->bbCheck;

   // Undo the move.
   UndoMove( sBoard,
             argsGeneralMoves );

   // Flip the color to move.
   if ( sBoard->siColorToMove == dWhite )
   {
      
      sBoard->siColorToMove = dBlack;

   }
   else
   {

      sBoard->siColorToMove = dWhite;

   }

   // Look for checkmate.
   // Copy the existing board into the new board.
   * sBoard = * argsBoard;

   // Allow the new board to do a shallow search.
   // Shouldn't this be 2 since we just made a move?
   sBoard->iMaxPlys = 2;
   
   // Look for check.  First make the move.       
   MakeMove( argsMove, 
             sBoard, 
             argsGeneralMoves, 
             0 );

   // Turn off the hash table.
   int iUseHashTable = GetUseHashTable();
   SetUseHashTable(        dNo );  // dYes dNo

   int iScore = 0;
   int iBestMove = -1;
   int iBestMoveSearched = 0;

   // Set some search parameters
   SetNumberOfNodes( 0 );
   SetNumberOfIncrementalNodes( 0 );
   SetNodeCheck( dNodeCheck );

   assert( CheckBoard( sBoard ) );

   // Perform a search.
   iScore = Search( sBoard,
                    argsGeneralMoves,
                    dAlpha,
                    dBeta );

   // Turn the hash back on if we were using it before.
   SetUseHashTable( iUseHashTable );

   // Undo the move.
   UndoMove( sBoard,
             argsGeneralMoves );

   // Assume we are not in check mate.
   int iCheckMate = 0;

   // This might need to be -dMate due to the previously made move.
   if ( iScore <= dMate )
   {

      iCheckMate = 1;

   }
                          
   // Print out check and checkmate.
   if ( iCheckMate == 1 )
   {
   
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "#", 1 );
      
   }                      
   else if ( iCheck != 0 )
   {
   
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "+", 1 );
      
   }

   // Release the board from memory.
   free( sBoard );

   // Return the number of characters.
   return iNumberOfCharacters--;

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
int PrintMoveFast( struct Board * argsBoard,
                   struct GeneralMove * argsGeneralMoves,
                   Move   * argsMove,
                   char   * argstrMove )
{

// Write out a move in algebraic notation.
//

   int iRowFrom;
   int iColFrom;
   int iRowTo;
   int iColTo;
   int iPieceFrom;
   int iPieceTo;
   int iDegeneracy;
   int iRowDegeneracy;
   int iColDegeneracy;
   int iNumberOfCharacters = 0;
   int siNumberOfMoves     = 0;
   int iMoveCount;
   int iMoveCountDummy;
   int viLegalMove[ dNumberOfMoves ];
   char strStartSquareCol[ 2 ];
   char strStartSquareRow[ 2 ];
   char strEndSquareCol[   2 ];
   char strEndSquareRow  [ 2 ];
   Move vsMoveList[      dNumberOfMoves ];
   Move vsMoveListDummy[ dNumberOfMoves ];

   // Keep an old board around for scoring.
   struct Board * sBoard;
   sBoard = (Board * ) malloc( 1 * sizeof( Board ) );
   * sBoard = * argsBoard;

   // Clear the string.
   strcpy( argstrMove, " " );         

   // Assume no degeneracy.
   iDegeneracy = 0;
   iRowDegeneracy = 0;
   iColDegeneracy = 0;   

   // Generate all of the legal moves.
   CalculateMoves( vsMoveList, 
                   sBoard, 
                   argsGeneralMoves );

   // Extract the number of moves to live locally.
   siNumberOfMoves = sBoard->siNumberOfMoves;

   // Check to see if the moves are legal.
   for ( iMoveCount = 0; iMoveCount < siNumberOfMoves; iMoveCount++ )
   {

      MakeMove( vsMoveList,
                sBoard,
                argsGeneralMoves,
                iMoveCount );

      CalculateMoves( vsMoveListDummy,
                      sBoard,
                      argsGeneralMoves );

      LegalState( sBoard,
                  argsGeneralMoves );

      viLegalMove[ iMoveCount ] = sBoard->siLegalMove;

      if ( sBoard->siLegalMove == 0 )
      {

         // Remove the move from the list.
         for ( iMoveCountDummy = iMoveCount; iMoveCountDummy < siNumberOfMoves - 1; iMoveCountDummy++ )
         {

            // Move the mooves down one count.
            vsMoveList[ iMoveCountDummy ] = vsMoveList[ iMoveCountDummy + 1 ];

         }

         // Decrement the number of moves.
         siNumberOfMoves--;

         // Decrement the counter.
         iMoveCount--;
         
      }

      UndoMove( sBoard,
                argsGeneralMoves );

   }

   // Calculate the from row and column.
   iRowFrom          = dRow( argsMove->iFromSquare );
   iColFrom          = dCol( argsMove->iFromSquare );
   iPieceFrom        = sBoard->mBoard[ argsMove->iFromSquare ];
   ConvertIntToRow( iRowFrom, 
                    & strStartSquareRow[ 0 ] );
   ConvertIntToCol( iColFrom, 
                    & strStartSquareCol[ 0 ] );

   // Calculate the to row and column.
   iRowTo          = dRow( argsMove->iToSquare );
   iColTo          = dCol( argsMove->iToSquare );
   iPieceTo        = sBoard->mBoard[ argsMove->iToSquare ];
   ConvertIntToRow( iRowTo, 
                    & strEndSquareRow[ 0 ] );
   ConvertIntToCol( iColTo, 
                    & strEndSquareCol[ 0 ] );
      
   // Look for a degeneracy.
   for ( int iMoveIndex = 0; iMoveIndex < siNumberOfMoves; iMoveIndex++ )
   {
      
      // Look for landing on the same square
      // Make sure it is not from a promotion
      if ( ( vsMoveList[ iMoveIndex ].iToSquare     == argsMove->iToSquare   ) &&
           ( vsMoveList[ iMoveIndex ].iFromSquare   != argsMove->iFromSquare ) &&
           ( vsMoveList[ iMoveIndex ].iPiece        == argsMove->iPiece      ) && 
           ( ( ( argsMove->iMoveType                == dPromote              ) ||
               ( argsMove->iMoveType                == dCaptureAndPromote    ) ||
               ( vsMoveList[ iMoveIndex ].iMoveType == dPromote              ) ||
               ( vsMoveList[ iMoveIndex ].iMoveType == dCaptureAndPromote ) ) == 0 ) )
      {	
      
         // Show that the move is degenerate.
         iDegeneracy = 1;
         
         // Look for column and row degeneracy.
         if ( iRowFrom == dRow( vsMoveList[ iMoveIndex ].iFromSquare ) )
         {
         
            // Set a degenerate row.
            iRowDegeneracy = 1;
            
         }
         if ( iColFrom == dCol( vsMoveList[ iMoveIndex ].iFromSquare ) )
         {
         
            // Set a degenerate row.
            iColDegeneracy = 1;
            
         }
         // Look for a complete mismatch.
         if ( ( iColDegeneracy == 0 ) &&
              ( iRowDegeneracy == 0 ) )
         {
         
            // The row takes precidence over the column 
            iColDegeneracy = 1;
            
         }
         
                                              
      }
      
   }

   // Set the default that there is no check.
   sBoard->bbCheck = 0;   
   if ( ( ( argsMove->iMoveType == dWhiteKingSideCastle ) &&
          ( sBoard->bbCheck == 0 ) )                   ||
        ( ( argsMove->iMoveType == dBlackKingSideCastle ) &&
          ( sBoard->bbCheck == 0 ) ) )
   {
   
      
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "O", 1 );
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "-", 1 );
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "O", 1 );      
      
   }
   if ( ( ( argsMove->iMoveType == dWhiteQueenSideCastle ) &&
          ( sBoard->bbCheck == 0 ) )                    ||
        ( ( argsMove->iMoveType == dBlackQueenSideCastle ) &&
          ( sBoard->bbCheck == 0 ) ) )
   {

      
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "O", 1 );      
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "-", 1 );            
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "O", 1 );            
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "-", 1 );            
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "O", 1 );
      
   }
   if ( ( iPieceFrom == dWhiteRook ) ||
        ( iPieceFrom == dBlackRook ) )
   {
   
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "R", 1 );
      
   }
   if ( ( iPieceFrom == dWhiteKnight ) ||
        ( iPieceFrom == dBlackKnight ) )
   {
   
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "N", 1 );
      
   }
   if ( ( iPieceFrom == dWhiteBishop ) ||
        ( iPieceFrom == dBlackBishop ) )
   {
   
      
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "B", 1 );      
      
   }
   if ( ( iPieceFrom == dWhiteQueen ) ||
        ( iPieceFrom == dBlackQueen ) )
   {
   
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "Q", 1 );
      
   }
   if ( ( iPieceFrom == dWhiteKing ) ||
        ( iPieceFrom == dBlackKing ) )
   {
      
      if ( ( argsMove->iMoveType != dWhiteKingSideCastle  ) &&
           ( argsMove->iMoveType != dBlackKingSideCastle  ) &&
           ( argsMove->iMoveType != dWhiteQueenSideCastle ) &&
           ( argsMove->iMoveType != dBlackQueenSideCastle ) )
      {
         
         strncpy( & argstrMove[ iNumberOfCharacters++ ], "K", 1 );

      }
      
   }
   
   // Write out any degenerate information.
   if ( ( argsMove->iPiece != dWhitePawn ) &&
        ( argsMove->iPiece != dBlackPawn ) )
   {
      
      if ( iRowDegeneracy == 1 )
      {
   
         strncpy( & argstrMove[ iNumberOfCharacters++ ], 
                  & strStartSquareCol[ 0 ], 1 );
         
      }
      if ( iColDegeneracy == 1 )
      {
   
         strncpy( & argstrMove[ iNumberOfCharacters++ ], 
                  & strStartSquareRow[ 0 ], 1 );
         
      }
      
   }
      
   // Look for a capture.
   if ( ( argsMove->iMoveType == dCapture  ) ||
        ( argsMove->iMoveType == dEnPassant ) ||
        ( argsMove->iMoveType == dCaptureAndPromote ) )
   {
   
      if ( ( iPieceFrom == dWhitePawn ) ||
           ( iPieceFrom == dBlackPawn ) )
      {
            
         strncpy( & argstrMove[ iNumberOfCharacters++ ], & strStartSquareRow[ 0 ], 1 );
         
      }
      
      strncpy( & argstrMove[ iNumberOfCharacters++ ], "x", 1 );
      
   }

   // Write out the final square if required.
   if ( ( argsMove->iMoveType == dCapture           ) ||
        ( argsMove->iMoveType == dCaptureAndPromote ) ||
        ( argsMove->iMoveType == dEnPassant         ) ||
        ( argsMove->iMoveType == dRegular           ) ||
        ( argsMove->iMoveType == dTwoSquare         ) ||
        ( argsMove->iMoveType == dOneSquare         ) ||
        ( argsMove->iMoveType == dPromote           ) )
   {
   
      // Write out the final square.
      strncpy( &argstrMove[ iNumberOfCharacters++ ], 
               &strEndSquareRow[ 0 ], 1 );
      strncpy( &argstrMove[ iNumberOfCharacters++ ], 
               &strEndSquareCol[ 0 ], 1 );
       
   }
   
   // Take care of promotions.   
   if ( ( argsMove->iMoveType == dPromote           ) ||
        ( argsMove->iMoveType == dCaptureAndPromote ) )
   {
      
     strncpy( & argstrMove[ iNumberOfCharacters++ ], "=", 1 );
      
      if ( ( argsMove->iPiece == dWhiteQueen ) ||
           ( argsMove->iPiece == dBlackQueen ) )
      {
      
         strncpy( & argstrMove[ iNumberOfCharacters++ ], "Q", 1 );
         
      } 
      if ( ( argsMove->iPiece == dWhiteBishop ) ||
           ( argsMove->iPiece == dBlackBishop ) )
      {
      
         strncpy( & argstrMove[ iNumberOfCharacters++ ], "B", 1 );
         
      } 
      if ( ( argsMove->iPiece == dWhiteKnight ) ||
           ( argsMove->iPiece == dBlackKnight ) )
      {
      
         strncpy( & argstrMove[ iNumberOfCharacters++ ], "N", 1 );
         
      } 
      if ( ( argsMove->iPiece == dWhiteRook ) ||
           ( argsMove->iPiece == dBlackRook ) )
      {
      
         strncpy( & argstrMove[ iNumberOfCharacters++ ], "R", 1 );
         
      } 
      
   }

   // Free the board
   free( sBoard );

   // Return the number of characters.
   return iNumberOfCharacters--;

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void ComputerMove( struct Board * argsBoard,
                   struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Define some variables.
   int iScore = 0;
   int iBookMove = -1;
   Move vsMoveList[ dNumberOfMoves ];
   char strMove[ 64 ];
   Move sBookMove;

   // See if we should use the book.
   if ( GetUseOpeningBook() == 0 )
   {

      // If we are not using the book, just say we are
      // not using the book from the beginning.
      SetIsInBook( dNo );

   }

   // Reset the plys.
   argsBoard->iNumberOfPlys = -1;

   // Reset the clocks
   argsBoard->cStart = clock();

   // Reset Alpha and beta
   int iAlpha = dAlpha;
   int iBeta  = dBeta;

   // Do some debugging.
   if ( GetInterfaceMovesDebug() )
   {

      gofDebugMoves << endl;
      gofDebugMoves << "Starting the computer move routine." << endl;
      gofDebugMoves << "InBook = " << GetIsInBook() << endl;
      gofDebugMoves << "Starting Hash = " << GetHash() << endl;

   } 

   // Find a book move.
   if ( GetIsInBook() == dYes )
   {

      // Find a move from the opening book.
      FindBookMove( argsBoard,
                    argsGeneralMoves,
                    vsMoveList,
                    & sBookMove,
                    & iBookMove );

      // Do some debugging.
      if ( GetInterfaceMovesDebug() )
      {

         gofDebugMoves << endl;
         gofDebugMoves << "iBookMove = " << iBookMove << endl;

      }

      // Store the book move in a special container.
      if ( iBookMove > -1 )
      {

         //sBookMove = vsMoveList[ iBookMove ];

         strncpy( strMove, "      ", 6 );
         // Create a book move string.
         CreateAlgebraicMove( strMove,
                              & sBookMove,
                              0 );

         // Some Debugging.
         if ( GetInterfaceMovesDebug() )
         {

            gofDebugMoves << endl;
            gofDebugMoves << "Juat after FindBookMove strMove = " << strMove << endl;

         }

      }

      // Write out the book statistics for the folks at home.
      if ( GetInterfaceMode() == dConsole )
      {
      
         PrintOpeningBookMoveStatistics( argsBoard,
                                         argsGeneralMoves );

      }

   }

   // Once we leave the book, stay out in order to avoid hash collisions.
   if ( iBookMove == -1 )
   {

      // Signal that we are out of book.
      SetIsInBook( dNo );

      // Perform a search.
      iScore = StartSearch( argsBoard, 
                            argsGeneralMoves,
                            iAlpha,
                            iBeta ); 
   
   }


   // Get the finish time of the calculation.                                      
   argsBoard->cFinish = clock();

   // Take care of the Console input and output.
   if ( GetInterfaceMode() == dConsole )
   {

      // print out the time.
      cout << "Time for evaluation = " << ( argsBoard->cFinish - argsBoard->cStart ) / 1000.0 << endl;

      // Print the score from the position if out of book.
      if ( iBookMove == -1 )
      {

         cout << "Here's the score from the position. " << iScore << endl;

         PrintPrincipalVariation( argsBoard,
                                  argsGeneralMoves );

      }

      // Print the book move if in book.
      if ( iBookMove != -1 )
      {

         int iNumberOfChars = PrintMove( argsBoard,
                                         argsGeneralMoves,
                                         & vsMoveList[ iBookMove ],
                                         strMove );

         cout << "Book Move = ";
         for ( int iCharCount = 0; iCharCount < iNumberOfChars; iCharCount++ )
         {

            cout << strMove[ iCharCount ];

         }

     }
 
      cout << endl;

   }

   // Make the best move on the board.
   if ( iBookMove == -1 )
   {

      // Make an out of book move.
      MakeMove( *argsBoard->vmPrincipalVariation,
	             argsBoard,
	             argsGeneralMoves,
		          0 );

   }
   else
   {

      // Make a book move.
      MakeMove( & vsMoveList[ iBookMove ],
	             argsBoard,
	             argsGeneralMoves,
	             0 );
      
   }
       
   gofDebugMoves << "Hash after move = " << GetHash() << endl;  

   // Do special cases for the console
   /*
   if ( GetInterfaceMode() == dConsole )
   {

      if ( argsBoard->bbCheck & 4 )
      {

         cout << endl << "White check mate" << endl;
         system( "pause" );
         return;

      }
      else if ( argsBoard->bbCheck & 8 )
      {

         cout << endl << "Black check mate" << endl;
         system( "pause" );
         return;

      }
      else if ( argsBoard->bbCheck & 16 )
      {

         cout << endl << "White stalemate" << endl;
         system( "pause" );
         return;

      }
      else if ( argsBoard->bbCheck & 32 )
      {

         cout << endl << "Black stalemate" << endl;
         system( "pause" );
         return;
      }

   }
   */

   // Reset some search parameters.
   argsBoard->iNumberOfPlys = -1;

   // Show the end results.
   if ( GetInterfaceMode() == dConsole )
   {

      PrintBoard( argsBoard->mBoard );
      PrintFEN( argsBoard,
                argsGeneralMoves );

   }
   else if ( GetInterfaceMode() == dUCI )
   {
   
      strncpy( strMove, "      ", 6 );
   
      if ( iBookMove == -1 )
      {

         // Create a search string.
         CreateAlgebraicMove( strMove,
                              argsBoard->vmPrincipalVariation[ 0 ],
                              0 );
                              
      }
      else
      {

         // Create a book move string.
         CreateAlgebraicMove( strMove,
                              & sBookMove,
                              0 );
      
      }

      // Some Debugging.
      if ( GetInterfaceMovesDebug() )
      {

         gofDebugMoves << endl;
         gofDebugMoves << "Just before interface strMove = " << strMove << endl;

      }

      // Create the best move string.
      char strUpdate[ 640 ];
      strcpy( strUpdate, " " );
      sprintf( strUpdate, "bestmove %s info depth %d seldepth %d nodes %d", 
               strMove,
               argsBoard->iMaxPlys + 1, 
               argsBoard->iMaxPlysReached + 1, 
               GetNumberOfNodes() );

      // Some Debugging.
      if ( GetInterfaceMovesDebug() )
      {

         gofDebugMoves << endl;
         gofDebugMoves << "strUpdate = " << strUpdate << endl;
         gofDebugMoves << endl << endl;

      }



      SendCommand( strUpdate );

   }

}

//
//---------------------------------------------------------------------
//
// Check to see if the board is in a legal state.
void LegalState( struct Board * argsBoard,
                 struct GeneralMove * argsGeneralMoves )
{

   // Debug the input.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Set the default to legal.
   argsBoard->siLegalMove = 1;

   // Look for check.
   if ( ( argsBoard->bbCheck & 1 ) &&
        ( argsBoard->siColorToMove == dBlack ) )
   {

      argsBoard->siLegalMove = 0;

   }
 
   if ( ( argsBoard->bbCheck & 2 ) &&
        ( argsBoard->siColorToMove == dWhite ) )
   {

      argsBoard->siLegalMove = 0;

   }

   // See if the king is off the board.
   if ( argsBoard->bbWhiteKing == 0 )
   {

      argsBoard->siLegalMove = 0;
    
   }

   if ( argsBoard->bbBlackKing == 0 )
   {

      argsBoard->siLegalMove = 0;

   }

   // Look for an illegal white king side castle.
   if ( ( argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iMoveType == dWhiteKingSideCastle ) &&
        ( ( argsGeneralMoves->bbKingSideWhiteCastleArea & ( argsBoard->bbBlackMove | argsBoard->bbBlackAttack ) ) > 0 ) &&
        ( argsBoard->siColorToMove == dBlack ) )
   {

      argsBoard->siLegalMove = 0;

   }

   // Look for an illegal white queen side castle.
   if ( ( argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iMoveType == dWhiteQueenSideCastle ) &&
        ( ( argsGeneralMoves->bbQueenSideWhiteCastleArea & ( argsBoard->bbBlackMove | argsBoard->bbBlackAttack ) ) > 0 ) &&
        ( argsBoard->siColorToMove == dBlack ) )
   {

      argsBoard->siLegalMove = 0;

   }

   // Look for an illegal Black King side castle.
   if ( ( argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iMoveType == dBlackKingSideCastle ) &&
        ( ( argsGeneralMoves->bbKingSideBlackCastleArea & ( argsBoard->bbWhiteMove | argsBoard->bbWhiteAttack ) ) > 0 ) &&
        ( argsBoard->siColorToMove == dWhite ) )
   {

      argsBoard->siLegalMove = 0;

   }

   // Look for an illegal Black queen side castle.
   if ( ( argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iMoveType == dBlackQueenSideCastle ) &&
        ( ( argsGeneralMoves->bbQueenSideBlackCastleArea & ( argsBoard->bbWhiteMove | argsBoard->bbWhiteAttack ) ) > 0 ) &&
        ( argsBoard->siColorToMove == dWhite ) )
   {

      argsBoard->siLegalMove = 0;

   }

   // Debugging
   assert( argsBoard->siLegalMove >= 0 );
   assert( argsBoard->siLegalMove <= 1 );

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateMovesForCheck( struct Move * argsMoves, 
                             struct Board * argsBoard, 
                             struct GeneralMove * argsGeneralMoves )
{
// This is a controlling function.
//
// This function is used to see if a move creates check.
// E. G.  After white has moved, calculate the white moves for check.

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Initialize the counters.
   argsBoard->siNumberOfMoves    = 0;
	argsBoard->bbWhiteAttack      = 0;
   argsBoard->bbWhiteMove        = 0;
   argsBoard->bbBlackAttack      = 0;
   argsBoard->bbBlackMove        = 0;
   argsBoard->bbWhiteKingMove    = 0;
   argsBoard->bbWhiteKingAttack  = 0;
   argsBoard->bbBlackKingMove    = 0;
   argsBoard->bbBlackKingAttack  = 0;
   argsBoard->bbEP               = 0;

   // Calculate the moves for the pieces.

   // Calculate for the white pieces.
   if ( argsBoard->siColorToMove == dBlack )
   {
   
      if ( argsBoard->bbWhitePawn > 0 )
      {

         CalculateWhitePawnsAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteKnight > 0 )
      {

         CalculateWhiteKnightsAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteBishop > 0 )
      {

         CalculateWhiteBishopsAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteRook > 0 )
      {

         CalculateWhiteRooksAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteQueen > 0 )
      {

         CalculateWhiteQueensAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteKing > 0 )
      {

         CalculateWhiteKingAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbCastle > 0 )
      {

         CastleWhite( argsMoves, argsBoard, argsGeneralMoves );

      }
      
   }

   if ( argsBoard->siColorToMove == dWhite )
   {

      if ( argsBoard->bbBlackPawn > 0 )
      {

         CalculateBlackPawnsAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbBlackKnight > 0 )
      {

         CalculateBlackKnightsAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbBlackBishop > 0 )
      {

         CalculateBlackBishopsAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbBlackRook > 0 )
      {

         CalculateBlackRooksAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbBlackQueen > 0 )
      {

         CalculateBlackQueensAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbBlackKing > 0 )
      {

         CalculateBlackKing( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbCastle > 0 )
      {

         CastleBlack( argsMoves, argsBoard, argsGeneralMoves );

      }
      
   }

   // Calculate Check.
   CalculateCheck( argsMoves, argsBoard, argsGeneralMoves );

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateAttacks( struct Move * argsMoves, 
                       struct Board * argsBoard, 
                       struct GeneralMove * argsGeneralMoves )
{
// This is a controlling function.
//

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Initialize the counters.
   argsBoard->siNumberOfMoves    = 0;
	argsBoard->bbWhiteAttack      = 0;
   argsBoard->bbBlackAttack      = 0;
   argsBoard->bbWhiteKingMove    = 0;
   argsBoard->bbWhiteKingAttack  = 0;
   argsBoard->bbBlackKingMove    = 0;
   argsBoard->bbBlackKingAttack  = 0;
   argsBoard->bbEP               = 0;

   // Calculate the moves for the pieces.

   // Calculate for the white pieces.
   if ( argsBoard->siColorToMove == dWhite )
   {
   
      if ( argsBoard->bbWhitePawn > 0 )
      {

         CalculateWhitePawnsAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteKnight > 0 )
      {

         CalculateWhiteKnightsAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteBishop > 0 )
      {

         CalculateWhiteBishopsAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteRook > 0 )
      {

         CalculateWhiteRooksAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteQueen > 0 )
      {

         CalculateWhiteQueensAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteKing > 0 )
      {

         CalculateWhiteKingAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }
      
   }

   if ( argsBoard->siColorToMove == dBlack )
   {

      if ( argsBoard->bbBlackPawn > 0 )
      {

         CalculateBlackPawnsAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbBlackKnight > 0 )
      {

         CalculateBlackKnightsAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbBlackBishop > 0 )
      {

         CalculateBlackBishopsAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbBlackRook > 0 )
      {

         CalculateBlackRooksAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbBlackQueen > 0 )
      {

         CalculateBlackQueensAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }

      if ( argsBoard->bbBlackKing > 0 )
      {

         CalculateBlackKingAttacks( argsMoves, argsBoard, argsGeneralMoves );

      }
      
   }

   // Calculate Check.
   CalculateCheck( argsMoves, argsBoard, argsGeneralMoves );

}

//
//-----------------------------------------------------------------------------------
//
void CalculateWhitePawnsAttacks( struct Move * argsMoves, 
                                 struct Board * argsBoard, 
                                 struct GeneralMove * argsGeneralMoves )
{
//
// This function looks at the white pawn attacks.
//

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Do the attacks to the right.
   BitBoard bbAttacksRight = ( argsBoard->bbWhitePawn << 9 );
   bbAttacksRight          = bbAttacksRight & argsBoard->bbBlackPieces;
   bbAttacksRight          = bbAttacksRight & ( ~ argsGeneralMoves->bbRowSeven );

   //Do the Attacks to the left.
   BitBoard bbAttacksLeft = ( argsBoard->bbWhitePawn << 7 );
   bbAttacksLeft          = bbAttacksLeft & argsBoard->bbBlackPieces;
   bbAttacksLeft          = bbAttacksLeft & ( ~ argsGeneralMoves->bbRowZero );

   // Take out the promotion things.
   //Find the promotions
   BitBoard bbPromotionsRight = ( argsGeneralMoves->bbColSeven & bbAttacksRight );
   BitBoard bbPromotionsLeft  = ( argsGeneralMoves->bbColSeven & bbAttacksLeft );

   // Combine the attacks and moves.
   argsBoard->bbWhiteAttack = argsBoard->bbWhiteAttack | bbAttacksRight | bbAttacksLeft;

   // Remove the pormotions from other moves.
   bbAttacksRight = bbAttacksRight & ~argsGeneralMoves->bbColSeven;
   bbAttacksLeft  = bbAttacksLeft & ~argsGeneralMoves->bbColSeven;

   // Find the moves and attacks.
   int vAttacksLeft[  8 ];
   int vAttacksRight[ 8 ];
   int vPromotionsRight[ 8 ];
   int vPromotionsLeft[ 8 ];
   int numberOfAttacksRight    = Find( bbAttacksRight,    vAttacksRight,    argsGeneralMoves );
   int numberOfAttacksLeft     = Find( bbAttacksLeft,     vAttacksLeft,     argsGeneralMoves );
   int numberOfPromotionsRight = Find( bbPromotionsRight, vPromotionsRight, argsGeneralMoves );
   int numberOfPromotionsLeft  = Find( bbPromotionsLeft,  vPromotionsLeft,  argsGeneralMoves );
   	
   // Enter the right attacks.
   for ( int moveIndex = 0; moveIndex < numberOfAttacksRight; moveIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vAttacksRight[ moveIndex ] - 9 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vAttacksRight[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vAttacksRight[ moveIndex ] - 9;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vAttacksRight[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhitePawn;

      // Score the moves.
      switch ( argsBoard->mBoard[  argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare ] ) 
      {

         case dBlackPawn:
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesPawn;
            break;
      
         }
         case dBlackKnight :
         {   
              
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesKnight;
            break;

         }
         case dBlackBishop :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesBishop;
            break;
   
         }
         case dBlackRook :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesRook;
            break;
   
         }
         case dBlackQueen :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesQueen;
            break;
   
         }
         case dBlackKing :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
            break;
   
         }

      }

   }
   		
   // Enter the left attacks.
   for ( int moveIndex = 0; moveIndex < numberOfAttacksLeft; moveIndex++ )
   {
       
      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare, 
    		                                                                     vAttacksLeft[ moveIndex ] - 7 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vAttacksLeft[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vAttacksLeft[ moveIndex ] - 7;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vAttacksLeft[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhitePawn;

      // Score the moves.
      switch ( argsBoard->mBoard[  argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare ] ) 
      {

         case dBlackPawn:
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesPawn;
            break;
      
         }
         case dBlackKnight :
         {   
              
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesKnight;
            break;

         }
         case dBlackBishop :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesBishop;
            break;
   
         }
         case dBlackRook :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesRook;
            break;
   
         }
         case dBlackQueen :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesQueen;
            break;
   
         }
         case dBlackKing :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
            break;
   
         }

      }

   }

   // Enter the promotions right.
   for ( int moveIndex = 0; moveIndex < numberOfPromotionsRight; moveIndex++ )
   {

      for ( int pieceIndex = 0; pieceIndex < 4; pieceIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                              vPromotionsRight[ moveIndex ] - 9 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                                 vPromotionsRight[ moveIndex ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPromotionsRight[ moveIndex ] - 9;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vPromotionsRight[ moveIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCaptureAndPromote;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = argsGeneralMoves->vPromotionPieces[ pieceIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iCapture     = argsBoard->mBoard[ vPromotionsRight[ moveIndex ] ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPromotion;

      }

   }

   // Enter the promotions Left.
   for ( int moveIndex = 0; moveIndex < numberOfPromotionsLeft; moveIndex++ )
   {

      for ( int pieceIndex = 0; pieceIndex < 4; pieceIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                              vPromotionsLeft[ moveIndex ] - 7 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                                 vPromotionsLeft[ moveIndex ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPromotionsLeft[ moveIndex ] - 7;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vPromotionsLeft[ moveIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCaptureAndPromote;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = argsGeneralMoves->vPromotionPieces[ pieceIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iCapture     = argsBoard->mBoard[ vPromotionsLeft[ moveIndex ] ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPromotion;

      }

   }
   		
   // Do that EP thing!
   if ( argsBoard->iMoveHistory > 0 )
   {

      if ( argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare > 0 )
      {
      
         // Do the attacks to the right.
         BitBoard bbAttacksRight = ( argsBoard->bbWhitePawn << 9 );
         bbAttacksRight = bbAttacksRight & ( ~ argsGeneralMoves->bbRowSeven );
         bbAttacksRight = bbAttacksRight & 
                          argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare &
                          ~argsBoard->bbAllPieces;

         // See if we found one.
         if ( bbAttacksRight > 0 )
         {

            int viVector[ 1 ];
            int iSquare = Find( bbAttacksRight, 
                                     viVector, 
                                     argsGeneralMoves );
            argsBoard->siNumberOfMoves++;
            InitializeMoves( argsMoves,
                             argsBoard->siNumberOfMoves - 1 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare, 
     		                                                                           viVector[ 0 ] - 9 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                                 viVector[ 0 ] );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = viVector[ 0 ] - 9;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = viVector[ 0 ];
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dEnPassant;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhitePawn;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msCaptureSide;

         }

         // Do the attacks to the Left.
         BitBoard bbAttacksLeft = ( argsBoard->bbWhitePawn << 7 );
         bbAttacksLeft = bbAttacksLeft & ( ~ argsGeneralMoves->bbRowZero );
         bbAttacksLeft = bbAttacksLeft & 
                         argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare &
                         ~argsBoard->bbAllPieces;

         // See if we found one.
         if ( bbAttacksLeft > 0 )
         {

            int viVector[ 1 ];
            int iSquare = Find( bbAttacksLeft, viVector, argsGeneralMoves );
            argsBoard->siNumberOfMoves++;
            InitializeMoves( argsMoves,
                             argsBoard->siNumberOfMoves - 1 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare, 
    		                                                                           viVector[ 0 ] - 7 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                                 viVector[ 0 ] );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = viVector[ 0 ] - 7;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = viVector[ 0 ];
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dEnPassant;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhitePawn;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msCaptureSide;

         }

      }

   }

}   

//
//-----------------------------------------------------------------------------------
//
void CalculateWhiteRooksAttacks( struct Move * argsMoves, 
                                 struct Board * argsBoard, 
                                 struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Do the Rooks.
   int numberOfRooks = 0;
   int vWhiteRooks[ 10 ];
   numberOfRooks = Find( argsBoard->bbWhiteRook, vWhiteRooks, argsGeneralMoves );

   // Loop over the rooks.
   for ( int rookIndex = 0; rookIndex < numberOfRooks; rookIndex++ )
   {
      
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves,
                     argsGeneralMoves->mRN[ vWhiteRooks[ rookIndex ] ], 
                     argsGeneralMoves->vQNOMN[ vWhiteRooks[ rookIndex ] ], 
                     vWhiteRooks[ rookIndex ], 
                     dWhiteRook);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves,
                     argsGeneralMoves->mRE[ vWhiteRooks[ rookIndex ] ], 
                     argsGeneralMoves->vQNOME[ vWhiteRooks[ rookIndex ] ], 
                     vWhiteRooks[ rookIndex ], 
                     dWhiteRook);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves,
                     argsGeneralMoves->mRS[ vWhiteRooks[ rookIndex ] ], 
                     argsGeneralMoves->vQNOMS[ vWhiteRooks[ rookIndex ] ], 
                     vWhiteRooks[ rookIndex ], 
                     dWhiteRook);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves,
                     argsGeneralMoves->mRW[ vWhiteRooks[ rookIndex ] ], 
                     argsGeneralMoves->vQNOMW[ vWhiteRooks[ rookIndex ] ], 
                     vWhiteRooks[ rookIndex ], 
                     dWhiteRook);

   }
   
}

//
//-----------------------------------------------------------------------------------
//
void CalculateWhiteKnightsAttacks( struct Move * argsMoves, 
                                   struct Board * argsBoard, 
                                   struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Do the Knights.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbWhiteKnight, 
                          vPieces, 
                          argsGeneralMoves );

   int vAttacks[ 8 ];

   // Loop over the knights.
   for ( int index = 0; index < numberOfPieces; index++ )
   {

      // Look for the good moves..
      BitBoard bbAttacks = argsGeneralMoves->bbNMove[ vPieces[ index ] ] &
                           (  argsBoard->bbBlackPieces );

      // Enter the moves.
      int numberOfAttacks = Find( bbAttacks, 
                                       vAttacks, 
                                       argsGeneralMoves );
      argsBoard->bbWhiteAttack = argsBoard->bbWhiteAttack | bbAttacks;

      // Enter the attacks.
      for ( int attackIndex = 0; attackIndex < numberOfAttacks; attackIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
		                                                                           vPieces[ index ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
             			                                                            vAttacks[ attackIndex ] );
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPieces[ index ];
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vAttacks[ attackIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
		   argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhiteKnight;

         // Score the moves.
         switch ( argsBoard->mBoard[  argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare ] ) 
         {

            case dBlackPawn:
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesPawn;
               break;
         
            }
            case dBlackKnight :
            {   
                 
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesKnight;
               break;

            }
            case dBlackBishop :
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesBishop;
               break;
      
            }
            case dBlackRook :
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesRook;
               break;
      
            }
            case dBlackQueen :
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesQueen;
               break;
      
            }
            case dBlackKing :
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
               break;
      
            }

         }

      }

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateWhiteBishopsAttacks( struct Move * argsMoves, 
                                   struct Board * argsBoard, 
                                   struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Do the Bishops.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbWhiteBishop, 
                          vPieces, 
                          argsGeneralMoves );

   // Loop over the Bishops.
   for ( int pieceIndex = 0; pieceIndex < numberOfPieces; pieceIndex++ )
   {
      
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mBNE[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMNE[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dWhiteBishop);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mBSE[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMSE[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dWhiteBishop);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mBSW[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMSW[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dWhiteBishop);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mBNW[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMNW[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dWhiteBishop);

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateWhiteQueensAttacks( struct Move * argsMoves, 
                                  struct Board * argsBoard, 
                                  struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Do the Queens.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbWhiteQueen, vPieces, argsGeneralMoves );

   // Loop over the Queens.
   for ( int pieceIndex = 0; pieceIndex < numberOfPieces; pieceIndex++ )
   {

      
      SliderAttacks( argsMoves, 
                     argsBoard,
                     argsGeneralMoves, 
                     argsGeneralMoves->mQN[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMN[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dWhiteQueen);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mQNE[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMNE[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dWhiteQueen);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mQE[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOME[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dWhiteQueen);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mQSE[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMSE[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dWhiteQueen);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mQS[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMS[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dWhiteQueen);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mQSW[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMSW[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dWhiteQueen);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mQW[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMW[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dWhiteQueen);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mQNW[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMNW[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dWhiteQueen);

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateWhiteKingAttacks( struct Move * argsMoves, 
                                struct Board * argsBoard, 
                                struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Do the King.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbWhiteKing, 
                          vPieces, 
                          argsGeneralMoves );
   int index = 0;

   int vAttacks[ 8 ];

   // Look for the good moves..
   BitBoard bbAttacks = argsGeneralMoves->bbKMove[ vPieces[ index ] ] &
                        ( argsBoard->bbBlackPieces );

   argsBoard->bbWhiteAttack = argsBoard->bbWhiteAttack | bbAttacks;

   // Enter the moves.
   int numberOfAttacks = Find( bbAttacks, vAttacks, argsGeneralMoves );

   // Enter the attacks.
   for ( int attackIndex = 0; attackIndex < numberOfAttacks; attackIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vPieces[ index ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vAttacks[ attackIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPieces[ index ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vAttacks[ attackIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
	   argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhiteKing;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msCaptureDown;

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateBlackPawnsAttacks( struct Move * argsMoves, 
                                 struct Board * argsBoard, 
                                 struct GeneralMove * argsGeneralMoves )
{
//
// This function looks at the Black pawn moves.
//

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Do the attacks to the right.
   BitBoard bbAttacksRight = ( argsBoard->bbBlackPawn >> 9 );
   bbAttacksRight          = bbAttacksRight & ( ~ argsGeneralMoves->bbRowZero );
   bbAttacksRight          = bbAttacksRight & argsBoard->bbWhitePieces;
   bbAttacksRight          = bbAttacksRight & ( ~ argsBoard->bbBlackPieces );

   //Do the Attacks to the left.
   BitBoard bbAttacksLeft = ( argsBoard->bbBlackPawn >> 7 );
   bbAttacksLeft          = bbAttacksLeft & argsBoard->bbWhitePieces;
   bbAttacksLeft          = bbAttacksLeft & ( ~ argsGeneralMoves->bbRowSeven );
   bbAttacksLeft          = bbAttacksLeft & ( ~ argsBoard->bbBlackPieces );

   // Combine the attacks and moves.
   argsBoard->bbBlackAttack = argsBoard->bbBlackAttack | bbAttacksRight | bbAttacksLeft;

   // Take out the promotion things.
   //Find the promotions
   BitBoard bbPromotionsRight = ( argsGeneralMoves->bbColZero & bbAttacksRight );
   BitBoard bbPromotionsLeft  =  ( argsGeneralMoves->bbColZero & bbAttacksLeft );

   // Remove the pormotions from other moves.
   bbAttacksRight = bbAttacksRight & ~argsGeneralMoves->bbColZero;
   bbAttacksLeft  = bbAttacksLeft  & ~argsGeneralMoves->bbColZero;

   // Find the moves and attacks.
   int vAttacksLeft[  8 ];
   int vAttacksRight[ 8 ];
   int vPromotionsRight[ 8 ];
   int vPromotionsLeft[ 8 ];
   int numberOfAttacksRight    = Find( bbAttacksRight,    vAttacksRight,    argsGeneralMoves );
   int numberOfAttacksLeft     = Find( bbAttacksLeft,     vAttacksLeft,     argsGeneralMoves );
   int numberOfPromotionsRight = Find( bbPromotionsRight, vPromotionsRight, argsGeneralMoves );
   int numberOfPromotionsLeft  = Find( bbPromotionsLeft,  vPromotionsLeft,  argsGeneralMoves );

   // Enter the right attacks.
   for ( int moveIndex = 0; moveIndex < numberOfAttacksRight; moveIndex++ )
   {
      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vAttacksRight[ moveIndex ] + 9 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vAttacksRight[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vAttacksRight[ moveIndex ] + 9;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vAttacksRight[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
  	   argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackPawn;

      // Score the moves.
      switch ( argsBoard->mBoard[  argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare ] ) 
      {

         case dWhitePawn:
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesPawn;
            break;
      
         }
         case dWhiteKnight :
         {   
              
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesKnight;
            break;

         }
         case dWhiteBishop :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesBishop;
            break;
   
         }
         case dWhiteRook :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesRook;
            break;
   
         }
         case dWhiteQueen :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesQueen;
            break;
   
         }
         case dWhiteKing :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
            break;
   
         }

      }

   }

   // Enter the left attacks.
   for ( int moveIndex = 0; moveIndex < numberOfAttacksLeft; moveIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vAttacksLeft[ moveIndex ] +7 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vAttacksLeft[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vAttacksLeft[ moveIndex ] + 7;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vAttacksLeft[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
	   argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackPawn;

      // Score the moves.
      switch ( argsBoard->mBoard[  argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare ] ) 
      {

         case dWhitePawn:
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesPawn;
            break;
      
         }
         case dWhiteKnight :
         {   
              
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesKnight;
            break;

         }
         case dWhiteBishop :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesBishop;
            break;
   
         }
         case dWhiteRook :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesRook;
            break;
   
         }
         case dWhiteQueen :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msPawnTakesQueen;
            break;
   
         }
         case dWhiteKing :
         {
     
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
            break;
   
         }

      }

   }

   // Enter the promotions right.
   for ( int moveIndex = 0; moveIndex < numberOfPromotionsRight; moveIndex++ )
   {

      for ( int pieceIndex = 0; pieceIndex < 4; pieceIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );

         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                              vPromotionsRight[ moveIndex ] + 9 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                                 vPromotionsRight[ moveIndex ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPromotionsRight[ moveIndex ] + 9;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vPromotionsRight[ moveIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCaptureAndPromote;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = argsGeneralMoves->vPromotionPieces[ pieceIndex ] + 6;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iCapture     = argsBoard->mBoard[ vPromotionsRight[ moveIndex ] ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPromotion;

      }

   }

   // Enter the promotions Left.
   for ( int moveIndex = 0; moveIndex < numberOfPromotionsLeft; moveIndex++ )
   {

      for ( int pieceIndex = 0; pieceIndex < 4; pieceIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );

         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                              vPromotionsLeft[ moveIndex ] + 7 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                                 vPromotionsLeft[ moveIndex ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPromotionsLeft[ moveIndex ] + 7;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vPromotionsLeft[ moveIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCaptureAndPromote;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = argsGeneralMoves->vPromotionPieces[ pieceIndex ] + 6;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iCapture     = argsBoard->mBoard[ vPromotionsLeft[ moveIndex ] ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPromotion;

      }

   }

   // Do that EP thing!
   if ( argsBoard->iMoveHistory > 0 )
   {

      if ( argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare > 0 )
      {
      
         // Do the attacks to the right.
         BitBoard bbAttacksRight = ( argsBoard->bbBlackPawn >> 9 );
         bbAttacksRight = bbAttacksRight & ( ~ argsGeneralMoves->bbRowZero );
         bbAttacksRight = bbAttacksRight & 
                          argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare;

         // See if we found one.
         if ( ( bbAttacksRight > 0 ) )
         {

            int viVector[ 1 ];
            int iSquare = Find( bbAttacksRight, viVector, argsGeneralMoves );
             
            argsBoard->siNumberOfMoves++;
            InitializeMoves( argsMoves,
                             argsBoard->siNumberOfMoves - 1 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare, 
    		                                                                           viVector[ 0 ] + 9 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                                 viVector[ 0 ] );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = viVector[ 0 ] + 9;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = viVector[ 0 ];
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dEnPassant;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackPawn;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msCaptureSide;

         }

         // Do the attacks to the Left.
         bbAttacksLeft = ( argsBoard->bbBlackPawn >> 7 );
         bbAttacksLeft = bbAttacksLeft & ( ~ argsGeneralMoves->bbRowSeven );
         bbAttacksLeft = bbAttacksLeft & 
                         argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare;

         // See if we found one.
         if ( ( bbAttacksLeft > 0 ) )
         {

            int viVector[ 1 ];
            int iSquare = Find( bbAttacksLeft, viVector, argsGeneralMoves );
             
            argsBoard->siNumberOfMoves++;
            InitializeMoves( argsMoves,
                             argsBoard->siNumberOfMoves - 1 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare, 
    		                                                                           viVector[ 0 ] + 7 );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                                 viVector[ 0 ] );
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = viVector[ 0 ] + 7;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = viVector[ 0 ];
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dEnPassant;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackPawn;
            argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msCaptureSide;

         }

      }

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateBlackRooksAttacks( struct Move * argsMoves, 
                                 struct Board * argsBoard, 
                                 struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Do the Rooks.
   int numberOfRooks = 0;
   int vBlackRooks[ 10 ];
   numberOfRooks = Find( argsBoard->bbBlackRook, vBlackRooks, argsGeneralMoves );

   // Loop over the rooks.
   for ( int rookIndex = 0; rookIndex < numberOfRooks; rookIndex++ )
   {
      
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mRN[ vBlackRooks[ rookIndex ] ], 
                     argsGeneralMoves->vQNOMN[ vBlackRooks[ rookIndex ] ], 
                     vBlackRooks[ rookIndex ], 
                     dBlackRook);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mRE[ vBlackRooks[ rookIndex ] ], 
                     argsGeneralMoves->vQNOME[ vBlackRooks[ rookIndex ] ], 
                     vBlackRooks[ rookIndex ], 
                     dBlackRook);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mRS[ vBlackRooks[ rookIndex ] ], 
                     argsGeneralMoves->vQNOMS[ vBlackRooks[ rookIndex ] ], 
                     vBlackRooks[ rookIndex ], 
                     dBlackRook);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mRW[ vBlackRooks[ rookIndex ] ], 
                     argsGeneralMoves->vQNOMW[ vBlackRooks[ rookIndex ] ], 
                     vBlackRooks[ rookIndex ], 
                     dBlackRook);

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateBlackKnightsAttacks( struct Move * argsMoves, 
                                   struct Board * argsBoard, 
                                   struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Do the Knights.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbBlackKnight, vPieces, argsGeneralMoves );

   int vAttacks[ 8 ];

   // Loop over the knights.
   for ( int index = 0; index < numberOfPieces; index++ )
   {
      // Look for the good moves..
      BitBoard bbAttacks = argsGeneralMoves->bbNMove[ vPieces[ index ] ] &
                           ( argsBoard->bbWhitePieces );

      argsBoard->bbBlackAttack = argsBoard->bbBlackAttack | bbAttacks;

      // Enter the moves.
      int numberOfAttacks = Find( bbAttacks, vAttacks, argsGeneralMoves );

      // Enter the attacks.
      for ( int attackIndex = 0; attackIndex < numberOfAttacks; attackIndex++ )
      {
          
         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
		                                                                           vPieces[ index ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
             			                                                            vAttacks[ attackIndex ] );
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPieces[ index ];
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vAttacks[ attackIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackKnight;

         // Score the moves.
         switch ( argsBoard->mBoard[  argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare ] ) 
         {

            case dWhitePawn:
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesPawn;
               break;
         
            }
            case dWhiteKnight :
            {   
                 
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesKnight;
               break;

            }
            case dWhiteBishop :
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesBishop;
               break;
      
            }
            case dWhiteRook :
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesRook;
               break;
      
            }
            case dWhiteQueen :
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msKnightTakesQueen;
               break;
      
            }
            case dWhiteKing :
            {
        
               argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore = argsGeneralMoves->msCheck;
               break;
      
            }

         }

      }

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateBlackBishopsAttacks( struct Move * argsMoves, 
                                   struct Board * argsBoard, 
                                   struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Do the Bishops.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbBlackBishop, vPieces, argsGeneralMoves );

   // Loop over the Bishops.
   for ( int pieceIndex = 0; pieceIndex < numberOfPieces; pieceIndex++ )
   {
      
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mBNE[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMNE[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dBlackBishop);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mBSE[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMSE[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dBlackBishop);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mBSW[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMSW[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dBlackBishop);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mBNW[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMNW[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dBlackBishop);

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateBlackQueensAttacks(  struct Move * argsMoves, 
                                   struct Board * argsBoard, 
                                   struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Do the Queens.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbBlackQueen, vPieces, argsGeneralMoves );

   // Loop over the Queens.
   for ( int pieceIndex = 0; pieceIndex < numberOfPieces; pieceIndex++ )
   {
      
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mQN[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMN[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dBlackQueen);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mQNE[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMNE[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dBlackQueen);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mQE[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOME[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dBlackQueen);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mQSE[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMSE[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dBlackQueen);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mQS[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMS[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dBlackQueen);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mQSW[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMSW[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dBlackQueen);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mQW[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMW[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dBlackQueen);
      SliderAttacks( argsMoves, 
                     argsBoard, 
                     argsGeneralMoves, 
                     argsGeneralMoves->mQNW[ vPieces[ pieceIndex ] ], 
                     argsGeneralMoves->vQNOMNW[ vPieces[ pieceIndex ] ], 
                     vPieces[ pieceIndex ], 
                     dBlackQueen);

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateBlackKingAttacks( struct Move * argsMoves, 
                                struct Board * argsBoard, 
                                struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Do the King.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbBlackKing, vPieces, argsGeneralMoves );
   int index = 0;

   int vAttacks[ 8 ];

   // Look for the good moves..
   BitBoard bbAttacks = argsGeneralMoves->bbKMove[ vPieces[ index ] ] &
                        ( argsBoard->bbWhitePieces );

   argsBoard->bbBlackAttack = argsBoard->bbBlackAttack | bbAttacks;

   // Enter the moves.
   int numberOfAttacks = Find( bbAttacks, vAttacks, argsGeneralMoves );

   // Enter the attacks.
   for ( int attackIndex = 0; attackIndex < numberOfAttacks; attackIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vPieces[ index ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vAttacks[ attackIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPieces[ index ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vAttacks[ attackIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dCapture;
	   argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackKing;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msCaptureDown;

   }

}

//
//-------------------------------------------------------------------------------------------------
//
void ConvertIntToCol( int iCol,
                      char * strCol )
{

   // Debug the inputs
   assert( iCol >= 0 );
   assert( iCol <= 8 );

   // This function converts an integer to the equilivant column character.
   //char strCol[ 1 ];
   
   // First a few checks.
   if ( iCol < 0 )
   {
   
      cout << "iCol < 0" << endl;
      system( "pause" );
      
   }
   if ( iCol > 7 )
   {
   
      cout << "iCol > 7" << endl;
      system( "pause" );
      
   }
   
   switch( iCol )
   {
   
      case 0 : 
      {
      
         strncpy( & strCol[ 0 ], "1", 1 );
         break;
         
      }
      case 1 : 
      {
      
         strncpy( & strCol[ 0 ], "2", 1 );
         break;
         
      }
      case 2 : 
      {
      
         strncpy( & strCol[ 0 ], "3", 1 );
         break;
         
      }
      case 3 : 
      {
      
         strncpy( & strCol[ 0 ], "4", 1 );
         break;
         
      }
      case 4 : 
      {
      
         strncpy( & strCol[ 0 ], "5", 1 );
         break;
         
      }
      case 5 : 
      {
      
         strncpy( & strCol[ 0 ], "6", 1 );
         break;
         
      }
      case 6 : 
      {
      
         strncpy( & strCol[ 0 ], "7", 1 );
         break;

      }
      case 7 : 
      {
      
         strncpy( & strCol[ 0 ], "8", 1 );
         break;
         
      }
      
   }
   
}


//
//-------------------------------------------------------------------------------------------------
//
void ConvertIntToRow( int iRow,
                      char * strRow )
{
   // Debug the inputs
   assert( iRow >= 0 );
   assert( iRow <= 8 );

   // This function converts an integer to the equilivant Rowumn character.
   //char strRow[ 1 ];
   
   // First a few checks.
   if ( iRow < 0 )
   {
   
      cout << "iRow < 0" << endl;
      system( "pause" );
      
   }
   if ( iRow > 7 )
   {
   
      cout << "iRow > 7" << endl;
      system( "pause" );
      
   }
   
   switch( iRow )
   {
   
      case 0 : 
      {
      
         strncpy( & strRow[ 0 ], "a", 1 );
         break;
         
      }
      case 1 : 
      {
      
         strncpy( & strRow[ 0 ], "b", 1 );
         break;
         
      }
      case 2 : 
      {
      
         strncpy( & strRow[ 0 ], "c", 1 );
         break;
         
      }
      case 3 : 
      {
         
         strncpy( & strRow[ 0 ], "d", 1 );
         break;
         
      }
      case 4 : 
      {
      
         strncpy( & strRow[ 0 ], "e", 1 );
         break;
         
      }
      case 5 : 
      {
      
         strncpy( & strRow[ 0 ], "f", 1 );
         break;
         
      }
      case 6 : 
      {
      
         strncpy( & strRow[ 0 ], "g", 1 );
         break;
         
      }
      case 7 : 
      {
      
         strncpy( & strRow[ 0 ], "h", 1 );
         break;
         
      }
      
   }
   
}

//
//-------------------------------------------------------------------------------------------------------
//
void ConvertRowToInt( char * strRow,
                      int * iRow )
{

   // Convert an input string to an integer column.

   // Convert the string to lower case
   * iRow = (int)( tolower( * strRow ) - 97 );

   // A check.
   if ( ( * iRow < 0 ) || ( * iRow > 8 ) )
   {

      cout << "ConvertRowToInt failed." << endl;
      system( "pause" );

   }

}

void ConvertColToInt( char * strCol,
                      int * iCol )
{

   // Convert an input string to an integer column.

   * iCol = ( (int)*strCol - 49 );

   // A check
   if ( ( * iCol < 0 ) || ( * iCol > 8 ) )
   {

      cout << "ConverColToInt failed." << endl;
      system( "pause" );

   }

}

//
//------------------------------------------------------------------------------------
//
int GetMoveNumber( struct Board * argsBoard,
                   struct GeneralMove * argsGeneralMoves,
                   struct Move * argsMoveList,
                   char * argstrMove )
{

   // Debug the inputs.
   assert( argsMoveList >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

// This function gets a move to match an inputted move string
   int iNumberOfMoves;
   int viLegalMove[ dNumberOfMoves ];
   int iMoveNumber;
   int iNumberOfCharacters;
   char strUniverseOfMoves[ dNumberOfMoves][ 10 ];
   int iCharacterIndex;
   int iNumberInUniverse;
   Move vsDummyMoveList[ dNumberOfMoves ];
   int iFoundMove;
   int iStringLength;
   int iMatch;
   int iMoveIndex;
   char strMove[ 64 ];
   //char strDummyMove[ 64 ];
   int iCharIndex;
                   
   iNumberOfMoves = argsBoard->siNumberOfMoves;

   // Initialize the character array.
   for ( iMoveNumber = 0; iMoveNumber < iNumberOfMoves; iMoveNumber++ )
   {

      for ( iCharIndex = 0; iCharIndex < 10; iCharIndex++ )
      {

         strcpy( & strUniverseOfMoves[ iMoveNumber ][ iCharIndex ], " " );

      }

   }
   
   // Loop over the moves and store the appropriate moves in strings.
   iNumberInUniverse = 0;
   for ( iMoveNumber = 0; iMoveNumber < iNumberOfMoves; iMoveNumber++ )
   {

      // Check to see if the move is legal.
      MakeMove( argsMoveList,
                argsBoard,
                argsGeneralMoves,
                iMoveNumber );

      // Generate all of the legal moves.
      CalculateMoves( vsDummyMoveList, 
                      argsBoard, 
                      argsGeneralMoves );

      // Check the state of the board.
      LegalState( argsBoard,
                  argsGeneralMoves );

      // Record the legality.
      viLegalMove[ iMoveNumber ] = argsBoard->siLegalMove;

      // Undo the move.
      UndoMove( argsBoard,
                argsGeneralMoves );

      // Clear the string.
      strcpy( strMove, "         " );

      // Print out the move.
      iNumberOfCharacters = PrintMove( argsBoard,
                                       argsGeneralMoves,
                                       & argsMoveList[ iMoveNumber ],
                                       strMove );

      // Put the move in storage.
      for ( iCharacterIndex = 0; iCharacterIndex < iNumberOfCharacters; iCharacterIndex++ )
      {
   
         strncpy( & strUniverseOfMoves[ iMoveNumber ][ iCharacterIndex ], & strMove[ iCharacterIndex ], 1 );
         
      }
      
   }
   
   // See if the move is in the move list.
   iFoundMove = 0;
   iStringLength = strlen( argstrMove );
   for ( iMoveNumber = 0; iMoveNumber < iNumberOfMoves; iMoveNumber++ )
   {
   
      // Loop over the length of the string.
      iMatch = 1;

      // Only look at legal moves.
      if ( viLegalMove[ iMoveNumber ] == 1 )
      {

         for ( iCharacterIndex = 0; iCharacterIndex < iStringLength; iCharacterIndex++ )
         {
         
            // Look for a mismatch. 
            if ( strncmp( & argstrMove[ iCharacterIndex ], 
                          & strUniverseOfMoves[ iMoveNumber ][ iCharacterIndex ],
                          1 ) )          
            {
               
               // Assign the mismatch.
               iMatch = 0;
               break;
         
            }
         
         }

      }
      else
      {
         
         // In this case we have an illegal move and there is not a match.
         iMatch = 0;

      }
      
      // Look for a perfect match.
      if ( iMatch == 1 )
      {
      
         iFoundMove = 1;
         iMoveIndex = iMoveNumber;
         break;
         
      }
      
   }
   
   // If the move was not found, try to get another.
   if ( iFoundMove == 0 )
   {

         cout << "Move was not found, try again and look for degeneracy." << endl;
         PrintFullBoard( argsBoard );
         PrintFEN( argsBoard,
                   argsGeneralMoves );
         system( "pause" );
         PrintBoard( argsBoard->mBoard  );
         return -1;
         
   }

   return iMoveNumber;
}


//
//------------------------------------------------------------------------------------
//
int GetMoveNumberFast( struct Board * argsBoard,
                       struct GeneralMove * argsGeneralMoves,
                       struct Move * argsMoveList,
                       char * argstrMove )
{

   // Debug the inputs.
   assert( argsMoveList >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

// This function gets a move to match an inputted move string
   int iNumberOfMoves;
   int viLegalMove[ dNumberOfMoves ];
   int iMoveNumber;
   int iNumberOfCharacters;
   char strUniverseOfMoves[ dNumberOfMoves][ 10 ];
   int iCharacterIndex;
   int iNumberInUniverse;
   Move vsDummyMoveList[ dNumberOfMoves ];
   int iFoundMove;
   int iStringLength;
   int iMatch;
   int iMoveIndex;
   char strMove[ 64 ];
   //char strDummyMove[ 64 ];
   int iCharIndex;
                   
   iNumberOfMoves = argsBoard->siNumberOfMoves;

   // Initialize the character array.
   for ( iMoveNumber = 0; iMoveNumber < iNumberOfMoves; iMoveNumber++ )
   {

      for ( iCharIndex = 0; iCharIndex < 10; iCharIndex++ )
      {

         strcpy( & strUniverseOfMoves[ iMoveNumber ][ iCharIndex ], " " );

      }

   }
   
   // Loop over the moves and store the appropriate moves in strings.
   iNumberInUniverse = 0;
   for ( iMoveNumber = 0; iMoveNumber < iNumberOfMoves; iMoveNumber++ )
   {

      // Check to see if the move is legal.
      MakeMove( argsMoveList,
                argsBoard,
                argsGeneralMoves,
                iMoveNumber );

      // Generate all of the legal moves.
      CalculateMoves( vsDummyMoveList, 
                      argsBoard, 
                      argsGeneralMoves );

      // Check the state of the board.
      LegalState( argsBoard,
                  argsGeneralMoves );

      // Record the legality.
      viLegalMove[ iMoveNumber ] = argsBoard->siLegalMove;

      // Undo the move.
      UndoMove( argsBoard,
                argsGeneralMoves );

      // Clear the string.
      strcpy( strMove, "         " );

      // Print out the move.
      iNumberOfCharacters = PrintMoveFast( argsBoard,
                                           argsGeneralMoves,
                                           & argsMoveList[ iMoveNumber ],
                                           strMove );

      // Put the move in storage.
      for ( iCharacterIndex = 0; iCharacterIndex < iNumberOfCharacters; iCharacterIndex++ )
      {
   
         strncpy( & strUniverseOfMoves[ iMoveNumber ][ iCharacterIndex ], & strMove[ iCharacterIndex ], 1 );
         
      }
      
   }
   
   // See if the move is in the move list.
   iFoundMove = 0;
   iStringLength = strlen( argstrMove );
   for ( iMoveNumber = 0; iMoveNumber < iNumberOfMoves; iMoveNumber++ )
   {
   
      // Loop over the length of the string.
      iMatch = 1;

      // Only look at legal moves.
      if ( viLegalMove[ iMoveNumber ] == 1 )
      {

         for ( iCharacterIndex = 0; iCharacterIndex < iStringLength; iCharacterIndex++ )
         {
         
            // Skip over checkmate to make this fast.
            if ( strncmp( & argstrMove[ iCharacterIndex ],
                          "#",
                          1 ) != 0 )
            {

               // Skip over check to make this fast.
               if ( strncmp( & argstrMove[ iCharacterIndex ],
                             "+",
                             1 ) != 0 )
               {
               

                  // Look for a mismatch. 
                  if ( strncmp( & argstrMove[ iCharacterIndex ], 
                                & strUniverseOfMoves[ iMoveNumber ][ iCharacterIndex ],
                                1 ) )          
                  {
                     
                     // Assign the mismatch.
                     iMatch = 0;
                     break;
               
                  }

               }

            }
         
         }

      }
      else
      {
         
         // In this case we have an illegal move and there is not a match.
         iMatch = 0;

      }
      
      // Look for a perfect match.
      if ( iMatch == 1 )
      {
      
         iFoundMove = 1;
         iMoveIndex = iMoveNumber;
         break;
         
      }
      
   }
   
   // If the move was not found, try to get another.
   if ( iFoundMove == 0 )
   {

      return -1;
         
   }

   // Debug.
   assert( iMoveNumber >= 0 );
   assert( iMoveNumber <= dNumberOfMoves );

   // Return the move number.
   return iMoveNumber;

}

//
//-------------------------------------------------------------------------------------------------
//
void FindAlgebraicMove( struct Board * argsBoard,
                        struct GeneralMove * argsGeneralMoves,
                        char * argstrMove,
                        struct Move * argsMoveList,
                        int * argiMoveNumber )
{

   // Debug the inputs.
   assert( argsMoveList >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // This function assumes an array of precalculated moves and a set up board.
   // This function translates in an input string of algebraic moves to a index
   // number of the moves in an array.

   // Initialize the variables.
   int viRow[ 2 ];
   int viCol[ 2 ];
   int viSquares[ 2 ];
   int siLength = 2;
   int iStringLength = 0;
   char strPiece[ 1 ];
   strncpy( strPiece, " ", 1 );
   int iPromotionPiece = -1;
   int iMatch = 0;

   // Parse the string.  It is assumed that the move notation looks like:
   // Examples:  e2e4, e7e5, e1g1 (white short castling), e7e8q (for promotion)

   // Parse the string.
   ConvertRowToInt( & argstrMove[ 0 ], & viRow[ 0 ] );
   ConvertColToInt( & argstrMove[ 1 ], & viCol[ 0 ] );
   ConvertRowToInt( & argstrMove[ 2 ], & viRow[ 1 ] );
   ConvertColToInt( & argstrMove[ 3 ], & viCol[ 1 ] );

   // Calculate the squares.
   CalculateSquares( viRow,
                     viCol,
                     viSquares,
                     & siLength );

   // Look for a promotion
   iStringLength = strlen( argstrMove );
   if ( iStringLength > 4 )
   {
   
      // Note, the UCI interface from Fritz is not putting an equal sign in front of a promotion.
      // This assumes no equal sign.
      strncpy( strPiece, & argstrMove[ 4 ], 1 );
      * strPiece = tolower( * strPiece );

      if ( argsBoard->siColorToMove == dWhite )
      {

         if ( ! strncmp( & strPiece[ 0 ], "q", 1 ) )
         {

            iPromotionPiece = dWhiteQueen;

         }
      
         if ( ! strncmp( & strPiece[ 0 ], "r", 1 ) )
         {
   
            iPromotionPiece = dWhiteRook;

         }

         if ( ! strncmp( & strPiece[ 0 ], "b", 1 ) )
         {

            iPromotionPiece = dWhiteBishop;

         }

         if ( ! strncmp( & strPiece[ 0 ], "n", 1 ) )
         {

            iPromotionPiece = dWhiteKnight;

         }

      }
      else      
      {

         if ( ! strncmp( & strPiece[ 0 ], "q", 1 ) )
         {

            iPromotionPiece = dBlackQueen;

         }
      
         if ( ! strncmp( & strPiece[ 0 ], "r", 1 ) )
         {
   
            iPromotionPiece = dBlackRook;

         }

         if ( ! strncmp( & strPiece[ 0 ], "b", 1 ) )
         {

            iPromotionPiece = dBlackBishop;

         }

         if ( ! strncmp( & strPiece[ 0 ], "n", 1 ) )
         {

            iPromotionPiece = dBlackKnight;

         }


      }

   }

   // Loop over the moves and look for a match.
   for ( int iMoveIndex = 0; iMoveIndex < argsBoard->siNumberOfMoves; iMoveIndex++ )
   {

      // Look for a match.
      if ( ( viSquares[ 0 ] == argsMoveList[ iMoveIndex ].iFromSquare ) &&
           ( viSquares[ 1 ] == argsMoveList[ iMoveIndex ].iToSquare ) )
      {

         // If a promotion, make sure the pieces match.
         if ( iStringLength > 4 )
         {

            if ( iPromotionPiece == argsMoveList[ iMoveIndex ].iPiece )
            {

               * argiMoveNumber = iMoveIndex;
               break;

            }
         
         }
         else
         {
            
            * argiMoveNumber = iMoveIndex;
            break;

         }

      }

   }

}

//
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Take a normal move and convert in into an algbraic move that can be read by the interface.
//
void CreateAlgebraicMove( char * argstrMove,
                          struct Move * argsMoveList,
                          int argiMoveNumber )
{

   // Debug the inputs.
   assert( argsMoveList >= 0 );
   //assert( argsBoard >= 0 );
   //assert( argsGeneralMoves >= 0 );

   // Declare the string variables.
   char strRowStart;
   char strRowEnd;
   char strColStart[ 6 ];
   char strColEnd[ 3 ];
   //char strPiece;

   strcpy( argstrMove, "      " );

   // Extract the characters
   ConvertIntToRow( dRow( argsMoveList[ argiMoveNumber ].iFromSquare ) ,
                    & strRowStart ); 
   sprintf( & strColStart[ 0 ], 
            "%d", 
            dCol( argsMoveList[ argiMoveNumber ].iFromSquare ) + 1 );
   ConvertIntToRow( dRow( argsMoveList[ argiMoveNumber ].iToSquare ),
                    & strRowEnd );  
   sprintf( & strColEnd[ 0 ], 
            "%d", 
            dCol( argsMoveList[ argiMoveNumber ].iToSquare ) + 1 );

//*
   strncpy( & argstrMove[ 0 ], & strRowStart, 1 );
   strncpy( & argstrMove[ 1 ], & strColStart[ 0 ], 1 );
   strncpy( & argstrMove[ 2 ], & strRowEnd, 1 );
   strncpy( & argstrMove[ 3 ], & strColEnd[ 0 ], 1 );
//*/

//   sprintf( argstrMove, "%s%s%s%s", argstrMove[ 0 ], argstrMove[ 1 ], argstrMove[ 2 ], argstrMove[ 3 ] );

   // Handle a promotion.
   if ( argsMoveList[ argiMoveNumber ].iMoveType == dPromote )
   {

      strncpy( & argstrMove[ 4 ], "=", 1 );

      if ( ( argsMoveList[ argiMoveNumber ].iPiece == dWhiteRook ) ||
           ( argsMoveList[ argiMoveNumber ].iPiece == dBlackRook ) )
      {
   
         strncpy( & argstrMove[ 5 ], "R", 1 );
      
      }
      else if ( ( argsMoveList[ argiMoveNumber ].iPiece == dWhiteKnight ) ||
                ( argsMoveList[ argiMoveNumber ].iPiece == dBlackKnight ) )
      {
   
         strncpy( & argstrMove[ 5 ], "N", 1 );
      
      }
      else if ( ( argsMoveList[ argiMoveNumber ].iPiece == dWhiteBishop ) ||
                ( argsMoveList[ argiMoveNumber ].iPiece == dBlackBishop ) )
      {
   
         strncpy( & argstrMove[ 5 ], "B", 1 );
      
      }
      else if ( ( argsMoveList[ argiMoveNumber ].iPiece == dWhiteQueen ) ||
                ( argsMoveList[ argiMoveNumber ].iPiece == dBlackQueen ) )
      {
   
         strncpy( & argstrMove[ 5 ], "Q", 1 );
      
      }

   }
   
}


//
//---------------------------------------------------------------------------------
//
void MakeMove( struct Move * argsMoves, 
               struct Board * argsBoard, 
               struct GeneralMove * argsGeneralMoves,
               int iMoveNumber )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );
   assert( iMoveNumber >= 0 );
   assert( iMoveNumber <= dNumberOfMoves );
   assert( CheckBoard( argsBoard ) );

//  This function is used to make a move on the existing board.
//

   // Reset the best move.
   argsBoard->iBestMove = 128;

   // Update the hisotry count.
   argsBoard->iMoveHistory++;   

   // Extract the squares of interest.
   int startSquare = argsMoves[ iMoveNumber ].iFromSquare;
   int endSquare   = argsMoves[ iMoveNumber ].iToSquare;

   // Put the squares in the history stack.
   argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iFromSquare  = argsMoves[ iMoveNumber ].iFromSquare;
   argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iToSquare    = argsMoves[ iMoveNumber ].iToSquare;
   argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbFromSquare = argsMoves[ iMoveNumber ].bbFromSquare;
   argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbToSquare   = argsMoves[ iMoveNumber ].bbToSquare;
   argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iCapture     = argsBoard->mBoard[ endSquare ];
   argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iPiece       = argsBoard->mBoard[ startSquare ]; 
   argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare   = argsMoves[ iMoveNumber ].bbEPSquare; 
   argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbCastle     = argsBoard->bbCastle;
   argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbCheck      = argsBoard->bbCheck;
   argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iMoveType    = argsMoves[ iMoveNumber ].iMoveType;
   argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iPromote     = 0; 

   // Update the hash.
   //cout << "    Hash before UpdateHash() = " << GetHash() << endl;
   UpdateHash( & argsMoves[ iMoveNumber ],
               argsBoard,
               0,
               argsGeneralMoves ); 
   //cout << "    Hash after UpdateHash() = " << GetHash() << endl;


   argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbHash = GetHash(); 
   //argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbHash = gsHashTable.bbHash; 

   // Get the piece number on the square we are moving to.
   int capturedPiece = argsBoard->mBoard[ endSquare ];

   switch ( capturedPiece )
   {

      case dWhitePawn :
      {

         argsBoard->bbWhitePawn   = SetBitToZero( argsBoard->bbWhitePawn, endSquare );
         break;

      }

      case dWhiteRook :
      {

         argsBoard->bbWhiteRook   = SetBitToZero( argsBoard->bbWhiteRook, endSquare );

         // Take away the ability to castle.
         if ( endSquare == dA1 )
         {

            argsBoard->bbCastle = SetBitToZero( argsBoard->bbCastle, 2 );

         }

         if ( endSquare == dH1 )
         {

            argsBoard->bbCastle = SetBitToZero( argsBoard->bbCastle, 0 );

         }
         break;

      }

      case dWhiteKnight :
      {

         argsBoard->bbWhiteKnight = SetBitToZero( argsBoard->bbWhiteKnight, endSquare );
         break;

      }

      case dWhiteBishop :
      {

         argsBoard->bbWhiteBishop = SetBitToZero( argsBoard->bbWhiteBishop, endSquare );
         break;

      }

      case dWhiteQueen :
      {

         argsBoard->bbWhiteQueen  = SetBitToZero( argsBoard->bbWhiteQueen, endSquare );
         break;
   
      }

      case dWhiteKing :
      {

         argsBoard->bbWhiteKing  = SetBitToZero( argsBoard->bbWhiteKing, endSquare );

         // Take away the ability to castle.
         argsBoard->bbCastle = SetBitToZero( argsBoard->bbCastle, 0 );
         argsBoard->bbCastle = SetBitToZero( argsBoard->bbCastle, 2 );
         break;

      }

      case dBlackPawn :
      {

         argsBoard->bbBlackPawn   = SetBitToZero( argsBoard->bbBlackPawn, endSquare );
         break;

      }

      case dBlackRook :
      {

         argsBoard->bbBlackRook   = SetBitToZero( argsBoard->bbBlackRook, endSquare );

         // Take away the ability to castle :
         if ( endSquare == dA8 )
         {

            argsBoard->bbCastle = SetBitToZero( argsBoard->bbCastle, 3 );

         }

         if ( endSquare == dH8 )
         {

            argsBoard->bbCastle = SetBitToZero( argsBoard->bbCastle, 1 );

         }
         break;

      }

      case dBlackKnight :
      {

         argsBoard->bbBlackKnight = SetBitToZero( argsBoard->bbBlackKnight, endSquare );
         break;

      }

      case dBlackBishop :
      {

         argsBoard->bbBlackBishop = SetBitToZero( argsBoard->bbBlackBishop, endSquare );
         break;

      }

      case dBlackQueen :
      {

         argsBoard->bbBlackQueen  = SetBitToZero( argsBoard->bbBlackQueen, endSquare );
         break;

      }

      case dBlackKing :
      {

         argsBoard->bbBlackKing  = SetBitToZero( argsBoard->bbBlackKing, endSquare );

         // Take away the ability to castle.
         argsBoard->bbCastle = SetBitToZero( argsBoard->bbCastle, 3 );
         argsBoard->bbCastle = SetBitToZero( argsBoard->bbCastle, 1 );
         break;

      }

   }
   
   // Switch on the piece.
   switch ( argsBoard->mBoard[ startSquare ] )
   {
   
      case dWhitePawn :
      {
           
         argsBoard->bbWhitePawn   = SetBitToOne( argsBoard->bbWhitePawn, endSquare );
         argsBoard->bbWhitePawn   = SetBitToZero( argsBoard->bbWhitePawn, startSquare );

         // Do the En Passant thing.
         if ( argsMoves[ iMoveNumber ].iMoveType == dEnPassant )
         {

            argsBoard->bbBlackPawn = SetBitToZero( argsBoard->bbBlackPawn, endSquare - 8 );
            argsBoard->mBoard[ endSquare - 8 ] =  0;
            argsBoard->bbEP = 0;

         }

         // If a two square more, set the en passant square.
         if ( argsMoves[ iMoveNumber ].iMoveType == dTwoSquare )
         {

            argsBoard->bbEP = argsMoves[ iMoveNumber ].bbEPSquare;

         }

         // Do the promotions.
         if ( ( argsMoves[ iMoveNumber ].iMoveType == dPromote ) ||
              ( argsMoves[ iMoveNumber ].iMoveType == dCaptureAndPromote ) )
         {

            argsBoard->bbWhitePawn = argsBoard->bbWhitePawn & ( ~ argsMoves[ iMoveNumber ].bbToSquare );
            
            switch ( argsMoves[ iMoveNumber ].iPiece )
            {

               case dWhiteKnight :
               {

                  argsBoard->mBoard[ endSquare ] = dWhiteKnight;
                  argsBoard->bbWhiteKnight = argsBoard->bbWhiteKnight | argsMoves[ iMoveNumber ].bbToSquare;
                  argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iPiece = dWhiteKnight; 
                  break;

               }

               case dWhiteBishop :
               {

                  argsBoard->mBoard[ endSquare ] = dWhiteBishop;
                  argsBoard->bbWhiteBishop = argsBoard->bbWhiteBishop | argsMoves[ iMoveNumber ].bbToSquare;
                  argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iPiece = dWhiteBishop; 
                  break;

               }

               case dWhiteRook :
               {

                  argsBoard->mBoard[ endSquare ] = dWhiteRook;
                  argsBoard->bbWhiteRook = argsBoard->bbWhiteRook | argsMoves[ iMoveNumber ].bbToSquare;
                  argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iPiece = dWhiteRook; 
                  break;

               }

               case dWhiteQueen :
               {

                  argsBoard->mBoard[ endSquare ] = dWhiteQueen;
                  argsBoard->bbWhiteQueen = argsBoard->bbWhiteQueen | argsMoves[ iMoveNumber ].bbToSquare;
                  argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iPiece = dWhiteQueen; 
                  break;

               }

            }

         }

         break;

      }

      case dWhiteRook :
      {

         argsBoard->bbWhiteRook = SetBitToOne( argsBoard->bbWhiteRook, endSquare );
         argsBoard->bbWhiteRook = SetBitToZero( argsBoard->bbWhiteRook, startSquare );

         if ( startSquare == dA1 )
         {

            argsBoard->bbCastle = SetBitToZero( argsBoard->bbCastle, 2 );

         }

         if ( startSquare == dH1 )
         {

            argsBoard->bbCastle = SetBitToZero( argsBoard->bbCastle, 0 );

         }
         break;

      }

      case dWhiteKnight :
      {

         argsBoard->bbWhiteKnight = SetBitToOne( argsBoard->bbWhiteKnight, endSquare );
         argsBoard->bbWhiteKnight = SetBitToZero( argsBoard->bbWhiteKnight, startSquare );
         break;

      }

      case dWhiteBishop :
      {

         argsBoard->bbWhiteBishop = SetBitToOne( argsBoard->bbWhiteBishop, endSquare );
         argsBoard->bbWhiteBishop = SetBitToZero( argsBoard->bbWhiteBishop, startSquare );
         break;
            
      }

      case dWhiteQueen :
      {

         argsBoard->bbWhiteQueen  = SetBitToOne( argsBoard->bbWhiteQueen, endSquare );
         argsBoard->bbWhiteQueen  = SetBitToZero( argsBoard->bbWhiteQueen, startSquare );
         break;

      }

      case dWhiteKing :
      {

         argsBoard->bbWhiteKing  = SetBitToOne( argsBoard->bbWhiteKing, endSquare );
         argsBoard->bbWhiteKing  = SetBitToZero( argsBoard->bbWhiteKing, startSquare );

         // Take away the ability to castle.
         argsBoard->bbCastle = SetBitToZero( argsBoard->bbCastle, 2 );
         argsBoard->bbCastle = SetBitToZero( argsBoard->bbCastle, 0 );

         // Move the rook if it is a castle.
         if (  argsMoves[ iMoveNumber ].iMoveType == dWhiteKingSideCastle )
         {

            // Move the rook.
            argsBoard->bbWhiteRook = SetBitToZero( argsBoard->bbWhiteRook, dH1 );
            argsBoard->bbWhiteRook = SetBitToOne( argsBoard->bbWhiteRook, dF1 );
            argsBoard->mBoard[ dH1 ] = dEmpty;
            argsBoard->mBoard[ dF1 ] = dWhiteRook;
            argsBoard->iHasWhiteCastled = dYes;

         }

         if (  argsMoves[ iMoveNumber ].iMoveType == dWhiteQueenSideCastle )
         {

            // Move the rook.
            argsBoard->bbWhiteRook = SetBitToZero( argsBoard->bbWhiteRook, dA1 );
            argsBoard->bbWhiteRook = SetBitToOne( argsBoard->bbWhiteRook, dD1 );
            argsBoard->mBoard[ dA1 ] = dEmpty;
            argsBoard->mBoard[ dD1 ] = dWhiteRook;
            argsBoard->iHasWhiteCastled = dYes;

         }
         break;

      }

      case dBlackPawn :
      {

         argsBoard->bbBlackPawn   = SetBitToOne( argsBoard->bbBlackPawn, endSquare );
         argsBoard->bbBlackPawn   = SetBitToZero( argsBoard->bbBlackPawn, startSquare );

         // Do the En Passant thing.
         if ( argsMoves[ iMoveNumber ].iMoveType == dEnPassant )
         {

            argsBoard->bbWhitePawn = SetBitToZero( argsBoard->bbWhitePawn, endSquare + 8 );
            argsBoard->mBoard[ endSquare + 8 ] =  0;
            argsBoard->bbEP = 0;

         }

         // If a two square more, set the en passant square.
         if ( argsMoves[ iMoveNumber ].iMoveType == dTwoSquare )
         {

            argsBoard->bbEP = argsMoves[ iMoveNumber ].bbEPSquare;

         }

         // Do the promotions.
         if ( ( argsMoves[ iMoveNumber ].iMoveType == dPromote ) ||
              ( argsMoves[ iMoveNumber ].iMoveType == dCaptureAndPromote ) )
         {

            argsBoard->bbBlackPawn = argsBoard->bbBlackPawn & ( ~ argsMoves[ iMoveNumber ].bbToSquare );
            
            switch ( argsMoves[ iMoveNumber ].iPiece )
            {

               case dBlackKnight :
               {

                  argsBoard->mBoard[ endSquare ] = dBlackKnight;
                  argsBoard->bbBlackKnight = argsBoard->bbBlackKnight | argsMoves[ iMoveNumber ].bbToSquare;
                  argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iPiece = dBlackKnight; 
                  break;

               }

               case dBlackBishop :
               {

                  argsBoard->mBoard[ endSquare ] = dBlackBishop;
                  argsBoard->bbBlackBishop = argsBoard->bbBlackBishop | argsMoves[ iMoveNumber ].bbToSquare;
                  argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iPiece = dBlackBishop; 
                  break;

               }

               case dBlackRook :
               {

                  argsBoard->mBoard[ endSquare ] = dBlackRook;
                  argsBoard->bbBlackRook = argsBoard->bbBlackRook | argsMoves[ iMoveNumber ].bbToSquare;
                  argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iPiece = dBlackRook; 
                  break;

               }

               case dBlackQueen :
               {

                  argsBoard->mBoard[ endSquare ] = dBlackQueen;
                  argsBoard->bbBlackQueen = argsBoard->bbBlackQueen | argsMoves[ iMoveNumber ].bbToSquare;
                  argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].iPiece = dBlackQueen; 
                  break;

               }

            }

         }

         break;

      }

      case dBlackRook :
      {

         argsBoard->bbBlackRook   = SetBitToOne( argsBoard->bbBlackRook, endSquare );
         argsBoard->bbBlackRook   = SetBitToZero( argsBoard->bbBlackRook, startSquare );

         if ( startSquare == dH8 )
         {

            argsBoard->bbCastle = SetBitToZero( argsBoard->bbCastle, 1 );

         }

         if ( startSquare == dA8 )
         {

            argsBoard->bbCastle = SetBitToZero( argsBoard->bbCastle, 3 );

         }
         break;

      }

      case dBlackKnight:
      {

         argsBoard->bbBlackKnight = SetBitToOne( argsBoard->bbBlackKnight, endSquare );
         argsBoard->bbBlackKnight = SetBitToZero( argsBoard->bbBlackKnight, startSquare );
         break;

      }

      case dBlackBishop :
      {

         argsBoard->bbBlackBishop = SetBitToOne( argsBoard->bbBlackBishop, endSquare );
         argsBoard->bbBlackBishop = SetBitToZero( argsBoard->bbBlackBishop, startSquare );
         break;

      }

      case dBlackQueen :
      {

         argsBoard->bbBlackQueen = SetBitToOne( argsBoard->bbBlackQueen, endSquare );
         argsBoard->bbBlackQueen = SetBitToZero( argsBoard->bbBlackQueen, startSquare );
         break;

      }

      case dBlackKing :
      {

         argsBoard->bbBlackKing = SetBitToOne( argsBoard->bbBlackKing, endSquare );
         argsBoard->bbBlackKing = SetBitToZero( argsBoard->bbBlackKing, startSquare );

         // Take away the ability to castle.
         argsBoard->bbCastle = SetBitToZero( argsBoard->bbCastle, 1 );
         argsBoard->bbCastle = SetBitToZero( argsBoard->bbCastle, 3 );

         // Move the rook if it is a castle.
         if (  argsMoves[ iMoveNumber ].iMoveType == dBlackKingSideCastle )
         {

            // Move the rook.
            argsBoard->bbBlackRook = SetBitToZero( argsBoard->bbBlackRook, dH8 );
            argsBoard->bbBlackRook = SetBitToOne( argsBoard->bbBlackRook, dF8 );
            argsBoard->mBoard[ dH8 ] = dEmpty;
            argsBoard->mBoard[ dF8 ] = dBlackRook;
            argsBoard->iHasBlackCastled = dYes;
            
         }

         if (  argsMoves[ iMoveNumber ].iMoveType == dBlackQueenSideCastle )
         {

            // Move the rook.
            argsBoard->bbBlackRook = SetBitToZero( argsBoard->bbBlackRook, dA8 );
            argsBoard->bbBlackRook = SetBitToOne( argsBoard->bbBlackRook, dD8 );
            argsBoard->mBoard[ dA8 ] = dEmpty;
            argsBoard->mBoard[ dD8 ] = dBlackRook;
            argsBoard->iHasBlackCastled = dYes;

         }
         break;
   
      }

   }

   // Update the board.
   if ( ( argsMoves[ iMoveNumber ].iMoveType != dPromote ) &&
        ( argsMoves[ iMoveNumber ].iMoveType != dCaptureAndPromote ) )
   {

      argsBoard->mBoard[ endSquare ] = argsBoard->mBoard[ startSquare ];
   
   }

   argsBoard->mBoard[ startSquare ] = 0;

   // Set up the white pieces
   argsBoard->bbWhitePieces = argsBoard->bbWhitePawn   | 
                              argsBoard->bbWhiteRook   | 
                              argsBoard->bbWhiteKnight | 
                              argsBoard->bbWhiteBishop | 
                              argsBoard->bbWhiteQueen  | 
                              argsBoard->bbWhiteKing;

   // Set up the black pieces
   argsBoard->bbBlackPieces = argsBoard->bbBlackPawn   | 
                              argsBoard->bbBlackRook   | 
                              argsBoard->bbBlackKnight | 
                              argsBoard->bbBlackBishop | 
                              argsBoard->bbBlackQueen  |
                              argsBoard->bbBlackKing;

   // Set up all the pieces.
   argsBoard->bbAllPieces = argsBoard->bbWhitePieces | argsBoard->bbBlackPieces;

   // Change the color to move.
   int colorChange = 0;
   if ( argsBoard->siColorToMove  == dWhite )
   {
 
      colorChange = dBlack;
 
   }
 
   if ( argsBoard->siColorToMove == dBlack )
   {
 
      colorChange = dWhite;
 
   }	

   argsBoard->siColorToMove = colorChange;

   // Update the ply.
   argsBoard->iNumberOfPlys++;
   if ( argsBoard->iNumberOfPlys > argsBoard->iMaxPlysReached )
   {

      argsBoard->iMaxPlysReached++;

   }

   // Debugging
   assert( CheckBoard( argsBoard ) );

}


//
//----------------------------------------------------------------------------------------
//
// Undo Move.
void UndoMove( struct Board * argsBoard, 
               struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );
   assert( CheckBoard( argsBoard ) );
/*
   if ( argsBoard->mBoard[ dH6 ] == dBlackKnight &&
        argsBoard->mBoard[ dA5 ] == dBlackPawn &&
        argsBoard->bbEP != 0 )
   {
      //PrintFullBoard( argsBoard );
      PrintBoard( argsBoard->mBoard );
   }
*/
   // Check the board.
   //CheckBoard( argsBoard );

   // This function undoes a move on the board.
	int iMoveNumber = argsBoard->iMoveHistory;
   assert( iMoveNumber >= 0 );
   assert( iMoveNumber <= dNumberOfMoves );

   assert( CheckBoard( argsBoard ) );

   // Undo the hash.
   if ( iMoveNumber == 0 )
   {
      SetHash( GetHashInitial() );
   }
   else
   {
      SetHash( argsBoard->sHistoryStack[ iMoveNumber - 1 ].bbHash );
   }
   //gsHashTable.bbHash = argsBoard->sHistoryStack[ iMoveNumber ].bbHash;

   // Put the captured piece or empty the square.        
   assert( argsBoard->sHistoryStack[ iMoveNumber ].iCapture >= dEmpty );
   assert( argsBoard->sHistoryStack[ iMoveNumber ].iCapture <= dBlackKing );
   assert( argsBoard->sHistoryStack[ iMoveNumber ].iToSquare >= dA1 );
   assert( argsBoard->sHistoryStack[ iMoveNumber ].iToSquare <= dH8 );
   argsBoard->mBoard[ argsBoard->sHistoryStack[ iMoveNumber ].iToSquare ] = 
                      argsBoard->sHistoryStack[ iMoveNumber ].iCapture;
         
   // Put the piece that moved back it's original position.
   assert( argsBoard->sHistoryStack[ iMoveNumber ].iPiece >= dEmpty );
   assert( argsBoard->sHistoryStack[ iMoveNumber ].iPiece <= dBlackKing );
   argsBoard->mBoard[ argsBoard->sHistoryStack[ iMoveNumber ].iFromSquare ] = 
                             argsBoard->sHistoryStack[ iMoveNumber ].iPiece;
   if ( ( argsBoard->sHistoryStack[ iMoveNumber ].iMoveType == dPromote ) ||
        ( argsBoard->sHistoryStack[ iMoveNumber ].iMoveType == dCaptureAndPromote ) )
   {
   
      if ( argsBoard->sHistoryStack[ iMoveNumber ].iPiece < dBlackPawn )
      {

         argsBoard->mBoard[ argsBoard->sHistoryStack[ iMoveNumber ].iFromSquare ] = dWhitePawn;
         
      }
      else if ( argsBoard->sHistoryStack[ iMoveNumber ].iPiece > dWhiteKing )
      {

         argsBoard->mBoard[ argsBoard->sHistoryStack[ iMoveNumber ].iFromSquare ] = dBlackPawn;
         
      }

   }

   assert( CheckBoard( argsBoard ) );

   // Update the EP things.
   argsBoard->bbEP = argsBoard->sHistoryStack[ iMoveNumber ].bbEPSquare;

   // Make the move legal agains.
   argsBoard->siLegalMove = 1;

   // Update castleing.
   argsBoard->bbCastle = argsBoard->sHistoryStack[ iMoveNumber ].bbCastle;
   
   // Set the old check.
   argsBoard->bbCheck = argsBoard->sHistoryStack[ iMoveNumber ].bbCheck;

   assert( CheckBoard( argsBoard ) );

   //Update the captures.
   switch ( argsBoard->sHistoryStack[ iMoveNumber ].iCapture )
   {

      case dEmpty :
      {
         
         break;

      }
    
      case dWhitePawn :
      {

         argsBoard->bbWhitePawn = argsBoard->bbWhitePawn |
                                  argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbAllPieces = argsBoard->bbAllPieces |
			                         argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbWhitePieces = argsBoard->bbWhitePieces |
			                          argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
         break;

      }
               
      case dWhiteRook :
      {

         argsBoard->bbWhiteRook = argsBoard->bbWhiteRook |
                                  argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbAllPieces = argsBoard->bbAllPieces |
			                         argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbWhitePieces = argsBoard->bbWhitePieces |
			                          argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
         break;

      }
         
      case dWhiteKnight :
      {

         argsBoard->bbWhiteKnight = argsBoard->bbWhiteKnight |
                                    argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbAllPieces = argsBoard->bbAllPieces |
			                         argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbWhitePieces = argsBoard->bbWhitePieces |
			                          argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
         break;

      }
               
      case dWhiteBishop :
      {

         argsBoard->bbWhiteBishop = argsBoard->bbWhiteBishop |
                                    argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbAllPieces = argsBoard->bbAllPieces |
			                         argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbWhitePieces = argsBoard->bbWhitePieces |
			                          argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;

         break;
      }
               
      // Put a white queen back on the board.
      case dWhiteQueen :
      {

         argsBoard->bbWhiteQueen = argsBoard->bbWhiteQueen |
                                   argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbAllPieces = argsBoard->bbAllPieces |
			                         argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbWhitePieces = argsBoard->bbWhitePieces |
			                          argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
         break;

      }
               
      // Put a white king back on the board.
      case dWhiteKing :
      {

         argsBoard->bbWhiteKing = argsBoard->bbWhiteKing |
                                   argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbAllPieces = argsBoard->bbAllPieces |
			                         argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbWhitePieces = argsBoard->bbWhitePieces |
			                          argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
         break;

      }
               
      // Put a black pawn back on the board.
      case dBlackPawn :
      {

         argsBoard->bbBlackPawn = argsBoard->bbBlackPawn |
                                  argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbAllPieces = argsBoard->bbAllPieces |
			                         argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbBlackPieces = argsBoard->bbBlackPieces |
			                          argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
         break;

      }
               
      // Put a black rook back on the board.
      case dBlackRook :
      {

         argsBoard->bbBlackRook = argsBoard->bbBlackRook |
                                  argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbAllPieces = argsBoard->bbAllPieces |
			                         argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbBlackPieces = argsBoard->bbBlackPieces |
			                          argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
         break;

      }
               
      // Put a black knight back on the board.
      case dBlackKnight :
      {

         argsBoard->bbBlackKnight = argsBoard->bbBlackKnight |
                                    argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbAllPieces = argsBoard->bbAllPieces |
			                         argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbBlackPieces = argsBoard->bbBlackPieces |
			                          argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
         break;

      }
               
      // Put a black bishop back on the board.
      case dBlackBishop :
      {

         argsBoard->bbBlackBishop = argsBoard->bbBlackBishop |
                                    argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbAllPieces = argsBoard->bbAllPieces |
			                         argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbBlackPieces = argsBoard->bbBlackPieces |
			                          argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
         break;

      }
               
      // Put a black queen back on the board.
      case dBlackQueen :
      {

         argsBoard->bbBlackQueen = argsBoard->bbBlackQueen |
                                  argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbAllPieces = argsBoard->bbAllPieces |
			                         argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbBlackPieces = argsBoard->bbBlackPieces |
			                          argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
         break;

      }
               
      // Put a black king back on the board.
      case dBlackKing :
      {

         argsBoard->bbBlackKing = argsBoard->bbBlackKing |
                                  argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbAllPieces = argsBoard->bbAllPieces |
			                         argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
		   argsBoard->bbBlackPieces = argsBoard->bbBlackPieces |
			                          argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
         break;

      }

   }

   assert( CheckBoard( argsBoard ) );

   // Put the pieces back on the board.
   if ( ( argsBoard->sHistoryStack[ iMoveNumber ].iMoveType == dPromote ) ||
        ( argsBoard->sHistoryStack[ iMoveNumber ].iMoveType == dCaptureAndPromote ) )
   {

      // Look for the color.
      if ( ( argsBoard->sHistoryStack[ iMoveNumber ].iPiece > 0 ) &&
           ( argsBoard->sHistoryStack[ iMoveNumber ].iPiece < dBlackPawn ) )
      {

         argsBoard->bbWhitePawn = argsBoard->bbWhitePawn |
                                  argsBoard->sHistoryStack[ iMoveNumber ].bbFromSquare;

         // Switch on the promotion piece.
         switch ( argsBoard->sHistoryStack[ iMoveNumber ].iPiece )
         {

            case dWhiteRook :
            {

               argsBoard->bbWhiteRook = argsBoard->bbWhiteRook & ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );

            }
            case dWhiteKnight :
            {

               argsBoard->bbWhiteKnight = argsBoard->bbWhiteKnight & ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );

            }
            case dWhiteBishop :
            {

               argsBoard->bbWhiteBishop = argsBoard->bbWhiteBishop & ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );

            }
            case dWhiteQueen :
            {

               argsBoard->bbWhiteQueen = argsBoard->bbWhiteQueen & ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );

            }

         }   
         
      }

   assert( CheckBoard( argsBoard ) );

      // Look for the color.
      if ( argsBoard->sHistoryStack[ iMoveNumber ].iPiece > dWhiteKing )
      {

         argsBoard->bbBlackPawn = argsBoard->bbBlackPawn |
                                  argsBoard->sHistoryStack[ iMoveNumber ].bbFromSquare;

         // Switch on the promotion piece.
         switch ( argsBoard->sHistoryStack[ iMoveNumber ].iPiece )
         {

            case dBlackRook :
            {

               argsBoard->bbBlackRook = argsBoard->bbBlackRook & ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );

            }
            case dBlackKnight :
            {

               argsBoard->bbBlackKnight = argsBoard->bbBlackKnight & ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );

            }
            case dBlackBishop :
            {

               argsBoard->bbBlackBishop = argsBoard->bbBlackBishop & ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );

            }
            case dBlackQueen :
            {

               argsBoard->bbBlackQueen = argsBoard->bbBlackQueen & ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );

            }

         }   
         
      }

   }
   else
   {

      switch ( argsBoard->sHistoryStack[ iMoveNumber ].iPiece )
      {

         case dWhitePawn :
         {

   assert( CheckBoard( argsBoard ) );
      	  
		      argsBoard->bbWhitePawn = argsBoard->bbWhitePawn &
			                            ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );
            argsBoard->bbWhitePawn = argsBoard->bbWhitePawn |
                                     argsBoard->sHistoryStack[ iMoveNumber ].bbFromSquare;

            // Undo the En Passant.
            if ( argsBoard->sHistoryStack[ iMoveNumber ].iMoveType == dEnPassant )
            {

               int iPosition = 0;
               int vPosition[ 1 ];
               iPosition = Find( argsBoard->sHistoryStack[ iMoveNumber - 1 ].bbEPSquare, 
                                 vPosition,
                                 argsGeneralMoves );
               argsBoard->bbBlackPawn = SetBitToOne( argsBoard->bbBlackPawn,
                                                     vPosition[ 0 ] - 8 );
               argsBoard->mBoard[ vPosition[ 0 ] - 8 ] = dBlackPawn;  

               //assert( CheckBoard( argsBoard ) );
               if ( !CheckBoard( argsBoard ) )
               {
   
                  PrintBitBoard( argsBoard->sHistoryStack[ iMoveNumber - 1 ].bbFromSquare );
                  PrintBitBoard( argsBoard->sHistoryStack[ iMoveNumber - 1 ].bbToSquare );
                  PrintBitBoard( argsBoard->sHistoryStack[ iMoveNumber - 1 ].bbEPSquare );
                  cout << "Hash = " << GetHash();

               }

            }

            break;

         }
                  
         // Put a white rook back on the board.
         case dWhiteRook :
         {
      	  
		      argsBoard->bbWhiteRook = argsBoard->bbWhiteRook &
			                            ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );
            argsBoard->bbWhiteRook = argsBoard->bbWhiteRook |
                                     argsBoard->sHistoryStack[ iMoveNumber ].bbFromSquare;
            break;
            
         }
                  
         // Put a white knight back on the board.
         case dWhiteKnight :
         {
      	  
		      argsBoard->bbWhiteKnight = argsBoard->bbWhiteKnight &
			                            ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );
            argsBoard->bbWhiteKnight = argsBoard->bbWhiteKnight |
                                     argsBoard->sHistoryStack[ iMoveNumber ].bbFromSquare;
            break;

	      }
                  
         // Put a white bishop back on the board.
         case dWhiteBishop :
         {
      	  
		      argsBoard->bbWhiteBishop = argsBoard->bbWhiteBishop &
			                            ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );
            argsBoard->bbWhiteBishop = argsBoard->bbWhiteBishop |
                                     argsBoard->sHistoryStack[ iMoveNumber ].bbFromSquare;
            break;

         }
                  
         // Put a white queen back on the board.
         case dWhiteQueen :
         {
      	  
		      argsBoard->bbWhiteQueen = argsBoard->bbWhiteQueen &
			                            ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );
            argsBoard->bbWhiteQueen = argsBoard->bbWhiteQueen |
                                     argsBoard->sHistoryStack[ iMoveNumber ].bbFromSquare;
            break;

	      }
                  
         // Put a white king back on the board.
         case dWhiteKing :
         {
      	  
		      argsBoard->bbWhiteKing = argsBoard->bbWhiteKing &
			                            ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );
            argsBoard->bbWhiteKing = argsBoard->bbWhiteKing |
                                     argsBoard->sHistoryStack[ iMoveNumber ].bbFromSquare;

            // Put the rook back if it was a castle.
            if ( argsBoard->sHistoryStack[ iMoveNumber ].iMoveType  == dWhiteKingSideCastle )
            {

               argsBoard->bbWhiteRook   = SetBitToZero( argsBoard->bbWhiteRook,   dF1 );
               argsBoard->bbWhiteRook   = SetBitToOne(  argsBoard->bbWhiteRook,   dH1 );
               argsBoard->bbWhitePieces = SetBitToZero( argsBoard->bbWhitePieces, dF1 );
               argsBoard->bbWhitePieces = SetBitToOne(  argsBoard->bbWhitePieces, dH1 );
               argsBoard->bbAllPieces   = SetBitToZero( argsBoard->bbAllPieces,   dF1 );
               argsBoard->bbAllPieces   = SetBitToZero( argsBoard->bbAllPieces,   dH1 );
               argsBoard->mBoard[ dF1 ] = dEmpty;
               argsBoard->mBoard[ dH1 ] = dWhiteRook;
               argsBoard->iHasWhiteCastled = dNo;

            }

            if ( argsBoard->sHistoryStack[ iMoveNumber ].iMoveType  == dWhiteQueenSideCastle )
            {

               argsBoard->bbWhiteRook   = SetBitToZero( argsBoard->bbWhiteRook,   dD1 );
               argsBoard->bbWhiteRook   = SetBitToOne(  argsBoard->bbWhiteRook,   dA1 );
               argsBoard->bbWhitePieces = SetBitToZero( argsBoard->bbWhitePieces, dD1 );
               argsBoard->bbWhitePieces = SetBitToOne(  argsBoard->bbWhitePieces, dA1 );
               argsBoard->bbAllPieces   = SetBitToZero( argsBoard->bbAllPieces,   dD1 );
               argsBoard->bbAllPieces   = SetBitToZero( argsBoard->bbAllPieces,   dA1 );
               argsBoard->mBoard[ dD1 ] = dEmpty;
               argsBoard->mBoard[ dA1 ] = dWhiteRook;
               argsBoard->iHasWhiteCastled = dNo;
          
            }

            break;

	      }
                  
         // Put a black pawn back on the board.
         case dBlackPawn :
         {
   assert( CheckBoard( argsBoard ) );
      	  
		      argsBoard->bbBlackPawn = argsBoard->bbBlackPawn &
			                            ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );
            argsBoard->bbBlackPawn = argsBoard->bbBlackPawn |
                                     argsBoard->sHistoryStack[ iMoveNumber ].bbFromSquare;

            // Undo the En Passant.
            if ( argsBoard->sHistoryStack[ iMoveNumber ].iMoveType == dEnPassant )
            {

               int iPosition = 0;
               int vPosition[ 1 ];
               iPosition = Find( argsBoard->sHistoryStack[ iMoveNumber - 1 ].bbEPSquare, 
                                 vPosition,
                                 argsGeneralMoves );
               argsBoard->bbWhitePawn = SetBitToOne( argsBoard->bbWhitePawn,
                                                     vPosition[ 0 ] + 8 );
               argsBoard->mBoard[ vPosition[ 0 ] + 8 ] = dWhitePawn;

               assert( CheckBoard( argsBoard ) );


            }

            break;

         }
                  
         // Put a black rook back on the board.
         case dBlackRook :
         {
      			  
		      argsBoard->bbBlackRook = argsBoard->bbBlackRook &
			                            ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );
            argsBoard->bbBlackRook = argsBoard->bbBlackRook |
                                     argsBoard->sHistoryStack[ iMoveNumber ].bbFromSquare;
            break;

         }
                  
         // Put a black knight back on the board.
         case dBlackKnight :
         {
      		
		      argsBoard->bbBlackKnight = argsBoard->bbBlackKnight &
			                            ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );
            argsBoard->bbBlackKnight = argsBoard->bbBlackKnight |
                                     argsBoard->sHistoryStack[ iMoveNumber ].bbFromSquare;
            break;

         }
                  
         // Put a black bishop back on the board.
         case dBlackBishop :
         {
      			  
		      argsBoard->bbBlackBishop = argsBoard->bbBlackBishop &
			                              ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );
            argsBoard->bbBlackBishop = argsBoard->bbBlackBishop |
                                       argsBoard->sHistoryStack[ iMoveNumber ].bbFromSquare;
            break;

	      }
                  
         // Put a black queen back on the board.
         case dBlackQueen :
         {
      			  
		      argsBoard->bbBlackQueen = argsBoard->bbBlackQueen &
			                             ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );
            argsBoard->bbBlackQueen = argsBoard->bbBlackQueen |
                                      argsBoard->sHistoryStack[ iMoveNumber ].bbFromSquare;
            break;

	      }
                  
         // Put a black King back on the board.
         case dBlackKing :
         {
      			  
		      argsBoard->bbBlackKing = argsBoard->bbBlackKing &
			                            ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );
            argsBoard->bbBlackKing = argsBoard->bbBlackKing |
                                     argsBoard->sHistoryStack[ iMoveNumber ].bbFromSquare;

            // Put the rook back if it was a castle.
            if ( argsBoard->sHistoryStack[ iMoveNumber ].iMoveType == dBlackKingSideCastle )
            {

               argsBoard->bbBlackRook   = SetBitToZero( argsBoard->bbBlackRook,   dF8 );
               argsBoard->bbBlackRook   = SetBitToOne(  argsBoard->bbBlackRook,   dH8 );
               argsBoard->bbBlackPieces = SetBitToZero( argsBoard->bbBlackPieces, dF8 );
               argsBoard->bbBlackPieces = SetBitToOne(  argsBoard->bbBlackPieces, dH8 );
               argsBoard->bbAllPieces   = SetBitToZero( argsBoard->bbAllPieces,   dF8 );
               argsBoard->bbAllPieces   = SetBitToZero( argsBoard->bbAllPieces,   dH8 );
               argsBoard->mBoard[ dF8 ] = dEmpty;
               argsBoard->mBoard[ dH8 ] = dBlackRook;
               argsBoard->iHasBlackCastled = dNo;

            }

            if ( argsBoard->sHistoryStack[ iMoveNumber ].iMoveType == dBlackQueenSideCastle )
            {

               argsBoard->bbBlackRook   = SetBitToZero( argsBoard->bbBlackRook,   dD8 );
               argsBoard->bbBlackRook   = SetBitToOne(  argsBoard->bbBlackRook,   dA8 );
               argsBoard->bbBlackPieces = SetBitToZero( argsBoard->bbBlackPieces, dD8 );
               argsBoard->bbBlackPieces = SetBitToOne(  argsBoard->bbBlackPieces, dA8 );
               argsBoard->bbAllPieces   = SetBitToZero( argsBoard->bbAllPieces,   dD8 );
               argsBoard->bbAllPieces   = SetBitToZero( argsBoard->bbAllPieces,   dA8 );
               argsBoard->mBoard[ dD8 ] = dEmpty;
               argsBoard->mBoard[ dA8 ] = dBlackRook;
               argsBoard->iHasBlackCastled = dNo;

            }

            break;

         }
      
      }

   }

   assert( CheckBoard( argsBoard ) );

   // Restore the moved piece to its original place on the board. 
	if ( argsBoard->sHistoryStack[ iMoveNumber ].iPiece > dWhiteKing )
	{

		argsBoard->bbBlackPieces = argsBoard->bbBlackPieces &
			                        ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );
		argsBoard->bbBlackPieces = argsBoard->bbBlackPieces |
			                        argsBoard->sHistoryStack[ iMoveNumber ].bbFromSquare;

	}

	else if ( ( argsBoard->sHistoryStack[ iMoveNumber ].iPiece > 0 ) &&
             ( argsBoard->sHistoryStack[ iMoveNumber ].iPiece < dBlackPawn ) )
	{

		argsBoard->bbWhitePieces = argsBoard->bbWhitePieces &
			                        ( ~ argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare );
		argsBoard->bbWhitePieces = argsBoard->bbWhitePieces |
			                        argsBoard->sHistoryStack[ iMoveNumber ].bbFromSquare;

	}

   assert( CheckBoard( argsBoard ) );

   argsBoard->bbAllPieces = 0;
	argsBoard->bbAllPieces = argsBoard->bbWhitePieces | argsBoard->bbBlackPieces;

   // Change the color to move.
   int colorChange = 0;
   if ( argsBoard->siColorToMove  == dWhite )
   {
 
      colorChange = dBlack;
 
   }
 
   if ( argsBoard->siColorToMove == dBlack )
   {
 
      colorChange = dWhite;
 
   }	

   argsBoard->siColorToMove = colorChange;

   // Update the history count.
	argsBoard->iMoveHistory--;

   // Update the ply.
   argsBoard->iNumberOfPlys--;

   assert( CheckBoard( argsBoard ) );


   // Update the moves
   Move sMove;
   sMove.bbFromSquare = argsBoard->sHistoryStack[ iMoveNumber ].bbFromSquare;
   sMove.bbToSquare   = argsBoard->sHistoryStack[ iMoveNumber ].bbToSquare;
   sMove.bbEPSquare   = argsBoard->sHistoryStack[ iMoveNumber ].bbEPSquare;
   sMove.iPiece       = argsBoard->sHistoryStack[ iMoveNumber ].iPiece;
   sMove.iFromSquare  = argsBoard->sHistoryStack[ iMoveNumber ].iFromSquare;
   sMove.iToSquare    = argsBoard->sHistoryStack[ iMoveNumber ].iToSquare;
   sMove.iPromote     = argsBoard->sHistoryStack[ iMoveNumber ].iPromote;
   sMove.iMoveType    = argsBoard->sHistoryStack[ iMoveNumber ].iMoveType;
   sMove.bbCastle     = argsBoard->sHistoryStack[ iMoveNumber ].bbCastle;
   sMove.iCapture     = argsBoard->sHistoryStack[ iMoveNumber ].iCapture;
   sMove.iScore       = 0;

   // Debugging
   assert( CheckBoard( argsBoard ) );

 }   

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void Ponder( struct Board * argsBoard,
             struct GeneralMove * argsGeneralMoves )
{

   // Define some variables.
   int iScore = 0;
   int iBookMove = -1;
   //Move vsMoveList[ dNumberOfMoves ];
   //char strMove[ 64 ];
   int iOldMode = 0;

   // This will automatically set up a ponder
   SetPonder( dYes );        // dYes dNo
   iOldMode = GetInterfaceMode();
   SetInterfaceMode( dUCI ); // dConsole dUCI dXboard

   // If we are still in book then move on.
   if ( GetIsInBook() == dYes )
   {

      return;

   }

   // Reset the plys.
   argsBoard->iNumberOfPlys = -1;

   // Reset the clocks
   argsBoard->cStart = clock();

   // Reset Alpha and beta
   int iAlpha = dAlpha;
   int iBeta  = dBeta;

   // Perform a search.
   iScore = StartSearch( argsBoard, 
                         argsGeneralMoves,
                         iAlpha,
                         iBeta );                                       

   // Turn control back over the console if need be.
   SetPonder( dNo );        // dYes dNo
   SetInterfaceMode( iOldMode ); // dConsole dUCI dXboard


}

//
//---------------------------------------------------------------------------------------------------------
//
// Initialize the communications.
void InitializeMoveDebug()
{

   gofDebugMoves.open( "MoveInterfaceLog.txt", ios::out | ios::app );
   gofDebugMoves << endl;
   gofDebugMoves << "Move log started." << endl << endl;


}

//
//---------------------------------------------------------------------------------------------------------
//
void CloseMoveDebug()
{

   // Close down the communications.
   gofDebugMoves << "Closing file." << endl;
   gofDebugMoves << endl;
   gofDebugMoves.close();
  

}



// Used for find the code for calculating quietes

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateQuiets( struct Move        * argsMoves, 
                      struct Board       * argsBoard, 
                      struct GeneralMove * argsGeneralMoves )
{
// This is a controlling function.
//

   // Debug the inputs.
   assert( argsMoves        >= 0 );
   assert( argsGeneralMoves >= 0 );
   assert( argsMoves        >= 0 );

   // Initialize the counters.
   argsBoard->siNumberOfMoves    = 0;
   argsBoard->bbWhiteAttack      = 0;
   argsBoard->bbWhiteMove        = 0;
   argsBoard->bbBlackAttack      = 0;
   argsBoard->bbBlackMove        = 0;
   argsBoard->bbWhiteKingMove    = 0;
   argsBoard->bbWhiteKingAttack  = 0;
   argsBoard->bbBlackKingMove    = 0;
   argsBoard->bbBlackKingAttack  = 0;

   // Calculate the moves for the pieces.

   // Calculate for the white pieces.
   if ( argsBoard->siColorToMove == dWhite )
   {
   
      if ( argsBoard->bbWhitePawn > 0 )
      {

         CalculateWhitePawnsQuiets( argsMoves, 
                                    argsBoard, 
                                    argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteKnight > 0 )
      {

         CalculateWhiteKnightsQuiets( argsMoves, 
                                      argsBoard, 
                                      argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteBishop > 0 )
      {

         CalculateWhiteBishopsQuiets( argsMoves, 
                                      argsBoard, 
                                      argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteRook > 0 )
      {

         CalculateWhiteRooksQuiets( argsMoves, 
                                    argsBoard, 
                                    argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteQueen > 0 )
      {

         CalculateWhiteQueensQuiets( argsMoves, 
                                     argsBoard, 
                                     argsGeneralMoves );

      }

      if ( argsBoard->bbWhiteKing > 0 )
      {

         CalculateWhiteKingQuiets( argsMoves, 
                                   argsBoard, 
                                   argsGeneralMoves );

      }

      if ( argsBoard->bbCastle > 0 )
      {

         CastleWhite( argsMoves, 
                      argsBoard, 
                      argsGeneralMoves );

      }
      
   }

   if ( argsBoard->siColorToMove == dBlack )
   {

      if ( argsBoard->bbBlackPawn > 0 )
      {

         CalculateBlackPawnsQuiets( argsMoves, 
                                    argsBoard, 
                                    argsGeneralMoves );

      }

      if ( argsBoard->bbBlackKnight > 0 )
      {

         CalculateBlackKnightsQuiets( argsMoves, 
                                      argsBoard, 
                                      argsGeneralMoves );

      }

      if ( argsBoard->bbBlackBishop > 0 )
      {

         CalculateBlackBishopsQuiets( argsMoves, 
                                      argsBoard, 
                                      argsGeneralMoves );

      }

      if ( argsBoard->bbBlackRook > 0 )
      {

         CalculateBlackRooksQuiets( argsMoves, 
                                    argsBoard, 
                                    argsGeneralMoves );

      }

      if ( argsBoard->bbBlackQueen > 0 )
      {

         CalculateBlackQueensQuiets( argsMoves, 
                                     argsBoard, 
                                     argsGeneralMoves );

      }

      if ( argsBoard->bbBlackKing > 0 )
      {

         CalculateBlackKingQuiets( argsMoves, 
                                   argsBoard, 
                                   argsGeneralMoves );

      }

      if ( argsBoard->bbCastle > 0 )
      {

         CastleBlack( argsMoves, 
                      argsBoard, 
                      argsGeneralMoves );

      }
      
   }

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateWhitePawnsQuiets( struct Move * argsMoves, 
                                struct Board * argsBoard, 
                                struct GeneralMove * argsGeneralMoves )
{
//
// This function looks at the white pawn moves.
//

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbWhitePawn == 0 )
   {
   
      return;

   }

   // Do the one step moves.
   BitBoard bbOneStep = ( argsBoard->bbWhitePawn << 8 ) & ( ~ argsBoard->bbAllPieces );

   // Do the two step moves.
   BitBoard bbElegibleMoves = argsBoard->bbWhitePawn & argsGeneralMoves->bbColOne;
   bbElegibleMoves          = bbElegibleMoves & ( bbOneStep >> 8 );
   bbElegibleMoves          = bbElegibleMoves << 16;
   BitBoard bbTwoStep       = ( ~ argsBoard->bbAllPieces ) & bbElegibleMoves;

   // Combine the attacks and moves.
   argsBoard->bbWhiteMove   = argsBoard->bbWhiteMove   | bbTwoStep      | bbOneStep;

   // Take out the promotion things.
   //Find the promotions
   BitBoard bbPromotionsOne   = ( argsGeneralMoves->bbColSeven & bbOneStep );

   // Remove the pormotions from other moves.
   bbOneStep      = bbOneStep & ~argsGeneralMoves->bbColSeven;

   // Find the moves and attacks.
   int vMovesOneStep[    8 ];
   int vMovesTwoStep[    8 ];
   int vPromotionsOne[   8 ];
   int vPromotionsRight[ 8 ];
   int vPromotionsLeft[  8 ];
   int siNumberOfMovesOneStep  = Find( bbOneStep,         vMovesOneStep,    argsGeneralMoves );
   int siNumberOfMovesTwoStep  = Find( bbTwoStep,         vMovesTwoStep,    argsGeneralMoves );
   int numberOfPromotionsOne   = Find( bbPromotionsOne,   vPromotionsOne,   argsGeneralMoves );

   // Enter the one step moves.
   for ( int moveIndex = 0; moveIndex < siNumberOfMovesOneStep; moveIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );

      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vMovesOneStep[ moveIndex ] - 8 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                              vMovesOneStep[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vMovesOneStep[ moveIndex ] - 8;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vMovesOneStep[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dOneSquare;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhitePawn;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msRegular;

   }

   // Enter the two step moves.
   for ( int moveIndex = 0; moveIndex < siNumberOfMovesTwoStep; moveIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vMovesTwoStep[ moveIndex ] - 16 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vMovesTwoStep[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vMovesTwoStep[ moveIndex ] - 16;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vMovesTwoStep[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dTwoSquare;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhitePawn;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbEPSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbEPSquare,
                                                                              vMovesTwoStep[ moveIndex ] - 8 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPawnTwo;

   }

   // Enter the promotions one.
   for ( int moveIndex = 0; moveIndex < numberOfPromotionsOne; moveIndex++ )
   {

      for ( int pieceIndex = 0; pieceIndex < 4; pieceIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                              vPromotionsOne[ moveIndex ] - 8 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                                 vPromotionsOne[ moveIndex ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPromotionsOne[ moveIndex ] - 8;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vPromotionsOne[ moveIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dPromote;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = argsGeneralMoves->vPromotionPieces[ pieceIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPromotion;

      }

   }

}   

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateWhiteRooksQuiets( struct Move * argsMoves, 
                                struct Board * argsBoard, 
                                struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbWhiteRook == 0 )
   {
   
      return;

   }

   // Do the Rooks.
   int numberOfRooks = 0;
   int vWhiteRooks[ 10 ];
   numberOfRooks = Find( argsBoard->bbWhiteRook, 
                         vWhiteRooks, 
                         argsGeneralMoves );

   // Loop over the rooks.
   for ( int rookIndex = 0; rookIndex < numberOfRooks; rookIndex++ )
   {
      
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mRN[ vWhiteRooks[ rookIndex ] ], 
                    argsGeneralMoves->vQNOMN[ vWhiteRooks[ rookIndex ] ], 
                    vWhiteRooks[ rookIndex ], 
                    dWhiteRook);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mRE[ vWhiteRooks[ rookIndex ] ], 
                    argsGeneralMoves->vQNOME[ vWhiteRooks[ rookIndex ] ], 
                    vWhiteRooks[ rookIndex ], 
                    dWhiteRook);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mRS[ vWhiteRooks[ rookIndex ] ], 
                    argsGeneralMoves->vQNOMS[ vWhiteRooks[ rookIndex ] ], 
                    vWhiteRooks[ rookIndex ], 
                    dWhiteRook);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mRW[ vWhiteRooks[ rookIndex ] ], 
                    argsGeneralMoves->vQNOMW[ vWhiteRooks[ rookIndex ] ], 
                    vWhiteRooks[ rookIndex ], 
                    dWhiteRook);

   }
   
}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateWhiteKnightsQuiets( struct Move * argsMoves, 
                                  struct Board * argsBoard, 
                                  struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If no knight, bail.
   if ( argsBoard->bbWhiteKnight == 0 )
   {
   
      return;

   }

   // Do the Knights.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbWhiteKnight, 
                          vPieces, 
                          argsGeneralMoves );

   int vMoves[ 8 ];

   // Loop over the knights.
   for ( int index = 0; index < numberOfPieces; index++ )
   {

      // Look for the good moves..
      BitBoard bbMoves = argsGeneralMoves->bbNMove[ vPieces[ index ] ] &
                         ( ~ argsBoard->bbAllPieces );

      // Enter the moves.
      int siNumberOfMoves   = Find( bbMoves, 
                                    vMoves, 
                                    argsGeneralMoves );
      argsBoard->bbWhiteMove   = argsBoard->bbWhiteMove   | bbMoves;

      // Enter the moves.
      for ( int moveIndex = 0; moveIndex < siNumberOfMoves; moveIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
		                                                                           vPieces[ index ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
             			                                                            vMoves[ moveIndex ] );
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPieces[ index ];
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vMoves[ moveIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dRegular;
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhiteKnight;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPiece;

      }

   }

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateWhiteBishopsQuiets( struct Move * argsMoves, 
                                  struct Board * argsBoard, 
                                  struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbWhiteBishop == 0 )
   {
   
      return;

   }


   // Do the Bishops.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbWhiteBishop, 
                          vPieces, 
                          argsGeneralMoves );

   // Loop over the Bishops.
   for ( int pieceIndex = 0; pieceIndex < numberOfPieces; pieceIndex++ )
   {
      
      SliderQuiets( argsMoves, 
                   argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mBNE[    vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMNE[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dWhiteBishop);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mBSE[    vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMSE[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dWhiteBishop);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mBSW[    vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMSW[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dWhiteBishop);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mBNW[    vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMNW[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dWhiteBishop);

   }

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateWhiteQueensQuiets(  struct Move * argsMoves, 
                                  struct Board * argsBoard, 
                                  struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbWhiteQueen == 0 )
   {
   
      return;

   }

   // Do the Queens.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbWhiteQueen, 
                          vPieces, 
                          argsGeneralMoves );

   // Loop over the Queens.
   for ( int pieceIndex = 0; pieceIndex < numberOfPieces; pieceIndex++ )
   {

      
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mQN[    vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMN[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dWhiteQueen);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mQNE[    vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMNE[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dWhiteQueen);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mQE[    vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOME[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dWhiteQueen);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mQSE[    vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMSE[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dWhiteQueen);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mQS[    vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMS[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dWhiteQueen);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mQSW[    vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMSW[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dWhiteQueen);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mQW[    vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMW[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dWhiteQueen);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mQNW[    vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMNW[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dWhiteQueen);

   }

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateWhiteKingQuiets( struct Move * argsMoves, 
                               struct Board * argsBoard, 
                               struct GeneralMove * argsGeneralMoves )
{
   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbWhiteKing == 0 )
   {
   
      return;

   }


   // Do the King.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbWhiteKing, 
                          vPieces, 
                          argsGeneralMoves );
   int index = 0;

   int vMoves[ 8 ];

   // Look for the good moves..
   BitBoard bbMoves   = argsGeneralMoves->bbKMove[ vPieces[ index ] ] &
                        ( ~ argsBoard->bbAllPieces );

   argsBoard->bbWhiteMove   = argsBoard->bbWhiteMove   | bbMoves;

   // Enter the moves.
   int siNumberOfMoves = Find( bbMoves,   vMoves,   argsGeneralMoves );

   // Enter the moves.
   for ( int moveIndex = 0; moveIndex < siNumberOfMoves; moveIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vPieces[ index ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vMoves[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPieces[ index ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vMoves[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dRegular;
	   argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dWhiteKing;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msRegular;
            
   }

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateBlackPawnsQuiets( struct Move        * argsMoves, 
                          struct Board       * argsBoard, 
                          struct GeneralMove * argsGeneralMoves )
{
//
// This function looks at the Black pawn moves.
//

   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbBlackPawn == 0 )
   {
   
      return;

   }

   // Do the one step moves.
   BitBoard bbOneStep = ( argsBoard->bbBlackPawn >> 8 ) & ( ~ argsBoard->bbAllPieces );

   // Do the two step moves.
   BitBoard bbElegibleMoves = argsBoard->bbBlackPawn & argsGeneralMoves->bbColSix;
   bbElegibleMoves          = bbElegibleMoves & ( bbOneStep << 8 );  
   BitBoard bbTwoStep         = ( bbElegibleMoves >> 16 ) & ( ~ argsBoard->bbAllPieces );

   // Combine the attacks and moves.
   argsBoard->bbBlackMove   = argsBoard->bbBlackMove   | bbTwoStep      | bbOneStep;

   // Take out the promotion things.
   //Find the promotions
   BitBoard bbPromotionsOne   = ( argsGeneralMoves->bbColZero & bbOneStep );

   // Remove the pormotions from other moves.
   bbOneStep      = bbOneStep & ~argsGeneralMoves->bbColZero;

   // Find the moves and attacks.
   int vMovesOneStep[    8 ];
   int vMovesTwoStep[    8 ];
   int vPromotionsOne[   8 ];
   int siNumberOfMovesOneStep  = Find( bbOneStep,         vMovesOneStep,    argsGeneralMoves );
   int siNumberOfMovesTwoStep  = Find( bbTwoStep,         vMovesTwoStep,    argsGeneralMoves );
   int numberOfPromotionsOne   = Find( bbPromotionsOne,   vPromotionsOne,   argsGeneralMoves );

   // Enter the one step moves.
   for ( int moveIndex = 0; moveIndex < siNumberOfMovesOneStep; moveIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare, 
	                                                                           vMovesOneStep[ moveIndex ] + 8 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vMovesOneStep[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vMovesOneStep[ moveIndex ] + 8;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vMovesOneStep[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dOneSquare;
	   argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackPawn;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msRegular;

   }

   // Enter the two step moves.
   for ( int moveIndex = 0; moveIndex < siNumberOfMovesTwoStep; moveIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vMovesTwoStep[ moveIndex ] +16 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vMovesTwoStep[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vMovesTwoStep[ moveIndex ] +16;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vMovesTwoStep[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dTwoSquare;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackPawn;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbEPSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbEPSquare,
	                                                                           vMovesTwoStep[ moveIndex ] + 8 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPawnTwo;

   }

   // Enter the promotions one.
   for ( int moveIndex = 0; moveIndex < numberOfPromotionsOne; moveIndex++ )
   {

      for ( int pieceIndex = 0; pieceIndex < 4; pieceIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );

         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                              vPromotionsOne[ moveIndex ] + 8 );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
                                                                                 vPromotionsOne[ moveIndex ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPromotionsOne[ moveIndex ] + 8;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vPromotionsOne[ moveIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dPromote;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = argsGeneralMoves->vPromotionPieces[ pieceIndex ] + 6;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPromotion;

      }

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateBlackRooksQuiets( struct Move * argsMoves, 
                                struct Board * argsBoard, 
                                struct GeneralMove * argsGeneralMoves )
{
   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbBlackRook == 0 )
   {
   
      return;

   }

   // Do the Rooks.
   int numberOfRooks = 0;
   int vBlackRooks[ 10 ];
   numberOfRooks = Find( argsBoard->bbBlackRook, 
                         vBlackRooks, 
                         argsGeneralMoves );

   // Loop over the rooks.
   for ( int rookIndex = 0; rookIndex < numberOfRooks; rookIndex++ )
   {
      
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mRN[ vBlackRooks[ rookIndex ] ], 
                    argsGeneralMoves->vQNOMN[ vBlackRooks[ rookIndex ] ], 
                    vBlackRooks[ rookIndex ], 
                    dBlackRook);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mRE[ vBlackRooks[ rookIndex ] ], 
                    argsGeneralMoves->vQNOME[ vBlackRooks[ rookIndex ] ], 
                    vBlackRooks[ rookIndex ], 
                    dBlackRook);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mRS[ vBlackRooks[ rookIndex ] ], 
                    argsGeneralMoves->vQNOMS[ vBlackRooks[ rookIndex ] ], 
                    vBlackRooks[ rookIndex ], 
                    dBlackRook);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mRW[ vBlackRooks[ rookIndex ] ], 
                    argsGeneralMoves->vQNOMW[ vBlackRooks[ rookIndex ] ], 
                    vBlackRooks[ rookIndex ], 
                    dBlackRook);

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateBlackKnightsQuiets( struct Move * argsMoves, 
                            struct Board * argsBoard, 
                            struct GeneralMove * argsGeneralMoves )
{
   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbBlackKnight == 0 )
   {
   
      return;

   }

   // Do the Knights.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbBlackKnight, 
                          vPieces, 
                          argsGeneralMoves );

   int vMoves[   8 ];

   // Loop over the knights.
   for ( int index = 0; index < numberOfPieces; index++ )
   {
      // Look for the good moves..
      BitBoard bbMoves = argsGeneralMoves->bbNMove[ vPieces[ index ] ] &
                         ( ~ argsBoard->bbAllPieces );

      argsBoard->bbBlackMove   = argsBoard->bbBlackMove   | bbMoves;

      // Enter the moves.
      int siNumberOfMoves   = Find( bbMoves,  vMoves,    argsGeneralMoves );

      // Enter the moves.
      for ( int moveIndex = 0; moveIndex < siNumberOfMoves; moveIndex++ )
      {

         argsBoard->siNumberOfMoves++;
         InitializeMoves( argsMoves,
                          argsBoard->siNumberOfMoves - 1 );
   	   argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
 		                                                                           vPieces[ index ] );
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
      		                                                                     vMoves[ moveIndex ] );
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPieces[ index ];
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vMoves[ moveIndex ];
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dRegular;
	      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackKnight;
         argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPiece;

      }

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateBlackBishopsQuiets( struct Move * argsMoves, 
                                  struct Board * argsBoard, 
                                  struct GeneralMove * argsGeneralMoves )
{
   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbBlackBishop == 0 )
   {
   
      return;

   }

   // Do the Bishops.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbBlackBishop, 
                          vPieces, 
                          argsGeneralMoves );

   // Loop over the Bishops.
   for ( int pieceIndex = 0; pieceIndex < numberOfPieces; pieceIndex++ )
   {
      
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves, 
                    argsGeneralMoves->mBNE[ vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMNE[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dBlackBishop);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves, 
                    argsGeneralMoves->mBSE[ vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMSE[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dBlackBishop);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves, 
                    argsGeneralMoves->mBSW[ vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMSW[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dBlackBishop);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves, 
                    argsGeneralMoves->mBNW[ vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMNW[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dBlackBishop);

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateBlackQueensQuiets(  struct Move * argsMoves, 
                                  struct Board * argsBoard, 
                                  struct GeneralMove * argsGeneralMoves )
{
   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbBlackQueen == 0 )
   {
   
      return;

   }


   // Do the Queens.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbBlackQueen, 
                          vPieces, 
                          argsGeneralMoves );

   // Loop over the Queens.
   for ( int pieceIndex = 0; pieceIndex < numberOfPieces; pieceIndex++ )
   {
      
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mQN[ vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMN[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dBlackQueen);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mQNE[ vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMNE[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dBlackQueen);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mQE[ vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOME[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dBlackQueen);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mQSE[ vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMSE[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dBlackQueen);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mQS[ vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMS[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dBlackQueen);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mQSW[ vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMSW[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dBlackQueen);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mQW[ vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMW[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dBlackQueen);
      SliderQuiets( argsMoves, 
                    argsBoard, 
                    argsGeneralMoves,
                    argsGeneralMoves->mQNW[ vPieces[ pieceIndex ] ], 
                    argsGeneralMoves->vQNOMNW[ vPieces[ pieceIndex ] ], 
                    vPieces[ pieceIndex ], 
                    dBlackQueen);

   }

}

//
//-----------------------------------------------------------------------------------
//
void CalculateBlackKingQuiets(    struct Move * argsMoves, 
                            struct Board * argsBoard, 
                            struct GeneralMove * argsGeneralMoves )
{
   // Debug the inputs.
   assert( argsMoves >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // If not pawns, bail.
   if ( argsBoard->bbBlackKing == 0 )
   {
   
      return;

   }

   // Do the King.
   int numberOfPieces = 0;
   int vPieces[ 10 ];
   numberOfPieces = Find( argsBoard->bbBlackKing, 
                          vPieces, 
                          argsGeneralMoves );
   int index = 0;

   int vMoves[   8 ];
   int vAttacks[ 8 ];

   // Look for the good moves..
   BitBoard bbMoves = argsGeneralMoves->bbKMove[ vPieces[ index ] ] &
                      ( ~ argsBoard->bbAllPieces );


   argsBoard->bbBlackMove   = argsBoard->bbBlackMove   | bbMoves;

   // Enter the moves.
   int siNumberOfMoves = Find( bbMoves,   
                               vMoves,   
                               argsGeneralMoves );

   // Enter the moves.
   for ( int moveIndex = 0; moveIndex < siNumberOfMoves; moveIndex++ )
   {

      argsBoard->siNumberOfMoves++;
      InitializeMoves( argsMoves,
                       argsBoard->siNumberOfMoves - 1 );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbFromSquare,
	                                                                           vPieces[ index ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare   = SetBitToOne( argsMoves[ argsBoard->siNumberOfMoves - 1 ].bbToSquare,
	                                                                           vMoves[ moveIndex ] );
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iFromSquare  = vPieces[ index ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iToSquare    = vMoves[ moveIndex ];
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iMoveType    = dRegular;
	   argsMoves[ argsBoard->siNumberOfMoves - 1 ].iPiece       = dBlackKing;
      argsMoves[ argsBoard->siNumberOfMoves - 1 ].iScore       = argsGeneralMoves->msPiece;

   }

}
