#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
//#include <stdlib.h>
#include <fstream>
#include "Functions.h"
#include "Structures.h"
// #include <omp.h>
#include "Definitions.h"
#include <math.h>
#include <string.h>

using namespace std;

int giTotalCount;
int giMarginalCount;
int giCutCount;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Define global variables for keeping track of the number of nodes counted.
// Global variables suck, but are awsome for allowing for Deep Violet.
// access to the table:
// Note that the scope for the globe variables is only this file.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   //HashTable gsOpeningBook;
   OpeningBook gsOpeningBook;

   // A global file for writing out a checked book.  Needed because of the recursive nature of the calculation.
   ofstream gofCheckedBook;

   // A global file for debug output for the interface and the book.
   ofstream gofDebugBook;

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

//
//---------------------------------------------------------------------------------------------------------------------------------------
//

//
//-------------------------------------------------------------------------------------------------------------
//
void InitializeOpeningBook()
{
// This function is used to initialize the hash table.  The function is stored in the search file because the 
// hash table is a global variable and is needed the most in the search routines.
//

   // Allocate the memory.
   gsOpeningBook.bbOpeningBookSize = Power( 2, dNumberOfBitsInOpeningBook ) - 1;
   gsOpeningBook.mbbHash = ( BitBoard * ) malloc( gsOpeningBook.bbOpeningBookSize * sizeof( BitBoard ) );
   gsOpeningBook.mbbHashTable = ( BitBoard * ) malloc( gsOpeningBook.bbOpeningBookSize * sizeof( BitBoard ) ); 
  
   // Loop over the hash table entries and set them to zero.
   // #pragma omp parallel for schedule( dynamic, 1 )
   for ( BitBoard bbHashIndex = 0; bbHashIndex < gsOpeningBook.bbOpeningBookSize; bbHashIndex++ )
   {

      gsOpeningBook.mbbHashTable[ bbHashIndex ] = 0;
      gsOpeningBook.mbbHash[ bbHashIndex ]      = 0;

   }
   
   // Set up the mask for extracting the index.
   gsOpeningBook.bbMaskIndex = 0;
   for ( int iBitIndex = 0; iBitIndex < dNumberOfBitsInOpeningBook; iBitIndex++ )
   {
   
      gsOpeningBook.bbMaskIndex = SetBitToOne( gsOpeningBook.bbMaskIndex, iBitIndex );
      
   }

   // Get ready to count the positions in the book.
   gsOpeningBook.bbNumberOfPositionsInBook = 0;

   // Set the initial hash for later reference
   //gsOpeningBook.bbHashInitial = gsOpeningBook.bbHash;

}

//
//---------------------------------------------------------------------------------------------------------------------------------
//
void DestroyOpeningBook()
{
// This function releases the memory taken by the hash table.

   free( gsOpeningBook.mbbHash );
   free( gsOpeningBook.mbbHashTable );

}


// Define some book sets.
void SetBookElement( BitBoard bbKey,
                     BitBoard bbElement )
{
   gsOpeningBook.mbbHashTable[ bbKey ] = bbElement;
} 
void SetBookElementHash( BitBoard bbKey,
                         BitBoard bbElement )
{
   gsOpeningBook.mbbHash[bbKey ] = bbElement;
}


