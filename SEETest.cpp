#include "SEE.h"
#include "Definitions.h"
#include "Functions.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct SeeTestCase 
{
   string sFen;
   string sMoveStr; // e.g., "e4d5"
   int iExpectedValue;
   string sDescription;
};

//
//---------------------------------------------------------------------
//
string MoveToString( struct Move * argsMove ) 
{
   string s = "";
   s += ( char )( 'a' + dRow( argsMove->iFromSquare ) );
   s += ( char )( '1' + dCol( argsMove->iFromSquare ) );
   s += ( char )( 'a' + dRow( argsMove->iToSquare ) );
   s += ( char )( '1' + dCol( argsMove->iToSquare ) );
   return s;
}

//
//---------------------------------------------------------------------
//
void TestSEE( struct Board * argsBoard, 
              struct GeneralMove * argsGeneralMoves ) 
{
   cout << "Starting SEE Tests..." << endl;
    
   vector<SeeTestCase> vTests = 
   {
      // 1. Simple Hanging Piece
      { "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1", "d7d5", 0, "Pawn push (not capture)" }, 
        
      // 2. Pawn takes Pawn (protected)
      // White Pawn on e4, Black Pawn on d5. Black to move.
      // d5e4. White Knight on f2 protects e4.
      { "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1NPP/RNBQKB1R b KQkq - 1 2", "d5e4", 0, "Pawn takes protected Pawn" },
        
      // 3. Queen takes protected Pawn (Bad)
      { "rnbqkbnr/ppp1pppp/8/3p4/4Q3/8/PPPP1PPP/RNB1KBNR w KQkq - 0 1", "e4d5", -800, "Queen takes protected Pawn" },
        
      // 4. Rook takes Pawn protected by Rook
      // Rook on d8 protects d5.
      { "3r4/8/8/3p4/3R4/8/8/4K3 w - - 0 1", "d4d5", -400, "Rook takes protected Pawn" },
        
      // 5. X-Ray Attack
      { "3k4/8/8/3p4/8/8/3Q4/3R4 w - - 0 1", "d2d5", 100, "Queen takes hanging pawn (X-ray support)" },
        
      // 7. Attacker Ordering (Queen vs Rook)
      // White Rook on e1. Black Pawn on e5.
      // Defenders: Black Rook on e8, Black Queen on e7.
      // Move: Rxe5.
      // Correct (Rook defends):
      // 1. Rxe5 (100).
      // 2. Rxe5 (400). White loses 400. SEE = -400.
      // Bug (Queen defends):
      // 1. Rxe5 (100).
      // 2. Qxe5 (800). White loses 800. SEE = -800.
      { "4r1k1/4q3/8/4p3/8/8/8/4R1K1 w - - 0 1", "e1e5", -400, "Attacker Order: R vs R, Q" }
   };
    
   int iPassed = 0;
    
   for ( const auto & test : vTests ) 
   {
   
      // Setup board
      ReadFEN( test.sFen.c_str(), 
               argsBoard, 
               argsGeneralMoves, 
               2 );
        
      // Find the move
      struct Move sMoveList[ dNumberOfMoves ];
      CalculateMoves( sMoveList, 
                      argsBoard, 
                      argsGeneralMoves );
        
      int iMoveIndex = -1;
      for ( int i = 0; i < argsBoard->siNumberOfMoves; ++i ) 
      {
      
         string sMoveString = MoveToString( &sMoveList[ i ] );
         if ( test.sMoveStr == sMoveString ) 
         {
         
            iMoveIndex = i;
            break;
            
         }
      }
        
      if ( iMoveIndex == -1 ) 
      {
      
         cout << "[FAIL] " << test.sDescription << ": Move " << test.sMoveStr << " not found." << endl;
         continue;
         
      }
        
      int iSeeValue = See( argsBoard, 
                           argsGeneralMoves, 
                           &sMoveList[ iMoveIndex ] );
        
      if ( iSeeValue == test.iExpectedValue ) 
      {
      
         cout << "[PASS] " << test.sDescription << " (Expected: " << test.iExpectedValue << ", Got: " << iSeeValue << ")" << endl;
         iPassed++;
         
      } 
      else 
      {
      
         cout << "[FAIL] " << test.sDescription << " (Expected: " << test.iExpectedValue << ", Got: " << iSeeValue << ")" << endl;
         
      }
   }
    
   cout << "SEE Tests Completed: " << iPassed << "/" << vTests.size() << " passed." << endl;
}
