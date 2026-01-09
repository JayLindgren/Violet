// Copyright 2006 by Jay Lindgren. All Rights Reserved.
//
//  This function contains the details of the implementation of bit boards.
//

/*
   #define _CRTDBG_MAP_ALLOC
   #include <stdlib.h>
   #include <crtdbg.h>
//*/

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdlib.h>
#include <fstream>
// #include <string.h>
#include <string>
#include "Definitions.h"
#include "Functions.h"
#include "Structures.h"
#include <intrin.h> // For MSVC
using namespace std;

//
//
//---------------------------------------------------------------------
//
//
void CreateBoard( struct Board       *argsBoard,
                  struct GeneralMove *argsGeneralMoves )
{
   //
   // This function is used to create a board using a set of logicals.
   // Note:  This does NOT reset the value of the hash table to the original value.
   //

   // Debug the inputs
   assert( argsBoard > 0 );
   assert( argsGeneralMoves > 0 );

   // Allocate moves for initializing the board.
   struct Move sDummyMoves[ 100 ];

   // NOTE: this resets the hash table!!!
   ReadFEN( "FEN.txt",
            argsBoard,
            argsGeneralMoves,
            1 );

   if ( GetModeOfPlay() == dSetUp )
   {

      ReadFEN( "FEN.txt",
               argsBoard,
               argsGeneralMoves,
               0 );
   }

   // Set the initial best move to something higher than any number of moves.
   argsBoard->iBestMove = 128;

   // Set the history count to zero.
   argsBoard->iMoveHistory = 0;

   // Calculate the initial moves.
   CalculateMoves( sDummyMoves,
                   argsBoard,
                   argsGeneralMoves );

   // Set the initial ply to zero.
   argsBoard->iNumberOfPlys   = -1;
   argsBoard->iMaxPlysReached = -1;

   // Initialize the ply length vector.
   for ( int iPly = 0; iPly < dNumberOfPlys; iPly++ )
   {

      // Set the length of the PV to zero.
      argsBoard->viPrincipalVariationLength[ iPly ] = iPly;
   }

   // Initialize the best move for the hash table.
   argsBoard->iBestMove = 0;

   // Set the board as being in book.
   SetIsInBook( dYes );

   // Set the maximum search depth.
   argsBoard->iMaxPlys = GetSearchDepth();

   // Computer to play black or white.
   argsBoard->siComputerColor = GetComputerColor();

   // Start the clock for timing purposes.
   argsBoard->cStart = clock();

   // Set that the last nmove not a null.
   argsBoard->iLastMoveNull = dNo;

   // Set the null verification to yes.
   argsBoard->iUseNullVerification = dYes;

   // Reset the PV.
   ClearPV( argsBoard );

   // Reset the hash if applicable, but at least
   // reset the hash itself
   argsBoard->bbHash                    = GetHashInitial();
   argsBoard->sHistoryStack[ 0 ].bbHash = argsBoard->bbHash;
   if ( GetResetHash() == dYes )
   {

      ClearHashTable();
   }
}

//
//
//---------------------------------------------------------------------
//
//
BitBoard Count( BitBoard bbBoard )
{
#if defined( __GNUC__ ) || defined( __clang__ )
   return __builtin_popcountll( bbBoard );
#else
   return __popcnt64( bbBoard );
#endif
}

//
//
//---------------------------------------------------------------------
//
//
int Find( BitBoard            bbBoard,
          int                *vPosition,
          struct GeneralMove *argsGeneralMoves )
{
   int           siCount = 0;
   unsigned long index;

   // This function finds the non-zero bits.
   while ( _BitScanForward64( &index, bbBoard ) )
   {

      vPosition[ siCount++ ] = index;
      bbBoard &= bbBoard - 1; // Clear the LSB
   }

   return siCount;
}

//
//
//---------------------------------------------------------------------
//
//
BitBoard SetBitToOne( BitBoard bbBoard,
                      int      siPosition )
{
   // This function is used to set a single bit on an integer.
   //
   // Warning, this function assumes the bit is initially set to zero;

   // Debugging.
   assert( bbBoard >= 0 );
   /*
      if ( siPosition < 0 )
      {
         cout << "Here's the rub." << endl;
      }
   */
   assert( siPosition >= 0 );
   assert( siPosition < 64 );

   // Create the dummy integer.
   BitBoard dummy = 1;

   // Shift the bit.
   dummy = dummy << siPosition;

   // Put the new bit in the variable.
   bbBoard = bbBoard | dummy;

   return bbBoard;
}

//
//
//---------------------------------------------------------------------
//
//
BitBoard SetBitToZero( BitBoard bbBoard,
                       int      siPosition )
{
   // This function is used to set a single bit on an integer.
   //

   assert( bbBoard >= 0 );
   assert( siPosition >= 0 );
   assert( siPosition <= 64 );

   // Create the dummy integer.
   BitBoard bbDummy = 1;

   // Shift the bit.
   bbDummy = bbDummy << siPosition;

   // Put the new bit in the variable.
   bbBoard = bbBoard & ( ~bbDummy );

   // Debugging
   assert( bbBoard >= 0 );

   // Return the bit board.
   return bbBoard;
}

//
//
//---------------------------------------------------------------------
//
//
BitBoard FlipBit( BitBoard bbBoard,
                  int      siPosition )
{
   // This function is used flip a single bit on an integer.
   //

   // Debugging
   assert( bbBoard >= 0 );
   assert( siPosition >= 0 );
   assert( siPosition <= 64 );

   // Create the dummy integer.
   BitBoard bbDummy = 1;

   // Shift the bit.
   bbDummy = bbDummy << siPosition;

   // Put the new bit in the variable.
   bbBoard = bbBoard ^ bbDummy;

   // Debugging
   assert( bbBoard >= 0 );

   // Return the bit board
   return bbBoard;
}

