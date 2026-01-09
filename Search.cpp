// Copyright 2006 by Jay Lindgren. All Rights Reserved.
//
// This function contains the details of the search.
#include "Fathom-master/src/tbprobe.h"
#define _CRT_SECURE_NO_WARNINGS
#include "Functions.h"
#include "Thread.h"
#include "SEE.h"

#include <cmath> // For log and other math functions
#include <iostream>
#include <sstream> // Add to top of file
#include <vector>  // Needed for this example

// Global pointers for move lists - REPLACED BY ThreadData
// Move (*gvsMoveList)[dNumberOfMoves] = nullptr;
// int (*gvsiMoveOrder)[dNumberOfMoves] = nullptr;
// int (*gviMoveScore)[dNumberOfMoves] = nullptr;

// Main thread data
ThreadData *gMainThreadData = nullptr;

// Shared History Heuristic
SharedHistory gSharedHistory;

// === Enhanced LMR: Logarithmic Reduction Table ===
// Initialized at startup with formula: R = floor(ln(depth) * ln(move) / LMR_BASE)
int              gLMRTable[ 64 ][ 64 ]; // [depth][moveNumber]
constexpr double LMR_BASE = 2.0;        // Tunable: lower = more aggressive reduction

void InitializeLMRTable()
{
   for ( int depth = 0; depth < 64; depth++ )
   {
      for ( int move = 0; move < 64; move++ )
      {
         if ( depth < 3 || move == 0 )
         {
            gLMRTable[ depth ][ move ] = 0;
         }
         else
         {
            double reduction           = ( log( (double)depth ) * log( (double)move ) ) / LMR_BASE;
            gLMRTable[ depth ][ move ] = (int)floor( reduction );

            // Ensure minimum reduction of 1 for late moves at reasonable depth
            if ( depth >= 3 && move >= 4 && gLMRTable[ depth ][ move ] == 0 )
            {
               gLMRTable[ depth ][ move ] = 1;
            }
         }
      }
   }
}

// If in deep mode, include the appropriate files

// SearchParameters gsSearchParameters;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lazy SMP Helper Functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initialize shared PV at start of search
void InitializeSharedPV()
{
   if ( gsTempus.sharedPV )
   {
      std::lock_guard<std::mutex> lock( gsTempus.sharedPV->pvMutex );
      gsTempus.sharedPV->score                = -99999;
      gsTempus.sharedPV->depth                = 0;
      gsTempus.sharedPV->bestMove.iFromSquare = -1;
      gsTempus.sharedPV->bestMove.iToSquare   = -1;
      for ( int i = 0; i < dNumberOfPlys; i++ )
      {
         gsTempus.sharedPV->pv[ i ].iFromSquare = -1;
         gsTempus.sharedPV->pv[ i ].iToSquare   = -1;
      }
   }
}

// Update shared PV if this thread found a better move
void UpdateSharedPV( Move *pv, int score, int depth )
{
   if ( !gsTempus.sharedPV || !pv )
      return;

   // Use try_lock to avoid blocking helper threads
   // If the lock is busy, it means another thread is updating it.
   // We can skip this update to avoid contention, as the other update is likely
   // good enough or we'll get another chance later.
   std::unique_lock<std::mutex> lock( gsTempus.sharedPV->pvMutex,
                                      std::try_to_lock );
   if ( !lock.owns_lock() )
      return;

   // Only update if better depth or same depth with better score
   if ( depth > gsTempus.sharedPV->depth ||
        ( depth == gsTempus.sharedPV->depth && score > gsTempus.sharedPV->score ) )
   {
      gsTempus.sharedPV->bestMove = pv[ 0 ];
      gsTempus.sharedPV->score    = score;
      gsTempus.sharedPV->depth    = depth;
      // Copy entire PV line
      for ( int i = 0; i < dNumberOfPlys && pv[ i ].iFromSquare >= 0; i++ )
      {
         gsTempus.sharedPV->pv[ i ] = pv[ i ];
      }
   }
}

