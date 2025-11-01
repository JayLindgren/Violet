// Copyright 2006 by Jay Lindgren. All Rights Reserved.
//
#define _CRT_SECURE_NO_WARNINGS
#include "Functions.h"
#include "Structures.h"
#include "Definitions.h"

#include <iostream>
#include <string>       // Add to top of file
#include <sstream>      // Add to top of file
#include <algorithm> // Add to top of file
#include <vector>    // Needed for this example

// Delcare some global variables for now.
Move gvsMoveList[dNumberOfPlys][dNumberOfMoves];
int gvsiMoveOrder[dNumberOfPlys][dNumberOfMoves];
int gviMoveScore[dNumberOfPlys][dNumberOfMoves];

// If in deep mode, include the appropriate files
#if defined( dDeepMode )
   #include <omp.h>
#endif

//SearchParameters gsSearchParameters;

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
SearchParameters gsSearchParameters;

//
//------------------------------------------------------------------------------------------------------------
//
// The function does iterative deeping.
//
int StartSearch( Board * argsBoard, 
                 GeneralMove * argsGeneralMoves,
                 int argiAlpha,
                 int argiBeta )
{
   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Declare holders for determining if a new partial search of a level has returned a good search.
   // The idea is not to compare moves across plys, because the scores will not be consistent.
   // The idea, is that if after a ply has been searched, to make sure that at least the previous best move
   // has been searched.  If a better one has been found at ply, then use that one.
   int iBestMove         = -1; // hold the best move from the previous ply
   int iBestMoveOld      = iBestMove;
   int iBestMoveSearched = 0; // holds whether or not the best move has been searched.
   int iNumberOfMoves    = 0;
   int iOldScore         = 0;
   int iQScore           = 0; // used for setting the initial score of the aspirational search.

   // Initialize search.
   InitializeSearch();

   // Keep an old board around for scoring.
   Board* sOldBoard = new Board();

   // Set the Tempus to search.
   SetStopGo( dGo );

   // Delcare the score.
   int iScore = 0;
   Move vsMoveList[ dNumberOfMoves ];

   // Generate all of the legal moves.
   CalculateMoves( vsMoveList, 
                   argsBoard, 
                   argsGeneralMoves );

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
      iQScore = - QuiesenceSearch( argsBoard,
                                   argsGeneralMoves,
                                   - argiBeta,
                                   - argiAlpha );

      // Define the aspirational search window
      argiBeta  = iQScore + dAspirationalWindowWidth;
      argiAlpha = iQScore - dAspirationalWindowWidth;

   }

   // Loop over the depth.
   for ( int iDepth = 0; iDepth < GetSearchDepth(); iDepth++ )
   {

      // Look for control.
      if ( GetStopGo() == dStop )
      {

         // Time is up. Restore the board state and PV from the last completed search.
         *argsBoard = *sOldBoard;
         GetTempusPVOldToBoard(argsBoard);
         iScore = iOldScore;

         delete sOldBoard;
         return iScore;
      }

      // Set whether or not the best move has been searched at this ply.
      iBestMoveSearched = 0;
      iBestMove = argsBoard->iBestMove;
      argsBoard->iLastMoveNull = dNo;

      // Set the depth of the board.
      argsBoard->iMaxPlys = iDepth;
      argsBoard->iMaxPlysReached = -1;

      // Call the first search routine.
      iScore = FirstSearch( argsBoard, 
                            argsGeneralMoves,
                            argiAlpha,
                            argiBeta,                       
                            & iBestMove,
                            & iBestMoveSearched );

      // Only do for Aspirational searchs.
      if ( GetAspirationSearch() )
      {

         // If the aspriational search failed, research the whole window.
         if ( iScore >= argiBeta ||
              iScore <= argiAlpha )
         {

            // Reset the window.
            argiAlpha = dAlpha;
            argiBeta = dBeta;

            // Research the using the whole window
            iScore = FirstSearch( argsBoard, 
                                  argsGeneralMoves,
                                  argiAlpha,
                                  argiBeta,                       
                                  & iBestMove,
                                  & iBestMoveSearched );

         }
 
      }

      // If the search for this depth completed without being stopped, save the state.
      if (GetStopGo() == dGo)
      {
         // Save the state from the completed search.
         *sOldBoard = *argsBoard;
         iOldScore = iScore;
         SetTempusPVOldFromBoard(argsBoard);
      }

      // Update the number of nodes searched in Tempus for use in opotimization of parameters
      SetNumberOfNodesSearched( (int)(GetNumberOfNodes()) );

      // Update the folks at home.
      if ( GetInterfaceMode() == dConsole &&
           GetStopGo() == dGo &&
           GetConsoleOutput() == dYes )
      {

         cout << "Ply = " << iDepth + 1 << " Score = " << iScore << " qDepth = " << argsBoard->iMaxPlysReached << " Nodes searched = " << FormatWithCommas(GetNumberOfNodes()) << endl;
         //cout << "max depth reached = " << argsBoard->iMaxPlysReached << endl;
         //cout << "Number of nodes searched = " <<   << endl;
         //cout << "Maximum depth obtained = " << argsBoard->iMaxPlysReached << endl;
         if ( GetStopGo() == dGo )
         {

            // Print the principal variation
            PrintPrincipalVariation( argsBoard,
                                     argsGeneralMoves ); 

            // Print the multiPV if ponder is on:
            if ( GetPonder() )
            {

               PrintMultiPV( argsBoard,
                             argsGeneralMoves,
                             iNumberOfMoves );

            }

         }

         // If we are at check mate, stop the search.
         if ( iScore <= dMate )
         {
        
            SetStopGo( dStop );
            iDepth = dNumberOfPlys;

         }

      }
      if ( GetInterfaceMode() == dUCI &&
           GetStopGo() == dGo )
      {

         char strUpdate[ 640 ];
         sprintf( strUpdate, 
                  "info depth %d seldepth %d score cp %d nodes %d", 
                  iDepth + 1, 
                  argsBoard->iMaxPlysReached + 1, 
                  iScore,
                  (int)GetNumberOfNodes() );
         SendCommand( strUpdate );

      }
      if ( iScore <= dMate &&
           GetStopGo() == dGo )
      {
        
         SetStopGo( dStop );
         iDepth = dNumberOfPlys;

      }
  
   }

   // This prints out the tried and vaile for the null move search/pruning.
   //cout << "Gobal Tried = " << gsSearchParameters.iTried << " Failed = " << gsSearchParameters.iFailed << endl;

   // Clean up the old board.
   delete sOldBoard;

   // Return the score.
   return iScore;

}

