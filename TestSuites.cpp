
#include <iostream>
#include <fstream>


// If in deep mode, include the appropriate files
#if defined( dDeepMode )
   #include <omp.h>
#endif

#include "Definitions.h"
#include "Functions.h"
#include "Structures.h"

using namespace std;


//
//---------------------------------------------------------------------------------------
//
void TestSuite( struct Board * argsBoard,
                struct GeneralMove * argsGeneralMoves )
{

   // This will run Violet through a series of test positions and compare her score 
   // against the conventional wisdom.

   // Debug the input.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // First up, Win At Chess (WAC)
/*
   SetSearchTimeInMiliSeconds( dTwentySeconds ); // dInfiniteTime dOneSecond dTwentySeconds dOneMinute dTenMinutes
   RunTestEPDFile( "ecmgcp.epd",
                   argsBoard,
                   argsGeneralMoves );
//*/
///*

   SetSearchTimeInMiliSeconds( dTwentySeconds ); // dInfiniteTime dOneSecond dTwentySeconds dOneMinute dTenMinutes
   RunTestEPDFile( "wacnew.epd",
                   argsBoard,
                   argsGeneralMoves );
//*/
/*
   SetSearchTimeInMiliSeconds( dTenMinutes ); // dInfiniteTime dOneSecond dTwentySeconds dOneMinute dTenMinutes
   RunTestEPDFile( "bt2630.epd",
                   argsBoard,
                   argsGeneralMoves );
//*/
/*
   SetSearchTimeInMiliSeconds( dTenMinutes ); // dInfiniteTime dOneSecond dTwentySeconds dOneMinute dTenMinutes
   RunTestEPDFile( "lapuce2.epd",
                   argsBoard,
                   argsGeneralMoves );
//*/
/*
   SetSearchTimeInMiliSeconds( dTwentySeconds ); // dInfiniteTime dOneSecond dTwentySeconds dOneMinute dTenMinutes
   RunTestEPDFile( "pet.epd",
                   argsBoard,
                   argsGeneralMoves );
//*/
/*
   SetSearchTimeInMiliSeconds( dTwentySeconds ); // dInfiniteTime dOneSecond dTwentySeconds dOneMinute dTenMinutes
   RunTestEPDFile( "Arasan16.epd",
                   argsBoard,
                   argsGeneralMoves );
//*/
/*
   SetSearchTimeInMiliSeconds( dTwentySeconds ); // dInfiniteTime dOneSecond dTwentySeconds dOneMinute dTenMinutes
   RunTestEPDFile( "iq4.epd",
                   argsBoard,
                   argsGeneralMoves );
//*/
/*
   SetSearchTimeInMiliSeconds( dTwentySeconds ); // dInfiniteTime dOneSecond dTwentySeconds dOneMinute dTenMinutes
   RunTestEPDFile( "eet.epd",
                   argsBoard,
                   argsGeneralMoves );
//*/


}

//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void RunTestEPDFile( const char * argstrFileName,
                     struct Board * argsBoard,
                     struct GeneralMove * argsGeneralMoves )
{

   // This routine will run Violet through a file of positions save the best move and compare it conventional wisdom
   char strLine[ 640 ];
   char strOutput[ 64 ];
   char * strPointer;   
   char strBestMove[ 64 ];
   char strMove[ 10 ];
   int iInputFlag = 1;
   int iPositionCount = 0;
   int iMatchCount = 0;
   int iMissMatchCount = 0;
   int iScore = 0;
   int iNumberOfCharacters = 0;

   // Open the input file.
   ifstream ifPosition( argstrFileName );
   if ( ifPosition.fail() )
   {
      
      cout << "Input file failed to open." << endl;
      
   }

   // Make sure the file opened.
   assert( ifPosition );

   // Open the output file.
   strcpy( strOutput, argstrFileName );
   strPointer = strstr( strOutput, "." );
   * strPointer = 0;
   strcat( strOutput, "Output.txt" );   
   ofstream ofOutput( strOutput );
   if ( ofOutput.fail() )
   {
      
      cout << "Ouotput file failed to open." << endl;
      
   }

   // Make sure the file opened.
   assert( ofOutput );

   // Insert some header info.
   ofOutput << "From file " << argstrFileName << endl << endl;