//
//
//---------------------------------------------------------------------
//
//
void CalculateSquares( int *vRow,
                       int *vCol,
                       int *vSquare,
                       int *length )
{
   // This function calculates the square that are valid for a vector of inputs.

   // Debug the inputs.
   assert( vRow >= 0 );
   assert( vCol >= 0 );
   assert( vSquare >= 0 );
   assert( length >= 0 );

   // Some variables.
   int siLengthIndex;

   if ( *length <= 0 )
   {
      cout << "Inappropriate length." << endl;
   }

   int count = 0;
   for ( siLengthIndex = 0; siLengthIndex < *length; siLengthIndex++ )
   {
      if ( ( vRow[ siLengthIndex ] >= 0 ) &&
           ( vRow[ siLengthIndex ] <= 7 ) &&
           ( vCol[ siLengthIndex ] >= 0 ) &&
           ( vCol[ siLengthIndex ] <= 7 ) )
      {
         vSquare[ count ] = vRow[ siLengthIndex ] + ( vCol[ siLengthIndex ] << 3 );
         count++;
      }
   }

   // Debugging.
   assert( count >= 0 );
   assert( count <= 64 );

   // Set the length.
   *length = count;
}
//
//
//---------------------------------------------------------------------
//
//
void PrintBitBoard( BitBoard bbBoard )
{
   int      rowIndex;
   int      colIndex;
   int      iSquare;
   BitBoard bit;
   BitBoard bbDummy;

   // Debug the input.
   assert( bbBoard >= 0 );

   // This function is used for print out a set of bits.
   // for ( rowIndex = 0; rowIndex < 8; rowIndex ++ )
   for ( colIndex = 0; colIndex < 8; colIndex++ )
   {

      for ( rowIndex = 0; rowIndex < 8; rowIndex++ )
      // for ( colIndex = 0; colIndex < 8; colIndex++ )
      {

         // Convert the rows and cols to a square.
         iSquare = ( rowIndex ) + 8 * ( 7 - colIndex );

         // Shift the bits.
         bbDummy = bbBoard >> iSquare;

         // Extract the bit.
         bit = bbDummy % 2;

         // Output the bit.
         cout << " " << bit;
      }

      // Output a new line.
      cout << endl;
   }

   cout << endl;
}

//
//
//---------------------------------------------------------------------
//
//
void PrintBoard( int *vBoard )
{
   // This function is used for print out a set of bits.
   // This funciton prints the boards out as if looking at them
   // from the white side.  A1 is in the lower left hand corner and
   // H8 is in the upper right hand corcer

   // Debug the input.
   assert( vBoard >= 0 );

   // Put out a space.
   cout << endl;

   for ( int colIndex = 0; colIndex < 8; colIndex++ )
   {

      // Write out the col.
      printf( "%1.0d  ", ( 8 - colIndex ) );

      for ( int rowIndex = 0; rowIndex < 8; rowIndex++ )
      {

         // Convert the rows and cols to a square.
         int iSquare      = ( rowIndex ) + 8 * ( 7 - colIndex );
         int iPiece       = vBoard[ iSquare ];
         int iOutputPiece = iPiece;
         switch ( iPiece )
         {

         case dBlackPawn:
         {
            iOutputPiece = -1;
            break;
         }

         case dBlackRook:
         {

            iOutputPiece = -2;
            break;
         }

         case dBlackKnight:
         {

            iOutputPiece = -3;
            break;
         }

         case dBlackBishop:
         {

            iOutputPiece = -4;
            break;
         }

         case dBlackQueen:
         {

            iOutputPiece = -5;
            break;
         }

         case dBlackKing:
         {

            iOutputPiece = -6;
            break;
         }
         }

         // Output the bit.
         if ( iOutputPiece >= 0 )
         {

            cout << "  " << iOutputPiece;
         }

         if ( iOutputPiece < 0 )
         {

            cout << " " << iOutputPiece;
         }
      }

      // Output a new line.
      cout << endl
           << endl;
   }

   // Write out the rows.
   cout << endl
        << "     a  b  c  d  e  f  g  h" << endl
        << endl;
}

//
//
//---------------------------------------------------------------------
//
//
void PrintFEN( struct Board       *argsBoard,
               struct GeneralMove *argsGeneralMoves )
{
   // This function is used for print out a set of bits.

   // Debug the inputs
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   char fen[ 1024 ];
   GenerateFENFromBoard( argsBoard, argsGeneralMoves, fen );
   cout << endl
        << fen << endl
        << endl;
}

//
//
//---------------------------------------------------------------------
//
//
void PrintFullBoard( struct Board *argsBoard )
{
   // This will print every aspect of the board structure.
   //

   // Debug the input.
   assert( argsBoard >= 0 );

   cout << "White Pawns " << endl;
   PrintBitBoard( argsBoard->bbWhitePawn );
   cout << "White Rooks " << endl;
   PrintBitBoard( argsBoard->bbWhiteRook );
   cout << "White Knights " << endl;
   PrintBitBoard( argsBoard->bbWhiteKnight );
   cout << "White Bishops " << endl;
   PrintBitBoard( argsBoard->bbWhiteBishop );
   cout << "White Queen " << endl;
   PrintBitBoard( argsBoard->bbWhiteQueen );
   cout << "White King " << endl;
   PrintBitBoard( argsBoard->bbWhiteKing );

   cout << "Black Pawns " << endl;
   PrintBitBoard( argsBoard->bbBlackPawn );
   cout << "Black Rook " << endl;
   PrintBitBoard( argsBoard->bbBlackRook );
   cout << "Black Knight " << endl;
   PrintBitBoard( argsBoard->bbBlackKnight );
   cout << "Black Bishop " << endl;
   PrintBitBoard( argsBoard->bbBlackBishop );
   cout << "Black Queen " << endl;
   PrintBitBoard( argsBoard->bbBlackQueen );
   cout << "Black King " << endl;
   PrintBitBoard( argsBoard->bbBlackKing );

   cout << "Castling State " << endl;
   PrintBitBoard( argsBoard->bbCastle );

   cout << "All Pieces" << endl;
   PrintBitBoard( argsBoard->bbAllPieces );
   cout << "White Pieces" << endl;
   PrintBitBoard( argsBoard->bbWhitePieces );
   cout << "Black Pieces" << endl;
   PrintBitBoard( argsBoard->bbBlackPieces );

   // cout << "King Side White     = " << argsBoard->bbCastle << endl;
   cout << "Is Check White      = " << argsBoard->bbCheck << endl;
   cout << "Color to Move       = " << argsBoard->siColorToMove << endl;
   cout << endl;
   cout << "The Board" << endl;
   PrintBoard( argsBoard->mBoard );
}