//
//------------------------------------------------------------------------------------------------------------
//
// The function does a recursive nega max search. (Refactored Version)
//
int FirstSearch( Board* argsBoard,
                 GeneralMove* argsGeneralMoves,
                 int argiAlpha,
                 int argiBeta,
                 int* argiBestMove,
                 int* argiBestMoveSearched )
{

   // Debug the inputs.
   assert(argsBoard >= 0);
   assert(argsGeneralMoves >= 0);

   // Allocations.

   assert(argsBoard->iNumberOfPlys + 1 >= 0);
   Move* vsMoveList   = gvsMoveList[argsBoard->iNumberOfPlys + 1];
   int*  vsiMoveOrder = gvsiMoveOrder[argsBoard->iNumberOfPlys + 1];
   int*  viMoveScores = gviMoveScore[argsBoard->iNumberOfPlys + 1];
   int   iMoveCount;
   char  strUpdate[ 1280 ];
   char  strMove[     64 ];
   int   iHaveMove     =  0;
   int  iBestMoveIndex = -1;


   // Set the search depth to zero.
   argsBoard->iNumberOfPlys   = -1;
   argsBoard->iMaxPlysReached = -1;

   // Set the null verification to yes.
   argsBoard->iUseNullVerification = dYes;

   // Initialize the history stack.
   if (argsBoard->iMoveHistory == -1)
   {
      argsBoard->iMoveHistory = 0;
   }

   // Put the initial hash into the move history.
   argsBoard->sHistoryStack[argsBoard->iMoveHistory].bbHash = GetHash();

   // Initialize the ply length.
   argsBoard->viPrincipalVariationLength[argsBoard->iNumberOfPlys + 1] = argsBoard->iNumberOfPlys + 1;

   // Look for a draw - function returns a if a three fold repetition is found.
   if (LookForDraw(argsBoard, argsGeneralMoves))
   {
      // Return a draw score.
      return 0;
   }

   // Generate all of the legal moves.
   CalculateMoves(vsMoveList,
      argsBoard,
      argsGeneralMoves);

   // Let the number of moves live in a local variable.
   int iNumberOfMoves = argsBoard->siNumberOfMoves;

   // Check the state of the board and see if it is legal.
   LegalState(argsBoard, argsGeneralMoves);

   // If the move is not legal, undo the move and continue.
   if (argsBoard->siLegalMove == 0)
   {
      // return because we found an illegal move.
      return -dAlpha;
   }
   else
   {
# if defined( dUseHash )
      // See if the hash table can help.
      ExtractFromHashTable( argsBoard, 
                            argsGeneralMoves);

      // If found at the appropriate depth, return the score.
      if ((GetQueryState() == 1))
      {
         return GetScoreHash();
      }
# endif

      // Sort the moves to take the best bet.
      SortMovesHash( vsiMoveOrder,
                     vsMoveList,
                     iNumberOfMoves,
                     argsBoard,
                     argsGeneralMoves,
                     viMoveScores );

      // Loop over all generated moves.
      for (iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++)
      {

         if (GetStopGo() == dGo)
         {

            int iCurrentMoveIndex = vsiMoveOrder[iMoveCount];

            // Update the interface as to what move we are searching.
            if (GetTimeSinceLastMoveUpdate() > dMoveUpdateTime && GetInterfaceMode() == dUCI)
            { 

               CreateAlgebraicMove(strMove, vsMoveList, iCurrentMoveIndex);
               sprintf(strUpdate, "info currmove %s currmovenumber %d ", strMove, iMoveCount + 1);
               SendCommand(strUpdate);
            
            }

            // The SearchMove function will modify the board, so we operate on a temporary copy.
            Board tempBoard = *argsBoard;
            int oldAlpha    = argiAlpha; // Store alpha before the call

            // Call the new helper function to process the move. Alpha is passed by reference.
            int iScore = SearchMove( &tempBoard,
                                     argsGeneralMoves,
                                     vsMoveList,
                                     iCurrentMoveIndex,
                                     argiAlpha, // This will be updated inside the function
                                     argiBeta,
                                     iMoveCount,
                                     viMoveScores,
                                     iHaveMove);

            // If the previous best move has been searched, set the flag.
            if (iCurrentMoveIndex == *argiBestMove)
            {
               *argiBestMoveSearched = 1;
            }

            // Check if a new best move was found (i.e., alpha was updated)
            if (argiAlpha > oldAlpha)
            {
               // Mark which move is the best so far.
               iBestMoveIndex = iCurrentMoveIndex;
               argsBoard->iBestMove = iBestMoveIndex;

               // IMPORTANT: The PV was updated on tempBoard, so we must copy it back
               // to the main board to preserve it for the next iteration/depth.
               *argsBoard = tempBoard;

               // Update the interface with the new score and PV.
               if (GetInterfaceMode() == dUCI)
               {
                  SetScore(argiAlpha);

                  // --- Refactored using modern C++ for safety ---

                  // 1. Build the Principal Variation (PV) string safely using a stringstream.
                  std::stringstream pv_stream;
                  for (int iPVCount = 0; iPVCount <= argsBoard->iMaxPlys; iPVCount++)
                  {
                     // The existing CreateAlgebraicMove function requires a C-style buffer, which is fine to use locally.
                     char strMove[64];
                     CreateAlgebraicMove(strMove, &argsBoard->vmPrincipalVariation[0][iPVCount], 0);
                     pv_stream << " " << strMove;
                  }

                  // 2. Build the final "info" string using another stringstream. This replaces sprintf.
                  std::stringstream update_stream;
                  update_stream << "info depth " << (argsBoard->iMaxPlys + 1)
                     << " seldepth " << (argsBoard->iMaxPlysReached + 1)
                     << " nodes " << GetNumberOfNodes()
                     << " pv" << pv_stream.str()
                     << " score cp " << argiAlpha;

                  // 3. Send the command. 
                  // .str() gets the std::string, and .c_str() provides the const char* that SendCommand expects.
                  SendCommand(update_stream.str().c_str());
               }

            }

            // Look for a beta cutoff.
            if (argiAlpha >= argiBeta)
            {
               break; // Exit the loop
            }
         }
      } // end of loop over moves

   } //  end of if for legal move

   // Take care of the case where we didn't find any legal moves.
   if (iHaveMove == 0)
   {
      // Look for check and stale mates
      argiAlpha = LookForCheckAndStale( argsBoard,
                                        argsGeneralMoves);
      // Copy the move history into the principal variation.
      CreatePV( argsBoard );
   }

   // Return the score.
   return argiAlpha;
}

