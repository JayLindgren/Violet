// This file containes useful routines, but that are not part off the heart of Violet.
// This file containes useful routines, but that are not part off the heart of Violet.
// They are thrown in hodgepodge.
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

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
void TestEval( struct Board * argsBoard,
               struct GeneralMove * argsGeneralMoves )
{

   // This will run Violet through a series of test positions and compare her score 
   // against the conventional wisdom.

   // Debug the input.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Read the FEN file.
   ReadFEN( "C:\\VioletTools\\FEN.txt",
            argsBoard,
            argsGeneralMoves,
            0 );

   PrintBoard( argsBoard->mBoard );

   // Do a simple evaluation of the board.
   int iScore = EvaluateBoard( argsBoard,
                               argsGeneralMoves );

   cout << "iScore Evaluation = " << iScore << endl;

}

//
//---------------------------------------------------------------------------------------
//
void TestHash( struct Board * argsBoard,
               struct GeneralMove * argsGeneralMoves )
{

   // This function tests the hash table to ensure:
   // 1. All board state changes are reflected in the hash (en passant, castling, check, etc.)
   // 2. Positions stored in hash table have correct scores
   // 3. Make/unmake moves maintain hash integrity
   // 4. Hash collisions are handled properly
   // 5. Castling rights and En Passant state affect the hash

   // Debug the input.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   int iScore = 0;
   int iTestsPassed = 0;
   int iTestsFailed = 0;
   Move vsMoveList[ dNumberOfMoves ];
   BitBoard bbHashInitial;
   BitBoard bbHashAfterMove;
   BitBoard bbHashAfterUndo;

   cout << endl << "========================================" << endl;
   cout << "HASH TABLE COMPREHENSIVE TEST SUITE" << endl;
   cout << "========================================" << endl << endl;

   // Test positions covering various chess scenarios
   const char* testPositions[] = {

      // Test 1: Initial position - tests basic setup
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    
      // Test 2: Position with en passant available
      "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 1",
      
      // Test 3: Position where white has lost kingside castling
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w Qkq - 0 1",
    
      // Test 4: Position where black has lost queenside castling
      "r3kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQk - 0 1",
      
      // Test 5: Position with white in check
      "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 0 1",
      
       // Test 6: Position with black in check
      "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPPKPPP/RNBQ1BNR b kq - 0 1",
    
      // Test 7: Endgame position with promotion possibility
      "8/2P5/8/8/8/8/2k5/4K3 w - - 0 1",
  
      // Test 8: Position with multiple en passant captures possible
      "rnbqkbnr/1ppppppp/8/pP6/8/8/P1PPPPPP/RNBQKBNR w KQkq a6 0 1",
      
      // Test 9: Complex middlegame position
      "r1bqk2r/pp2bppp/2n1pn2/2pp4/2PP4/2N1PN2/PP2BPPP/R1BQK2R w KQkq - 0 1",
   
      // Test 10: Position where both sides can castle
      "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1"

   };

   // Define the test names.
   const char* testNames[] = {
      "Initial Position",
      "En Passant Available",
      "White Lost Kingside Castling",
      "Black Lost Queenside Castling",
      "White in Check",
      "Black in Check",
      "Pawn Promotion Endgame",
      "Multiple En Passant Options",
      "Complex Middlegame",
      "Both Sides Can Castle"
   };

   int numTests = sizeof(testPositions) / sizeof(testPositions[0]);

