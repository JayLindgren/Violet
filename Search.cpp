// Copyright 2006 by Jay Lindgren. All Rights Reserved.
//

#include "Functions.h"
#include "Structures.h"
#include "Definitions.h"

#include <iostream>

// If in deep mode, include the appropriate files
#if defined( dDeepMode )
   #include <omp.h>
#endif

using namespace std;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Define global variables for keeping track of the number of nodes counted.
// Global variables suck, but are awsome for allowing for Deep Violet.
// access to the table:
// Note that the scope for the globe variables is only this file.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Define the global continer for search parameters.
SearchParameters gsSearchParameters;
   

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
   struct Board * sOldBoard;
   sOldBoard = (Board * ) malloc( 1 * sizeof( Board ) );

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

         // See if the new move has been searched.
         if ( iBestMoveSearched == 1 )
         {

            // We have to use the old board and old score.
            argsBoard->iBestMove = iBestMove;

         }
         else // Use the old board and old score.
         {

            // If cut off by time, then use the old board
            * argsBoard = * sOldBoard;

            // Use the old score too.
            iScore = iOldScore;

         }

         free( sOldBoard );
         return iScore;

      }
      else
      {

         // Store the board in the old board.   
         * sOldBoard = * argsBoard;

         // Safe the old score.
         iOldScore = iScore;

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

      // Update the number of nodes searched in Tempus for use in opotimization of parameters
      SetNumberOfNodesSearched( GetNumberOfNodes() );

      // Update the folks at home.
      if ( GetInterfaceMode() == dConsole &&
           GetStopGo() == dGo &&
           GetConsoleOutput() == dYes )
      {

         cout << "Ply = " << iDepth + 1 << " Score = " << iScore << endl;
         cout << "max depth reached = " << argsBoard->iMaxPlysReached << endl;
         cout << "Number of nodes searched = " << GetNumberOfNodes() << endl;
         cout << "Maximum depth obtained = " << argsBoard->iMaxPlysReached << endl;
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

   cout << "Gobal Tried = " << gsSearchParameters.iTried << " Failed = " << gsSearchParameters.iFailed << endl;

   // Return the score.
   free( sOldBoard );
   return iScore;

}

//
//------------------------------------------------------------------------------------------------------------
//
// The function does a recursive nega max search.
//
int FirstSearch( Board * argsBoard, 
                 GeneralMove * argsGeneralMoves,
                 int argiAlpha,
                 int argiBeta,
                 int * argiBestMove,
                 int * argiBestMoveSearched )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Allocations.
   int iMoveCount;
   Move vsMoveList[ dNumberOfMoves ]; // stored locally due to the complexities of the recursive search
   int vsiMoveOrder[ dNumberOfMoves ];
   struct Board * vsBoard;
   vsBoard = (Board * ) malloc( dNumberOfMoves * sizeof( Board ) );
   int iBestBoard = -1;
   int iBoardIndex;
   int viAlpha[ dNumberOfMoves ];
   int viBeta[ dNumberOfMoves ];
   int viScore[ dNumberOfMoves ];
   char strUpdate[ 1280 ];
   int viMoveScores[ dNumberOfMoves ];
   char strMove[ 64 ];
   char strPV[ 640 ];
   int viLMRSearch[ dNumberOfMoves ];
   int viOldMaxPlys[ dNumberOfMoves ];
   int iHaveMove = 0;   
   int iStartTime[ dNumberOfMoves ];
   int iEndTime [ dNumberOfMoves ];
   int iAlphaTime[ dNumberOfMoves ];

   // Set the search depth to zero.
   argsBoard->iNumberOfPlys = -1;
   argsBoard->iMaxPlysReached = -1;
   
   // Set the null verification to yes.
   argsBoard->iUseNullVerification = dYes;   

   // Initialize the history stack.
   if ( argsBoard->iMoveHistory == -1 )
   {
   
      argsBoard->iMoveHistory = 0;

   }
   
   // Put the initial hash into the move history.  
   argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbHash = GetHash();
   
   // Initialize the ply length.
   argsBoard->viPrincipalVariationLength[ argsBoard->iNumberOfPlys + 1 ] = argsBoard->iNumberOfPlys + 1;

   // Look for a draw - function returns a if a three fold repetition is found.
   if ( LookForDraw( argsBoard,
                     argsGeneralMoves ) )
   {

      // Release the board memory.
      free( vsBoard );

      // Return a draw score.
      return 0;

   }
   
   // Generate all of the legal moves.
   CalculateMoves( vsMoveList, 
                   argsBoard, 
                   argsGeneralMoves );

   // Let the number of moves live in a local variable.
   int iNumberOfMoves = argsBoard->siNumberOfMoves;

   // Check the state of the board and see if it is legal.
   LegalState( argsBoard,
               argsGeneralMoves );

   // If the move is not legal, undo the move and continue.
   if ( argsBoard->siLegalMove == 0  )
   {

      // Update the principal variation. 
      CreatePV( argsBoard );  

      // Release the board memory.
      free( vsBoard );

      // return because we found an illegal move.
      return -dAlpha;

   }
   else
   {

# if defined( dUseHash )
      // See if the hash table can help.
      ExtractFromHashTable( argsBoard,
                            argsGeneralMoves );
   
      // If found at the appropriate depth, return the score.
      if ( ( GetQueryState() == 1 ) )
      {

         // Release the board memory.
         free( vsBoard );

         //PrintBoard( argsBoard->mBoard  );
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

      // Copy the boards and limits.

#if defined( dDeepMode )
      #pragma omp parallel for schedule ( dynamic )
#endif
      for ( iBoardIndex = 0; iBoardIndex <  argsBoard->siNumberOfMoves; iBoardIndex++ )
      {

         vsBoard[ iBoardIndex ] = * argsBoard;
         viAlpha[ iBoardIndex ] = argiAlpha;
         viBeta[ iBoardIndex ]  = argiBeta;
         viScore[ iBoardIndex ] = 0;
         viLMRSearch[ iBoardIndex ] = 0;
         viOldMaxPlys[ iBoardIndex ] = argsBoard->iMaxPlys;

      }

      // In performing a Younger Brother Wait method, the first search is not parallel.
      for ( iMoveCount = 0; iMoveCount < 1; iMoveCount++ )
      {

         iStartTime[ iMoveCount ] = clock();

         // Only do this if we have a go.
         if ( GetStopGo() == dGo )
         {

            // The move being searced (used in DoNullMove)
            vsBoard[ vsiMoveOrder[ iMoveCount ] ].iMoveOrder = iMoveCount;

            // Update the interface as to what move we are searching.
            // Get the time since the last move.
            if ( GetTimeSinceLastMoveUpdate() > dMoveUpdateTime &&
                 GetInterfaceMode() == dUCI )
            {

               CreateAlgebraicMove( strMove,
                                    vsMoveList,
                                    vsiMoveOrder[ iMoveCount ] );

               // Create the update string
               sprintf( strUpdate, 
                        "info currmove %s currmovenumber %d ", 
                        strMove,
                        iMoveCount );

               // Send the command to the interface.
               SendCommand( strUpdate );

            }

            // Make a move on the board.
            MakeMove( vsMoveList, 
                      & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                      argsGeneralMoves,
                      vsiMoveOrder[ iMoveCount ] );

            // Look for a draw - function returns a one if a three fold repetition is found.
            if ( LookForDraw( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                              argsGeneralMoves ) )
            {

               // Return a draw score.
               viScore[ vsiMoveOrder[ iMoveCount ] ] = GetValueOfDraw();

            }
            // Search deeper if needed.
            else if ( vsBoard[ vsiMoveOrder[ iMoveCount ] ].iNumberOfPlys < vsBoard[ vsiMoveOrder[ iMoveCount ] ].iMaxPlys - 1 )
            {

               // Save the old plys to see if we can do a reduced LMR search.
               viOldMaxPlys[ vsiMoveOrder[ iMoveCount ] ] = vsBoard[ vsiMoveOrder[ iMoveCount ] ].iMaxPlys;

               // Check for an LMR search.
               if ( DoLMRSearch( viMoveScores,
                                 iMoveCount,
                                 & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                 viAlpha[ vsiMoveOrder[ iMoveCount ] ] ) ) // Note, this reduces the plys.
               {

                  // Do the search.
                  // If pondering, do not use the global alpha beta
                  // If searching on Violet's time, use them.
                  //int iTest = GetPonder();
                  if ( GetPonder() == dNo )
                  {

                     viScore[ vsiMoveOrder[ iMoveCount ] ] = - Search( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                       argsGeneralMoves,
                                                                       - viAlpha[ vsiMoveOrder[ iMoveCount ] ] - 1,
                                                                       - viAlpha[ vsiMoveOrder[ iMoveCount ] ] ); // Just to see if we improve alpha */

                  }
                  else if ( GetPonder() == dYes )
                  {

                     viScore[ vsiMoveOrder[ iMoveCount ] ] = - Search( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                       argsGeneralMoves,
                                                                       - dBeta,
                                                                       - dAlpha );

                  }

                  // Restore the board to the old search depth.
                  vsBoard[ vsiMoveOrder[ iMoveCount ] ].iMaxPlys = viOldMaxPlys[ vsiMoveOrder[ iMoveCount ] ];

                  // See if we have a legal move.
                  if ( vsBoard[ vsiMoveOrder[ iMoveCount ] ].siLegalMove == 1 )
                  {

                     iHaveMove = 1;

                  }
                  else
                  {

                     // Undo the move.
                     UndoMove( & vsBoard[ vsiMoveOrder[ iMoveCount ] ], 
                               argsGeneralMoves );

                     continue;
                  
                  }
               
                  // Look for a cut off from a reduced search.
                  if ( viScore[ vsiMoveOrder[ iMoveCount ] ] > argiAlpha )
                  {

                     // If we have reached this point, a late move has given us an increase in alpha
                     // and we need to search it to a full depth.
                     // If pondering, do not use the global alpha beta
                     // If searching on Violet's time, use them.
                     if ( GetPonder() == dNo )
                     {

                        // Do the negamax thing.
                        if ( iMoveCount > 0 )
                        {
                           viScore[ vsiMoveOrder[ iMoveCount ] ] = - Search( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                             argsGeneralMoves,
                                                                             - viAlpha[ vsiMoveOrder[ iMoveCount ] ] - 1,
                                                                             - viAlpha[ vsiMoveOrder[ iMoveCount ] ] );

                           // If the search fails, redo it.
                           if ( viScore[ vsiMoveOrder[ iMoveCount ] ] > viAlpha[ vsiMoveOrder[ iMoveCount ] ] &&
                                viScore[ vsiMoveOrder[ iMoveCount ] ] > viBeta[ vsiMoveOrder[ iMoveCount ] ]  )
                           {

                              viScore[ vsiMoveOrder[ iMoveCount ] ] = - Search( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                                argsGeneralMoves,
                                                                                - viBeta[ vsiMoveOrder[ iMoveCount ] ],
                                                                                - viScore[ vsiMoveOrder[ iMoveCount ] ] );

                           }

                        }
                        else
                        {

                           viScore[ vsiMoveOrder[ iMoveCount ] ] = - Search( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                             argsGeneralMoves,
                                                                             - viBeta[ vsiMoveOrder[ iMoveCount ] ],
                                                                             - viAlpha[ vsiMoveOrder[ iMoveCount ] ] );

                        }

                     }
                     else
                     {

                        viScore[ vsiMoveOrder[ iMoveCount ] ] = - Search( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                          argsGeneralMoves,
                                                                          - dBeta,
                                                                          - dAlpha );

                     }

                     // See if we have a legal move.
                     if ( vsBoard[ vsiMoveOrder[ iMoveCount ] ].siLegalMove == 1 )
                     {

                        iHaveMove = 1;

                     }
                     else
                     {

                        // Undo the move.
                        UndoMove( & vsBoard[ vsiMoveOrder[ iMoveCount ] ], 
                                  argsGeneralMoves );

                        continue;
                     
                     }

                  }            
                                  
               }
               // Do a regular search.
               else
               {
                  
                  // If pondering, do not use the global alpha beta
                  // If searching on Violet's time, use them.
                  if ( GetPonder() == dNo )
                  {

                     // Do the negamax thing.
                     if ( iMoveCount > 0 )
                     {

                        viScore[ vsiMoveOrder[ iMoveCount ] ] = - Search( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                          argsGeneralMoves,
                                                                          - viAlpha[ vsiMoveOrder[ iMoveCount ] ] - 1,
                                                                          - viAlpha[ vsiMoveOrder[ iMoveCount ] ] );

                        // If the search fails, redo it.
                        if ( viScore[ vsiMoveOrder[ iMoveCount ] ] > viAlpha[ vsiMoveOrder[ iMoveCount ] ] &&
                             viScore[ vsiMoveOrder[ iMoveCount ] ] > viBeta[ vsiMoveOrder[ iMoveCount ] ]  )
                        {

                           viScore[ vsiMoveOrder[ iMoveCount ] ] = - Search( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                             argsGeneralMoves,
                                                                             - viBeta[ vsiMoveOrder[ iMoveCount ] ],
                                                                             - viScore[ vsiMoveOrder[ iMoveCount ] ] );

                        }

                        }
                        else
                        {

                           viScore[ vsiMoveOrder[ iMoveCount ] ] = - Search( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                             argsGeneralMoves,
                                                                             - viBeta[ vsiMoveOrder[ iMoveCount ] ],
                                                                             - viAlpha[ vsiMoveOrder[ iMoveCount ] ] );

                        }

                  }
                  else
                  {

                     // Do a normal window search.
                     viScore[ vsiMoveOrder[ iMoveCount ] ] = - Search( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                       argsGeneralMoves,
                                                                       - dBeta,
                                                                       - dAlpha );

                  }

                  // See if we have a legal move.
                  if ( vsBoard[ vsiMoveOrder[ iMoveCount ] ].siLegalMove == 1 )
                  {

                     iHaveMove = 1;

                  }
                  else
                  {

                     // Undo the move.
                     UndoMove( &vsBoard[ vsiMoveOrder[ iMoveCount ] ], 
                               argsGeneralMoves );

                     continue;
                        
                  }

               }
      
            }
            // Perfor a Q search if at depth.
            else
            {
                        
               // Note, the ponder vs. search does not matter here.
               // Do the quiet search.
               int iLocalBeta = argiBeta;
               int iLocalAlpha = argiAlpha;
               viScore[ vsiMoveOrder[ iMoveCount ] ] = - QuiesenceSearch( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                          argsGeneralMoves,
                                                                          - iLocalBeta,
                                                                          - iLocalAlpha );

               // See if we have a legal move.
               if ( vsBoard[ vsiMoveOrder[ iMoveCount ] ].siLegalMove == 1 )
               {

                  iHaveMove = 1;

               }
               else
               {

                  // Undo the move.
                  UndoMove( & vsBoard[ vsiMoveOrder[ iMoveCount ] ], 
                            argsGeneralMoves );

                  continue;
               
               }
                                  
            }

            // Undo the move.
            UndoMove( & vsBoard[ vsiMoveOrder[ iMoveCount ] ], 
                      argsGeneralMoves );

# if defined( dUseHash )
                     // Put the score into the hash table.
                     if ( GetUseHashTable() == 1 )
                     {

                        InputToHashTable( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                          argsGeneralMoves,
                                          viAlpha[ vsiMoveOrder[ iMoveCount ] ],
                                          viBeta[ vsiMoveOrder[ iMoveCount ] ],
                                          viScore[ vsiMoveOrder[ iMoveCount ] ],
                                          & vsMoveList[ vsiMoveOrder[ iMoveCount ] ] );

                     } // end of hash table
# endif

            // Only on the first search to we create a PV for every board.
            // This is only for support of the Mulit PV feature.
            CreatePV( & vsBoard[ vsiMoveOrder[ iMoveCount ] ] );  
    
            iAlphaTime[ iMoveCount ] = clock();

            // If in deep mode, only let one thread through at a time.
            { 

               //Don't do this if during the search we received a stop command
               // as the last move may not have been a full search.// Onle 
               if ( GetStopGo() == dGo )
               { 

                  // If the previous best move has been searched, set the flag to one.
                  // This is used in case a partial ply has been searched when the interface
                  // asks for a result.  Don't do this if during the search we received a stop command
                  // as the last move may not have been a full search.
                  if ( vsiMoveOrder[  iMoveCount ] == * argiBestMove )
                  {

                     * argiBestMoveSearched = 1;
         
                  }

                  // Update the control of the game.
                  Update( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                          argsGeneralMoves );

                  // Update the multiple PVs.
                  UpdateMultiPV( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                 argsGeneralMoves,
                                 viScore[ vsiMoveOrder[ iMoveCount ] ],
                                 vsiMoveOrder[ iMoveCount ],
                                 argsBoard->siNumberOfMoves );


                  // Look for a beta event.
                  if ( viBeta[ vsiMoveOrder[ iMoveCount ] ] < argiBeta )
                  {

                     // A new winner in the search for the perfect move!
                     // Normalize the scores.
                     for ( int iMoveCountBeta = 0; iMoveCountBeta < iNumberOfMoves; iMoveCountBeta++ )
                     {

                        viBeta[ iMoveCountBeta ] = viBeta[ vsiMoveOrder[ iMoveCount ] ];

                     }

                     // Reset the global beta
                     argiBeta = viBeta[ vsiMoveOrder[ iMoveCount ] ];

                  }
                  

                  // Do that Alpha Beta thing.
                  if ( viScore[ vsiMoveOrder[ iMoveCount ] ] > argiAlpha )
                  {

                     // A new winner in the search for the perfect move!
                     // Normalize the scores.
                     for ( int iMoveCountAlpha = 0; iMoveCountAlpha < iNumberOfMoves; iMoveCountAlpha++ )
                     {

                        viAlpha[ iMoveCountAlpha ] = viScore[ vsiMoveOrder[ iMoveCount ] ];

                     }

                     // Set the global alpha
                     argiAlpha = viScore[ vsiMoveOrder[ iMoveCount ] ];

                     // Update the interface with the score.
                     if ( GetInterfaceMode() == dUCI )
                     {

                        // Set the score for later use in the Tempus object.
                        SetScore( viScore[ vsiMoveOrder[ iMoveCount ] ] ); 

                        // Set up the PV string
                        strcpy( strPV, " " );

                        // Loop over the PV
                        for ( int iPVCount = 0; iPVCount < argsBoard->iMaxPlys - 1; iPVCount++ )
                        {

                           // Create the PV to send to the interface.
                           CreateAlgebraicMove( strMove,
                                                & argsBoard->vmPrincipalVariation[ 0 ][ iPVCount ],
                                                0 );

                           // Add the move to the PV.
                           strcat( strPV, strMove );

                        }

                        // Create the string.
// CJL - need to debug this statement in Fritz or arena.
                        strcpy( strUpdate, " " );
                        sprintf( strUpdate, 
                                 "info depth %d seldepth %d nodes %d pv %s score cp %d ", 
                                 argsBoard->iMaxPlys + 1, 
                                 argsBoard->iMaxPlysReached + 1, 
                                 (int)GetNumberOfNodes(),
                                 strPV,
                                 argiAlpha );

                        // Send the command to the interface.
                        SendCommand( strUpdate );

                     }

# if defined( dUseHash )
                     // Update the history heuristic.
                     if ( GetUseHashTable() )
                     {

                        UpdateHH( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                  & vsMoveList[ vsiMoveOrder[ iMoveCount ] ] );

                     }
# endif

                     // Copy the move history into the principal variation.
                     CreatePV( & vsBoard[ vsiMoveOrder[ iMoveCount ] ] );
                              
                     // Mark which board is the best.
                     iBestBoard = vsiMoveOrder[ iMoveCount ];
                     vsBoard[ vsiMoveOrder[ iMoveCount ] ].iBestMove = iBestBoard;
        
                  } // end of cut off            

               } // End of if GetStopGo = dGo // meaning we had a good stop

            // end if statement over StopGo
            }

         } // end of pragma critical

         iEndTime [ iMoveCount ] = clock();

      } // end of loop over moves



	   // Loop over the moves.
      // If in deep mode, parallelize this loop.
      iMoveCount = 0;
#if defined( dDeepMode )
      #pragma omp_set_num_threads( 2 )
      #pragma omp parallel for schedule( dynamic ) 
#endif
      for ( iMoveCount = 1; iMoveCount < iNumberOfMoves; iMoveCount++ )
      {

         iStartTime[ iMoveCount ] = clock();

         // Only do this if we have a go.
         if ( GetStopGo() == dGo )
         {

            // The move being searced (used in DoNullMove)
            vsBoard[ vsiMoveOrder[ iMoveCount ] ].iMoveOrder = iMoveCount;

            // Update the interface as to what move we are searching.
            // Get the time since the last move.
            if ( GetTimeSinceLastMoveUpdate() > dMoveUpdateTime &&
                 GetInterfaceMode() == dUCI )
            {

               CreateAlgebraicMove( strMove,
                                    vsMoveList,
                                    vsiMoveOrder[ iMoveCount ] );

#if defined( dDeepMode )
               //#pragma omp atomic
               #pragma omp critical
#endif
               {

                  // Create the update string
                  sprintf( strUpdate, 
                           "info currmove %s currmovenumber %d ", 
                           strMove,
                           iMoveCount );

                  // Send the command to the interface.
                  SendCommand( strUpdate );

               }

            }

            // Make a move on the board.
            MakeMove( vsMoveList, 
                      & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                      argsGeneralMoves,
                      vsiMoveOrder[ iMoveCount ] );

            // Look for a draw - function returns a one if a three fold repetition is found.
            if ( LookForDraw( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                              argsGeneralMoves ) )
            {

               // Return a draw score.
               viScore[ vsiMoveOrder[ iMoveCount ] ] = GetValueOfDraw();

            }
            // Search deeper if needed.
            else if ( vsBoard[ vsiMoveOrder[ iMoveCount ] ].iNumberOfPlys < vsBoard[ vsiMoveOrder[ iMoveCount ] ].iMaxPlys - 1 )
            {

               // Save the old plys to see if we can do a reduced LMR search.
               viOldMaxPlys[ vsiMoveOrder[ iMoveCount ] ] = vsBoard[ vsiMoveOrder[ iMoveCount ] ].iMaxPlys;

               // Check for an LMR search.
               if ( DoLMRSearch( viMoveScores,
                                 iMoveCount,
                                 & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                 viAlpha[ vsiMoveOrder[ iMoveCount ] ] ) ) // Note, this reduces the plys.
               {

                  // Do the search.
                  // If pondering, do not use the global alpha beta
                  // If searching on Violet's time, use them.
                  //int iTest = GetPonder();
                  if ( GetPonder() == dNo )
                  {

                     viScore[ vsiMoveOrder[ iMoveCount ] ] = - Search( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                       argsGeneralMoves,
                                                                       - viAlpha[ vsiMoveOrder[ iMoveCount ] ] - 1,
                                                                       - viAlpha[ vsiMoveOrder[ iMoveCount ] ] ); // Just to see if we improve alpha */

                  }
                  else if ( GetPonder() == dYes )
                  {

                     viScore[ vsiMoveOrder[ iMoveCount ] ] = - Search( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                       argsGeneralMoves,
                                                                       - dBeta,
                                                                       - dAlpha );

                  }

                  // Restore the board to the old search depth.
                  vsBoard[ vsiMoveOrder[ iMoveCount ] ].iMaxPlys = viOldMaxPlys[ vsiMoveOrder[ iMoveCount ] ];

                  // See if we have a legal move.
                  if ( vsBoard[ vsiMoveOrder[ iMoveCount ] ].siLegalMove == 1 )
                  {

                     iHaveMove = 1;

                  }
                  else
                  {

                     // Undo the move.
                     UndoMove( & vsBoard[ vsiMoveOrder[ iMoveCount ] ], 
                               argsGeneralMoves );

                     continue;
                  
                  }
               
                  // Look for a cut off from a reduced search.
                  if ( viScore[ vsiMoveOrder[ iMoveCount ] ] > argiAlpha )
                  {

                     // If we have reached this point, a late move has given us an increase in alpha
                     // and we need to search it to a full depth.
                     // If pondering, do not use the global alpha beta
                     // If searching on Violet's time, use them.
                     if ( GetPonder() == dNo )
                     {

                        viScore[ vsiMoveOrder[ iMoveCount ] ] = - Search( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                          argsGeneralMoves,
                                                                          - viBeta[ vsiMoveOrder[ iMoveCount ] ],
                                                                          - viAlpha[ vsiMoveOrder[ iMoveCount ] ] );
                     }
                     else
                     {

                        viScore[ vsiMoveOrder[ iMoveCount ] ] = - Search( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                          argsGeneralMoves,
                                                                          - dBeta,
                                                                          - dAlpha );

                     }

                     // See if we have a legal move.
                     if ( vsBoard[ vsiMoveOrder[ iMoveCount ] ].siLegalMove == 1 )
                     {

                        iHaveMove = 1;

                     }
                     else
                     {

                        // Undo the move.
                        UndoMove( & vsBoard[ vsiMoveOrder[ iMoveCount ] ], 
                                  argsGeneralMoves );

                        continue;
                     
                     }

                  }            
                                  
               }
               // Do a regular search.
               else
               {
                  
                  // If pondering, do not use the global alpha beta
                  // If searching on Violet's time, use them.
                  if ( GetPonder() == dNo )
                  {

                     viScore[ vsiMoveOrder[ iMoveCount ] ] = - Search( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                       argsGeneralMoves,
                                                                       - viBeta[ vsiMoveOrder[ iMoveCount ] ],
                                                                       - viAlpha[ vsiMoveOrder[ iMoveCount ] ] );

                  }
                  else
                  {

                     viScore[ vsiMoveOrder[ iMoveCount ] ] = - Search( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                       argsGeneralMoves,
                                                                       - dBeta,
                                                                       - dAlpha );

                  }

                  // See if we have a legal move.
                  if ( vsBoard[ vsiMoveOrder[ iMoveCount ] ].siLegalMove == 1 )
                  {

                     iHaveMove = 1;

                  }
                  else
                  {

                     // Undo the move.
                     UndoMove( &vsBoard[ vsiMoveOrder[ iMoveCount ] ], 
                               argsGeneralMoves );

                     continue;
                        
                  }

               }
      
            }
            // Perfor a Q search if at depth.
            else
            {
                        
               // Note, the ponder vs. search does not matter here.
               // Do the quiet search.
               int iLocalAlpha = argiAlpha;
               int iLocalBeta  = argiBeta;
               viScore[ vsiMoveOrder[ iMoveCount ] ] = - QuiesenceSearch( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                                                          argsGeneralMoves,
                                                                          - iLocalBeta,
                                                                          - iLocalAlpha );

               // See if we have a legal move.
               if ( vsBoard[ vsiMoveOrder[ iMoveCount ] ].siLegalMove == 1 )
               {

                  iHaveMove = 1;

               }
               else
               {

                  // Undo the move.
                  UndoMove( & vsBoard[ vsiMoveOrder[ iMoveCount ] ], 
                            argsGeneralMoves );

                  continue;
               
               }
                                  
            }

            // Undo the move.
            UndoMove( & vsBoard[ vsiMoveOrder[ iMoveCount ] ], 
                      argsGeneralMoves );

# if defined( dUseHash )
                     // Put the score into the hash table.
                     if ( GetUseHashTable() == 1 )
                     {

                        InputToHashTable( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                          argsGeneralMoves,
                                          viAlpha[ vsiMoveOrder[ iMoveCount ] ],
                                          viBeta[ vsiMoveOrder[ iMoveCount ] ],
                                          viScore[ vsiMoveOrder[ iMoveCount ] ],
                                          & vsMoveList[ vsiMoveOrder[ iMoveCount ] ] );

                     } // end of hash table
# endif

            // Only on the first search to we create a PV for every board.
            // This is only for support of the Mulit PV feature.
            CreatePV( & vsBoard[ vsiMoveOrder[ iMoveCount ] ] );  
    
            iAlphaTime [ iMoveCount ] = clock();

            // If in deep mode, only let one thread through at a time.
#if defined( dDeepMode )
            #pragma omp critical // only let this be executed one thread at a time.
#endif
            { 

               //Don't do this if during the search we received a stop command
               // as the last move may not have been a full search.// Onle 
               if ( GetStopGo() == dGo )
               { 

                  // If the previous best move has been searched, set the flag to one.
                  // This is used in case a partial ply has been searched when the interface
                  // asks for a result.  Don't do this if during the search we received a stop command
                  // as the last move may not have been a full search.
                  if ( vsiMoveOrder[  iMoveCount ] == * argiBestMove )
                  {

                     * argiBestMoveSearched = 1;
         
                  }

                  // Update the control of the game.
                  Update( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                          argsGeneralMoves );

                  // Update the multiple PVs.
                  UpdateMultiPV( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                 argsGeneralMoves,
                                 viScore[ vsiMoveOrder[ iMoveCount ] ],
                                 vsiMoveOrder[ iMoveCount ],
                                 argsBoard->siNumberOfMoves );


                  // Look for a beta event.
                  if ( viBeta[ vsiMoveOrder[ iMoveCount ] ] < argiBeta )
                  {

                     // A new winner in the search for the perfect move!
                     // Normalize the scores.
                     for ( int iMoveCountBeta = 0; iMoveCountBeta < iNumberOfMoves; iMoveCountBeta++ )
                     {

                        viBeta[ iMoveCountBeta ] = viBeta[ vsiMoveOrder[ iMoveCount ] ];

                     }

                     // Reset the global beta
                     argiBeta = viBeta[ vsiMoveOrder[ iMoveCount ] ];

                  }
                  

                  // Do that Alpha Beta thing.
                  if ( viScore[ vsiMoveOrder[ iMoveCount ] ] > argiAlpha )
                  {

                     // A new winner in the search for the perfect move!
                     // Normalize the scores.
                     for ( int iMoveCountAlpha = 0; iMoveCountAlpha < iNumberOfMoves; iMoveCountAlpha++ )
                     {

                        viAlpha[ iMoveCountAlpha ] = viScore[ vsiMoveOrder[ iMoveCount ] ];

                     }

                     // Set the global alpha
                     argiAlpha = viScore[ vsiMoveOrder[ iMoveCount ] ];

                     // Update the interface with the score.
                     if ( GetInterfaceMode() == dUCI )
                     {

                        // Set the score for later use in the Tempus object.
                        SetScore( viScore[ vsiMoveOrder[ iMoveCount ] ] ); 

                        // Set up the PV string
                        strcpy( strPV, " " );

                        // Loop over the PV
                        for ( int iPVCount = 0; iPVCount < argsBoard->iMaxPlys - 1; iPVCount++ )
                        {

                           // Create the PV to send to the interface.
                           CreateAlgebraicMove( strMove,
                                                & argsBoard->vmPrincipalVariation[ 0 ][ iPVCount ],
                                                0 );

                           // Add the move to the PV.
                           strcat( strPV, strMove );

                        }

                        // Create the string.
// CJL - need to debug this statement in Fritz or arena.
                        strcpy( strUpdate, " " );
                        sprintf( strUpdate, 
                                 "info depth %d seldepth %d nodes %d pv %s score cp %d ", 
                                 argsBoard->iMaxPlys + 1, 
                                 argsBoard->iMaxPlysReached + 1, 
                                 (int)GetNumberOfNodes(),
                                 strPV,
                                 argiAlpha );

                        // Send the command to the interface.
                        SendCommand( strUpdate );

                     }

# if defined( dUseHash )
                     // Update the history heuristic.
                     if ( GetUseHashTable() )
                     {

                        UpdateHH( & vsBoard[ vsiMoveOrder[ iMoveCount ] ],
                                  & vsMoveList[ vsiMoveOrder[ iMoveCount ] ] );

                     }
# endif

                     // Copy the move history into the principal variation.
                     CreatePV( & vsBoard[ vsiMoveOrder[ iMoveCount ] ] );
                              
                     // Mark which board is the best.
                     iBestBoard = vsiMoveOrder[ iMoveCount ];
                     vsBoard[ vsiMoveOrder[ iMoveCount ] ].iBestMove = iBestBoard;
        
                  } // end of cut off            

               } // End of if GetStopGo = dGo // meaning we had a good stop

            // end if statement over StopGo
            }

         } // end of pragma critical

         iEndTime [ iMoveCount ] = clock();

      } // end of loop over moves

   } //  end of if for legal move

   //cout << "Tried = " << giTried << " Failed = " << giFailed << " Zug = " << giZug <<endl;

   // Take care of the case where we didn't find a move.
   if ( iBestBoard > -1 )
   {

      * argsBoard = vsBoard[ iBestBoard ];

   }
   else
   {
         
      // Look for check and stale mates
      argiAlpha = LookForCheckAndStale( argsBoard,
                                        argsGeneralMoves );

      // Copy the move history into the principal variation.
      CreatePV( argsBoard );

   }

   // Release the board memory.
   free( vsBoard );

   // Return the score.
   return argiAlpha;
       
}

//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void Update( struct Board * argsBoard,
             struct GeneralMove * argsGeneralMoves )
{
// Do the time control yada yada.

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Update the number of nodes counted.
   {
      BitBoard bbNumberOfNodes = GetNumberOfNodes();
      bbNumberOfNodes++;
      SetNumberOfNodes( bbNumberOfNodes );
      IncrementPollingCount();

      BitBoard bbNumberOfIncrementalNodes = GetNumberOfIncrementalNodes();
      bbNumberOfIncrementalNodes++;
      SetNumberOfIncrementalNodes( bbNumberOfIncrementalNodes );

      int iStartTime = GetSearchStartTime();
      int iClock = clock();
      int iSearchTime = GetSearchTimeInMiliSeconds();

      // Do some search time control.
      if ( GetSearchTimeInMiliSeconds() < ( clock() - GetSearchStartTime() ) )
      {

         SetStopGo( dStop );

      }

      // See if there is input from the GUI
      if ( GetInterfaceMode() == dUCI &&
           GetPollingCount() >= dPollingCount )
      {

         // Reset the polling count.
         SetPollingCount( 0 );

         // Poll for input.
         if ( CheckForInput() )
         {

            // See if we can process the command now.
            ReadInputAndExecute( argsBoard,
                                 argsGeneralMoves );

         }

      }

      // Look for a stop by number of notes
      if ( GetNodes() > 0 )
      {

         // If the number of search notes is larger than the node count, then bail out.
         if ( GetNodes() < bbNumberOfNodes )
         {

            // Set the stop.
            SetStopGo( dStop );

         }

      }
      

      // If at an incremental amount of nodes, let the folks at home know what is going on.
      if ( bbNumberOfIncrementalNodes == GetNodeCheck() )
      {

         // Calculate the nodes per second.
         SetEnd( clock() );
         double duration = (double)( GetEnd() - GetStart() ) / CLOCKS_PER_SEC;
         double rate = (double)( bbNumberOfIncrementalNodes ) / duration;
         SetStart( clock() );

         if ( GetInterfaceMode() == dConsole )
         {

            cout << "node Count = " << bbNumberOfIncrementalNodes << " nodes/sec = " << (int)rate << endl;

         }
         if ( GetInterfaceMode() == dUCI &&
              GetStopGo() == dGo )
         {
 
            // Send an update to the interface.
            char strUpdate[ 640 ];
            int iNodes = GetNumberOfNodes();
            sprintf( strUpdate, 
                     "info nodes %d rate %i ", 
                     iNodes,
                     (int)rate );
            SendCommand( strUpdate );

        }

         SetNumberOfIncrementalNodes( 0 );

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
//----------------------------------------------------------------------------------
//
int QSortMoves( int * argvsiMoveOrder,
                struct Move * argvsMoveList,
                int argNumberOfMoves )
{
// This routine keeps only the captures.

   // Debug the inputs.
   assert( argvsiMoveOrder >= 0 );
   assert( argvsMoveList >= 0 );
   assert( argNumberOfMoves <= dNumberOfMoves );

   int iMoveIndex;
   int iCaptureCount = 0;
   int iBestScore = -1;
   int iSortFlag = 1;

   // Extract the score into it's own vector.
   for ( iMoveIndex = 0; iMoveIndex < argNumberOfMoves; iMoveIndex++ )
   {

      if ( argvsMoveList[ iMoveIndex ].iMoveType == dCapture )
      {

         iCaptureCount++;
         argvsiMoveOrder[ iCaptureCount - 1 ] = iMoveIndex;

      }

   }

   // Return the capture count.
   return iCaptureCount;

}

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
   sBoard = (Board * ) malloc( 1 * sizeof( Board ) );

   * sBoard = * argsBoard;
   sBoard->iMaxPlys = 1;
               
   // Do the search.
   int iScore = Search( sBoard,
                        argsGeneralMoves,
                        dAlpha,
                        dBeta );

   if ( sBoard->bbCheck & 16 ||
        sBoard->bbCheck & 32 )
   {
   
      cout << "Stalemate or draw." << endl << endl;
      free( sBoard );
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
         free( sBoard );
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
   free( sBoard );

}


//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The function does a nega max search using a recursive search.
//
int Search( Board * argsBoard, 
            GeneralMove * argsGeneralMoves,
            int argiAlpha,
            int argiBeta )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves > 0 );
   assert( CheckBoard( argsBoard ) );

   // Allocations.
   int iMoveCount;
   int iScore;
   int siHaveMove = 0;
   Move vsMoveList[ dNumberOfMoves ]; // stored locally due to the complexities of the recursive search
   int vsiMoveOrder[ dNumberOfMoves ];
   int iBestMove = 128;
   int iNullScore = 0;
   int viMoveScore[ dNumberOfMoves ];
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
   argsBoard->viPrincipalVariationLength[ argsBoard->iNumberOfPlys + 1 ] = argsBoard->iNumberOfPlys + 1;

   // Look for the case where we have already found checkmate
   if ( argiAlpha >= -dMate ||
        argiBeta  <= dMate )
   {

      return - dMate;

   }

# if defined( dUseHash )
   // See if this has been in the hash table before.

   ExtractFromHashTable( argsBoard,
                         argsGeneralMoves );
   
   // If found at the appropriate depth, return the score.
   if ( ( GetQueryState() == 1 ) )
   {

      return GetScoreHash();

   }

# endif

   // Null Search
   // First step make sure the null move is turned on.
   if ( GetUseNullMove() )
   {

      if ( DoNullSearch( argsBoard,
                         argsGeneralMoves,
                         argiAlpha,
                         argiBeta,
                         iScore,
                         vsMoveList ) )
      {

         // Create a local copy of the board.  This is important to preserve the old PV. This also cleans up the history and EP problem
         struct Board * sNullBoard;
         sNullBoard = (Board * ) malloc( 1 * sizeof( Board ) );
         * sNullBoard = * argsBoard;
         iVerifiedSearch = 0;

         // Update the number tried.
         gsSearchParameters.iTried++;

         // Extract the old LMR and hash states
         oldHash = GetUseHashTable(); 

         // Reduce the depth of the search.
         sNullBoard->iMaxPlys = argsBoard->iMaxPlys - GetNullReduction(); // check if the -1 is needed

         // Switch the side to move
         SwitchSideToMove( sNullBoard );

         // Perform the null search.  
         iNullScore = - Search( sNullBoard,
                                 argsGeneralMoves,
                                 - argiBeta,
                                 - argiBeta + 1 );

         // Free up the created board.
         free( sNullBoard );

         // Reset the hash.
         SetUseHashTable( oldHash ); 

         // Look for a cut off
         if ( iNullScore >= argiBeta )
         {

            // This does a verified search and searches less plys.  If not verified, just return beta.
            //cout << endl;
            //PrintBoard( argsBoard->mBoard );
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
   // for ( iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++ )
   while( iSearchFlag )
   {

      // Increment the move count.
      iMoveCount++;

      assert( iMoveCount < dNumberOfMoves );

      // Calculate the attackes first, then the quiets.  This is done to conserve resources and only calculate moves as they are needed.
      // First Calculate the attacks.
      if ( iMoveCount == 0 )
      {

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
            return -dAlpha;

         }

         // let the number of moves live in a local variable.
         iNumberOfAttacks = argsBoard->siNumberOfMoves;

         // In the case of no attacks, make sure the quites are calculated
         if ( iNumberOfAttacks == 0 )
         {

            iZeroAttacks = 1;

         }

         // Sort the moves to take the best bet.
         SortMovesHash( vsiMoveOrder,
                        vsMoveList,
                        iNumberOfAttacks,
                        argsBoard,
                        argsGeneralMoves,
                        viMoveScore );

      }

      // Calculate the quiet moves if required
      if ( ( iMoveCount > iNumberOfAttacks - 1 && iAttacksSearchFlag == 0 ) ||
           ( iMoveCount == 0                   && iZeroAttacks       == 1 ) )
      {

         // Mark that all attacks have been searched.
         iAttacksSearchFlag = 1;

         // Make sure we don't recalculate the moves because we think we have zero attacks.
         iZeroAttacks = 0;

         // Generate all of the legal moves.
         CalculateQuiets( vsMoveList, 
                          argsBoard, 
                          argsGeneralMoves );

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
                        viMoveScore );

      }

      // Stop the search if both the attacks and the quiets have been searched.
      if ( iMoveCount         >= iNumberOfQuiets - 1 &&
           iAttacksSearchFlag >= 1 )
      {
  
         // Jump out of the loop next time through
         iSearchFlag = 0;

      }



      // Update the move order, to be used in DoNullMove.
      argsBoard->iMoveOrder = iMoveCount;

      // Update the state of the game.
      Update( argsBoard,
               argsGeneralMoves );

      // Let the interface take a look at the search.
      InterfaceControl( argsBoard );

      // Stop if the interface says too.
      if ( GetStopGo() == dStop )
      {

         if ( argiAlpha < dAlpha )
         {

            argsBoard->iMaxPlys = iMaxPlysOld;
            return argiAlpha;

         }
         else
         {

            argsBoard->iMaxPlys = iMaxPlysOld;
            return iScore;

         }

      }

      // Futility Pruning.
      // This checks to see if all hope is lost.
      // The side must not be in check. there must be no captures
      // And the material count must be below alpha plus a margin(depth)
      // Do the cheap checks first.
      iRemainingDepth = argsBoard->iMaxPlys - argsBoard->iNumberOfPlys;
      
      // See if we should do a futility prune.
      if ( iRemainingDepth <= 6 &&
           iRemainingDepth >  0 &&
           dUseFutility == dYes &&
           ! ( argsBoard->bbWhitePawn & argsGeneralMoves->bbCol7 ) && // Make sure not pawns are close to promoting
           ! ( argsBoard->bbWhitePawn & argsGeneralMoves->bbCol6 ) &&
           ! ( argsBoard->bbBlackPawn & argsGeneralMoves->bbCol3 ) &&
           ! ( argsBoard->bbBlackPawn & argsGeneralMoves->bbCol2 ) &&
           ! ( argsBoard->bbCheck > 0 ) &&  // Make sure we aren't in check.
           iMoveCount > 0 ) // Make sure this isn't the firt move.
      {

         // Count the material
         iNullScore = EvaluateNull( argsBoard, argsGeneralMoves );
         if ( iNullScore < argiAlpha - 500 )
         {  
///*
            // Perform a Q search to check the score
            iQSearch = QuiesenceSearch( argsBoard,
                                          argsGeneralMoves,
                                          - argiBeta,
                                          - argiAlpha );

            // See if the quiesence search still obtains.
            if ( iQSearch < argiAlpha - 300 )
            {

               // If we get this far, most likely, all is lost.
               //PrintBoard( argsBoard->mBoard );
               return argiAlpha;

            }
            else
            {
   
               //PrintBoard( argsBoard->mBoard );
               //cout << "Here's the rub." << endl;

            }
//*/

            //PrintBoard( argsBoard->mBoard );
            //return argiAlpha;

         }

      }// End if over futility prucing.

      // Make a move on the board.
      MakeMove( vsMoveList, 
                argsBoard,
                argsGeneralMoves,
                vsiMoveOrder[ iMoveCount ] );

      // Debugging routine.
      if ( GetSearchMonitorTogel() )
      {

         SearchMonitor( argsBoard,
                        vsMoveList,
                        argsGeneralMoves );

      }

      // Look for a draw - function returns a if a three fold repetition is found.
      if ( LookForDraw( argsBoard,
                        argsGeneralMoves ) )
      {

         // Return a draw score.
         iScore = GetValueOfDraw();

      }
      // Check if we need to search deeper.
      else if ( argsBoard->iNumberOfPlys < argsBoard->iMaxPlys - 1 )
      {

         // Save the old plys to see if we can do a reduced LMR search.
         int iOldMaxPlys = argsBoard->iMaxPlys;

         // Do the LMR search  if applicable.
         if ( DoLMRSearch( viMoveScore,
                           iMoveCount,
                           argsBoard,
                           argiAlpha ) )
         {
      
            // Do the search.
            iScore = - Search( argsBoard,
                               argsGeneralMoves,
                               - argiAlpha - 1,
                               - argiAlpha );

            // Restore the board to the old search depth.
            argsBoard->iMaxPlys = iOldMaxPlys;

            // See if we have a legal move.
            if ( argsBoard->siLegalMove == 1 )
            {

               siHaveMove = 1;

            }
            else
            {

               // Undo the move.
               UndoMove( argsBoard, 
                         argsGeneralMoves );

               continue;
         
            }

            // Look for an LMR research
            if ( iScore > argiAlpha )
            {
      
               // Do a negamax search.  Make sure it is not the first search.
               if ( iMoveCount > 0 )
               {

                  // Do the search with a null window..
                  iScore = - Search( argsBoard,
                                     argsGeneralMoves,
                                     -argiAlpha - 1,
                                     -argiAlpha );

                  // If the scout search failed, redo the full search.
                  if ( iScore > argiAlpha &&
                       iScore < argiBeta )
                  {

                     // Do the search.
                     iScore = - Search( argsBoard,
                                        argsGeneralMoves,
                                        -argiBeta,
                                        -iScore );

                  }

               }
               else
               {

                  // Do the search.
                  iScore = - Search( argsBoard,
                                     argsGeneralMoves,
                                     -argiBeta,
                                     -argiAlpha );

               }

               // See if we have a legal move.
               if ( argsBoard->siLegalMove == 1 )
               {

                  siHaveMove = 1;

               }
               else
               {

                  // Undo the move.
                  UndoMove( argsBoard, 
                            argsGeneralMoves );

                  continue;
         
               }

            }

         }
         // If not LMR, do a normal search
         else
         {

            // Do a negamax search.  Make sure it is not the first search.
            if ( iMoveCount > 0 )
            {

               // Do the search with a null window..
               iScore = - Search( argsBoard,
                                  argsGeneralMoves,
                                  -argiAlpha - 1,
                                  -argiAlpha );

               // If the scout search failed, redo the full search.
               if ( iScore > argiAlpha &&
                    iScore < argiBeta )
               {

                  // Do the search.
                  iScore = - Search( argsBoard,
                                     argsGeneralMoves,
                                     -argiBeta,
                                     -iScore );

               }

            }
            else
            {

               // Do the search.
               iScore = - Search( argsBoard,
                                  argsGeneralMoves,
                                  -argiBeta,
                                  -argiAlpha );

            }

            // See if we have a legal move.
            if ( argsBoard->siLegalMove == 1 )
            {

               siHaveMove = 1;

            }
            else
            {

               // Undo the move.
               UndoMove( argsBoard, 
                         argsGeneralMoves );

               continue;
         
            }

         }

         // Look to see if we need to bail out.
         if ( GetStopGo() == dStop )
         {

            // Undo the move.
            UndoMove( argsBoard,
                        argsGeneralMoves );
               
            // Return a cut off.
            if ( argiAlpha < dAlpha )
            {

               // 
               argsBoard->iMaxPlys = iMaxPlysOld;
               return argiAlpha;

            }
            else
            {
               
               // 
               argsBoard->iMaxPlys = iMaxPlysOld;
               return iScore;

            }

         }

         // See if we have a legal move.
         if ( argsBoard->siLegalMove == 1 )
         {

            siHaveMove = 1;

         }
         else
         {

            // Undo the move.
            UndoMove( argsBoard, 
                      argsGeneralMoves );
            continue;
            
         }         

      }
      // if we are in check, search one move deeper.
      else if ( LookForSearchExtensions( argsBoard,
                                         argsGeneralMoves, 
                                         vsMoveList,
                                         vsiMoveOrder[  iMoveCount ],
                                         iNewCheck ) )
      {

         argsBoard->iMaxPlys++;

         // Do the nega scout thing.
         iScore = - Search( argsBoard,
                              argsGeneralMoves,
                              - argiAlpha - 1,
                              - argiAlpha );

         // If the nega scout failed, research.
         if ( iScore > argiAlpha &&
              iScore < argiBeta )
         {

            iScore = - Search( argsBoard,
                                 argsGeneralMoves,
                                 - argiBeta,
                                 - iScore );

         }

         argsBoard->iMaxPlys--;

         // See if we have a legal move.
         if ( argsBoard->siLegalMove == 1 )
         {

            siHaveMove = 1;

         }
         else
         {

            // Undo the move.
            UndoMove( argsBoard, 
                      argsGeneralMoves );

            continue;
         
         }

         // See if we have a legal move.
         if ( argsBoard->siLegalMove == 1 )
         {

            siHaveMove = 1;

         }

      }
      // Perform a quiesence search.
      else
      {

         // If we are at depth, find a quiet mind.
         // Do the negascout thing
         iScore = - QuiesenceSearch( argsBoard,
                                     argsGeneralMoves,
                                     - argiAlpha - 1,
                                     - argiAlpha );

         // See if the scout failed.
         if ( iScore > argiAlpha &&
              iScore < argiBeta )
         {

            iScore = - QuiesenceSearch( argsBoard,
                                        argsGeneralMoves,
                                        - argiBeta,
                                        - iScore );

         }

         // See if we have a legal move.
         if ( argsBoard->siLegalMove == 1 )
         {

            siHaveMove = 1;

         }
         else
         {

            // Undo the move.
            UndoMove( argsBoard, 
                      argsGeneralMoves );

            continue;
            
         }
       
         // Mark that we have a move at this ply
         if ( argsBoard->siLegalMove == 1 )
         {

            siHaveMove = 1;

         }

      }  

      // Undo the move.
      UndoMove( argsBoard, 
                argsGeneralMoves );


      // Set that a real search has been done.
      argsBoard->iLastMoveNull = dNo;

# if defined( dUseHash )
         if ( GetUseHashTable() == 1 )
         {

            // Put the move into the hash table.
            InputToHashTable( argsBoard,
                              argsGeneralMoves,
                              argiAlpha,
                              argiBeta,
                              iScore,
                              & vsMoveList[ vsiMoveOrder[ iMoveCount ] ] );
          }
# endif

      // Do that Alpha Beta thing.
      if ( iScore > argiAlpha )
      {  

         // A new winner in the search for the perfect move!t
         argiAlpha = iScore;

         // Update the history heuristic.
         UpdateHH( argsBoard,
                   & vsMoveList[ vsiMoveOrder[ iMoveCount ] ] );

         // Copy the move history into the principal variation.
         CreatePV( argsBoard );

         // Set the best move for the hash table.
         argsBoard->iBestMove = vsiMoveOrder[ iMoveCount ];
         iBestMove = vsiMoveOrder[ iMoveCount ];
 
         // Look for a cut off.
         if ( iScore >= argiBeta )
         {
              
            // This is a killer move so update the heuristic.
            UpdateKillerMoves( argsBoard,
                               & vsMoveList[ vsiMoveOrder[ iMoveCount ] ] );

# if defined( dUseHash )
            if ( GetUseHashTable() == 1 )
            {

               // Put the move into the hash table.
               InputToHashTable( argsBoard,
                                 argsGeneralMoves,
                                 argiAlpha,
                                 argiBeta,
                                 iScore,
                                 & vsMoveList[ vsiMoveOrder[ iMoveCount ] ] );
             }
# endif
  
            // Return the cut off.              
            argsBoard->iMaxPlys = iMaxPlysOld;
            return argiBeta;
   
         }
                    
      } // End alpha/beta if

   } // End loop over moves

   // Set to the old depth in case a null move was done.
   argsBoard->iMaxPlys = iMaxPlysOld;


      // Look for checkmate, if there is no legal move and we have checkmate.
   if ( siHaveMove == 0 )
   {

      // Debugging routine.
      if ( GetSearchMonitorTogel() == 1 )
      {

         SearchMonitor( argsBoard,
                        vsMoveList,
                        argsGeneralMoves );

      }

         
      // Look for check and stale mates
      argiAlpha = LookForCheckAndStale( argsBoard,
                                        argsGeneralMoves );

   }

// Reinput with the best move.
# if defined( dUseHash )
         if ( GetUseHashTable() == 1 )
         {

            // Put the move into the hash table.
            InputToHashTable( argsBoard,
                              argsGeneralMoves,
                              argiAlpha,
                              argiBeta,
                              iScore,
                              & vsMoveList[ vsiMoveOrder[ iMoveCount ] ] );
          }
# endif

   // Return the score.
   return argiAlpha;
       
}

//
//-------------------------------------------------------------
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
   int iMoveCount;
   int iScore;
   int siHaveMove = 0;
   Move vsMoveList[ dNumberOfMoves ];
   int vsiMoveOrder[ dNumberOfMoves ];
   int iNumberOfCaptures;

   // Initialize the score.
   iScore = 0;

   // Look for a special case.
   if ( argiAlpha == argiBeta &&
        GetAspirationSearch() )
   {

      return argiAlpha;

   }

/*
   cout << endl << endl;
   cout << "Depth = " << argsBoard->iNumberOfPlys << endl;
   cout << "alpha = " << argiAlpha << endl;
   cout << "beta  = " << argiBeta << endl;
   PrintBoard( argsBoard->mBoard );
*/
   
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
   
      // No PV can be created in the Q search, only the main.
      // Copy the move history into the principal variation.
      //CreatePV( argsBoard );

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

   // Return the score.
   return argiAlpha;

}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void SortMoves( int * argvsiMoveOrder,
                struct Move * argvsMoveList,
                int argNumberOfMoves )
{
// The moves get assigned a score during their creation.
// This routine simply sorts that score.
// See the Calculate Moves routines for the score creation.

   // Debug the inputs.
   assert( argvsiMoveOrder >= 0 );
   assert( argvsMoveList > 0 );
   assert( argNumberOfMoves >= 0 );
   assert( argNumberOfMoves <= dNumberOfMoves );
   
   int iMoveIndex;
   int iBestScore = -1;
   int iSortFlag = 1;
   int viScore[ dNumberOfMoves ];

   // Extract the score into it's own vector.
   for ( iMoveIndex = 0; iMoveIndex < argNumberOfMoves; iMoveIndex++ )
   {

      viScore[ iMoveIndex ] = argvsMoveList[ iMoveIndex ].iScore;
      argvsiMoveOrder[ iMoveIndex ] = iMoveIndex;

   }
   
   while( iSortFlag )
   {

      // Set the default to bail.
      iSortFlag = 0;

      // Use a cocktail sort and to from top to bottom
      for ( iMoveIndex = 0; iMoveIndex < argNumberOfMoves - 1; iMoveIndex++ )
      {

         if ( viScore[ iMoveIndex + 1 ] > viScore[ iMoveIndex ] )
         {

            int iDummyScore           = viScore[ iMoveIndex ];
            viScore[ iMoveIndex ]     = viScore[ iMoveIndex + 1 ];
            viScore[ iMoveIndex + 1 ] = iDummyScore;

            int iDummyPosition                = argvsiMoveOrder[ iMoveIndex ];
            argvsiMoveOrder[ iMoveIndex ]     = argvsiMoveOrder[ iMoveIndex + 1 ];
            argvsiMoveOrder[ iMoveIndex + 1 ] = iDummyPosition;

            iSortFlag = 1;

         }

      }
           
   }

}

// Sort the moves based on what is found in the hash table.
void SortMovesHash( int * argvsiMoveOrder,
                    struct Move * argvsMoveList,
                    int argiNumberOfMoves,
                    struct Board * argsBoard,
                    struct GeneralMove * argsGeneralMoves,
                    int * viScore )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves > 0 );
   assert( argiNumberOfMoves >= 0 );
   assert( argiNumberOfMoves <= dNumberOfMoves );
   assert( argvsMoveList >=0 );

   // This function assumes that all of the nodes are stored in the hash table and sorts
   // the moves according to the previous score.
   int iSortFlag = 1;
   //int viScore[ dNumberOfMoves ];
   int iMoveIndex;
   int iBestScore = -1;
   
   // Loop over the moves and assign a score.
   for ( int iMoveCount = 0; iMoveCount < argiNumberOfMoves; iMoveCount++ )
   {

      // Set an initial move order.
      argvsiMoveOrder[ iMoveCount ] = iMoveCount;

      // Assign the move score:
      viScore[ iMoveCount ] = argvsMoveList[ iMoveCount ].iScore;

      // Update the score for the History Heuristic.
      viScore[ iMoveCount ] += UpdateScoreHH( & argvsMoveList[ iMoveCount ],
                                              argsGeneralMoves );

      // Update the score for the Killer Moves.
      viScore[ iMoveCount ] += UpdateScoreKillerMoves( argsBoard,
                                                       & argvsMoveList[ iMoveCount ],
                                                       argsGeneralMoves );

      // Look for a move in the principal variation
      // Make sure we are before the frontier.
      if ( argsBoard->iNumberOfPlys + 1 < argsBoard->iMaxPlys - 1 )
      {

         if ( ( argsBoard->vmPrincipalVariation[ 0 ][ argsBoard->iNumberOfPlys + 1  ].iFromSquare == argvsMoveList[ iMoveCount ].iFromSquare  ) &&
              ( argsBoard->vmPrincipalVariation[ 0 ][ argsBoard->iNumberOfPlys + 1  ].iToSquare == argvsMoveList[ iMoveCount ].iToSquare  ) )
         {

            // Give the move a principal variation score.
            viScore[ iMoveCount ] += argsGeneralMoves->msPVMove;

         }

      }

      // Debug the squares
      assert( argvsMoveList[ iMoveCount ].iToSquare >= dA1 );
      assert( argvsMoveList[ iMoveCount ].iToSquare <= dH8 );
      assert( argvsMoveList[ iMoveCount ].iFromSquare >= dA1 );
      assert( argvsMoveList[ iMoveCount ].iFromSquare <= dH8 );
      assert( argvsMoveList[ iMoveCount ].iPiece >= dWhitePawn );
      assert( argvsMoveList[ iMoveCount ].iPiece <= dBlackKing );

      // Look for change in placement
      switch ( argvsMoveList[ iMoveCount ].iPiece )
      {

         case dWhitePawn :
         {
   
            viScore[ iMoveCount ] += GetWhitePawnPlacementScore( argvsMoveList[ iMoveCount ].iToSquare );
            viScore[ iMoveCount ] -= GetWhitePawnPlacementScore( argvsMoveList[ iMoveCount ].iFromSquare );
            break;

         }
         case dBlackPawn :
         {
   
            viScore[ iMoveCount ] += GetBlackPawnPlacementScore( argvsMoveList[ iMoveCount ].iToSquare );
            viScore[ iMoveCount ] -= GetBlackPawnPlacementScore( argvsMoveList[ iMoveCount ].iFromSquare );
            break;

         }
         case dWhiteRook :
         {
   
            viScore[ iMoveCount ] += GetWhiteRookPlacementScore( argvsMoveList[ iMoveCount ].iToSquare );
            viScore[ iMoveCount ] -= GetWhiteRookPlacementScore( argvsMoveList[ iMoveCount ].iFromSquare );
            break;

         }
         case dBlackRook :
         {
   
            viScore[ iMoveCount ] += GetBlackRookPlacementScore( argvsMoveList[ iMoveCount ].iToSquare );
            viScore[ iMoveCount ] -= GetBlackRookPlacementScore( argvsMoveList[ iMoveCount ].iFromSquare );
            break;

         }
         case dWhiteKnight :
         {
   
            viScore[ iMoveCount ] += GetWhiteKnightPlacementScore( argvsMoveList[ iMoveCount ].iToSquare );
            viScore[ iMoveCount ] -= GetWhiteKnightPlacementScore( argvsMoveList[ iMoveCount ].iFromSquare );
            break;

         }
         case dBlackKnight :
         {
   
            viScore[ iMoveCount ] += GetBlackKnightPlacementScore( argvsMoveList[ iMoveCount ].iToSquare );
            viScore[ iMoveCount ] -= GetBlackKnightPlacementScore( argvsMoveList[ iMoveCount ].iFromSquare );
            break;

         }
         case dWhiteBishop :
         {
   
            viScore[ iMoveCount ] += GetWhiteBishopPlacementScore( argvsMoveList[ iMoveCount ].iToSquare );
            viScore[ iMoveCount ] -= GetWhiteBishopPlacementScore( argvsMoveList[ iMoveCount ].iFromSquare );
            break;

         }
         case dBlackBishop :
         {
   
            viScore[ iMoveCount ] += GetBlackBishopPlacementScore( argvsMoveList[ iMoveCount ].iToSquare );
            viScore[ iMoveCount ] -= GetBlackBishopPlacementScore( argvsMoveList[ iMoveCount ].iFromSquare );
            break;

         }
         case dWhiteQueen :
         {
   
            viScore[ iMoveCount ] += GetWhiteQueenPlacementScore( argvsMoveList[ iMoveCount ].iToSquare );
            viScore[ iMoveCount ] -= GetWhiteQueenPlacementScore( argvsMoveList[ iMoveCount ].iFromSquare );
            break;

         }
         case dBlackQueen :
         {
   
            viScore[ iMoveCount ] += GetBlackQueenPlacementScore( argvsMoveList[ iMoveCount ].iToSquare );
            viScore[ iMoveCount ] -= GetBlackQueenPlacementScore( argvsMoveList[ iMoveCount ].iFromSquare );
            break;

         }
         case dWhiteKing :
         {
   
            viScore[ iMoveCount ] += GetWhiteKingPlacementScore( argvsMoveList[ iMoveCount ].iToSquare );
            viScore[ iMoveCount ] -= GetWhiteKingPlacementScore( argvsMoveList[ iMoveCount ].iFromSquare );
            break;

         }
         case dBlackKing :
         {
   
            viScore[ iMoveCount ] += GetBlackKingPlacementScore( argvsMoveList[ iMoveCount ].iToSquare );
            viScore[ iMoveCount ] -= GetBlackKingPlacementScore( argvsMoveList[ iMoveCount ].iFromSquare );
            break;

         }

      }

   } 

