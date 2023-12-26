// This file containes useful routines, but that are not part off the heart of Violet.
// They are thrown in hodgepodge.
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
void TestEval( struct Board * argsBoard,
               struct GeneralMove * argsGeneralMoves )
{

   // This will run Violet through a series of test positions and compare her score 
   // against the conventional wisdom.

   // Debug the input.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Read the FEN file.
   ReadFEN( "fen.txt",
            argsBoard,
            argsGeneralMoves,
            0 );

   PrintBoard( argsBoard->mBoard );

   // Do a simple evaluation of the board.
   int iScore = EvaluateBoard( argsBoard,
                               argsGeneralMoves );

   cout << "iScore Evaluation = " << iScore << endl;
/*
   // Perform a search.
   iScore =  Search( & sBoard, 
                     & sGeneralMoves,
                     dAlpha,
                     dBeta );

   cout << "iScore Search = " << iScore << endl << endl;
//*/
/*
   // Read the test FEN
   ReadFEN( "FEN e5.txt",
            argsBoard,
            argsGeneralMoves,
            0 );

   PrintBoard( argsBoard->mBoard );

   int iEvalScore = EvaluateBoard( argsBoard,
                                   argsGeneralMoves );

   cout << "Evaluation of the Board = " << iEvalScore << endl << endl;

   // Read the test FEN
   ReadFEN( "FEN d5.txt",
            argsBoard,
            argsGeneralMoves,
            0 );

   PrintBoard( argsBoard->mBoard );

   iEvalScore = EvaluateBoard( argsBoard,
                               argsGeneralMoves );

   cout << "Evaluation of the Board = " << iEvalScore << endl << endl;

   // Read the test FEN
   ReadFEN( "FEN 4.txt",
            argsBoard,
            argsGeneralMoves,
            0 );

   PrintBoard( argsBoard->mBoard );

   iEvalScore = EvaluateBoard( argsBoard,
                               argsGeneralMoves );

   cout << "Evaluation of the Board = " << iEvalScore << endl << endl;
//*/

}

//
//---------------------------------------------------------------------------------------
//
void TestHash( struct Board * argsBoard,
               struct GeneralMove * argsGeneralMoves )
{

   // This will run Violet through a series of test positions and compare her score 
   // against the conventional wisdom.

   // Debug the input.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   int iScore = 0;
   int iMoveNumber = 12;
   int iBestMove = 0;
   int iBestMoveSearched = 0;
   Move vsMoveList[ dNumberOfMoves ]; // stored locally due to the complexities of the recursive search

   // Read the FEN file.
   ReadFEN( "fen.txt",
            argsBoard,
            argsGeneralMoves,
            1 );


   PrintBoard( argsBoard->mBoard );
   cout << "Hash = " << GetHash() << endl;
   SetStopGo( dGo );
   iScore = StartSearch( argsBoard,
                         argsGeneralMoves,
                         dAlpha,
                         dBeta );
   cout << "Hash = " << GetHash() << endl;


   SetUseNullMove(    dNo );
   PrintBoard( argsBoard->mBoard );
   cout << "Hash = " << GetHash() << endl;
   SetStopGo( dGo );
   iScore = StartSearch( argsBoard,
                         argsGeneralMoves,
                         dAlpha,
                         dBeta );
   cout << "Hash = " << GetHash() << endl;
   SetUseNullMove(    dYes );

///*
   SetUseHashTable(    dNo );
   PrintBoard( argsBoard->mBoard );
   cout << "Hash = " << GetHash() << endl;
   SetStopGo( dGo );
   iScore = StartSearch( argsBoard,
                         argsGeneralMoves,
                         dAlpha,
                         dBeta );
   cout << "Hash = " << GetHash() << endl;
   SetUseHashTable(    dYes );
//*/
///*
   SetUseLMR(    dNo );
   PrintBoard( argsBoard->mBoard );
   cout << "Hash = " << GetHash() << endl;
   SetStopGo( dGo );
   iScore = StartSearch( argsBoard,
                         argsGeneralMoves,
                         dAlpha,
                         dBeta );
   cout << "Hash = " << GetHash() << endl;
   SetUseLMR(    dYes );
//*/