//
//----------------------------------------------------------------------------------------------------------
//
int QuiesenceSearch( struct Board * argsBoard, 
                     struct GeneralMove * argsGeneralMoves,
                     int argiAlpha,
                     int argiBeta)
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves > 0 );
   assert( argiBeta >= argiAlpha );

   // Allocations.
   //Move* vsMoveList = new Move[dNumberOfMoves];
   //int* vsiMoveOrder = new int[dNumberOfMoves];
   Move* vsMoveList = gvsMoveList[argsBoard->iNumberOfPlys + 1];
   int* vsiMoveOrder = gvsiMoveOrder[argsBoard->iNumberOfPlys + 1];
   int* viMoveScores = gviMoveScore[argsBoard->iNumberOfPlys + 1];
   int iMoveCount;
   int iScore;
   int siHaveMove = 0;
   //Move vsMoveList[ dNumberOfMoves ];
   //int vsiMoveOrder[ dNumberOfMoves ];
   int iNumberOfCaptures;

   // Initialize the score.
   iScore = 0;

   // Look for a special case.
   if ( argiAlpha == argiBeta &&
        GetAspirationSearch() )
   {

      return argiAlpha;

   }
   
   // Initialize the ply length.
   argsBoard->viPrincipalVariationLength[ argsBoard->iNumberOfPlys ] = argsBoard->iNumberOfPlys;
                                            
      
   // Generate all of the legal moves.
   CalculateAttacks( vsMoveList, 
                     argsBoard, 
                     argsGeneralMoves );

   // Check the state of the board and see if it is legal.
   LegalState( argsBoard,
               argsGeneralMoves );


   // If the move is not legal, undo the move and continue.
   if ( argsBoard->siLegalMove == 0  )
   {

      // return because we found an illegal move.
      return - dAlpha;

   }

   // Take a look at the evaluation.
   iScore = EvaluateBoard( argsBoard,
                           argsGeneralMoves );

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
   SortMoves( vsiMoveOrder,
              vsMoveList,
              argsBoard->siNumberOfMoves );

   iNumberOfCaptures = argsBoard->siNumberOfMoves;


   // Check and see if we are at a quiet point.
   if ( iNumberOfCaptures == 0 )
   {

      return iScore;

   }

	// Loop over the captures.
   for ( iMoveCount = 0; iMoveCount < iNumberOfCaptures; iMoveCount++ )
   {

      // Make a move on the board.
      MakeMove( vsMoveList, 
                argsBoard,
                argsGeneralMoves,
                vsiMoveOrder[ iMoveCount ] );

      // Update the control of the game.
      Update( argsBoard,
              argsGeneralMoves );
  
      // If we are at depth, find a quiet mind.
      iScore = - QuiesenceSearch( argsBoard,
                                  argsGeneralMoves,
                                  -argiBeta,
                                  -argiAlpha );

      // Mark that we have a move at this ply
      if ( argsBoard->siLegalMove == 1 )
      {

         siHaveMove = 1;

      }

      // Undo the move.
      UndoMove( argsBoard, 
                argsGeneralMoves );

      // Do that Alpha Beta thing.
      if ( iScore > argiAlpha )
      {
           
         // A new winner in the search for the perfect move!
         argiAlpha = iScore;

         // Look for a cute off.
         if ( iScore >= argiBeta )
         {
           
            // No use searching further.
            return argiBeta; // Should we return beta here?
                
         }
           
      }
      
   }

   // Clean up the memory.
   //delete[] vsMoveList;
   //delete[] vsiMoveOrder;

   // Return the score.
   return argiAlpha;

}

//
//----------------------------------------------------------------------------------------------------------
//
int LookForSearchExtensions( struct Board * argsBoard,
                             struct GeneralMove * argsGeneralMoves,
                             struct Move * argvsMoveList,
                             int iMoveNumber,
                             int iNewCheck )
{

   // This function is called for both checking for if a null move is appropriate and
   // to see if the search needs to be extended.
   // For the null search, the first look at check is appropriate.
   // for the extension, the additional calculation for check is required.
   // iNewCheck is a variable for deciding on whether or not to fun the additional check.

   // Store some local attacks
   Move vsMoveList[ dNumberOfMoves ]; // stored locally due to the complexities of the recursive search

   if ( argsBoard->bbCheck & 1 ||
        argsBoard->bbCheck & 2 )
   {

      return 1;

   }

   // Do a basic check to see if the side to move is in check.
   int iSearchDeeper = 0;

   // The look for an appropriate move number.  The null move routine will pass in -1.
   if ( iMoveNumber >= 0 )
   {
   
      // Look for a passed pawn.  search deeper, note, this should catch pawn races.
      if ( argvsMoveList[ iMoveNumber ].iPiece == dWhitePawn )
      {

         // Calculate a passed pawn
         int iEndSquare = argvsMoveList[ iMoveNumber ].iToSquare;
         if ( ! ( argsGeneralMoves->vbbWhitePPWideVector[ iEndSquare ] & argsBoard->bbBlackPawn ) )
         {

            return 1; // A hack for testing

         }

      }
      if ( argvsMoveList[ iMoveNumber ].iPiece == dBlackPawn )
      {

         // Calculate a passed pawn
         int iEndSquare = argvsMoveList[ iMoveNumber ].iToSquare;
         if ( ! ( argsGeneralMoves->vbbBlackPPWideVector[ iEndSquare ] & argsBoard->bbWhitePawn ) )
         {

            return 1; // A hack for testing

         }

      }

   } // move number

   // See if we need to check for an uncalculated check.
   if ( iNewCheck == 1 )
   {
      
      // Generate all of the legal attacks.
      CalculateAttacks( vsMoveList, 
                        argsBoard, 
                        argsGeneralMoves );

      // If we are in check for any reason search deeper
      if ( argsBoard->bbCheck & 1 ||
           argsBoard->bbCheck & 2 )
      {

         return 1;

      }

   }

   return iSearchDeeper;

}

