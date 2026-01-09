#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>

// If in deep mode, include the appropriate files

#include "Definitions.h"
#include "Functions.h"
#include "Structures.h"
#include "Thread.h"

using namespace std;

struct ArasanTestDef
{
   const char *name;
   const char *filename;
   int         timeLimitSec;
   int         arasanScore;
   int         arasanTotal;
};

// Based on https://www.arasanchess.org/tests.shtml
ArasanTestDef g_ArasanTests[] = {
    { "arasan2024", "arasan2024.epd", 10, 151, 200 },
    { "ecmgcp", "ecmgcp.epd", 10, 168, 183 },
    { "iq4", "iq4.epd", 10, 161, 183 },
    { "Win At Chess (new)", "wacnew.epd", 10, 294, 300 },
    { "Pet", "pet.epd", 60, 43, 48 },
    { "EET", "eet.epd", 60, 95, 100 },
    { "BT2630", "bt2630.epd", 900, 29, 30 },
    { "Lapuce II", "lapuce2.epd", 600, 31, 35 } };

//
//
//---------------------------------------------------------------------
//
//
void TestSuite( struct Board       *argsBoard,
                struct GeneralMove *argsGeneralMoves )
{

   // This will run Violet through a series of test positions and compare her score
   // against the conventional wisdom.

   // Debug the input.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   const char *baseDir    = "C:\\VioletTools\\ArasanTests\\";
   const char *reportFile = "ArasanComparisonReport.txt";

   ofstream report( reportFile );
   if ( !report )
   {
      cout << "Failed to create report file: " << reportFile << endl;
      return;
   }

   report << "Violet vs Arasan Test Suite Comparison" << endl;
   report << "======================================" << endl
          << endl;
   report << left << setw( 20 ) << "Test Name"
          << setw( 12 ) << "Time Limit"
          << setw( 15 ) << "Violet"
          << setw( 15 ) << "Arasan"
          << endl;
   report << string( 62, '-' ) << endl;

   cout << "Starting Arasan Test Suite..." << endl;

   for ( const auto &test : g_ArasanTests )
   {
      string fullPath = string( baseDir ) + test.filename;

      SetSearchTimeInMiliSeconds( test.timeLimitSec * 1000 );

      int matches = 0;
      int total   = 0;

      cout << "Running test: " << test.name << " (" << test.filename << ")" << endl;

      RunTestEPDFile( fullPath.c_str(), argsBoard, argsGeneralMoves, &matches, &total );

      // Validate total count match
      if ( total != test.arasanTotal && total > 0 )
      {
         cout << "WARNING: Position count mismatch for " << test.name
              << ". Expected " << test.arasanTotal << ", found " << total << endl;
         report << "NOTE: " << test.name << " position count mismatch (" << total << " vs " << test.arasanTotal << ")" << endl;
      }

      if ( total == 0 )
      {
         // Error occurred (file missing etc)
         report << left << setw( 20 ) << test.name
                << setw( 12 ) << ( to_string( test.timeLimitSec ) + "s" )
                << setw( 15 ) << "ERROR"
                << setw( 15 ) << ( to_string( test.arasanScore ) + "/" + to_string( test.arasanTotal ) )
                << endl;
      }
      else
      {
         string violetScore = to_string( matches ) + "/" + to_string( total );
         string arasanScore = to_string( test.arasanScore ) + "/" + to_string( test.arasanTotal );

         report << left << setw( 20 ) << test.name
                << setw( 12 ) << ( to_string( test.timeLimitSec ) + "s" )
                << setw( 15 ) << violetScore
                << setw( 15 ) << arasanScore
                << endl;
      }

      report.flush();
   }

   report << endl
          << "Testing Complete." << endl;
   report.close();

   cout << "Testing Complete. Report saved to " << reportFile << endl;
   // Prevent exit or pause if automated, but usually useful to see
   // system("pause");
}