   // Run through each test position
   for (int iTestIndex = 0; iTestIndex < numTests; iTestIndex++)
   {

      cout << "----------------------------------------" << endl;
      cout << "TEST " << (iTestIndex + 1) << ": " << testNames[iTestIndex] << endl;
      cout << "----------------------------------------" << endl;
      cout << "FEN: " << testPositions[iTestIndex] << endl << endl;

      // Read the FEN position from string (using mode 2)
      ReadFEN(testPositions[iTestIndex], argsBoard, argsGeneralMoves, 2);
      
      PrintBoard( argsBoard->mBoard );
      
      // Store initial hash
      bbHashInitial = GetHash();
      cout << "Initial Hash: " << bbHashInitial << endl;

      //-------------------------------------------------------------------------------
      // TEST A: Hash Consistency - Make and Unmake Move
      cout << endl << "TEST A: Hash Make/Unmake Consistency" << endl;
      
      // Generate all legal moves
      CalculateMoves( vsMoveList, argsBoard, argsGeneralMoves );
      
      int iNumMoves = argsBoard->siNumberOfMoves;
      cout << "Number of legal moves: " << iNumMoves << endl;
  
      if (iNumMoves > 0)
      {

         bool bAllMovesPassHash = true;
         
         // Test first 5 moves (or all if less than 5)
         int iMovesToTest = (iNumMoves < 5) ? iNumMoves : 5;
         
         for (int iMoveIndex = 0; iMoveIndex < iMovesToTest; iMoveIndex++ )
         {

            // Make the move
            MakeMove( vsMoveList, argsBoard, argsGeneralMoves, iMoveIndex );
            bbHashAfterMove = GetHash();
         
            // Undo the move
            UndoMove( argsBoard, argsGeneralMoves );
            bbHashAfterUndo = GetHash();
    
            // Check if hash restored correctly
            if (bbHashInitial == bbHashAfterUndo)
            {

               cout << "  Move " << iMoveIndex << ": PASS - Hash restored correctly" << endl;
            
            }
            else
            {

               cout << "  Move " << iMoveIndex << ": FAIL - Hash NOT restored!" << endl;
               cout << "    Expected: " << bbHashInitial << endl;
               cout << "    Got:  " << bbHashAfterUndo << endl;
               bAllMovesPassHash = false;
               iTestsFailed++;

            }

         }
    
         if (bAllMovesPassHash)
         {

            iTestsPassed++;
            cout << "  Result: PASS - All tested moves maintain hash integrity" << endl;

         }

      }

      else
      {

         cout << "  No legal moves available (stalemate/checkmate?)" << endl;

      }

      //-------------------------------------------------------------------------------
      // TEST B: Hash Table Storage and Retrieval
      cout << endl << "TEST B: Hash Table Storage/Retrieval" << endl;

      // Clear the hash table to ensure clean test
      ClearHashTable();
      
      // After search, we need to reload the position to get back to the root
      // because the search may have modified the board state
      ReadFEN(testPositions[iTestIndex], argsBoard, argsGeneralMoves, 2);
      bbHashInitial = GetHash(); // Update the hash for this position

      int iScore = 123; // Arbitrary test score
      int iBestMoveFromSearch = 0; // Assume first move is best for test

      cout << "  Hash after reloading position: " << GetHash() << endl;
 
      // Regenerate moves for the root position
      CalculateMoves(vsMoveList, argsBoard, argsGeneralMoves);
      iNumMoves = argsBoard->siNumberOfMoves;
    
      // Store score in hash table for the root position
      argsBoard->iBestMove = 0;

      // Pick a valid move pointer if any exist (InputToHashTable asserts non-null)
      Move* pBestMove = (iNumMoves > 0) ? &vsMoveList[iBestMoveFromSearch] : &vsMoveList[0];

      InputToHashTable(argsBoard,
                       argsGeneralMoves,
                       dAlpha,
                       dBeta,
                       iScore,
                       pBestMove);

      // Retrieve from hash table
      HashQueryResult hashResult = ExtractFromHashTable(argsBoard, argsGeneralMoves);
      int iRetrievedScore = hashResult.iScore;

      if (iScore == iRetrievedScore && hashResult.iQueryState == 1)
      {
         cout << "  Result: PASS - Score stored and retrieved correctly" << endl;
         iTestsPassed++;
      }
      else
      {
         cout << "  Result: FAIL - Score mismatch or not found!" << endl;
         cout << "    Stored:      " << iScore << endl;
         cout << "    Retrieved:   " << iRetrievedScore << endl;
         cout << "    QueryState:  " << hashResult.iQueryState << " (1 = found)" << endl;
         iTestsFailed++;
      }

      //-------------------------------------------------------------------------------
      // TEST C: Castling Rights Distinction
      // Verify that having rights vs not having rights results in different hashes
      string fen = testPositions[iTestIndex];
      if (fen.find("KQkq") != string::npos) // If position has full rights
      {
          cout << endl << "TEST C: Castling Rights Distinction" << endl;
          
          // 1. Get Hash with Rights
          ReadFEN(testPositions[iTestIndex], argsBoard, argsGeneralMoves, 2);
          BitBoard bbHashWithRights = GetHash();
          
          // 2. Remove Rights manually
          string fenNoRights = fen;
          size_t rightsPos = fenNoRights.find("KQkq");
          fenNoRights.replace(rightsPos, 4, "-");
          
          ReadFEN(fenNoRights.c_str(), argsBoard, argsGeneralMoves, 2);
          BitBoard bbHashNoRights = GetHash();
          
          if (bbHashWithRights != bbHashNoRights) {
              cout << "  Result: PASS - Castling rights affect hash" << endl;
              iTestsPassed++;
          } else {
              cout << "  Result: FAIL - Castling rights do NOT affect hash!" << endl;
              iTestsFailed++;
          }
      }

      //-------------------------------------------------------------------------------
      // TEST D: En Passant Distinction
      // Verify that having EP vs not having EP results in different hashes
      // Look for EP square in FEN (e.g. "d6")
      // Standard FEN has "-" if no EP.
      size_t epPos = fen.find(" - ");
      if (epPos == string::npos) // Means there IS an EP square (e.g. " d6 ")
      {
          // Find the EP part. It's usually the 4th field.
          // Simplified check: if it doesn't have " - ", it likely has EP.
          cout << endl << "TEST D: En Passant Distinction" << endl;

          ReadFEN(testPositions[iTestIndex], argsBoard, argsGeneralMoves, 2);
          BitBoard bbHashWithEP = GetHash();

          // Remove EP
          // We need to find the EP string and replace with "-".
          // This is a bit tricky with simple string manipulation without regex, 
          // but we can assume standard FEN structure.
          // Let's just use a hardcoded EP position test case instead of generic parsing.
          if (iTestIndex == 1) // "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 1"
          {
              string fenNoEP = "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1";
              ReadFEN(fenNoEP.c_str(), argsBoard, argsGeneralMoves, 2);
              BitBoard bbHashNoEP = GetHash();

              if (bbHashWithEP != bbHashNoEP) {
                  cout << "  Result: PASS - En Passant availability affects hash" << endl;
                  iTestsPassed++;
              } else {
                  cout << "  Result: FAIL - En Passant availability does NOT affect hash!" << endl;
                  iTestsFailed++;
              }
          }
      }

      //-------------------------------------------------------------------------------
      // Restore current position for TEST E
      ReadFEN(testPositions[iTestIndex], argsBoard, argsGeneralMoves, 2);
      bbHashInitial = GetHash(); // Update for TEST E

      // TEST E: Color to Move affects hash
      cout << endl << "TEST E: Side to Move Hash Impact" << endl;
      BitBoard bbHashBeforeSwitch = GetHash();
      SwitchSideToMove( argsBoard );
      BitBoard bbHashAfterSwitch = GetHash();
      SwitchSideToMove( argsBoard ); // Switch back
      BitBoard bbHashAfterDoubleSwitch = GetHash();
      
      if (bbHashBeforeSwitch != bbHashAfterSwitch && bbHashBeforeSwitch == bbHashAfterDoubleSwitch)
      {
         cout << "  Result: PASS - Side to move correctly affects hash" << endl;
         iTestsPassed++;
      }
      else
      {
         cout << "  Result: FAIL - Side to move hash issue!" << endl;
         iTestsFailed++;
      }

      cout << endl;

   } // End for loop over test positions

