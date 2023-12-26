
#include "Definitions.h"
#include "Functions.h"
#include "Structures.h"

#include <iostream>
#include <Windows.h>
#include <fstream>
#include <io.h>

using namespace std;

// Give the program a global pipe, unix style.
static int giPipe;

// Give the program a global handle to the standard input
static HANDLE ghStandardInput;

// Create a global file for debugging.
ofstream gofDebug;

// Initialize the communications.
void InitializeCommunications()
{
   // Create a variable to hold the mode type.
   unsigned long  lpMode;

   // Don't allow buffering
   setbuf( stdout, NULL );
   setbuf( stdin, NULL );
   setvbuf( stdout, NULL, _IONBF, 0 );
   setvbuf( stdin, NULL, _IONBF, 0 );
   fflush( NULL );  

   // Create the handle
   ghStandardInput = GetStdHandle( STD_INPUT_HANDLE );

   // Set the pipe
   giPipe = !GetConsoleMode( ghStandardInput, 
                             & lpMode );

   // Set the initial task.
   SetStopGo( dStop );

   // Set the debug to no.
   SetDebug( dNo );

   // If the pipe failed to open then set the mode.
   if ( !giPipe )
   {

        SetConsoleMode( ghStandardInput,
                        lpMode&~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT) );
        FlushConsoleInputBuffer( ghStandardInput );

   }

   // Open the debug file if needed.  
   if ( GetInterfaceDebug() )
   {
//  filestr.open ("test.txt", fstream::in | fstream::out | fstream::app);

      gofDebug.open( "InterfaceLog.txt", ios::out | ios::app );
      gofDebug << endl;
      gofDebug << "Interface started from interface." << endl << endl;

   }

}