   // Record the time given for the search.
   ofOutput << "Time for search in seconds = " << GetSearchTimeInMiliSeconds() / 1000 << endl << endl;

   // Loop over the input file.
   while( ! ifPosition.eof( ) )
   //while( iInputFlag )
   {

      // Increment the counter.
      iPositionCount++;

      // Get a line off from the file
      ifPosition.getline( strLine, 640 );

      // Update the output file.
      ofOutput << endl << endl << "Position = " << iPositionCount << endl;
      ofOutput << "FEN = " << strLine << endl;
      cout << "Full Line = " << strLine << endl;

      // Extract the best move
      strPointer = strchr( strLine, 'm' );
      strPointer++;
      strPointer++;
      strcpy( strBestMove, strPointer );
      strPointer = strstr( strBestMove, ";" );
      * strPointer = 0;
      ofOutput << "Best move             = " << strBestMove << endl;   

      // Cut the line to the best move
      strPointer = strchr( strLine, 'bm' );
      strPointer--; // back up to the b.
      * strPointer = 0;

      // Let the folks at home know what is going on
      //cout << strLine << endl;

      //Set up the fen position
      ReadFEN( strLine,
               argsBoard,
               argsGeneralMoves,
               2 );

      // Let the folks at home see the board.
      PrintBoard( argsBoard->mBoard );

      // Set the side to move.
      if ( argsBoard->siColorToMove == dWhite )
      {

         SetComputerColor( dComputerWhite );

      }
      else if ( argsBoard->siColorToMove == dBlack )
      {

         SetComputerColor( dComputerBlack );

      }
      else
      {

         assert( argsBoard->siColorToMove > dWhite );
         assert( argsBoard->siColorToMove < dBlack );

      }

      // Perform a search
      // Search parameters are set in Game Control.
      // Reset the plys.
      // Reset the clocks
      // Reset Alpha and beta
      argsBoard->iNumberOfPlys = -1;
      argsBoard->cStart = clock();
      int iAlpha = dAlpha;
      int iBeta  = dBeta;

      int iMatchFound = 0;
      SetStopGo( dGo );
      SetTempusParameters();

      // Loop over the depth.
      for ( int iDepth = 0; iDepth < GetSearchDepth(); iDepth++ )
      {

         // Set whether or not the best move has been searched at this ply.
         int iBestMoveSearched = 0;
         int iBestMove = argsBoard->iBestMove;

         // Set the depth of the board.
         argsBoard->iMaxPlys = iDepth;
         argsBoard->iMaxPlysReached = -1;

         // Call the first search routine.
         iScore = FirstSearch( argsBoard, 
                               argsGeneralMoves,
                               iAlpha,
                               iBeta,                       
                               & iBestMove,
                               & iBestMoveSearched ); 

         // If we have timed out.  Move on.
         if ( GetStopGo() == dStop )
         {

            iDepth = 999999999;

         }
         else
         {

            // Check to see if the moves match.
            // Print the best move
            iNumberOfCharacters =  PrintMove( argsBoard,
                                              argsGeneralMoves,
                                              &argsBoard->vmPrincipalVariation[ 0 ][ 0 ],
                                              strMove );

            // Trim down the string
            strPointer = strMove + iNumberOfCharacters;
            * strPointer = 0;

            cout << "Depth = " << iDepth + 1 << " Violet = " << strMove << " Score = " << iScore << endl;
            ofOutput << "Depth = " << iDepth + 1 << " Violet = " << strMove << " Score = " << iScore << endl;

            // See if the moves match.
            // Note, the best move string may contain several moves.  This is a way to see if our move is
            // in the longer string.
            if ( strstr( strBestMove, strMove ) )     
            {

               // There is a match
               iMatchCount++;
               iMatchFound = 1;        
               iDepth = 999999999;

            }
  
         }

      }

      // If we did not find a match, then update the mismatch.
      if( iMatchFound == 0 ) 
      {
      
         iMissMatchCount++;

      }

      // Write out the cumulative statistics
      ofOutput << "Matching = " << iMatchCount << " Mismatch = " << iMissMatchCount << " Total = " << iPositionCount << endl << endl;
      cout << endl << "Matching = " << iMatchCount << " Mismatch = " << iMissMatchCount << " Total = " << iPositionCount << endl << endl;

   }

   // Close the files.
   ifPosition.close();
   ofOutput.close();

}