int CheckBoard( struct Board *argsBoard )
{
   // This subroutine checks the current chess board and makes sure everything is in a good state.

   // Debug the input.
   assert( argsBoard >= 0 );

   //  Check the piece numbers.
   BitBoard iWhitePawn   = 0;
   BitBoard iWhiteRook   = 0;
   BitBoard iWhiteKnight = 0;
   BitBoard iWhiteBishop = 0;
   BitBoard iWhiteQueen  = 0;
   BitBoard iWhiteKing   = 0;
   BitBoard iBlackPawn   = 0;
   BitBoard iBlackRook   = 0;
   BitBoard iBlackKnight = 0;
   BitBoard iBlackBishop = 0;
   BitBoard iBlackQueen  = 0;
   BitBoard iBlackKing   = 0;

   // PrintFullBoard( argsBoard );

   for ( int iSquareNumber = 0; iSquareNumber < 64; iSquareNumber++ )
   {
      if ( ( argsBoard->mBoard[ iSquareNumber ] > 12 ) ||
           ( argsBoard->mBoard[ iSquareNumber ] < 0 ) )
      {

         // PrintBoard( argsBoard->mBoard );
         // cout << "Invalid piece on board." << endl;
         // system("pause");
      }

      if ( argsBoard->mBoard[ iSquareNumber ] == dWhitePawn )
      {

         iWhitePawn++;
      }
      if ( argsBoard->mBoard[ iSquareNumber ] == dWhiteRook )
      {

         iWhiteRook++;
      }
      if ( argsBoard->mBoard[ iSquareNumber ] == dWhiteKnight )
      {

         iWhiteKnight++;
      }
      if ( argsBoard->mBoard[ iSquareNumber ] == dWhiteBishop )
      {

         iWhiteBishop++;
      }
      if ( argsBoard->mBoard[ iSquareNumber ] == dWhiteQueen )
      {

         iWhiteQueen++;
      }
      if ( argsBoard->mBoard[ iSquareNumber ] == dWhiteKing )
      {

         iWhiteKing++;
      }
      if ( argsBoard->mBoard[ iSquareNumber ] == dBlackPawn )
      {

         iBlackPawn++;
      }
      if ( argsBoard->mBoard[ iSquareNumber ] == dBlackRook )
      {

         iBlackRook++;
      }
      if ( argsBoard->mBoard[ iSquareNumber ] == dBlackKnight )
      {

         iBlackKnight++;
      }
      if ( argsBoard->mBoard[ iSquareNumber ] == dBlackBishop )
      {

         iBlackBishop++;
      }
      if ( argsBoard->mBoard[ iSquareNumber ] == dBlackQueen )
      {

         iBlackQueen++;
      }
      if ( argsBoard->mBoard[ iSquareNumber ] == dBlackKing )
      {

         iBlackKing++;
      }
   }

   if ( iWhitePawn > 8 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of white pawns" << endl;
      // system("pause");
   }
   if ( iWhiteRook > 10 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of white rooks" << endl;
      // system("pause");
   }
   if ( iWhiteKnight > 10 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of white knights" << endl;
      // system("pause");
   }
   if ( iWhiteBishop > 10 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of white bishops" << endl;
      // system("pause");
   }
   if ( iWhiteQueen > 9 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of white queens" << endl;
      // system("pause");
   }
   if ( ( iWhiteKing > 1 ) )
   {

      // cout << "Invalid number of white king" << endl;
      // PrintFullBoard( argsBoard );
      // system("pause");
   }
   if ( iBlackPawn > 8 )
   {

      // PrintBoard( argsBoard->mBoard );
      // PrintBitBoard( argsBoard->bbBlackPawn );
      // cout << "Invalid number of Black pawns" << endl;
      // for ( int iHistory = 1; iHistory <= argsBoard->iNumberOfPlys + 1; iHistory ++ )
      // {
      //    cout << "Piece = " << argsBoard->sHistoryStack[ iHistory].iPiece << " Move = " << argsBoard->sHistoryStack[ iHistory].iMoveType
      //         << " From = " << argsBoard->sHistoryStack[ iHistory].iFromSquare << " To = " << argsBoard->sHistoryStack[ iHistory].iToSquare
      //         << " iHistory = " << iHistory << endl;
      // }
      // system("pause");
      return 0;
   }
   if ( iBlackRook > 10 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of Black rooks" << endl;
      // system("pause");
   }
   if ( iBlackKnight > 10 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of Black knights" << endl;
      // system("pause");
   }
   if ( iBlackBishop > 10 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of Black bishops" << endl;
      // system("pause");
   }
   if ( iBlackQueen > 9 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of Black queens" << endl;
      // system("pause");
   }
   if ( iBlackKing > 1 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of Black king" << endl;
      // system("pause");
   }

   iWhitePawn   = Count( argsBoard->bbWhitePawn );
   iWhiteRook   = Count( argsBoard->bbWhiteRook );
   iWhiteKnight = Count( argsBoard->bbWhiteKnight );
   iWhiteBishop = Count( argsBoard->bbWhiteBishop );
   iWhiteQueen  = Count( argsBoard->bbWhiteQueen );
   iWhiteKing   = Count( argsBoard->bbWhiteKing );
   iBlackPawn   = Count( argsBoard->bbBlackPawn );
   iBlackRook   = Count( argsBoard->bbBlackRook );
   iBlackKnight = Count( argsBoard->bbBlackKnight );
   iBlackBishop = Count( argsBoard->bbBlackBishop );
   iBlackQueen  = Count( argsBoard->bbBlackQueen );
   iBlackKing   = Count( argsBoard->bbBlackKing );

   if ( iWhitePawn > 8 )
   {

      // PrintFullBoard( argsBoard );
      // cout << "Invalid number of white pawns" << endl;
      // system("pause");
   }
   if ( iWhiteRook > 3 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of white rooks" << endl;
      // system("pause");
   }
   if ( iWhiteKnight > 3 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of white knights" << endl;
      // system("pause");
   }
   if ( iWhiteBishop > 3 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of white bishops" << endl;
      // system("pause");
   }
   if ( iWhiteQueen > 2 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of white queens" << endl;
      // system("pause");
   }
   if ( iWhiteKing > 1 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of white king" << endl;
      // system("pause");
   }
   if ( iBlackPawn > 8 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of Black pawns" << endl;
   }
   if ( iBlackRook > 3 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of Black rooks" << endl;
      // system("pause");
   }
   if ( iBlackKnight > 3 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of Black knights" << endl;
      // system("pause");
   }
   if ( iBlackBishop > 3 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of Black bishops" << endl;
      // system("pause");
   }
   if ( iBlackQueen > 2 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of Black queens" << endl;
      // system("pause");
   }
   if ( iBlackKing > 1 )
   {

      // PrintBoard( argsBoard->mBoard );
      // cout << "Invalid number of Black king" << endl;
      // system("pause");
   }

   BitBoard iAllPieces   = Count( argsBoard->bbAllPieces );
   BitBoard iWhitePieces = Count( argsBoard->bbWhitePieces );
   BitBoard iBlackPieces = Count( argsBoard->bbBlackPieces );

   if ( iAllPieces > 32 )
   {

      PrintBoard( argsBoard->mBoard );
      cout << "All pieces > 32" << endl;
      // system("pause");
   }
   if ( iWhitePieces > 32 )
   {

      PrintBoard( argsBoard->mBoard );
      cout << "White pieces > 32" << endl;
      // system("pause");
   }
   if ( iBlackPieces > 32 )
   {

      PrintBoard( argsBoard->mBoard );
      cout << "Black pieces > 32" << endl;
      system( "pause" );
   }

   // Check the color to move.
   if ( ( argsBoard->siColorToMove != dWhite ) &
        ( argsBoard->siColorToMove != dBlack ) )
   {

      cout << "Color to move not found." << endl;
      // system ( "pause" );
   }

   // Check the check.
   if ( ( argsBoard->bbCheck < 0 ) |
        ( argsBoard->bbCheck > 64 ) )
   {

      cout << "Color to move not found." << endl;
      // system ( "pause" );
   }

   // return a positive
   return 1;
}

