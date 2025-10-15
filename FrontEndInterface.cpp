#define _CRT_SECURE_NO_WARNINGS
#include "Definitions.h"
#include "Functions.h"
#include "Structures.h"

#include <iostream>
#include <Windows.h>
#include <fstream>
#include <io.h>
#include <string>    // For std::string
#include <vector>    // For std::vector (used for searchmoves)
#include <sstream>   // For std::stringstream (used for parsing)
#include <algorithm> // For std::find

using namespace std;

// --- Global Variables for Interface ---

// Give the program a global pipe, unix style.
static int giPipe;

// Give the program a global handle to the standard input
static HANDLE ghStandardInput;

// Create a global file for debugging.
ofstream gofDebug;

// New: Store the list of moves for a "go searchmoves" command.
// This allows the search function to access it without changing its signature.
static std::vector<std::string> g_searchMoves;

// --- New Helper Functions ---

// New: Helper function to get the restricted list of search moves.
// Your ComputerMove() function should call this to check if the search is restricted.
const std::vector<std::string>& GetSearchMoves()
{
   return g_searchMoves;
}

// New: Sends a formatted UCI "info" command.
// CALL THIS PERIODICALLY FROM YOUR SEARCH FUNCTION (e.g., ComputerMove)
// to update the GUI on search progress.
void SendInfoCommand(int depth, int score, long long nodes, unsigned long time_ms, const std::string& pv)
{
   long long nps = (time_ms > 0) ? (nodes * 1000) / time_ms : 0;
   // In UCI, scores are from the side to move's perspective.
   // A score like "mate 5" means the engine has found a mate in 5.
   // A score like "mate -5" means the engine is getting mated in 5.
   // For centipawns, use "score cp".
   cout << "info depth " << depth << " score cp " << score
      << " nodes " << nodes << " nps " << nps << " pv " << pv << endl;
}

// New: Sends the final "bestmove" command.
// CALL THIS AT THE END OF YOUR SEARCH FUNCTION (e.g., ComputerMove)
// to report the final result.
void SendBestMoveCommand(const std::string& bestMove, const std::string& ponderMove)
{
   cout << "bestmove " << bestMove;
   if (!ponderMove.empty())
   {
      cout << " ponder " << ponderMove;
   }
   cout << endl;
}


// --- Core Interface Functions ---

