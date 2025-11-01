#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string.h>

// If in deep mode, include the appropriate files
#if defined( dDeepMode )
#include <omp.h>
#endif

#include "Definitions.h"
#include "Functions.h"
#include "Structures.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Define global variables for keeping track of the number of nodes counted.
// Global variables suck, but are awsome for allowing for Deep Violet.
// access to the table:
// Note that the scope for the globe variables is only this file.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Define the game control structure called Tempus
Tempus gsTempus;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SetInitialParameters()
{
   
   // Set up standard playing styles.
   // 1 UCI engine
   // 2 FEN Set up
   // 3 Normal game play
   int iStyle = 3;
   int iComputerColor = dComputerBlack; // dComputerWhite dComputerBlack dRandom
   int iDepth = 12;
   int iTime = dInfiniteTime; //dInfiniteTime dOneSecond dTwentySeconds dOneMinute dTenMinutes

   if ( iStyle == 1 )
   {
      //---------------------------------------------------------------------------------------------
      // UCI engine
      // Set the initial parameters for controling the gamee4
      SetSearchDepth(             dInfiniteDepth ); // dInfiniteDepth dOpeningBookVerificationSearchDepth
      SetSearchTimeInMiliSeconds( dInfiniteTime  ); // dInfiniteTime dOneSecond dTwentySeconds dOneMinute dTenMinutes
      SetModeOfPlay(              dGame          ); // dGame dSetUp dComputerGame dBookProbe dTestSuite dTestEval dTune dTestDraw dCheckBook dTestHash dUserSetUp dTestNull
      SetUseOpeningBook(          dNo            ); // dYes dNo
      SetComputerColor(           iComputerColor ); // dComputerWhite dComputerBlack
      SetInterfaceMode(           dUCI           ); // dConsole dUCI dXboard
   }
   else if( iStyle == 2 )
   {

      //---------------------------------------------------------------------------------------------
      // FEN Set up
      // Set the initial parameters for controling the gamee4
      SetSearchDepth(             iDepth         ); // dInfiniteDepth dOpeningBookVerificationSearchDepth
      SetSearchTimeInMiliSeconds( dInfiniteTime  ); // dInfiniteTime dOneSecond dTwentySeconds dOneMinute dTenMinutes
      SetModeOfPlay(              dSetUp         ); // dGame dSetUp dComputerGame dBookProbe dTestSuite dTestEval dTune stEval dTune
      SetUseOpeningBook(          dNo            ); // dYes dNo
      SetComputerColor(           iComputerColor ); // dComputerWhite dComputerBlack
      SetInterfaceMode(           dConsole       ); // dConsole dUCI dXboard
   }
   else if( iStyle == 3 )
   {

      //---------------------------------------------------------------------------------------------
      // Normal game play
      // Set the initial parameters for controling the gamee4
      SetSearchDepth(             iDepth         ); // dInfiniteDepth dOpeningBookVerificationSearchDepth
      SetSearchTimeInMiliSeconds( iTime  ); // dInfiniteTime dOneSecond dTwentySeconds dOneMinute dTenMinutes
      SetModeOfPlay(              dGame          ); // dGame dSetUp dComputerGame dBookProbe dTestSuite dTestEval dTestDraw dCheckBook dTestHash dUserSetUp dTestNull
      SetUseOpeningBook(          dNo            ); // dYes dNo
      SetComputerColor(           iComputerColor ); // dComputerWhite dComputerBlack
      SetInterfaceMode(           dConsole       ); // dConsole dUCI dXboard
   }
    


   // Main features
   SetUseHashTable(    dYes ); // dYes dNo   // can be used dynamically, but commented out in definitions.h
   SetUseNullMove(     dYes ); // dYes dNo   // can be used dynamically, but commented out in definitions.h
   SetUseLMR(          dYes ); // dYes dNo
   SetAspirationSearch(dYes );

   // Debug Issues$
   SetSearchMonitorTogel(  dNo  );  // dYes dNo
   SetDebug(               dNo  );  // dYes dNo
   SetOpeningBookDebug(    dNo  );  // dYes dNo
   SetInterfaceDebug(      dYes ); // dYes dNo
   SetInterfaceMovesDebug( dNo  ); // dYes dNo
   SetInterfaceBookDebug(  dNo  ); // dYes dNo

   // Opening book analysis
   SetOpeningBookAnalysis( dNo  ); // dYes dNo
   SetLimitELO(            dNo  ); // dYes dNo
   SetConsoleOutput(       dYes ); // dYes dNo
   SetIsInBook(            dYes ); // dYes dNo // Just make sure this is initialized.

   // Hash table issues 
   SetPersistantKeys(dCode); // dNewKeys dFile Code
   SetResetHash(dYes); // Controls if the hash table is reset in ReadFEN.

   // Set the use of the null move heurisic.
   SetUseNullEval(dNo);
   SetNullReduction(3); // Sets how much shorter the null search is.

   // Set the Late Move Reduction parameter
   SetLMRMinimumMoveSearch(1);
   SetLMRMinimumMoveScore(99);
   SetLMRReducedSearchDepth(1);

   // Set the evaluator issues.
   SetUseMobility(dNo); // dYes dNo

   // Set the number of PVs to print out.
   SetNumberOfPVs(8);

   // Make sure Violet is pondering to start
   // If this is set to no, MutilPV will fail if used.
   SetPonder(dNo); // dYes dNo

}

//
//---------------------------------------------------------------------------------------------
//
void GameControl(struct Board* argsBoard,
   struct GeneralMove* argsGeneralMoves)
{

   // Debug the input.
   assert(argsBoard >= 0);
   assert(argsGeneralMoves >= 0);