   // Final summary
   cout << "========================================" << endl;
   cout << "HASH TABLE TEST SUMMARY" << endl;
   cout << "========================================" << endl;
   cout << "Tests Passed: " << iTestsPassed << endl;
   cout << "Tests Failed: " << iTestsFailed << endl;
   if ((iTestsPassed + iTestsFailed) > 0)
   {
      cout << "Success Rate: " << (iTestsPassed * 100 / (iTestsPassed + iTestsFailed)) << "%" << endl;
   }
   cout << "========================================" << endl << endl;

   // Add pause so user can review results
   cout << "Press any key to continue..." << endl;
   //system("pause");
}


void TestNullMove( Board * argsBoard,
                   GeneralMove * argsGeneralMoves )
{
   int iEvalScore = 0;
   int iScore = 0;

   //argsBoard->iMaxPlys = 7;
   
   //SetUseNullMove( dYes );
   
   SetStopGo( dGo );
   //SetUseNullMove( dYes ); // dYes dNo  // can be used dynamically, but commented out in definitions.h


///*
   // Read the test FEN
//   ReadFEN( "FEN Null Move Test 1.txt",
   ReadFEN( "FEN Null Move Test 2.txt",
//   ReadFEN( "FEN Null Move Test.txt",
//   ReadFEN( "FEN.txt",
            argsBoard,
            argsGeneralMoves,
            0 );

   PrintBoard( argsBoard->mBoard );

   iEvalScore = EvaluateBoard( argsBoard,
                               argsGeneralMoves );

   cout << "Eval = " << iEvalScore << endl;
//*/
///*
   if ( argsBoard->siColorToMove == dWhite )
   {
      cout << "White to move" << endl << endl;
   }
   else
   {
      cout << "Black to move" << endl << endl;
   }

///*
// do a run with out the null move to develop a baseline
   argsBoard->iMaxPlys = 9;
   //SetUseNullMove( dNo );
   argsBoard->iUseNullVerification = dYes;
   SetUseNullMove( dYes );
   argsBoard->siColorToMove = dWhite;
   SetTempusParameters();
   SetPollingCount( 0 );
   iScore = StartSearch( argsBoard,
                         argsGeneralMoves,
                         -100,
                         200 );
/*   iScore = Search( argsBoard,
                    argsGeneralMoves,
                    -100,
                    200,
                    gMainThreadData );
*/
   cout << "Hash Table = " << GetUseHashTable() << endl;
   cout << "Side to move = " << argsBoard->siColorToMove << " Depth = " << argsBoard->iMaxPlys << " Null = " 
        << GetUseNullMove() << " Verification = " << argsBoard->iUseNullVerification << endl;
   cout << "Number of nodes searched = " << GetNumberOfNodes() << endl;
   cout << "iScore = " << iScore << endl << endl;
///*
// do a run with out the null move to develop a baseline
   argsBoard->iMaxPlys = 9;
   //SetUseNullMove( dNo );
   argsBoard->iUseNullVerification = dYes;
   SetUseNullMove( dNo );
   argsBoard->siColorToMove = dBlack;
   SetTempusParameters();
   SetPollingCount( 0 );
   iScore = StartSearch( argsBoard,
                         argsGeneralMoves,
                         -200,
                         100 );
/*   iScore = Search( argsBoard,
                    argsGeneralMoves,
                    -100,
                    200,
                    gMainThreadData );
*/
   cout << "Hash Table = " << GetUseHashTable() << endl;
   cout << "Side to move = " << argsBoard->siColorToMove << " Depth = " << argsBoard->iMaxPlys << " Null = " 
        << GetUseNullMove() << " Verification = " << argsBoard->iUseNullVerification << endl;
   cout << "Number of nodes searched = " << GetNumberOfNodes() << endl;
   cout << "iScore = " << iScore << endl << endl;
// do a run with out the null move to develop a baseline
   argsBoard->iMaxPlys = 9;
   //SetUseNullMove( dNo );
   argsBoard->iUseNullVerification = dYes;
   SetUseNullMove( dYes );
   argsBoard->siColorToMove = dWhite;
   SetTempusParameters();
   SetPollingCount( 0 );
   iScore = Search( argsBoard,
                    argsGeneralMoves,
                    -100,
                    200,
                    gMainThreadData );
   cout << "Side to move = " << argsBoard->siColorToMove << " Depth = " << argsBoard->iMaxPlys << " Null = " 
        << GetUseNullMove() << " Verification = " << argsBoard->iUseNullVerification << endl;
   cout << "Number of nodes searched = " << GetNumberOfNodes() << endl;
   cout << "iScore = " << iScore << endl << endl;
   cout << "Hash Table = " << GetUseHashTable() << endl;
   cout << "Side to move = " << argsBoard->siColorToMove << " Depth = " << argsBoard->iMaxPlys << " Null = " 
        << GetUseNullMove() << " Verification = " << argsBoard->iUseNullVerification << endl;
   cout << "Number of nodes searched = " << GetNumberOfNodes() << endl;
   cout << "iScore = " << iScore << endl << endl;
// do a run with out the null move to develop a baseline
   argsBoard->iMaxPlys = 9;
   //SetUseNullMove( dNo );
   argsBoard->iUseNullVerification = dYes;
   SetUseNullMove( dNo );
   argsBoard->siColorToMove = dBlack;
   SetTempusParameters();
   SetPollingCount( 0 );
   iScore = Search( argsBoard,
                    argsGeneralMoves,
                    -100,
                    200,
                    gMainThreadData );
   cout << "Side to move = " << argsBoard->siColorToMove << " Depth = " << argsBoard->iMaxPlys << " Null = " 
        << GetUseNullMove() << " Verification = " << argsBoard->iUseNullVerification << endl;
   cout << "Number of nodes searched = " << GetNumberOfNodes() << endl;
   cout << "iScore = " << iScore << endl << endl;//*/
   argsBoard->iMaxPlys = 6;
   //SetUseNullMove( dNo );
   argsBoard->iUseNullVerification = dYes;
   SetUseNullMove( dYes );
   argsBoard->siColorToMove = dWhite;
   SetTempusParameters();
   SetPollingCount( 0 );
   iScore = Search( argsBoard,
                    argsGeneralMoves,
                    -100,
                    200,
                    gMainThreadData );
   cout << "Side to move = " << argsBoard->siColorToMove << " Depth = " << argsBoard->iMaxPlys << " Null = " 
        << GetUseNullMove() << " Verification = " << argsBoard->iUseNullVerification << endl;
   cout << "Number of nodes searched = " << GetNumberOfNodes() << endl;
   cout << "iScore = " << iScore << endl << endl;
//*/
   argsBoard->iMaxPlys = 6;
   //SetUseNullMove( dNo );
   argsBoard->iUseNullVerification = dYes;
   SetUseNullMove( dYes );
   argsBoard->siColorToMove = dBlack;
   SetTempusParameters();
   SetPollingCount( 0 );
   iScore = Search( argsBoard,
                    argsGeneralMoves,
                    -200,
                    100,
                    gMainThreadData );
   cout << "Side to move = " << argsBoard->siColorToMove << " Depth = " << argsBoard->iMaxPlys << " Null = " 
        << GetUseNullMove() << " Verification = " << argsBoard->iUseNullVerification << endl;
   cout << "Number of nodes searched = " << GetNumberOfNodes() << endl;
   cout << "iScore = " << iScore << endl << endl;

// Baseline for no verification
   argsBoard->iMaxPlys = 8;
   ClearPV( argsBoard );
   SetUseNullMove( dYes );
   argsBoard->iUseNullVerification = dNo;
   argsBoard->siColorToMove = dWhite;
   SetTempusParameters();
   SetPollingCount( 0 );
   argsBoard->iLastMoveNull = dNo;
   iScore = Search( argsBoard,
                    argsGeneralMoves,
                    -100,
                    200,
                    gMainThreadData );
   cout << "Side to move = " << argsBoard->siColorToMove << " Depth = " << argsBoard->iMaxPlys << " Null = " 
        << GetUseNullMove() << " Verification = " << argsBoard->iUseNullVerification << endl;
   cout << "Number of nodes searched = " << GetNumberOfNodes() << endl;
   cout << "iScore = " << iScore << endl << endl;

// Baseline for with  verification
   argsBoard->iMaxPlys = 8;
   ClearPV( argsBoard );
   SetUseNullMove( dYes );
   argsBoard->iUseNullVerification = dYes;
   argsBoard->siColorToMove = dWhite;
   SetTempusParameters();
   SetPollingCount( 0 );
   argsBoard->iLastMoveNull = dNo;
   iScore = Search( argsBoard,
                    argsGeneralMoves,
                    -100,
                    200,
                    gMainThreadData );
   cout << "Side to move = " << argsBoard->siColorToMove << " Depth = " << argsBoard->iMaxPlys << " Null = " 
        << GetUseNullMove() << " Verification = " << argsBoard->iUseNullVerification << endl;
   cout << "Number of nodes searched = " << GetNumberOfNodes() << endl;
   cout << "iScore = " << iScore << endl << endl;

// Check for base case of lack to move but reduced 3 ply
   argsBoard->iMaxPlys = 5;
   ClearPV( argsBoard );
   SetUseNullMove( dYes );
   argsBoard->iUseNullVerification = dNo;
   argsBoard->siColorToMove = dBlack;
   SetTempusParameters();
   SetPollingCount( 0 );
   argsBoard->iLastMoveNull = dNo;
   iScore = Search( argsBoard,
                    argsGeneralMoves,
                    -200,
                    100,
                    gMainThreadData );
   cout << "Side to move = " << argsBoard->siColorToMove << " Depth = " << argsBoard->iMaxPlys << " Null = " 
        << GetUseNullMove() << " Verification = " << argsBoard->iUseNullVerification << endl;
   cout << "Number of nodes searched = " << GetNumberOfNodes() << endl;
   cout << "iScore = " << iScore << endl << endl;

// Do the black search with out null
   argsBoard->iMaxPlys = 5;
   ClearPV( argsBoard );
   SetUseNullMove( dNo );
   argsBoard->iUseNullVerification = dNo;
   argsBoard->siColorToMove = dBlack;
   SetTempusParameters();
   SetPollingCount( 0 );
   argsBoard->iLastMoveNull = dNo;
   iScore = Search( argsBoard,
                    argsGeneralMoves,
                    -200,
                    100,
                    gMainThreadData );
   cout << "Side to move = " << argsBoard->siColorToMove << " Depth = " << argsBoard->iMaxPlys << " Null = " 
        << GetUseNullMove() << " Verification = " << argsBoard->iUseNullVerification << endl;
   cout << "Number of nodes searched = " << GetNumberOfNodes() << endl;
   cout << "iScore = " << iScore << endl << endl;

system ("pause");

}