// Define some input and some output variables.
BitBoard GetBookElement( BitBoard bbKey )
{
   
   assert( bbKey <= gsOpeningBook.bbOpeningBookSize );

   return gsOpeningBook.mbbHashTable[ bbKey ];
} 
BitBoard GetBookElementHash( BitBoard bbKey )
{

   assert( bbKey <= gsOpeningBook.bbOpeningBookSize );

   return gsOpeningBook.mbbHash[ bbKey ];
} 





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Some analysis elements
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OpeningBookAnalysis( struct Board * argsBoard,
                          struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   //////////////////////////////////////////////
   //  Warning: Turn off the Hash table before using.
   //  This is done in the GameControl file.
   ////////////////////////////////////////////////

   int iFlagBook = 1;
   int iFlagLine = 1;
   char strMove[ 64 ];
   char strMoveNumber[ 64 ];
   int iMoveCount = 0;
   int iMoveCountOld = -1;
   int iBlackMove = 0;
   int iPlyCount = 0;
   int iNumberOfCharacters = 0;
   int iGameResult;
   int iGameCount = 0;
   int iIncrementalGameCount = 0;
   int iIsGoodGame = 1;
   int iBadGameCount = 0;
   char strFileName[ 21 ];
   char strBookName[ 21 ];
   int iGoodWhiteELO = 0;
   int iGoodBlackELO = 0;
   BitBoard bbHashInitial = GetHash();
   int iMaxPlyIndex = 81;

   // Clear the string.
   strcpy( strFileName, "TWICGames.txt" );
   strcpy( strBookName, "BookTWIC.txt" );
   //strcpy( strFileName, "MentorGames.txt" );
   //strcpy( strBookName, "BookMentor.txt" );

   // Loop over the moves
   int iPlyIndex = 20;
   //for ( int iPlyIndex = 20; iPlyIndex < iMaxPlyIndex; iPlyIndex++ )
   {

      // Reset the flag for the book.
      iFlagBook = 1;
      
      // Open the file.
      ifstream ifBook( strFileName );
      if ( ifBook.fail() )
      {
      
         cout << "Input file failed to open." << endl;
         iFlagBook = 0;
      
      }

      // Reset some counters
      iGameCount = 0;
      iIncrementalGameCount = 0;

      // Loop over the data file.
      while( iFlagBook )
      {

         // Print out some diagonistics for the folks at home.
         if ( GetOpeningBookDebug() )
         {

            PrintOpeningBookMoveStatistics( argsBoard,
                                            argsGeneralMoves );
          
         }

         iMoveCount = 1;
         iPlyCount  = 1;
         iFlagLine = 1;

         iGameCount++;
         iIncrementalGameCount++;
         iIsGoodGame = 1;

         // Write out the opening book if we have read a reasonable number of games.
         if ( iIncrementalGameCount == 5000 )
         {

            // Let the folks at home know what is going on.
            cout << "iGameCount = " << iGameCount << " iPlyIndex = " << iPlyIndex << endl;

            //WriteOutOpeningBook( argstrBook );
            WriteOutOpeningBook( strBookName,
                                 iPlyIndex );
            iIncrementalGameCount = 0;
            PrintOpeningBookMoveStatistics( argsBoard,
                                            argsGeneralMoves );
            cout << "Bad game count = " << iBadGameCount << endl << endl;
            //cout << "File  = " << iFileIndex << endl << endl;

         }

         // Reset the game result.
         iGameResult = 0;

         // Look for an ELO setting
         iGoodBlackELO = 0;
         iGoodWhiteELO = 0;

         // Loop over a game.
         while( iFlagLine )
         {

            // Input a string.
            ifBook >> strMove;
            if ( GetOpeningBookDebug() )
            {
               cout << strMove << endl;
            }
         
            // Put the move number into a string.
            iNumberOfCharacters = sprintf( strMoveNumber, "%d.", iMoveCount );

            // Look to see if we are at the end of a game.
            if ( LookForGameResult( strMove, 
                                    & iGameResult ) == 1 )
            {

               iFlagLine = 0;
               break;
      
            }
            if ( LookForGameResult( strMove, 
                                    & iGameResult ) == -1 )
            {

               iFlagLine = 0;
               iFlagBook = 0;
               break;
      
            }

            // Limit the ELO if appropriate.
            if ( GetLimitELO() )
            {
   
               if ( ! strcmp( "[WhiteElo", strMove ) )
               {

                  // A white ELO setting has been found.
                  // Get the actual ELO.
                  ifBook >> strMove;
                  if ( GetOpeningBookDebug() )
                  {
                     cout << strMove << endl;
                  }

                  // Remove the quotation mark and rest will be ignored.
                  strncpy( strMove, " ", 1 );

                  // Calculate the ELO from the string.
                  int iWhiteELO = atoi( strMove );

                  // See if we have a good ELO.
                  if ( iWhiteELO >= 2500 )
                  {

                     iGoodWhiteELO = 1;

                  }

                  // Get another string
                  ifBook >> strMove;

               }
               // Look for an ELO setting
               if ( ! strcmp( "[BlackElo", strMove ) )
               {

                  // A white ELO setting has been found.
                  // Get the actual ELO.
                  ifBook >> strMove;
                  if ( GetOpeningBookDebug() )
                  {
                     cout << strMove << endl;
                  }

                  // Remove the quotation mark and rest will be ignored.
                  strncpy( strMove, " ", 1 );

                  // Calculate the ELO from the string.
                  int iBlackELO = atoi( strMove );

                  // See if we have a good ELO.
                  if ( iBlackELO >= 2500 )
                  {

                     iGoodBlackELO = 1;
                     ifBook >> strMove;

                  }

               }
            
            }
            else
            {

               //If we reach here, we are not limiting the ELO
               iGoodWhiteELO = 1;
               iGoodBlackELO = 1;

            }


            // Look for a move.
            if ( strncmp( strMove, strMoveNumber, strlen ( strMoveNumber ) ) == 0 )
            {


               // See if we found good ELO's for this game.
               if ( ( iGoodWhiteELO == 0 ) ||
                    ( iGoodBlackELO == 0 ) )
               {

                  iIsGoodGame = 0;

               }

               // We have a new move.
               if ( iIsGoodGame )
               {

                  iMoveCount++;
                  iPlyCount++;

               }

               // Reset the global count to avoid an over flow.
               //gbbNumberOfIncrementalNodes = 0;
               //gbbNumberOfNodes = 0;

               // Get the white move. Make sure the move number and move have not white space.
               if ( strlen( strMove ) > strlen( strMoveNumber ) )
               {  

                  char strMoveDummy[ 64 ] = " ";
                  int iMoveNumberLength = (int)(strlen( strMoveNumber ));
                  int iMoveLength       = (int)(strlen( strMove       ));
                  int iDummy            = (int)(strlen( strMoveDummy  ));
                  strncpy( strMoveDummy, strMove + strlen( strMoveNumber ), strlen( strMove ) - strlen( strMoveNumber ) );  
                  strcpy( strMove, strMoveDummy ); 
 
               }
               else
               {

                  ifBook >> strMove;
                  if ( GetOpeningBookDebug() )
                  {
                     cout << strMove << endl;
                  }

               }

               // Read a move from the file.
               ReadAPGNMove( & iGameResult,
                             & iFlagLine,
                             & iFlagBook,
                             & iIsGoodGame,
                             & iGoodWhiteELO,
                             & iGoodBlackELO,
                             strMove,
                             argsBoard,
                             argsGeneralMoves,
                             iPlyCount,
                             iPlyIndex );

               if ( iFlagLine == 0 )
               {

                  break;

               }

               // Get the Black move.
               ifBook >> strMove;
               if ( GetOpeningBookDebug() )
               {
                  cout << strMove << endl;
               }

               // We have a new move.
               if ( iIsGoodGame )
               {

                  iPlyCount++;

               }

               // Read a move from the file.
               ReadAPGNMove( & iGameResult,
                             & iFlagLine,
                             & iFlagBook,
                             & iIsGoodGame,
                             & iGoodWhiteELO,
                             & iGoodBlackELO,
                             strMove,
                             argsBoard,
                             argsGeneralMoves,
                             iPlyCount,
                             iPlyIndex );

               if ( iFlagLine == 0 )
               {

                  break;

               }

            }

         }

         //  At the end of a game, to the statistics
         if ( iIsGoodGame )
         {
         
            int iMoveHistory = argsBoard->iMoveHistory + 2;
            for ( int iReverseIndex = 0; iReverseIndex < iMoveHistory; iReverseIndex++ )
            {

               // Roll back over the game and update the statistics.
               //PrintBoard( argsBoard->mBoard );
               UpdateOpeningBook( argsBoard,
                                  argsGeneralMoves,
                                  iGameResult );

               //cout << "index = " << iReverseIndex << " History = " << iMoveHistory << endl;
               if ( iReverseIndex < iMoveHistory - 1 )
               {

                  UndoMove( argsBoard,
                            argsGeneralMoves );

               }

            }

         }

         if ( iIsGoodGame == 0 )
         {

            iBadGameCount++;
            //cout << "Bad game." << endl;

         }

         // Reset the board, hard.
         CreateBoard( argsBoard,
                      argsGeneralMoves ); 
      
         // Reset the hash should we have lost it.
         SetHash( bbHashInitial );
         //gsHashTable.bbHash = gsHashTable.bbHashInitial;

         // Prepare for the next game.
         iFlagLine = 1;

         // Reset the ELO flags.
         iGoodWhiteELO = 0;
         iGoodBlackELO = 0;

      }

      // Close the input book
       ifBook.close();

      // Write the output for the ply.
      WriteOutOpeningBook( strBookName,
                           iPlyIndex );


   }

   // Write out the book one last time.
   WriteOutOpeningBook( strBookName,
                        iMaxPlyIndex );

}