   // Switch on the modes of play.
   int iMode = GetModeOfPlay();
   assert(iMode >= 0);
   switch (iMode)
   {

   case dSetUp:
   {

      SetUp(argsBoard,
         argsGeneralMoves);
      break;

   }

   case dGame:
   {

      Game(argsBoard,
         argsGeneralMoves);
      break;

   }

   case dComputerGame:
   {


      ComputerGame(argsBoard,
         argsGeneralMoves);
      break;

   }

   case dBookProbe:
   {

      BookProbe(argsBoard,
         argsGeneralMoves);

   }

   case dTestSuite:
   {

      TestSuite(argsBoard,
         argsGeneralMoves);
      break;

   }

   case dTestEval:
   {

      TestEval(argsBoard,
         argsGeneralMoves);
      break;

   }

   case dTestDraw:
   {

      TestDraw(argsBoard,
         argsGeneralMoves);
      break;

   }
   case dCheckBook:
   {

      //cout << "Nodes in book = " << GetNumberOfPositionsInOpeningBook() << endl;
      StartCheckBook(argsBoard,
         argsGeneralMoves);
      break;

   }
   case dTestHash:
   {

      TestHash(argsBoard,
         argsGeneralMoves);
      break;

   }
   case dUserSetUp:
   {

      UserSetUp(argsBoard,
         argsGeneralMoves);
      break;

   }
   case dTestNull:
   {

      TestNullMove(argsBoard,
         argsGeneralMoves);
      break;

   }
   case dOpeningBook:
   {

      // Be sure not to use the hash table when using this routine.
      SetUseOpeningBook(dNo);
      // The name of the files are set in the routine.
      OpeningBookAnalysis(argsBoard,
         argsGeneralMoves);
      break;

   }
   case dTune:
   {

      TuneMoveParameters(argsBoard,
         argsGeneralMoves);

   }

   }

   system("PAUSE");

}


//
//----------------------------------------------------------------------------------------------
//
void SetUp(struct Board* argsBoard,
   struct GeneralMove* argsGeneralMoves)
{

   // Debug the input.
   assert(argsBoard >= 0);
   assert(argsGeneralMoves >= 0);

   // Set the board to being out of book.
   SetIsInBook(dNo);

   // Read the FEN file.
   ReadFEN("FEN.txt",
      argsBoard,
      argsGeneralMoves,
      0); // 1 reads from the standard set up, 0 from the file.

   // Show the board.
   if (GetInterfaceMode() == dConsole)
   {

      PrintBoard(argsBoard->mBoard);
      PrintFEN(argsBoard,
         argsGeneralMoves);

   }

   // Switch on the color to move the color the computer is playing.
   if (((argsBoard->siComputerColor == dComputerBlack) &&
      (argsBoard->siColorToMove == dWhite)) ||
      ((argsBoard->siComputerColor == dComputerWhite) &&
         (argsBoard->siColorToMove == dBlack)))
   {

      InputUserMove(argsBoard,
         argsGeneralMoves);

      ComputerMove(argsBoard,
         argsGeneralMoves);

   }
   else
   {

      ComputerMove(argsBoard,
         argsGeneralMoves);

      InputUserMove(argsBoard,
         argsGeneralMoves);

   }

   // Switch on the color to move the color the computer is playing.
   if (((argsBoard->siComputerColor == dComputerBlack) &&
      (argsBoard->siColorToMove == dWhite)) ||
      ((argsBoard->siComputerColor == dComputerWhite) &&
         (argsBoard->siColorToMove == dBlack)))
   {

      InputUserMove(argsBoard,
         argsGeneralMoves);

      ComputerMove(argsBoard,
         argsGeneralMoves);

   }
   else
   {

      ComputerMove(argsBoard,
         argsGeneralMoves);

      InputUserMove(argsBoard,
         argsGeneralMoves);

   }

   while (0)
   {

      // Have the computer make a move.
      ComputerMove(argsBoard,
         argsGeneralMoves);

      if (GetInterfaceMode() == dConsole)
      {

         cout << " In SetUp control code." << endl;
         system("pause");

      }


   }

}

//
//-------------------------------------------------------------------------------------
//
void Game(struct Board* argsBoard,
   struct GeneralMove* argsGeneralMoves)
{

   // Debug the input.
   assert(argsBoard >= 0);
   assert(argsGeneralMoves >= 0);

   if (argsBoard->siComputerColor == dComputerWhite)
   {

      ComputerMove(argsBoard,
         argsGeneralMoves);


      // Loop over the game.
      argsBoard->iPlayGame = 1;
      while (argsBoard->iPlayGame)
      {


         // Input a user move, user plays white.
         InputUserMove(argsBoard,
            argsGeneralMoves);

         ComputerMove(argsBoard,
            argsGeneralMoves);

      }

   }
   else
   {
      // Show the initial board.
      if (GetInterfaceMode() == dConsole)
      {

         PrintBoard(argsBoard->mBoard);

      }

      // Loop over the game.
      argsBoard->iPlayGame = 1;
      while (argsBoard->iPlayGame)
      {

         // Input a user move, user plays white.
         InputUserMove(argsBoard,
            argsGeneralMoves);

         // Have the computer ponder while waiting.
         ComputerMove(argsBoard,
            argsGeneralMoves);

         // Go think about it.
         // This can be turned on and off
         if (GetPonder())
         {

            Ponder(argsBoard,
               argsGeneralMoves);

            // Print the multi PV's
            PrintMultiPV(argsBoard,
               argsGeneralMoves,
               8);

         }

      }

   }

}


//
//-------------------------------------------------------------------------------------
//
void ComputerGame(struct Board* argsBoard,
   struct GeneralMove* argsGeneralMoves)
{

   // Debug the input.
   assert(argsBoard >= 0);
   assert(argsGeneralMoves >= 0);

   // Loop until paused.
   while (1)
   {

      // Have the computer make a move.
      ComputerMove(argsBoard,
         argsGeneralMoves);

      if (GetInterfaceMode() == dConsole)
      {

         cout << "In the computer game control code." << endl;
         system("pause");

      }

   }

}

//
//---------------------------------------------------------------------------------------
//
void BookProbe(struct Board* argsBoard,
   struct GeneralMove* argsGeneralMoves)
{

   // Debug the input.
   assert(argsBoard >= 0);
   assert(argsGeneralMoves >= 0);

   ///*
   if (GetUseOpeningBook() == 1)
   {

      PrintOpeningBookMoveStatistics(argsBoard,
         argsGeneralMoves);

   }
   //*/