   // Generate all of the legal moves.
   CalculateMoves( vsMoveList, 
                   argsBoard, 
                   argsGeneralMoves );

   // Make a move on the board.
   MakeMove( vsMoveList, 
             argsBoard,
             argsGeneralMoves,
             12 );
   cout << "Moved Hash = " << GetHash() << endl;

   //  Now switch the side to move.
   SwitchSideToMove( argsBoard );
   cout << "Switched Hash = " << GetHash() << endl;

   // Generate all of the legal moves.
   CalculateMoves( vsMoveList, 
                   argsBoard, 
                   argsGeneralMoves );
/*
   PrintMoves( vsMoveList,
               argsBoard->siNumberOfMoves,
				 	argsBoard);
*/
   //  Now switch the side to move.
   SwitchSideToMove( argsBoard );
   cout << "Switched Hash = " << GetHash() << endl;

   // Now switch back.
   //  Now switch the side to move.
   SwitchSideToMove( argsBoard );
   cout << "Switched Hash = " << GetHash() << endl;

   //Move vsMoveList[ dNumberOfMoves ]; // stored locally due to the complexities of the recursive search

   // Search the initial position
   PrintBoard( argsBoard->mBoard );
   cout << "Hash = " << GetHash() << endl;
   SetStopGo( dGo );
   iScore = FirstSearch( argsBoard,
                         argsGeneralMoves,
                         dAlpha,
                         dBeta,
                         & iBestMove,
                         & iBestMoveSearched );
   cout << "Hash = " << GetHash() << endl;
   cout << "Hash = " << GetHash() << endl;
   SetStopGo( dGo );
   iScore = StartSearch( argsBoard,
                         argsGeneralMoves,
                         dAlpha,
                         dBeta );
   cout << "Hash = " << GetHash() << endl;
   cout << "Score from the initial position = " << iScore << endl;

   // Input the info into the hash table.
   argsBoard->iBestMove = iMoveNumber;
   cout << "Hash = " << GetHash() << endl;
   InputToHashTable( argsBoard,
                     argsGeneralMoves,
                     dAlpha,
                     dBeta,
                     iScore,
                     & vsMoveList[ iMoveNumber ] );
   cout << "Hash = " << GetHash() << endl;


   // Extract the data from the hash table
   ExtractFromHashTable( argsBoard,
                         argsGeneralMoves );
   cout << "Hash = " << GetHash() << endl;


   cout << "Score from initial position as extracted from the hash table = " << GetHashScore() << endl;
   
   // Generate all of the legal moves.
   CalculateMoves( vsMoveList, 
                   argsBoard, 
                   argsGeneralMoves );

   // Make a move on the board.
   MakeMove( vsMoveList, 
             argsBoard,
             argsGeneralMoves,
             iMoveNumber );
   cout << "Hash = " << GetHash() << endl;
   // Undo the move.
   UndoMove( argsBoard, 
             argsGeneralMoves );
   cout << "Hash = " << GetHash() << endl;

   // Score the position.
   PrintBoard( argsBoard->mBoard );
   SetStopGo( dGo );
   iScore = FirstSearch( argsBoard,
                         argsGeneralMoves,
                         dAlpha,
                         dBeta,
                         & iBestMove,
                         & iBestMoveSearched );

   cout << "Score from first move = " << iScore << endl;

   // Put the data into the hash table.
   argsBoard->iBestMove = iMoveNumber;
   InputToHashTable( argsBoard,
                     argsGeneralMoves,
                     dAlpha,
                     dBeta,
                     iScore,
                     & vsMoveList[ iMoveNumber ] );

   // Extract the original position information.
   PrintBoard( argsBoard->mBoard );
   ExtractFromHashTable( argsBoard,
                         argsGeneralMoves );