void ReadAPGNMove( int * iGameResult,
                   int * iFlagLine,
                   int * iFlagBook,
                   int * iIsGoodGame,
                   int * iGoodWhiteELO,
                   int * iGoodBlackELO,
                   char * strMove,
                   struct Board * argsBoard,
                   struct GeneralMove * argsGeneralMoves,
                   int iPlyCount,
                   int iPlyIndex )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // This routine reads a move, either black or white from a file.
   Move vsMoveList[ dNumberOfMoves ];
   int iMoveNumber;

   // Look to see if we are at the end of a game.
   // Look for an ending.
   if ( LookForGameResult( strMove, 
                           iGameResult ) == 1 )
   {

      // Reset the ELO flags.
      * iGoodWhiteELO = 0;
      * iGoodBlackELO = 0;

      * iFlagLine = 0;
      return;
         
   }
   // Look for an EOF or more precisely an empty strMove.
   if ( LookForGameResult( strMove, 
                           iGameResult ) == -1 )
   {

      // Reset the ELO flags.
      * iGoodWhiteELO = 0;
      * iGoodBlackELO = 0;

      * iFlagLine = 0;
      * iFlagBook = 0;
      return;
         
   }

   // Do the hash thing and up date the board if we are at a ply of interest.
   // if ( iIsGoodGame )
   //PrintBoard( argsBoard->mBoard );
   if ( iIsGoodGame &&
        ( iPlyCount <= iPlyIndex + 1 ) )
   {
 
      // Calculate the moves.
      CalculateMoves( vsMoveList, 
                      argsBoard, 
                      argsGeneralMoves );

      // Get the matching move number.
      iMoveNumber = GetMoveNumberFast( argsBoard,
                                       argsGeneralMoves,
                                       vsMoveList,
                                       strMove ); 

      // Look for a bad move in the data.
      if ( iMoveNumber == -1 )
      {
         
         /*
         PrintBoard( argsBoard->mBoard );
         PrintFEN( argsBoard,
                   argsGeneralMoves );
         */
         * iIsGoodGame = 0;

      }

      if ( * iIsGoodGame )
      {

         // If the move was found make it move on with life.
         MakeMove( vsMoveList, 
                     argsBoard,
                     argsGeneralMoves,
                     iMoveNumber );

         // Reset the current ply depth.
         argsBoard->iNumberOfPlys = -1;

      }

   }

}


int LookForGameResult( char * argstrMove, 
                       int * argiGameResult )
{

   // Debug the inputs.
   assert( argstrMove >= 0 );

   // Declare some variables.
   int iResultFound = 0;

   if ( strcmp( argstrMove, "1-0" ) == 0 )
   {

      * argiGameResult = dWhiteWin;
      iResultFound = 1;

   }   
   if ( strcmp( argstrMove, "0-1" ) == 0 )
   {

      * argiGameResult = dBlackWin;
      iResultFound = 1;

   }
   if ( strcmp( argstrMove, "1/2-1/2" ) == 0 )
   {

      * argiGameResult = dDraw;
      iResultFound = 1;

   }
   if ( strcmp( argstrMove, "*" ) == 0 )
   {

      * argiGameResult = dUnknown;
      iResultFound = 1;

   }
   if ( strcmp( argstrMove, "" ) == 0 )
   {

      iResultFound = -1;

   }

   assert( iResultFound >= -1 );
   assert( iResultFound <= 1 );

   // Return whether or not we found a result
   return iResultFound;

}

//
//----------------------------------------------------------------------------------------------------------
//
void PrintOpeningBookMoveStatistics( struct Board * argsBoard,
                                     struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Print the statistics from the open book for the moves from a given position.
   int iMoveCount;
   Move vsMoveList[ dNumberOfMoves ];
   int viSortOrder[ dNumberOfMoves ];
   int viPopularity[ dNumberOfMoves ];
   BitBoard bbWhiteWins = 0;
   BitBoard bbBlackWins = 0;
   BitBoard bbDraws = 0;
   char strMove[ 64 ];
   int iNumberOfChars;
   int iCharCount;
   int iNumberOfMoves;
   double dPercentWhiteWins = 0;
   double dPercentBlackWins = 0;
   double dPercentDraws = 0;
   BitBoard bbTotalGames = 0;

   //cout << "Inside the stats routine." << endl;

   // Calculate the moves for this position.
   CalculateMoves( vsMoveList,
                   argsBoard,
                   argsGeneralMoves );

   //cout << "After calculating the moves." << endl;

   cout << "Depth = " << argsBoard->iMoveHistory << endl;


   iNumberOfMoves = argsBoard->siNumberOfMoves;

   cout << endl;

   for ( iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++ )
   {

      // Make the move.
      MakeMove( vsMoveList,
                argsBoard,
                argsGeneralMoves,
                iMoveCount );

      // Get the statistics.
      ExtractOpeningBookStats( bbWhiteWins,
                               bbBlackWins,
                               bbDraws,
                               argsGeneralMoves );

      // Sum the total games.
      bbTotalGames += bbWhiteWins;
      bbTotalGames += bbBlackWins;
      bbTotalGames += bbDraws;

      // Undo the move.
      UndoMove( argsBoard,
                argsGeneralMoves );

      // Collect the popularity.
      viPopularity[ iMoveCount ] = (int)(bbWhiteWins + bbBlackWins + bbDraws);
      viSortOrder[ iMoveCount ] = iMoveCount;
          
   }

   // Sort on the popularity
   int iSortFlag = 1;
   while( iSortFlag )
   {

      // Set the default to bail.
      iSortFlag = 0;

      // Use a cocktail sort and to from top to bottom
      for ( int iMoveIndex = 0; iMoveIndex < iNumberOfMoves - 1; iMoveIndex++ )
      {

         if ( viPopularity[ iMoveIndex + 1 ] > viPopularity[ iMoveIndex ] )
         {

            int iDummyScore                = viPopularity[ iMoveIndex ];
            viPopularity[ iMoveIndex ]     = viPopularity[ iMoveIndex + 1 ];
            viPopularity[ iMoveIndex + 1 ] = iDummyScore;

            int iDummyPosition            = viSortOrder[ iMoveIndex ];
            viSortOrder[ iMoveIndex ]     = viSortOrder[ iMoveIndex + 1 ];
            viSortOrder[ iMoveIndex + 1 ] = iDummyPosition;

            iSortFlag = 1;

         }

      }
           
   }

   // This loop is for printing out.
   for ( iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++ )
   {

      // Only show non-zero popularities.
      if ( viPopularity[ iMoveCount ] > 0 )
      {      

         // Print out the move name.
         iNumberOfChars = PrintMove( argsBoard,
                                     argsGeneralMoves,
                                     & vsMoveList[ viSortOrder[ iMoveCount ] ],
                                     strMove );

         // Write out the results
         for ( iCharCount = 0; iCharCount < iNumberOfChars; iCharCount++ ) 
         {

            cout << strMove[ iCharCount ];

         }

         // Write out spaces to line up the output.
         for ( iCharCount = 0; iCharCount < 6 - iNumberOfChars; iCharCount++ )  
         {

            cout << " ";

         }

         cout << " Popularity = ";

         // Left justify the numbers;
         for ( iCharCount = 0; iCharCount < 8 - log10( (double)viPopularity[ iMoveCount ] ); iCharCount++ )
         {

            cout << " ";

         }

         cout << viPopularity[ iMoveCount ] << endl;
       
      }
   
   }

   // Show total games in the book.
   cout << "Total games in book = " << bbTotalGames << endl << endl;

   // Close the book if no games are found.
   if ( bbTotalGames == 0 )
   {

      SetIsInBook( dNo );

   }

}