   while (1)
   {

      // print out the starting hash.
      //cout << endl << "Starting Hash = " << GetHash() << endl << endl;

      // Input a user move, user plays white.
      if (GetInterfaceMode() == dConsole)
      {

         PrintBoard(argsBoard->mBoard);
         PrintFEN(argsBoard,
            argsGeneralMoves);
         int iEvalScore = EvaluateBoard(argsBoard,
            argsGeneralMoves);
         cout << "iEval = " << iEvalScore << endl << endl;

      }

      InputUserMove(argsBoard,
         argsGeneralMoves);

      if (GetInterfaceMode() == dConsole)
      {

         PrintOpeningBookMoveStatistics(argsBoard,
            argsGeneralMoves);

      }

      // Input a user move, user plays white.
      if (GetInterfaceMode() == dConsole)
      {

         PrintBoard(argsBoard->mBoard);

      }

      InputUserMove(argsBoard,
         argsGeneralMoves);

      //cout << "gsHashTable.bbHash = " << GetHash() << endl;

      if (GetInterfaceMode() == dConsole)
      {

         PrintOpeningBookMoveStatistics(argsBoard,
            argsGeneralMoves);

      }

   }

}

//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void SetTempusParameters()
{

   // Set the parameters for testing.
   gsTempus.giTimeStart = clock();

   // Set the Tempus to search.
   SetStopGo(dGo);
   SetStart(clock());
   SetSearchStartTime();

   // Initialize the global node count.
   SetNumberOfNodes(0);
   SetNumberOfIncrementalNodes(0);
   SetNodeCheck(dNodeCheck);

}

//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void InterfaceControl(struct Board* argsBoard)
{

   // Debug the input.
   assert(argsBoard >= 0);

   // Periodiaclly wake up and check on the interface.

   // Look for a timed move.
   if (clock() - gsTempus.giTimeStart >= gsTempus.giSearchTime)
   {

      gsTempus.giStopGo = dStop;

   }

}

//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CalculateTimeForMove(struct Board* argsBoard)
{

   // Debug the input.
   assert(argsBoard >= 0);

   // This function calculates how much time should be used for a move given the following convention
   // Standard time for 40 moves with 
   int iTimeLeft = 0;
   int iMovesToGo = 0;
   int iIncTime = 0;
   int iTime = 0;

   // Get the time left
   if (argsBoard->siColorToMove == dWhite)
   {

      iTimeLeft = GetWhiteTime();

   }
   else
   {

      iTimeLeft = GetBlackTime();

   }

   // Get the incremental time left
   if (argsBoard->siColorToMove == dWhite)
   {

      iIncTime = GetWhiteIncrementalTime();

   }
   else
   {

      iIncTime = GetBlackIncrementalTime();

   }

   // Debugging.
   assert(iTimeLeft >= 0);

   // See if there are moves to go.
   iMovesToGo = GetMovesToGo();

   // Debugging.
   assert(iMovesToGo >= 0);

   // If MoveToGo is zero then assume there are 40 moves left.
   if (iMovesToGo == 0)
   {

      iMovesToGo = 40;

   }

   // Calculate the time left.
   iTime = (int)((double)iTimeLeft / (double)iMovesToGo);

   // Add the incremental time, but make sure there is enough time left.
   if (iTimeLeft > iIncTime)
   {

      iTime += iIncTime;

   }

   // A safety check
   if (iTime > iTimeLeft)
   {

      iTime = (int)((double)iTimeLeft / (double)iMovesToGo);

   }

   // Debugging
   assert(iTime >= 0);

   // Check for a zero time.
   if (iTime <= 0)
   {
      iTime = 1000;
   }

   // Adjust for communication time.
   if (iTime > (dBufferTime + 1000))
   {

      iTime -= dBufferTime;

   }

   // Set the search time.
   SetSearchTimeInMiliSeconds(iTime);

}

// Initialize that which needs to be
void Initialize(struct Board* argsBoard,
   struct GeneralMove* argsGeneralMoves)
{

   // Debug the input.
   assert(argsBoard >= 0);
   assert(argsGeneralMoves >= 0);

   // Set some global parameters for this game.
   SetInitialParameters();

   // Calculate all of the general moves
   GenerateGeneralMove(argsGeneralMoves);

   //  Create the initial board.
   CreateBoard(argsBoard,
      argsGeneralMoves);

   // Initialize the evaluator with score information.
   InitializeEvaluator();

   // Initialize the search parameters.
   InitializeSearch();

   // Initialize the hash table. and opening book.
   InitializeHashTable();
   InitializeOpeningBook();
   if (GetInterfaceMode() == dUCI)
   {

      InitializeCommunications();

   }

   // Load the book if we are using it.
   if (GetUseOpeningBook())
   {

      ReadOpeningBook("Book.txt",
         argsGeneralMoves);

   }

   // Do we want an opening book anaysis?
   if (GetOpeningBookAnalysis())
   {
      OpeningBookAnalysis(argsBoard,
         argsGeneralMoves);
      system("pause");

   }

   // Initialize the time controls.
   SetBlackIncrementalTime(0);
   SetBlackTime(0);
   SetMoveTime(0);
   SetWhiteIncrementalTime(0);
   SetWhiteTime(0);
   SetMovesToGo(0);
   SetNodes(0);
   SetMate(0);

   // Initialize the multiple PV's
   for (int iMoveIndex = 0; iMoveIndex <= dNumberOfMoves; iMoveIndex++)
   {

      gsTempus.viScore[iMoveIndex] = dAlpha;
      gsTempus.viScoreOld[iMoveIndex] = dAlpha;
      gsTempus.viHaveScore[iMoveIndex] = 0;

   }

   // Initialize the time for move updating.
   SetTimeLastMoveUpdate();

   // Open degug files as needed.
   if (GetInterfaceMovesDebug())
   {

      InitializeMoveDebug();

   }

   if (GetInterfaceBookDebug())
   {

      InitializeBookDebug();

   }


}

// The control loop.
void ControlLoop(struct Board* argsBoard,
   struct GeneralMove* argsGeneralMoves)
{

   // Debug the input.
   assert(argsBoard >= 0);
   assert(argsGeneralMoves >= 0);

   // This is the main loop for looking for input from the interface.
   if (GetInterfaceMode() == dUCI)
   {

      while (GetInterfaceMode() == dUCI)
      {

         // Do a low level read from the input.
         ReadInputAndExecute(argsBoard,
            argsGeneralMoves);

      }

   }
   else if (GetInterfaceMode() == dConsole) // Do the console thing.
   {

      // Hand over to Game Control.
      GameControl(argsBoard,
         argsGeneralMoves);

   }
   else
   {

      cout << "Here's the rub.  mode not found." << endl;
      system("pause");

   }

}