# if defined( dUseHash )
   // Add the score for the best move from the hash table.
   if ( ( GetQueryState()  )&&
        ( GetBestMove() < 128 ) &&
        ( GetUseHashTable() ) )
   {

      // Add a best move score on.
      viScore[ GetBestMove() ] += argsGeneralMoves->msBestMove;

   }
# endif
   
   while( iSortFlag )
   {

      // Set the default to bail.
      iSortFlag = 0;

      // Use a cocktail sort and to from top to bottom
      for ( iMoveIndex = 0; iMoveIndex < argiNumberOfMoves - 1; iMoveIndex++ )
      {

         if ( viScore[ iMoveIndex + 1 ] > viScore[ iMoveIndex ] )
         {

            int iDummyScore           = viScore[ iMoveIndex ];
            viScore[ iMoveIndex ]     = viScore[ iMoveIndex + 1 ];
            viScore[ iMoveIndex + 1 ] = iDummyScore;

            int iDummyPosition                = argvsiMoveOrder[ iMoveIndex ];
            argvsiMoveOrder[ iMoveIndex ]     = argvsiMoveOrder[ iMoveIndex + 1 ];
            argvsiMoveOrder[ iMoveIndex + 1 ] = iDummyPosition;

            iSortFlag = 1;

         }

      }
           
   }

}

