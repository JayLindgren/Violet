// This file containes useful routines, but that are not part off the heart of
// Violet. They are thrown in hodgepodge.
#include <cmath>
#include <fstream>
#include <iostream>

#include "Definitions.h"
#include "Functions.h"
#include "Structures.h"

using namespace std;

// This file is used to hold routings that are useful for tuning various
// parameters in violet.
void TuneMoveParameters( struct Board       *argsBoard,
                         struct GeneralMove *argsGeneralMoves )
{

   // Allocate some variables
   int iNumberOfNodesSearched;
   int iSortFlag;
   int iScoreIndex;

   // Set up the number of move scores
   int numberHeuristicMoves = dNumberHeuristicMoveScores;
   int numberSpecificMoves  = dNumberSpecificMoveScores;

   // Set up an arbitrary scoring system.
   // Use log scores to span a large space and let the two sets of score
   // interact.
   int iMaxScore = dMaxMoveScore;

   int vHeuristicScore[ dNumberHeuristicMoveScores ];
   int vSpecificScore[ dNumberSpecificMoveScores ];
   int vHeuristicOrder[ dNumberHeuristicMoveScores ];
   int vSpecificOrder[ dNumberSpecificMoveScores ];
   int vHeuristicRand[ dNumberHeuristicMoveScores ];
   int vSpecificRand[ dNumberSpecificMoveScores ];

   // Open a file for storing the outputs.
   ofstream gofOutputParameters;
   gofOutputParameters.open( "TuningParameters.txt" );
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