//
//---------------------------------------------------------------------------------------
//
void TestDraw(struct Board* argsBoard,
   struct GeneralMove* argsGeneralMoves)
{

   // This will run Violet through a series of test positions and compare her score 
   // against the conventional wisdom.

   // Debug the input.
   assert(argsBoard >= 0);
   assert(argsGeneralMoves >= 0);

   int iFlagBook = 1;
   int iFlagLine = 1;
   char strMove[64];
   char strMoveNumber[64];
   int iMoveCount = 0;
   int iMoveCountOld = -1;
   int iBlackMove = 0;
   int iPlyCount = 0;
   int iNumberOfCharacters = 0;
   //int iNumberOfMovesDummy;
   //int iGameResult;
   int iGameCount = 0;
   int iIncrementalGameCount = 0;
   int iIsGoodGame = 1;
   int iBadGameCount = 0;
   char strFileName[21];
   //char strBookName[ 21 ];
   int iGoodWhiteELO = 0;
   int iGoodBlackELO = 0;
   BitBoard bbHashInitial = GetHash();
   int iMaxPlyIndex = 81;

   // Read the FEN file.  Set up the initial position.
   ReadFEN(strFileName,
      argsBoard,
      argsGeneralMoves,
      1);

   /*
   cout << "iHistoryIndex = " << argsBoard->iMoveHistory << " Hash = " << argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbHash << endl;
   PrintBoard( argsBoard->mBoard );
   */

   // Clear the string.
   strcpy(strFileName, "Draw Game.txt");

   // Open the file.
   ifstream ifBook(strFileName);
   if (ifBook.fail())
   {

      cout << "Input file failed to open." << endl;
      iFlagBook = 0;

   }

   // Loop over the data file.
   while (iFlagBook)
   {


      int iMoveCount = 1;
      int iPlyCount = 1;
      int iFlagLine = 1;
      int iIsGoodGame = 1;

      // Reset the game result.
      int iGameResult = 0;

      // Look for an ELO setting
      int iGoodBlackELO = 0;
      int iGoodWhiteELO = 0;

      // Loop over a game.
      while (iFlagLine)
      {

         // Input a string.
         ifBook >> strMove;
         if (GetOpeningBookDebug())
         {
            cout << strMove << endl;
         }

         // Put the move number into a string.
         iNumberOfCharacters = sprintf(strMoveNumber, "%d.", iMoveCount);

         // Look to see if we are at the end of a game.
         if (LookForGameResult(strMove,
            &iGameResult) == 1)
         {

            iFlagLine = 0;
            break;

         }
         if (LookForGameResult(strMove,
            &iGameResult) == -1)
         {

            iFlagLine = 0;
            iFlagBook = 0;
            break;

         }

         // Look for a move.
         if (strncmp(strMove, strMoveNumber, strlen(strMoveNumber)) == 0)
         {

            // We have a new move.
            if (iIsGoodGame)
            {

               iMoveCount++;
               iPlyCount++;

            }

            // Get the white move. Make sure the move number and move have not white space.
            if (strlen(strMove) > strlen(strMoveNumber))
            {

               char strMoveDummy[64] = " ";
               int iMoveNumberLength = static_cast<int>(strlen(strMoveNumber));
               int iMoveLength = static_cast<int>(strlen(strMove));
               int iDummy = static_cast<int>(strlen(strMoveDummy));
               strncpy(strMoveDummy, strMove + strlen(strMoveNumber), strlen(strMove) - strlen(strMoveNumber));
               strcpy(strMove, strMoveDummy);

            }
            else
            {

               ifBook >> strMove;
               if (GetOpeningBookDebug())
               {
                  cout << strMove << endl;
               }

            }

            // Read a move from the file.
            int iPlyIndex = 999;
            ReadAPGNMove(&iGameResult,
               &iFlagLine,
               &iFlagBook,
               &iIsGoodGame,
               &iGoodWhiteELO,
               &iGoodBlackELO,
               strMove,
               argsBoard,
               argsGeneralMoves,
               iPlyCount,
               iPlyIndex);

            if (iFlagLine == 0)
            {

               break;

            }

            // Get the Black move.
            ifBook >> strMove;
            if (GetOpeningBookDebug())
            {
               cout << strMove << endl;
            }

            // We have a new move.
            if (iIsGoodGame)
            {

               iPlyCount++;

            }

            // Read a move from the file.
            ReadAPGNMove(&iGameResult,
               &iFlagLine,
               &iFlagBook,
               &iIsGoodGame,
               &iGoodWhiteELO,
               &iGoodBlackELO,
               strMove,
               argsBoard,
               argsGeneralMoves,
               iPlyCount,
               iPlyIndex);

            if (iFlagLine == 0)
            {

               break;

            }

         }

      }

   }


   PrintBoard(argsBoard->mBoard);

   int iScore = EvaluateBoard(argsBoard,
      argsGeneralMoves);

   cout << "Evaluation Score = " << iScore << endl << endl;

   // Check the history of the hash to make sure it has been repeated three times.
   int iHistoryIndex = 0;

   // Look for a draw - function returns a if a three fold repetition is found.
   if (LookForDraw(argsBoard,
      argsGeneralMoves))
   {

      // Return a draw score.
      iScore = GetValueOfDraw();

   }


   UndoMove(argsBoard,
      argsGeneralMoves);

   PrintBoard(argsBoard->mBoard);

   iScore = EvaluateBoard(argsBoard,
      argsGeneralMoves);

   cout << "Evaluation Score = " << iScore << endl;

   SetStopGo(dGo);
   iScore = -Search(argsBoard,
      argsGeneralMoves,
      dAlpha,
      dBeta);

   cout << "Search score = " << iScore << endl;

   SetStopGo(dGo);
   iScore = StartSearch(argsBoard,
      argsGeneralMoves,
      dAlpha,
      dBeta);


   MakeMove(*argsBoard->vmPrincipalVariation,
      argsBoard,
      argsGeneralMoves,
      0);

   PrintBoard(argsBoard->mBoard);

