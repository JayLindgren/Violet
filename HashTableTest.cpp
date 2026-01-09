// Copyright 2006 by Jay Lindgren. All Rights Reserved.
//
// HashTable Performance Testing
// This file contains tests to analyze how hashtable size affects search
// performance
//

#define _CRT_SECURE_NO_WARNINGS
#include "Definitions.h"
#include "Functions.h"
#include "Structures.h"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <time.h>

using namespace std;

//////////////////////////////////////////////////////////////////////////////
// Test hashtable performance across different sizes
//////////////////////////////////////////////////////////////////////////////
void TestHashTablePerformance( struct Board       *argsBoard,
                               struct GeneralMove *argsGeneralMoves )
{
   cout << "==================================================" << endl;
   cout << "Hashtable Performance Analysis" << endl;
   cout << "==================================================" << endl;
   cout << endl;

   // Open CSV file for results
   ofstream csvFile( "C:\\VioletTools\\hashtable_results.csv" );
   if ( !csvFile.is_open() )
   {
      cout << "ERROR: Could not create hashtable_results.csv" << endl;
      return;
   }

   // Write CSV header
   csvFile << "HashBits,HashSizeMB,Position,SearchDepth,NodesEvaluated,TimeMS,"
              "NodesPerSecond"
           << endl;

   // Define hashtable sizes to test (bits)
   int hashSizes[]  = { 10, 12, 14, 16, 18, 20, 22, 24 };
   int numHashSizes = sizeof( hashSizes ) / sizeof( hashSizes[ 0 ] );

   // Define search depths to test
   int searchDepths[] = { 6, 7, 8 };
   int numDepths      = sizeof( searchDepths ) / sizeof( searchDepths[ 0 ] );

   // Test positions
   struct TestPosition
   {
      const char *name;
      const char *fen;
   };

   TestPosition positions[] = {
       { "StartingPosition",
         "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" },
       { "MiddleGame",
         "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4" },
       { "Tactical",
         "r1bqkb1r/pppp1ppp/2n5/4p3/2BnP3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 5" } };
   int numPositions = sizeof( positions ) / sizeof( positions[ 0 ] );

   // ============================================================================
   // FIRST: Test with hashtable completely disabled (baseline)
   // ============================================================================

   cout << "===================================================" << endl;
   cout << "Testing WITHOUT Hashtable (Baseline)" << endl;
   cout << "===================================================" << endl;

   SetUseHashTable( dNo ); // Disable hashtable completely

   for ( int p = 0; p < numPositions; p++ )
   {
      cout << "  Position: " << positions[ p ].name << endl;

      for ( int d = 0; d < numDepths; d++ )
      {
         int depth = searchDepths[ d ];

         // Set up the position
         ofstream fenFile( "C:\\VioletTools\\TestGenFEN.txt" );
         if ( fenFile.is_open() )
         {
            fenFile << positions[ p ].fen << endl;
            fenFile.close();
         }

         ReadFEN( "C:\\VioletTools\\TestGenFEN.txt", argsBoard, argsGeneralMoves, 0 );

         // Reset node counter
         SetNumberOfNodes( 0 );

         // Set search parameters
         // Record start time
         clock_t startTime = clock();

         // Perform the search
         int score = StartSearch( argsBoard, argsGeneralMoves, dAlpha, dBeta );

         // Get results
         BitBoard nodesEvaluated = GetNumberOfNodesSearched();
         clock_t  endTime        = clock();

         // Calculate metrics
         int timeMS =
             (int)( ( (double)( endTime - startTime ) / CLOCKS_PER_SEC ) * 1000.0 );
         if ( timeMS == 0 )
            timeMS = 1;

         long long nodesPerSecond =
             (long long)( (double)nodesEvaluated / (double)timeMS * 1000.0 );

         // Display results
         cout << "    Depth " << depth << ": " << nodesEvaluated << " nodes in "
              << timeMS << " ms (" << nodesPerSecond << " nps)" << endl;

         // Write to CSV - use 0 for hashBits and hashSizeMB to indicate "disabled"
         csvFile << "0,0.00," << positions[ p ].name << "," << depth << ","
                 << nodesEvaluated << "," << timeMS << "," << nodesPerSecond
                 << endl;
      }
   }
   cout << endl;

   // ============================================================================
   // NOW: Test with hashtable enabled at various sizes
   // ============================================================================

   SetUseHashTable( dYes ); // Re-enable hashtable

   // Iterate through all combinations
   for ( int h = 0; h < numHashSizes; h++ )
   {
      int    hashBits   = hashSizes[ h ];
      double hashSizeMB = ( pow( 2.0, hashBits ) * 16.0 ) / ( 1024.0 * 1024.0 );

      cout << "---------------------------------------------------" << endl;
      cout << "Testing Hash Size: 2^" << hashBits << " (" << fixed
           << setprecision( 2 ) << hashSizeMB << " MB)" << endl;
      cout << "---------------------------------------------------" << endl;

      // Set the hashtable size
      SetHashTableSizeBits( hashBits );
      ClearHashTable();

      for ( int p = 0; p < numPositions; p++ )
      {
         cout << "  Position: " << positions[ p ].name << endl;

         for ( int d = 0; d < numDepths; d++ )
         {
            int depth = searchDepths[ d ];

            // Set up the position by writing FEN to file and reading it
            ofstream fenFile( "C:\\VioletTools\\TestGenFEN.txt" );
            if ( fenFile.is_open() )
            {
               fenFile << positions[ p ].fen << endl;
               fenFile.close();
            }

            ReadFEN( "C:\\VioletTools\\TestGenFEN.txt", argsBoard, argsGeneralMoves, 0 );

            // Clear hash table for each test
            ClearHashTable();

            // Reset node counter
            SetNumberOfNodes( 0 );

            // Set search parameters
            SetSearchDepth( depth );
            SetStopGo( dGo );

            // Record start time
            clock_t startTime = clock();

            // Perform the search
            int score = StartSearch( argsBoard, argsGeneralMoves, dAlpha, dBeta );

            // CRITICAL: Use GetNumberOfNodesSearched() which is set by StartSearch
            // GetNumberOfNodes() returns the live counter which may be reset
            BitBoard nodesEvaluated = GetNumberOfNodesSearched();

            // Record end time
            clock_t endTime = clock();

            // Calculate metrics
            int timeMS =
                (int)( ( (double)( endTime - startTime ) / CLOCKS_PER_SEC ) * 1000.0 );
            if ( timeMS == 0 )
               timeMS = 1; // Avoid division by zero

            long long nodesPerSecond =
                (long long)( (double)nodesEvaluated / (double)timeMS * 1000.0 );

            // Display results
            cout << "    Depth " << depth << ": " << nodesEvaluated << " nodes in "
                 << timeMS << " ms (" << nodesPerSecond << " nps)" << endl;

            // Write to CSV
            csvFile << hashBits << "," << fixed << setprecision( 2 ) << hashSizeMB
                    << "," << positions[ p ].name << "," << depth << ","
                    << nodesEvaluated << "," << timeMS << "," << nodesPerSecond
                    << endl;
         }
      }
      cout << endl;
   }

   csvFile.close();

   cout << "==================================================" << endl;
   cout << "Test Complete! Results saved to hashtable_results.csv" << endl;
   cout << "==================================================" << endl;
   cout << endl;

   // Restore default hashtable size
   SetHashTableSizeBits( dNumberOfBitsInHash );
   ClearHashTable();
}