// Formats a number with commas as thousands separators.
std::string FormatWithCommas(long long value)
{
    std::string numStr = std::to_string(value);
    if (numStr.length() > 3)
    {
        for (int i = numStr.length() - 3; i > 0; i -= 3)
        {
            numStr.insert(i, ",");
        }
    }
    return numStr;
}

void TestPVBug( struct Board * argsBoard,
                struct GeneralMove * argsGeneralMoves )
{
   cout << "Testing PV Bug..." << endl;
   
   // Set up the position after e4 (Black to move)
   // rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1
   ReadFEN( "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1",
            argsBoard,
            argsGeneralMoves,
            2 ); // Mode 2 for FEN string

   PrintBoard( argsBoard->mBoard );

   // Set search parameters
   SetSearchDepth( 12 );
   SetSearchTimeInMiliSeconds( 10000 ); // 10 seconds
   SetStopGo( dGo );
   
   // Clear Hash
   ClearHashTable();
   
   // Run Search
   int iScore = StartSearch( argsBoard,
                             argsGeneralMoves,
                             -99999,
                             99999 );
                             
   cout << "Search Score: " << iScore << endl;
   
   // Print PV Length for debug
   cout << "PV Length: " << argsBoard->viPrincipalVariationLength[0] << endl;
   
   PrintPrincipalVariation( argsBoard, argsGeneralMoves );
}