   cout << "Score from second move as extracted from the hash table = " << GetHashScore() << endl;

   // Undo the move.
   UndoMove( argsBoard, 
             argsGeneralMoves );
   argsBoard->iNumberOfPlys = -1;

   // Extract the original position information.
   PrintBoard( argsBoard->mBoard );
   ExtractFromHashTable( argsBoard,
                         argsGeneralMoves );

   cout << "Score from initial position as extracted from the hash table = " << GetHashScore() << endl;









   
   // Generate all of the legal moves.
   CalculateMoves( vsMoveList, 
                   argsBoard, 
                   argsGeneralMoves );

   // Make a move on the board.
   MakeMove( vsMoveList, 
             argsBoard,
             argsGeneralMoves,
             iMoveNumber );

   // Score the position.
   PrintBoard( argsBoard->mBoard );
   SetStopGo( dGo );
   iScore = FirstSearch( argsBoard,
                         argsGeneralMoves,
                         dAlpha,
                         dBeta,
                         & iBestMove,
                         & iBestMoveSearched );
/*
   iScore = - Search( argsBoard,
                      argsGeneralMoves,
                      - dBeta,
                      - dAlpha );
*/

   // Put the data into the hash table.
   argsBoard->iBestMove = iMoveNumber;
   InputToHashTable( argsBoard,
                     argsGeneralMoves,
                     dAlpha,
                     dBeta,
                     iScore,
                     & vsMoveList[ iMoveNumber ] );

   // Score the position.
   PrintBoard( argsBoard->mBoard );
   SetStopGo( dGo );
   iScore = FirstSearch( argsBoard,
                         argsGeneralMoves,
                         dAlpha,
                         dBeta,
                         & iBestMove,
                         & iBestMoveSearched );

   // Undo the move.
   UndoMove( argsBoard, 
             argsGeneralMoves );
   argsBoard->iNumberOfPlys = -1;

   // Extract the original position information.
   PrintBoard( argsBoard->mBoard );
   ExtractFromHashTable( argsBoard,
                         argsGeneralMoves );

   // Score the position.
   PrintBoard( argsBoard->mBoard );
   SetStopGo( dGo );
   iScore = FirstSearch( argsBoard,
                         argsGeneralMoves,
                         dAlpha,
                         dBeta,
                         & iBestMove,
                         & iBestMoveSearched );

   // Undo the move.
   UndoMove( argsBoard, 
             argsGeneralMoves );
   argsBoard->iNumberOfPlys = -1;

   // Extract the original position information.
   PrintBoard( argsBoard->mBoard );
   ExtractFromHashTable( argsBoard,
                         argsGeneralMoves );

   // Score the position.
   PrintBoard( argsBoard->mBoard );
   SetStopGo( dGo );
   iScore = FirstSearch( argsBoard,
                         argsGeneralMoves,
                         dAlpha,
                         dBeta,
                         & iBestMove,
                         & iBestMoveSearched );


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
                    200 );
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
                    200 );
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
                    200 );
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
                    200 );
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
                    200 );
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
                    100 );
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
                    200 );
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
                    200 );
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
                    100 );
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
                    100 );
   cout << "Side to move = " << argsBoard->siColorToMove << " Depth = " << argsBoard->iMaxPlys << " Null = " 
        << GetUseNullMove() << " Verification = " << argsBoard->iUseNullVerification << endl;
   cout << "Number of nodes searched = " << GetNumberOfNodes() << endl;
   cout << "iScore = " << iScore << endl << endl;

system ("pause");

}