//
//---------------------------------------------------------------------------------------------------------------------
//
// See if we should do the null search - IMPROVED VERSION
int DoNullSearch( struct Board * argsBoard,
                  struct GeneralMove * argsGeneralMoves,
                  int argiAlpha,
                  int argiBeta,
                  int iScore,
                  struct Move * argsMove )
{

   int iDoSearch = 0;
   int iMoveNumber = -1; // This is used to more efficiently use LookForSearchExtensions.
   int iCheck      = 0; // This is used to more efficiently use LookForSearchExtensions;
   int iQScore     = 0;

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
   if ( ( iRemainingDepth > GetNullReduction() ) &&
        ( argiBeta < -dMate ) &&
        ( argsBoard->iLastMoveNull == dNo ) &&
        ( argsBoard->iMoveOrder != 0 ) )

   { 
      // Check if we're in check - null move is illegal when in check
      if ( LookForCheck( argsBoard ) )
      {
         return 0; // Can't do null move when in check
      }

      /*
      // See if pieces other than just the king and pawns are on the board.
      // This is done to avoid zugzwang.
      BitBoard bbTotalNonKingPawn = argsBoard->bbBlackRook + 
                                    argsBoard->bbBlackKnight + 
                                    argsBoard->bbBlackBishop +
                                    argsBoard->bbBlackQueen +
                                    argsBoard->bbWhiteRook + 
                                    argsBoard->bbWhiteKnight +
                                    argsBoard->bbWhiteBishop +
                                    argsBoard->bbWhiteQueen;

      if ( bbTotalNonKingPawn == 0 )
      {
         // Do not do a null search in pure king and pawn endgames (zugzwang risk)
         return 0;
      }
      */

      // --- Recommended Change in DoNullSearch ---

      // See if pieces other than just the king and pawns are on the board for the side to move.
      // This is done to avoid zugzwang.
      bool hasMajorOrMinorPieces = false;
      if (argsBoard->siColorToMove == dWhite)
      {
         if (argsBoard->bbWhiteRook | argsBoard->bbWhiteKnight | argsBoard->bbWhiteBishop | argsBoard->bbWhiteQueen)
         {
            hasMajorOrMinorPieces = true;
         }
      }
      else // Black to move
      {
         if (argsBoard->bbBlackRook | argsBoard->bbBlackKnight | argsBoard->bbBlackBishop | argsBoard->bbBlackQueen)
         {
            hasMajorOrMinorPieces = true;
         }
      }

      if (!hasMajorOrMinorPieces)
      {
         // In a king-and-pawn endgame for the side to move, don't do a null search.
         return 0;
      }
      
      /*
      // Additional zugzwang detection: if material is low, be more cautious
      int iTotalPieces = CountBits( bbTotalNonKingPawn );
      
      // In positions with very few pieces (e.g., <= 3), consider verification search
      if ( iTotalPieces <= 3 && argsBoard->iUseNullVerification == dYes )
      {
         // Could implement verification search here or skip null move
         // For now, we allow it but this is where you'd add verification logic
      }
      */

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
//----------------------------------------------------------------------------------------------------------
//
int LookForCheck( struct Board * argsBoard )
{
   // Do a basic check to see if the side to move is in check.

   int iInCheck = ( ( ( argsBoard->bbCheck & 1 ) &&                   // Make sure we are not in check.
                      ( argsBoard->siColorToMove == dWhite ) ) ||
                    ( ( argsBoard->bbCheck & 2 ) &&
                    ( argsBoard->siColorToMove == dBlack ) ) );

   return iInCheck;

}

//
//-------------------------------------------------------------------------------------------------------------
//
// See if we should to a LMR (Late Move Reduction)
int DoLMRSearch( int * viMoveScore,
                 int iMoveCount,
                 struct Board * argsBoard,
                 int argiAlpha )
{

   // Set the default reduction to zero
   int iLMR = 0;
   int iReduction = GetLMRReducedSearchDepth();
   iReduction = 1;

   // If LMR is turned off, just return a zero.
   if ( GetUseLMR() == dNo )
   {

      return iLMR;

   }

   // If we have already reached mate, no need to search (messes up the hash table)
   if ( argiAlpha >= - dMate )
   {

      return iLMR;

   }

   // Make sure we have search a minimum number of moves.
   if ( iMoveCount > ( GetLMRMinimumMoveSearch() - 1 ) )
   {

      // Make sure the move is below a minimum move score.
      if ( viMoveScore[ iMoveCount ] <= GetLMRMinimumMoveScore() )
      {

         // See if reducing the depth will help at all.
         if ( ( argsBoard->iNumberOfPlys + iReduction - 1 ) < argsBoard->iMaxPlys )
         { 

            // Set the we are doing an LMR.
            iLMR = 1;

            // Set the new search depth.
            // Note the minus one because the move is already made in Search()
            // Reduce the search depth by the amount of the reduction.
            //argsBoard->iMaxPlys = argsBoard->iNumberOfPlys + GetLMRReducedSearchDepth() - 1;
            // Note the Plus one because the move is already made in Search()
            argsBoard->iMaxPlys = argsBoard->iMaxPlys - iReduction ;

         }

      }

   }

   // return the logical LMR
   return iLMR;

}

//
//--------------------------------------------------------------------------------------------------------------
//
// Look for check and stale mates
int LookForCheckAndStale( struct Board * argsBoard,
                          struct GeneralMove * argsGeneralMoves )
{

   // Define some variables
   int iScore = 0;

   // Calculate the attacks and see if we are in check.
   Move vsMoveListCheck[ dNumberOfMoves ]; // stored locally due to the complexities of the recursive search
      
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
   CalculateAttacks( vsMoveListCheck, 
                     argsBoard, 
                     argsGeneralMoves );
     
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
  if ( argsBoard->siColorToMove == dWhite  &&
       ( argsBoard->bbBlackAttack & argsBoard->bbWhiteKing ) )
  {

     // Set the score.
     iScore = dMate;

      // Set that white is in check mate.
      argsBoard->bbCheck = SetBitToOne( argsBoard->bbCheck, 2 );

      return iScore;

  }
  else if ( argsBoard->siColorToMove == dBlack  &&
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

   // Initialize the depth reduction for LMR
   for ( int iDepth = 0; iDepth < dNumberOfPlys; iDepth++ )
   {

      for ( int iMove = 0; iMove < dNumberOfMoves; iMove++ )
      {

         // Set the LMR reduction parameters
         gsSearchParameters.miSearchReduction[ iDepth ][ iMove ] = int( 0.33 + log(  double( iDepth + 1 ) ) * log( double( iMove + 1 ) ) / 2.25 );

         // Set the history hueristic counters.
         gsSearchParameters.mWhiteHistoryHeuristic[ iDepth ][ iMove ] = 0;
         gsSearchParameters.mBlackHistoryHeuristic[ iDepth ][ iMove ] = 0;      

      }

   }

   // Reset the Tempus parameters
   SetTempusParameters();

   // Reset the history heuristic.
   ResetHistoryHeuristic();
   ResetKillerMoves();

   // Initialize the multi PV
   InitializeMultiPV();

   gsSearchParameters.iWhiteHHCount = 0;
   gsSearchParameters.iBlackHHCount = 0;

   gsSearchParameters.iWhiteHHCount = 0;
   gsSearchParameters.iBlackHHCount = 0;

   gsSearchParameters.iTried = 0;
   gsSearchParameters.iFailed = 0;
   gsSearchParameters.iZug = 0;

   // Set up the null pruning parameters
   gsSearchParameters.iPruningSchedule[ 0 ] = dForwardPrune0;
   gsSearchParameters.iPruningSchedule[ 1 ] = dForwardPrune1;
   gsSearchParameters.iPruningSchedule[ 2 ] = dForwardPrune2;
   gsSearchParameters.iPruningSchedule[ 3 ] = dForwardPrune3;
   gsSearchParameters.iPruningSchedule[ 4 ] = dForwardPrune4;
   gsSearchParameters.iPruningSchedule[ 5 ] = dForwardPrune5;
   gsSearchParameters.iPruningSchedule[ 6 ] = dForwardPrune6;
   gsSearchParameters.iPruningSchedule[ 7 ] = dForwardPrune7;
   gsSearchParameters.iPruningSchedule[ 8 ] = dForwardPrune8;
   gsSearchParameters.iPruningSchedule[ 9 ] = dForwardPrune9;

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void Update(struct Board* argsBoard,
   struct GeneralMove* argsGeneralMoves)
{
   // Do the time control yada yada.

      // Debug the inputs.
   assert(argsBoard >= 0);
   assert(argsGeneralMoves >= 0);

   // Update the number of nodes counted.
   {
      BitBoard bbNumberOfNodes = GetNumberOfNodes();
      bbNumberOfNodes++;
      SetNumberOfNodes(bbNumberOfNodes);
      IncrementPollingCount();

      BitBoard bbNumberOfIncrementalNodes = GetNumberOfIncrementalNodes();
      bbNumberOfIncrementalNodes++;
      SetNumberOfIncrementalNodes(bbNumberOfIncrementalNodes);

      int iStartTime = GetSearchStartTime();
      int iClock = clock();
      int iSearchTime = GetSearchTimeInMiliSeconds();

      // Do some search time control.
      if (GetSearchTimeInMiliSeconds() < (clock() - GetSearchStartTime()))
      {
         SetStopGo(dStop);
      }

      // See if there is input from the GUI
      if (GetInterfaceMode() == dUCI &&
         GetPollingCount() >= dPollingCount)
      {
         // Reset the polling count.
         SetPollingCount(0);

         // Poll for input.
         if (CheckForInput())
         {
            // See if we can process the command now.
            ReadInputAndExecute(argsBoard,
               argsGeneralMoves);
         }
      }

      // Look for a stop by number of notes
      if (GetNodes() > 0)
      {
         // If the number of search notes is larger than the node count, then bail out.
         if (GetNodes() < bbNumberOfNodes)
         {
            // Set the stop.
            SetStopGo(dStop);
         }
      }

      // If at an incremental amount of nodes, let the folks at home know what is going on.
      if (bbNumberOfIncrementalNodes == GetNodeCheck())
      {
         // Calculate the nodes per second.
         SetEnd(clock());
         double duration = (double)(GetEnd() - GetStart()) / CLOCKS_PER_SEC;
         double rate = (double)(bbNumberOfIncrementalNodes) / duration;
         SetStart(clock());

         if (GetInterfaceMode() == dConsole)
         {
            cout << "node Count = " << bbNumberOfIncrementalNodes << " nodes/sec = " << (int)rate << endl;
         }
         if (GetInterfaceMode() == dUCI &&
            GetStopGo() == dGo)
         {
            // Send an update to the interface.
            char strUpdate[640];
            int iNodes = (int)(GetNumberOfNodes());
            snprintf(strUpdate, sizeof(strUpdate),
               "info nodes %d nps %i ", // Changed "rate" to "nps" for UCI standard
               iNodes,
               (int)rate);
            SendCommand(strUpdate);
         }
         SetNumberOfIncrementalNodes(0);
      }
   } // End critical block.
}

//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void SearchMonitor( struct Board * argsBoard,
                    struct Move * argvsMoves,
                    struct GeneralMove * argsGeneralMoves )
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
//--------------------------------------------------------------------------------------------------------------------------------
//
void CreatePV( struct Board * argsBoard )
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
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].iFromSquare  = argsBoard->sHistoryStack[ iGamePly ].iFromSquare;
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].iToSquare    = argsBoard->sHistoryStack[ iGamePly ].iToSquare;
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].bbFromSquare = argsBoard->sHistoryStack[ iGamePly ].bbFromSquare;
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].bbToSquare   = argsBoard->sHistoryStack[ iGamePly ].bbToSquare;
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].iCapture     = argsBoard->sHistoryStack[ iGamePly ].iCapture;
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].iPiece       = argsBoard->sHistoryStack[ iGamePly ].iPiece; 
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].bbEPSquare   = argsBoard->sHistoryStack[ iGamePly ].bbEPSquare; 
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].bbCastle     = argsBoard->sHistoryStack[ iGamePly ].bbCastle;
   argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyIndex ].iMoveType    = argsBoard->sHistoryStack[ iGamePly ].iMoveType;

   // Update the PV matrix.
   for ( int iPlyLoop = iPlyIndex + 1; iPlyLoop <= argsBoard->viPrincipalVariationLength[ iPlyIndex + 1 ]; iPlyLoop++ )
   {

      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].iFromSquare  = argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].iFromSquare;
      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].iToSquare    = argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].iToSquare;
      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].bbFromSquare = argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].bbFromSquare;
      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].bbToSquare   = argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].bbToSquare;
      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].iCapture     = argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].iCapture;
      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].iPiece       = argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].iPiece; 
      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].bbEPSquare   = argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].bbEPSquare; 
      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].bbCastle     = argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].bbCastle;
      argsBoard->vmPrincipalVariation[ iPlyIndex ][ iPlyLoop ].iMoveType    = argsBoard->vmPrincipalVariation[ iPlyIndex + 1 ][ iPlyLoop ].iMoveType;

   }

   // Update the principal variation length.
   argsBoard->viPrincipalVariationLength[ iPlyIndex ] = argsBoard->viPrincipalVariationLength[ iPlyIndex + 1 ];

}