//
//----------------------------------------------------------------------------------------------------------
//
// This function returns the actual move in the argument argsBestMove.
// The argtiBookMove is -1 if no book move was found.
void FindBookMove( struct Board * argsBoard,
                   struct GeneralMove * argsGeneralMoves,
                   struct Move * argsvMoveList,
                   struct Move * argsBestMove,
                   int * argiBookMove )
{

   // Debug the inputs.
   assert( argsBoard        >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Print the statistics from the open book for the moves from a given position.
   BitBoard bbWhiteWins = 0;
   BitBoard bbBlackWins = 0;
   BitBoard bbDraws     = 0;
   int iNumberOfMoves;
   int iMoveCount;
   int iWins                  = 0;
   int iTotalGames            = 0;
   int iNumberOfFeasibleMoves = 0;
   double dPercentNotLoss     = 0;
   double vdPercentWhiteWins[ dNumberOfMoves ];
   double vdPercentBlackWins[ dNumberOfMoves ];
   double vdPercentDraws[     dNumberOfMoves ];
   double vCumPopularity[     dNumberOfMoves ];
   int viTotalGames[          dNumberOfMoves ];
   int viMoveList[            dNumberOfMoves ];
   char strMove[ 64 ];


   // Calculate the moves for this position.
   CalculateMoves( argsvMoveList,
                   argsBoard,
                   argsGeneralMoves );

   // Some Debugging.
   if ( GetInterfaceBookDebug() )
   {

      gofDebugBook << "Hash at start = " << GetHash() << endl << endl;

   }

   // Extract the number of moves.
   iNumberOfMoves = argsBoard->siNumberOfMoves;

   // Some Debugging.
   if ( GetInterfaceBookDebug() )
   {

      gofDebugBook << endl;
      gofDebugBook << "Here are a list of the moves." << endl << endl;

   }

   // Loop over the moves.
   for ( iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++ )
   {

      // Set the score of the move to zero, as the score here is based on popularity and
      // not the standard move scoring used for searching
      argsvMoveList[ iMoveCount ].iScore = 0;

      // Make the move.
      MakeMove( argsvMoveList,
                argsBoard,
                argsGeneralMoves,
                iMoveCount );

      // Get the statistics.
      ExtractOpeningBookStats( bbWhiteWins,
                               bbBlackWins,
                               bbDraws,
                               argsGeneralMoves );

      // Undo the move.
      UndoMove( argsBoard,
                argsGeneralMoves );

      // Write out the stats in terms of percentages.
      // Only consider moves above the Violet threshold of quality.
      if ( ( bbWhiteWins + bbBlackWins + bbDraws ) > 0 )
      {

         vdPercentWhiteWins[ iMoveCount ] = (double)( bbWhiteWins ) / 
                                            ( (double)( bbWhiteWins ) + 
                                              (double)( bbBlackWins ) + 
                                              (double)( bbDraws ) ) * 
                                            100.0;
         vdPercentBlackWins[ iMoveCount ] = (double)( bbBlackWins ) / 
                                            ( (double)( bbWhiteWins ) + 
                                              (double)( bbBlackWins ) + 
                                              (double)( bbDraws ) ) * 
                                            100.0;
         vdPercentDraws[ iMoveCount ]     = (double)( bbDraws )     / 
                                            ( (double)( bbWhiteWins ) + 
                                              (double)( bbBlackWins ) + 
                                              (double)( bbDraws ) ) * 
                                            100.0;

      }
      else
      {

         vdPercentWhiteWins[ iMoveCount ] = 0;
         vdPercentBlackWins[ iMoveCount ] = 0;
         vdPercentDraws[ iMoveCount ]     = 0;

      }

      // Here are criteria for rejecting a book move.
      viTotalGames[ iMoveCount ] = (int)bbWhiteWins + (int)bbBlackWins + (int)bbDraws;

      // Calculate the total games for this position.
      iTotalGames += viTotalGames[ iMoveCount ]; 
      
      if ( argsBoard->siComputerColor == dComputerWhite )
      {

         iWins = (int)bbWhiteWins;
         dPercentNotLoss = vdPercentWhiteWins[ iMoveCount ] +  vdPercentDraws[ iMoveCount ];

      }
      if ( argsBoard->siComputerColor == dComputerBlack )
      {

         iWins = (int)bbBlackWins;
         dPercentNotLoss = vdPercentBlackWins[ iMoveCount ] +  vdPercentDraws[ iMoveCount ];

      }

      // See if the move is feasibility.
      if ( ( viTotalGames[ iMoveCount ] >= dMoveCutOff ) &&
           ( iWins                      >= dWinCutOff  ) &&
           ( dPercentNotLoss            >= dNotALoss   ) )
      {

         // Update the number of feasible moves.
         iNumberOfFeasibleMoves++;

         // If the move is feasible, set the move score to the total number of games,
         // this will allow the moves to be sorted by popularity.
         argsvMoveList[ iMoveCount ].iScore = viTotalGames[ iMoveCount ];

      }
      else
      {

         // Make sure the score is zero.
         argsvMoveList[ iMoveCount ].iScore = 0;

      }

         strncpy( strMove, "      ", 6 );
         // Create a book move string.
         CreateAlgebraicMove( strMove,
                              & argsvMoveList[ iMoveCount ],
                              0 );

      // Some Debugging.
      if ( GetInterfaceBookDebug() )
      {

         gofDebugBook << endl;
         gofDebugBook << "iMoveCount = " << iMoveCount << 
                         " Move = " << strMove << 
                         " Score = " << argsvMoveList[ iMoveCount ].iScore << endl;

      }
         
   }

   // If not feasible, call the book quits.
   if ( iNumberOfFeasibleMoves == 0 )
   {

      // Return a book fail.
      * argiBookMove = -1;

      // Mark the board as being out of book.
      // This is imortant in other parts of the code where 
      // the function ClearHash() is called.
      SetIsInBook( dNo );

      return;

   }

   // Sort the moves according to popularity.
   SortMoves( viMoveList,
              argsvMoveList,
              iNumberOfMoves );

   // Some Debugging.
   if ( GetInterfaceBookDebug() )
   {

      gofDebugBook << endl;
      gofDebugBook << "Here is a list of the sorted Moves." << endl << endl;

   }

   // Also calculate the cumulative popularity of the moves.
   for ( iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++ )
   {

      // Calculate the cumulative popularity.
      if ( iMoveCount == 0 )
      {

         // Assign the first value;
         vCumPopularity[ iMoveCount ] = (double)( argsvMoveList[ viMoveList[ iMoveCount ] ].iScore ) /
                                        (double)( iTotalGames );

      }
      else
      {

         // Assign the cumulative values;
         vCumPopularity[ iMoveCount ] = (double)( argsvMoveList[ viMoveList[ iMoveCount ] ].iScore ) /
                                        (double)( iTotalGames ) + 
                                        vCumPopularity[ iMoveCount - 1 ];         

      }

         strncpy( strMove, "      ", 6 );
         // Create a book move string.
         CreateAlgebraicMove( strMove,
                              & argsvMoveList[ viMoveList[ iMoveCount ] ],
                              0 );

      // Some Debugging.
      if ( GetInterfaceBookDebug() )
      {

         gofDebugBook << endl;
         gofDebugBook << "iMoveCount = " << iMoveCount <<  
                         " Move = " << strMove << 
                         " CumPop = " << vCumPopularity[ iMoveCount ] << endl;

      }
      
   }

   // Some QA/QC
   assert( vCumPopularity[ iNumberOfMoves - 1 ] > 0.999 );
   assert( vCumPopularity[ iNumberOfMoves - 1 ] < 1.001 );

   // Some Debugging.
   if ( GetInterfaceBookDebug() )
   {

      gofDebugBook << "Hash at end = " << GetHash() << endl << endl;

   }

   // Choose the move to make via a random number and normalize to the popularity cutoff.
   // This means that only moves that have a cumulative popularity of 90% will be played.
   // This is done to stop the weirder in the database from being played.
   double dSamplePopularity =  (double)( rand() ) / (double)( RAND_MAX ) * dPopularityCutOff;

   // Some Debugging.
   if ( GetInterfaceBookDebug() )
   {

      gofDebugBook << endl;
      gofDebugBook << "Here is the random number = " << dSamplePopularity << endl << endl;

   }

   // Loop over the cumulative popularity to find a move.
   for ( int iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++ )
   {

      // Some Debugging.
      if ( GetInterfaceBookDebug() )
      {

         gofDebugBook << endl << "iMoveCount = " << iMoveCount << endl << endl;
 
      }   

      // See if we are freshly over the criteria for a good move.
      // As a redundate check, make sure the move is feasable.
      if ( vCumPopularity[ iMoveCount ] >= dSamplePopularity )
      {

         // Some Debugging.
         if ( GetInterfaceBookDebug() )
         {

            gofDebugBook << "Now setting the move." << endl;  
 
         }   

         * argiBookMove = viMoveList[ iMoveCount ];
         * argsBestMove = argsvMoveList[ * argiBookMove ];
         assert( * argiBookMove >= -1 );
         assert( * argiBookMove <= dNumberOfMoves );

         gofDebugBook <<  "argiBooMove = " << * argiBookMove << endl;

         strncpy( strMove, "      ", 6 );
         // Create a book move string.
         CreateAlgebraicMove( strMove,
                              argsBestMove,
                              0 );

         // Some Debugging.
         if ( GetInterfaceBookDebug() )
         {

            gofDebugBook << endl;
            gofDebugBook << "Just after FindBookMove strMove = " << strMove << endl;
 
         }   

         return;

      }

   }

   // This point should not have been reached.
   // If it is reached, return a book fail.
   * argiBookMove = iNumberOfMoves - 1;
   return;
  
}

//
//---------------------------------------------------------------------------------------------------------------
//
int FindMaxScore ( double * vdWins,
                   int argiNumberOfMoves )
{
   int iMoveNumber = -1;
   int iNumberOfMoves;
   int iMaxWins = -1;

   // Loop over the statistics.
   for ( iNumberOfMoves = 0; iNumberOfMoves < argiNumberOfMoves; iNumberOfMoves++ )
   {

      // See if we have a good score and that the score exists.
      if ( ( iMaxWins < vdWins[ iNumberOfMoves ] ) &&
           ( vdWins[ iNumberOfMoves ] > 0 ) )
      {

         iMaxWins = (int)vdWins[ iNumberOfMoves ];
         iMoveNumber = iNumberOfMoves;

      }

   }
  
   // Look for a failure.
   if ( iMaxWins <= dWinCutOff )
   {

      iMoveNumber = -1;

   }

   assert( iMoveNumber >= -1 );
   assert( iMoveNumber <= dNumberOfMoves );

   return iMoveNumber;

}

//
//------------------------------------------------------------------------------------------------------------------
//
void CombineOpeningBooks( struct Board * argsBoard,
                          struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // This funtion is used to combine opening books that were generated seperately.

   // Delcare some variables.
   int iReadMoreFlag = 1;

   // Read the first book
   ReadOpeningBook( "BookTWIC20.txt",
                    argsGeneralMoves );

   PrintOpeningBookMoveStatistics( argsBoard,
                                   argsGeneralMoves );
 
   // Append a book.
   AppendBook( argsGeneralMoves,
               "BookMentor20.txt" );

   // Print the opening book move statistics for this position.
   PrintOpeningBookMoveStatistics( argsBoard,
                                   argsGeneralMoves );
   // Write out the combined book.
   WriteOutOpeningBook( "VioletBook.txt", 0 );

   // Trim opening book.
   TrimOpeningBook( argsGeneralMoves );

   // Write out the combined book.
   WriteOutOpeningBook( "VioletBookTrimmed.txt", 0 );

}

//                                                              
//-----------------------------------------------------------------------------------------------------------
//
void WriteOutOpeningBook( const char * argstrBook,
                          int iPlyIndex )
{
   // Write out the opening book to a file.
   char strPly[ 2 ];
   char strBookNameNumbered[ 64 ];
   char * strEndOfLine;

   // Copy the ply to an integer.
   sprintf( strPly, "%d", iPlyIndex );

   // Create a fresh copy of the file name.
   strcpy( strBookNameNumbered, argstrBook );

   // Find the period in the name and replace.
   strEndOfLine = strchr( strBookNameNumbered, '.' );

   // Trim the end of line.
   * strEndOfLine = 0;

   // Insert the ply number.
   strcat( strBookNameNumbered, strPly );

   // Add the file type.
   strcat( strBookNameNumbered, ".txt" );

   // open the output file.
   ofstream ofBook( strBookNameNumbered, ofstream::binary );
   if ( ofBook.fail() )
   {
   
      cout << "Output Book.txt file failed to open." << endl;
      
   }

   for ( BitBoard bbHashCount = 0; bbHashCount < gsOpeningBook.bbOpeningBookSize; bbHashCount++ )
   {

      //if ( gsHashTable.mbbHashTable[ bbHashCount ] != 0 )
      if ( GetBookElement( bbHashCount ) != 0 )
      {

         ofBook << GetBookElementHash( bbHashCount ) << " " << GetBookElement( bbHashCount ) << " " << endl;
     
      }

   }

   ofBook.close();

}

// 
//----------------------------------------------------------------------------------------------------------
//
void ReadOpeningBook( const char * argstrBookName,
                      struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argstrBookName >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Delcare some variables.
   int iReadMoreFlag = 1;
   BitBoard bbKey;
   BitBoard bbData;
   BitBoard bbHash;

   // Read the opening book.
   ifstream ifBook( argstrBookName, ifstream::binary );
   if ( ifBook.fail() )
   {
   
      SetUseOpeningBook( dNo );
      return;
      //cout << "Output Book.txt file failed to open." << endl;
      
   }

   // Loop over the file.
   while( !ifBook.eof( ) )
   {

      // Get the key from the file.
      ifBook >> bbHash;

      // Calculate the key.
      bbKey = bbHash & gsOpeningBook.bbMaskIndex;

      assert( bbKey < gsOpeningBook.bbOpeningBookSize );

      // Get the data.
      ifBook >> bbData;

      // Put the book in the hash table.
      SetBookElement( bbKey,
                      bbData );

      SetBookElementHash( bbKey,
                          bbHash );

      // Count the number of Positions.
      gsOpeningBook.bbNumberOfPositionsInBook++;

   }

   ifBook.close();

}

//
//------------------------------------------------------------------------------------------------------------------
//
void TrimOpeningBook( struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsGeneralMoves >= 0 );

   // This funtion is used to get rid of positions that haven't been reached at least a given amount of time.

   for ( BitBoard bbHashIndex = 0; bbHashIndex < gsOpeningBook.bbOpeningBookSize; bbHashIndex++ )
   {

      // Find the number of positions.
      BitBoard bbCountWhite = ( GetBookElement( bbHashIndex ) & 
                                argsGeneralMoves->bbWhiteScore ) >>
                                argsGeneralMoves->iWhiteScoreShift;
      BitBoard bbCountBlack = ( GetBookElement( bbHashIndex ) & 
                                argsGeneralMoves->bbBlackScore ) >>
                                argsGeneralMoves->iBlackScoreShift;
      BitBoard bbCountDraw  = ( GetBookElement( bbHashIndex ) & 
                                argsGeneralMoves->bbDrawScore ) >>
                                argsGeneralMoves->iDrawScoreShift;
      BitBoard bbCount = bbCountWhite + bbCountBlack + bbCountDraw;

      // Use the cut off level
      if ( bbCount < dBookCutOff )
      {
        
         SetBookElement( bbHashIndex,
                         0 );

      }

   }

}