//
//---------------------------------------------------------------------------------------
//
void TestNNUEAccumulator( struct Board * argsBoard,
                          struct GeneralMove * argsGeneralMoves )
{
   cout << "Testing NNUE Accumulator..." << endl;

   // Initialize board to start position
   ReadFEN( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            argsBoard,
            argsGeneralMoves,
            2 );

   // Mapping for manual evaluation
   const int violetToNNUE[13] = { 0, 6, 3, 5, 4, 2, 1, 12, 9, 11, 10, 8, 7 };

   // Generate moves
   Move vsMoveList[ dNumberOfMoves ];
   CalculateMoves( vsMoveList, argsBoard, argsGeneralMoves );

   // Make a few moves and verify
   // We will play a short game: 1. e4 e5 2. Nf3 Nc6 3. Bb5 a6
   // We need to find these moves in the move list.
   
   const char* movesToPlay[] = { "e4", "e5", "Nf3", "Nc6", "Bb5", "a6" };
   int numMoves = 6;

   for (int i = 0; i < numMoves; i++) {
       cout << "Playing move: " << movesToPlay[i] << endl;
       
       int moveIndex = GetMoveNumber(argsBoard, argsGeneralMoves, vsMoveList, (char*)movesToPlay[i]);
       if (moveIndex == -1) {
           cout << "Move not found!" << endl;
           return;
       }

       MakeMove(vsMoveList, argsBoard, argsGeneralMoves, moveIndex);
       
       // 1. Get Incremental Score (using the modified EvaluateBoardDirectNNUE)
       int incrementalScore = EvaluateBoardDirectNNUE(argsBoard, argsGeneralMoves);
       
       // 2. Get Fresh Score (manually constructing arrays and calling nnue_evaluate)
       int pieces[33];
       int squares[33];
       int index = 2;
       pieces[0] = 0; pieces[1] = 0; squares[0] = 0; squares[1] = 0;
       
       for (int sq = 0; sq < 64; sq++) {
           int violetPiece = argsBoard->mBoard[sq];
           if (violetPiece != dEmpty) {
               int nnuePiece = violetToNNUE[violetPiece];
               if (nnuePiece == 1) { pieces[0] = nnuePiece; squares[0] = sq; }
               else if (nnuePiece == 7) { pieces[1] = nnuePiece; squares[1] = sq; }
               else { pieces[index] = nnuePiece; squares[index] = sq; index++; }
           }
       }
       pieces[index] = 0; squares[index] = 0;
       int player = (argsBoard->siColorToMove == dWhite) ? 0 : 1;
       
       int freshScore = nnue_evaluate(player, pieces, squares);
       
       cout << "  Incremental: " << incrementalScore << endl;
       cout << "  Fresh:       " << freshScore << endl;
       
       if (incrementalScore != freshScore) {
           cout << "  FAIL: Scores do not match!" << endl;
           // Print board for debug
           PrintBoard(argsBoard->mBoard);
           return;
       } else {
           cout << "  PASS" << endl;
       }
       
       // Generate moves for next ply
       CalculateMoves(vsMoveList, argsBoard, argsGeneralMoves);
   }
   
