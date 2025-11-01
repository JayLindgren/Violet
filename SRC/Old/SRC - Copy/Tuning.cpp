// This file containes useful routines, but that are not part off the heart of Violet.
// They are thrown in hodgepodge.
#include <iostream>
#include <fstream>


#include "Definitions.h"
#include "Functions.h"
#include "Structures.h"

using namespace std;

// This file is used to hold routings that are useful for tuning various parameters in violet.
void TuneMoveParameters( struct Board * argsBoard,
                         struct GeneralMove * argsGeneralMoves )
{

   // Allocate some variables
   int iNumberOfNodesSearched; 
   int iSortFlag;
   int iScoreIndex;

   // Set up the number of move scores
   int numberHeuristicMoves = dNumberHeuristicMoveScores;
   int numberSpecificMoves  = dNumberSpecificMoveScores;

   // Set up an arbitrary scoring system.
   // Use log scores to span a large space and let the two sets of score interact.
   int iMaxScore = dMaxMoveScore;

   int vHeuristicScore[ dNumberHeuristicMoveScores ];
   int vSpecificScore[  dNumberSpecificMoveScores ];
   int vHeuristicOrder[ dNumberHeuristicMoveScores ];
   int vSpecificOrder[  dNumberSpecificMoveScores ];
   int vHeuristicRand[  dNumberHeuristicMoveScores ];
   int vSpecificRand[   dNumberSpecificMoveScores ];

   // Open a file for storing the outputs.
   ofstream gofOutputParameters;
   gofOutputParameters.open ( "TuningParameters.txt" );
   if ( gofOutputParameters.fail() )
   {
   
      cout << "gofOutputParameters.txt failed to open." << endl;
      system( "Pause" );
      return;
      
   }
   // Loop over the heuristic scores and set arbitrary scores
   cout << endl;
   cout << "Heuristic Move Scores" << endl;
   for ( int i = 0; i < dNumberHeuristicMoveScores; i++ )
   {

      vHeuristicScore[ i ] = int( exp( float( i + 1 ) * log( float( iMaxScore ) ) / float( dNumberHeuristicMoveScores ) ) );
      //cout << "i = " << vHeuristicScore[ i ] << endl;

   }

   // Loop over the heuristic scores and set arbitrary scores
   cout << endl;
   cout << "Specific Move Scores" << endl;
   for ( int i = 0; i < dNumberSpecificMoveScores; i++ )
   {

      vSpecificScore[ i ] = int( exp( float( i + 1 ) * log( float( iMaxScore ) ) / float( dNumberSpecificMoveScores ) ) );
      //cout << "i = " << vSpecificScore[ i ] << endl;

   }