//
//----------------------------------------------------------------------------------------------------------------------------
//
void AppendBook( struct GeneralMove * argsGeneralMoves,
                 const char * argstrBookName )
{

   // Debug the inputs.
   assert( argstrBookName >= 0 );
   assert( argsGeneralMoves >= 0 );

   BitBoard bbHash = 0;
   BitBoard bbKey  = 0;
   BitBoard bbData = 0;


   // Read the second opening book and add it in to the first.
   ifstream ifBook( argstrBookName, 
                    ifstream::binary );
   if ( ifBook.fail() )
   {
   
      cout << "Input Book.txt file failed to open." << endl;
      
   }

   // Loop over the file.
   while( !ifBook.eof() )
   {

      // Get the key from the file.
      ifBook >> bbHash;

      // Calculate the key.
      bbKey = bbHash & gsOpeningBook.bbMaskIndex;

      assert( bbKey < gsOpeningBook.bbOpeningBookSize );

      // Get the data.
      ifBook >> bbData;

      // Combine the counts here.

      // Extract the current White count.
      BitBoard bbCountWhite1 = ( GetBookElement( bbKey ) & 
                                 argsGeneralMoves->bbWhiteScore ) >>
                                 argsGeneralMoves->iWhiteScoreShift;

      BitBoard bbCountWhite2 = ( bbData & 
                                 argsGeneralMoves->bbWhiteScore ) >>
                                 argsGeneralMoves->iWhiteScoreShift;

      BitBoard bbCountWhite = bbCountWhite1 + bbCountWhite2;

      // Extract the current Black count.
      BitBoard bbCountBlack1 = ( GetBookElement( bbKey ) & 
                                 argsGeneralMoves->bbBlackScore ) >>
                                 argsGeneralMoves->iBlackScoreShift;

      BitBoard bbCountBlack2 = ( bbData & 
                                 argsGeneralMoves->bbBlackScore ) >>
                                 argsGeneralMoves->iBlackScoreShift;

      BitBoard bbCountBlack  = bbCountBlack1 + bbCountBlack2;

      // Extract the current Draw count.
      BitBoard bbCountDraw1 = ( GetBookElement( bbKey ) & 
                                argsGeneralMoves->bbDrawScore ) >>
                                argsGeneralMoves->iDrawScoreShift;

      BitBoard bbCountDraw2 = ( bbData & 
                                argsGeneralMoves->bbDrawScore ) >>
                                argsGeneralMoves->iDrawScoreShift;

      BitBoard bbCountDraw  = bbCountDraw1 + bbCountDraw2;

      // Input the data back into the hash.
      SetBookElement( bbKey,
                      0 );

      bbCountWhite = bbCountWhite << argsGeneralMoves->iWhiteScoreShift;

      bbCountBlack = bbCountBlack << argsGeneralMoves->iBlackScoreShift;

      bbCountDraw  = bbCountDraw  << argsGeneralMoves->iDrawScoreShift;

      BitBoard bbElement = GetBookElement( bbKey );

      bbElement |= bbCountWhite;

      bbElement |= bbCountBlack;

      bbElement |= bbCountDraw;

      SetBookElement( bbKey,
                      bbElement );

   }

   // Close the input books
   ifBook.close();

}


