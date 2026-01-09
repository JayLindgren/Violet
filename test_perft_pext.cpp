/*
 * File: test_perft_pext.cpp
 * Purpose: Verification and Benchmarking of PEXT Move Generation
 *
 * Description:
 * This file serves as the primary validation harness for the Parallel Bit Extract (PEXT)
 * move generation modernization in the Violet chess engine. It performs two key roles:
 *
 * 1. Correctness Verification:
 *    - Compares PEXT attack lookups against a slow, classical reference implementation
 *      (RookAttacksSlowRef, BishopAttacksSlowRef) on random board states.
 *    - Ensures 100% agreement before proceeding to benchmarking.
 *
 * 2. Performance & Regression Testing (Perft):
 *    - Runs a recursive Perft test to depth 5 from the starting position.
 *    - Validates the total node count against the known correct value (4,865,609).
 *    - Provides a "Perft Divide" breakdown to isolate mismatches at the root move level.
 *    - Measures and reports Nodes Per Second (NPS) to quantify performance gains.
 *
 * Expected Output:
 *    - PEXT Logic: 100% CORRECT
 *    - Perft(5) Nodes: 4,865,609
 */

#include "Definitions.h"
#include "Functions.h"
#include "Structures.h"
#include "MoveGen.h"
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace std;

// Reference implementations copied for verification
BitBoard RookAttacksSlowRef( int sq, BitBoard block )
{
   BitBoard attacks = 0;
   int      r = sq / 8, c = sq % 8;
   for ( int r2 = r + 1; r2 <= 7; r2++ )
   {
      attacks |= ( 1ULL << ( r2 * 8 + c ) );
      if ( block & ( 1ULL << ( r2 * 8 + c ) ) )
         break;
   }
   for ( int r2 = r - 1; r2 >= 0; r2-- )
   {
      attacks |= ( 1ULL << ( r2 * 8 + c ) );
      if ( block & ( 1ULL << ( r2 * 8 + c ) ) )
         break;
   }
   for ( int c2 = c + 1; c2 <= 7; c2++ )
   {
      attacks |= ( 1ULL << ( r * 8 + c2 ) );
      if ( block & ( 1ULL << ( r * 8 + c2 ) ) )
         break;
   }
   for ( int c2 = c - 1; c2 >= 0; c2-- )
   {
      attacks |= ( 1ULL << ( r * 8 + c2 ) );
      if ( block & ( 1ULL << ( r * 8 + c2 ) ) )
         break;
   }
   return attacks;
}

BitBoard BishopAttacksSlowRef( int sq, BitBoard block )
{
   BitBoard attacks = 0;
   int      r = sq / 8, c = sq % 8;
   for ( int r2 = r + 1, c2 = c + 1; r2 <= 7 && c2 <= 7; r2++, c2++ )
   {
      attacks |= ( 1ULL << ( r2 * 8 + c2 ) );
      if ( block & ( 1ULL << ( r2 * 8 + c2 ) ) )
         break;
   }
   for ( int r2 = r + 1, c2 = c - 1; r2 <= 7 && c2 >= 0; r2++, c2-- )
   {
      attacks |= ( 1ULL << ( r2 * 8 + c2 ) );
      if ( block & ( 1ULL << ( r2 * 8 + c2 ) ) )
         break;
   }
   for ( int r2 = r - 1, c2 = c + 1; r2 >= 0 && c2 <= 7; r2--, c2++ )
   {
      attacks |= ( 1ULL << ( r2 * 8 + c2 ) );
      if ( block & ( 1ULL << ( r2 * 8 + c2 ) ) )
         break;
   }
   for ( int r2 = r - 1, c2 = c - 1; r2 >= 0 && c2 >= 0; r2--, c2-- )
   {
      attacks |= ( 1ULL << ( r2 * 8 + c2 ) );
      if ( block & ( 1ULL << ( r2 * 8 + c2 ) ) )
         break;
   }
   return attacks;
}