/*
//
//-----------------------------------------------------------------------------------
//
void TestHashUpdate()
{


   int i = 0;
   i = 2;
/*
   // This function tests the hash updating.
   struct GeneralMove sGeneralMoves;
   struct Board sBoard;


   // Calculate all of the general moves
   //GenerateGeneralMove( &sGeneralMoves );

   //  Create the initial board.$
   CreateBoard( &sBoard,
                &sGeneralMoves );                

   // Initialize the hash table.
   InitializeHashTable();

   // Check the update of the hash.
   Move vsMoveList[ dNumberOfMoves ]; 

   // Generate all of the legal moves.
   CalculateMoves( vsMoveList, 
                   & sBoard, 
                   & sGeneralMoves );

   cout << "Hash before move" << endl;
   PrintBitBoard( gsHashTable.bbHash );

   MakeMove( vsMoveList, 
             & sBoard, 
             & sGeneralMoves,
             1 );

   cout << "Hash after move" << endl;
   PrintBitBoard( gsHashTable.bbHash );

   UndoMove( & sBoard,
             & sGeneralMoves );

   //UpdateHash( & sBoard,
   //            vsMoveList,
   //            1,
   //            & sGeneralMoves );

   cout << "Hash of restored board" << endl;
   PrintBitBoard( gsHashTable.bbHash );

   //UpdateHash( & sBoard,
   //            vsMoveList,
   //            1,
   //            & sGeneralMoves );

   //PrintBitBoard( gsHashTable.bbHash );
*/
//}   

/*
//
//-----------------------------------------------------------------------------------
//
void TestHashUpdateing()
{

   // This function tests the hash updating.
   struct GeneralMove sGeneralMoves;
   struct Board sBoard;


   // Calculate all of the general moves
   GenerateGeneralMove( &sGeneralMoves );

   //  Create the initial board.$
   CreateBoard( &sBoard,
                &sGeneralMoves );                

   // Initialize the hash table.
   InitializeHashTable();

   // Check the update of the hash.
   Move vsMoveList[ dNumberOfMoves ]; 

   // Generate all of the legal moves.
   CalculateMoves( vsMoveList, 
                   & sBoard, 
                   & sGeneralMoves );

   cout << "Hash before move" << endl;
   PrintBitBoard( GetHash() );

   MakeMove( vsMoveList, 
             & sBoard, 
             & sGeneralMoves,
             1 );

   cout << "Hash after move" << endl;
   PrintBitBoard( GetHash() );

   // Generate all of the legal moves.
   CalculateMoves( vsMoveList, 
                   & sBoard, 
                   & sGeneralMoves );

   MakeMove( vsMoveList, 
             & sBoard, 
             & sGeneralMoves,
             1 );

   cout << "Hash after move" << endl;
   PrintBitBoard( gsHashTable.bbHash );

   UndoMove( & sBoard,
             & sGeneralMoves );

   cout << "Hash of undomove" << endl;
   PrintBitBoard( gsHashTable.bbHash );

   UndoMove( & sBoard,
             & sGeneralMoves );

   cout << "Hash of restored board" << endl;
   PrintBitBoard( gsHashTable.bbHash );

   //UpdateHash( & sBoard,
   //            vsMoveList,
   //            1,
   //            & sGeneralMoves );

   //PrintBitBoard( gsHashTable.bbHash );

} 
*/  
/*
//
//-----------------------------------------------------------------------------------
//

void TestHashScores( struct Board * argsBoard,
                     struct GeneralMove * argsGeneralMoves )
{


   // Allocations.
   //int iMoveCount;
   int iScore;
   Move vsMoveList[ dNumberOfMoves ]; // stored locally due to the complexities of the recursive search
   int vsiMoveOrder[ dNumberOfMoves ];
   BitBoard bbKey = gsHashTable.bbHash & argsGeneralMoves->bbKey1;
   int viScore[ dNumberOfMoves ];

   // Initialize the score.
   iScore = 0;

   // Initialize the depth.
   argsBoard->iMaxPlys = 7;

   // Generate all of the legal moves.
   CalculateMoves( vsMoveList, 
                   argsBoard, 
                   argsGeneralMoves );


   // let the number of moves live in a local variable.
   int iNumberOfMoves = argsBoard->siNumberOfMoves;

   // Calculate the score and put into the hash table.
   for ( int iMoveIndex = 0; iMoveIndex < iNumberOfMoves; iMoveIndex++ )
   //for ( int iMoveIndex = 0; iMoveIndex < 1; iMoveIndex++ )
   {
      
      //cout << "Initial hash = " << gsHashTable.bbHash << endl;
   
      MakeMove( vsMoveList, 
                argsBoard, 
                argsGeneralMoves, 
                iMoveIndex );

      //cout << "New hash     = " << gsHashTable.bbHash << endl;
                
      vsMoveList[ iMoveIndex ].iScore = EvaluateBoard( argsBoard,
                                                       argsGeneralMoves );

      //argsBoard->iBestMove = iMoveIndex;
      //argsBoard->iBestMove = -999;
      argsBoard->iBestMove = 128;
            
      // Put the score into the hash table.
      InputToHashTable( argsBoard,
                        argsGeneralMoves,
                        1000,
                        -1000,
                        vsMoveList[ iMoveIndex ].iScore,
                        & vsMoveList[ iMoveIndex ] );

      bbKey = gsHashTable.bbHash & argsGeneralMoves->bbKey1;
      //cout << "Entry        = " << gsHashTable.mbbHashTable[ bbKey ] << endl;

      //cout << "iMoveIndex = " << iMoveIndex << " iScore = " << vsMoveList[ iMoveIndex ].iScore << endl;
      //cout << "iMoveIndex = " << iMoveIndex << " iDepth = " << argsBoard->iMaxPlys - argsBoard->iNumberOfPlys << endl;
      cout << "iMoveIndex = " << iMoveIndex << " iBestMove = " << argsBoard->iBestMove << endl;        
                                       
      UndoMove( argsBoard,
                argsGeneralMoves );

   }

   cout << endl;

   // Test the sorting of the hash moves.
   SortMovesHash( vsiMoveOrder,
                  vsMoveList,
                  iNumberOfMoves,
                  argsBoard,
                  argsGeneralMoves,
                  viScore );


   // Extract the score from the hash table.
/*   for ( int iMoveIndex = 0; iMoveIndex < iNumberOfMoves; iMoveIndex++ )
   {

      cout << "iMoveIndex = " << iMoveIndex << " Score = " << vsMoveList[ vsiMoveOrder[ iMoveIndex ] ].iScore << endl;

   } */