//
//----------------------------------------------------------------------------------------
//
void UpdateOpeningBook( struct Board * argsBoard,
                        struct GeneralMove * argsGeneralMoves,
                        int iGameResult )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );
   assert( iGameResult >= -1 );
   assert( iGameResult <= 2 );

   // Declare some variables.
   BitBoard bbCount = 0;
   int iBitCount;

   // Input the data to hash table.
   BitBoard bbKey = GetHash() & gsOpeningBook.bbMaskIndex;

   // Look for a collision.
   BitBoard bbHash = GetHash();
   BitBoard bbFullHash = GetBookElementHash( bbKey );
   if ( GetBookElementHash( bbKey ) == 0 )
   {

      // Store the full key for later checking of collisions.
      SetBookElementHash( bbKey,
                          bbHash );

   }
   else if ( GetHash() != GetBookElementHash( bbKey ) )
   {

      // If a collision, don't store the data.
      //cout << "Collision, Move = " << argsBoard->iMoveHistory << endl;
      return;

   }

   // Put the opening book into the hash table memory.
   switch ( iGameResult )
   {

      case dWhiteWin :
      {

         // Extract the current White count.
         bbCount = ( GetBookElement( bbKey ) & 
                     argsGeneralMoves->bbWhiteScore ) >>
                     argsGeneralMoves->iWhiteScoreShift;


         // Increment the white win count.
         bbCount++;

         // Zero out the current count.
         BitBoard bbElement = GetBookElement( bbKey );
         for ( iBitCount = 0; iBitCount < dNumberOfBitsPerScore; iBitCount++ )
         {
         
            bbElement = SetBitToZero( bbElement,
                                      iBitCount );

         }

         // Calculate the depth and enter it.
         bbCount = bbCount << argsGeneralMoves->iWhiteScoreShift;
         bbElement |= bbCount;
         SetBookElement( bbKey,
                         bbElement );

         // Extract it as a check.
         // Extract the current Black count.
         /*
         bbCount = ( GetBookElement( bbKey ) & 
                     argsGeneralMoves->bbBlackScore ) >>
                     argsGeneralMoves->iBlackScoreShift;
         */

         break;


      }
      case dBlackWin :
      {

         // Extract the current Black count.
         bbCount = ( GetBookElement( bbKey ) & 
                     argsGeneralMoves->bbBlackScore ) >>
                     argsGeneralMoves->iBlackScoreShift;

         // Increment the Black win count.
         bbCount++;

         // Zero out the current count.
         BitBoard bbElement = GetBookElement( bbKey );
         for ( iBitCount = dNumberOfBitsPerScore; iBitCount < 2 * dNumberOfBitsPerScore; iBitCount++ )
         {
         
            bbElement = SetBitToZero( bbElement,
                                      iBitCount );

         }

         // Calculate the depth and enter it.
         bbCount = bbCount << argsGeneralMoves->iBlackScoreShift;
         bbElement |= bbCount;
         SetBookElement( bbKey,
                         bbElement );

         break;

      }
      case dDraw :
      {

         // Extract the current Draw count.
         bbCount = ( GetBookElement( bbKey ) & 
                     argsGeneralMoves->bbDrawScore ) >>
                     argsGeneralMoves->iDrawScoreShift;

         // Increment the Draw win count.
         bbCount++;

         // Zero out the current count.
         BitBoard bbElement = GetBookElement( bbKey );
         for ( iBitCount = 2 * dNumberOfBitsPerScore; iBitCount < 3 * dNumberOfBitsPerScore; iBitCount++ )
         {
         
            bbElement = SetBitToZero( bbElement,
                                      iBitCount );

         }

         // Calculate the depth and enter it.
         bbCount = bbCount << argsGeneralMoves->iDrawScoreShift;
         bbElement |= bbCount;
         SetBookElement( bbKey,
                         bbElement );

         break;

      }
      case dUnknown :
      {

         // In this case do nothing.
         break;

      }

   }

}