// Get shared PV and copy to board's PV
void GetSharedPV( Board *board )
{
   if ( !gsTempus.sharedPV || !board )
      return;

   std::lock_guard<std::mutex> lock( gsTempus.sharedPV->pvMutex );
   // Copy shared PV to board's PV
   for ( int i = 0; i < dNumberOfPlys; i++ )
   {
      board->vmPrincipalVariation[ 0 ][ i ] = gsTempus.sharedPV->pv[ i ];
   }
}

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper function to count the number of set bits in a BitBoard
// This is used for material counting and zugzwang detection
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline int CountBits( BitBoard bb )
{
   int count = 0;
   while ( bb )
   {
      count++;
      bb &= bb - 1; // Clear the least significant bit
   }
   return count;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Define global variables for keeping track of the number of nodes counted.
// Global variables suck, but are awsome for allowing for Deep Violet.
// access to the table:
// Note that the scope for the globe variables is only this file.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Define the global continer for search parameters.
// SearchParameters gsSearchParameters; // REMOVED for Thread Safety

// Countermove heuristic: stores best reply to opponent's last move
Move gCounterMoves[ 64 ][ 64 ];

//
//
//---------------------------------------------------------------------
//
//
// The function does iterative deeping.
//
int StartSearch( Board *argsBoard, GeneralMove *argsGeneralMoves, int argiAlpha,
                 int argiBeta )
{
   // std::cout << "StartSearch: Begin" << std::endl;
   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // [Syzygy Root Integration - FIXED]
   // Phase 2: Corrected castling rights encoding mismatch between Violet and Fathom
   {
      // Only probe if tablebases are actually loaded
      if ( TB_LARGEST > 0 )
      {
         int tbPieceCount = Count( argsBoard->bbWhitePieces | argsBoard->bbBlackPieces );
         if ( tbPieceCount <= TB_LARGEST )
         {
            // CRITICAL FIX: Castling rights encoding conversion
            // Violet encoding: bit0=WK, bit1=BK, bit2=WQ, bit3=BQ
            // Fathom expects:  bit0=WK, bit1=WQ, bit2=BK, bit3=BQ
            unsigned tbCastling = 0;
            if ( argsBoard->bbCastle & 1 )
               tbCastling |= 1; // TB_CASTLING_K (White KS) - correct
            if ( argsBoard->bbCastle & 4 )
               tbCastling |= 2; // TB_CASTLING_Q (White QS) - remapped 4→2
            if ( argsBoard->bbCastle & 2 )
               tbCastling |= 4; // TB_CASTLING_k (Black KS) - remapped 2→4
            if ( argsBoard->bbCastle & 8 )
               tbCastling |= 8; // TB_CASTLING_q (Black QS) - correct

            // En passant square with validation
            unsigned tbEP = 0;
            if ( argsBoard->bbEP )
            {
               int tbEPSquares[ 64 ];
               if ( Find( argsBoard->bbEP, tbEPSquares, argsGeneralMoves ) > 0 )
               {
                  tbEP = tbEPSquares[ 0 ];
                  // Validate EP square is in valid range (rank 3 or 6)
                  if ( !( ( tbEP >= 16 && tbEP <= 23 ) || ( tbEP >= 40 && tbEP <= 47 ) ) )
                  {
                     if ( GetInterfaceMode() == dUCI || GetInterfaceMode() == dVFE )
                     {
                        std::cout << "info string TB: Invalid EP square " << tbEP << ", setting to 0" << std::endl;
                     }
                     tbEP = 0;
                  }
               }
            }

            bool tbTurn = ( argsBoard->siColorToMove == dWhite );

            // Validate piece counts before probing
            int whitePieceCount = Count( argsBoard->bbWhitePieces );
            int blackPieceCount = Count( argsBoard->bbBlackPieces );

            if ( whitePieceCount == 0 || blackPieceCount == 0 )
            {
               if ( GetInterfaceMode() == dUCI || GetInterfaceMode() == dVFE )
               {
                  std::cout << "info string TB: Invalid piece count (W:" << whitePieceCount
                            << " B:" << blackPieceCount << ")" << std::endl;
               }
            }
            else
            {
               // Diagnostic logging (can be disabled after testing)
               if ( GetInterfaceMode() == dConsole && false ) // Set to true for debugging
               {
                  std::cout << "TB Probe Debug:" << std::endl;
                  std::cout << "  White pieces: 0x" << std::hex << argsBoard->bbWhitePieces << std::dec << std::endl;
                  std::cout << "  Black pieces: 0x" << std::hex << argsBoard->bbBlackPieces << std::dec << std::endl;
                  std::cout << "  Castling: Violet=0x" << std::hex << (int)argsBoard->bbCastle
                            << " Fathom=0x" << tbCastling << std::dec << std::endl;
                  std::cout << "  EP: " << tbEP << std::endl;
                  std::cout << "  Turn: " << ( tbTurn ? "White" : "Black" ) << std::endl;
               }

               unsigned tbRootResults[ TB_MAX_MOVES ];
               unsigned tbRootScore = tb_probe_root(
                   argsBoard->bbWhitePieces, argsBoard->bbBlackPieces,
                   argsBoard->bbWhiteKing | argsBoard->bbBlackKing,
                   argsBoard->bbWhiteQueen | argsBoard->bbBlackQueen,
                   argsBoard->bbWhiteRook | argsBoard->bbBlackRook,
                   argsBoard->bbWhiteBishop | argsBoard->bbBlackBishop,
                   argsBoard->bbWhiteKnight | argsBoard->bbBlackKnight,
                   argsBoard->bbWhitePawn | argsBoard->bbBlackPawn,
                   argsBoard->iFiftyMoveCounter,
                   tbCastling, tbEP, tbTurn, tbRootResults );

               if ( tbRootScore != TB_RESULT_FAILED )
               {
                  int tbWDL   = TB_GET_WDL( tbRootScore );
                  int tbDTZ   = TB_GET_DTZ( tbRootScore );
                  int tbFrom  = TB_GET_FROM( tbRootScore );
                  int tbTo    = TB_GET_TO( tbRootScore );
                  int tbPromo = TB_GET_PROMOTES( tbRootScore );

                  // Convert TB move to algebraic notation
                  char fromFile = 'a' + ( tbFrom % 8 );
                  char fromRank = '1' + ( tbFrom / 8 );
                  char toFile   = 'a' + ( tbTo % 8 );
                  char toRank   = '1' + ( tbTo / 8 );

                  std::string tbMoveStr = "";
                  tbMoveStr += fromFile;
                  tbMoveStr += fromRank;
                  tbMoveStr += toFile;
                  tbMoveStr += toRank;

                  // Handle promotion
                  if ( tbPromo != TB_PROMOTES_NONE )
                  {
                     if ( tbPromo == TB_PROMOTES_QUEEN )
                        tbMoveStr += 'q';
                     else if ( tbPromo == TB_PROMOTES_ROOK )
                        tbMoveStr += 'r';
                     else if ( tbPromo == TB_PROMOTES_BISHOP )
                        tbMoveStr += 'b';
                     else if ( tbPromo == TB_PROMOTES_KNIGHT )
                        tbMoveStr += 'n';
                  }

                  if ( GetInterfaceMode() == dUCI || GetInterfaceMode() == dVFE )
                  {
                     std::string resultLabel = "Draw";
                     if ( tbWDL == TB_WIN )
                     {
                        resultLabel = tbTurn ? "Win" : "Loss";
                     }
                     else if ( tbWDL == TB_LOSS )
                     {
                        resultLabel = tbTurn ? "Loss" : "Win";
                     }

                     std::cout << "info string Syzygy Found Move: " << tbMoveStr
                               << " Result: " << resultLabel << " DTZ: " << tbDTZ
                               << " (White's perspective)" << std::endl;
                  }

                  // Calculate score based on WDL
                  int tbScore = 0;
                  if ( tbWDL == TB_WIN )
                     tbScore = 30000 - tbDTZ; // Win score
                  else if ( tbWDL == TB_LOSS )
                     tbScore = -30000 + tbDTZ; // Loss score
                  else
                     tbScore = 0; // Draw

                  // Store in PV so ComputerMove can handle the bestmove output
                  argsBoard->vmPrincipalVariation[ 0 ][ 0 ].iFromSquare = tbFrom;
                  argsBoard->vmPrincipalVariation[ 0 ][ 0 ].iToSquare   = tbTo;
                  argsBoard->vmPrincipalVariation[ 0 ][ 0 ].iMoveType   = dRegular;
                  argsBoard->vmPrincipalVariation[ 0 ][ 0 ].iPiece      = argsBoard->mBoard[ tbFrom ];

                  // Handle promotion details
                  if ( tbPromo != TB_PROMOTES_NONE )
                  {
                     argsBoard->vmPrincipalVariation[ 0 ][ 0 ].iMoveType = dPromote;
                     if ( tbTurn ) // White
                     {
                        if ( tbPromo == TB_PROMOTES_QUEEN )
                           argsBoard->vmPrincipalVariation[ 0 ][ 0 ].iPiece = dWhiteQueen;
                        else if ( tbPromo == TB_PROMOTES_ROOK )
                           argsBoard->vmPrincipalVariation[ 0 ][ 0 ].iPiece = dWhiteRook;
                        else if ( tbPromo == TB_PROMOTES_BISHOP )
                           argsBoard->vmPrincipalVariation[ 0 ][ 0 ].iPiece = dWhiteBishop;
                        else if ( tbPromo == TB_PROMOTES_KNIGHT )
                           argsBoard->vmPrincipalVariation[ 0 ][ 0 ].iPiece = dWhiteKnight;
                     }
                     else // Black
                     {
                        if ( tbPromo == TB_PROMOTES_QUEEN )
                           argsBoard->vmPrincipalVariation[ 0 ][ 0 ].iPiece = dBlackQueen;
                        else if ( tbPromo == TB_PROMOTES_ROOK )
                           argsBoard->vmPrincipalVariation[ 0 ][ 0 ].iPiece = dBlackRook;
                        else if ( tbPromo == TB_PROMOTES_BISHOP )
                           argsBoard->vmPrincipalVariation[ 0 ][ 0 ].iPiece = dBlackBishop;
                        else if ( tbPromo == TB_PROMOTES_KNIGHT )
                           argsBoard->vmPrincipalVariation[ 0 ][ 0 ].iPiece = dBlackKnight;
                     }
                  }

                  // Set max plys to indicate valid PV
                  argsBoard->iMaxPlys        = 0;
                  argsBoard->iMaxPlysReached = 0;

                  // Generate moves to find the matching move struct
                  Move vsTBMoveList[ dNumberOfMoves ];
                  CalculateMoves( vsTBMoveList, argsBoard, argsGeneralMoves );
                  for ( int i = 0; i < dNumberOfMoves; i++ )
                  {
                     if ( vsTBMoveList[ i ].iFromSquare == -1 )
                        break;
                     if ( vsTBMoveList[ i ].iFromSquare == tbFrom && vsTBMoveList[ i ].iToSquare == tbTo )
                     {
                        argsBoard->vmPrincipalVariation[ 0 ][ 0 ]  = vsTBMoveList[ i ];
                        argsBoard->viPrincipalVariationLength[ 0 ] = 1;
                        break;
                     }
                  }

                  if ( GetInterfaceMode() == dUCI || GetInterfaceMode() == dVFE )
                  {
                     std::cout << "info string TB: Returning with score " << tbScore << std::endl;
                  }

                  return tbScore;
               }
               else if ( GetInterfaceMode() == dUCI || GetInterfaceMode() == dVFE )
               {
                  std::cout << "info string TB: Probe failed (position not in tablebase)" << std::endl;
               }
            }
         }
      }
   }

   // Declare holders for determining if a new partial search of a level has
   // returned a good search. The idea is not to compare moves across plys,
   // because the scores will not be consistent. The idea, is that if after a ply
   // has been searched, to make sure that at least the previous best move has
   // been searched.  If a better one has been found at ply, then use that one.
   int iBestMove    = -1; // hold the best move from the previous ply
   int iBestMoveOld = iBestMove;
   int iBestMoveSearched =
       0; // holds whether or not the best move has been searched.
   int iNumberOfMoves = 0;
   int iOldScore      = 0;
   int iQScore =
       0; // used for setting the initial score of the aspirational search.

   // Initialize search.
   InitializeSearch();

   // Keep an old board around for scoring.
   Board *sOldBoard = new Board();

   // Reset atomic node counter for this search
   // Reset atomic node counter for this search behavior
   // gTotalNodesSearched.store(0, std::memory_order_relaxed);

   // Reset Main Thread counter
   gMainThreadData->nodesSearched = 0;

   // Reset Helper Threads counters
   for ( auto *thread : gThreads )
   {
      if ( thread && thread->GetData() )
      {
         thread->GetData()->nodesSearched = 0;
      }
   }

   // Set the Tempus to search.
   SetStopGo( dGo );

   // Delcare the score.
   int  iScore = 0;
   Move vsMoveList[ dNumberOfMoves ];

   // Generate all of the legal moves.
   CalculateMoves( vsMoveList, argsBoard, argsGeneralMoves );

   // Save the number of moves.
   iNumberOfMoves = argsBoard->siNumberOfMoves;

   // Reset the plys counted
   argsBoard->iMaxPlys = 0;

   // Reset the polling count
   SetPollingCount( 0 );

   // Set the new time for the move update.
   SetTimeLastMoveUpdate();

   // Only do if using an aspirational search.
   if ( GetAspirationSearch() )
   {

      // Set up an aspirational search.
      iQScore = -QuiesenceSearch( argsBoard, argsGeneralMoves, -argiBeta,
                                  -argiAlpha, gMainThreadData );

      // Define the aspirational search window
      argiBeta  = iQScore + giAspirationWindowWidth;
      argiAlpha = iQScore - giAspirationWindowWidth;
   }

   // Initialize threads if not already done (default to 1 if not set via UCI)
   if ( gThreads.empty() )
   {
      InitializeThreads( 1 );
   }

   // LAZY SMP: Initialize shared PV structure
   InitializeSharedPV();

   // Launch helper threads once before the ID loop
   for ( size_t i = 1; i < gThreads.size(); ++i )
   {
      // FIX: Helpers should search to the same depth as main thread, not
      // dNumberOfPlys
      gThreads[ i ]->StartSearch( argsBoard, argsGeneralMoves, argiAlpha, argiBeta,
                                  GetSearchDepth() );
   }

   // Loop over the depth.
   for ( int iDepth = 0; iDepth < GetSearchDepth(); iDepth++ )
   {
      // Debug print to trace ID loop progress
      // Debug print to trace ID loop progress
      if ( GetInterfaceMode() == dConsole )
      {
         // std::cout << "DEBUG: StartSearch ID Loop Depth " << iDepth << std::endl;
      }

      // Look for control.
      if ( GetStopGo() == dStop )
      {
         // Stop helper threads
         SetStopGo( dStop );

         // Time is up. Restore the board state and PV from the last completed
         // search.
         *argsBoard = *sOldBoard;
         GetTempusPVOldToBoard( argsBoard );
         iScore = iOldScore;

         // Stop helper threads when search is done
         SetStopGo( dStop );

         delete sOldBoard;
         return iScore;
      }

      // Set whether or not the best move has been searched at this ply.
      iBestMoveSearched        = 0;
      iBestMove                = argsBoard->iBestMove;
      argsBoard->iLastMoveNull = dNo;

      // Set the depth of the board.
      argsBoard->iMaxPlys        = iDepth;
      argsBoard->iMaxPlysReached = -1;

      // cout << "Board before search." << endl;
      // PrintBoard(argsBoard->mBoard);

      // Call the first search routine.
      iScore = FirstSearch( argsBoard, argsGeneralMoves, argiAlpha, argiBeta,
                            &iBestMove, &iBestMoveSearched, gMainThreadData );

      // Aspiration Window Re-Search Logic
      if ( GetAspirationSearch() )
      {
         // Fail Low or Fail High - Re-search with full window
         if ( iScore <= argiAlpha || iScore >= argiBeta )
         {

            argiAlpha = -dInfiniteTime; // -Infinity
            argiBeta  = dInfiniteTime;  // +Infinity
            iScore    = FirstSearch( argsBoard, argsGeneralMoves, argiAlpha, argiBeta,
                                     &iBestMove, &iBestMoveSearched, gMainThreadData );
         }

         // Update window for next iteration centered on current score
         argiAlpha = iScore - giAspirationWindowWidth;
         argiBeta  = iScore + giAspirationWindowWidth;
      }
      // cout << "Board after search." << endl;

      // LAZY SMP: Update shared PV with main thread's results
      if ( argsBoard->vmPrincipalVariation[ 0 ][ 0 ].iFromSquare >= 0 )
      {
         UpdateSharedPV( argsBoard->vmPrincipalVariation[ 0 ], iScore, iDepth );
      }

      // If the search for this depth completed without being stopped, save the
      // state.
      if ( GetStopGo() == dGo )
      {
         // Save the state from the completed search.
         memcpy( sOldBoard, argsBoard, sizeof( Board ) );
         iOldScore = iScore;
         SetTempusPVOldFromBoard( argsBoard );
      }

      // Update the number of nodes searched in Tempus for use in opotimization of
      // parameters
      SetNumberOfNodesSearched( (int)( GetNumberOfNodes() ) );

      // Update the folks at home.
      if ( GetInterfaceMode() == dConsole && GetStopGo() == dGo &&
           GetConsoleOutput() == dYes )
      {

         cout << "Ply = " << iDepth + 1 << " Score = " << iScore
              << " qDepth = " << argsBoard->iMaxPlysReached
              << " Nodes searched = " << FormatWithCommas( GetTotalNodes() ) << endl;
         // cout << "max depth reached = " << argsBoard->iMaxPlysReached << endl;
         // cout << "Number of nodes searched = " <<   << endl;
         // cout << "Maximum depth obtained = " << argsBoard->iMaxPlysReached <<
         // endl;
         if ( GetStopGo() == dGo )
         {

            // Print the principal variation
            PrintPrincipalVariation( argsBoard, argsGeneralMoves );

            // Print the multiPV if ponder is on:
            if ( GetPonder() )
            {

               PrintMultiPV( argsBoard, argsGeneralMoves, iNumberOfMoves );
            }
         }
         // cout << "Board after PV print." << endl;
         // PrintBoard(argsBoard->mBoard);
         // std::cout << "Please hit any key to continue."; std::cin.get();

         // If we are at check mate, stop the search.
         if ( iScore <= dMate )
         {

            SetStopGo( dStop );
            iDepth = dNumberOfPlys;
         }
      }
      if ( GetInterfaceMode() == dUCI && GetStopGo() == dGo )
      {
         std::stringstream pv_stream;
         int               pvLen = argsBoard->viPrincipalVariationLength[ 0 ];
         if ( pvLen >= dNumberOfPlys )
            pvLen = dNumberOfPlys - 1;

         for ( int iPVCount = 0; iPVCount <= pvLen; iPVCount++ )
         {
            if ( argsBoard->vmPrincipalVariation[ 0 ][ iPVCount ].iFromSquare < 0 ||
                 argsBoard->vmPrincipalVariation[ 0 ][ iPVCount ].iToSquare < 0 )
               break;
            char strMove[ 64 ];
            CreateAlgebraicMove( strMove, &argsBoard->vmPrincipalVariation[ 0 ][ iPVCount ], 0 );
            if ( iPVCount > 0 )
               pv_stream << " ";
            pv_stream << strMove;
         }

         SendInfoCommand( iDepth + 1, argsBoard->iMaxPlysReached + 1, iScore,
                          (long long)GetTotalNodes(),
                          (unsigned long)( ( clock() - gsTempus.giTimeStart ) * 1000 / CLOCKS_PER_SEC ),
                          pv_stream.str() );
      }
      if ( iScore <= dMate && GetStopGo() == dGo )
      {

         SetStopGo( dStop );
         iDepth = dNumberOfPlys;
      }
   }

   // NOW stop helper threads (after main thread completes all iterations)
   SetStopGo( dStop );

   // Wait briefly for helpers to finish their current iteration
   std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );

   // Check SharedPV for better results from helper threads
   if ( gsTempus.sharedPV )
   {
      std::lock_guard<std::mutex> lock( gsTempus.sharedPV->pvMutex );

      bool useShared = false;
      // If helper found a deeper search, or same depth with better score
      // Note: SharedPV depth is 1-based, iMaxPlys is 0-based
      if ( gsTempus.sharedPV->depth > argsBoard->iMaxPlys + 1 )
      {
         useShared = true;
      }
      else if ( gsTempus.sharedPV->depth == argsBoard->iMaxPlys + 1 &&
                gsTempus.sharedPV->score > iScore )
      {
         useShared = true;
      }

      if ( useShared )
      {
         // Update Score
         iScore = gsTempus.sharedPV->score;

         // Update PV
         for ( int i = 0; i < dNumberOfPlys; i++ )
         {
            argsBoard->vmPrincipalVariation[ 0 ][ i ] = gsTempus.sharedPV->pv[ i ];
         }

         // Update Best Move Index
         // We need to find the move in vsMoveList to get its index
         int  bestMoveIndex = -1;
         Move bestMove      = gsTempus.sharedPV->bestMove;
         for ( int i = 0; i < argsBoard->siNumberOfMoves; i++ )
         {
            if ( vsMoveList[ i ].iFromSquare == bestMove.iFromSquare &&
                 vsMoveList[ i ].iToSquare == bestMove.iToSquare &&
                 vsMoveList[ i ].iPiece == bestMove.iPiece &&
                 vsMoveList[ i ].iPromote == bestMove.iPromote )
            {
               bestMoveIndex = i;
               break;
            }
         }

         if ( bestMoveIndex != -1 )
         {
            argsBoard->iBestMove = bestMoveIndex;
         }
      }
   }

   // This prints out the tried and vaile for the null move search/pruning.
   // cout << "Gobal Tried = " << gsSearchParameters.iTried << " Failed = " <<
   // gsSearchParameters.iFailed << endl;

   // Clean up the old board.
   delete sOldBoard;

   // Return the score.
   return iScore;
}