void PrintPrincipalVariation( struct Board * argsBoard,
                              struct GeneralMove * argsGeneralMoves )
{
// Print out the principal variation.

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves > 0 );

   int iPlyIndex;
   char strMove[ 10 ];
   int iNumberOfCharacters;
   int iCount;

   // Initialize some parameters
   iCount = -1;

   // Rest the control if needed.
   SetStopGo( dGo );
   
   // Perform a shallow search.
   // Keep an old board around for scoring.
   struct Board * sBoard;
   sBoard = new Board();

   // Create a new board for a shallow search.
   * sBoard = * argsBoard;
   sBoard->iMaxPlys = 3;

   // Give the board some time to search.
   int iOldTime = GetSearchTimeInMiliSeconds();
   SetSearchTimeInMiliSeconds( 99999999999  );
               
   // Do the search.
   int iScore = Search( sBoard,
                        argsGeneralMoves,
                        dAlpha,
                        dBeta );

   // Reset the search time.
   SetSearchTimeInMiliSeconds( iOldTime  );

   if ( sBoard->bbCheck & 16 ||
        sBoard->bbCheck & 32 )
   {
   
      cout << "Stalemate or draw." << endl << endl;
      delete sBoard;
      return;

   }

   // Reset the board.
   * sBoard = * argsBoard;

   // Loop over the forward plys.
   for ( iPlyIndex = 0; iPlyIndex < argsBoard->iMaxPlys; iPlyIndex++ )
   {

      // Note this counter is needed incase a check mate is found.  Then we will jump out.
      iCount++;

      // At this time, because of the hash table, the structure sBoard may not have a full PV (because if it is being scored from a hash)
      // an early cut off will not allow it to store the PV
      // This is a hack to get out of the loop if the data is bad.
      if ( ( sBoard->vmPrincipalVariation[ 0 ][iPlyIndex ].iFromSquare < 0 ) || 
           ( sBoard->vmPrincipalVariation[ 0 ][iPlyIndex ].iToSquare < 0 ) || 
           ( sBoard->vmPrincipalVariation[ 0 ][iPlyIndex ].iFromSquare > 64 ) || 
           ( sBoard->vmPrincipalVariation[ 0 ][iPlyIndex ].iToSquare > 64 ) ) 
      {
         
         // If we have a bad square, jump out of the loop.
         cout << endl << endl;
         delete sBoard;
         return;

      }


      // Print out a move.
      //* sBoard = * argsBoard;
      iNumberOfCharacters =  PrintMove( sBoard,
                                        argsGeneralMoves,
                                        &sBoard->vmPrincipalVariation[ 0 ][iPlyIndex ],
                                        strMove );
      cout << " ";
                
      for ( int iCharCount = 0; iCharCount < iNumberOfCharacters; iCharCount++ )
      {
      
         cout << strMove[ iCharCount ];
         
      }

      // Make the move on the board.
      // Note that PrintMove() requires the board be at the appropriate state.
      MakeMove( &sBoard->vmPrincipalVariation[ 0 ][iPlyIndex ],
                sBoard,
                argsGeneralMoves,
                0 );

      // If checkmate is found, bail out of the loop.
      if ( !strncmp( & strMove[ iNumberOfCharacters - 1 ], 
                     "#",
                     1 ) )
      {

         break;

      }  

      // Perform a shallow search.
      //* sBoard = * arg6;
      sBoard->iMaxPlys = 1;
               
      // Do the search.
      int iScore = Search( sBoard,
                           argsGeneralMoves,
                           dAlpha,
                           dBeta );

      // Look for the special cases of checkmate and stalemate.
      if ( sBoard->bbCheck & 16 ||
           sBoard->bbCheck & 32 )
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
      UndoMove( sBoard,
                argsGeneralMoves );

   }