void LookForCheckmate( struct Board       *argsBoard,
                       struct GeneralMove *argsGeneralMoves,
                       int                *argsiHaveMove,
                       int                *argiAlpha,
                       int                *argiBeta )
{

   // Debug the input.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );
   assert( argsiHaveMove >= 0 );

   // Calculate the attacks.
   Move vsMoveList[ dNumberOfMoves ];

   // Switch the colors and calculate the attacks.
   if ( argsBoard->siColorToMove == dWhite )
   {

      argsBoard->siColorToMove = dBlack;
      CalculateAttacks( vsMoveList,
                        argsBoard,
                        argsGeneralMoves );
      argsBoard->siColorToMove = dWhite;
   }
   else
   {

      argsBoard->siColorToMove = dWhite;
      CalculateAttacks( vsMoveList,
                        argsBoard,
                        argsGeneralMoves );
      argsBoard->siColorToMove = dBlack;
   }

   // If there is check then there is check mate
   if ( argsBoard->bbCheck & 2 )
   {

      // There is no legal move and we have checkmate.
      // PrintBoard( argsBoard->mBoard );
      argsBoard->bbCheck = SetBitToOne( argsBoard->bbCheck, 3 );
      *argiAlpha         = dMate;
      *argiBeta          = -dMate;
   }
   else if ( argsBoard->bbCheck & 1 )
   {

      // There is no legal move and we have checkmate.
      argsBoard->bbCheck = SetBitToOne( argsBoard->bbCheck, 2 );
      *argiAlpha         = dMate;
      *argiBeta          = -dMate;
   }

   // Update the PV if check mate is found.
   if ( *argiAlpha == dMate ||
        *argiAlpha == 0 )
   {

      // Copy the move history into the principal variation.
      CreatePV( argsBoard );
   }
}