//
//
//---------------------------------------------------------------------
//
//
// The function does a recursive nega max search. (Refactored Version)
//
int FirstSearch( struct Board *argsBoard, struct GeneralMove *argsGeneralMoves,
                 int argiAlpha, int argiBeta, int *argiBestMove,
                 int *argiBestMoveSearched, ThreadData *threadData )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );
   assert( threadData != nullptr );

   // Increment node counter thread-locally
   threadData->nodesSearched++;

   // Allocations.
   assert( argsBoard->iNumberOfPlys + 1 >= 0 );
   Move *vsMoveList   = threadData->vsMoveList[ argsBoard->iNumberOfPlys + 1 ];
   int  *vsiMoveOrder = threadData->vsiMoveOrder[ argsBoard->iNumberOfPlys + 1 ];
   int  *viMoveScores = threadData->viMoveScore[ argsBoard->iNumberOfPlys + 1 ];
   int   iMoveCount;
   char  strUpdate[ 1280 ];
   char  strMove[ 64 ];
   int   iHaveMove      = 0;
   int   iBestMoveIndex = -1;

   // Set the search depth to zero.
   argsBoard->iNumberOfPlys   = -1;
   argsBoard->iMaxPlysReached = -1;

   // Set the null verification to yes.
   argsBoard->iUseNullVerification = dYes;

   // Initialize the history stack.
   if ( argsBoard->iMoveHistory == -1 )
   {
      argsBoard->iMoveHistory = 0;
   }

   // Put the initial hash into the move history.
   argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbHash = argsBoard->bbHash;

   // Initialize the ply length.
   argsBoard->viPrincipalVariationLength[ argsBoard->iNumberOfPlys + 1 ] =
       argsBoard->iNumberOfPlys;

   // Look for a draw - function returns a if a three fold repetition is found.
   if ( LookForDraw( argsBoard, argsGeneralMoves ) )
   {
      // Return a draw score.
      return 0;
   }

   // Generate all of the legal moves.
   CalculateMoves( vsMoveList, argsBoard, argsGeneralMoves );

   // Let the number of moves live in a local variable.
   int iNumberOfMoves = argsBoard->siNumberOfMoves;

   // Check the state of the board and see if it is legal.
   LegalState( argsBoard, argsGeneralMoves );

   // If the move is not legal, undo the move and continue.
   if ( argsBoard->siLegalMove == 0 )
   {
      // return because we found an illegal move.
      return -dAlpha;
   }
#if defined( dUseHash )
   // See if the hash table can help.
   // We EXTRACT from the hash table to get the 'Best Move' for sorting,
   // but we DO NOT return the score. The root must always search.
   // We EXTRACT from the hash table to get the 'Best Move' for sorting,
   // but we DO NOT return the score. The root must always search.
   HashQueryResult hashResult =
       ExtractFromHashTable( argsBoard, argsGeneralMoves );
#else
   HashQueryResult hashResult;
   hashResult.iQueryState = 0;
   hashResult.iBestMove   = 128;
#endif

   // [Syzygy WDL Integration]
   {
      int tbPieceCount = Count( argsBoard->bbWhitePieces | argsBoard->bbBlackPieces );
      if ( tbPieceCount <= TB_LARGEST )
      {
         unsigned tbCastling = 0;
         if ( argsBoard->bbCastle & 1 )
            tbCastling |= TB_CASTLING_K;
         if ( argsBoard->bbCastle & 4 )
            tbCastling |= TB_CASTLING_Q;
         if ( argsBoard->bbCastle & 2 )
            tbCastling |= TB_CASTLING_k;
         if ( argsBoard->bbCastle & 8 )
            tbCastling |= TB_CASTLING_q;

         unsigned tbEP = 0;
         if ( argsBoard->bbEP )
         {
            int tbEPSquares[ 64 ];
            if ( Find( argsBoard->bbEP, tbEPSquares, argsGeneralMoves ) > 0 )
               tbEP = tbEPSquares[ 0 ];
         }

         bool tbTurn = ( argsBoard->siColorToMove == dWhite );

         // Use tb_probe_root instead of tb_probe_wdl to handle 50-move rule correctly
         // tb_probe_wdl returns TB_RESULT_FAILED when rule50 != 0, but tb_probe_root
         // properly handles it and returns CURSED_WIN/BLESSED_LOSS when appropriate
         unsigned tbRootResults[ TB_MAX_MOVES ];
         unsigned tbRootResult = tb_probe_root(
             argsBoard->bbWhitePieces, argsBoard->bbBlackPieces,
             argsBoard->bbWhiteKing | argsBoard->bbBlackKing,
             argsBoard->bbWhiteQueen | argsBoard->bbBlackQueen,
             argsBoard->bbWhiteRook | argsBoard->bbBlackRook,
             argsBoard->bbWhiteBishop | argsBoard->bbBlackBishop,
             argsBoard->bbWhiteKnight | argsBoard->bbBlackKnight,
             argsBoard->bbWhitePawn | argsBoard->bbBlackPawn,
             argsBoard->iFiftyMoveCounter,
             tbCastling, tbEP, tbTurn, tbRootResults );

         if ( tbRootResult != TB_RESULT_FAILED )
         {
            unsigned tbWDL = TB_GET_WDL( tbRootResult );
            unsigned tbDTZ = TB_GET_DTZ( tbRootResult );

            // Diagnostic output for testing (especially for 50-move rule edge cases)
            if ( GetInterfaceMode() == dUCI )
            {
               const char *wdlStr[] = { "LOSS", "BLESSED_LOSS", "DRAW", "CURSED_WIN", "WIN" };
               std::cout << "info string Syzygy Found: WDL=" << wdlStr[ tbWDL ]
                         << " DTZ=" << tbDTZ
                         << " 50-move=" << argsBoard->iFiftyMoveCounter << std::endl;
            }

            int tbScore = 0;
            // Handle all 5 WDL values according to Syzygy spec:
            // TB_LOSS (0), TB_BLESSED_LOSS (1), TB_DRAW (2), TB_CURSED_WIN (3), TB_WIN (4)

            if ( tbWDL == TB_WIN ) // 4 - Definite win
            {
               tbScore = -dMate - 1000 - argsBoard->iNumberOfPlys;
            }
            else if ( tbWDL == TB_CURSED_WIN ) // 3 - Win but 50-move draw
            {
               // This is theoretically winning but cannot be won before 50-move rule
               // Treat as draw for practical purposes
               tbScore = 0;
               if ( GetInterfaceMode() == dUCI )
               {
                  std::cout << "info string TB: CURSED_WIN treated as DRAW (50-move rule)" << std::endl;
               }
            }
            else if ( tbWDL == TB_DRAW ) // 2 - Draw
            {
               tbScore = 0;
            }
            else if ( tbWDL == TB_BLESSED_LOSS ) // 1 - Loss but 50-move draw
            {
               // This is theoretically losing but can be held to draw via 50-move rule
               tbScore = 0;
               if ( GetInterfaceMode() == dUCI )
               {
                  std::cout << "info string TB: BLESSED_LOSS treated as DRAW (50-move rule)" << std::endl;
               }
            }
            else if ( tbWDL == TB_LOSS ) // 0 - Definite loss
            {
               tbScore = dMate + 1000 + argsBoard->iNumberOfPlys;
            }

            // Apply the score to alpha-beta window
            if ( tbWDL == TB_WIN )
            {
               if ( tbScore >= argiBeta )
                  return tbScore;
               if ( tbScore > argiAlpha )
                  argiAlpha = tbScore;
            }
            else if ( tbWDL == TB_LOSS )
            {
               if ( tbScore <= argiAlpha )
                  return tbScore;
               if ( tbScore < argiBeta )
                  argiBeta = tbScore;
            }
            else // DRAW, CURSED_WIN, or BLESSED_LOSS - all treated as draws
            {
               return tbScore; // Return draw score
            }
         }
      }
   }

   // Sort the moves to take the best bet.
   SortMovesHash( vsiMoveOrder, vsMoveList, iNumberOfMoves, argsBoard,
                  argsGeneralMoves, viMoveScores, hashResult,
                  &threadData->searchParameters );

   // THREAD DIVERSITY: Ordering Offset (Stockfish approach)
   // Each thread searches moves in a different order to maximize parallel
   // efficiency. Thread 0 searches in normal order (0,1,2,...), Thread 1 starts
   // at offset 1 (1,2,3,...,0), Thread 2 starts at offset 2 (2,3,4,...,0,1),
   // etc. This is deterministic, reproducible, and ensures the best move stays
   // near the front.
   int orderingOffset = 0;
   if ( threadData->id > 0 && iNumberOfMoves > 1 )
   {
      orderingOffset = threadData->id % iNumberOfMoves;
   }

   // Loop over all generated moves.
   for ( iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++ )
   {
      if ( GetStopGo() == dGo )
      {
         // Apply ordering offset for thread diversity
         int offsetMoveCount   = ( iMoveCount + orderingOffset ) % iNumberOfMoves;
         int iCurrentMoveIndex = vsiMoveOrder[ offsetMoveCount ];

         // Update the interface as to what move we are searching.
         if ( GetTimeSinceLastMoveUpdate() > dMoveUpdateTime &&
              GetInterfaceMode() == dUCI )
         {
            CreateAlgebraicMove( strMove, vsMoveList, iCurrentMoveIndex );
            sprintf( strUpdate, "info currmove %s currmovenumber %d ", strMove,
                     iMoveCount + 1 );
            SendCommand( strUpdate );
         }

         // The SearchMove function will modify the board, so we operate on a
         // temporary copy. OPTIMIZATION: Removed heap allocation. SearchMove uses
         // Make/Undo pattern.
         int oldAlpha = argiAlpha; // Store alpha before the call

         // Call the new helper function to process the move. Alpha is passed by
         // reference. PVS is already handled inside SearchMove, so we just call it
         // with full window
         int iScore = SearchMove( argsBoard, argsGeneralMoves, vsMoveList,
                                  iCurrentMoveIndex, argiAlpha, argiBeta,
                                  iMoveCount, viMoveScores, iHaveMove, threadData );

         // If the previous best move has been searched, set the flag.
         if ( iCurrentMoveIndex == *argiBestMove )
         {
            *argiBestMoveSearched = 1;
         }

         // Check if a new best move was found (i.e., alpha was updated)
         if ( argiAlpha > oldAlpha )
         {
            // Mark which move is the best so far.
            iBestMoveIndex       = iCurrentMoveIndex;
            argsBoard->iBestMove = iBestMoveIndex;

            // PV is already updated in argsBoard by SearchMove

            // Update the interface with the new score and PV.
            if ( GetInterfaceMode() == dUCI && threadData->id == 0 )
            {
               SetScore( argiAlpha );
               std::stringstream pv_stream;
               int               pvLen = argsBoard->viPrincipalVariationLength[ 0 ];
               if ( pvLen >= dNumberOfPlys )
                  pvLen = dNumberOfPlys - 1;
               for ( int iPVCount = 0; iPVCount <= pvLen; iPVCount++ )
               {
                  if ( argsBoard->vmPrincipalVariation[ 0 ][ iPVCount ].iFromSquare < 0 ||
                       argsBoard->vmPrincipalVariation[ 0 ][ iPVCount ].iToSquare < 0 )
                     break;
                  char strMove[ 64 ];
                  CreateAlgebraicMove(
                      strMove, &argsBoard->vmPrincipalVariation[ 0 ][ iPVCount ], 0 );
                  if ( iPVCount > 0 )
                     pv_stream << " ";
                  pv_stream << strMove;
               }

               SendInfoCommand( argsBoard->iMaxPlys + 1, argsBoard->iMaxPlysReached + 1, argiAlpha,
                                (long long)GetTotalNodes(),
                                (unsigned long)( ( clock() - gsTempus.giTimeStart ) * 1000 / CLOCKS_PER_SEC ),
                                pv_stream.str() );
            }
         }

         // Look for a beta cutoff.
         if ( argiAlpha >= argiBeta )
         {
            break; // Exit the loop
         }
      }
   } // end of loop over moves

   // Take care of the case where we didn't find any legal moves.
   if ( iHaveMove == 0 )
   {
      // Look for check and stale mates
      argiAlpha = LookForCheckAndStale( argsBoard, argsGeneralMoves );
      // Copy the move history into the principal variation.
      CreatePV( argsBoard );
   }

   // Return the score.
   return argiAlpha;
}