//
//-----------------------------------------------------------------------------------
//

void ResetHistoryHeuristic()
{

   int iSquareCount1;
   int iSquareCount2;

   // Set the count to zero.
   gsSearchParameters.iWhiteHHCount = 1;
   gsSearchParameters.iBlackHHCount = 1;


   // This history heuristic resets at the begining of each computer move.
   for ( iSquareCount1 = 0; iSquareCount1 < 64; iSquareCount1++ )
   {

      for ( iSquareCount2 = 0; iSquareCount2 < 64; iSquareCount2++ )
      {

         gsSearchParameters.mWhiteHistoryHeuristic[ iSquareCount1 ][ iSquareCount2 ] = 0;
         gsSearchParameters.mBlackHistoryHeuristic[ iSquareCount1 ][ iSquareCount2 ] = 0;

      }

   }

}

//
//-----------------------------------------------------------------------------------
//

void UpdateHH( struct Board * argsBoard,
               struct Move * argsMove )
{
// This funciton updates the history heuristic.

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsMove > 0 );

   // Update the matrix according to the color.
   if ( argsMove->iPiece < dBlackPawn )
   {
      
      gsSearchParameters.iWhiteHHCount++;
      gsSearchParameters.mWhiteHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ]++;
      
   }
   else
   {
      
      gsSearchParameters.iBlackHHCount++;
      gsSearchParameters.mBlackHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ]++;
 
   }   

}