// Initialize the communications.
void InitializeCommunications()
{
   // Create a variable to hold the mode type.
   unsigned long  lpMode;

   // Don't allow buffering
   setvbuf(stdout, NULL, _IONBF, 0);
   setvbuf(stdin, NULL, _IONBF, 0);
   fflush(NULL);

   // Create the handle
   ghStandardInput = GetStdHandle(STD_INPUT_HANDLE);

   // Set the pipe
   giPipe = !GetConsoleMode(ghStandardInput, &lpMode);

   // Set the initial task.
   SetStopGo(dStop);

   // Set the debug to no.
   SetDebug(dNo);

   // If the pipe failed to open then set the mode.
   if (!giPipe)
   {
      SetConsoleMode(ghStandardInput, lpMode & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
      FlushConsoleInputBuffer(ghStandardInput);
   }

   // Open the debug file if needed.  
   if (GetInterfaceDebug())
   {
      gofDebug.open("InterfaceLog.txt", ios::out | ios::app);
      gofDebug << endl;
      gofDebug << "Interface started from interface." << endl << endl;
   }
}

//
//---------------------------------------------------------------------------------------------------------
//
void CloseCommunications()
{
   // Close debug files as needed.
   if (GetInterfaceBookDebug())
   {
      CloseBookDebug();
   }
   if (GetInterfaceMovesDebug())
   {
      CloseMoveDebug();
   }
   if (GetInterfaceDebug())
   {
      gofDebug << "Closing file." << endl << endl;
      gofDebug.close();
   }
}

//
//------------------------------------------------------------------------------------------------
//
// Check for input.
int CheckForInput()
{
   // Create a variable to hold the mode type.
   DWORD dwInput;
   int iPassFail = 0;

   // Switch on the type of console we are running
   if (giPipe == 0)
   {
      GetNumberOfConsoleInputEvents(ghStandardInput, &dwInput);
      // In console mode, any input is treated as a command (e.g., for "stop")
      if (dwInput > 1)
      {
         return 1;
      }
      return 0;
   }
   else
   {
      // For pipes, PeekNamedPipe is used. It checks for data in the pipe without blocking.
      iPassFail = PeekNamedPipe(ghStandardInput, NULL, 0, NULL, &dwInput, NULL);
      if (iPassFail != 0 && dwInput > 0)
      {
         return 1;
      }
      return 0;
   }
}

//
//--------------------------------------------------------------------------------------------
//      
void ReadInputAndExecute(struct Board* argsBoard, struct GeneralMove* argsGeneralMoves)
{
   // Debug the inputs.
   assert(argsBoard >= 0);
   assert(argsGeneralMoves >= 0);

   // Use std::string for safer and easier string handling.
   string line;

   // Read a complete line from stdin. This is more reliable than a fixed-size buffer.
   if (getline(cin, line))
   {
      if (gofDebug.is_open())
      {
         gofDebug << "Full string in from interface = " << line << endl;
      }

      // In UCI, one line is one command.
      CommandFromInterface(line, argsBoard, argsGeneralMoves);
   }
}

//
//------------------------------------------------------------------------------------------------
//
void SendCommand(const char* argstrCommand)
{
   // Send a command to the interface. Notice the end of line character is now handled by endl.
   // Always use this routine to send a command to make sure we have the end of line.

   // Debug.
   if (GetInterfaceDebug())
   {
      gofDebug << "Sending a command to the interface = " << argstrCommand << endl << endl;
   }

   // Don't send an empty string.
   if (argstrCommand && argstrCommand[0] != '\0')
   {
      // Using cout and endl is safer and more idiomatic C++
      cout << argstrCommand << endl;
   }
}

//
//------------------------------------------------------------------------------------------------------------------
//
void CommandFromInterface(std::string argstrCommand, struct Board* argsBoard, struct GeneralMove* argsGeneralMoves)
{
   // Debug the inputs.
   assert(argsBoard >= 0);
   assert(argsGeneralMoves >= 0);

   // Use string streams for easy parsing
   std::stringstream ss(argstrCommand);
   std::string command;
   ss >> command;

   if (GetInterfaceDebug())
   {
      gofDebug << "Processing command: " << command << " | Full line: " << argstrCommand << endl << endl;
   }

   if (command == "uci")
   {
      SetInterfaceMode(dUCI);
      SendCommand("id name DeepViolet " dsVersion);
      SendCommand("id author Dr. Jay Lindgren");

      // New: Declare supported options to the GUI, per UCI spec.
      // Customize these to match your engine's actual options.
      SendCommand("option name Hash type spin default 128 min 1 max 2048");
      SendCommand("option name Ponder type check default false");
      SendCommand("option name OwnBook type check default true");

      SendCommand("uciok");
   }
   else if (command == "debug")
   {
      std::string action;
      ss >> action;
      if (action == "on") SetDebug(dYes);
      else SetDebug(dNo);
   }
   else if (command == "isready")
   {
      SendCommand("readyok");
   }
   else if (command == "setoption")
   {
      std::string temp, name, value;
      ss >> temp; // "name"
      // Loop to read option names that may contain spaces
      while (ss >> temp && temp != "value")
      {
         if (!name.empty()) name += " ";
         name += temp;
      }
      // Loop to read option values that may contain spaces
      while (ss >> temp)
      {
         if (!value.empty()) value += " ";
         value += temp;
      }

      if (name == "Hash")
      {
         int hash_size_mb = std::stoi(value);
         // ToDo: Add your logic to resize the transposition table here.
      }
      // Add other options here
   }
   else if (command == "ucinewgame")
   {
      CreateBoard(argsBoard, argsGeneralMoves);
      InitializeHashTable(); // Reset hash table for a new game
   }
   else if (command == "position")
   {
      PositionCommand(argstrCommand, argsBoard, argsGeneralMoves);
   }
   else if (command == "go")
   {
      GoCommand(argstrCommand, argsBoard, argsGeneralMoves);
   }
   else if (command == "stop")
   {
      SetStopGo(dStop); // Signal the search to stop
   }
   else if (command == "ponderhit")
   {
      // The user made the move we were pondering. Switch to normal search.
      SetStopGo(dGo);
   }
   else if (command == "quit")
   {
      SetInterfaceMode(dConsole); // Breaks the main loop in main()
   }
   else if (command == "printboard") // For your own debugging
   {
      PrintBoard(argsBoard->mBoard);
   }
}

//
//------------------------------------------------------------------
//
// Rewritten: Parse through and act upon a "position" command using modern C++.
//
void PositionCommand(std::string argstrCommand, struct Board* argsBoard, struct GeneralMove* argsGeneralMoves)
{
   std::stringstream ss(argstrCommand);
   std::string token;
   ss >> token; // Consume "position"

   // Set up the board from FEN or startpos
   ss >> token;
   if (token == "startpos")
   {
      ReadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", argsBoard, argsGeneralMoves, 2);
      ss >> token; // Consume "moves" if it exists
   }
   else if (token == "fen")
   {
      std::string fen;
      while (ss >> token && token != "moves")
      {
         fen += token + " ";
      }
      ReadFEN(fen.c_str(), argsBoard, argsGeneralMoves, 2);
   }

   // Process the moves list
   if (token == "moves")
   {
      Move vsMoveList[dNumberOfMoves];
      int iMoveNumber = -1;
      char moveStr[10]; // FIX: Create a mutable buffer for FindAlgebraicMove

      while (ss >> token) // Read each move string (e.g., "e2e4")
      {
         CalculateMoves(vsMoveList, argsBoard, argsGeneralMoves);

         // FIX: Copy the const string from token.c_str() into the mutable buffer
         strcpy_s(moveStr, sizeof(moveStr), token.c_str());

         FindAlgebraicMove(argsBoard, argsGeneralMoves, moveStr, vsMoveList, &iMoveNumber);

         if (iMoveNumber >= 0 && iMoveNumber < dNumberOfMoves)
         {
            MakeMove(vsMoveList, argsBoard, argsGeneralMoves, iMoveNumber);
         }
      }
   }
}

//
//--------------------------------------------------------------------------------------------------------------------------------
//
// Rewritten: Parse through and act upon a "go" command using modern C++.
//
void GoCommand(std::string argstrCommand, struct Board* argsBoard, struct GeneralMove* argsGeneralMoves)
{
   // Debug the inputs.
   assert(argsBoard >= 0);
   assert(argsGeneralMoves >= 0);

   // Use stringstream for robust parsing
   std::stringstream ss(argstrCommand);
   std::string token;
   ss >> token; // Consume "go"

   // Clear any previous search move restrictions
   g_searchMoves.clear();

   // Default search parameters
   SetSearchDepth(dInfiniteDepth);
   SetSearchTimeInMiliSeconds(dInfiniteTime);

   // Parse all options from the "go" string
   while (ss >> token)
   {
      if (token == "wtime") ss >> token, SetWhiteTime(std::stoi(token));
      else if (token == "btime") ss >> token, SetBlackTime(std::stoi(token));
      else if (token == "winc") ss >> token, SetWhiteIncrementalTime(std::stoi(token));
      else if (token == "binc") ss >> token, SetBlackIncrementalTime(std::stoi(token));
      else if (token == "movestogo") ss >> token, SetMovesToGo(std::stoi(token));
      else if (token == "depth")
      {
         ss >> token;
         SetSearchDepth(std::stoi(token));
         SetSearchTimeInMiliSeconds(dInfiniteTime); // Depth search overrides time
      }
      else if (token == "nodes")
      {
         ss >> token;
         SetNodes(std::stoll(token)); // Using stoll for long long
      }
      else if (token == "mate") ss >> token, SetMate(std::stoi(token));
      else if (token == "movetime")
      {
         ss >> token;
         SetSearchTimeInMiliSeconds(std::stoi(token));
         SetSearchDepth(dInfiniteDepth); // Movetime search overrides depth
      }
      else if (token == "infinite")
      {
         SetUseOpeningBook(dNo);
         SetSearchDepth(dInfiniteDepth);
         SetSearchTimeInMiliSeconds(dInfiniteTime);
      }
      else if (token == "ponder")
      {
         SetPonder(dYes);
      }
      else if (token == "searchmoves")
      {
         // New: Correctly parse the list of moves to search
         while (ss >> token)
         {
            g_searchMoves.push_back(token);
         }
      }
   }

   // --- Time Management ---
   if (GetSearchDepth() == dInfiniteDepth && GetSearchTimeInMiliSeconds() == dInfiniteTime && GetNodes() == 0)
   {
      if (argsBoard->siColorToMove == dWhite)
      {
         SetComputerColor(dComputerWhite);
      }
      else
      {
         SetComputerColor(dComputerBlack);
      }
      CalculateTimeForMove(argsBoard);
   }

   // --- Start Search ---
   SetSearchStartTime();
   SetStopGo(dGo); // Signal that the search should start

   ComputerMove(argsBoard, argsGeneralMoves);
}