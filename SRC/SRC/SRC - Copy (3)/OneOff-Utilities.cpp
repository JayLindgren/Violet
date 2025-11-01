#include <iostream>
#include <fstream>
#include <string.h>
#include <assert.h>
#include <iomanip>

#include "Definitions.h"
#include "Structures.h"
#include "Functions.h"

using namespace std;

//
//------------------------------------------------------------------------------------------------
//
void PrintPrincipalVariation( struct Board *argsBoard,
							  struct GeneralMove *argsGeneralMoves )
{

	// Debug the input.
	assert( argsBoard >= 0 );
	assert( argsGeneralMoves >= 0 );

	// Loop over the moves in the principal variation.
	for( int iCounter = 0; iCounter < argsBoard->iMaxPlys -1; iCounter++ )
	{

		// Make sure the move is valid.
		if( argsBoard->vmPrincipalVariation[ 0 ][ iCounter ].iMoveType == 0 )
		{

			break;

		}

		// Print the move.
		PrintMove( &argsBoard->vmPrincipalVariation[ 0 ][ iCounter ],
			       argsGeneralMoves );

		// Print a space.
		cout << " ";

	}

	// Print a carriage return.
	cout << endl;

}

void TestComputer( struct Board * argsBoard,
                   struct GeneralMove * argsGeneralMoves )
{

   const int dGo = 2147483647;
   const int dAlpha = -2147483647;
   const int dBeta = 2147483647;

   // Perform the search.
   SetStopGo( dGo );
   int iScore = StartSearch( argsBoard,
                             argsGeneralMoves,
                             dAlpha,
                             dBeta );

   cout << "Score = " << iScore << endl;
   cout << "Number of nodes searched = " << (long long)GetNumberOfNodes() << endl;

   // Print the principal variation
   PrintPrincipalVariation( argsBoard,
                            argsGeneralMoves );

   //
   // Repeat the search with different moves disabled.
   //

   // Disable pawn moves.
   argsBoard->ulAvailableMoves |= 0x000000FF00000000;
   argsBoard->ulProbeHash -= 8;
   SetStopGo( dGo );
   iScore = StartSearch( argsBoard,
                         argsGeneralMoves,
                         dAlpha,
                         dBeta );

   cout << "Score (no pawns) = " << iScore << endl;
   cout << "Number of nodes searched (no pawns) = " << (long long)GetNumberOfNodes() << endl;

   // Print the principal variation
   PrintPrincipalVariation( argsBoard,
                            argsGeneralMoves );

   // Restore the original board state.
   InitializeBoard( argsBoard );

   // Disable knight moves.
   argsBoard->ulAvailableMoves |= 0x0000FF0000000000;
   argsBoard->ulProbeHash -= 8;
   SetStopGo( dGo );
   iScore = StartSearch( argsBoard,
                         argsGeneralMoves,
                         dAlpha,
                         dBeta );

   cout << "Score (no knights) = " << iScore << endl;
   cout << "Number of nodes searched (no knights) = " << (long long)GetNumberOfNodes() << endl;

   // Print the principal variation
   PrintPrincipalVariation( argsBoard,
                            argsGeneralMoves );

   // Restore the original board state.
   InitializeBoard( argsBoard );

   // Disable bishop moves.
   argsBoard->ulAvailableMoves |= 0x00FF000000000000;
   argsBoard->ulProbeHash -= 8;
   SetStopGo( dGo );
   iScore = StartSearch( argsBoard,
                         argsGeneralMoves,
                         dAlpha,
                         dBeta );

   cout << "Score (no bishops) = " << iScore << endl;
   cout << "Number of nodes searched (no bishops) = " << (long long)GetNumberOfNodes() << endl;

   // Print the principal variation
   PrintPrincipalVariation( argsBoard,
                            argsGeneralMoves );

   // Restore the original board state.
   InitializeBoard( argsBoard );