//
//---------------------------------------------------------------------------------------------------------------
//
void ExtractOpeningBookStats( BitBoard & argbbWhiteWins,
                              BitBoard & argbbBlackWins,
                              BitBoard & argbbDraws,
                              struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsGeneralMoves >= 0 );

   // Calculate the key to the hash table.
   BitBoard bbKey = GetHash() & gsOpeningBook.bbMaskIndex;
   BitBoard bbHash = GetBookElementHash( bbKey );
   BitBoard bbElement = GetBookElement( bbKey );
   BitBoard bbHashFromTable = GetHash();

   assert( bbKey <= gsOpeningBook.bbOpeningBookSize );


   // Look for a collision.
   if ( ( GetHash() != GetBookElementHash( bbKey ) ) ||
        ( GetBookElementHash( bbKey ) == 0 ) )
   {

      //cout << "Collision" << endl;
      argbbWhiteWins = 0;
      argbbBlackWins = 0;
      argbbDraws     = 0;

   }
   else
   {

      // Extract the current White count.
      argbbWhiteWins = ( GetBookElement( bbKey ) & 
                         argsGeneralMoves->bbWhiteScore ) >>
                         argsGeneralMoves->iWhiteScoreShift;

      // Extract the current Black count.
      argbbBlackWins = ( GetBookElement( bbKey ) & 
                         argsGeneralMoves->bbBlackScore ) >>
                         argsGeneralMoves->iBlackScoreShift;

      // Extract the current Draw count.
      argbbDraws = ( GetBookElement( bbKey ) & 
                     argsGeneralMoves->bbDrawScore ) >>
                     argsGeneralMoves->iDrawScoreShift;

   }

}

//
//------------------------------------------------------------------------------------------
//
void StartCheckBook( struct Board * argsBoard,
                     struct GeneralMove * argsGeneralMoves )
{

   // Set the counters to zero.
   giTotalCount = 0;
   giMarginalCount = 0;
   giCutCount = 0;

   // Open the file for output of the checked book.
   //ifstream ifBook( argstrBookName, ifstream::binary );
   gofCheckedBook.open ( "CheckedBook.txt", 
                        ofstream::binary );
   if ( gofCheckedBook.fail() )
   {
   
      cout << "CheckedBook.txt failed to open." << endl;
      system( "Pause" );
      return;
      
   }

   CheckBook( argsBoard,
              argsGeneralMoves );   

   // Close the checked book.
   gofCheckedBook.close();

}

//
//-------------------------------------------------------------------------------------------
//
/*
void CheckBook( struct Board * argsBoard,
                struct GeneralMove * argsGeneralMoves,
                ofstream * argofCheckedBook )
*/
void CheckBook( struct Board * argsBoard,
                struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );
   assert( CheckBoard( argsBoard ) );

   // This function is called recursively to check the validity of the book.
   int viPopularMoves[ dNumberOfMoves ];  
   int iNumberOfMoves;
   int iNumberOfPopularMoves;
   Move * vsMoveList;
   vsMoveList = ( Move * ) malloc( dNumberOfMoves * sizeof( Move ) );
   int iScore = 0;
   int iAlpha = dAlpha;
   int iBeta  = dBeta;
   BitBoard bbKey = 0;
   int iMoveCount;

   // Calculate the moves for this position.
   CalculateMoves( vsMoveList,
                   argsBoard,
                   argsGeneralMoves );

   // Put the number of moves into a local variable.
   iNumberOfMoves = argsBoard->siNumberOfMoves;

   // Get the popular moves for this board.
   GetPopularMoves( argsBoard,
                    argsGeneralMoves,
                    viPopularMoves,
                    & iNumberOfPopularMoves,
                    vsMoveList,
                    iNumberOfMoves );


   // If the end of the book, return.
   if ( ( iNumberOfPopularMoves == 0 ) ||  // skip if the end of the book
        ( LookForDraw( argsBoard, argsGeneralMoves ) ) ||  // skip if we have seen this position before
        ( ( argsGeneralMoves->bbVerified & GetBookElement( GetKey() ) ) ) ) // skip if we have already verified this.
   {

      free( vsMoveList );
      return;

   }

   // Update the counters and publish.
   giTotalCount++;
   giMarginalCount++;
   if ( giMarginalCount > 9 )
   {

      cout << "Count = " << giTotalCount << " Cut = " << giCutCount << endl;
      giMarginalCount = 0;

   }


///*
   // Perform a search.
   iScore = StartSearch( argsBoard, 
                         argsGeneralMoves,
                         iAlpha,
                         iBeta ); 