//
//---------------------------------------------------------------------------------------------------------
//
void CloseCommunications()
{

   // Close degug files as needed.
   if ( GetInterfaceBookDebug() )
   {

      CloseBookDebug();

   }

   // Close degug files as needed.
   if ( GetInterfaceMovesDebug() )
   {

      CloseMoveDebug();

   }

   // Close down the communications.
   if ( GetInterfaceDebug() )
   {

      gofDebug << "Closing file." << endl;
      gofDebug << endl;
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
   // Is the console up or are we using a pipe.
   if ( giPipe == 0 )
   {

      GetNumberOfConsoleInputEvents( ghStandardInput,
                                     & dwInput );

      if ( dwInput > 1 )  // Damn the Microsoft Corporation
      {
   
         return 1;
      
      }
      else
      {

         return 0;

      }

   }
   else
   {

      // If the peek works, look at the data.
      // Damn the microsoft Corporation
      // Damn them to hell!!!!
      // The PeekNamedPipe function only fails if no eol is found!!!!!!
      // Don't contol the input ono the number of bits found
      // Control int the input on if the function fails!!!!!
      // Damn the Microsoft corporation!
   
      iPassFail = PeekNamedPipe( ghStandardInput, 
                                 NULL, 
                                 0, 
                                 NULL, 
                                 & dwInput, 
                                 NULL );
   
      if ( iPassFail == 0 )
      {

         return 1;

      }
      else
      {

         return 0;

      }

   }

}
//
//--------------------------------------------------------------------------------------------
//      
void ReadInputAndExecute( struct Board * argsBoard,
                          struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Allocate a string for the command.
   char strCommand[ dStringSize ];
   char strSingleCommand[ dStringSize ];
   int iFlag = 1;
   int iCommandCount = 0;
   char * strEndOfLine;

   // Clear the strings
   * ( strCommand ) = 0;
   * ( strSingleCommand ) = 0;
   strEndOfLine = 0;

   // Pull a string from the standard input
   int bytes = _read( _fileno( stdin ),
                     strCommand,
                     2048 );

   // Write out the full string.  Suspecting that we might be getting multiple commands at a time.
   gofDebug << "Full string in from interface = " << strCommand  << endl;

   // Point the end of line to the start of the tring to start.
   strEndOfLine = strCommand;

   // Count the number of commands in the lines.
   while( iFlag )
   {

      strEndOfLine = strchr( strEndOfLine, '\n' );

      // See if there is an end of line in the command.
      if ( strEndOfLine != NULL )
      {

         // Increment the counter
         iCommandCount++;
   
         // Increment the pointer.
         strEndOfLine++;

      }
      else
      {

         // Get out of this loop.
         iFlag = 0;

      }

   }

   // Let the users at home know if we have more than one command.
   if ( iCommandCount > 1 )
   {

      gofDebug << "Command Count = " << iCommandCount  << endl;
      //gofDebug << "Full string in from interface = " << strCommand  << endl;

   }

   // Go back over the command and clean up the string and get rid of the noise.
   // Cleaning is done just after this loop.
   strEndOfLine = strCommand;
   for ( int iCommand = 0; iCommand < iCommandCount; iCommand++ )
   {

      strEndOfLine = strchr( strEndOfLine, '\n' );
      strEndOfLine++;

   }

   // Clear the end of line. Being sure to remove the EOL from the command
   ( * strEndOfLine ) = 0;

   // Loop over the lines in the command.
   for ( int iCommand = 0; iCommand < iCommandCount; iCommand++ )
   {

      // Clear the single command.
      * (strSingleCommand ) = 0;

      // Copy the command string into the single command string
      strcpy( strSingleCommand, strCommand );

      // Log the substring being used.
      //gofDebug << "1string being processed =  " << strSingleCommand << endl;

      // In the future, switch here depending on the interface.
      CommandFromInterface( strSingleCommand,
                            argsBoard,
                            argsGeneralMoves );

      // Now clean up the full command.
      iFlag = 1;
      while( iFlag )
      {

         if ( strCommand[ 0 ] != '\n' )
         {

            memmove( strCommand, strCommand + 1, strlen( strCommand ) );

         }
         else
         {

            iFlag = 0;

            memmove( strCommand, strCommand + 1, strlen( strCommand ) );
            
         }

      }

   }

}


//
//------------------------------------------------------------------------------------------------
//
void SendCommand( const char * argstrCommand )
{

   // Send a command to the interface.  Notice the end of line character.
   // Always use this routine to send a command to make sure we have the end of line.

   // Debug.
   if ( GetInterfaceDebug() )
   {

      gofDebug << "Sending a command to the interface = " << argstrCommand << endl << endl;

   }

   // Don't send an empty string.
   if ( argstrCommand[ 0 ] != '\0' )
   {

      printf( "%s\n", argstrCommand );

   }

}
 
//
//------------------------------------------------------------------------------------------------------------------
//
void CommandFromInterface( char * argstrCommand,
                           struct Board * argsBoard,
                           struct GeneralMove * argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Declare some strings.
   char strString[ 64 ] = "UCI Test";
   char strTest[ 64 ] = "UCI";
   char * pBeginning;

   // Debug.
   if ( GetInterfaceDebug() )
   {

      //gofDebug << "In CommandFromInterface." << endl;
      gofDebug << "String in from interface  = " << argstrCommand << endl << endl;

   }

   // See if we have found the string.
   pBeginning = strstr( strString, strTest );

   // Look for the initial command telling the program that it is using the UCI interface.
   if ( strstr( argstrCommand, "uci" ) )
   {
/*

* uci
	tell engine to use the uci (universal chess interface),
	this will be sent once as a first command after program boot
	to tell the engine to switch to uci mode.
	After receiving the uci command the engine must identify itself with the "id" command
	and send the "option" commands to tell the GUI which engine settings the engine supports if any.
	After that the engine should send "uciok" to acknowledge the uci mode.
	If no uciok is sent within a certain time period, the engine task will be killed by the GUI.
*/
      // Set the global mode to UCI.
      SetInterfaceMode( dUCI );

      // Talk back to the interface and give it some basic info and tell the interface that
      // the engine is ready.
      SendCommand( "id name DeepViolet " dsVersion );
      SendCommand( "id author Dr. Jay Lindgren" );
      SendCommand( "uciok" );

   }

   // Turn on debug mode.
   if ( strstr( argstrCommand, "debug" ) )
   {

/*
* debug [ on | off ]
	switch the debug mode of the engine on and off.
	In debug mode the engine should send additional infos to the GUI, e.g. with the "info string" command,
	to help debugging, e.g. the commands that the engine has received etc.
	This mode should be switched off by default and this command can be sent
	any time, also when the engine is thinking.
*/

      // scan the names to see what the action is.
      char strAction[ 256 ];
      sscanf( argstrCommand, "debug  %s", & strAction );

      // Find the on or off.
      if ( ! strcmp( strAction, "on" ) )
      {
         
         SetDebug( dYes );

      }
      else
      {

         SetDebug( dNo );

      }

   }

   // Ready, OK! (cheerleader joke)
   if ( strstr( argstrCommand, "isready" ) )
   {
/*
* isready
	this is used to synchronize the engine with the GUI. When the GUI has sent a command or
	multiple commands that can take some time to complete,
	this command can be used to wait for the engine to be ready again or
	to ping the engine to find out if it is still alive.
	E.g. this should be sent after setting the path to the tablebases as this can take some time.
	This command is also required once before the engine is asked to do any search
	to wait for the engine to finish initializing.
	This command must always be answered with "readyok" and can be sent also when the engine is calculating
	in which case the engine should also immediately answer with "readyok" without stopping the search.
*/
      SendCommand( "readyok" );

   }

   // Set an option. Look for the first 9 letters to contain "setoption"
   if ( strstr( argstrCommand, "setoption" ) )
   {

      // See which option to set.
      char strName[ 256 ];
      char strValue[ 256 ];

      // Scan the values into the new names.
      sscanf( argstrCommand, "setoption name %s value %s", & strName, & strValue );

      // Switch on the options.
      if ( ! strcmp( strName, "Hash" ) )
      {
/*
* setoption name <id> [value <x>]
	this is sent to the engine when the user wants to change the internal parameters
	of the engine. For the "button" type no value is needed.
	One string will be sent for each parameter and this will only be sent when the engine is waiting.
	The name and value of the option in <id> should not be case sensitive and can inlude spaces.
	The substrings "value" and "name" should be avoided in <id> and <x> to allow unambiguous parsing,
	for example do not use <name> = "draw value".
	Here are some strings for the example below:
	   "setoption name Nullmove value true\n"
      "setoption name Selectivity value 3\n"
	   "setoption name Style value Risky\n"
	   "setoption name Clear Hash\n"
	   "setoption name NalimovPath value c:\chess\tb\4;c:\chess\tb\5\n"
*/
         int iSize = 0;
         //sscanf( iSize, "%d", & strValue );
         //  - write a routine to resize the hash table here.

      }

   }

   // Register the engine.
   if ( strstr( argstrCommand, "register" ) )
   {

/*
* register
	this is the command to try to register an engine or to tell the engine that registration
	will be done later. This command should always be sent if the engine	has sent "registration error"
	at program startup.
	The following tokens are allowed:
	* later
	   the user doesn't want to register the engine now.
	* name <x>
	   the engine should be registered with the name <x>
	* code <y>
	   the engine should be registered with the code <y>
	Example:
	   "register later"
	   "register name Stefan MK code 4359874324"
*/

      SendCommand( "register name Deep Violet" );

   }

   // Start a new game.
   if ( strstr( argstrCommand, "ucinewgame" ) )
   {

/*      
* ucinewgame
   this is sent to the engine when the next search (started with "position" and "go") will be from
   a different game. This can be a new game the engine should play or a new game it should analyse but
   also the next position from a testsuite with positions only.
   If the GUI hasn't sent a "ucinewgame" before the first "position" command, the engine shouldn't
   expect any further ucinewgame commands as the GUI is probably not supporting the ucinewgame command.
   So the engine should not rely on this command even though all new GUIs should support it.
   As the engine's reaction to "ucinewgame" can take some time the GUI should always send "isready"
   after "ucinewgame" to wait for the engine to finish its operation.
*/

      // Reset the board, hard.
      CreateBoard( argsBoard,
                   argsGeneralMoves ); 
      
      // Set some global parameters for this game.
      //SetInitialParameters();

      // Initialize the hash table.
      InitializeHashTable();

      // We are ready to return.
      return;

   }


   // Set up an FEN position.
   if ( strstr( argstrCommand, "position" ) )
   {

      // Set up the FEN and make the moves if there are any.
      PositionCommand( argstrCommand,
                       argsBoard,
                       argsGeneralMoves );

      return;

/*
* position [fen <fenstring> | startpos ]  moves <move1> .... <movei>
	set up the position described in fenstring on the internal board and
	play the moves on the internal chess board.
	if the game was played  from the start position the string "startpos" will be sent
	Note: no "new" command is needed. However, if this position is from a different game than
	the last position sent to the engine, the GUI should have sent a "ucinewgame" inbetween.
*/

   }

   // Do the go
   if ( strstr( argstrCommand, "go") )
   {

      GoCommand( argstrCommand,
                 argsBoard,
                 argsGeneralMoves );
      return;
/*      
* go
	start calculating on the current position set up with the "position" command.
	There are a number of commands that can follow this command, all will be sent in the same string.
	If one command is not sent its value should be interpreted as it would not influence the search.
	* searchmoves <move1> .... <movei>
		restrict search to this moves only
		Example: After "position startpos" and "go infinite searchmoves e2e4 d2d4"
		the engine should only search the two moves e2e4 and d2d4 in the initial position.
	* ponder
		start searching in pondering mode.
		Do not exit the search in ponder mode, even if it's mate!
		This means that the last move sent in in the position string is the ponder move.
		The engine can do what it wants to do, but after a "ponderhit" command
		it should execute the suggested move to ponder on. This means that the ponder move sent by
		the GUI can be interpreted as a recommendation about which move to ponder. However, if the
		engine decides to ponder on a different move, it should not display any mainlines as they are
		likely to be misinterpreted by the GUI because the GUI expects the engine to ponder
	   on the suggested move.
	* wtime <x>
		white has x msec left on the clock
	* btime <x>
		black has x msec left on the clock
	* winc <x>
		white increment per move in mseconds if x > 0
	* binc <x>
		black increment per move in mseconds if x > 0
	* movestogo <x>
      there are x moves to the next time control,
		this will only be sent if x > 0,
		if you don't get this and get the wtime and btime it's sudden death
	* depth <x>
		search x plies only.
	* nodes <x>
	   search x nodes only,
	* mate <x>
		search for a mate in x moves
	* movetime <x>
		search exactly x mseconds
	* infinite
		search until the "stop" command. Do not exit the search without being told so in this mode!
*/


   }

   // Do the stop
   if (strstr( argstrCommand, "stop" ) )
   {

      // Stop the search.
      SetStopGo( dStop );
      return;

/*
* stop
	stop calculating as soon as possible,
	don't forget the "bestmove" and possibly the "ponder" token when finishing the search
*/      

   }
   
   // Ponderhit
   if ( strstr( argstrCommand, "ponderhit" ) )
   {

      SetStopGo( dGo );
      return;

/*
* ponderhit
	the user has played the expected move. This will be sent if the engine was told to ponder on the same move
	the user has played. The engine should continue searching but switch from pondering to normal search.
*/

   }

   // Quit
   if ( strstr( argstrCommand, "quit" ) )
   {

   	// quit the program as soon as possible
      // This breaks the loop back in main and uses that exit coding.
      SetInterfaceMode( dConsole );       
      return;

   }

   // These commands are for debugging.
   if ( strstr( argstrCommand, "printboard" ) )
   {

      PrintBoard( argsBoard->mBoard );

   }

}

//
//------------------------------------------------------------------
//
// Parse through and act upon a "parse command.
//
void PositionCommand( char * argstrCommand,
                      struct Board * argsBoard,
                      struct GeneralMove * argsGeneralMoves ) 
{

   char * pFENPosition;
   char * pMovePosition;
   char * pStartPosPosition;
   char * pMove;
   char strMove[ 8 ];
   Move vsMoveList[ dNumberOfMoves ];
   int iMoveNumber = 0;

   // Reset the board, but preserve the book or hash table
   // Reset the hash if applicable, but at least
   // reset the hash itself
   if ( GetIsInBook() == dYes )
   {
 
      // If in book, don't wipte the book out.
      SetResetHash( dNo );

   }

   // Reset the board.
   CreateBoard( argsBoard,
                argsGeneralMoves );

   // Make sure the hash is updated in the future if need be.
   SetResetHash( dYes );

   // Add an end of line to the command
   strcat( argstrCommand, "\n" ); // hack

   // See if "fen" is in the string.
   pFENPosition = strstr( argstrCommand,
                          "fen " );
   
   // See if "moves " is in the string.
   pMovePosition = strstr( argstrCommand,
                           "moves " );

   // See if we are starting from the starting position.
   pStartPosPosition = strstr( argstrCommand,
                               "startpos" );

   // If "fen" was found this parse out the string and set up the board.
   if ( pFENPosition )
   {

      // Set up the board according to the string.
      ReadFEN( pFENPosition + 4,
               argsBoard,
               argsGeneralMoves,
               2 );

   }
   else if ( pStartPosPosition )
   {

      // Set up according to the basic FEN.
      if ( GetIsInBook() == dYes )
      {

         // Turn off the hash table reset.
         SetResetHash( dNo );

      }

      // Reset the book.
      ReadFEN( argstrCommand,
               argsBoard,
               argsGeneralMoves,
               1 );

      // Make sure the reset the hash table.
      SetResetHash( dYes );

   }   
   else
   {

      // If "fen" was not found, assume the initial position is required.
      ReadFEN( argstrCommand,
               argsBoard,
               argsGeneralMoves,
               1 );

   }
   
   // If "moves " was found, parse and make the moves.
   if ( pMovePosition )
   {
   
      // Move the end of the word and space "moves "
      pMove = pMovePosition + 6;

      // Loop to the end of the line.
      //while ( strcmp( pMove, "\n" ) )
      while ( strncmp( pMove, "\n", 1 ) )
      {

         // Copy the to and from square
         strncpy( strMove, pMove, 4 );
         pMove = pMove + 4;

         // Terminate the string.
         strncpy( & strMove[ 4 ], "\0", 1 );
         

         // Look for a promotion.
         if ( !( strncmp( pMove, "\n", 1 ) == 0 ) && 
              !( strncmp( pMove, " ", 1 )  == 0 ) )
         {
            
            // Copy the promotion and the piece.
            strMove[ 4 ] = * pMove++;
            //strMove[ 5 ] = * pMove++;

            // Terminate the string.
            strncpy( & strMove[ 5 ], "\0", 1 );

         }

         // Calculate the moves.
         CalculateMoves( vsMoveList, 
                         argsBoard, 
                         argsGeneralMoves );

         // Point an incoming algebraic move to a move in an array
         FindAlgebraicMove( argsBoard,
                            argsGeneralMoves,
                            strMove,
                            vsMoveList,
                            & iMoveNumber );
         
            //gofDebug << "Hash Before Move = " << GetHash() << endl;
            //cout     << "Hash Before Move = " << GetHash() << endl;
            

         // Make the move if we found a legal one.
         if ( iMoveNumber >= 0 && iMoveNumber < 200 )
         {

            MakeMove( vsMoveList,
                      argsBoard,
                      argsGeneralMoves,
                      iMoveNumber );

         }

           // gofDebug << "Hash After Move = " << GetHash() << endl << endl;
            //cout     << "Hash After Move = " << GetHash() << endl << endl;


         // Read the rest of the white space.
         while( !strncmp( pMove, " ", 1 ) )
         {

            pMove++;
      
         }

      }

   }

}
 
//
//--------------------------------------------------------------------------------------------------------------------------------
//
void GoCommand( char * argstrCommand,
                struct Board * argsBoard,
                struct GeneralMove * argsGeneralMoves )
{
   // Debut the inputs.
   assert( argstrCommand >= 0 );
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Delcare some variables.
   char * pParser;
   int iMoveNumber = 0;

   // The input UCI variables.
   int wtime     = 0;
   int btime     = 0;
   int winc      = 0;
   int binc      = 0;
   int movestogo = 0;
   int depth     = 0;
   int nodes     = 0;
   int mate      = 0;
   int movetime  = 0;

   // Find the first command after go
   pParser = strstr( argstrCommand, "go" ) + 2;

   // Debugging.
   assert( pParser >= 0 );

   // Loop over the white space.
   while( !strncmp( pParser, " ", 1 ) )
   {

      // Move the parser forward one space.
      pParser++;
      
   }

   // Get the initial time from the clock.
   SetSearchStartTime();
   SetNodes( 0 );

   // Switch on the potential elements
   if ( strstr( pParser, "movestogo" ) )
   {

      // Move to the next space and extract the depth of the search.
      pParser = strstr( argstrCommand, "movestogo" ) + 9;

      // Convert the time to an integer.
      movestogo = atoi( pParser );

      // Set the moves to go until the next time control.
      SetMovesToGo( movestogo );

   } 
   if ( strstr( argstrCommand, "binc" ) )
   {

      // Move to the next space and extract the black incremental time.
      pParser = strstr( argstrCommand, "binc" ) + 4;

      // Convert the time to an integer.
      binc = atoi( pParser );

      // Set the incremental time
      SetBlackIncrementalTime( binc );

      // Set the depth just in case
      SetSearchDepth( dInfiniteDepth );

   } 
   if ( strstr( argstrCommand, "btime" ) )
   {

      // Move to the next space and extract the black time.
      pParser = strstr( argstrCommand, "btime" ) + 5;

      // Convert the time to an integer.
      btime = atoi( pParser );

      // Set the black time
      SetBlackTime( btime );

      // Set the depth just in case
      SetSearchDepth( dInfiniteDepth );

      // Calculate the time for the move.
      if ( argsBoard->siColorToMove == dBlack )
      {

         // Calculate the search time.
         CalculateTimeForMove( argsBoard );

      }

   } 
   if ( strstr( argstrCommand, "movetime" ) )
   {

      // Move to the next space and extract the move time.
      pParser = strstr( argstrCommand, "movetime" ) + 8;

      // Convert the time to an integer.
      movetime = atoi( pParser );

      // Set the move time
      SetSearchTimeInMiliSeconds( movetime );

      // Set the depth just in case
      SetSearchDepth( dInfiniteDepth );

   } 
   if ( strstr( argstrCommand, "winc" ) )
   {

      // Move to the next space and extract the white incremental time.
      pParser = strstr( argstrCommand, "winc" ) + 4;

      // Convert the time to an integer.
      winc = atoi( pParser );

      // Set the white incremental time
      SetWhiteIncrementalTime( winc );

      // Set the depth just in case
      SetSearchDepth( dInfiniteDepth );

   } 
   if ( strstr( argstrCommand, "wtime" ) )
   {

      // Move to the next space and extract the white time.
      pParser = strstr( argstrCommand, "wtime" ) + 5;

      // Convert the time to an integer.
      wtime = atoi( pParser );

      // Set the white time
      SetWhiteTime( wtime );

      // Set the depth just in case
      SetSearchDepth( dInfiniteDepth );

      // Calculate the time for the move.
      if ( argsBoard->siColorToMove == dWhite )
      {

         // Calculate the search time.
         CalculateTimeForMove( argsBoard );

      }

   } 
   if ( strstr( pParser, "depth" ) )
   {

      // Move to the next space and extract the depth of the search.
      pParser = strstr( argstrCommand, "depth" ) + 5;

      // Loop over the white space.
      while( !strncmp( pParser, " ", 1 ) )
      {

         // Advance the parser one space.
         pParser++;
         
      }

      // Convert the time to an integer.
      depth = atoi( pParser );

      // Set the search depth.
      SetSearchDepth( depth );

      // Set the search time.
      SetSearchTimeInMiliSeconds( dInfiniteTime );

   } 
   if ( strstr( pParser, "infinite" ) )
   {

      // Move to the next space and extract the depth of the search.
      pParser = strstr( argstrCommand, "infinite" ) + 8;

      // Convert the time to an integer.
      depth = atoi( pParser );


      // Step out of book.
      SetUseOpeningBook( dNo ); 

      // Set the search depth.
      SetSearchDepth( dInfiniteDepth );

      // Set the search time.
      SetSearchTimeInMiliSeconds( dInfiniteTime );

   } 
   if ( strstr( pParser, "nodes" ) )
   {

      // Move to the next space and extract the depth of the search.
      pParser = strstr( argstrCommand, "nodes" ) + 5;

      // Convert the time to an integer.
      nodes = atoi( pParser );

      // Set the moves to go until the next time control.
      SetNodes( nodes );

   } 
   if ( strstr( pParser, "mate" ) )
   {

      // Move to the next space and extract the depth of the search.
      pParser = strstr( argstrCommand, "mate" ) + 4;

      // Convert the time to an integer.
      mate = atoi( pParser );

      // Set the moves to go until the next time control.
      SetMate( mate );
      SetSearchDepth( mate + 3 );

   } 
   if ( strstr( pParser, "searchmoves" ) )
   {

      // Move to the next space and extract the depth of the search.
      pParser = strstr( argstrCommand, "searchmoves" ) + 11;

   } 
   if ( strstr( pParser, "ponder" ) )
   {

      // Move to the next space and extract the depth of the search.
      pParser = strstr( argstrCommand, "searchmoves" ) + 11;

      // Set pondering on to preserve the Multi PV
      SetPonder( dYes ); // dYes dNo


      // Set the search to go
      SetStopGo( dGo );

   } 

   // Look for things that could be wrong.
   if ( GetSearchTimeInMiliSeconds() <= 0 )
   {

      // Set the search time
      SetSearchTimeInMiliSeconds( 1000 );

   }
   if ( GetSearchDepth() < 0 )
   {
  
      // Set the search depth.
      SetSearchDepth( 3 );

   }
   if ( argsBoard->siColorToMove == dWhite )
   {

      SetComputerColor( dComputerWhite );  // dComputerWhite dComputerBlack

   }
   else
   {

      SetComputerColor( dComputerBlack );

   }

   // Get a computer move.
   // The move will be sent in the below routine.
   ComputerMove( argsBoard,
                 argsGeneralMoves );

}


 