//
//
//---------------------------------------------------------------------
//
//
int QuiesenceSearch( struct Board       *argsBoard,
                     struct GeneralMove *argsGeneralMoves, int argiAlpha,
                     int argiBeta, ThreadData *threadData )
{
   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves > 0 );
   assert( CheckBoard( argsBoard ) );
   assert( threadData != nullptr );

   // Increment node counter thread-locally
   threadData->nodesSearched++;

   // Allocations.
   Move *vsMoveList   = threadData->vsMoveList[ argsBoard->iNumberOfPlys + 1 ];
   int  *vsiMoveOrder = threadData->vsiMoveOrder[ argsBoard->iNumberOfPlys + 1 ];
   int  *viMoveScores = threadData->viMoveScore[ argsBoard->iNumberOfPlys + 1 ];
   int   iMoveCount;
   int   iScore;
   int   siHaveMove = 0;
   int   iNumberOfCaptures;

   // Initialize the score.
   iScore = 0;

   // Track how deep QS has gone for qDepth reporting
   if ( argsBoard->iNumberOfPlys > argsBoard->iMaxPlysReached )
   {
      argsBoard->iMaxPlysReached = argsBoard->iNumberOfPlys;
   }

   // Initialize the PV length for this ply to "Empty" (Ply - 1).
   // This ensures that if we Stand Pat, the parent sees we have NO moves.
   argsBoard->viPrincipalVariationLength[ argsBoard->iNumberOfPlys ] =
       argsBoard->iNumberOfPlys - 1;

   // Initialize the child PV length to "Empty" as well (Child Ply - 1 = Ply).
   if ( argsBoard->iNumberOfPlys + 1 < dNumberOfPlys )
   {
      argsBoard->viPrincipalVariationLength[ argsBoard->iNumberOfPlys + 1 ] =
          argsBoard->iNumberOfPlys;
   }

   // Look for a special case.
   if ( argiAlpha == argiBeta && GetAspirationSearch() )
   {

      return argiAlpha;
   }

   // Generate all of the legal moves.

   // Generate all of the legal moves.
   CalculateAttacks( vsMoveList, argsBoard, argsGeneralMoves );

   // Check the state of the board and see if it is legal.
   LegalState( argsBoard, argsGeneralMoves );

   // If the move is not legal, undo the move and continue.
   if ( argsBoard->siLegalMove == 0 )
   {

      // return because we found an illegal move.
      return -dAlpha;
   }

   /*
   cout << "--------------------------------------------" << endl;
   cout << endl << endl << "Board before search." << endl;
   PrintBoard(argsBoard->mBoard);
   */

   // Take a look at the evaluation.
   if ( dUseNNUE == dYes )
   {

      // NOTE: In your Evaluation.cpp, EvaluateBoardDirectNNUE calculates
      // the score from scratch by converting mBoard to a piece list.
      // Therefore, we do not need to manually refresh the accumulator here.
      // If you implement incremental updates later, you will need to ensure
      // the dirty bits are set in MakeMove.

      iScore = EvaluateBoardDirectNNUE( argsBoard, argsGeneralMoves );
   }
   else
   {

      iScore = EvaluateBoard( argsBoard, argsGeneralMoves );
   }
   /*
   cout << "Board after evaluation." << endl;
   PrintBoard(argsBoard->mBoard);
   std::cout << "Please hit any key to continue."; std::cin.get();
   cout << endl;
   */
   // Look for an irrational move.

   if ( iScore >= argiBeta )
   {

      return argiBeta; // , should we return beta here?
   }
   if ( iScore > argiAlpha )
   {

      argiAlpha = iScore;
   }

   // Select only the moves that are captures.
   SortMoves( vsiMoveOrder, vsMoveList, argsBoard->siNumberOfMoves );

   iNumberOfCaptures = argsBoard->siNumberOfMoves;

   // Check and see if we are at a quiet point.
   if ( iNumberOfCaptures == 0 )
   {
      // At leaf: ensure the child PV row the parent will read is marked empty
      int pvIndexParent = argsBoard->iNumberOfPlys; // current row index baseline
      int childRow      = pvIndexParent + 1;        // parent's child row
      if ( childRow < dNumberOfPlys )
      {
         // No further moves: last valid PV index is current ply
         argsBoard->viPrincipalVariationLength[ childRow ] =
             argsBoard->iNumberOfPlys;
      }
      return iScore;
   }

   // Loop over the captures.
   for ( iMoveCount = 0; iMoveCount < iNumberOfCaptures; iMoveCount++ )
   {
      // SEE Pruning: Skip bad captures in Q-Search unless they're promotions
      // Promotions (even with SEE < 0) can fundamentally change the position
      Move &currentMove = vsMoveList[ vsiMoveOrder[ iMoveCount ] ];

      if ( currentMove.iMoveType == dCapture ||
           currentMove.iMoveType == dCaptureAndPromote ||
           currentMove.iMoveType == dEnPassant )
      {
         // Promotions are always searched (they change material significantly)
         if ( currentMove.iPromote == 0 )
         {
            int iSeeValue = See( argsBoard, argsGeneralMoves, &currentMove );

            if ( iSeeValue < 0 )
            {
               continue; // Skip this bad capture
            }
         }
      }

      // Make a move on the board.
      MakeMove( vsMoveList, argsBoard, argsGeneralMoves, vsiMoveOrder[ iMoveCount ] );

      // Update the control of the game.
      Update( argsBoard, argsGeneralMoves, threadData );

      // If we are at depth, find a quiet mind.
      iScore = -QuiesenceSearch( argsBoard, argsGeneralMoves, -argiBeta,
                                 -argiAlpha, threadData );

      // Mark that we have a move at this ply
      if ( argsBoard->siLegalMove == 1 )
      {

         siHaveMove = 1;
      }

      // Undo the move.
      UndoMove( argsBoard, argsGeneralMoves );

      // Do that Alpha Beta thing.
      if ( iScore > argiAlpha )
      {

         // A new winner in the search for the perfect move!
         argiAlpha = iScore;

         // Build PV for QS at current row
         // FIX: MakeMove increments the ply count, so iNumberOfPlys is the child
         // depth. However, we are storing the move for the *current* ply index.
         int pvIndex = argsBoard->iNumberOfPlys;

         // Store the move that caused the improvement (copy fields explicitly)
         const Move &m = vsMoveList[ vsiMoveOrder[ iMoveCount ] ];
         argsBoard->vmPrincipalVariation[ pvIndex ][ pvIndex ].iFromSquare =
             m.iFromSquare;
         argsBoard->vmPrincipalVariation[ pvIndex ][ pvIndex ].iToSquare = m.iToSquare;
         argsBoard->vmPrincipalVariation[ pvIndex ][ pvIndex ].bbFromSquare =
             m.bbFromSquare;
         argsBoard->vmPrincipalVariation[ pvIndex ][ pvIndex ].bbToSquare =
             m.bbToSquare;
         argsBoard->vmPrincipalVariation[ pvIndex ][ pvIndex ].iCapture = m.iCapture;
         argsBoard->vmPrincipalVariation[ pvIndex ][ pvIndex ].iPiece   = m.iPiece;
         argsBoard->vmPrincipalVariation[ pvIndex ][ pvIndex ].bbEPSquare =
             m.bbEPSquare;
         argsBoard->vmPrincipalVariation[ pvIndex ][ pvIndex ].bbCastle  = m.bbCastle;
         argsBoard->vmPrincipalVariation[ pvIndex ][ pvIndex ].iMoveType = m.iMoveType;
         argsBoard->vmPrincipalVariation[ pvIndex ][ pvIndex ].iPromote  = m.iPromote;
         argsBoard->vmPrincipalVariation[ pvIndex ][ pvIndex ].iScore    = m.iScore;

         // Child PV row is pvIndex+1. If it's not initialized (leaf), fall back to
         // empty tail.
         // Child PV row is pvIndex+1.
         int childPVLen = argsBoard->viPrincipalVariationLength[ pvIndex + 1 ];

         // If childPVLen is exactly 'pvIndex', it means the child (pvIndex+1) is Empty.
         // (Because Child - 1 = pvIndex + 1 - 1 = pvIndex).
         // In this case, our PV ends at 'pvIndex' (the current move).
         if ( childPVLen < pvIndex )
            childPVLen = pvIndex;

         // Update our length to match the tail (or just our move if tail is empty)
         argsBoard->viPrincipalVariationLength[ pvIndex ] = childPVLen;

         // Copy child's PV tail
         for ( int k = pvIndex + 1; k <= childPVLen; ++k )
         {
            argsBoard->vmPrincipalVariation[ pvIndex ][ k ] =
                argsBoard->vmPrincipalVariation[ pvIndex + 1 ][ k ];
         }

         // Look for a cute off.
         if ( iScore >= argiBeta )
         {

            // No use searching further.
            return argiBeta; // Should we return beta here?
         }
      }
   }

   // Return the score.
   return argiAlpha;
}

//
//
//---------------------------------------------------------------------
//
//
int LookForSearchExtensions( struct Board       *argsBoard,
                             struct GeneralMove *argsGeneralMoves,
                             struct Move *argvsMoveList, int iMoveNumber,
                             int iNewCheck )
{

   // This function is called for both checking for if a null move is appropriate
   // and to see if the search needs to be extended. For the null search, the
   // first look at check is appropriate. for the extension, the additional
   // calculation for check is required. iNewCheck is a variable for deciding on
   // whether or not to fun the additional check.

   // Store some local attacks
   Move vsMoveList[ dNumberOfMoves ]; // stored locally due to the complexities of
                                      // the recursive search

   if ( argsBoard->bbCheck & 1 || argsBoard->bbCheck & 2 )
   {

      return 1;
   }

   // Do a basic check to see if the side to move is in check.
   int iSearchDeeper = 0;

   // The look for an appropriate move number.  The null move routine will pass
   // in -1.
   if ( iMoveNumber >= 0 )
   {

      // Look for a passed pawn.  search deeper, note, this should catch pawn
      // races.
      if ( argvsMoveList[ iMoveNumber ].iPiece == dWhitePawn )
      {

         // Calculate a passed pawn
         int iEndSquare = argvsMoveList[ iMoveNumber ].iToSquare;
         if ( !( argsGeneralMoves->vbbWhitePPWideVector[ iEndSquare ] &
                 argsBoard->bbBlackPawn ) )
         {
         }
      }
      if ( argvsMoveList[ iMoveNumber ].iPiece == dBlackPawn )
      {

         // Calculate a passed pawn
         int iEndSquare = argvsMoveList[ iMoveNumber ].iToSquare;
         if ( !( argsGeneralMoves->vbbBlackPPWideVector[ iEndSquare ] &
                 argsBoard->bbWhitePawn ) )
         {

            return 1; // A hack for testing
         }
      }

   } // move number

   // See if we need to check for an uncalculated check.
   if ( iNewCheck == 1 )
   {

      // Generate all of the legal attacks.
      CalculateAttacks( vsMoveList, argsBoard, argsGeneralMoves );

      // If we are in check for any reason search deeper
      if ( argsBoard->bbCheck & 1 || argsBoard->bbCheck & 2 )
      {

         return 1;
      }
   }

   return iSearchDeeper;
}

//
//
//---------------------------------------------------------------------
//
//
// See if we should do the null search - IMPROVED VERSION
int DoNullSearch( struct Board *argsBoard, struct GeneralMove *argsGeneralMoves,
                  int argiAlpha, int argiBeta, int &argiScore, Move *vsMoveList,
                  ThreadData *threadData )
{

   int iDoSearch = 0;
   int iMoveNumber =
       -1; // This is used to more efficiently use LookForSearchExtensions.
   int iCheck =
       0; // This is used to more efficiently use LookForSearchExtensions;
   int iQScore = 0;

   // Basic depth check - ensure we have enough depth left after reduction
   int iRemainingDepth = argsBoard->iMaxPlys - argsBoard->iNumberOfPlys;
   if ( iRemainingDepth <= GetNullReduction() )
   {
      return 0; // Not enough depth for null move
   }

   // Don't do null move if:
   // 1. Not enough depth remaining
   // 2. Already in a PV node (beta is too close to mate)
   // 3. Last move was null (prevents double null move)
   // 4. We're in check (can't pass when in check)
   if ( ( iRemainingDepth > GetNullReduction() ) && ( argiBeta < -dMate ) &&
        ( argsBoard->iLastMoveNull == dNo ) && ( argsBoard->iMoveOrder != 0 ) )

   {
      // Check if we're in check - null move is illegal when in check
      if ( LookForCheck( argsBoard ) )
      {
         return 0; // Can't do null move when in check
      }

      // See if pieces other than just the king and pawns are on the board for the
      // side to move. This is done to avoid zugzwang.
      bool hasMajorOrMinorPieces = false;
      if ( argsBoard->siColorToMove == dWhite )
      {
         if ( argsBoard->bbWhiteRook | argsBoard->bbWhiteKnight |
              argsBoard->bbWhiteBishop | argsBoard->bbWhiteQueen )
         {
            hasMajorOrMinorPieces = true;
         }
      }
      else // Black to move
      {
         if ( argsBoard->bbBlackRook | argsBoard->bbBlackKnight |
              argsBoard->bbBlackBishop | argsBoard->bbBlackQueen )
         {
            hasMajorOrMinorPieces = true;
         }
      }

      if ( !hasMajorOrMinorPieces )
      {
         // In a king-and-pawn endgame for the side to move, don't do a null
         // search.
         return 0;
      }

      // Give it a try.
      return 1;
   }
   else
   {
      // Do not do the null search.
      return 0;
   }

   // Do not do the null search.
   // This point should not be reached.  Just some cheap insurance.
   return 0;
}