   // Disable rook moves.
   argsBoard->ulAvailableMoves |= 0xFF00000000000000;
   argsBoard->ulProbeHash -= 8;
   SetStopGo( dGo );
   iScore = StartSearch( argsBoard,
                         argsGeneralMoves,
                         dAlpha,
                         dBeta );

   cout << "Score (no rooks) = " << iScore << endl;
   cout << "Number of nodes searched (no rooks) = " << (long long)GetNumberOfNodes() << endl;

   // Print the principal variation
   PrintPrincipalVariation( argsBoard,
                            argsGeneralMoves );

   // Restore the original board state.
   InitializeBoard( argsBoard );

   // Disable queen moves.
   argsBoard->ulAvailableMoves |= 0x00000000FF000000;
   argsBoard->ulProbeHash -= 8;
   SetStopGo( dGo );
   iScore = StartSearch( argsBoard,
                         argsGeneralMoves,
                         dAlpha,
                         dBeta );

   cout << "Score (no queens) = " << iScore << endl;
   cout << "Number of nodes searched (no queens) = " << (long long)GetNumberOfNodes() << endl;

   // Print the principal variation
   PrintPrincipalVariation( argsBoard,
                            argsGeneralMoves );

   // Restore the original board state.
   InitializeBoard( argsBoard );

   // Disable king moves.
   argsBoard->ulAvailableMoves |= 0x0000000000FF0000;
   argsBoard->ulProbeHash -= 8;
   SetStopGo( dGo );
   iScore = StartSearch( argsBoard,
                         argsGeneralMoves,
                         dAlpha,
                         dBeta );

   cout << "Score (no kings) = " << iScore << endl;
   cout << "Number of nodes searched (no kings) = " << (long long)GetNumberOfNodes() << endl;

   // Print the principal variation
   PrintPrincipalVariation( argsBoard,
                            argsGeneralMoves );

   // Restore the original board state.
   InitializeBoard( argsBoard );

   // //
   // // Search for the best move for White.
   // //

   // int iBestMove = 0;
   // int iScore = 0;
   // int iAlpha = -2147483647;
   // int iBeta = 2147483647;
   // for( int i = 0; i < iNumberOfMoves; i++ )
   // {

   //    // Make the move.
   //    MakeMove( argsBoard,
   //               argsGeneralMoves );

   //    // Search the resulting position.
   //    iScore = -StartSearch( argsBoard,
   //                           argsGeneralMoves,
   //                           -iBeta,
   //                           -iAlpha );

   //    // Unmake the move.
   //    UndoMove( argsBoard,
   //              argsGeneralMoves );

   //    // If we have a new best move, update the principal variation.
   //    if( iScore > iAlpha )
   //    {

   //       // Update the principal variation.
   //       argsBoard->vmPrincipalVariation[ argsBoard->iPlys ][ argsBoard->iPlys ] = mMoveList[ i ];
   //       for( int j = argsBoard->iPlys + 1; j < argsBoard->iMaxPlys; j++ )
   //       {

   //          argsBoard->vmPrincipalVariation[ argsBoard->iPlys ][ j ] = argsBoard->vmPrincipalVariation[ argsBoard->iPlys + 1 ][ j ];

   //       }

   //       // Update the best move.
   //       iBestMove = i;

   //       // Update alpha.
   //       iAlpha = iScore;

   //       // If we have a beta cutoff, break out of the loop.
   //       if( iAlpha >= iBeta )
   //       {

   //          // Store the killer move.
   //          gvmKillerMoves[ argsBoard->iPlys ][ 1 ] = gvmKillerMoves[ argsBoard->iPlys ][ 0 ];
   //          gvmKillerMoves[ argsBoard->iPlys ][ 0 ] = mMoveList[ i ];

   //          // Store the hash entry.
   //          if( GetUseHashTable() )
   //          {

   //             RecordHashTable( argsBoard,
   //                              iBeta,
   //                              dHashBeta,
   //                              iBestMove );

   //          }

   //          // Return beta.
   //          return iBeta;

   //       }

   //    }

   // }

   // return iAlpha;

}