   ifBook.close();

}

// Destroy that which needs to be
void Destroy()
{

   // Destroy as needed.
   DestroyOpeningBook();
   DestroyHashTable();
   CloseCommunications();

}

//
///////////////////////////////////////////////////////////////////////////////////////////
//
void InitializeMultiPV()
{

   // Clear out all the data for the multi PV
   for (int iMoveCount = 0; iMoveCount < dNumberOfMoves; iMoveCount++)
   {

      gsTempus.viScore[iMoveCount] = 0;
      gsTempus.viScoreOld[iMoveCount] = 0;
      gsTempus.viHaveScore[iMoveCount] = 0;
      gsTempus.viDepth[iMoveCount] = -1;

   }

   // Clear out all the data for the multi PV
   for (int iMoveCount = 0; iMoveCount < dNumberOfPlys; iMoveCount++)
   {

      for (int iPlyCount = 0; iPlyCount < dNumberOfPlys; iPlyCount++)
      {

         gsTempus.vmPrincipalVariation[iPlyCount][iMoveCount].bbFromSquare = 0;
         gsTempus.vmPrincipalVariation[iPlyCount][iMoveCount].bbToSquare = 0;
         gsTempus.vmPrincipalVariation[iPlyCount][iMoveCount].bbEPSquare = 0;
         gsTempus.vmPrincipalVariation[iPlyCount][iMoveCount].iPiece = 0;
         gsTempus.vmPrincipalVariation[iPlyCount][iMoveCount].iFromSquare = 0;
         gsTempus.vmPrincipalVariation[iPlyCount][iMoveCount].iToSquare = 0;
         gsTempus.vmPrincipalVariation[iPlyCount][iMoveCount].iPromote = 0;
         gsTempus.vmPrincipalVariation[iPlyCount][iMoveCount].iMoveType = 0;
         gsTempus.vmPrincipalVariation[iPlyCount][iMoveCount].bbCastle = 0;
         gsTempus.vmPrincipalVariation[iPlyCount][iMoveCount].iCapture = 0;
         gsTempus.vmPrincipalVariation[iPlyCount][iMoveCount].iScore = 0;

         gsTempus.vmPrincipalVariationOld[iPlyCount][iMoveCount].bbFromSquare = 0;
         gsTempus.vmPrincipalVariationOld[iPlyCount][iMoveCount].bbToSquare = 0;
         gsTempus.vmPrincipalVariationOld[iPlyCount][iMoveCount].bbEPSquare = 0;
         gsTempus.vmPrincipalVariationOld[iPlyCount][iMoveCount].iPiece = 0;
         gsTempus.vmPrincipalVariationOld[iPlyCount][iMoveCount].iFromSquare = 0;
         gsTempus.vmPrincipalVariationOld[iPlyCount][iMoveCount].iToSquare = 0;
         gsTempus.vmPrincipalVariationOld[iPlyCount][iMoveCount].iPromote = 0;
         gsTempus.vmPrincipalVariationOld[iPlyCount][iMoveCount].iMoveType = 0;
         gsTempus.vmPrincipalVariationOld[iPlyCount][iMoveCount].bbCastle = 0;
         gsTempus.vmPrincipalVariationOld[iPlyCount][iMoveCount].iCapture = 0;
         gsTempus.vmPrincipalVariationOld[iPlyCount][iMoveCount].iScore = 0;

      }

   }

}

//
///////////////////////////////////////////////////////////////////////////////////////////
//
void UpdateMultiPV(struct Board* argsBoard,
   struct GeneralMove* argsGeneralMoves,
   int argiScore,
   int argiMoveNumber,
   int argiNumberOfMoves)
{

   // This function is used to udate the multi PV's
   // In the function FirstSearch, a new board strcuture is created for every new move as part
   // of the strcuture for using OpenMP and going deep.

   // Figure out where the move fits in the scheme of things.
   int iFlag = 1;
   int iMoveCounter = -1;

   // Set the depth
   gsTempus.viDepth[argiMoveNumber] = argsBoard->iMaxPlys;

   // Set the score.
   gsTempus.viScore[argiMoveNumber] = argiScore;

   // Loop over the plys
   for (int iPlyCount = 0; iPlyCount < argsBoard->iMaxPlys; iPlyCount++)
   {

      // Take the PV of interest and store it in the globalstructure for holding all of the PVs
      gsTempus.vmPrincipalVariation[iPlyCount][argiMoveNumber].bbFromSquare = argsBoard->vmPrincipalVariation[0][iPlyCount].bbFromSquare;
      gsTempus.vmPrincipalVariation[iPlyCount][argiMoveNumber].bbToSquare = argsBoard->vmPrincipalVariation[0][iPlyCount].bbToSquare;
      gsTempus.vmPrincipalVariation[iPlyCount][argiMoveNumber].bbEPSquare = argsBoard->vmPrincipalVariation[0][iPlyCount].bbEPSquare;
      gsTempus.vmPrincipalVariation[iPlyCount][argiMoveNumber].iPiece = argsBoard->vmPrincipalVariation[0][iPlyCount].iPiece;
      gsTempus.vmPrincipalVariation[iPlyCount][argiMoveNumber].iFromSquare = argsBoard->vmPrincipalVariation[0][iPlyCount].iFromSquare;
      gsTempus.vmPrincipalVariation[iPlyCount][argiMoveNumber].iToSquare = argsBoard->vmPrincipalVariation[0][iPlyCount].iToSquare;
      gsTempus.vmPrincipalVariation[iPlyCount][argiMoveNumber].iPromote = argsBoard->vmPrincipalVariation[0][iPlyCount].iPromote;
      gsTempus.vmPrincipalVariation[iPlyCount][argiMoveNumber].iMoveType = argsBoard->vmPrincipalVariation[0][iPlyCount].iMoveType;
      gsTempus.vmPrincipalVariation[iPlyCount][argiMoveNumber].bbCastle = argsBoard->vmPrincipalVariation[0][iPlyCount].bbCastle;
      gsTempus.vmPrincipalVariation[iPlyCount][argiMoveNumber].iCapture = argsBoard->vmPrincipalVariation[0][iPlyCount].iCapture;
      gsTempus.vmPrincipalVariation[iPlyCount][argiMoveNumber].iScore = argsBoard->vmPrincipalVariation[0][iPlyCount].iScore;

   }

}