//
//
//---------------------------------------------------------------------
//
//
void ReadFEN( const char          strFileName[ 640 ],
              struct Board       *argsBoard,
              struct GeneralMove *argsGeneralMoves,
              int                 argiInitialBoardFlag )
{

   // This function reads and parses a FEN positoin
   // argiIInitialBoardFlag
   //    0 - reads from the FEN.txt file in the Violet directory
   //    1 - Set up the standard position
   //    2 - Use the file name a FEN string and parses

   int iNumberOfCharacters;
   int iCharacterIndex;
   int iSquare = -1;
   int iCol    = 0;
   int iRow    = 0;

   // Reset the hash
   // Note, if we are resetting the board while still in the opening book,
   // we won't want to reset the hash as it holds the book.
   if ( GetIsInBook() == dNo )
   {

      ClearHashTable();
   }

   // Reset the initial Hash.
   argsBoard->bbHash = GetHashInitial();

   // Debug the input
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );
   assert( argiInitialBoardFlag >= 0 );
   assert( argiInitialBoardFlag < 3 );

   // Initialize the piece bit boards to zeros.
   argsBoard->bbWhitePawn   = 0;
   argsBoard->bbWhiteRook   = 0;
   argsBoard->bbWhiteKnight = 0;
   argsBoard->bbWhiteBishop = 0;
   argsBoard->bbWhiteQueen  = 0;
   argsBoard->bbWhiteKing   = 0;
   argsBoard->bbBlackPawn   = 0;
   argsBoard->bbBlackRook   = 0;
   argsBoard->bbBlackKnight = 0;
   argsBoard->bbBlackBishop = 0;
   argsBoard->bbBlackQueen  = 0;
   argsBoard->bbBlackKing   = 0;
   argsBoard->bbAllPieces   = 0;
   argsBoard->bbWhitePieces = 0;
   argsBoard->bbBlackPieces = 0;
   argsBoard->bbCastle      = 0;
   argsBoard->bbCheck       = 0;
   argsBoard->bbEP          = 0;

   // Assume the position is legal.
   argsBoard->siLegalMove = 1;

   for ( iSquare = 0; iSquare < 64; iSquare++ )
   {

      argsBoard->mBoard[ iSquare ] = 0;
   }

   // FIX: Initialize Principal Variation array to invalid values to prevent garbage moves
   for ( int i = 0; i < dNumberOfPlys; i++ )
   {
      for ( int j = 0; j < dNumberOfPlys; j++ )
      {
         argsBoard->vmPrincipalVariation[ i ][ j ].iFromSquare = -1;
         argsBoard->vmPrincipalVariation[ i ][ j ].iToSquare   = -1;
      }
   }

   char str1[ 64 ] = { 0 };
   char str2[ 64 ] = { 0 };
   char str3[ 64 ] = { 0 };
   char str4[ 64 ] = { 0 };
   char str5[ 64 ] = { 0 };
   char str6[ 64 ] = { 0 };

   // Read a file with the specifications of an FEN file.
   if ( argiInitialBoardFlag == 1 )
   {

      // str2 << "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR                    ";
      strcpy( str1, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR" );
      sprintf( str2, "%s", "w" );
      sprintf( str3, "%s", "KQkq" );
      sprintf( str4, "%s", "-" );
      sprintf( str5, "%s", "0" );
      sprintf( str6, "%s", "1" );
   }
   else if ( argiInitialBoardFlag == 0 )
   {

      ifstream ifFEN( strFileName );
      //      ifstream ifFEN( strFileName );
      if ( ifFEN.fail() )
      {

         cout << "Input FEN file failed to open." << endl;
         cout << "Falling back to Standard Start Position." << endl;
         ReadFEN( "", argsBoard, argsGeneralMoves, 1 ); // Recursive call to load standard start pos
         return;
      }

      ifFEN >> str1;
      ifFEN >> str2;
      ifFEN >> str3;
      ifFEN >> str4;
      ifFEN >> str5;
      ifFEN >> str6;

      ifFEN.close();
   }
   else if ( argiInitialBoardFlag == 2 )
   {
      // Note: strncpy does not null-terminate if length is reached, but since we init to 0 and buffer is large, it should be fine.
      // We also need to be careful about pointer arithmetic.

      const char *strPointer1;
      const char *strPointer2;
      const char *strPointer3;
      const char *strPointer4;
      const char *strPointer5;

      // Note, for this case, the file name contains the FEN string.
      strPointer1 = strstr( strFileName, " " );
      strPointer2 = strstr( strPointer1 + 1, " " );
      strPointer3 = strstr( strPointer2 + 1, " " );
      strPointer4 = strstr( strPointer3 + 1, " " );
      strPointer5 = strstr( strPointer4 + 1, " " );

      if ( strPointer1 && strPointer2 && strPointer3 && strPointer4 )
      {
         strncpy( str1, strFileName, strPointer1 - strFileName );
         strncpy( str2, strPointer1 + 1, strPointer2 - strPointer1 );
         strncpy( str3, strPointer2 + 1, strPointer3 - strPointer2 );
         strncpy( str4, strPointer3 + 1, strPointer4 - strPointer3 );
         // Manually null terminate just in case of weirdness, though {0} init handles it if length < 64
         if ( ( strPointer4 - strPointer3 ) < 64 )
            str4[ strPointer4 - strPointer3 ] = '\0';
      }
      else
      {
         cout << "Error parsing FEN string: " << strFileName << endl;
      }
   }
   else
   {

      cout << "Initial file read flag was not understood." << endl;
      // system( "pause" );
   }

   // Strint 1 - Put the pieces on the board.
   iNumberOfCharacters = (int)strlen( str1 );

   // Set the initial square for h8.
   iSquare = dH8;
   iCol    = 7;
   iRow    = 0;

   // Loop over the characters in the string.
   for ( iCharacterIndex = 0; iCharacterIndex < iNumberOfCharacters; iCharacterIndex++ )
   {

      if ( str1[ iCharacterIndex ] == 'r' ||
           str1[ iCharacterIndex ] == 'n' ||
           str1[ iCharacterIndex ] == 'b' ||
           str1[ iCharacterIndex ] == 'q' ||
           str1[ iCharacterIndex ] == 'k' ||
           str1[ iCharacterIndex ] == 'p' ||
           str1[ iCharacterIndex ] == 'R' ||
           str1[ iCharacterIndex ] == 'N' ||
           str1[ iCharacterIndex ] == 'B' ||
           str1[ iCharacterIndex ] == 'Q' ||
           str1[ iCharacterIndex ] == 'K' ||
           str1[ iCharacterIndex ] == 'P' )
      {

         switch ( str1[ iCharacterIndex ] )
         {

         case 'P':
         {

            iSquare                      = iCol * 8 + iRow;
            argsBoard->bbWhitePawn       = SetBitToOne( argsBoard->bbWhitePawn, iSquare );
            argsBoard->mBoard[ iSquare ] = dWhitePawn;
            iRow++;
            break;
         }

         case 'R':
         {

            iSquare                      = iCol * 8 + iRow;
            argsBoard->bbWhiteRook       = SetBitToOne( argsBoard->bbWhiteRook, iSquare );
            argsBoard->mBoard[ iSquare ] = dWhiteRook;
            iRow++;
            break;
         }

         case 'N':
         {

            iSquare                      = iCol * 8 + iRow;
            argsBoard->bbWhiteKnight     = SetBitToOne( argsBoard->bbWhiteKnight, iSquare );
            argsBoard->mBoard[ iSquare ] = dWhiteKnight;
            iRow++;
            break;
         }

         case 'B':
         {

            iSquare                      = iCol * 8 + iRow;
            argsBoard->bbWhiteBishop     = SetBitToOne( argsBoard->bbWhiteBishop, iSquare );
            argsBoard->mBoard[ iSquare ] = dWhiteBishop;
            iRow++;
            break;
         }

         case 'Q':
         {

            iSquare                      = iCol * 8 + iRow;
            argsBoard->bbWhiteQueen      = SetBitToOne( argsBoard->bbWhiteQueen, iSquare );
            argsBoard->mBoard[ iSquare ] = dWhiteQueen;
            iRow++;
            break;
         }

         case 'K':
         {

            iSquare                      = iCol * 8 + iRow;
            argsBoard->bbWhiteKing       = SetBitToOne( argsBoard->bbWhiteKing, iSquare );
            argsBoard->mBoard[ iSquare ] = dWhiteKing;
            iRow++;
            break;
         }

         case 'p':
         {

            iSquare                      = iCol * 8 + iRow;
            argsBoard->bbBlackPawn       = SetBitToOne( argsBoard->bbBlackPawn, iSquare );
            argsBoard->mBoard[ iSquare ] = dBlackPawn;
            iRow++;
            break;
         }

         case 'r':
         {

            iSquare                      = iCol * 8 + iRow;
            argsBoard->bbBlackRook       = SetBitToOne( argsBoard->bbBlackRook, iSquare );
            argsBoard->mBoard[ iSquare ] = dBlackRook;
            iRow++;
            break;
         }

         case 'n':
         {

            iSquare                      = iCol * 8 + iRow;
            argsBoard->bbBlackKnight     = SetBitToOne( argsBoard->bbBlackKnight, iSquare );
            argsBoard->mBoard[ iSquare ] = dBlackKnight;
            iRow++;
            break;
         }

         case 'b':
         {

            iSquare                      = iCol * 8 + iRow;
            argsBoard->bbBlackBishop     = SetBitToOne( argsBoard->bbBlackBishop, iSquare );
            argsBoard->mBoard[ iSquare ] = dBlackBishop;
            iRow++;
            break;
         }

         case 'q':
         {

            iSquare                      = iCol * 8 + iRow;
            argsBoard->bbBlackQueen      = SetBitToOne( argsBoard->bbBlackQueen, iSquare );
            argsBoard->mBoard[ iSquare ] = dBlackQueen;
            iRow++;
            break;
         }

         case 'k':
         {

            iSquare                      = iCol * 8 + iRow;
            argsBoard->bbBlackKing       = SetBitToOne( argsBoard->bbBlackKing, iSquare );
            argsBoard->mBoard[ iSquare ] = dBlackKing;
            iRow++;
            break;
         }
         }
      }

      // PlacePieceOnBoard( argsBoard );*/

      else if ( str1[ iCharacterIndex ] == '/' )
      {

         iCol--;
         iRow = 0;
         continue;
      }
      else if ( str1[ iCharacterIndex ] == '1' ||
                str1[ iCharacterIndex ] == '2' ||
                str1[ iCharacterIndex ] == '3' ||
                str1[ iCharacterIndex ] == '4' ||
                str1[ iCharacterIndex ] == '5' ||
                str1[ iCharacterIndex ] == '6' ||
                str1[ iCharacterIndex ] == '7' ||
                str1[ iCharacterIndex ] == '8' )
      {

         iRow += (int)str1[ iCharacterIndex ] - '0';
      }
   }

   // Set the color to move.
   if ( str2[ 0 ] == 'w' )
   {

      argsBoard->siColorToMove = dWhite;
   }
   else if ( str2[ 0 ] == 'b' )
   {

      argsBoard->siColorToMove = dBlack;
   }
   else
   {

      cout << endl
           << "Color to move not recognized." << endl;
      system( "Pause" );
   }

   // Set the Castling abilitites.
   argsBoard->iHasWhiteCastled = dYes;
   argsBoard->iHasBlackCastled = dYes;
   iNumberOfCharacters         = (int)strlen( str3 );
   for ( iCharacterIndex = 0; iCharacterIndex < iNumberOfCharacters; iCharacterIndex++ )
   {

      switch ( (int)str3[ iCharacterIndex ] )
      {

      case ( 'K' ):
      {

         argsBoard->bbCastle         = SetBitToOne( argsBoard->bbCastle, 0 );
         argsBoard->iHasWhiteCastled = dNo;
         break;
      }

      case 'k':
      {

         argsBoard->bbCastle         = SetBitToOne( argsBoard->bbCastle, 1 );
         argsBoard->iHasBlackCastled = dNo;
         break;
      }

      case 'Q':
      {

         argsBoard->bbCastle         = SetBitToOne( argsBoard->bbCastle, 2 );
         argsBoard->iHasWhiteCastled = dNo;
         break;
      }

      case ( 'q' ):
      {

         argsBoard->bbCastle         = SetBitToOne( argsBoard->bbCastle, 3 );
         argsBoard->iHasBlackCastled = dNo;
         break;
      }

      case '-':
      {

         break;
      }
      }
   }

   // Collect the EP square.
   // if ( strlen( str4 ) == 1 )
   if ( !strncmp( str4, "-", 1 ) )
   {

      // No EP Square.
      argsBoard->bbEP                          = 0;
      argsBoard->sHistoryStack[ 0 ].bbEPSquare = 0;
   }
   else // We have a square.
   {
      // Parse algebraic notation (e.g., "d6")
      // File: 'a' -> 0, 'b' -> 1...
      // Rank: '1' -> 0, '2' -> 1...
      // Note: str4 might contain a trailing space due to parsing logic, so we just take first 2 chars.

      int iFile = str4[ 0 ] - 'a';
      int iRank = str4[ 1 ] - '1';

      // cout << "DEBUG: EP Parsing str4='" << str4 << "' File=" << iFile << " Rank=" << iRank << endl;

      // Safety check
      if ( iFile >= 0 && iFile <= 7 && iRank >= 0 && iRank <= 7 )
      {
         int iSquare                              = iFile + iRank * 8;
         argsBoard->bbEP                          = SetBitToOne( argsBoard->bbEP, iSquare );
         argsBoard->sHistoryStack[ 0 ].bbEPSquare = SetBitToOne( argsBoard->sHistoryStack[ 0 ].bbEPSquare,
                                                                 iSquare );
         // cout << "DEBUG: Set EP bit for square " << iSquare << endl;
      }
      else
      {
         // Fallback or error logging? For now just ignore invalid EP to avoid crash/assert
         cout << "Invalid EP square in FEN: " << str4[ 0 ] << str4[ 1 ] << endl;
      }
   }

   // Set the half move clock.
   // Set the half move clock.
   if ( !strcmp( str5, "-" ) )
   {
      argsBoard->iFiftyMoveCounter = 0;
   }
   else
   {
      argsBoard->iFiftyMoveCounter = atoi( str5 );
   }

   // Set up the white pieces
   argsBoard->bbWhitePieces = argsBoard->bbWhitePawn |
                              argsBoard->bbWhiteRook |
                              argsBoard->bbWhiteKnight |
                              argsBoard->bbWhiteBishop |
                              argsBoard->bbWhiteQueen |
                              argsBoard->bbWhiteKing;

   // Set up the black pieces
   argsBoard->bbBlackPieces = argsBoard->bbBlackPawn |
                              argsBoard->bbBlackRook |
                              argsBoard->bbBlackKnight |
                              argsBoard->bbBlackBishop |
                              argsBoard->bbBlackQueen |
                              argsBoard->bbBlackKing;

   // Set up all the pieces.
   argsBoard->bbAllPieces = argsBoard->bbWhitePieces | argsBoard->bbBlackPieces;

   // Set the history count to zero.
   argsBoard->iMoveHistory = 0;

   // Calculate the initial moves.
   Move sDummyMoves[ dNumberOfMoves ];
   CalculateMoves( sDummyMoves,
                   argsBoard,
                   argsGeneralMoves );

   // Set the initial ply to minus 1.
   argsBoard->iNumberOfPlys = -1;

   // Ensure null-termination
   str1[ sizeof( str1 ) - 1 ] = '\0';
   str2[ sizeof( str2 ) - 1 ] = '\0';
   str3[ sizeof( str3 ) - 1 ] = '\0';
   str4[ sizeof( str4 ) - 1 ] = '\0';
   str5[ sizeof( str5 ) - 1 ] = '\0';
   str6[ sizeof( str6 ) - 1 ] = '\0';

   // Calculate the initial hash based on the board state.
   GenerateHashFromBoard( argsBoard, argsGeneralMoves );

   // Initialize NNUE accumulator for the starting position
   // Set iMoveHistory to 0 (not -1) so NNUE can access sNNUEHistory[0]
   argsBoard->iMoveHistory                                       = 0;
   argsBoard->sNNUEHistory[ 0 ].accumulator.computedAccumulation = 0; // Mark as uncomputed
   argsBoard->sNNUEHistory[ 0 ].dirtyPiece.dirtyNum              = 0; // No incremental update - will compute from scratch
}