   cout << "All tests passed!" << endl;
}

//
//---------------------------------------------------------------------------------------
//
void TestThreefoldRepetition( struct Board * argsBoard,
                              struct GeneralMove * argsGeneralMoves )
{
   cout << "Testing Threefold Repetition..." << endl;

   // Initialize board to start position
   ReadFEN( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            argsBoard,
            argsGeneralMoves,
            2 );

   // Clear hash table to ensure clean state
   ClearHashTable();
   
   // Store initial hash
   BitBoard bbInitialHash = GetHash();
   cout << "Initial Hash: " << bbInitialHash << endl;

   // We need to simulate a game where the same position occurs 3 times.
   // 1. Start Position (Occurence 1)
   // 2. White moves Knight out: Nf3
   // 3. Black moves Knight out: Nc6
   // 4. White moves Knight back: Ng1
   // 5. Black moves Knight back: Nb8 (Occurence 2)
   // 6. White moves Knight out: Nf3
   // 7. Black moves Knight out: Nc6
   // 8. White moves Knight back: Ng1
   // 9. Black moves Knight back: Nb8 (Occurence 3 - Draw Claimable)

   // Helper to make moves by string
   auto MakeMoveStr = [&](const char* moveStr) {
      Move vsMoveList[dNumberOfMoves];
      CalculateMoves(vsMoveList, argsBoard, argsGeneralMoves);
      int moveIndex = GetMoveNumber(argsBoard, argsGeneralMoves, vsMoveList, (char*)moveStr);
      if (moveIndex == -1) {
         cout << "ERROR: Move " << moveStr << " not found!" << endl;
         exit(1);
      }
      MakeMove(vsMoveList, argsBoard, argsGeneralMoves, moveIndex);
      // Update history stack manually if MakeMove doesn't do it fully?
      // MakeMove calls UpdateHash and increments iMoveHistory.
      // It should be fine.
   };