//*/
   cout << endl << endl;
   
   // Free the memory.
   delete sBoard;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// This function encapsulates the logic for searching a single move. It is called from
// within the main loops of FirstSearch() and Search() to reduce code duplication.
//
// It handles:
// - Making and undoing the move.
// - Deciding between a full search, LMR search, or quiescence search.
// - Performing the recursive search call.
// - Storing results in the hash table.
// - Updating alpha and associated heuristics (PV, Killer, History).
//
int SearchMove(Board* argsBoard,
   GeneralMove* argsGeneralMoves,
   Move* vsMoveList,
   int iMoveIndex,
   int& argiAlpha, // Pass alpha by reference to update it
   int argiBeta,
   int iMoveCount, // The index in the sorted move list (0 for best move)
   int* viMoveScores,
   int& siHaveMove)
{
   int iScore = 0;

   // Make a move on the board.
   MakeMove(vsMoveList,
      argsBoard,
      argsGeneralMoves,
      iMoveIndex);

   // Look for a draw - function returns true if a three fold repetition is found.
   if (LookForDraw(argsBoard, argsGeneralMoves))
   {
      iScore = GetValueOfDraw();
   }
   // Check if we need to search deeper.
   else if (argsBoard->iNumberOfPlys < argsBoard->iMaxPlys - 1)
   {
      int iOldMaxPlys = argsBoard->iMaxPlys;

      // Do the LMR search if applicable.
      if (DoLMRSearch(viMoveScores, iMoveCount, argsBoard, argiAlpha))
      {
         // Do the search with a null window.
         iScore = -Search( argsBoard,
                           argsGeneralMoves,
                           -argiAlpha - 1,
                           -argiAlpha);

         // Restore the board to the old search depth.
         argsBoard->iMaxPlys = iOldMaxPlys;

         // Look for an LMR research if the null-window search beat alpha
         if (iScore > argiAlpha)
         {
            // Redo the search with the full window.
            iScore = -Search(argsBoard,
               argsGeneralMoves,
               -argiBeta,
               -argiAlpha);
         }
      }
      // If not LMR, do a normal PVS search
      else
      {
         // For moves other than the first, use a null-window search first.
         if (iMoveCount > 0)
         {
            iScore = -Search(argsBoard,
               argsGeneralMoves,
               -argiAlpha - 1,
               -argiAlpha);

            // If the scout search failed high, redo the full search.
            if (iScore > argiAlpha && iScore < argiBeta)
            {
               iScore = -Search(argsBoard,
                  argsGeneralMoves,
                  -argiBeta,
                  -iScore); // Note: some engines use -argiAlpha here
            }
         }
         else // For the first move, do a full window search.
         {
            iScore = -Search(argsBoard,
               argsGeneralMoves,
               -argiBeta,
               -argiAlpha);
         }
      }
   }
   // Perform a quiesence search if we are at the horizon.
   else
   {
      iScore = -QuiesenceSearch(argsBoard,
         argsGeneralMoves,
         -argiBeta,
         -argiAlpha);
   }


   // After the recursive call, check if the move was legal and mark that we have one.
   if (argsBoard->siLegalMove == 1)
   {
      siHaveMove = 1;
   }

   // Undo the move to restore the board state.
   UndoMove(argsBoard,
      argsGeneralMoves);


   // If the move was illegal, just return the current alpha and continue.
   if (argsBoard->siLegalMove == 0)
   {
      return argiAlpha;
   }


   // Do that Alpha Beta thing.
   if (iScore > argiAlpha)
   {
      // A new best move was found! Update alpha.
      argiAlpha = iScore;

#if defined( dUseHash )
      // Update the history heuristic.
      if (GetUseHashTable())
      {
         UpdateHH(argsBoard, &vsMoveList[iMoveIndex]);
      }
#endif

      // Copy the move history into the principal variation.
      CreatePV(argsBoard);

      // Set the best move for the hash table.
      argsBoard->iBestMove = iMoveIndex;
   }

#if defined( dUseHash )
   // Put the score into the hash table regardless of whether it raised alpha.
   if (GetUseHashTable() == 1)
   {
      InputToHashTable(argsBoard,
         argsGeneralMoves,
         argiAlpha,
         argiBeta,
         iScore, // Use the actual score of the move
         &vsMoveList[iMoveIndex]);
   }
#endif

   return iScore;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The function does a nega max search using a recursive search. (Refactored Version)
//
int Search(Board* argsBoard,
   GeneralMove* argsGeneralMoves,
   int argiAlpha,
   int argiBeta)
{

   // Debug the inputs.
   assert(argsBoard >= 0 );
   assert(argsGeneralMoves > 0);
   assert(CheckBoard(argsBoard));

   // Allocations.
   //Move* vsMoveList = new Move[dNumberOfMoves];
   //int* vsiMoveOrder = new int[dNumberOfMoves];
   //int* viMoveScore = new int[dNumberOfMoves];
   Move* vsMoveList = gvsMoveList[argsBoard->iNumberOfPlys + 1];
   int* vsiMoveOrder = gvsiMoveOrder[argsBoard->iNumberOfPlys + 1];
   int* viMoveScores = gviMoveScore[argsBoard->iNumberOfPlys + 1];
   int iMoveCount;
   int iScore;
   int siHaveMove = 0;
   int iBestMove = 128;
   int iNullScore = 0;
   int iLMRSearch = 0;
   int iSearchExtensions = 0;
   int iNewCheck = 1; // This is used to efficiently look for search extesions
   int iVerifiedSearch = 0;
   int iMaxPlysOld = 0;
   int oldHash = 0;
   int iRemainingDepth = 0;
   int iQSearch = 0;
   int iSearchFlag = 1;
   int iNumberOfAttacks = 0;
   int iNumberOfQuiets = 0;
   int iNumberOfMoves;
   int iAttacksSearchFlag = 0;
   int iZeroAttacks = 0;

   // Initialize the score.
   iScore = 0;

   // Store the max depth as it might be changed with the null search.
   iMaxPlysOld = argsBoard->iMaxPlys;

   // Initialize the ply length.
   argsBoard->viPrincipalVariationLength[argsBoard->iNumberOfPlys + 1] = argsBoard->iNumberOfPlys + 1;

   // Look for the case where we have already found checkmate
   if (argiAlpha >= -dMate ||
      argiBeta <= dMate)
   {

      return -dMate;

   }

# if defined( dUseHash )
   // See if this has been in the hash table before.

   ExtractFromHashTable(argsBoard,
      argsGeneralMoves);

   // If found at the appropriate depth, return the score.
   if ((GetQueryState() == 1))
   {

      return GetScoreHash();

   }

# endif

   // Null Search
   // First step make sure the null move is turned on.
   if (GetUseNullMove())
   {

      if (DoNullSearch( argsBoard,
                        argsGeneralMoves,
                        argiAlpha,
                        argiBeta,
                        iScore,
                        vsMoveList ))
      {
         // --- Efficient In-Place Null Move Search ---
         gsSearchParameters.iTried++;

         // 1. Make the null move on the current board
         int iOldLastMoveNull = argsBoard->iLastMoveNull;
         argsBoard->iLastMoveNull = dYes;
         SwitchSideToMove(argsBoard); // This also updates the hash for side-to-move

         // 2. Reduce depth and perform the search
         int iNullReduction = GetNullReduction();
         argsBoard->iMaxPlys -= iNullReduction;

         iNullScore = -Search(argsBoard,
            argsGeneralMoves,
            -argiBeta,
            -argiBeta + 1);

         // 3. Unmake the null move to restore the board state
         argsBoard->iMaxPlys += iNullReduction; // Restore depth
         SwitchSideToMove(argsBoard); // Switch side back
         argsBoard->iLastMoveNull = iOldLastMoveNull;


         // Look for a cut off
         if (iNullScore >= argiBeta)
         {
            // Null move pruning successful!
            // Optional: implement verification search for zugzwang positions
            // For now, just return beta
            
            // Consider verification search in endgames
            if ( argsBoard->iUseNullVerification == dYes )
            {
               // Could do a shallow verification search here
               // This would help detect zugzwang positions
               // For now, we trust the null move result
            }
            
            return argiBeta;

         }
         else
         {
            //Keep track of the failures.
            gsSearchParameters.iFailed++;
         }

      }

   }

   // Set a default numberOf Moves
   iNumberOfMoves = 1;
   iMoveCount = -1;

   // Loop over the moves.
   while (iSearchFlag)
   {

      // Increment the move count.
      iMoveCount++;

      assert(iMoveCount < dNumberOfMoves);

      // Calculate the attacks first, then the quiets.  This is done to conserve resources and only calculate moves as they are needed.
      // First Calculate the attacks.
      if (iMoveCount == 0 && iAttacksSearchFlag == 0)
      {
         // Generate all of the legal moves.
         CalculateAttacks(vsMoveList, argsBoard, argsGeneralMoves);
         // Check the state of the board and see if it is legal.
         LegalState(argsBoard, argsGeneralMoves);
         // If the move is not legal, undo the move and continue.
         if (argsBoard->siLegalMove == 0)
         {
            // return because we found an illegal move.
            return -dAlpha;
         }
         // let the number of moves live in a local variable.
         iNumberOfAttacks = argsBoard->siNumberOfMoves;
         // In the case of no attacks, make sure the quiets are calculated
         if (iNumberOfAttacks == 0)
         {
            iZeroAttacks = 1;
         }
         // Sort the moves to take the best bet.
         SortMovesHash( vsiMoveOrder, 
                        vsMoveList, 
                        iNumberOfAttacks, 
                        argsBoard, 
                        argsGeneralMoves, 
                        viMoveScores );
      }

      // Calculate the quiet moves if required
      if ((iMoveCount >= iNumberOfAttacks && iAttacksSearchFlag == 0) ||
         (iMoveCount == 0 && iZeroAttacks == 1))
      {
         // Mark that all attacks have been searched.
         iAttacksSearchFlag = 1;
         // Make sure we don't recalculate the moves because we think we have zero attacks.
         iZeroAttacks = 0;
         // Generate all of the legal moves.
         CalculateQuiets(vsMoveList, argsBoard, argsGeneralMoves);
         // let the number of moves live in a local variable.
         iNumberOfQuiets = argsBoard->siNumberOfMoves;
         // Reset the move count.
         iMoveCount = 0;
         // Sort the moves to take the best bet.
         SortMovesHash( vsiMoveOrder, 
                        vsMoveList, 
                        iNumberOfQuiets, 
                        argsBoard, 
                        argsGeneralMoves, 
                        viMoveScores );
      }

      // Determine the current list size and check if we are done
      int currentMoveListSize = (iAttacksSearchFlag == 0) ? iNumberOfAttacks : iNumberOfQuiets;
      if (iMoveCount >= currentMoveListSize) {
         iSearchFlag = 0; // Set flag to exit loop
         continue;
      }

      // Update the move order, to be used in DoNullMove.
      argsBoard->iMoveOrder = iMoveCount;

      // Update the state of the game.
      Update(argsBoard, argsGeneralMoves);

      // Let the interface take a look at the search.
      InterfaceControl(argsBoard);

      // Stop if the interface says to.
      if (GetStopGo() == dStop)
      {
         argsBoard->iMaxPlys = iMaxPlysOld;
         return argiAlpha;
      }

      // Call the helper function to search the move and update alpha
      iScore = SearchMove( argsBoard,
                           argsGeneralMoves,
                           vsMoveList,
                           vsiMoveOrder[ iMoveCount ],
                           argiAlpha,
                           argiBeta,
                           iMoveCount,
                           viMoveScores,
                           siHaveMove );

      // Look for a beta cut off. (alpha was already updated by reference in SearchMove)
      if (argiAlpha >= argiBeta)
      {
         // This is a killer move so update the heuristic.
         UpdateKillerMoves( argsBoard, 
                            &vsMoveList[vsiMoveOrder[iMoveCount]]);

         // Return the cut off.
         argsBoard->iMaxPlys = iMaxPlysOld;
         return argiBeta;
      }

   } // End loop over moves

   // Set to the old depth in case a null move was done.
   argsBoard->iMaxPlys = iMaxPlysOld;

   // Look for checkmate, if there is no legal move and we have checkmate.
   if (siHaveMove == 0)
   {
      // Debugging routine.
      if (GetSearchMonitorTogel() == 1)
      {
         SearchMonitor(argsBoard,
            vsMoveList,
            argsGeneralMoves);
      }

      // Look for check and stale mates
      argiAlpha = LookForCheckAndStale( argsBoard,
                                        argsGeneralMoves );
   }


   // Re-input with the best move. This logic is debatable, as the hash entry
   // is already made inside SearchMove. It can be kept for ensuring the final
   // best move for the node has the most accurate flag (e.g. EXACT vs LOWERBOUND)
# if defined( dUseHash )
   if (GetUseHashTable() == 1 && iBestMove < dNumberOfMoves)
   {
      // Put the move into the hash table.
      InputToHashTable( argsBoard,
                        argsGeneralMoves,
                        argiAlpha,
                        argiBeta,
                        argiAlpha, // Final score for this node is alpha
                        &vsMoveList[ iBestMove ] );
   }
# endif

   // Return the score.
   return argiAlpha;
}