//
//
//---------------------------------------------------------------------
//
//
int LookForCheck( struct Board *argsBoard )
{
   // Do a basic check to see if the side to move is in check.

   int iInCheck =
       ( ( ( argsBoard->bbCheck & 1 ) && // Make sure we are not in check.
           ( argsBoard->siColorToMove == dWhite ) ) ||
         ( ( argsBoard->bbCheck & 2 ) && ( argsBoard->siColorToMove == dBlack ) ) );

   return iInCheck;
}

//
//
//---------------------------------------------------------------------
//
//
// Helper function: Get history score for a move to adjust LMR
inline int GetHistoryScoreForLMR( struct Move *argsMove, SearchParameters *argsSearchParameters )
{
   if ( !argsMove )
   {
      return 0;
   }

   int from = argsMove->iFromSquare;
   int to   = argsMove->iToSquare;

   if ( from < 0 || from >= 64 || to < 0 || to >= 64 )
   {
      return 0;
   }

   // Access the shared history heuristic (color-specific)
   // Note: History is typically indexed by piece color, but we'll use a simplified approach
   // Just return White heuristic for now (most engines use unified history)
   return gSharedHistory.mWhiteHistoryHeuristic[ from ][ to ].load();
}

//
//
//---------------------------------------------------------------------
//
//
// See if we should to a LMR (Late Move Reduction)
// Enhanced with logarithmic formula, history integration, killer/PV exclusions
int DoLMRSearch( int iMoveCount, struct Board *argsBoard,
                 struct GeneralMove *argsGeneralMoves, int argiAlpha, int argiBeta,
                 int iDepth, struct Move *argsMove, SearchParameters *argsSearchParameters )
{

   // === Enhanced LMR Safety Checks ===
   // Do not reduce if:

   // 1. Depth is too low
   if ( iDepth < 3 )
   {
      return 0;
   }

   // 2. First move (always search at full depth)
   if ( iMoveCount < 1 )
   {
      return 0;
   }

   // 3. Tactical moves (capture, promotion, castle)
   if ( argsMove->iCapture != 0 || argsMove->iPromote != 0 ||
        ( argsMove->iMoveType & dCastle ) )
   {
      return 0;
   }

   // 4. Do not reduce Pawn Pushes
   // Pawn moves fundamentally change the position structure and are often
   // the only way to make progress in closed positions (e.g., pawn breaks).
   // Reducing them causes tactical blindness in locked endgames.
   if ( argsMove->iPiece == dWhitePawn || argsMove->iPiece == dBlackPawn )
   {
      return 0;
   }

   // 5. Move gives check
   int iKingSquare = -1;
   if ( argsBoard->siColorToMove == dWhite )
   {
      Find( argsBoard->bbWhiteKing, &iKingSquare, argsGeneralMoves );
      if ( IsSquareAttacked( argsBoard, argsGeneralMoves, iKingSquare, dBlack ) )
      {
         return 0;
      }
   }
   else
   {
      Find( argsBoard->bbBlackKing, &iKingSquare, argsGeneralMoves );
      if ( IsSquareAttacked( argsBoard, argsGeneralMoves, iKingSquare, dWhite ) )
      {
         return 0;
      }
   }

   // NEW: 6. Killer moves (caused recent beta cutoffs)
   bool isKiller = false;
   int  ply      = argsBoard->iNumberOfPlys;
   if ( ply >= 0 && ply < dNumberOfPlys )
   {
      for ( int i = 0; i < 2; i++ )
      {
         if ( argsMove->iFromSquare == argsSearchParameters->mKillerMoves[ ply ][ i ].iFromSquare &&
              argsMove->iToSquare == argsSearchParameters->mKillerMoves[ ply ][ i ].iToSquare )
         {
            isKiller = true;
            break;
         }
      }
   }
   if ( isKiller )
   {
      return 0;
   }

   // NEW: 7. PV node (wide alpha-beta window suggests principal variation)
   bool isPVNode = ( argiBeta - argiAlpha > 1 );
   if ( isPVNode && iMoveCount == 0 )
   {
      return 0;
   }

   // NEW: 8. Mate threats (close to mate score)
   if ( abs( argiAlpha ) > 9000 )
   {
      return 0;
   }

   // 9. LMR disabled globally
   if ( GetUseLMR() == dNo )
   {
      return 0;
   }

   // === Calculate Enhanced LMR Reduction ===

   // Use logarithmic table: R = ln(depth) * ln(move) / LMR_BASE
   int safeDepth = ( iDepth < 64 ) ? iDepth : 63;
   int safeMove  = ( iMoveCount < 64 ) ? iMoveCount : 63;

   int baseReduction = gLMRTable[ safeDepth ][ safeMove ];

   // NEW: History heuristic adjustment
   int historyScore      = GetHistoryScoreForLMR( argsMove, argsSearchParameters );
   int historyAdjustment = 0;

   if ( historyScore > HISTORY_GOOD_THRESHOLD )
   {
      historyAdjustment = -1; // Reduce less for good history
   }
   else if ( historyScore < HISTORY_BAD_THRESHOLD )
   {
      historyAdjustment = +1; // Reduce more for bad history
   }

   int finalReduction = baseReduction + historyAdjustment;

   // Clamp reduction
   if ( finalReduction < 0 )
   {
      finalReduction = 0;
   }

   // Don't reduce below depth 1
   if ( iDepth - 1 - finalReduction < 1 )
   {
      finalReduction = iDepth - 2;
      if ( finalReduction < 0 )
      {
         finalReduction = 0;
      }
   }

   return finalReduction;
}

//
//
//---------------------------------------------------------------------
//
//
// Look for check and stale mates
int LookForCheckAndStale( struct Board       *argsBoard,
                          struct GeneralMove *argsGeneralMoves )
{

   // Define some variables
   int iScore = 0;

   // Calculate the attacks and see if we are in check.
   Move vsMoveListCheck[ dNumberOfMoves ]; // stored locally due to the
                                           // complexities of the recursive search

   // Switch the color to move.
   if ( argsBoard->siColorToMove == dWhite )
   {

      argsBoard->siColorToMove = dBlack;
   }
   else
   {

      argsBoard->siColorToMove = dWhite;
   }

   // Generate all of the legal moves.
   CalculateAttacks( vsMoveListCheck, argsBoard, argsGeneralMoves );

   // Switch the color to move.
   if ( argsBoard->siColorToMove == dWhite )
   {

      argsBoard->siColorToMove = dBlack;
   }
   else
   {

      argsBoard->siColorToMove = dWhite;
   }

   // Switch between checkmate and stalemate.
   if ( argsBoard->siColorToMove == dWhite &&
        ( argsBoard->bbBlackAttack & argsBoard->bbWhiteKing ) )
   {

      // Set the score.
      iScore = dMate;

      // Set that white is in check mate.
      argsBoard->bbCheck = SetBitToOne( argsBoard->bbCheck, 2 );

      return iScore;
   }
   else if ( argsBoard->siColorToMove == dBlack &&
             ( argsBoard->bbWhiteAttack & argsBoard->bbBlackKing ) )
   {

      // Set the score
      iScore = dMate;

      // Set that white is in check mate.
      argsBoard->bbCheck = SetBitToOne( argsBoard->bbCheck, 3 );

      return iScore;
   }
   else if ( argsBoard->siColorToMove == dWhite )
   {

      // We have found stalemate.
      iScore = 0;

      // Set that white is in check mate.
      argsBoard->bbCheck = SetBitToOne( argsBoard->bbCheck, 4 );

      return iScore;
   }
   else if ( argsBoard->siColorToMove == dBlack )
   {

      // We have found stalemate.
      iScore = 0;

      // Set that white is in check mate.
      argsBoard->bbCheck = SetBitToOne( argsBoard->bbCheck, 5 );

      return iScore;
   }

   // Return the score.
   return iScore;
}

// Initialize the search parameters.
void InitializeSearch()
{
   // Initialize enhanced LMR table with logarithmic formula
   InitializeLMRTable();

   // Allocate heap memory for move lists
   // Initialize main thread data if not already done
   if ( gMainThreadData == nullptr )
   {
      gMainThreadData     = new ThreadData();
      gMainThreadData->id = 0;
   }

   // Ensure gThreads has the main thread
   if ( gThreads.empty() )
   {
      // We can't easily add main thread to gThreads here without circular deps or
      // logic change But we can iterate gThreads if it exists.
   }

   // Helper lambda to initialize a single SearchParameters struct
   auto InitParams = []( SearchParameters &params )
   {
      // Initialize the depth reduction for LMR
      for ( int iDepth = 0; iDepth < dNumberOfPlys; iDepth++ )
      {
         for ( int iMove = 0; iMove < dNumberOfMoves; iMove++ )
         {
            // Set the LMR reduction parameters
            params.miSearchReduction[ iDepth ][ iMove ] =
                int( 0.33 + log( double( iDepth + 1 ) ) * log( double( iMove + 1 ) ) / 2.25 );
         }
      }

      ResetHistoryHeuristic( &params );
      ResetKillerMoves( &params );

      params.iTried  = 0;
      params.iFailed = 0;
      params.iZug    = 0;

      // Set up the null pruning parameters
      params.iPruningSchedule[ 0 ] = dForwardPrune0;
      params.iPruningSchedule[ 1 ] = dForwardPrune1;
      params.iPruningSchedule[ 2 ] = dForwardPrune2;
      params.iPruningSchedule[ 3 ] = dForwardPrune3;
      params.iPruningSchedule[ 4 ] = dForwardPrune4;
      params.iPruningSchedule[ 5 ] = dForwardPrune5;
      params.iPruningSchedule[ 6 ] = dForwardPrune6;
      params.iPruningSchedule[ 7 ] = dForwardPrune7;
      params.iPruningSchedule[ 8 ] = dForwardPrune8;
      params.iPruningSchedule[ 9 ] = dForwardPrune9;
   };

   // Initialize for Main Thread
   InitParams( gMainThreadData->searchParameters );

   // Initialize for Helper Threads
   for ( auto *thread : gThreads )
   {
      if ( thread )
      {
         InitParams( thread->GetData()->searchParameters );
      }
   }

   // Initialize countermove table
   for ( int from = 0; from < 64; from++ )
   {
      for ( int to = 0; to < 64; to++ )
      {
         gCounterMoves[ from ][ to ].iFromSquare = -1;
         gCounterMoves[ from ][ to ].iToSquare   = -1;
      }
   }

   // Reset the Tempus parameters
   SetTempusParameters();

   // Initialize the multi PV
   InitializeMultiPV();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void Update( struct Board *argsBoard, struct GeneralMove *argsGeneralMoves,
             ThreadData *threadData )
{
   // Do the time control yada yada.

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Update the number of nodes counted.
   {
      // Use thread-local counter
      threadData->nodesSearched++;

      // Only main thread updates global polling count and checks time/input
      if ( threadData->id == 0 )
      {
         IncrementPollingCount();

         int iStartTime  = GetSearchStartTime();
         int iClock      = clock();
         int iSearchTime = GetSearchTimeInMiliSeconds();

         // Do some search time control.
         if ( GetSearchTimeInMiliSeconds() < ( clock() - GetSearchStartTime() ) )
         {
            SetStopGo( dStop );
         }

         // See if there is input from the GUI
         if ( GetInterfaceMode() == dUCI && GetPollingCount() >= dPollingCount )
         {
            // Reset the polling count.
            SetPollingCount( 0 );

            // Poll for input.
            if ( CheckForInput() )
            {
               // See if we can process the command now.
               ReadInputAndExecute( argsBoard, argsGeneralMoves );
            }
         }
      }

      // Look for a stop by number of notes
      if ( GetNodes() > 0 )
      {
         // If the number of search notes is larger than the node count, then bail
         // out. if (GetNodes() < bbNumberOfNodes)
         {
            // Set the stop.
            SetStopGo( dStop );
         }
      }

      // If at an incremental amount of nodes, let the folks at home know what is
      // going on. if (bbNumberOfIncrementalNodes == GetNodeCheck())
      {
         // Calculate the nodes per second.
         SetEnd( clock() );
         double duration = (double)( GetEnd() - GetStart() ) / CLOCKS_PER_SEC;
         // double rate = (double)(bbNumberOfIncrementalNodes) / duration;
         SetStart( clock() );

         if ( GetInterfaceMode() == dConsole )
         {
            // cout << "node Count = " << bbNumberOfIncrementalNodes << " nodes/sec
            // = " << (int)rate << endl;
         }
         if ( GetInterfaceMode() == dUCI && GetStopGo() == dGo )
         {
            // Send an update to the interface.
            char strUpdate[ 640 ];
            int  iNodes = (int)( GetNumberOfNodes() );
            // snprintf(strUpdate, sizeof(strUpdate),
            //    "info nodes %d nps %i ", // Changed "rate" to "nps" for UCI
            //    standard iNodes, (int)rate);
            // SendCommand(strUpdate);
         }
         SetNumberOfIncrementalNodes( 0 );
      }
   } // End critical block.
}

//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void SearchMonitor( struct Board *argsBoard, struct Move *argvsMoves,
                    struct GeneralMove *argsGeneralMoves )
{