//
//
//---------------------------------------------------------------------
//
// Random generator for a bit board.
BitBoard RandomBB()
{
   // This function creates a random bit board.

   // Set the initial board to zero.
   BitBoard bbBoard = 0;

   for ( int index = 0; index < 64; index++ )
   {

      double dRand = ( (double)rand() / (double)32767 );

      if ( dRand >= 0.5 )
      {

         bbBoard = SetBitToOne( bbBoard, index );
      }
   }

   // Debugging.
   assert( bbBoard >= 0 );

   // Return the random number.
   return bbBoard;
}

//
//
//---------------------------------------------------------------------
//
//
void ClearPV( struct Board *argsBoard )
{

   // This function resets the PV/
   // All value are set to -1;

   // Update the PV matrix.
   for ( int iPlyLoop1 = 0; iPlyLoop1 < dNumberOfPlys; iPlyLoop1++ )
   {

      // Update the PV matrix.
      for ( int iPlyLoop2 = 0; iPlyLoop2 < dNumberOfPlys; iPlyLoop2++ )
      {

         argsBoard->vmPrincipalVariation[ iPlyLoop1 ][ iPlyLoop2 ].iFromSquare  = 0;
         argsBoard->vmPrincipalVariation[ iPlyLoop1 ][ iPlyLoop2 ].iToSquare    = 0;
         argsBoard->vmPrincipalVariation[ iPlyLoop1 ][ iPlyLoop2 ].bbFromSquare = 0;
         argsBoard->vmPrincipalVariation[ iPlyLoop1 ][ iPlyLoop2 ].bbToSquare   = 0;
         argsBoard->vmPrincipalVariation[ iPlyLoop1 ][ iPlyLoop2 ].iCapture     = 0;
         argsBoard->vmPrincipalVariation[ iPlyLoop1 ][ iPlyLoop2 ].iPiece       = 0;
         argsBoard->vmPrincipalVariation[ iPlyLoop1 ][ iPlyLoop2 ].bbEPSquare   = 0;
         argsBoard->vmPrincipalVariation[ iPlyLoop1 ][ iPlyLoop2 ].bbCastle     = 0;
         argsBoard->vmPrincipalVariation[ iPlyLoop1 ][ iPlyLoop2 ].iMoveType    = 0;
      }
   }
}