// Validates PEXT bitboard generation against a slow reference implementation
// Checks 10,000 random board states to ensure 100% correctness.
void VerifyPEXT()
{
   cout << "Verifying PEXT logic..." << endl;
   for ( int i = 0; i < 10000; i++ )
   {
      BitBoard occ = 0;

      // Randomish occupancy
      for ( int k = 0; k < 10; k++ )
         occ |= ( 1ULL << ( rand() % 64 ) );

      for ( int sq = 0; sq < 64; sq++ )
      {
         BitBoard pextR = GetRookAttacksPEXT( sq, occ );
         BitBoard slowR = RookAttacksSlowRef( sq, occ );
         if ( pextR != slowR )
         {
            cout << "ROOK MISMATCH at sq " << sq << " occ " << hex << occ << dec << endl;
            cout << "PEXT: " << hex << pextR << endl;
            cout << "SLOW: " << slowR << dec << endl;
            exit( 1 );
         }

         BitBoard pextB = GetBishopAttacksPEXT( sq, occ );
         BitBoard slowB = BishopAttacksSlowRef( sq, occ );
         if ( pextB != slowB )
         {
            cout << "BISHOP MISMATCH at sq " << sq << " occ " << hex << occ << dec << endl;
            cout << "PEXT: " << hex << pextB << endl;
            cout << "SLOW: " << slowB << dec << endl;
            exit( 1 );
         }
      }
   }
   cout << "PEXT Logic 100% CORRECT on random samples." << endl;
}

// Standard recursive Perft function to count leaf nodes at a given depth.
long long Perft( int depth, Board *board, GeneralMove *genMoves )
{
   if ( depth == 0 )
      return 1;

   Move moveList[ dNumberOfMoves ];
   CalculateMoves( moveList, board, genMoves );
   int       numMoves = board->siNumberOfMoves;
   long long nodes    = 0;

   for ( int i = 0; i < numMoves; ++i )
   {
      MakeMove( moveList, board, genMoves, i );

      LegalState( board, genMoves );
      if ( board->siLegalMove )
      {
         nodes += Perft( depth - 1, board, genMoves );
      }

      UndoMove( board, genMoves );
   }
   return nodes;
}

// Runs Perft at the root level and prints the node count for each root move.
// Useful for debugging mismatches by adhering to the "divide" standard.
long long PerftDivide( int depth, Board *board, GeneralMove *genMoves )
{
   cout << "Perft Divide Depth " << depth << endl;

   Move moveList[ dNumberOfMoves ];
   CalculateMoves( moveList, board, genMoves );
   int       numMoves   = board->siNumberOfMoves;
   long long totalNodes = 0;

   for ( int i = 0; i < numMoves; ++i )
   {
      char moveString[ 10 ];
      PrintMove( board, genMoves, &moveList[ i ], moveString );

      MakeMove( moveList, board, genMoves, i );
      LegalState( board, genMoves );

      if ( board->siLegalMove )
      {
         long long nodes = Perft( depth - 1, board, genMoves );
         totalNodes += nodes;
         cout << moveString << ": " << nodes << endl;
      }

      UndoMove( board, genMoves );
   }
   cout << "Total: " << totalNodes << endl;
   return totalNodes;
}

int main()
{
   cout << "Violet Chess Engine - PEXT Performance Test" << endl;
   cout << "-------------------------------------------" << endl;

   GeneralMove *genMoves = new GeneralMove();
   Board       *board    = new Board();

   // Initialize engine subsystems
   Initialize( board, genMoves );

   // Set up start position
   ReadFEN( "FEN.txt", board, genMoves, 1 );

   cout << "Engine initialized. PEXT tables generated." << endl;
   cout << "RookMasks[0]: " << hex << RookMasks[ 0 ] << dec << endl;
   cout << "RookAttacks[0][0]: " << hex << RookAttacks[ 0 ][ 0 ] << dec << endl;
   cout << "BishopMasks[0]: " << hex << BishopMasks[ 0 ] << dec << endl;

   VerifyPEXT();

   cout << "Running Perft(5) on Start Position..." << endl;

   auto      start = chrono::high_resolution_clock::now();
   long long nodes = PerftDivide( 5, board, genMoves );
   auto      end   = chrono::high_resolution_clock::now();

   chrono::duration<double> elapsed = end - start;
   double                   seconds = elapsed.count();
   double                   nps     = nodes / seconds;

   cout << "-------------------------------------------" << endl;
   cout << "Nodes: " << nodes << endl;
   cout << "Time:  " << fixed << setprecision( 3 ) << seconds << " s" << endl;
   cout << "NPS:   " << fixed << setprecision( 0 ) << nps << endl;
   cout << "-------------------------------------------" << endl;

   // Verify against known Perft(5) = 4,865,609
   long long expected = 4865609;
   if ( nodes == expected )
   {
      cout << "SUCCESS: Node count matches expected value!" << endl;
   }
   else
   {
      cout << "WARNING: Node count mismatch! Expected " << expected << endl;
      cout << "Difference: " << ( nodes - expected ) << endl;
   }

   delete board;
   delete genMoves;
   return 0;
}