      vHeuristicScore[ i ] = int( exp( float( i + 1 ) * log( float( iMaxScore ) ) /
                                       float( dNumberHeuristicMoveScores ) ) );
      // cout << "i = " << vHeuristicScore[ i ] << endl;
   }

   // Loop over the heuristic scores and set arbitrary scores
   cout << endl;
   cout << "Specific Move Scores" << endl;
   for ( int i = 0; i < dNumberSpecificMoveScores; i++ )
   {

      vSpecificScore[ i ] = int( exp( float( i + 1 ) * log( float( iMaxScore ) ) /
                                      float( dNumberSpecificMoveScores ) ) );
      // cout << "i = " << vSpecificScore[ i ] << endl;
   }

   // Loop over the simulations.
   for ( int iSimulationCounter = 0; iSimulationCounter < dNumberOfSimulations;
         iSimulationCounter++ )
   {

      cout << endl
           << "iSimulationCounter = " << iSimulationCounter << endl
           << endl;
      for ( int iHeuristicCounter = 0;
            iHeuristicCounter < dNumberHeuristicMoveScores; iHeuristicCounter++ )
      {

         vHeuristicOrder[ iHeuristicCounter ] = iHeuristicCounter;
         vHeuristicRand[ iHeuristicCounter ]  = rand();
         // cout << "iHeuristicCounter = " << iHeuristicCounter << " " <<
         // vHeuristicRand[  iHeuristicCounter ] << endl;
      }

      cout << endl;
      for ( int iSpecificCounter = 0; iSpecificCounter < dNumberSpecificMoveScores;
            iSpecificCounter++ )
      {

         vSpecificOrder[ iSpecificCounter ] = iSpecificCounter;
         vSpecificRand[ iSpecificCounter ]  = rand();
         // cout << "iSpecificCounter = " << iSpecificCounter << " " <<
         // vSpecificRand[  iSpecificCounter ] << endl;
      }

      // Sort the Hueristics.
      // Sort the scores by the randome numbers.
      iSortFlag = 1;

      // Sort the move scores
      while ( iSortFlag )
      {

         // Set the default to bail.
         iSortFlag = 0;

         // Use a cocktail sort and to from top to bottom
         for ( iScoreIndex = 0; iScoreIndex < dNumberHeuristicMoveScores - 1;
               iScoreIndex++ )
         {

            if ( vHeuristicRand[ iScoreIndex + 1 ] < vHeuristicRand[ iScoreIndex ] )
            {

               int iDummyScore                   = vHeuristicRand[ iScoreIndex ];
               vHeuristicRand[ iScoreIndex ]     = vHeuristicRand[ iScoreIndex + 1 ];
               vHeuristicRand[ iScoreIndex + 1 ] = iDummyScore;

               int iDummyPosition                 = vHeuristicOrder[ iScoreIndex ];
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
      while ( iSortFlag )
      {

         // Set the default to bail.
         iSortFlag = 0;

         // Use a cocktail sort and to from top to bottom
         for ( iScoreIndex = 0; iScoreIndex < dNumberSpecificMoveScores - 1;
               iScoreIndex++ )
         {

            if ( vSpecificRand[ iScoreIndex + 1 ] < vSpecificRand[ iScoreIndex ] )
            {

               int iDummyScore                  = vSpecificRand[ iScoreIndex ];
               vSpecificRand[ iScoreIndex ]     = vSpecificRand[ iScoreIndex + 1 ];
               vSpecificRand[ iScoreIndex + 1 ] = iDummyScore;

               int iDummyPosition                = vSpecificOrder[ iScoreIndex ];
               vSpecificOrder[ iScoreIndex ]     = vSpecificOrder[ iScoreIndex + 1 ];
               vSpecificOrder[ iScoreIndex + 1 ] = iDummyPosition;

               iSortFlag = 1;
            }
         }
      }

      /*
            // Print out the sorts for QA/QC
            cout << endl << "Heuristic" << endl;
            for ( iScoreIndex = 0; iScoreIndex < dNumberHeuristicMoveScores;
         iScoreIndex++ )
            {

               cout << "index = " << iScoreIndex
                    << " order = " << vHeuristicOrder[ iScoreIndex ]
                    << " rand = " << vHeuristicRand[ iScoreIndex ]
                    << " score = " << vHeuristicScore[ vHeuristicOrder[
         iScoreIndex ] ]
                    << endl;

            }

            // Print out the sorts for QA/QC
            cout << endl << "Specifid" << endl;
            for ( iScoreIndex = 0; iScoreIndex < dNumberSpecificMoveScores;
         iScoreIndex++ )
            {

               cout << "index = " << iScoreIndex
                    << " order = " << vSpecificOrder[ iScoreIndex ]
                    << " rand = " << vSpecificRand[ iScoreIndex ]
                    << " score = " << vSpecificScore[ vSpecificOrder[ iScoreIndex
         ] ]
                    << endl;

            }
      */

      // Put the scores in the General Moves structure.
      argsGeneralMoves->msRegular     = vHeuristicScore[ vHeuristicOrder[ 0 ] ];
      argsGeneralMoves->msPawnTwo     = vHeuristicScore[ vHeuristicOrder[ 1 ] ];
      argsGeneralMoves->msPiece       = vHeuristicScore[ vHeuristicOrder[ 2 ] ];
      argsGeneralMoves->msQueen       = vHeuristicScore[ vHeuristicOrder[ 3 ] ];
      argsGeneralMoves->msBishop      = vHeuristicScore[ vHeuristicOrder[ 4 ] ];
      argsGeneralMoves->msRook        = vHeuristicScore[ vHeuristicOrder[ 5 ] ];
      argsGeneralMoves->msKnight      = vHeuristicScore[ vHeuristicOrder[ 6 ] ];
      argsGeneralMoves->msKing        = vHeuristicScore[ vHeuristicOrder[ 7 ] ];
      argsGeneralMoves->msCaptureDown = vHeuristicScore[ vHeuristicOrder[ 8 ] ];
      argsGeneralMoves->msCaptureSide = vHeuristicScore[ vHeuristicOrder[ 9 ] ];
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

      argsGeneralMoves->msPawnTakesQueen    = vSpecificScore[ vSpecificOrder[ 0 ] ];
      argsGeneralMoves->msPawnTakesRook     = vSpecificScore[ vSpecificOrder[ 2 ] ];
      argsGeneralMoves->msPawnTakesBishop   = vSpecificScore[ vSpecificOrder[ 3 ] ];
      argsGeneralMoves->msPawnTakesKnight   = vSpecificScore[ vSpecificOrder[ 4 ] ];
      argsGeneralMoves->msPawnTakesPawn     = vSpecificScore[ vSpecificOrder[ 5 ] ];
      argsGeneralMoves->msKnightTakesPawn   = vSpecificScore[ vSpecificOrder[ 6 ] ];
      argsGeneralMoves->msKnightTakesKnight = vSpecificScore[ vSpecificOrder[ 7 ] ];
      argsGeneralMoves->msKnightTakesBishop = vSpecificScore[ vSpecificOrder[ 8 ] ];
      argsGeneralMoves->msKnightTakesRook   = vSpecificScore[ vSpecificOrder[ 9 ] ];
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
      CreateBoard( argsBoard, argsGeneralMoves );
      InitializeHashTable();
      ClearHashTable();
      ClearPV( argsBoard );
      // ResetHistoryHeuristic(); // Handled in StartSearch
      // ResetKillerMoves();      // Handled in StartSearch
      SetInitialParameters();
      InitializeEvaluator();

      // Do a standard search and print out the number of nodes.
      ComputerMove( argsBoard, argsGeneralMoves );
      iNumberOfNodesSearched = GetNumberOfNodesSearched();
      cout << "iNumberOfNodesSearched = " << iNumberOfNodesSearched << endl;

      // Write out the parameters to a file.
      for ( iScoreIndex = 0; iScoreIndex < dNumberHeuristicMoveScores - 1;
            iScoreIndex++ )
      {

         if ( iScoreIndex == 14 )

            gofOutputParameters << dsKillerMove << " ";

         else if ( iScoreIndex == 15 )

            gofOutputParameters << dsHH << " ";

         else

            gofOutputParameters << vHeuristicScore[ vHeuristicOrder[ iScoreIndex ] ]
                                << " ";
      }

      // Write out the parameters to a file.
      for ( iScoreIndex = 0; iScoreIndex < dNumberSpecificMoveScores - 1;
            iScoreIndex++ )
      {

         gofOutputParameters << vSpecificScore[ vSpecificOrder[ iScoreIndex ] ] << " ";
      }

      gofOutputParameters << iNumberOfNodesSearched << endl;
   }

   gofOutputParameters.close();

   system( "pause" );
}

// SPSA Tuner for Move Ordering Mode B
void TuneMoveOrderingB( Board *argsBoard, GeneralMove *argsGeneralMoves )
{
   std::cout << "Starting SPSA Tuning for Move Ordering Mode B..." << std::endl;

   // Initial Parameters
   double pK1    = (double)gsTempus.iKillerScore1;
   double pK2    = (double)gsTempus.iKillerScore2;
   double pHMult = (double)gsTempus.iHistoryBonusMultiplier;
   double pHCap  = (double)gsTempus.iHistoryScoreCap;

   // SPSA Hyperparameters
   double a     = 20.0; // Learning rate scaling
   double c     = 5.0;  // Perturbation scaling
   double A     = 5.0;  // Stability constant
   double alpha = 0.602;
   double gamma = 0.101;

   int iterations = 20;

   long long bestNodes = -1;
   double    bestK1 = pK1, bestK2 = pK2, bestHMult = pHMult, bestHCap = pHCap;

   for ( int k = 0; k < iterations; k++ )
   {
      double ak = a / pow( k + 1 + A, alpha );
      double ck = c / pow( k + 1, gamma );

      // Bernoulli distribution for perturbation (+1 or -1)
      double deltaK1    = ( rand() % 2 == 0 ) ? 1.0 : -1.0;
      double deltaK2    = ( rand() % 2 == 0 ) ? 1.0 : -1.0;
      double deltaHMult = ( rand() % 2 == 0 ) ? 1.0 : -1.0;
      double deltaHCap  = ( rand() % 2 == 0 ) ? 1.0 : -1.0;

      // Perturb +
      gsTempus.iKillerScore1           = (int)( pK1 + ck * deltaK1 );
      gsTempus.iKillerScore2           = (int)( pK2 + ck * deltaK2 );
      gsTempus.iHistoryBonusMultiplier = (int)( pHMult + ck * deltaHMult );
      gsTempus.iHistoryScoreCap        = (int)( pHCap + ck * deltaHCap );

      // Ensure bounds
      if ( gsTempus.iHistoryBonusMultiplier < 1 )
         gsTempus.iHistoryBonusMultiplier = 1;

      long long yPlus =
          TestMoveOrdering( argsBoard, argsGeneralMoves, true, false );

      // Perturb -
      gsTempus.iKillerScore1           = (int)( pK1 - ck * deltaK1 );
      gsTempus.iKillerScore2           = (int)( pK2 - ck * deltaK2 );
      gsTempus.iHistoryBonusMultiplier = (int)( pHMult - ck * deltaHMult );
      gsTempus.iHistoryScoreCap        = (int)( pHCap - ck * deltaHCap );

      if ( gsTempus.iHistoryBonusMultiplier < 1 )
         gsTempus.iHistoryBonusMultiplier = 1;

      long long yMinus =
          TestMoveOrdering( argsBoard, argsGeneralMoves, true, false );

      // Estimate Gradient
      double gK1    = ( yPlus - yMinus ) / ( 2.0 * ck * deltaK1 );
      double gK2    = ( yPlus - yMinus ) / ( 2.0 * ck * deltaK2 );
      double gHMult = ( yPlus - yMinus ) / ( 2.0 * ck * deltaHMult );
      double gHCap  = ( yPlus - yMinus ) / ( 2.0 * ck * deltaHCap );

      // Update Parameters
      pK1    = pK1 - ak * gK1;
      pK2    = pK2 - ak * gK2;
      pHMult = pHMult - ak * gHMult;
      pHCap  = pHCap - ak * gHCap;

      // Keep track of best
      long long currentNodes = ( yPlus + yMinus ) / 2;
      if ( bestNodes == -1 || currentNodes < bestNodes )
      {
         bestNodes = currentNodes;
         bestK1    = pK1;
         bestK2    = pK2;
         bestHMult = pHMult;
         bestHCap  = pHCap;
      }

      std::cout << "Iter " << k << ": Nodes=" << currentNodes
                << " K1=" << (int)pK1 << " K2=" << (int)pK2
                << " HMult=" << (int)pHMult << " HCap=" << (int)pHCap
                << std::endl;
   }

   std::cout << "Tuning Complete." << std::endl;
   std::cout << "Best Parameters: " << std::endl;
   std::cout << "Killer1: " << (int)bestK1 << std::endl;
   std::cout << "Killer2: " << (int)bestK2 << std::endl;
   std::cout << "HistoryMult: " << (int)bestHMult << std::endl;
   std::cout << "HistoryCap: " << (int)bestHCap << std::endl;
   std::cout << "Best Nodes: " << bestNodes << std::endl;

   // Set best parameters
   gsTempus.iKillerScore1           = (int)bestK1;
   gsTempus.iKillerScore2           = (int)bestK2;
   gsTempus.iHistoryBonusMultiplier = (int)bestHMult;
   gsTempus.iHistoryScoreCap        = (int)bestHCap;
}