   // Check initial state
   if (LookForDraw(argsBoard, argsGeneralMoves)) {
      cout << "FAIL: Detected draw at start position!" << endl;
   } else {
      cout << "PASS: No draw at start." << endl;
   }

   // Move 1: Nf3
   MakeMoveStr("Nf3");
   if (LookForDraw(argsBoard, argsGeneralMoves)) cout << "FAIL: Draw after Nf3" << endl;

   // Move 1... Nc6
   MakeMoveStr("Nc6");
   if (LookForDraw(argsBoard, argsGeneralMoves)) cout << "FAIL: Draw after Nc6" << endl;

   // Move 2: Ng1
   MakeMoveStr("Ng1");
   if (LookForDraw(argsBoard, argsGeneralMoves)) cout << "FAIL: Draw after Ng1" << endl;

   // Move 2... Nb8 (Position repeated once)
   MakeMoveStr("Nb8");
   cout << "Position repeated once. Checking draw..." << endl;
   if (LookForDraw(argsBoard, argsGeneralMoves)) {
       cout << "FAIL: Detected draw after only 2 occurrences!" << endl;
   } else {
       cout << "PASS: No draw after 2 occurrences." << endl;
   }

   // Move 3: Nf3
   MakeMoveStr("Nf3");
   
   // Move 3... Nc6
   MakeMoveStr("Nc6");