//
//-----------------------------------------------------------------------------------
//

int UpdateScoreHH( struct Move * argsMove,
                   struct GeneralMove * argsGeneralMoves )
{
   int iScore = 0;

      // Add the HH to the score.
   if ( argsMove->iPiece < dBlackPawn &&
        gsSearchParameters.iWhiteHHCount > 0 )
   {

      iScore = int( double( argsGeneralMoves->msKillerMove ) * ( (double)gsSearchParameters.mWhiteHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ] /
                                                                 (double)gsSearchParameters.iWhiteHHCount ) );

   }
   else if ( gsSearchParameters.iBlackHHCount > 0 )
   {

       iScore = int( double( argsGeneralMoves->msKillerMove ) * ( (double)gsSearchParameters.mBlackHistoryHeuristic[ argsMove->iFromSquare ][ argsMove->iToSquare ] /
                                                                  (double)gsSearchParameters.iBlackHHCount ) );

   }
   else
   {

      iScore = 0;

   }

   // Some QA/QC
   assert( iScore >= 0 );
   assert( iScore <= argsGeneralMoves->msKillerMove );


   // Make sure we aren't walking on a best move.
   if ( iScore > argsGeneralMoves->msBestMove )
   {

      iScore = argsGeneralMoves->msBestMove - 2;

   }

   return iScore;

}