   PrintBoard( argsBoard->mBoard );

   if ( argsBoard->mBoard[ dA6 ] == dWhiteBishop )
   {
      cout << "Here's the rub." << endl;
   }

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );
}

//
//
//---------------------------------------------------------------------
//
//
void CreatePV( struct Board *argsBoard )
{

   // Warning: Do not use in the Q search.

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( CheckBoard( argsBoard ) );

   // Allocate an index for looping over the plys of the search.
   int iPlyIndex = argsBoard->iNumberOfPlys + 1;

   // See if we are at maximum depth.
   if ( iPlyIndex > argsBoard->iMaxPlysReached )
   {

      argsBoard->iMaxPlysReached = iPlyIndex;
   }

   // Create an index that points to the first move of the PV.
   int iGamePly = argsBoard->iMoveHistory + 1;

   // Assign the move.
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].iFromSquare =
       argsBoard->sHistoryStack[ iGamePly ].iFromSquare;
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].iToSquare =
       argsBoard->sHistoryStack[ iGamePly ].iToSquare;
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].bbFromSquare =
       argsBoard->sHistoryStack[ iGamePly ].bbFromSquare;
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].bbToSquare =
       argsBoard->sHistoryStack[ iGamePly ].bbToSquare;
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].iCapture =
       argsBoard->sHistoryStack[ iGamePly ].iCapture;
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].iPiece =
       argsBoard->sHistoryStack[ iGamePly ].iPiece;
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].bbEPSquare =
       argsBoard->sHistoryStack[ iGamePly ].bbEPSquare;
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].bbCastle =
       argsBoard->sHistoryStack[ iGamePly ].bbCastle;
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].iMoveType =
       argsBoard->sHistoryStack[ iGamePly ].iMoveType;

   // Update the PV matrix - copy the child's PV.
   int iChildPVLength = argsBoard->viPrincipalVariationLength[ iPlyIndex + 1 ];
   for ( int iPlyLoop = iPlyIndex + 1; iPlyLoop <= iChildPVLength; iPlyLoop++ )
   {

      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].iFromSquare =
          argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].iFromSquare;
      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].iToSquare =
          argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].iToSquare;
      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].bbFromSquare =
          argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].bbFromSquare;
      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].bbToSquare =
          argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].bbToSquare;
      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].iCapture =
          argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].iCapture;
      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].iPiece =
          argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].iPiece;
      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].bbEPSquare =
          argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].bbEPSquare;
      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].bbCastle =
          argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].bbCastle;
      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].iMoveType =
          argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].iMoveType;
   }

   // Update the principal variation length.
   // The child's PV length tells us how far the PV extends, and we preserve
   // that.
   argsBoard->viPrincipalVariationLength[ iPlyIndex ] =
       argsBoard->viPrincipalVariationLength[ iPlyIndex + 1 ];
}

void PrintPrincipalVariation( struct Board       *argsBoard,
                              struct GeneralMove *argsGeneralMoves )
{
   // Print out the principal variation.

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves > 0 );

   int  iPlyIndex;
   char strMove[ 10 ];
   int  iNumberOfCharacters;
   int  iCount;

   // Initialize some parameters
   iCount = -1;

   // Rest the control if needed.
   SetStopGo( dGo );

   // Keep an old board around for scoring.
   struct Board *sBoard;
   sBoard = new Board();

   // Create a new board for a shallow search.
   memcpy( sBoard, argsBoard, sizeof( Board ) );

   if ( sBoard->bbCheck & 16 || sBoard->bbCheck & 32 )
   {

      cout << "Stalemate or draw." << endl
           << endl;
      delete sBoard;
      return;
   }

   // Reset the board.
   memcpy( sBoard, argsBoard, sizeof( Board ) );

   // Loop over the forward plys based on actual PV length.
   int pvLen = argsBoard->viPrincipalVariationLength[ 0 ];
   if ( pvLen >= dNumberOfPlys )
      pvLen = dNumberOfPlys - 1; // clamp to array size
   for ( iPlyIndex = 0; iPlyIndex <= pvLen; iPlyIndex++ )
   {

      // stop if invalid entry
      if ( argsBoard->vmPrincipalVariation[ 0 ][ iPlyIndex ].iFromSquare < 0 ||
           argsBoard->vmPrincipalVariation[ 0 ][ iPlyIndex ].iToSquare < 0 )
         break;

      // Note this counter is needed incase a check mate is found.  Then we will
      // jump out.
      iCount++;

      // At this time, because of the hash table, the structure sBoard may not
      // have a full PV (because if it is being scored from a hash) an early cut
      // off will not allow it to store the PV This is a hack to get out of the
      // loop if the data is bad.
      if ( ( sBoard->vmPrincipalVariation[ 0 ][ iPlyIndex ].iFromSquare < 0 ) ||
           ( sBoard->vmPrincipalVariation[ 0 ][ iPlyIndex ].iToSquare < 0 ) ||
           ( sBoard->vmPrincipalVariation[ 0 ][ iPlyIndex ].iFromSquare > 64 ) ||
           ( sBoard->vmPrincipalVariation[ 0 ][ iPlyIndex ].iToSquare > 64 ) )
      {

         // If we have a bad square, jump out of the loop.
         cout << endl
              << endl;
         delete sBoard;
         return;
      }

      // Print out a move.
      //* sBoard = * argsBoard;
      iNumberOfCharacters =
          PrintMove( sBoard, argsGeneralMoves,
                     &sBoard->vmPrincipalVariation[ 0 ][ iPlyIndex ], strMove );
      cout << " ";

      for ( int iCharCount = 0; iCharCount < iNumberOfCharacters; iCharCount++ )
      {

         cout << strMove[ iCharCount ];
      }

      // Make the move on the board.
      // Note that PrintMove() requires the board be at the appropriate state.
      MakeMove( &sBoard->vmPrincipalVariation[ 0 ][ iPlyIndex ], sBoard,
                argsGeneralMoves, 0 );

      // If checkmate is found, bail out of the loop.
      if ( !strncmp( &strMove[ iNumberOfCharacters - 1 ], "#", 1 ) )
      {

         break;
      }

      // Look for the special cases of checkmate and stalemate.
      if ( sBoard->bbCheck & 16 || sBoard->bbCheck & 32 )
      {

         cout << "  Stalemate." << endl;
         break;
      }
   }

   // Unwind the moves : this is critical for keep the hash correct.
   ///*
   for ( iPlyIndex = 0; iPlyIndex < iCount + 1; iPlyIndex++ )
   {

      // Undo the moves.
      UndoMove( sBoard, argsGeneralMoves );
   }
   //*/
   cout << endl
        << endl;

   // Free the memory.
   delete sBoard;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// This function encapsulates the logic for searching a single move. It is
// called from within the main loops of FirstSearch() and Search() to reduce
// code duplication.
//
// It handles:
// - Making and undoing the move.
// - Deciding between a full search, LMR search, or quiescence search.
// - Performing the recursive search call.
// - Storing results in the hash table.
// - Updating alpha and associated heuristics (PV, Killer, History).
//
int SearchMove( Board *argsBoard, GeneralMove *argsGeneralMoves,
                Move *vsMoveList, int iMoveIndex, int &argiAlpha, int argiBeta,
                int iMoveCount, int *viMoveScores, int &siHaveMove,
                ThreadData *threadData )
{
   int iScore = 0;

   // Make the move (board now at child ply)
   MakeMove( vsMoveList, argsBoard, argsGeneralMoves, iMoveIndex );
   // Evaluate deeper or QSearch
   if ( LookForDraw( argsBoard, argsGeneralMoves ) )
   {
      iScore = GetValueOfDraw();
   }
   else if ( argsBoard->iNumberOfPlys < argsBoard->iMaxPlys - 1 )
   {
      int iRemainingDepth = argsBoard->iMaxPlys - argsBoard->iNumberOfPlys;

      int iReduction = DoLMRSearch(
          iMoveCount, argsBoard, argsGeneralMoves, argiAlpha, argiBeta, iRemainingDepth,
          &vsMoveList[ iMoveIndex ], &threadData->searchParameters );

      if ( iReduction > 0 )
      {
         // Perform reduced search
         int savedMax = argsBoard->iMaxPlys;
         argsBoard->iMaxPlys =
             argsBoard->iNumberOfPlys + iRemainingDepth - 1 - iReduction;

         // Ensure we don't reduce to 0 depth (though DoLMRSearch should handle
         // this)
         if ( argsBoard->iMaxPlys <= argsBoard->iNumberOfPlys )
            argsBoard->iMaxPlys = argsBoard->iNumberOfPlys + 1;

         iScore = -Search( argsBoard, argsGeneralMoves, -argiAlpha - 1, -argiAlpha,
                           threadData );

         argsBoard->iMaxPlys = savedMax; // Restore depth

         // If the reduced search improved alpha, we need to re-search at full
         // depth
         if ( iScore > argiAlpha )
         {
            iScore = -Search( argsBoard, argsGeneralMoves, -argiBeta, -argiAlpha,
                              threadData );
         }
      }
      else
      {
         // Normal search (no LMR)
         if ( iMoveCount > 0 )
         {
            iScore = -Search( argsBoard, argsGeneralMoves, -argiAlpha - 1,
                              -argiAlpha, threadData );
            if ( iScore > argiAlpha && iScore < argiBeta )
            {
               iScore = -Search( argsBoard, argsGeneralMoves, -argiBeta, -argiAlpha,
                                 threadData );
            }
         }
         else
         {
            iScore = -Search( argsBoard, argsGeneralMoves, -argiBeta, -argiAlpha,
                              threadData );
         }
      }
   }
   else
   {
      iScore = -QuiesenceSearch( argsBoard, argsGeneralMoves, -argiBeta,
                                 -argiAlpha, threadData );
   }
   if ( argsBoard->siLegalMove == 1 )
   {
      siHaveMove = 1;
   }

   // Alpha improvement: build PV BEFORE undo
   if ( iScore > argiAlpha )
   {
      argiAlpha = iScore;
#if defined( dUseHash )
      if ( GetUseHashTable() )
      {
         UpdateHH( argsBoard, &vsMoveList[ iMoveIndex ],
                   &threadData->searchParameters );
      }
#endif
      // Assemble PV at current ply (child becomes row = iNumberOfPlys)
      int row =
          argsBoard->iNumberOfPlys; // after MakeMove, root first move has row 0
      // Copy current move into PV row/column [row][row]
      argsBoard->vmPrincipalVariation[ row ][ row ] = vsMoveList[ iMoveIndex ];
      // Copy tail from child row (row+1)
      int childRow = row + 1;
      int childLen = ( childRow < dNumberOfPlys )
                         ? argsBoard->viPrincipalVariationLength[ childRow ]
                         : row;
      if ( childLen < row )
         childLen = row; // ensure non-decreasing
      for ( int k = row + 1; k <= childLen; ++k )
      {
         argsBoard->vmPrincipalVariation[ row ][ k ] =
             argsBoard->vmPrincipalVariation[ childRow ][ k ];
      }
      argsBoard->viPrincipalVariationLength[ row ] = childLen;
      if ( row == 0 )
      {
         argsBoard->iBestMove = iMoveIndex;
      }
   }

   // Undo move AFTER PV assembly
   UndoMove( argsBoard, argsGeneralMoves );
   if ( argsBoard->siLegalMove == 0 )
   {
      return argiAlpha;
   }

#if defined( dUseHash )
   if ( GetUseHashTable() == 1 )
   {
      InputToHashTable( argsBoard, argsGeneralMoves, argiAlpha, argiBeta, iScore,
                        &vsMoveList[ iMoveIndex ] );
   }
#endif
   return iScore;
}

//
//
//---------------------------------------------------------------------
//
//