   // Loop over the simulations.
   for ( int iSimulationCounter = 0; iSimulationCounter < dNumberOfSimulations; iSimulationCounter++ )
   {

      cout  << endl << "iSimulationCounter = " << iSimulationCounter << endl << endl;
      for ( int iHeuristicCounter = 0; iHeuristicCounter < dNumberHeuristicMoveScores; iHeuristicCounter++ )
      {

         vHeuristicOrder[ iHeuristicCounter ] = iHeuristicCounter;
         vHeuristicRand[  iHeuristicCounter ] = rand();
         //cout << "iHeuristicCounter = " << iHeuristicCounter << " " << vHeuristicRand[  iHeuristicCounter ] << endl;
   

      }

      cout << endl;
      for ( int iSpecificCounter = 0; iSpecificCounter < dNumberSpecificMoveScores; iSpecificCounter++ )
      {

         vSpecificOrder[ iSpecificCounter ] = iSpecificCounter;
         vSpecificRand[ iSpecificCounter ] = rand();
         //cout << "iSpecificCounter = " << iSpecificCounter << " " << vSpecificRand[  iSpecificCounter ] << endl;

      }

      // Sort the Hueristics.
      // Sort the scores by the randome numbers.
      iSortFlag = 1;

      // Sort the move scores
      while( iSortFlag )
      {

         // Set the default to bail.
         iSortFlag = 0;

         // Use a cocktail sort and to from top to bottom
         for ( iScoreIndex = 0; iScoreIndex < dNumberHeuristicMoveScores - 1; iScoreIndex++ )
         {

            if ( vHeuristicRand[ iScoreIndex + 1 ] < vHeuristicRand[ iScoreIndex ] )
            {

               int iDummyScore           = vHeuristicRand[ iScoreIndex ];
               vHeuristicRand[ iScoreIndex ]     = vHeuristicRand[ iScoreIndex + 1 ];
               vHeuristicRand[ iScoreIndex + 1 ] = iDummyScore;

               int iDummyPosition        = vHeuristicOrder[ iScoreIndex ];
               vHeuristicOrder[ iScoreIndex ]     = vHeuristicOrder[ iScoreIndex + 1 ];
               vHeuristicOrder[ iScoreIndex + 1 ] = iDummyPosition;

               iSortFlag = 1;

            }

         }
           
      }

      // Sort the Spedifics.
      // Sort the scores by the randome numbers.
      iSortFlag = 1;

      // Sort the move scores
      while( iSortFlag )
      {

         // Set the default to bail.
         iSortFlag = 0;

         // Use a cocktail sort and to from top to bottom
         for ( iScoreIndex = 0; iScoreIndex < dNumberSpecificMoveScores - 1; iScoreIndex++ )
         {

            if ( vSpecificRand[ iScoreIndex + 1 ] < vSpecificRand[ iScoreIndex ] )
            {

               int iDummyScore           = vSpecificRand[ iScoreIndex ];
               vSpecificRand[ iScoreIndex ]     = vSpecificRand[ iScoreIndex + 1 ];
               vSpecificRand[ iScoreIndex + 1 ] = iDummyScore;

               int iDummyPosition        = vSpecificOrder[ iScoreIndex ];
               vSpecificOrder[ iScoreIndex ]     = vSpecificOrder[ iScoreIndex + 1 ];
               vSpecificOrder[ iScoreIndex + 1 ] = iDummyPosition;

               iSortFlag = 1;

            }

         }
           
      }

/*
      // Print out the sorts for QA/QC
      cout << endl << "Heuristic" << endl;
      for ( iScoreIndex = 0; iScoreIndex < dNumberHeuristicMoveScores; iScoreIndex++ )
      {        

         cout << "index = " << iScoreIndex 
              << " order = " << vHeuristicOrder[ iScoreIndex ] 
              << " rand = " << vHeuristicRand[ iScoreIndex ] 
              << " score = " << vHeuristicScore[ vHeuristicOrder[ iScoreIndex ] ]
              << endl;

      }

      // Print out the sorts for QA/QC
      cout << endl << "Specifid" << endl;
      for ( iScoreIndex = 0; iScoreIndex < dNumberSpecificMoveScores; iScoreIndex++ )
      {        

         cout << "index = " << iScoreIndex 
              << " order = " << vSpecificOrder[ iScoreIndex ] 
              << " rand = " << vSpecificRand[ iScoreIndex ]
              << " score = " << vSpecificScore[ vSpecificOrder[ iScoreIndex ] ]
              << endl;

      }
*/

      // Put the scores in the General Moves structure.
      argsGeneralMoves->msRegular     = vHeuristicScore[ vHeuristicOrder[  0 ] ];
      argsGeneralMoves->msPawnTwo     = vHeuristicScore[ vHeuristicOrder[  1 ] ];
      argsGeneralMoves->msPiece       = vHeuristicScore[ vHeuristicOrder[  2 ] ];
      argsGeneralMoves->msQueen       = vHeuristicScore[ vHeuristicOrder[  3 ] ];
      argsGeneralMoves->msBishop      = vHeuristicScore[ vHeuristicOrder[  4 ] ];
      argsGeneralMoves->msRook        = vHeuristicScore[ vHeuristicOrder[  5 ] ];
      argsGeneralMoves->msKnight      = vHeuristicScore[ vHeuristicOrder[  6 ] ];
      argsGeneralMoves->msKing        = vHeuristicScore[ vHeuristicOrder[  7 ] ];  
      argsGeneralMoves->msCaptureDown = vHeuristicScore[ vHeuristicOrder[  8 ] ];
      argsGeneralMoves->msCaptureSide = vHeuristicScore[ vHeuristicOrder[  9 ] ];
      argsGeneralMoves->msCaptureUp   = vHeuristicScore[ vHeuristicOrder[ 10 ] ];
      argsGeneralMoves->msCheck       = vHeuristicScore[ vHeuristicOrder[ 11 ] ];
      argsGeneralMoves->msPromotion   = vHeuristicScore[ vHeuristicOrder[ 12 ] ];
      argsGeneralMoves->msCastle      = vHeuristicScore[ vHeuristicOrder[ 13 ] ];
      argsGeneralMoves->msKillerMove  = dsKillerMove;
      argsGeneralMoves->msHH          = dsHH;
//      vHeuristicScore[ vHeuristicOrder[ 14 ] ] = dsKillerMove;
//      vHeuristicScore[ vHeuristicOrder[ 15 ] ] = dsHH;    
      argsGeneralMoves->msKingCapture = vHeuristicScore[ vHeuristicOrder[ 16 ] ];
      argsGeneralMoves->msBestMove    = vHeuristicScore[ vHeuristicOrder[ 17 ] ];
      argsGeneralMoves->msPVMove      = vHeuristicScore[ vHeuristicOrder[ 18 ] ];

      argsGeneralMoves->msPawnTakesQueen    = vSpecificScore[ vSpecificOrder[  0 ] ];
      argsGeneralMoves->msPawnTakesRook     = vSpecificScore[ vSpecificOrder[  2 ] ];
      argsGeneralMoves->msPawnTakesBishop   = vSpecificScore[ vSpecificOrder[  3 ] ];
      argsGeneralMoves->msPawnTakesKnight   = vSpecificScore[ vSpecificOrder[  4 ] ];
      argsGeneralMoves->msPawnTakesPawn     = vSpecificScore[ vSpecificOrder[  5 ] ];
      argsGeneralMoves->msKnightTakesPawn   = vSpecificScore[ vSpecificOrder[  6 ] ];
      argsGeneralMoves->msKnightTakesKnight = vSpecificScore[ vSpecificOrder[  7 ] ];
      argsGeneralMoves->msKnightTakesBishop = vSpecificScore[ vSpecificOrder[  8 ] ];
      argsGeneralMoves->msKnightTakesRook   = vSpecificScore[ vSpecificOrder[  9 ] ];
      argsGeneralMoves->msKnightTakesQueen  = vSpecificScore[ vSpecificOrder[ 10 ] ];
      argsGeneralMoves->msBishopTakesPawn   = vSpecificScore[ vSpecificOrder[ 11 ] ];
      argsGeneralMoves->msBishopTakesKnight = vSpecificScore[ vSpecificOrder[ 12 ] ];
      argsGeneralMoves->msBishopTakesBishop = vSpecificScore[ vSpecificOrder[ 13 ] ];
      argsGeneralMoves->msBishopTakesRook   = vSpecificScore[ vSpecificOrder[ 14 ] ];
      argsGeneralMoves->msBishopTakesQueen  = vSpecificScore[ vSpecificOrder[ 15 ] ];
      argsGeneralMoves->msRookTakesPawn     = vSpecificScore[ vSpecificOrder[ 16 ] ];
      argsGeneralMoves->msRookTakesKnight   = vSpecificScore[ vSpecificOrder[ 17 ] ];
      argsGeneralMoves->msRookTakesBishop   = vSpecificScore[ vSpecificOrder[ 18 ] ];
      argsGeneralMoves->msRookTakesBishop   = vSpecificScore[ vSpecificOrder[ 19 ] ];
      argsGeneralMoves->msRookTakesRook     = vSpecificScore[ vSpecificOrder[ 20 ] ];
      argsGeneralMoves->msRookTakesQueen    = vSpecificScore[ vSpecificOrder[ 21 ] ];
      argsGeneralMoves->msQueenTakesPawn    = vSpecificScore[ vSpecificOrder[ 22 ] ];
      argsGeneralMoves->msQueenTakesKnight  = vSpecificScore[ vSpecificOrder[ 23 ] ];
      argsGeneralMoves->msQueenTakesBishop  = vSpecificScore[ vSpecificOrder[ 24 ] ];
      argsGeneralMoves->msQueenTakesRook    = vSpecificScore[ vSpecificOrder[ 25 ] ];
      argsGeneralMoves->msQueenTakesQueen   = vSpecificScore[ vSpecificOrder[ 26 ] ];

      // Reset the board.
      CreateBoard( argsBoard,
                   argsGeneralMoves );
      InitializeHashTable();
      ClearHashTable();
      ClearPV( argsBoard );
      ResetHistoryHeuristic();
      ResetKillerMoves();
      SetInitialParameters();
      InitializeEvaluator();

      // Do a standard search and print out the number of nodes.
      ComputerMove( argsBoard,
                    argsGeneralMoves );
      iNumberOfNodesSearched = GetNumberOfNodesSearched();
      cout << "iNumberOfNodesSearched = " << iNumberOfNodesSearched << endl;

      // Write out the parameters to a file.
      for ( iScoreIndex = 0; iScoreIndex < dNumberHeuristicMoveScores - 1; iScoreIndex++ )
      {

         if ( iScoreIndex == 14 )

            gofOutputParameters << dsKillerMove << " ";

         else if ( iScoreIndex == 15 )

            gofOutputParameters << dsHH << " ";

         else

            gofOutputParameters << vHeuristicScore[ vHeuristicOrder[ iScoreIndex ] ] << " ";

      }

      // Write out the parameters to a file.
      for ( iScoreIndex = 0; iScoreIndex < dNumberSpecificMoveScores - 1; iScoreIndex++ )
      {

         gofOutputParameters << vSpecificScore[ vSpecificOrder[ iScoreIndex ] ] << " ";

      }

      gofOutputParameters << iNumberOfNodesSearched << endl;

   }

   gofOutputParameters.close();

   system( "pause" );

}