//*/

   // If the score is good, continue and write to checked book file.  If not, return.
   if ( iScore < 0 )
   {

      iScore = - iScore;

   }
   if ( iScore < dOpeningBookScoreCutOff )
   {

      // This is a good position and we will continue.
      gofCheckedBook << GetBookElementHash( GetKey() ) << " " << GetBookElement( GetKey() ) << " " << endl;

   }
   else
   {

      // If we fail, to a research to a deeper depth.
      SetSearchDepth(             2 * dOpeningBookVerificationSearchDepth ); // dInfiniteDepth dOpeningBookVerificationSearchDepth

      SetSearchTimeInMiliSeconds( dTenMinutes ); // dInfiniteTime dOneSecond dTwentySeconds dOneMinute dTenMinutes

      // Perform a search.
      iScore = StartSearch( argsBoard, 
                            argsGeneralMoves,
                            iAlpha,
                            iBeta ); 

      // Set the initial parameters for controling the game
      SetSearchDepth(             dOpeningBookVerificationSearchDepth ); // dInfiniteDepth dOpeningBookVerificationSearchDepth

      SetSearchTimeInMiliSeconds( dInfiniteTime ); // dInfiniteTime dOneSecond dTwentySeconds dOneMinute dTenMinutes

      // If the score is good, continue and write to checked book file.  If not, return.
      if ( iScore < 0 )
      {
         iScore = - iScore;
      }
      if ( iScore < dOpeningBookScoreCutOff )
      {

         // This is a good position and we will continue.
         gofCheckedBook << GetBookElementHash( GetKey() ) << " " << GetBookElement( GetKey() ) << " " << endl;

      }
      else
      {

         // The score didn't pass the cutoff.  Baill out.
         giCutCount++;
         PrintBoard( argsBoard->mBoard );
         PrintFEN( argsBoard,
                   argsGeneralMoves );
         int iEvalScore = EvaluateBoard( argsBoard,
                                         argsGeneralMoves ); 
         PrintPrincipalVariation( argsBoard,
                                  argsGeneralMoves );
         //cout << "iScore = " << iScore << " iEval = " << iEvalScore << endl << endl;
         //system( "Pause" );
         free( vsMoveList );
         return;

      }

   }


   // If we made it here we are verified.
   // Mark the position as having been checked.
   BitBoard bbElement = GetBookElement( GetKey() );
   BitBoard bbHash = GetBookElementHash( GetKey() );
      
///*
   // See if the bit is already set to zero.
   if ( !( argsGeneralMoves->bbVerified & bbElement ) )
   {

      bbElement = SetBitToOne(  bbElement, 60 );

   }
//*/
///*
   // Put the element back in the book.
   SetBookElement( GetKey(),
                   bbElement );
//*/

   // Loop over the moves
   for ( iMoveCount = 0; iMoveCount < iNumberOfPopularMoves; iMoveCount++ )
   {

      if ( argsBoard->iMoveHistory == -1 )
      {

         if ( iMoveCount == 0 )
         {

            giTotalCount = 0;
            giMarginalCount = 0;

         }

         cout << "Move number " << iMoveCount << " of " << iNumberOfPopularMoves << endl;

      }
 
     // Make the move.
      MakeMove( vsMoveList,
                argsBoard,
                argsGeneralMoves,
                viPopularMoves[ iMoveCount ] );

      CheckBook( argsBoard,
                 argsGeneralMoves );

      // Undo the move.
      UndoMove( argsBoard,
                argsGeneralMoves );

   }

   free( vsMoveList );

   // put in a final loop here to take out all unverified elemensts.

}

//
//----------------------------------------------------------------------------------------------------------
//
void GetPopularMoves( struct Board * argsBoard,
                      struct GeneralMove * argsGeneralMoves,
                      int * viPopularMoves,
                      int * iNumberOfPopularMoves,
                      struct Move * vsMoveList,
                      int iNumberOfMoves )

{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Print the statistics from the open book for the moves from a given position.
   int viSortOrder[ dNumberOfMoves ];
   int viPopularity[ dNumberOfMoves ];
   BitBoard bbWhiteWins = 0;
   BitBoard bbBlackWins = 0;
   BitBoard bbDraws = 0;
   double dPercentWhiteWins = 0;
   double dPercentBlackWins = 0;
   double dPercentDraws = 0;
   int iMoveCount = 0;

   // Reset the count of populare moves.
   * iNumberOfPopularMoves = 0;

   // Loop over the moves
   for ( iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++ )
   {

      // Make the move.
      MakeMove( vsMoveList,
                argsBoard,
                argsGeneralMoves,
                iMoveCount );

      // Get the statistics.
      ExtractOpeningBookStats( bbWhiteWins,
                               bbBlackWins,
                               bbDraws,
                               argsGeneralMoves );

      // Undo the move.
      UndoMove( argsBoard,
                argsGeneralMoves );

      // Collect the popularity.
      viPopularity[ iMoveCount ] = (int)(bbWhiteWins + bbBlackWins + bbDraws);
      viSortOrder[ iMoveCount ] = iMoveCount;
          
      // Calculate the number of populare moves.
      if ( viPopularity[ iMoveCount ] > 0 )
      {

         (*iNumberOfPopularMoves)++;

      }

   }

   // Sort on the popularity
   int iSortFlag = 1;
   while( iSortFlag )
   {

      // Set the default to bail.
      iSortFlag = 0;

      // Use a cocktail sort and to from top to bottom
      for ( int iMoveIndex = 0; iMoveIndex < iNumberOfMoves - 1; iMoveIndex++ )
      {

         if ( viPopularity[ iMoveIndex + 1 ] > viPopularity[ iMoveIndex ] )
         {

            int iDummyScore                = viPopularity[ iMoveIndex ];
            viPopularity[ iMoveIndex ]     = viPopularity[ iMoveIndex + 1 ];
            viPopularity[ iMoveIndex + 1 ] = iDummyScore;

            int iDummyPosition            = viSortOrder[ iMoveIndex ];
            viSortOrder[ iMoveIndex ]     = viSortOrder[ iMoveIndex + 1 ];
            viSortOrder[ iMoveIndex + 1 ] = iDummyPosition;

            iSortFlag = 1;

         }

      }
           
   }

   // Extract the populare moves
   for ( int iMoveIndex = 0; iMoveIndex < * iNumberOfPopularMoves; iMoveIndex++ )
   {

      viPopularMoves[ iMoveIndex ] = viSortOrder[ iMoveIndex ];

   }


}

//
//------------------------------------------------------------------------------------------------------------------------
//
int GetNumberOfPositionsInOpeningBook()
{
   return (int)(gsOpeningBook.bbNumberOfPositionsInBook);
}

//
//------------------------------------------------------------------------------------------------------------------------
//
int GetNumberOfPositionsVerified()
{
   return (int)(gsOpeningBook.bbNumberOfPositionsVerified);
}

//
//------------------------------------------------------------------------------------------------------------------------
//

void InitializeBookDebug()
{

   // Open some debugging files.
   gofDebugBook.open( "BookInterfaceLog.txt", ios::out | ios::app );
   gofDebugBook << endl;
   gofDebugBook << "Book log started." << endl << endl;


}

//
//---------------------------------------------------------------------------------------------------------
//
void CloseBookDebug()
{

   // Close down the communications.
   gofDebugBook << "Closing file." << endl;
   gofDebugBook << endl;
   gofDebugBook.close();
  
}