int Search( Board *argsBoard, GeneralMove *argsGeneralMoves, int argiAlpha,
            int argiBeta, ThreadData *threadData, int iExcludedMove )
{
   // Debug print (limited)
   // if (threadData && threadData->id > 0) std::cout << "Thread " <<
   // threadData->id << " Search depth " << argsBoard->iMaxPlys << std::endl;

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves > 0 );
   assert( CheckBoard( argsBoard ) );
   assert( threadData != nullptr );

   // STRICT PV PLAN: Define what a PV node is
   bool bIsPV = ( argiBeta - argiAlpha > 1 );

   // Increment node counter thread-locally
   threadData->nodesSearched++;

   // Safety check for ply overflow
   if ( argsBoard->iNumberOfPlys >= dNumberOfPlys - 2 )
   {
      return EvaluateBoard( argsBoard, argsGeneralMoves );
   }

   Move *vsMoveList   = threadData->vsMoveList[ argsBoard->iNumberOfPlys + 1 ];
   int  *vsiMoveOrder = threadData->vsiMoveOrder[ argsBoard->iNumberOfPlys + 1 ];
   int  *viMoveScores = threadData->viMoveScore[ argsBoard->iNumberOfPlys + 1 ];
   int   iMoveCount;
   int   iScore;
   int   siHaveMove        = 0;
   int   iBestMove         = 128;
   int   iNullScore        = 0;
   int   iLMRSearch        = 0;
   int   iSearchExtensions = 0;
   int   iNewCheck         = 1;
   int   iVerifiedSearch   = 0;
   int   iMaxPlysOld       = 0;
   int   oldHash           = 0;
   int   iRemainingDepth   = 0;
   int   iQSearch          = 0;
   int   iSearchFlag       = 1;
   int   iNumberOfAttacks  = 0;
   int   iNumberOfQuiets   = 0;
   int   iNumberOfMoves;
   int   iAttacksSearchFlag = 0;
   int   iZeroAttacks       = 0;

   iScore      = 0;
   iMaxPlysOld = argsBoard->iMaxPlys;

   // Initialize the PV length for this ply to prevent garbage data from child
   // nodes if no move improves alpha. Initialize the PV length for this ply to
   // prevent garbage data from child nodes if no move improves alpha.
   argsBoard->viPrincipalVariationLength[ argsBoard->iNumberOfPlys + 1 ] =
       argsBoard->iNumberOfPlys;

   if ( argiAlpha >= -dMate || argiBeta <= dMate )
   {
      return -dMate;
   }

#if defined( dUseHash )
   HashQueryResult hashResult =
       ExtractFromHashTable( argsBoard, argsGeneralMoves );
   // Only attempt a cutoff if we successfully retrieved data
   if ( hashResult.iQueryState == 1 )
   {
      // STRICT PV IMPLEMENTATION
      // If this is a PV Node (bIsPV is true), we MUST NOT return a score from the
      // hash table. We must search the moves to ensure the Principal Variation
      // array is populated correctly.
      // We ONLY return if this is NOT a PV node.
      if ( !bIsPV )
      {
         int hashScore = hashResult.iScore;

         // Fail Low (Upper Bound)
         if ( hashScore <= argiAlpha )
         {
            return hashScore;
         }

         // Fail High (Lower Bound)
         if ( hashScore >= argiBeta )
         {
            return hashScore;
         }
      }
      // If bIsPV is true, we fall through here.
      // We effectively ignore the score for cutoff purposes,
      // BUT we still hold hashResult.iBestMove which SortMovesHash will use
      // to order the best move first.
   }
#else
   HashQueryResult hashResult;
   hashResult.iQueryState = 0;
   hashResult.iBestMove   = 128;
#endif

   // Singular Extension Logic
   int iExtension = 0;

   // Calculate current depth once
   int iCurrentDepth = argsBoard->iMaxPlys - argsBoard->iNumberOfPlys;

   // Only attempt SE if:
   // 1. This is a PV node (best line of play)
   // 2. Depth is sufficient
   // 3. Valid hash move exists
   // 4. Not already in a mating line
   // 5. Not in a verification search (iExcludedMove == -1)
   if ( bIsPV &&
        iCurrentDepth >= dSingularMinDepth &&
        hashResult.iQueryState == 1 &&
        hashResult.iBestMove != 128 &&
        abs( hashResult.iScore ) < 29000 &&
        iExcludedMove == -1 ) // Don't do SE during SE verification
   {
      int iHashMove  = hashResult.iBestMove;
      int iHashScore = hashResult.iScore;

      // FIXED: Calculate singular beta with proper margin
      // The margin determines how much better the hash move must be vs alternatives
      // to be considered "singular" (uniquely good).
      //
      // Old bug: dSingularMargin=1, depth=10 => margin=10cp (way too narrow!)
      // This caused almost every move to appear singular, extending everything.
      //
      // Fix: Use a much wider margin. Standard approaches:
      // - Flat margin: 200cp (2 pawns) - simple and effective
      // - Scaled margin: depth * 20cp - still reasonable at depth 10 (200cp)
      //
      // Using flat 400cp for extreme safety against node explosion
      // We only want to extend if the move preserves a win while others LOSE.
      int iCalculatedMargin = 400;

      // Alternative scaled approach (commented out):
      // int iCalculatedMargin = dSingularMargin * 20 * iCurrentDepth;

      int iSingularBeta = iHashScore - iCalculatedMargin;

      // FIXED: Ensure reduced depth is exactly half
      // This prevents the verification search from being too expensive
      int iReducedDepth = iCurrentDepth / 2;

      // Safety: Ensure we don't reduce to 0 or negative depth
      if ( iReducedDepth < 1 )
         iReducedDepth = 1;

      // Save current depth
      int iSavedMaxPlys = argsBoard->iMaxPlys;

      // Set reduced search depth
      argsBoard->iMaxPlys = argsBoard->iNumberOfPlys + iReducedDepth;

      // Perform verification search with null window, excluding hash move
      // This searches all moves EXCEPT the hash move to see if any can reach singularBeta
      int iScoreSE = Search( argsBoard, argsGeneralMoves,
                             iSingularBeta - 1, iSingularBeta,
                             threadData, iHashMove );

      // Restore original depth
      argsBoard->iMaxPlys = iSavedMaxPlys;

      // If verification search failed low, the hash move is singular
      // (no other move could reach the threshold)
      if ( iScoreSE < iSingularBeta )
      {
         iExtension = 1;

         // FIXED: Double extension only for EXTREMELY singular moves
         // Be very conservative here - double extensions explode tree size
         // Only extend by 2 if:
         // 1. Not at PV node (PV already gets lots of attention)
         // 2. Alternatives fail REALLY low (100cp+ below singular beta)
         // This catches forced mate scenarios without exploding the tree
         if ( !bIsPV && iScoreSE < iSingularBeta - 100 )
         {
            // Disabled for now - too risky
            // iExtension = 2;
         }
      }
   }

   // [Syzygy WDL Integration]
   {
      int tbPieceCount = Count( argsBoard->bbWhitePieces | argsBoard->bbBlackPieces );
      if ( tbPieceCount <= TB_LARGEST )
      {
         unsigned tbCastling = 0;
         if ( argsBoard->bbCastle & 1 )
            tbCastling |= TB_CASTLING_K;
         if ( argsBoard->bbCastle & 4 )
            tbCastling |= TB_CASTLING_Q;
         if ( argsBoard->bbCastle & 2 )
            tbCastling |= TB_CASTLING_k;
         if ( argsBoard->bbCastle & 8 )
            tbCastling |= TB_CASTLING_q;

         unsigned tbEP = 0;
         if ( argsBoard->bbEP )
         {
            int tbEPSquares[ 64 ];
            if ( Find( argsBoard->bbEP, tbEPSquares, argsGeneralMoves ) > 0 )
               tbEP = tbEPSquares[ 0 ];
         }

         bool tbTurn = ( argsBoard->siColorToMove == dWhite );

         unsigned tbWDL = tb_probe_wdl(
             argsBoard->bbWhitePieces, argsBoard->bbBlackPieces,
             argsBoard->bbWhiteKing | argsBoard->bbBlackKing,
             argsBoard->bbWhiteQueen | argsBoard->bbBlackQueen,
             argsBoard->bbWhiteRook | argsBoard->bbBlackRook,
             argsBoard->bbWhiteBishop | argsBoard->bbBlackBishop,
             argsBoard->bbWhiteKnight | argsBoard->bbBlackKnight,
             argsBoard->bbWhitePawn | argsBoard->bbBlackPawn,
             argsBoard->iFiftyMoveCounter,
             tbCastling, tbEP, tbTurn );

         if ( tbWDL != TB_RESULT_FAILED )
         {
            bool bCanReturn = true;

            // If we are at the root (Ply 0), we MUST find the best move if we are to use this score.
            // tb_probe_wdl only gives score. We need tb_probe_root for the move (requires DTZ).
            if ( argsBoard->iNumberOfPlys == 0 )
            {
               unsigned tbResults[ TB_MAX_MOVES ];
               unsigned tbRootResult = tb_probe_root(
                   argsBoard->bbWhitePieces, argsBoard->bbBlackPieces,
                   argsBoard->bbWhiteKing | argsBoard->bbBlackKing,
                   argsBoard->bbWhiteQueen | argsBoard->bbBlackQueen,
                   argsBoard->bbWhiteRook | argsBoard->bbBlackRook,
                   argsBoard->bbWhiteBishop | argsBoard->bbBlackBishop,
                   argsBoard->bbWhiteKnight | argsBoard->bbBlackKnight,
                   argsBoard->bbWhitePawn | argsBoard->bbBlackPawn,
                   argsBoard->iFiftyMoveCounter,
                   tbCastling, tbEP, tbTurn, tbResults );

               if ( tbRootResult != TB_RESULT_FAILED )
               {
                  int from  = TB_GET_FROM( tbRootResult );
                  int to    = TB_GET_TO( tbRootResult );
                  int promo = TB_GET_PROMOTES( tbRootResult );

                  int violetPromo = 0;
                  if ( promo != TB_PROMOTES_NONE )
                  {
                     if ( argsBoard->siColorToMove == dWhite )
                     {
                        if ( promo == TB_PROMOTES_QUEEN )
                           violetPromo = dWhiteQueen;
                        else if ( promo == TB_PROMOTES_ROOK )
                           violetPromo = dWhiteRook;
                        else if ( promo == TB_PROMOTES_BISHOP )
                           violetPromo = dWhiteBishop;
                        else if ( promo == TB_PROMOTES_KNIGHT )
                           violetPromo = dWhiteKnight;
                     }
                     else
                     {
                        if ( promo == TB_PROMOTES_QUEEN )
                           violetPromo = dBlackQueen;
                        else if ( promo == TB_PROMOTES_ROOK )
                           violetPromo = dBlackRook;
                        else if ( promo == TB_PROMOTES_BISHOP )
                           violetPromo = dBlackBishop;
                        else if ( promo == TB_PROMOTES_KNIGHT )
                           violetPromo = dBlackKnight;
                     }
                  }

                  // Generate moves to find the index
                  CalculateMoves( vsMoveList, argsBoard, argsGeneralMoves );
                  int foundIndex = -1;
                  for ( int i = 0; i < dNumberOfMoves; i++ )
                  {
                     if ( vsMoveList[ i ].iFromSquare == -1 )
                        break;
                     if ( vsMoveList[ i ].iFromSquare == from && vsMoveList[ i ].iToSquare == to )
                     {
                        if ( violetPromo == 0 && vsMoveList[ i ].iPromote == 0 )
                        {
                           foundIndex = i;
                           break;
                        }
                        if ( violetPromo != 0 && vsMoveList[ i ].iPromote == violetPromo )
                        {
                           foundIndex = i;
                           break;
                        }
                     }
                  }

                  if ( foundIndex != -1 )
                  {
                     // Update PV for Root (or any depth, but critical for root)
                     // Search uses vmPrincipalVariation[ply][0] as the best move for this ply.
                     argsBoard->vmPrincipalVariation[ argsBoard->iNumberOfPlys ][ 0 ] = vsMoveList[ foundIndex ];

                     // The length is ply number + 1 (since it's the move AT this ply)
                     // But usually length is relative to this ply...
                     // Let's check how Search usually sets it.
                     // Usually: vmPrincipalVariation[ ply ][ 0 ] = bestMove;
                     //          Copy rest of PV from child...
                     // Here we have no child PV, so length is just 1 (this move).

                     // PV Length is handled as relative length from this node?
                     // Standard: viPrincipalVariationLength[ ply ] = ...
                     // If we look at line 1961: argsBoard->viPrincipalVariationLength[ childRow ] = argsBoard->iNumberOfPlys;
                     // This suggests length might be absolute ply index of end of PV?
                     // Let's assume standard triangular PV logic where we just set the move.
                     // But wait, the caller reads PV from [0][0], [0][1] etc.
                     // We just set [ply][0].

                     // We set length for this ply to be "NumberOfPlys + 1" (i.e. ending at next ply).
                     argsBoard->viPrincipalVariationLength[ argsBoard->iNumberOfPlys ] = argsBoard->iNumberOfPlys; // Ends here?
                     // No, if I have 1 move, the end is ply+1?
                     // Let's approximate: we just need the move at vmPrincipalVariation[0][0].
                     // The search loop usually handles copying. Here we must do it manually.

                     // Correct assignment for "Just this move":
                     // We don't have a child PV to copy.
                     argsBoard->viPrincipalVariationLength[ argsBoard->iNumberOfPlys ] = argsBoard->iNumberOfPlys;

                     // Also update local iBestMove in case it's used (though we return shortly)
                     iBestMove = foundIndex;
                  }
                  else
                  {
                     bCanReturn = false; // Move not found? Fallback to search
                  }
               }
               else
               {
                  bCanReturn = false; // Root probe failed (no DTZ?), Fallback to search
               }
            }

            if ( bCanReturn )
            {
               int tbScore = 0;
               // dMate is -99999 (Loss), so -dMate is 99999 (Win)
               if ( tbWDL == TB_WIN )
                  tbScore = -dMate - 1000 - argsBoard->iNumberOfPlys;
               else if ( tbWDL == TB_LOSS )
                  tbScore = dMate + 1000 + argsBoard->iNumberOfPlys;
               else
                  tbScore = 0; // Draw

               if ( tbWDL == TB_WIN )
               {
                  if ( tbScore >= argiBeta )
                     return tbScore;
                  if ( tbScore > argiAlpha )
                     argiAlpha = tbScore;
               }
               else if ( tbWDL == TB_LOSS )
               {
                  if ( tbScore <= argiAlpha )
                     return tbScore;
                  if ( tbScore < argiBeta )
                     argiBeta = tbScore;
               }
               else
               {
                  return tbScore;
               }
            }
         }
      }
   }

   // --------------------------------------------------------------------------
   // Reverse Futility Pruning (Static Null Move Pruning)
   // --------------------------------------------------------------------------