//
//-----------------------------------------------------------------------------------
//

void ResetKillerMoves()
{

   int iSquareCount1;
   int iSquareCount2;
   int iPlyCount;   

   // This history heuristic resets at the begining of each computer move.

   for ( iPlyCount = 0; iPlyCount < dNumberOfMoves; iPlyCount++ )
   {

      // Set the count at the ply level to one (one to avoid a divide by zero.
      gsSearchParameters.vKillerMoveCount[ iPlyCount ] = 1;

      for ( iSquareCount1 = 0; iSquareCount1 < 64; iSquareCount1++ )
      {
   
         for ( iSquareCount2 = 0; iSquareCount2 < 64; iSquareCount2++ )
         {

            gsSearchParameters.mKillerMove[ iPlyCount ][ iSquareCount1 ][ iSquareCount2 ] = 0;

         }
   
      }

   }

}

//
//-----------------------------------------------------------------------------------
//

void UpdateKillerMoves( struct Board * argsBoard,
                        struct Move * argsMove )
{
// This funciton updates the history heuristic.

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsMove > 0 );

   gsSearchParameters.mKillerMove[ argsBoard->iNumberOfPlys ][  argsMove->iFromSquare ][ argsMove->iToSquare ]++;
   gsSearchParameters.vKillerMoveCount[ argsBoard->iNumberOfPlys ]++;
   
}