//
//
//---------------------------------------------------------------------
//
//
// Generate FEN string from board position
void GenerateFENFromBoard( struct Board       *argsBoard,
                           struct GeneralMove *argsGeneralMoves,
                           char               *fenString )
{
   // Debug inputs
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );
   assert( fenString != NULL );

   char buffer[ 256 ] = { 0 };
   int  bufferPos     = 0;
   int  iEmptySquares = 0;

   // Loop through board from rank 8 to rank 1 (FEN standard)
   for ( int iCol = 7; iCol >= 0; iCol-- )
   {
      iEmptySquares = 0;

      for ( int iRow = 0; iRow < 8; iRow++ )
      {
         int iSquareIndex = iRow + iCol * 8;
         int piece        = argsBoard->mBoard[ iSquareIndex ];

         // If empty, count consecutive empty squares
         if ( piece == dEmpty )
         {
            iEmptySquares++;
         }
         else
         {
            // Output any pending empty squares
            if ( iEmptySquares > 0 )
            {
               buffer[ bufferPos++ ] = '0' + iEmptySquares;
               iEmptySquares         = 0;
            }

            // Output the piece character
            switch ( piece )
            {
            case dWhitePawn:
               buffer[ bufferPos++ ] = 'P';
               break;
            case dWhiteRook:
               buffer[ bufferPos++ ] = 'R';
               break;
            case dWhiteKnight:
               buffer[ bufferPos++ ] = 'N';
               break;
            case dWhiteBishop:
               buffer[ bufferPos++ ] = 'B';
               break;
            case dWhiteQueen:
               buffer[ bufferPos++ ] = 'Q';
               break;
            case dWhiteKing:
               buffer[ bufferPos++ ] = 'K';
               break;
            case dBlackPawn:
               buffer[ bufferPos++ ] = 'p';
               break;
            case dBlackRook:
               buffer[ bufferPos++ ] = 'r';
               break;
            case dBlackKnight:
               buffer[ bufferPos++ ] = 'n';
               break;
            case dBlackBishop:
               buffer[ bufferPos++ ] = 'b';
               break;
            case dBlackQueen:
               buffer[ bufferPos++ ] = 'q';
               break;
            case dBlackKing:
               buffer[ bufferPos++ ] = 'k';
               break;
            }
         }
      }

      // Output any remaining empty squares for this rank
      if ( iEmptySquares > 0 )
      {
         buffer[ bufferPos++ ] = '0' + iEmptySquares;
      }

      // Add rank separator (except after the last rank)
      if ( iCol > 0 )
      {
         buffer[ bufferPos++ ] = '/';
      }
   }

   // Add side to move
   buffer[ bufferPos++ ] = ' ';
   buffer[ bufferPos++ ] = ( argsBoard->siColorToMove == dWhite ) ? 'w' : 'b';

   // Add castling rights
   buffer[ bufferPos++ ] = ' ';
   int hasCastling       = 0;

   // Castling availability (check bbCastle bitfield)
   // bit 0: White kingside
   // bit 1: Black kingside
   // bit 2: White queenside
   // bit 3: Black queenside
   if ( argsBoard->bbCastle & 0x01 )
   {
      buffer[ bufferPos++ ] = 'K';
      hasCastling           = 1;
   }
   if ( argsBoard->bbCastle & 0x04 )
   {
      buffer[ bufferPos++ ] = 'Q';
      hasCastling           = 1;
   }
   if ( argsBoard->bbCastle & 0x02 )
   {
      buffer[ bufferPos++ ] = 'k';
      hasCastling           = 1;
   }
   if ( argsBoard->bbCastle & 0x08 )
   {
      buffer[ bufferPos++ ] = 'q';
      hasCastling           = 1;
   }

   if ( !hasCastling )
   {
      buffer[ bufferPos++ ] = '-';
   }

   // Add en passant target square
   buffer[ bufferPos++ ] = ' ';
   if ( argsBoard->bbEP == 0 )
   {
      buffer[ bufferPos++ ] = '-';
   }
   else
   {
      // Find EP square and convert to algebraic notation
      int epSquare = 0;
      Find( argsBoard->bbEP, &epSquare, argsGeneralMoves );

      char colChar          = 'a' + ( epSquare % 8 );
      char rowChar          = '1' + ( epSquare / 8 );
      buffer[ bufferPos++ ] = colChar;
      buffer[ bufferPos++ ] = rowChar;
   }

   // Add halfmove clock
   buffer[ bufferPos++ ] = ' ';
   // Convert integer to string
   char fiftyStr[ 16 ];
   sprintf( fiftyStr, "%d", argsBoard->iFiftyMoveCounter );
   for ( int i = 0; fiftyStr[ i ] != '\0'; i++ )
   {
      buffer[ bufferPos++ ] = fiftyStr[ i ];
   }

   // Add fullmove number (set to 1 if not tracked)
   buffer[ bufferPos++ ] = ' ';
   buffer[ bufferPos++ ] = '1';

   // Null terminate
   buffer[ bufferPos ] = '\0';

   // Copy to output
   strcpy( fenString, buffer );
}