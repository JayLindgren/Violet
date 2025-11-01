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

   * sBoard = * argsBoard;
   
   // Reset the board.
   * sBoard = * argsBoard;

   // Check if we were stopped by time - if so, don't validate checkmate
   int iWasStoppedByTime = ( GetStopGo() == dStop );

   // Loop over the forward plys.
   for ( iPlyIndex = 0; iPlyIndex < argsBoard->viPrincipalVariationLength[ 0 ]; iPlyIndex++ )
   {

      // Note this counter is needed incase a check mate is found.  Then we will jump out.
      iCount++;

      // At this time, because of the hash table, the structure sBoard may not have a full PV (because if it is being scored from a hash)
      // an early cut off will not allow it to store the PV
      // This is a hack to get out of the loop if the data is bad.
      if ( ( argsBoard->vmPrincipalVariation[ 0 ][iPlyIndex ].iFromSquare < 0 ) || 
           ( argsBoard->vmPrincipalVariation[ 0 ][iPlyIndex ].iToSquare < 0 ) || 
           ( argsBoard->vmPrincipalVariation[ 0 ][iPlyIndex ].iFromSquare > 64 ) || 
           ( argsBoard->vmPrincipalVariation[ 0 ][iPlyIndex ].iToSquare > 64 ) ) 
      {
         
         // If we have a bad square, jump out of the loop.
         cout << endl << endl;
         delete sBoard;
         return;

      }


      // Print out a move using PrintMoveFast to avoid unreliable checkmate detection
      // when search was interrupted by time
      iNumberOfCharacters =  PrintMoveFast( sBoard,
                                        argsGeneralMoves,
                                        &argsBoard->vmPrincipalVariation[ 0 ][iPlyIndex ],
                                        strMove );
      cout << " ";
                
      for ( int iCharCount = 0; iCharCount < iNumberOfCharacters; iCharCount++ )
      {
      
         cout << strMove[ iCharCount ];
         
      }

      // Make the move on the board.
      // Note that PrintMove() requires the board be at the appropriate state.
      MakeMove( &argsBoard->vmPrincipalVariation[ 0 ][iPlyIndex ],
                sBoard,
                argsGeneralMoves,
                0 );

      // If checkmate is found, bail out of the loop.
      // BUT: Don't trust checkmate notation if we were stopped by time
      if ( !iWasStoppedByTime &&
           !strncmp( & strMove[ iNumberOfCharacters - 1 ], 
                     "#",
                     1 ) )
      {

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
// ...existing code...

void InitializeSearch()
{

   // Initialize the search parameters.
   for( int iDepth = 0; iDepth < dNumberOfPlys; iDepth++ )
   {
      for (int i = 0; i < dNumberOfPlys; i++)
      {
         gvmKillerMoves[iDepth][i].iFromSquare = 0;
         gvmKillerMoves[iDepth][i].iToSquare = 0;
      }
   }

}

//
//--------------------------------------------------------------------------------------------------
//
void ClearPV(struct Board* argsBoard)
{
    // Debug the input.
    assert(argsBoard >= 0);

    // Clear out the PV.
    for (int i = 0; i < dNumberOfPlys; i++)
    {
        for (int j = 0; j < dNumberOfPlys; j++)
        {
            argsBoard->vmPrincipalVariation[i][j].iMoveType = 0;
        }
    }
}

//
//--------------------------------------------------------------------------------------------------
//
int StartSearch( struct Board *argsBoard,
                 struct GeneralMove *argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves > 0 );

   // Set the eliminate the checkers table first
   ClearEvasions( argsBoard );

   // Perhaps redundant, but clear the killer moves.
   InitializeSearch();

   // Set the board state.
   Board sBoard = *argsBoard;

   // Clear out the PV.
   ClearPV(&sBoard);

   // Set the number of nodes searched.
   int iNodesSearched = 0;

   // Search the move.
   wide Word64 llBestMove = SearchMoves( &sBoard,
                                    argsGeneralMoves,
                                    0,
                                    -1 * iInfinity,
                                    iInfinity );

   return (int)llBestMove;

}