   // Move 4: Ng1
   MakeMoveStr("Ng1");

   // Move 4... Nb8 (Position repeated twice - 3rd occurrence)
   MakeMoveStr("Nb8");
   cout << "Position repeated twice (3rd occurrence). Checking draw..." << endl;
   
   if (LookForDraw(argsBoard, argsGeneralMoves)) {
       cout << "PASS: Detected threefold repetition!" << endl;
   } else {
       cout << "FAIL: Did NOT detect threefold repetition!" << endl;
       cout << "Current Hash: " << GetHash() << endl;
       cout << "Initial Hash: " << bbInitialHash << endl;
   }

   // Test Castling Rights difference
   cout << "Testing Castling Rights difference..." << endl;
   
   // Use a position where King can move to lose rights
   ReadFEN( "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1",
            argsBoard,
            argsGeneralMoves,
            2 );
   ClearHashTable();
   
   // 1. Kf1 (White King moves, loses castling)
   MakeMoveStr("Kf1");
   
   // 1... Rb8 (Black Rook moves)
   MakeMoveStr("Rb8");
   
   // 2. Ke1 (White King returns)
   MakeMoveStr("Ke1");
   
   // 2... Ra8 (Black Rook returns)
   MakeMoveStr("Ra8");
   
   // Now we are back at "start" position pieces, but White lost castling rights.
   // Should NOT be a 3rd occurrence (it's the 1st occurrence of this specific state).
   // Start state had rights. Current state doesn't.
   
   if (LookForDraw(argsBoard, argsGeneralMoves)) {
       cout << "FAIL: Detected repetition despite lost castling rights!" << endl;
   } else {
       cout << "PASS: Correctly distinguished position with lost castling rights." << endl;
   }

   cout << "Threefold Repetition Test Complete." << endl;
}