//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void RunTestEPDFile( const char         *argstrFileName,
                     struct Board       *argsBoard,
                     struct GeneralMove *argsGeneralMoves,
                     int                *outMatches,
                     int                *outTotal )
{

   // This routine will run Violet through a file of positions save the best move and compare it conventional wisdom
   char  strLine[ 640 ];
   char  strOutput[ 256 ];
   char *strPointer;
   char  strBestMove[ 64 ];
   char  strMove[ 10 ];
   int   iInputFlag          = 1;
   int   iPositionCount      = 0;
   int   iMatchCount         = 0;
   int   iMissMatchCount     = 0;
   int   iScore              = 0;
   int   iNumberOfCharacters = 0;

   // Initialize outputs
   if ( outMatches )
      *outMatches = 0;
   if ( outTotal )
      *outTotal = 0;

   // Open the input file.
   ifstream ifPosition( argstrFileName );
   if ( ifPosition.fail() )
   {
      cout << "Input file failed to open: " << argstrFileName << endl;
      return;
   }

   // Make sure the file opened.
   // assert( ifPosition );

   // Open the output file.
   strcpy( strOutput, argstrFileName );
   strPointer = strstr( strOutput, "." );
   if ( strPointer )
      *strPointer = 0;
   strcat( strOutput, "Output.txt" );
   ofstream ofOutput( strOutput );
   if ( ofOutput.fail() )
   {
      cout << "Output file failed to open: " << strOutput << endl;
      // return; // Optional: continue without output file?
   }

   // Insert some header info.
   if ( ofOutput )
   {
      ofOutput << "From file " << argstrFileName << endl
               << endl;

      // Record the time given for the search.
      ofOutput << "Time for search in seconds = " << GetSearchTimeInMiliSeconds() / 1000 << endl
               << endl;
   }

   // Loop over the input file.
   while ( ifPosition.good() )
   {
      // Get a line off from the file
      ifPosition.getline( strLine, 640 );

      // Check for empty line or end of file
      if ( strlen( strLine ) < 5 )
         continue;

      // Increment the counter.
      iPositionCount++;

      // Update the output file.
      if ( ofOutput )
      {
         ofOutput << endl
                  << endl
                  << "Position = " << iPositionCount << endl;
         ofOutput << "FEN = " << strLine << endl;
      }
      cout << "Full Line = " << strLine << endl;

      // Extract the best move
      strPointer = strstr( strLine, "bm" );
      if ( !strPointer )
      {
         if ( ofOutput )
            ofOutput << "Error: No best move found in EPD line." << endl;
         continue;
      }

      // Cut the line to the best move
      *strPointer = 0;

      strPointer += 3; // Skip "bm "
      while ( *strPointer == ' ' )
         strPointer++;

      strcpy( strBestMove, strPointer );
      strPointer = strstr( strBestMove, ";" );
      if ( strPointer )
         *strPointer = 0;

      if ( ofOutput )
         ofOutput << "Best move             = " << strBestMove << endl;

      // Set up the fen position
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

      // Perform a search
      // Search parameters are set in Game Control.
      // Reset the plys.
      // Reset the clocks
      // Reset Alpha and beta
      argsBoard->iNumberOfPlys = -1;
      argsBoard->cStart        = clock();
      int iAlpha               = dAlpha;
      int iBeta                = dBeta;

      int iMatchFound = 0;
      SetStopGo( dGo );
      SetTempusParameters();

      // Initialize threads if not already done (default to 1 if not set via UCI)
      if ( gThreads.empty() )
      {
         InitializeThreads( 1 );
      }

      // LAZY SMP: Initialize shared PV structure
      InitializeSharedPV();

      // Launch helper threads once before the ID loop
      if ( !gThreads.empty() )
      {
         for ( size_t i = 1; i < gThreads.size(); ++i )
         {
            gThreads[ i ]->StartSearch( argsBoard, argsGeneralMoves, iAlpha, iBeta,
                                        GetSearchDepth() );
         }
      }

      // Loop over the depth.
      for ( int iDepth = 0; iDepth < GetSearchDepth(); iDepth++ )
      {

         // Set whether or not the best move has been searched at this ply.
         int iBestMoveSearched = 0;
         int iBestMove         = argsBoard->iBestMove;

         // Set the depth of the board.
         argsBoard->iMaxPlys        = iDepth;
         argsBoard->iMaxPlysReached = -1;

         // Call the first search routine.
         iScore = FirstSearch( argsBoard,
                               argsGeneralMoves,
                               iAlpha,
                               iBeta,
                               &iBestMove,
                               &iBestMoveSearched,
                               gMainThreadData );

         // LAZY SMP: Update shared PV with main thread's results
         if ( argsBoard->vmPrincipalVariation[ 0 ][ 0 ].iFromSquare >= 0 )
         {
            UpdateSharedPV( argsBoard->vmPrincipalVariation[ 0 ], iScore, iDepth );
         }

         // If we have timed out.  Move on.
         if ( GetStopGo() == dStop )
         {
            iDepth = 999999999;
         }
         else
         {

            // Check to see if the moves match.
            // Print the best move
            iNumberOfCharacters = PrintMove( argsBoard,
                                             argsGeneralMoves,
                                             &argsBoard->vmPrincipalVariation[ 0 ][ 0 ],
                                             strMove );

            // Trim down the string
            strPointer  = strMove + iNumberOfCharacters;
            *strPointer = 0;

            cout << "Depth = " << iDepth + 1 << " Violet = " << strMove << " Score = " << iScore << endl;
            if ( ofOutput )
               ofOutput << "Depth = " << iDepth + 1 << " Violet = " << strMove << " Score = " << iScore << endl;

            // See if the moves match.
            // Note, the best move string may contain several moves.  This is a way to see if our move is
            // in the longer string.
            if ( strstr( strBestMove, strMove ) )
            {
               // There is a match
               iMatchCount++;
               iMatchFound = 1;
               iDepth      = 999999999;
            }
         }
      }

      // Stop helper threads
      SetStopGo( dStop );
      std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );

      // If we did not find a match, then update the mismatch.
      if ( iMatchFound == 0 )
      {
         iMissMatchCount++;
         if ( ofOutput )
            ofOutput << "MISMATCH! Found: " << strMove << ", Expected: " << strBestMove << endl;
      }

      // Write out the cumulative statistics
      if ( ofOutput )
      {
         ofOutput << "Matching = " << iMatchCount << " Mismatch = " << iMissMatchCount << " Total = " << iPositionCount << endl
                  << endl;
      }
      cout << endl
           << "Matching = " << iMatchCount << " Mismatch = " << iMissMatchCount << " Total = " << iPositionCount << endl
           << endl;
   }

   // Update return values
   if ( outMatches )
      *outMatches = iMatchCount;
   if ( outTotal )
      *outTotal = iPositionCount;

   // Close the files.
   ifPosition.close();
   if ( ofOutput )
      ofOutput.close();
}