//
///////////////////////////////////////////////////////////////////////////////////////////
//
void PrintMultiPV(struct Board* argsBoard,
   struct GeneralMove* argsGeneralMoves,
   int argiNumberOfMoves)
{

   // This routine prints out the top PVs.

      // Sort and find the best scores.
   int viScore[dNumberOfMoves];
   int viIndex[dNumberOfMoves];
   int iSortFlag = 1;
   int iMoveIndex;
   int iMovesToPrint = 0;
   Board sBoard = *argsBoard;

   // Seperate the multi pv from everything else
   cout << "----------------------------------------------------" << endl;
   cout << "Multi PV" << endl << endl;


   // Define the number of moves to print.
   // Set it equal to the lesser of the defined moves or available moves.
   iMovesToPrint = GetNumberOfPVs();
   if (argiNumberOfMoves < iMovesToPrint)
   {

      iMovesToPrint = argiNumberOfMoves;

   }

   // Set up the score vector
   for (iMoveIndex = 0; iMoveIndex < argiNumberOfMoves; iMoveIndex++)
   {

      viScore[iMoveIndex] = gsTempus.viScore[iMoveIndex];
      viIndex[iMoveIndex] = iMoveIndex;

   }

   // Sort the move scores
   while (iSortFlag)
   {

      // Set the default to bail.
      iSortFlag = 0;

      // Use a cocktail sort and to from top to bottom
      for (iMoveIndex = 0; iMoveIndex < argiNumberOfMoves - 1; iMoveIndex++)
      {

         if (viScore[iMoveIndex + 1] > viScore[iMoveIndex])
         {

            int iDummyScore = viScore[iMoveIndex];
            viScore[iMoveIndex] = viScore[iMoveIndex + 1];
            viScore[iMoveIndex + 1] = iDummyScore;

            int iDummyPosition = viIndex[iMoveIndex];
            viIndex[iMoveIndex] = viIndex[iMoveIndex + 1];
            viIndex[iMoveIndex + 1] = iDummyPosition;

            iSortFlag = 1;

         }

      }

   }


   // Loop over the moves and print them.
   for (iMoveIndex = 0; iMoveIndex < iMovesToPrint; iMoveIndex++)
   {

      // Copy the PV of interest into the board structure.
      // Loop over the plys
      for (int iPlyCount = 0; iPlyCount < argsBoard->iMaxPlys; iPlyCount++)
      {

         // Take the PV of interest and store it in the globalstructure for holding all of the PVs
         sBoard.vmPrincipalVariation[0][iPlyCount].bbFromSquare = gsTempus.vmPrincipalVariation[iPlyCount][viIndex[iMoveIndex]].bbFromSquare;
         sBoard.vmPrincipalVariation[0][iPlyCount].bbToSquare = gsTempus.vmPrincipalVariation[iPlyCount][viIndex[iMoveIndex]].bbToSquare;
         sBoard.vmPrincipalVariation[0][iPlyCount].bbEPSquare = gsTempus.vmPrincipalVariation[iPlyCount][viIndex[iMoveIndex]].bbEPSquare;
         sBoard.vmPrincipalVariation[0][iPlyCount].iPiece = gsTempus.vmPrincipalVariation[iPlyCount][viIndex[iMoveIndex]].iPiece;
         sBoard.vmPrincipalVariation[0][iPlyCount].iFromSquare = gsTempus.vmPrincipalVariation[iPlyCount][viIndex[iMoveIndex]].iFromSquare;
         sBoard.vmPrincipalVariation[0][iPlyCount].iToSquare = gsTempus.vmPrincipalVariation[iPlyCount][viIndex[iMoveIndex]].iToSquare;
         sBoard.vmPrincipalVariation[0][iPlyCount].iPromote = gsTempus.vmPrincipalVariation[iPlyCount][viIndex[iMoveIndex]].iPromote;
         sBoard.vmPrincipalVariation[0][iPlyCount].iMoveType = gsTempus.vmPrincipalVariation[iPlyCount][viIndex[iMoveIndex]].iMoveType;
         sBoard.vmPrincipalVariation[0][iPlyCount].bbCastle = gsTempus.vmPrincipalVariation[iPlyCount][viIndex[iMoveIndex]].bbCastle;
         sBoard.vmPrincipalVariation[0][iPlyCount].iCapture = gsTempus.vmPrincipalVariation[iPlyCount][viIndex[iMoveIndex]].iCapture;
         sBoard.vmPrincipalVariation[0][iPlyCount].iScore = gsTempus.vmPrincipalVariation[iPlyCount][viIndex[iMoveIndex]].iScore;

      }

      // Print this principle variation.
      cout << gsTempus.viScore[viIndex[iMoveIndex]] << " ";
      PrintPrincipalVariation(&sBoard,
         argsGeneralMoves);

   }

   // Show the end of the multi PV.
   cout << "----------------------------------------------------" << endl;

}


//
//---------------------------------------------------------------------------------------
//
void UserSetUp(struct Board* argsBoard,
   struct GeneralMove* argsGeneralMoves)
{

   // Debug the input.
   assert(argsBoard >= 0);
   assert(argsGeneralMoves >= 0);
   int iControl = 1;
   char strMove[10];
   int iMoveCount = -1;



   while (iControl)
   {

      // Input a user move, user plays white.
      if (GetInterfaceMode() == dConsole)
      {

         PrintBoard(argsBoard->mBoard);

      }

      InputUserMove(argsBoard,
         argsGeneralMoves);

      iMoveCount++;

      if (GetInterfaceMode() == dConsole)
      {

         // Write out the potential moves to the screen.
         cout << endl;

         // Input the user move.
         cout << "Start to undo moves? (y/n) = ";

      }

      cin >> strMove;

      // See if the input was a numeric character.
      if (cin.fail())
      {

         // Clear the input.
         cin.clear();
         char c[64];
         cin >> c;


      }

      if (!strncmp(strMove, "y", 1) ||
         !strncmp(strMove, "Y", 1))
      {

         iControl = 0;

      }

   } // end loop over input.

