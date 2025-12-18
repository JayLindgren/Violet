// Copyright 2006 by Jay Lindgren. All Rights Reserved.
//
// Null Move Performance Testing
// This file contains tests to analyze how null move pruning affects search performance
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

//////////////////////////////////////////////////////////////////////////////
// Test null move performance
//////////////////////////////////////////////////////////////////////////////
void TestNullMovePerformance(struct Board * argsBoard,
                             struct GeneralMove * argsGeneralMoves)
{
    cout << "==================================================" << endl;
    cout << "Null Move Performance Analysis" << endl;
    cout << "==================================================" << endl;
    cout << endl;

    // Open CSV file for results
    ofstream csvFile("null_move_results.csv");
    if (!csvFile.is_open())
    {
        cout << "ERROR: Could not create null_move_results.csv" << endl;
        return;
    }

    // Write CSV header
    csvFile << "Position,NullMoveEnabled,SearchDepth,NodesEvaluated,TimeMS,NodesPerSecond" << endl;

    // Define search depth
    int testDepth = 8;

    // Test positions
    struct TestPosition
    {
        const char* name;
        const char* fen;
    };

    TestPosition positions[] = {
        { "StartingPosition", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" },
        { "MiddleGame1", "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4" },
        { "MiddleGame2", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1" },
        { "MiddleGame3", "2r3k1/1q1nbppp/r3p3/3pP3/pPpP4/P1Q2N2/2R2PPP/1R2B1K1 b - - 0 1" }
    };
    int numPositions = sizeof(positions) / sizeof(positions[0]);

    // Test configurations: Off (0) and On (1)
    int nullMoveSettings[] = { dNo, dYes };
    const char* settingNames[] = { "Disabled", "Enabled" };

    // Iterate through positions
    for (int p = 0; p < numPositions; p++)
    {
        cout << "---------------------------------------------------" << endl;
        cout << "Position: " << positions[p].name << endl;
        cout << "---------------------------------------------------" << endl;

        for (int i = 0; i < 2; i++)
        {
            int useNullMove = nullMoveSettings[i];
            cout << "  Null Move: " << settingNames[i] << endl;

            // Set null move setting
            SetUseNullMove(useNullMove);

            // Set up the position
            ofstream fenFile("FEN.txt");
            if (fenFile.is_open())
            {
                fenFile << positions[p].fen << endl;
                fenFile.close();
            }
            
            ReadFEN("FEN.txt", argsBoard, argsGeneralMoves, 0);
            
            // Reset node counter
            SetNumberOfNodes(0);
            
            // Set search parameters
            SetSearchDepth(testDepth);
            SetStopGo(dGo);
            
            // Clear hashtable to ensure fair comparison
            ClearHashTable();

            // Record start time
            clock_t startTime = clock();
            
            // Perform the search
            int score = StartSearch(argsBoard, argsGeneralMoves, dAlpha, dBeta);
            
            // CRITICAL: Use GetNumberOfNodesSearched() which is set by StartSearch
            BitBoard nodesEvaluated = GetNumberOfNodesSearched();
            
            // Record end time
            clock_t endTime = clock();
            
            // Calculate metrics
            int timeMS = (int)(((double)(endTime - startTime) / CLOCKS_PER_SEC) * 1000.0);
            if (timeMS == 0) timeMS = 1; // Avoid division by zero
            
            long long nodesPerSecond = (long long)((double)nodesEvaluated / (double)timeMS * 1000.0);
            
            // Display results
            cout << "    Depth " << testDepth << ": " << nodesEvaluated << " nodes in " 
                 << timeMS << " ms (" << nodesPerSecond << " nps)" << endl;
            
            // Write to CSV
            csvFile << positions[p].name << ","
                   << (useNullMove == dYes ? "Enabled" : "Disabled") << ","
                   << testDepth << ","
                   << nodesEvaluated << ","
                   << timeMS << ","
                   << nodesPerSecond << endl;
        }
        cout << endl;
    }

    csvFile.close();

    cout << "==================================================" << endl;
    cout << "Test Complete! Results saved to null_move_results.csv" << endl;
    cout << "==================================================" << endl;
    cout << endl;

    // Restore default settings
    SetUseNullMove(dYes);
}