/*
   cout << endl;

   // Extract the score from the hash table.
   for ( int iMoveIndex = 0; iMoveIndex < iNumberOfMoves; iMoveIndex++ )
   //for ( int iMoveIndex = 0; iMoveIndex < 1; iMoveIndex++ )
   {

      //cout << "Initial hash = " << gsHashTable.bbHash << endl;
      
      MakeMove( vsMoveList, 
                argsBoard, 
                argsGeneralMoves, 
                iMoveIndex );

      //cout << "New hash     = " << gsHashTable.bbHash << endl;

      bbKey = gsHashTable.bbHash & argsGeneralMoves->bbKey1;
      //cout << "Entry        = " << gsHashTable.mbbHashTable[ bbKey ] << endl;
 
      // Check to see if the move is in the hash table.
      ExtractFromHashTable( argsBoard,
                            argsGeneralMoves );

      //cout << "iMoveIndex = " << iMoveIndex << " iScore = " << gsHashTable.iScore << endl;
      //cout << "iMoveIndex = " << iMoveIndex << " iScore = " << gsHashTable.bbDepth << endl;
      //cout << "iMoveIndex = " << iMoveIndex << " bbBestMove = " << gsHashTable.bbBestMove << endl;
      //cout << "iMoveIndex = " << iMoveIndex << " bbAge = " << gsHashTable.bbAge << endl;
      //cout << "iMoveIndex = " << iMoveIndex << " iBestMove = " << gsHashTable.bbBestMove << endl;        

                                                       
      UndoMove( argsBoard,
                argsGeneralMoves );

   }

}
*/