   for (int iCounter = 0; iCounter < iMoveCount; iCounter++)
   {

      UndoMove(argsBoard,
         argsGeneralMoves);
      PrintBoard(argsBoard->mBoard);
      system("Pause");

   }


}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Wrapper fruntions for setting items on the Tempus strcutre;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Set some basic parameters.
void SetSearchDepth(int argiDepth)
{
   assert(argiDepth >= 0);
   gsTempus.giSearchDepth = argiDepth;
}
void SetSearchTimeInMiliSeconds(int argiTime)
{
   assert(argiTime >= 0);
   gsTempus.giSearchTime = argiTime;
}
void SetSearchMonitorTogel(int iTogel)
{
   assert(iTogel >= 0);
   assert(iTogel <= 1);
   gsTempus.giSearchMonitorTogel = iTogel;
}
void SetPersistantKeys(int iTogel)
{
   assert(iTogel >= 0);
   (iTogel <= dCode);
   gsTempus.giPersistantKeys = iTogel;
}
void SetModeOfPlay(int iTogel)
{
   assert(iTogel >= 0);
   assert(iTogel <= 20);
   gsTempus.giModeOfPlay = iTogel;
}
void SetDebug(int iTogel)
{
   assert(iTogel >= 0);
   assert(iTogel <= 1);
   gsTempus.giDebug = iTogel;
}
void SetUseHashTable(int iTogel)
{
   assert(iTogel >= 0);
   assert(iTogel <= 1);
   gsTempus.giUseHashTable = iTogel;
}
void SetOpeningBookDebug(int iTogel)
{
   assert(iTogel >= 0);
   assert(iTogel <= 1);
   gsTempus.giOpeningBookDebug = iTogel;
}
void SetUseOpeningBook(int iTogel)
{
   assert(iTogel >= 0);
   assert(iTogel <= 1);
   gsTempus.giUseOpeningBook = iTogel;
}
void SetComputerColor(int iColor)
{
   assert(iColor >= 0);
   assert(iColor <= 1);
   gsTempus.giComputerColor = iColor;
}
void SetInterfaceMode(int iMode)
{
   assert(iMode >= 0);
   //assert( iTogel <= 2 );
   gsTempus.giInterfaceMode = iMode;
}
void SetStopGo(int iTogel)
{
   assert(iTogel >= 0);
   assert(iTogel <= 1);
   gsTempus.giStopGo = iTogel;
}
void SetOpeningBookAnalysis(int iTogel)
{
   assert(iTogel >= 0);
   assert(iTogel <= 1);
   gsTempus.giOpeningBookAnalysis = iTogel;
}
void SetLimitELO(int iTogel)
{
   assert(iTogel >= 0);
   assert(iTogel <= 1);
   gsTempus.giLimitELO = iTogel;
}
void SetBlackIncrementalTime(int iTime)
{
   assert(iTime >= 0);
   gsTempus.giBlackIncrementalTime = iTime;
}
void SetBlackTime(int iTime)
{
   assert(iTime >= 0);
   gsTempus.giBlackTime = iTime;
}
void SetMoveTime(int iTime)
{
   assert(iTime >= 0);
   gsTempus.giMoveTime = iTime;
}
void SetWhiteIncrementalTime(int iTime)
{
   assert(iTime >= 0);
   gsTempus.giWhiteIncrementalTime = iTime;
}
void SetWhiteTime(int iTime)
{
   assert(iTime >= 0);
   gsTempus.giWhiteTime = iTime;
}
void SetMovesToGo(int iMovesToGo)
{
   assert(iMovesToGo >= 0);
   gsTempus.giMovesToGo = iMovesToGo;
}
void SetNodes(long long iNodes)
{
   assert(iNodes >= 0);
   gsTempus.giNodes = iNodes;
}
void SetMate(int iMate)
{
   assert(iMate >= 0);
   gsTempus.giMate = iMate;
}
void SetInterfaceDebug(int iTogel)
{
   assert(iTogel >= 0);
   assert(iTogel <= 1);
   gsTempus.giInterfaceDebug = iTogel;
}
void SetNumberOfNodes(BitBoard iNodes)
{
   assert(iNodes >= 0);
   gsTempus.gbbNumberOfNodes = iNodes;
}
void SetNumberOfIncrementalNodes(BitBoard iNodes)
{
   assert(iNodes >= 0);
   gsTempus.gbbNumberOfIncrementalNodes = iNodes;
}
void SetNodeCheck(BitBoard iTogel)
{
   assert(iTogel >= 0);
   gsTempus.gbbiNodeCheck = iTogel;
}
void SetNumberOfTranspositions(BitBoard iNode)
{
   assert(iNode >= 0);
   gsTempus.gnniNumberOfTranspositions = iNode;
}
void SetStart(int iTime)
{
   assert(iTime >= 0);
   gsTempus.gcStart = iTime;
}
void SetEnd(int iTime)
{
   assert(iTime >= 0);
   gsTempus.gcFinish = iTime;
}
void SetSearchStartTime()
{
   gsTempus.giSearchStartTime = clock();
}
void SetSearchStopTime()
{
   gsTempus.giSearchStopTime = clock();
}
void SetPollingCount(int iCount)
{
   assert(iCount >= 0);
   gsTempus.gbbPollingCount = iCount;
}
void IncrementPollingCount()
{
   gsTempus.gbbPollingCount++;
}
void SetUseNullMove(int iMode)
{
   assert(iMode == 0 || iMode == 1);
   gsTempus.iUseNullMove = iMode;
}
void SetUseNullEval(int iMode)
{
   assert(iMode == 0 || iMode == 1);
   gsTempus.iUseNullEval = iMode;
}
void SetNullReduction(int iMode)
{
   assert(iMode >= 0 && iMode <= 5);
   gsTempus.iNullReduction = iMode;
}
void SetLMRMinimumMoveSearch(int iMoves)
{
   assert(iMoves >= 0);
   assert(iMoves <= dNumberOfMoves);
   gsTempus.iMinimumMovesSearched = iMoves;
}
void SetLMRMinimumMoveScore(int iScore)
{
   assert(iScore >= 0);
   gsTempus.iMinimumScoreSearched = iScore;
}
void SetLMRReducedSearchDepth(int iDepth)
{
   assert(iDepth >= 0);
   gsTempus.iReducedSearchDepth = iDepth;
}
void SetUseLMR(int iMode)
{
   assert(iMode < 2);
   assert(iMode > -1);
   gsTempus.iUseLMR = iMode;
}
void SetScore(int iScore)
{

   gsTempus.iScore = iScore;

}
void SetNumberOfPVs(int iNumberOfPVs)
{
   gsTempus.iNumberOfPVs = iNumberOfPVs;
}
void SetPonder(int iPonder)
{
   gsTempus.iPonder = iPonder;
}
void SetTimeLastMoveUpdate()
{
   gsTempus.iTimeOfLastMoveUpdate = clock();
}
void SetConsoleOutput(int iYesNo)
{
   gsTempus.giConsoleOutput = iYesNo;
}
void SetResetHash(int iYesNo)
{
   assert(iYesNo >= 0);
   assert(iYesNo <= 1);
   gsTempus.giResetHash = iYesNo;
}
void SetIsInBook(int iYesNo)
{
   assert(iYesNo >= 0);
   assert(iYesNo <= 1);
   gsTempus.iInBook = iYesNo;
}
void SetInterfaceMovesDebug(int iYesNo)
{
   assert(iYesNo >= 0);
   assert(iYesNo <= 1);
   gsTempus.iInterfaceMoveDebug = iYesNo;
}
void SetInterfaceBookDebug(int iYesNo)
{
   assert(iYesNo >= 0);
   assert(iYesNo <= 1);
   gsTempus.iInterfaceBookDebug = iYesNo;
}
void SetAspirationSearch(int iYesNo)
{
   assert(iYesNo >= 0);
   assert(iYesNo <= 1);
   gsTempus.iAspirationSearch = iYesNo;
}
void SetNumberOfNodesSearched(int iNumber)
{
   assert(iNumber > 0);
   gsTempus.iNumberOfNodesSearched = iNumber;
}



