// Copyright 2025 by Jay Lindgren. All Rights Reserved.
//
// Aspiration Search Performance Testing
// This file contains tests to analyze how aspiration search affects search performance
//

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <iomanip>
#include <time.h>
#include "Functions.h"
#include "Structures.h"
#include "Definitions.h"

using namespace std;

//
//
//---------------------------------------------------------------------
//
//
void TestAspirationPerformance( struct Board       *argsBoard,
                                struct GeneralMove *argsGeneralMoves )
{
   cout << "==================================================" << endl;
   cout << "Aspiration Search Performance Analysis" << endl;
   cout << "==================================================" << endl;
   cout << endl;

   // Open CSV file for results
   ofstream ofsCsvFile( "aspiration_results.csv" );

   if ( !ofsCsvFile.is_open() )
   {
      cout << "ERROR: Could not create aspiration_results.csv" << endl;
      return;
   }

   // Write CSV header
   ofsCsvFile << "Position,AspirationEnabled,WindowWidth,SearchDepth,NodesEvaluated,TimeMS,NodesPerSecond" << endl;

   // Define search depth
   int siTestDepth = 8;

   // Test positions
   struct TestPosition
   {
      const char *mName;
      const char *mFEN;
   };

   struct TestPosition sTestPositions[] =
       {
           { "StartingPosition", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" },
           { "MiddleGame1", "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4" },
           { "MiddleGame2", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1" },
           { "MiddleGame3", "2r3k1/1q1nbppp/r3p3/3pP3/pPpP4/P1Q2N2/2R2PPP/1R2B1K1 b - - 0 1" } };

   int siNumPositions = sizeof( sTestPositions ) / sizeof( sTestPositions[ 0 ] );

   // Test configurations
   // 1. Aspiration OFF
   // 2. Aspiration ON with various window widths

   struct Config
   {
      int         mEnabled;
      int         mWidth;
      const char *mName;
   };

   struct Config sConfigs[] =
       {
           { dNo, 0, "Disabled" },
           { dYes, 10, "Enabled_10" },
           { dYes, 20, "Enabled_20" },
           { dYes, 30, "Enabled_30" }, // Default
           { dYes, 50, "Enabled_50" },
           { dYes, 100, "Enabled_100" } };

   int siNumConfigs = sizeof( sConfigs ) / sizeof( sConfigs[ 0 ] );

   // Iterate through positions
   for ( int iPositionIndex = 0; iPositionIndex < siNumPositions; iPositionIndex++ )
   {
      cout << "---------------------------------------------------" << endl;
      cout << "Position: " << sTestPositions[ iPositionIndex ].mName << endl;
      cout << "---------------------------------------------------" << endl;

      for ( int iConfigIndex = 0; iConfigIndex < siNumConfigs; iConfigIndex++ )
      {
         cout << "  Config: " << sConfigs[ iConfigIndex ].mName << endl;

         // Set aspiration settings
         SetAspirationSearch( sConfigs[ iConfigIndex ].mEnabled );

         if ( sConfigs[ iConfigIndex ].mEnabled == dYes )
         {
            SetAspirationWindowWidth( sConfigs[ iConfigIndex ].mWidth );
         }

         // Set up the position
         ofstream ofsFenFile( "FEN.txt" );

         if ( ofsFenFile.is_open() )
         {
            ofsFenFile << sTestPositions[ iPositionIndex ].mFEN << endl;
            ofsFenFile.close();
         }

         ReadFEN( "FEN.txt", argsBoard, argsGeneralMoves, 0 );

         // Reset node counter
         SetNumberOfNodes( 0 );

         // Set search parameters
         SetSearchDepth( siTestDepth );
         SetStopGo( dGo );

         // Clear hashtable to ensure fair comparison
         ClearHashTable();

         // Record start time
         clock_t clockStartTime = clock();

         cout << "    Starting search for " << sConfigs[ iConfigIndex ].mName << "..." << endl;

         // Perform the search
         int iScore = StartSearch( argsBoard, argsGeneralMoves, dAlpha, dBeta );
         (void)iScore; // Prevent unused variable warning

         cout << "    Search finished." << endl;

         // CRITICAL: Use GetNumberOfNodesSearched() which is set by StartSearch
         BitBoard bbNodesEvaluated = GetNumberOfNodesSearched();

         // Record end time
         clock_t clockEndTime = clock();

         // Calculate metrics
         int iTimeMS = (int)( ( (double)( clockEndTime - clockStartTime ) / CLOCKS_PER_SEC ) * 1000.0 );

         if ( iTimeMS == 0 )
         {
            iTimeMS = 1; // Avoid division by zero
         }

         long long siNodesPerSecond = (long long)( (double)bbNodesEvaluated / (double)iTimeMS * 1000.0 );

         // Display results
         cout << "    Depth " << siTestDepth << ": " << bbNodesEvaluated << " nodes in "
              << iTimeMS << " ms (" << siNodesPerSecond << " nps)" << endl;

         // Write to CSV
         ofsCsvFile << sTestPositions[ iPositionIndex ].mName << ","
                    << ( sConfigs[ iConfigIndex ].mEnabled == dYes ? "Enabled" : "Disabled" ) << ","
                    << ( sConfigs[ iConfigIndex ].mEnabled == dYes ? sConfigs[ iConfigIndex ].mWidth : 0 ) << ","
                    << siTestDepth << ","
                    << bbNodesEvaluated << ","
                    << iTimeMS << ","
                    << siNodesPerSecond << endl;
      }

      cout << endl;
   }

   ofsCsvFile.close();

   cout << "==================================================" << endl;
   cout << "Test Complete! Results saved to aspiration_results.csv" << endl;
   cout << "==================================================" << endl;
   cout << endl;

   // Restore default settings
   SetAspirationSearch( dYes );
   SetAspirationWindowWidth( 30 );
}