#if dUseRFP
   // 1. Conditions: Shallow depth, not in check, and Beta is not too high (avoid pruning in PV nodes or mate scenarios)
   iRemainingDepth = argsBoard->iMaxPlys - argsBoard->iNumberOfPlys;
   if ( iRemainingDepth > 0 && iRemainingDepth <= dRfpDepth &&
        argiBeta < ( dMate - 100 ) &&     // Don't prune near mate scores
        !( argsBoard->bbCheck & 0x3 ) &&  // Check bits 0 & 1 for White/Black check
        argsBoard->iLastMoveNull == dNo ) // Don't do RFP after null move
   {
      // 2. Get Static Evaluation
      int staticEval;
      if ( dUseNNUE == dYes )
      {
         staticEval = EvaluateBoardDirectNNUE( argsBoard, argsGeneralMoves );
      }
      else
      {
         staticEval = EvaluateBoard( argsBoard, argsGeneralMoves );
      }

      // 3. Calculate Margin
      // The margin scales with depth. Deeper searches need safer (larger) margins.
      int margin = dRfpMargin * iRemainingDepth;

      // 4. Pruning Check
      // "Is my position so good (staticEval) that even with a penalty (margin),
      // I am still beating Beta?"
      if ( staticEval - margin >= argiBeta )
      {
         // Return Beta (fail-high).
         return argiBeta;
      }
   }
#endif

   if ( GetUseNullMove() )
   {
      if ( DoNullSearch( argsBoard, argsGeneralMoves, argiAlpha, argiBeta, iScore,
                         vsMoveList, threadData ) )
      {
         threadData->searchParameters.iTried++;
         int iOldLastMoveNull     = argsBoard->iLastMoveNull;
         argsBoard->iLastMoveNull = dYes;
         SwitchSideToMove( argsBoard );
         int iNullReduction = GetNullReduction();
         argsBoard->iMaxPlys -= iNullReduction;
         iNullScore = -Search( argsBoard, argsGeneralMoves, -argiBeta,
                               -argiBeta + 1, threadData );
         argsBoard->iMaxPlys += iNullReduction;
         SwitchSideToMove( argsBoard );
         argsBoard->iLastMoveNull = iOldLastMoveNull;
         if ( iNullScore >= argiBeta )
         {
            return argiBeta;
         }
         else
         {
            threadData->searchParameters.iFailed++;
         }
      }
   }

   // Futility Pruning & LMP Setup
   int iStaticEval             = -dInfiniteTime;
   int iFutilityPruningAllowed = 0;
   int iPrunedMoves            = 0;
   iRemainingDepth             = argsBoard->iMaxPlys - argsBoard->iNumberOfPlys;

   if ( dUseFutility && iRemainingDepth <= dFutilityDepth &&
        !LookForCheck( argsBoard ) && abs( argiAlpha ) < 90000 &&
        abs( argiBeta ) < 90000 )
   {
      iFutilityPruningAllowed = 1;
      if ( dUseNNUE == dYes )
         iStaticEval = EvaluateBoardDirectNNUE( argsBoard, argsGeneralMoves );
      else
         iStaticEval = EvaluateBoard( argsBoard, argsGeneralMoves );

      // STOCKFISH RAZORING IMPLEMENTATION
      // Use a very conservative quadratic margin to avoid overhead
      if ( iRemainingDepth <= 3 )
      {
         int iRazorMargin = 512 + 293 * iRemainingDepth * iRemainingDepth;
         if ( iStaticEval + iRazorMargin < argiAlpha )
         {
            int iQScore = QuiesenceSearch( argsBoard, argsGeneralMoves,
                                           argiAlpha - 1, argiAlpha, threadData );
            if ( iQScore <= argiAlpha )
               return argiAlpha;
         }
      }
   }

   iNumberOfMoves = 1;
   iMoveCount     = -1;

   while ( iSearchFlag )
   {
      iMoveCount++;
      assert( iMoveCount < dNumberOfMoves );

      if ( iMoveCount == 0 && iAttacksSearchFlag == 0 )
      {
         CalculateAttacks( vsMoveList, argsBoard, argsGeneralMoves );
         LegalState( argsBoard, argsGeneralMoves );
         if ( argsBoard->siLegalMove == 0 )
         {
            return -dAlpha;
         }
         iNumberOfAttacks = argsBoard->siNumberOfMoves;
         if ( iNumberOfAttacks == 0 )
         {
            iZeroAttacks = 1;
         }
         SortMovesHash( vsiMoveOrder, vsMoveList, iNumberOfAttacks, argsBoard,
                        argsGeneralMoves, viMoveScores, hashResult,
                        &threadData->searchParameters );

         // COUNTERMOVE HEURISTIC: Boost countermove score
         if ( argsBoard->iNumberOfPlys > 0 )
         {
            Move *prevMove =
                &argsBoard->vmPrincipalVariation[ argsBoard->iNumberOfPlys - 1 ][ 0 ];
            if ( prevMove->iFromSquare >= 0 && prevMove->iFromSquare < 64 &&
                 prevMove->iToSquare >= 0 && prevMove->iToSquare < 64 )
            {
               Move counterMove =
                   gCounterMoves[ prevMove->iFromSquare ][ prevMove->iToSquare ];
               if ( counterMove.iFromSquare >= 0 )
               {
                  for ( int i = 0; i < iNumberOfAttacks; i++ )
                  {
                     if ( vsMoveList[ i ].iFromSquare == counterMove.iFromSquare &&
                          vsMoveList[ i ].iToSquare == counterMove.iToSquare )
                     {
                        viMoveScores[ i ] +=
                            8000; // High priority, below hash move (9000+)
                        break;
                     }
                  }
               }
            }
         }
      }
      if ( ( iMoveCount >= iNumberOfAttacks && iAttacksSearchFlag == 0 ) ||
           ( iMoveCount == 0 && iZeroAttacks == 1 ) )
      {
         iAttacksSearchFlag = 1;
         iZeroAttacks       = 0;
         CalculateQuiets( vsMoveList, argsBoard, argsGeneralMoves );
         iNumberOfQuiets = argsBoard->siNumberOfMoves;
         iMoveCount      = 0;
         SortMovesHash( vsiMoveOrder, vsMoveList, iNumberOfQuiets, argsBoard,
                        argsGeneralMoves, viMoveScores, hashResult,
                        &threadData->searchParameters );

         // COUNTERMOVE HEURISTIC: Boost countermove score (quiets)
         if ( argsBoard->iNumberOfPlys > 0 )
         {
            Move *prevMove =
                &argsBoard->vmPrincipalVariation[ argsBoard->iNumberOfPlys - 1 ][ 0 ];
            if ( prevMove->iFromSquare >= 0 && prevMove->iFromSquare < 64 &&
                 prevMove->iToSquare >= 0 && prevMove->iToSquare < 64 )
            {
               Move counterMove =
                   gCounterMoves[ prevMove->iFromSquare ][ prevMove->iToSquare ];
               if ( counterMove.iFromSquare >= 0 )
               {
                  for ( int i = 0; i < iNumberOfQuiets; i++ )
                  {
                     if ( vsMoveList[ i ].iFromSquare == counterMove.iFromSquare &&
                          vsMoveList[ i ].iToSquare == counterMove.iToSquare )
                     {
                        viMoveScores[ i ] +=
                            8000; // High priority, below hash move (9000+)
                        break;
                     }
                  }
               }
            }
         }
      }
      int currentMoveListSize =
          ( iAttacksSearchFlag == 0 ) ? iNumberOfAttacks : iNumberOfQuiets;
      if ( iMoveCount >= currentMoveListSize )
      {
         iSearchFlag = 0;
         continue;
      }

      argsBoard->iMoveOrder = iMoveCount;
      Update( argsBoard, argsGeneralMoves, threadData );

      // FIX: Only main thread checks interface/time
      if ( threadData->id == 0 )
      {
         InterfaceControl( argsBoard );
      }

      if ( GetStopGo() == dStop )
      {
         argsBoard->iMaxPlys = iMaxPlysOld;
         return argiAlpha;
      }

      // Pruning Logic (Futility & LMP)
      if ( iAttacksSearchFlag == 1 && iFutilityPruningAllowed )
      {
         int iCurrentMoveIndex = vsiMoveOrder[ iMoveCount ];
         int moveScore         = viMoveScores[ iCurrentMoveIndex ];

         // Do not prune moves that give check
         if ( moveScore != argsGeneralMoves->msCheck )
         {
            // Late Move Pruning (LMP)
            int lmpCount = 3 + iRemainingDepth * iRemainingDepth;
            if ( iMoveCount > lmpCount )
            {
               iPrunedMoves++;
               continue;
            }

            // Futility Pruning
            int iMargin = dFutilityMargin * iRemainingDepth;
            if ( iStaticEval + iMargin <= argiAlpha )
            {
               iPrunedMoves++;
               continue;
            }
         }
      }

      // Skip the excluded move during verification search (for Singular Extensions)
      int iCurrentMoveIndex = vsiMoveOrder[ iMoveCount ];
      if ( iCurrentMoveIndex == iExcludedMove )
      {
         continue; // Skip this move
      }

      // Apply Singular Extension if this is the hash move
      int iCurrentExtension = 0;
      if ( iCurrentMoveIndex == hashResult.iBestMove && iExtension > 0 )
      {
         iCurrentExtension = iExtension;
      }

      // Save original depth and apply extension
      int iSavedMaxPlysForExtension = argsBoard->iMaxPlys;
      argsBoard->iMaxPlys += iCurrentExtension;

      // PVS is already handled inside SearchMove
      iScore = SearchMove( argsBoard, argsGeneralMoves, vsMoveList,
                           iCurrentMoveIndex, argiAlpha, argiBeta,
                           iMoveCount, viMoveScores, siHaveMove, threadData );

      // Restore original depth
      argsBoard->iMaxPlys = iSavedMaxPlysForExtension;
      if ( argiAlpha >= argiBeta )
      {
         UpdateKillerMoves( argsBoard, &vsMoveList[ vsiMoveOrder[ iMoveCount ] ],
                            &threadData->searchParameters );

         // COUNTERMOVE HEURISTIC: Store this move as best reply to opponent's last
         // move
         if ( argsBoard->iNumberOfPlys > 0 )
         {
            Move *prevMove =
                &argsBoard->vmPrincipalVariation[ argsBoard->iNumberOfPlys - 1 ][ 0 ];
            if ( prevMove->iFromSquare >= 0 && prevMove->iFromSquare < 64 &&
                 prevMove->iToSquare >= 0 && prevMove->iToSquare < 64 )
            {
               gCounterMoves[ prevMove->iFromSquare ][ prevMove->iToSquare ] =
                   vsMoveList[ vsiMoveOrder[ iMoveCount ] ];
            }
         }

         argsBoard->iMaxPlys = iMaxPlysOld;
         return argiBeta;
      }
   }

   argsBoard->iMaxPlys = iMaxPlysOld;
   if ( siHaveMove == 0 )
   {
      if ( iPrunedMoves > 0 )
      {
         // If we pruned moves and found no legal moves, return alpha (fail low)
         // instead of stalemate (0).
         return argiAlpha;
      }
      argiAlpha = LookForCheckAndStale( argsBoard, argsGeneralMoves );
   }
#if defined( dUseHash )
   if ( GetUseHashTable() == 1 && iBestMove < dNumberOfMoves )
   {
      InputToHashTable( argsBoard, argsGeneralMoves, argiAlpha, argiBeta,
                        argiAlpha, &vsMoveList[ iBestMove ] );
   }
#endif
   return argiAlpha;
}

// Helper to get number of threads
int GetNumberOfThreads()
{
   return gThreads.size() + 1;
}