// Get some basic parameters.
int GetSearchDepth()
{
   return gsTempus.giSearchDepth;
}
int GetSearchTimeInMiliSeconds()
{
   return gsTempus.giSearchTime;
}
int GetSearchMonitorTogel()
{
   return gsTempus.giSearchMonitorTogel;
}
int GetPersistantKeys()
{
   return gsTempus.giPersistantKeys;
}
int GetModeOfPlay()
{
   return gsTempus.giModeOfPlay;
}
int GetDebug()
{
   return gsTempus.giDebug;
}
int GetUseHashTable()
{
   return gsTempus.giUseHashTable;
}
int GetOpeningBookDebug()
{
   return gsTempus.giOpeningBookDebug;
}
int GetUseOpeningBook()
{
   return gsTempus.giUseOpeningBook;
}
int GetComputerColor()
{
   return gsTempus.giComputerColor;
}
int GetInterfaceMode()
{
   return gsTempus.giInterfaceMode;
}
int GetStopGo()
{
   return  gsTempus.giStopGo;
}
int GetOpeningBookAnalysis()
{
   return gsTempus.giOpeningBookAnalysis;
}
int GetLimitELO()
{
   return gsTempus.giLimitELO;
}
int GetBlackIncrementalTime()
{
   return gsTempus.giBlackIncrementalTime;
}
int GetBlackTime()
{
   return gsTempus.giBlackTime;
}
int GetMoveTime()
{
   return gsTempus.giMoveTime;
}
int GetWhiteIncrementalTime()
{
   return gsTempus.giWhiteIncrementalTime;
}
int GetWhiteTime()
{
   return gsTempus.giWhiteTime;
}
int GetMovesToGo()
{
   return gsTempus.giMovesToGo;
}
long long GetNodes()
{
   return gsTempus.giNodes;
}
int GetMate()
{
   return gsTempus.giMate;
}
int GetInterfaceDebug()
{
   return gsTempus.giInterfaceDebug;
}
BitBoard GetNumberOfNodes()
{
   return gsTempus.gbbNumberOfNodes;
}
BitBoard GetNumberOfIncrementalNodes()
{
   return gsTempus.gbbNumberOfIncrementalNodes;
}
BitBoard GetNodeCheck()
{
   return gsTempus.gbbiNodeCheck;
}
BitBoard GetNumberOfTranspositions()
{
   return gsTempus.gnniNumberOfTranspositions;
}
int GetStart()
{
   return gsTempus.gcStart;
}
int GetEnd()
{
   return gsTempus.gcFinish;
}
int GetSearchStartTime()
{
   return gsTempus.giSearchStartTime;
}
int GetSearchStopTime()
{
   return gsTempus.giSearchStopTime;
}
BitBoard GetPollingCount()
{
   return gsTempus.gbbPollingCount;
}
int GetUseNullMove()
{
   return gsTempus.iUseNullMove;
}
int GetUseNullEval()
{
   return gsTempus.iUseNullEval;
}
int GetNullReduction()
{
   return gsTempus.iNullReduction;
}
int GetLMRMinimumMoveSearch()
{
   return gsTempus.iMinimumMovesSearched;
}
int GetLMRMinimumMoveScore()
{
   return gsTempus.iMinimumScoreSearched;
}
int GetLMRReducedSearchDepth()
{
   return gsTempus.iReducedSearchDepth;
}
int GetUseLMR()
{
   return gsTempus.iUseLMR;
}
int GetScore()
{
   return gsTempus.iScore;
}
int GetNumberOfPVs()
{
   return gsTempus.iNumberOfPVs;
}
int GetPonder()
{
   return gsTempus.iPonder;
}
int GetTimeSinceLastMoveUpdate()
{
   return clock() - gsTempus.iTimeOfLastMoveUpdate;
}
int GetConsoleOutput()
{
   return gsTempus.giConsoleOutput;
}
int GetResetHash()
{
   return gsTempus.giResetHash;
}
int GetIsInBook()
{
   return gsTempus.iInBook;
}
int GetInterfaceMovesDebug()
{
   return gsTempus.iInterfaceMoveDebug;
}
int GetInterfaceBookDebug()
{
   return gsTempus.iInterfaceBookDebug;
}
int GetAspirationSearch()
{
   return gsTempus.iAspirationSearch;
}
int GetNumberOfNodesSearched()
{
   return gsTempus.iNumberOfNodesSearched;
}