//
//-----------------------------------------------------------------------------------
//

int UpdateScoreKillerMoves( struct Board * argsBoard,
                            struct Move * argsMove,
                            struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsMove > 0 );

   int iScore = 0;

   // Avoid a divide by zero.
   if ( gsSearchParameters.vKillerMoveCount[ argsBoard->iNumberOfPlys ] == 0 )
   {

      return 0;

   }

   // Calculate the score.
   iScore = int( double( argsGeneralMoves->msKillerMove ) * ( (double)gsSearchParameters.mKillerMove[ argsBoard->iNumberOfPlys ][  argsMove->iFromSquare ][ argsMove->iToSquare ] /
                                                              (double)gsSearchParameters.vKillerMoveCount[ argsBoard->iNumberOfPlys ] ) );
     
   // Some QA/QC
   assert( iScore >= 0 );
   assert( iScore <= sdKillerMove );

   // Make sure we aren't walking on a best move.
   if ( iScore > argsGeneralMoves->msBestMove )
   {

      iScore = argsGeneralMoves->msBestMove - 1;

   }

   return iScore;

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
// See if we should do the null search.
int DoNullSearch( struct Board * argsBoard,
                  struct GeneralMove * argsGeneralMoves,
                  int argiAlpha,
                  int argiBeta,
                  int iScore,
                  struct Move * argsMove )
{

   int iDoSearch = 0;
   Move vsMoveList[ dNumberOfMoves ]; // stored locally due to the complexities of the recursive search
   int iMoveNumber = -1; // This is used to more efficiently use LookForSearchExtensions.
   int iCheck      = 0; // This is used to more efficiently use LookForSearchExtensions;
   int iQScore     = 0;


   if ( ( argsBoard->iMaxPlys - argsBoard->iNumberOfPlys > GetNullReduction() ) &&
        ( argiBeta < -dMate ) &&
        ( argsBoard->iLastMoveNull == dNo ) &&
        ( argsBoard->iMoveOrder != 0 ) )

   { 

      // See if pieces other than just the king and pawns are on the board.
      // This is done to avoid zugswan. (SP)
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

         // Do not to a null search.
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

   // If we have already reached maet, no need to search (messes up the hash table)
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

   // return the logical LMR6
   return iLMR;

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

int GetReductionElement( int argiDepth,
                         int argiMove )
{

   return gsSearchParameters.miSearchReduction[ argiDepth ][ argiMove ];

}
