#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
// #include <stdlib.h>
#include "Functions.h"
#include "Structures.h"
#include <fstream>
// #include <omp.h>
#include "Definitions.h"
#include <atomic>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <math.h>
#include <mutex>
#include <string.h>
#include <string>
#include <thread>
#include <vector>
#include <queue>
#include <condition_variable>

using namespace std;

int giTotalCount;
int giMarginalCount;
int giCutCount;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Define global variables for keeping track of the number of nodes counted.
// Global variables suck, but are awsome for allowing for Deep Violet.
// access to the table:
// Note that the scope for the globe variables is only this file.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// HashTable gsOpeningBook;
OpeningBook gsOpeningBook;

// A global file for writing out a checked book.  Needed because of the
// recursive nature of the calculation.
ofstream gofCheckedBook;

// A global file for debug output for the interface and the book.
ofstream gofDebugBook;

// Synchronization for parallel processing
std::mutex                            coutMutex;
std::atomic<int>                      gProcessedFiles( 0 );
std::atomic<int>                      gTotalFiles( 0 );
std::chrono::steady_clock::time_point gStartTime;

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

//
//
//---------------------------------------------------------------------
//
//

//
//----------------// Globals for debug
std::atomic<long long> g_BookUpdates( 0 );
std::atomic<long long> g_MergeOps( 0 );
std::atomic<int>       g_PendingTasks( 0 ); // Tracking active games + markers
std::atomic<long long> g_TotalGamesAnalyzed( 0 );
const int              dTaskMerge = -999;

//
//
//---------------------------------------------------------------------
//
//
void InitializeOpeningBook( OpeningBook *book )
{
   // This function is used to initialize the hash table.  The function is stored
   // in the search file because the hash table is a global variable and is
   // needed the most in the search routines.
   //

   // Allocate the memory.
   book->bbOpeningBookSize = Power( 2, dNumberOfBitsInOpeningBook );
   book->mbbHash =
       (BitBoard *)malloc( book->bbOpeningBookSize * sizeof( BitBoard ) );
   book->mbbHashTable =
       (BitBoard *)malloc( book->bbOpeningBookSize * sizeof( BitBoard ) );

   // Loop over the hash table entries and set them to zero.
   // #pragma omp parallel for schedule( dynamic, 1 )
   // Loop over the hash table entries and set them to zero.
   memset( book->mbbHashTable, 0, book->bbOpeningBookSize * sizeof( BitBoard ) );
   memset( book->mbbHash, 0, book->bbOpeningBookSize * sizeof( BitBoard ) );

   // Set up the mask for extracting the index.
   book->bbMaskIndex = 0;
   for ( int iBitIndex = 0; iBitIndex < dNumberOfBitsInOpeningBook; iBitIndex++ )
   {

      book->bbMaskIndex = SetBitToOne( book->bbMaskIndex, iBitIndex );
   }

   // Get ready to count the positions in the book.
   book->bbNumberOfPositionsInBook = 0;

   // Set the initial hash for later reference
   // book->bbHashInitial = book->bbHash;
}

void InitializeOpeningBook()
{
   InitializeOpeningBook( &gsOpeningBook );
}

//
//
//---------------------------------------------------------------------
//
//
void DestroyOpeningBook( OpeningBook *book )
{
   // This function releases the memory taken by the hash table.

   free( book->mbbHash );
   free( book->mbbHashTable );
}

void DestroyOpeningBook()
{
   DestroyOpeningBook( &gsOpeningBook );
}

// Define some book sets.
void SetBookElement( OpeningBook *book, BitBoard bbKey, BitBoard bbElement )
{
   book->mbbHashTable[ bbKey ] = bbElement;
}
void SetBookElementHash( OpeningBook *book, BitBoard bbKey, BitBoard bbElement )
{
   book->mbbHash[ bbKey ] = bbElement;
}

// Wrappers
void SetBookElement( BitBoard bbKey, BitBoard bbElement )
{
   SetBookElement( &gsOpeningBook, bbKey, bbElement );
}
void SetBookElementHash( BitBoard bbKey, BitBoard bbElement )
{
   SetBookElementHash( &gsOpeningBook, bbKey, bbElement );
}

// Define some input and some output variables.
BitBoard GetBookElement( OpeningBook *book, BitBoard bbKey )
{

   assert( bbKey <= book->bbOpeningBookSize );

   return book->mbbHashTable[ bbKey ];
}
BitBoard GetBookElementHash( OpeningBook *book, BitBoard bbKey )
{

   assert( bbKey <= book->bbOpeningBookSize );

   return book->mbbHash[ bbKey ];
}

// Wrappers
BitBoard GetBookElement( BitBoard bbKey )
{
   // Debug
   // cout << "GetBookElement(" << bbKey << ") = " << GetBookElement(&gsOpeningBook, bbKey) << endl;
   return GetBookElement( &gsOpeningBook, bbKey );
}
BitBoard GetBookElementHash( BitBoard bbKey )
{
   // Debug
   // cout << "GetBookElementHash(" << bbKey << ") = " << GetBookElementHash(&gsOpeningBook, bbKey) << endl;
   return GetBookElementHash( &gsOpeningBook, bbKey );
}

extern std::atomic<long long> g_MergeOps;

void MergeOpeningBooks( OpeningBook *dest, OpeningBook *src, GeneralMove *gm )
{
   long long mergeCount = 0;
   for ( BitBoard i = 0; i < src->bbOpeningBookSize; i++ )
   {
      BitBoard srcElement = src->mbbHashTable[ i ];
      if ( srcElement == 0 )
         continue;

      mergeCount++;
      g_MergeOps++;

      // Debug first few merges
      // if (g_MergeOps < 10) {
      //    cout << "Merge Ops: Found src element at index " << i << endl;
      // }

      BitBoard srcHash  = src->mbbHash[ i ];
      BitBoard destHash = dest->mbbHash[ i ];

      // printf("DEBUG: Merge Collision Check Index=%llu SrcHash=%llu DestHash=%llu\n", i, srcHash, destHash);

      if ( destHash == 0 )
      {
         // New entry
         dest->mbbHash[ i ]      = srcHash;
         dest->mbbHashTable[ i ] = srcElement;
         // printf("DEBUG: Merge NEW Entry Index=%llu Hash=%llu Elem=%llu\n", i, srcHash, srcElement);
      }
      else if ( destHash == srcHash )
      {
         // Same position, merge stats
         BitBoard w1 = ( srcElement & gm->bbWhiteScore ) >> gm->iWhiteScoreShift;
         BitBoard b1 = ( srcElement & gm->bbBlackScore ) >> gm->iBlackScoreShift;
         BitBoard d1 = ( srcElement & gm->bbDrawScore ) >> gm->iDrawScoreShift;

         BitBoard destElement = dest->mbbHashTable[ i ];
         BitBoard w2          = ( destElement & gm->bbWhiteScore ) >> gm->iWhiteScoreShift;
         BitBoard b2          = ( destElement & gm->bbBlackScore ) >> gm->iBlackScoreShift;
         BitBoard d2          = ( destElement & gm->bbDrawScore ) >> gm->iDrawScoreShift;

         BitBoard w = w1 + w2;
         BitBoard b = b1 + b2;
         BitBoard d = d1 + d2;

         // Reconstruct element
         BitBoard newElement = 0;
         newElement |= ( w << gm->iWhiteScoreShift );
         newElement |= ( b << gm->iBlackScoreShift );
         newElement |= ( d << gm->iDrawScoreShift );

         dest->mbbHashTable[ i ] = newElement;
         // printf("DEBUG: Merge UPDATE Entry Index=%llu Hash=%llu OldElem=%llu NewElem=%llu\n", i, srcHash, destElement, newElement);
      }
   }
}

// Defines
void UpdateOpeningBook( OpeningBook *book, struct Board *argsBoard,
                        struct GeneralMove *argsGeneralMoves, int iGameResult );
void ReadAPGNMove( int *iGameResult, int *iFlagLine, int *iFlagBook,
                   int *iIsGoodGame, int *iGoodWhiteELO, int *iGoodBlackELO,
                   char *strMove, struct Board *argsBoard,
                   struct GeneralMove *argsGeneralMoves, int iPlyCount,
                   int iPlyIndex );

#include <ctype.h>

// Helper to identify file/rank char
bool isSANFile( char c )
{
   return c >= 'a' && c <= 'h';
}
bool isSANRank( char c )
{
   return c >= '1' && c <= '8';
}

int GetMoveFromSAN( struct Board *argsBoard, struct GeneralMove *argsGeneralMoves, struct Move *vsMoveList, string strMove )
{

   // 1. Clean string (remove +, #, etc)
   string san = strMove;
   // Remove check/mate/annotation chars
   while ( !san.empty() && ( san.back() == '+' || san.back() == '#' || san.back() == '!' || san.back() == '?' ) )
   {
      san.pop_back();
   }

   // 2. Identify castling
   if ( san == "O-O" || san == "0-0" )
   {
      // Find Short Castle Move
      for ( int i = 0; i < argsBoard->siNumberOfMoves; ++i )
      {
         if ( vsMoveList[ i ].iMoveType & dWhiteKingSideCastle )
         {
            return i;
         }
         if ( vsMoveList[ i ].iMoveType & dBlackKingSideCastle )
         {
            return i;
         }
         // Also check standard Castle definition if flags are different
         if ( vsMoveList[ i ].iMoveType == dCastle )
         {
            if ( vsMoveList[ i ].iFromSquare == dE1 && vsMoveList[ i ].iToSquare == dG1 )
            {
               return i;
            }
            if ( vsMoveList[ i ].iFromSquare == dE8 && vsMoveList[ i ].iToSquare == dG8 )
            {
               return i;
            }
         }
      }
      return -1;
   }
   if ( san == "O-O-O" || san == "0-0-0" )
   {
      // Long Castle
      for ( int i = 0; i < argsBoard->siNumberOfMoves; ++i )
      {
         if ( vsMoveList[ i ].iMoveType & dWhiteQueenSideCastle )
         {
            return i;
         }
         if ( vsMoveList[ i ].iMoveType & dBlackQueenSideCastle )
         {
            return i;
         }
         if ( vsMoveList[ i ].iMoveType == dCastle )
         {
            if ( vsMoveList[ i ].iFromSquare == dE1 && vsMoveList[ i ].iToSquare == dC1 )
            {
               return i;
            }
            if ( vsMoveList[ i ].iFromSquare == dE8 && vsMoveList[ i ].iToSquare == dC8 )
            {
               return i;
            }
         }
      }
      return -1;
   }

   // 3. Parse Piece and Destination
   int pieceType = dWhitePawn; // Default Pawn (if no letter)
   if ( argsBoard->siColorToMove == dBlack )
      pieceType = dBlackPawn;

   int  strIdx    = 0;
   char firstChar = san[ 0 ];

   // Check if first char is a piece letter (and uppercase)
   if ( isupper( firstChar ) )
   {
      if ( firstChar == 'N' )
         pieceType = ( argsBoard->siColorToMove == dWhite ) ? dWhiteKnight : dBlackKnight;
      else if ( firstChar == 'B' )
         pieceType = ( argsBoard->siColorToMove == dWhite ) ? dWhiteBishop : dBlackBishop;
      else if ( firstChar == 'R' )
         pieceType = ( argsBoard->siColorToMove == dWhite ) ? dWhiteRook : dBlackRook;
      else if ( firstChar == 'Q' )
         pieceType = ( argsBoard->siColorToMove == dWhite ) ? dWhiteQueen : dBlackQueen;
      else if ( firstChar == 'K' )
         pieceType = ( argsBoard->siColorToMove == dWhite ) ? dWhiteKing : dBlackKing;
      strIdx++;
   }

   // Parse Destination (last 2 chars usually, unless promotion)
   // Handle promotion: e8=Q
   char   promoteChar = 0;
   size_t eqPos       = san.find( '=' );
   if ( eqPos != string::npos )
   {
      promoteChar = san[ eqPos + 1 ]; // e.g. Q
      san         = san.substr( 0, eqPos );
   }

   // Destination is the last 2 format chars of the remaining string
   // Example: Nbd7 -> d7. Naxb4 -> b4.
   // We need to walk backwards from end of san checking for rank/file

   int destFile = -1;
   int destRank = -1;

   int len = san.length();
   if ( len < 2 )
      return -1;

   // The last char should be rank, char before file
   // Check ranges safely
   if ( isSANRank( san[ len - 1 ] ) && isSANFile( san[ len - 2 ] ) )
   {
      destRank = san[ len - 1 ] - '1'; // '1'->0
      destFile = san[ len - 2 ] - 'a'; // 'a'->0
   }
   else
   {
      return -1; // Invalid format
   }

   // Violet: Square = Row + Col*8. Row=File, Col=Rank.
   // Square = File + Rank*8
   int destSquare = destFile + destRank * 8;

   // 4. Checking Moves
   int bestMoveIdx = -1;
   int matchCount  = 0;

   for ( int i = 0; i < argsBoard->siNumberOfMoves; ++i )
   {
      // Filter by Piece Type
      // Note: Violet Moves stores the piece type.
      // Special case: Promotion. Violet might store 'Pawn' but 'Promote' type,
      // OR it might store 'Queen' if it pre-generates.
      // Let's assume Violet stores the moving piece (Pawn).
      // So if we found 'Pawn' but it's a promotion, we still check Pawn.
      // Wait, if input is "N..." pieceType is Knight.

      if ( vsMoveList[ i ].iPiece != pieceType )
         continue;

      // Filter by Destination
      if ( vsMoveList[ i ].iToSquare != destSquare )
      {
         // if (vsMoveList[i].iPiece == pieceType)
         //    cout << "  Skip Dest: MoveTo=" << vsMoveList[i].iToSquare << " Wanted=" << destSquare << endl;
         continue;
      }

      // cout << "DEBUG SAN: Found Candidate Index=" << i << endl;

      // Filter by Promotion
      if ( promoteChar )
      {
         if ( !( vsMoveList[ i ].iMoveType & dPromote ) )
            continue;
         // Disambiguate promotion pieces if Violet supports multiple
         // Definitions.h doesn't seem to have distinct promotion move types per piece.
         // It just has dPromote.
         // Assume Queen promotion is default or handled elsewhere?
         // IF Violet generates 4 moves for promotion, we need to distinguish them.
         // Usually engines generate 4 moves with different 'special' flags or score.
         // Without knowing how Violet stores the promoted piece choice, we might pick the wrong one.
         // Re-reading definitions: dCaptureAndPromote 2048.
         // No dPromoteQueen etc.
         // Let's trust that the engine generates them in standard order (Q,R,B,N) or we default to first (usually Queen).
         // Ideally we check.
         // For now, accepting ANY promotion move matching source/dest.
      }

      // Disambiguation
      // Remaining string parts between Piece and Dest.
      // ex: "Nbd7" -> "b"
      // ex: "N1d7" -> "1"
      // ex: "Nxd4" -> "x" (ignore x)

      int src     = vsMoveList[ i ].iFromSquare;
      int srcFile = dRow( src ); // 0-7 (File)
      int srcRank = dCol( src ); // 0-7 (Rank)

      // Disambiguation check
      bool match = true;

      // If we skipped piece char coverage (strIdx)
      // Verify middle chars
      for ( int k = strIdx; k < len - 2; ++k )
      {
         char c = san[ k ];
         if ( c == 'x' )
            continue;
         if ( isSANFile( c ) )
         {
            if ( ( c - 'a' ) != srcFile )
               match = false;
         }
         else if ( isSANRank( c ) )
         {
            if ( ( c - '1' ) != srcRank )
               match = false;
         }
      }

      if ( match )
      {
         bestMoveIdx = i;
         matchCount++;
      }
   }

   if ( matchCount >= 1 )
      return bestMoveIdx;
   return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Some analysis elements
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// GameTask structure
struct GameTask
{
   vector<string> moves;
   int            whiteElo = 0;
   int            blackElo = 0;
   int            result   = 0; // dWhiteWin, dBlackWin, dDraw, dUnknown
};

// Thread-safe Queue
template <typename T>
class ThreadSafeQueue
{
 private:
   std::queue<T>           queue;
   std::mutex              mutex;
   std::condition_variable cond_empty;
   std::condition_variable cond_full;
   bool                    finished = false;
   size_t                  maxSize;

 public:
   ThreadSafeQueue( size_t size = 50000 ) : maxSize( size ) {}

   void push( T item )
   {
      std::unique_lock<std::mutex> lock( mutex );
      // Block if full
      cond_full.wait( lock, [ this ]()
                      { return queue.size() < maxSize; } );
      queue.push( item );
      cond_empty.notify_one();
   }

   bool pop( T &item )
   {
      std::unique_lock<std::mutex> lock( mutex );
      // Block if empty, unless finished
      cond_empty.wait( lock, [ this ]()
                       { return !queue.empty() || finished; } );

      if ( queue.empty() )
         return false;

      item = queue.front();
      queue.pop();
      cond_full.notify_one(); // Notify producer that space is available
      return true;
   }

   void setFinished()
   {
      std::lock_guard<std::mutex> lock( mutex );
      finished = true;
      cond_empty.notify_all();
   }

   void reset()
   {
      std::lock_guard<std::mutex> lock( mutex );
      finished = false;
      // Queue should ideally be empty when resetting, or we accept leftovers
   }

   bool empty()
   {
      std::lock_guard<std::mutex> lock( mutex );
      return queue.empty();
   }

   size_t size()
   {
      std::lock_guard<std::mutex> lock( mutex );
      return queue.size();
   }
};

void ResetOpeningBook( OpeningBook *book )
{
   // Zero out the hash table without freeing memory
   // #pragma omp parallel for schedule( dynamic, 1 )
   // Zero out the hash table without freeing memory
   memset( book->mbbHashTable, 0, book->bbOpeningBookSize * sizeof( BitBoard ) );
   memset( book->mbbHash, 0, book->bbOpeningBookSize * sizeof( BitBoard ) );

   book->bbMaskIndex = 0;
   for ( int iBitIndex = 0; iBitIndex < dNumberOfBitsInOpeningBook; iBitIndex++ )
   {
      book->bbMaskIndex = SetBitToOne( book->bbMaskIndex, iBitIndex );
   }
   book->bbNumberOfPositionsInBook = 0;
}

void ProcessGameTask( const GameTask &task, struct Board *argsBoard, struct GeneralMove *argsGeneralMoves, OpeningBook *localBook, int iPlyIndexTarget )
{
   // Reset Board
   CreateBoard( argsBoard, argsGeneralMoves );

   // Check ELO
   if ( GetLimitELO() )
   {
      if ( ( task.whiteElo > 0 && task.whiteElo < 2500 ) || ( task.blackElo > 0 && task.blackElo < 2500 ) )
         return;
   }

   Move moveList[ dNumberOfMoves ];
   Move bestMove;

   for ( size_t i = 0; i < task.moves.size(); ++i )
   {
      if ( (int)i > iPlyIndexTarget )
         break;

      // Update Book for CURRENT position (before move)
      UpdateOpeningBook( localBook, argsBoard, argsGeneralMoves, task.result );

      CalculateMoves( moveList, argsBoard, argsGeneralMoves );

      // Use clean string from task (assuming already cleaned or robust parser)
      int iMoveIndex = GetMoveFromSAN( argsBoard, argsGeneralMoves, moveList, (char *)task.moves[ i ].c_str() );

      // Fallback: The provided moves might have "1." prefix if not cleaned?
      // During my debug, I injected "e4".
      // The Book Creator uses `cleanMove`.
      // I should do minimal cleaning here just in case?
      // Actually `ProcessGameTask` is called by `Reader` which pushes raw tokens or cleaned?
      // Step 602 added cleaning in `ProcessGameTask` loop.
      // I will keep the cleaning logic if I can, but GetMoveFromSAN is robust enough if token is just "e4".

      if ( iMoveIndex == -1 )
      {
         // Try cleaning if failed (simplified cleaning)
         string clean = task.moves[ i ];
         size_t dot   = clean.find( '.' );
         if ( dot != string::npos )
            clean = clean.substr( dot + 1 );
         // trim
         clean.erase( 0, clean.find_first_not_of( " \t" ) );
         iMoveIndex = GetMoveFromSAN( argsBoard, argsGeneralMoves, moveList, (char *)clean.c_str() );
      }

      if ( iMoveIndex != -1 )
      {
         MakeMove( moveList, argsBoard, argsGeneralMoves, iMoveIndex );
      }
      else
      {
         // Invalid move, stop processing this game
         break;
      }
   }

   // Update for the final position reached
   UpdateOpeningBook( localBook, argsBoard, argsGeneralMoves, task.result );
}

void OpeningBookAnalysis( struct Board       *mainArgsBoard,
                          struct GeneralMove *mainArgsGeneralMoves )
{

   assert( mainArgsBoard >= 0 );
   assert( mainArgsGeneralMoves >= 0 );

   char strBookName[ 256 ];
   strcpy( strBookName, "C:\\VioletTools\\Book.txt" );

   // 1. Discovery
   vector<string> files;
   // Check if directory exists
   if ( !std::filesystem::exists( "Lichess Elite Database" ) )
   {
      cout << "Error: Directory 'Lichess Elite Database' not found." << endl;
      return;
   }

   for ( const auto &entry : std::filesystem::directory_iterator( "Lichess Elite Database" ) )
   {
      if ( entry.path().extension() == ".pgn" )
      {
         files.push_back( entry.path().string() );
      }
   }

   gTotalFiles     = files.size();
   gProcessedFiles = 0;
   gStartTime      = std::chrono::steady_clock::now();

   int numThreads = std::thread::hardware_concurrency();
   if ( numThreads == 0 )
      numThreads = 4;

   // Ensure we use all cores (minus one for main thread maybe, or just use all)
   cout << "Starting Opening Book Analysis with " << numThreads
        << " threads." << endl;
   cout << "Found " << gTotalFiles << " files to process." << endl;

   ThreadSafeQueue<GameTask> workQueue( 50000 ); // Buffer 50k games
   vector<thread>            threads;

   // Start Persistent Workers
   for ( int t = 0; t < numThreads; ++t )
   {
      threads.push_back( thread( [ &workQueue, t ]() {             // Capture t for ID
         struct Board       *argsBoard        = new Board();       // Local board
         struct GeneralMove *argsGeneralMoves = new GeneralMove(); // Local moves
         OpeningBook        *localBook        = new OpeningBook();
         InitializeOpeningBook( localBook );

         GenerateGeneralMove( argsGeneralMoves );

         GameTask task;
         // Loop until queue is finished and empty
         while ( workQueue.pop( task ) )
         {
            if ( task.result == dTaskMerge )
            {
               // Synchronization point: Merge and Reset
               {
                  // Lock the mutex to ensure that only one thread is merging at a time.
                  extern std::mutex           gBookMergeMutex;
                  std::lock_guard<std::mutex> lock( gBookMergeMutex );
                  MergeOpeningBooks( &gsOpeningBook, localBook, argsGeneralMoves );
               }

               ResetOpeningBook( localBook );
            }
            else
            {
               ProcessGameTask( task, argsBoard, argsGeneralMoves, localBook, dMaxBookPly );
               long long currentTotal = ++g_TotalGamesAnalyzed;
               if ( currentTotal % 1000 == 0 )
               {
                  extern std::mutex coutMutex;

                  auto                          now         = std::chrono::steady_clock::now();
                  std::chrono::duration<double> elapsed     = now - gStartTime;
                  double                        timePerGame = elapsed.count() / (double)currentTotal;

                  std::lock_guard<std::mutex> lock( coutMutex );
                  cout << "Analyzed " << currentTotal << " games. Time per game: " << std::fixed << std::setprecision( 6 ) << timePerGame << " seconds." << endl;
               }
            }
            g_PendingTasks--; // Task complete
         }

         // Final Merge (if any leftovers)
         if ( localBook->bbNumberOfPositionsInBook > 0 )
         {
            {
               extern std::mutex           gBookMergeMutex;
               std::lock_guard<std::mutex> lock( gBookMergeMutex );
               MergeOpeningBooks( &gsOpeningBook, localBook, argsGeneralMoves );
            }
         }

         DestroyOpeningBook( localBook );
         delete localBook;
         delete argsBoard;
         delete argsGeneralMoves;
      } ) );
   }

   // Main Thread: Producer (File Reader)
   long long totalGamesFound  = 0;
   long long g_LastPruneCount = 0; // Track when we last pruned

   // int iFileLimit = 0;
   for ( const auto &filename : files )
   {
      // if ( ++iFileLimit > 5 )
      //   break; // Limit to 5 files for testing

      gProcessedFiles++;

      {
         std::lock_guard<std::mutex> lock( coutMutex );
         cout << "Reading file " << gProcessedFiles << " of " << gTotalFiles << ": " << filename
              << " (Queue: " << workQueue.size() << ")" << endl;
      }

      ifstream ifBook( filename );
      if ( ifBook.fail() )
         continue;

      string   token;
      GameTask currentTask;
      bool     inTag = false;

      int  variationDepth = 0;     // Track parenthesis depth for variations
      bool inComment      = false; // Track curly brace comments

      while ( ifBook >> token )
      {
         if ( token.empty() )
            continue;

         // Debuging first few tokens of first file
         static int debugTokens = 0;
         if ( gProcessedFiles == 1 && debugTokens < 20 )
         {
            // cout << "Token[" << debugTokens << "]: " << token << endl;
            debugTokens++;
         }

         // Handle 'old style' comments ; to end of line?
         // Minimal PGN parser usually primarily cares about {} and ()

         // Check for start of tag
         if ( !inComment && variationDepth == 0 && token.front() == '[' )
         {
            inTag = true;
            // Check for new game (Event) to push previous game
            if ( token.find( "Event" ) != string::npos )
            {
               if ( !currentTask.moves.empty() )
               {
                  g_PendingTasks++;
                  workQueue.push( currentTask );
                  totalGamesFound++;
                  currentTask = GameTask(); // Reset
               }
            }
            // Check for Result tag
            if ( token.find( "Result" ) != string::npos )
            {
               // The next token (or rest of this line) contains the result string.
               // This is a simple tokenizer, so "Result" might be separate from "1-0".
               // PGN: [Result "1-0"]
               // Tokens: [Result, "1-0"]
            }
         }

         if ( inTag )
         {
            // Try to parse result from tag if we are inside a Result tag line
            // Simple heuristic: If token contains 1-0, 0-1, 1/2
            if ( token.find( "1-0" ) != string::npos )
               currentTask.result = dWhiteWin;
            else if ( token.find( "0-1" ) != string::npos )
               currentTask.result = dBlackWin;
            else if ( token.find( "1/2" ) != string::npos )
               currentTask.result = dDraw;

            if ( token.back() == ']' )
               inTag = false;
            continue;
         }

         // Use sanitization from before for robust move detection
         if ( token.find( '{' ) != string::npos )
         {
            inComment = true;
         }
         if ( inComment )
         {
            if ( token.find( '}' ) != string::npos )
            {
               inComment = false;
            }
            continue; // Skip comment token
         }

         // Variation Handling
         for ( char c : token )
         {
            if ( c == '(' )
               variationDepth++;
            else if ( c == ')' )
               variationDepth--;
         }
         if ( variationDepth > 0 || token.find( ')' ) != string::npos )
         {
            continue;
         }

         // NAGs
         if ( token[ 0 ] == '$' )
            continue;

         // Check for result (Inline) which ends the game immediately
         int iGameResult = -1;
         if ( token == "1-0" )
            iGameResult = dWhiteWin;
         else if ( token == "0-1" )
            iGameResult = dBlackWin;
         else if ( token == "1/2-1/2" )
            iGameResult = dDraw;
         else if ( token == "*" )
            iGameResult = dUnknown;

         if ( iGameResult != -1 )
         {
            // If result tag already set a result, this confirms it. Use inline if valid.
            if ( iGameResult != dUnknown )
               currentTask.result = iGameResult;

            // End of game
            if ( !currentTask.moves.empty() )
            {
               g_PendingTasks++;
               workQueue.push( currentTask );
               totalGamesFound++;
            }
            currentTask = GameTask(); // Reset
         }
         else
         {
            // Skip move numbers (e.g. "1.", "25.")
            if ( token.back() == '.' )
               continue;

            // Move
            currentTask.moves.push_back( token );
         }
      }
      ifBook.close();

      // End of File: Synchronize
      // Push Markers
      for ( int i = 0; i < numThreads; ++i )
      {
         GameTask marker;
         marker.result = dTaskMerge;
         g_PendingTasks++;
         workQueue.push( marker );
      }

      // Wait for all tasks to finish
      // Note: We use a simple spin-wait here. Could use condition variable but
      // g_PendingTasks changes rapidly, so spin/sleep is fine.
      while ( g_PendingTasks > 0 )
      {
         std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
      }

      // Determine if we should prune
      int bPrune = 0;
      // if ((g_TotalGamesAnalyzed - g_LastPruneCount) >= 10000) {
      //     bPrune = 1;
      //     g_LastPruneCount = g_TotalGamesAnalyzed;
      // }

      // Write Book
      cout << "File complete. Total Games Analyzed: " << g_TotalGamesAnalyzed << ". Writing Book..." << endl;
      WriteOutOpeningBook( strBookName, dMaxBookPly, bPrune );
      // break; // DEBUG: Stop after 1 file to quickly regenerate a valid book for testing.
   }

   cout << "All files read. Total games pushed: " << totalGamesFound << ". Waiting for workers..." << endl;

   // Signal workers that no more work is coming
   workQueue.setFinished();

   // Join threads
   for ( auto &t : threads )
   {
      if ( t.joinable() )
         t.join();
   }
   threads.clear();

   cout << "All workers joined. Writing final book..." << endl;
   WriteOutOpeningBook( strBookName, dMaxBookPly, 0 ); // Always prune at the end
   cout << "Opening Book Creation Complete." << endl;
}

void ReadAPGNMove( int *iGameResult, int *iFlagLine, int *iFlagBook,
                   int *iIsGoodGame, int *iGoodWhiteELO, int *iGoodBlackELO,
                   char *strMove, struct Board *argsBoard,
                   struct GeneralMove *argsGeneralMoves, int iPlyCount,
                   int iPlyIndex )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // This routine reads a move, either black or white from a file.
   Move vsMoveList[ dNumberOfMoves ];
   int  iMoveNumber;

   // Look to see if we are at the end of a game.
   // Look for an ending.
   if ( LookForGameResult( strMove, iGameResult ) == 1 )
   {

      // Reset the ELO flags.
      *iGoodWhiteELO = 0;
      *iGoodBlackELO = 0;

      *iFlagLine = 0;
      return;
   }
   // Look for an EOF or more precisely an empty strMove.
   if ( LookForGameResult( strMove, iGameResult ) == -1 )
   {

      // Reset the ELO flags.
      *iGoodWhiteELO = 0;
      *iGoodBlackELO = 0;

      *iFlagLine = 0;
      *iFlagBook = 0;
      return;
   }

   // Do the hash thing and up date the board if we are at a ply of interest.
   // if ( iIsGoodGame )
   // PrintBoard( argsBoard->mBoard );
   if ( iIsGoodGame && ( iPlyCount <= iPlyIndex + 1 ) )
   {

      // Calculate the moves.
      CalculateMoves( vsMoveList, argsBoard, argsGeneralMoves );

      // Get the matching move number.
      iMoveNumber =
          GetMoveNumberFast( argsBoard, argsGeneralMoves, vsMoveList, strMove );

      // Look for a bad move in the data.
      if ( iMoveNumber == -1 )
      {

         /*
         PrintBoard( argsBoard->mBoard );
         PrintFEN( argsBoard,
                   argsGeneralMoves );
         */
         *iIsGoodGame = 0;
      }

      if ( *iIsGoodGame )
      {

         // If the move was found make it move on with life.
         MakeMove( vsMoveList, argsBoard, argsGeneralMoves, iMoveNumber );

         // Reset the current ply depth.
         argsBoard->iNumberOfPlys = -1;
      }
   }
}

int LookForGameResult( char *argstrMove, int *argiGameResult )
{

   // Debug the inputs.
   assert( argstrMove >= 0 );

   // Declare some variables.
   int iResultFound = 0;

   if ( strcmp( argstrMove, "1-0" ) == 0 )
   {

      *argiGameResult = dWhiteWin;
      iResultFound    = 1;
   }
   if ( strcmp( argstrMove, "0-1" ) == 0 )
   {

      *argiGameResult = dBlackWin;
      iResultFound    = 1;
   }
   if ( strcmp( argstrMove, "1/2-1/2" ) == 0 )
   {

      *argiGameResult = dDraw;
      iResultFound    = 1;
   }
   if ( strcmp( argstrMove, "*" ) == 0 )
   {

      *argiGameResult = dUnknown;
      iResultFound    = 1;
   }
   if ( strcmp( argstrMove, "" ) == 0 )
   {

      iResultFound = -1;
   }

   assert( iResultFound >= -1 );
   assert( iResultFound <= 1 );

   // Return whether or not we found a result
   return iResultFound;
}

//
//
//---------------------------------------------------------------------
//
//
void PrintOpeningBookMoveStatistics( struct Board       *argsBoard,
                                     struct GeneralMove *argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Print the statistics from the open book for the moves from a given
   // position.
   int      iMoveCount;
   Move     vsMoveList[ dNumberOfMoves ];
   int      viSortOrder[ dNumberOfMoves ];
   int      viPopularity[ dNumberOfMoves ];
   BitBoard vbbWhiteWins[ dNumberOfMoves ];
   BitBoard vbbBlackWins[ dNumberOfMoves ];
   BitBoard vbbDraws[ dNumberOfMoves ];
   BitBoard bbWhiteWins = 0;
   BitBoard bbBlackWins = 0;
   BitBoard bbDraws     = 0;
   char     strMove[ 64 ];
   int      iNumberOfChars;
   int      iCharCount;
   int      iNumberOfMoves;
   double   dPercentWhiteWins = 0;
   double   dPercentBlackWins = 0;
   double   dPercentDraws     = 0;
   BitBoard bbTotalGames      = 0;

   // Calculate the moves for this position.
   CalculateMoves( vsMoveList, argsBoard, argsGeneralMoves );

   iNumberOfMoves = argsBoard->siNumberOfMoves;

   cout << endl;

   for ( iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++ )
   {

      // Use a temporary board to probe stats so we don't corrupt the main board
      // Allocate on heap to avoid stack overflow (Board struct is ~1MB)
      Board *tempBoard = new Board();
      *tempBoard       = *argsBoard;

      // Make the move on the temporary board
      MakeMove( vsMoveList, tempBoard, argsGeneralMoves, iMoveCount );

      // Get the statistics.
      ExtractOpeningBookStats( &gsOpeningBook, bbWhiteWins, bbBlackWins, bbDraws,
                               tempBoard, argsGeneralMoves );

      // Store the stats for this move
      vbbWhiteWins[ iMoveCount ] = bbWhiteWins;
      vbbBlackWins[ iMoveCount ] = bbBlackWins;
      vbbDraws[ iMoveCount ]     = bbDraws;

      // Sum the total games.
      bbTotalGames += bbWhiteWins;
      bbTotalGames += bbBlackWins;
      bbTotalGames += bbDraws;

      // Clean up temporary board
      delete tempBoard;

      // Collect the popularity.
      viPopularity[ iMoveCount ] = (int)( bbWhiteWins + bbBlackWins + bbDraws );
      viSortOrder[ iMoveCount ]  = iMoveCount;
   }

   // Sort on the popularity
   int iSortFlag = 1;
   while ( iSortFlag )
   {

      // Set the default to bail.
      iSortFlag = 0;

      // Use a cocktail sort and to from top to bottom
      for ( int iMoveIndex = 0; iMoveIndex < iNumberOfMoves - 1; iMoveIndex++ )
      {

         if ( viPopularity[ iMoveIndex + 1 ] > viPopularity[ iMoveIndex ] )
         {

            int iDummyScore                = viPopularity[ iMoveIndex ];
            viPopularity[ iMoveIndex ]     = viPopularity[ iMoveIndex + 1 ];
            viPopularity[ iMoveIndex + 1 ] = iDummyScore;

            int iDummyPosition            = viSortOrder[ iMoveIndex ];
            viSortOrder[ iMoveIndex ]     = viSortOrder[ iMoveIndex + 1 ];
            viSortOrder[ iMoveIndex + 1 ] = iDummyPosition;

            iSortFlag = 1;
         }
      }
   }

   // This loop is for printing out.
   for ( iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++ )
   {

      // Only show non-zero popularities.
      // if ( viPopularity[ iMoveCount ] > 0 )
      if ( true )
      {

         // Get the index of this move
         int idx = viSortOrder[ iMoveCount ];

         // Print out the move name.
         iNumberOfChars = PrintMove( argsBoard, argsGeneralMoves,
                                     &vsMoveList[ idx ], strMove );

         // Write out the move
         for ( iCharCount = 0; iCharCount < iNumberOfChars; iCharCount++ )
         {
            cout << strMove[ iCharCount ];
         }

         // Write out spaces to line up the output.
         for ( iCharCount = 0; iCharCount < 7 - iNumberOfChars; iCharCount++ )
         {
            cout << " ";
         }

         // Format and print the popularity
         cout << FormatWithCommas( (long long)viPopularity[ iMoveCount ] );

         // Calculate percentages
         BitBoard total = vbbWhiteWins[ idx ] + vbbBlackWins[ idx ] + vbbDraws[ idx ];
         if ( total > 0 )
         {
            dPercentWhiteWins = ( vbbWhiteWins[ idx ] * 100.0 ) / total;
            dPercentBlackWins = ( vbbBlackWins[ idx ] * 100.0 ) / total;
            dPercentDraws     = ( vbbDraws[ idx ] * 100.0 ) / total;

            double dUsagePercent = 0.0;
            if ( bbTotalGames > 0 )
            {
               dUsagePercent = (double)total / (double)bbTotalGames * 100.0;
            }

            cout << " (W:" << std::fixed << std::setprecision( 1 ) << dPercentWhiteWins << "% "
                 << "D:" << dPercentDraws << "% "
                 << "L:" << dPercentBlackWins << "%)"
                 << " U:" << dUsagePercent << "%";
         }

         cout << endl;
      }
   }

   // Show total games in the book with commas
   cout << "Total games in book = " << FormatWithCommas( (long long)bbTotalGames ) << endl
        << endl;

   // Close the book if no games are found.
   if ( bbTotalGames == 0 )
   {
      SetIsInBook( dNo );
   }
}

//
//
//---------------------------------------------------------------------
//
//
// This function returns the actual move in the argument argsBestMove.
// The argtiBookMove is -1 if no book move was found.
void FindBookMove( struct Board *argsBoard, struct GeneralMove *argsGeneralMoves,
                   struct Move *argsvMoveList, struct Move *argsBestMove,
                   int *argiBookMove )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Print the statistics from the open book for the moves from a given
   // position.
   BitBoard bbWhiteWins = 0;
   BitBoard bbBlackWins = 0;
   BitBoard bbDraws     = 0;
   int      iNumberOfMoves;
   int      iMoveCount;
   int      iWins                  = 0;
   int      iTotalGames            = 0;
   int      iNumberOfFeasibleMoves = 0;
   double   dPercentNotLoss        = 0;
   double   vdPercentWhiteWins[ dNumberOfMoves ];
   double   vdPercentBlackWins[ dNumberOfMoves ];
   double   vdPercentDraws[ dNumberOfMoves ];
   double   vCumPopularity[ dNumberOfMoves ];
   int      viTotalGames[ dNumberOfMoves ];
   int      viMoveList[ dNumberOfMoves ];
   char     strMove[ 64 ];

   // Calculate the moves for this position.
   CalculateMoves( argsvMoveList, argsBoard, argsGeneralMoves );

   // Some Debugging.
   if ( GetInterfaceBookDebug() )
   {

      gofDebugBook << "Hash at start = " << GetHash() << endl
                   << endl;
   }

   // Extract the number of moves.
   iNumberOfMoves = argsBoard->siNumberOfMoves;

   // Some Debugging.
   if ( GetInterfaceBookDebug() )
   {

      gofDebugBook << endl;
      gofDebugBook << "Here are a list of the moves." << endl
                   << endl;
   }

   // Loop over the moves.
   for ( iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++ )
   {

      // Set the score of the move to zero, as the score here is based on
      // popularity and not the standard move scoring used for searching
      argsvMoveList[ iMoveCount ].iScore = 0;

      // Make the move.
      MakeMove( argsvMoveList, argsBoard, argsGeneralMoves, iMoveCount );

      // Get the statistics.
      ExtractOpeningBookStats( &gsOpeningBook, bbWhiteWins, bbBlackWins, bbDraws,
                               argsBoard, argsGeneralMoves );

      // Undo the move.
      UndoMove( argsBoard, argsGeneralMoves );

      // Write out the stats in terms of percentages.
      // Only consider moves above the Violet threshold of quality.
      if ( ( bbWhiteWins + bbBlackWins + bbDraws ) > 0 )
      {

         vdPercentWhiteWins[ iMoveCount ] =
             (double)( bbWhiteWins ) /
             ( (double)( bbWhiteWins ) + (double)( bbBlackWins ) + (double)( bbDraws ) ) *
             100.0;
         vdPercentBlackWins[ iMoveCount ] =
             (double)( bbBlackWins ) /
             ( (double)( bbWhiteWins ) + (double)( bbBlackWins ) + (double)( bbDraws ) ) *
             100.0;
         vdPercentDraws[ iMoveCount ] =
             (double)( bbDraws ) /
             ( (double)( bbWhiteWins ) + (double)( bbBlackWins ) + (double)( bbDraws ) ) *
             100.0;
      }
      else
      {

         vdPercentWhiteWins[ iMoveCount ] = 0;
         vdPercentBlackWins[ iMoveCount ] = 0;
         vdPercentDraws[ iMoveCount ]     = 0;
      }

      // Here are criteria for rejecting a book move.
      viTotalGames[ iMoveCount ] =
          (int)bbWhiteWins + (int)bbBlackWins + (int)bbDraws;

      // Calculate the total games for this position.
      iTotalGames += viTotalGames[ iMoveCount ];

      if ( argsBoard->siComputerColor == dComputerWhite )
      {

         iWins = (int)bbWhiteWins;
         dPercentNotLoss =
             vdPercentWhiteWins[ iMoveCount ] + vdPercentDraws[ iMoveCount ];
      }
      if ( argsBoard->siComputerColor == dComputerBlack )
      {

         iWins = (int)bbBlackWins;
         dPercentNotLoss =
             vdPercentBlackWins[ iMoveCount ] + vdPercentDraws[ iMoveCount ];
      }

      // See if the move is feasibility.
      // Temporary debug: Show all moves with any games.
      if ( viTotalGames[ iMoveCount ] > 0 )
      {
         // if ((viTotalGames[iMoveCount] >= dMoveCutOff) && (iWins >= dWinCutOff) && (dPercentNotLoss >= dNotALoss)) {

         // Update the number of feasible moves.

         // New Logic: Filter out moves with < dMinBookUsagePercent
         double dUsagePercent = (double)viTotalGames[ iMoveCount ] / (double)iTotalGames * 100.0;

         if ( dUsagePercent >= dMinBookUsagePercent )
         {
            iNumberOfFeasibleMoves++;
            // If the move is feasible, set the move score to the total number of
            // games, this will allow the moves to be sorted by popularity.
            argsvMoveList[ iMoveCount ].iScore = viTotalGames[ iMoveCount ];
         }
         else
         {
            // Ignore this move
            argsvMoveList[ iMoveCount ].iScore = 0;
         }
      }
      else
      {

         // Make sure the score is zero.
         argsvMoveList[ iMoveCount ].iScore = 0;
      }

      strncpy( strMove, "      ", 6 );
      // Create a book move string.
      CreateAlgebraicMove( strMove, &argsvMoveList[ iMoveCount ], 0 );

      // Some Debugging.
      if ( GetInterfaceBookDebug() )
      {

         gofDebugBook << endl;
         gofDebugBook << "iMoveCount = " << iMoveCount << " Move = " << strMove
                      << " Score = " << argsvMoveList[ iMoveCount ].iScore << endl;
      }
   }

   // If not feasible, call the book quits.
   if ( iNumberOfFeasibleMoves == 0 )
   {

      // Return a book fail.
      *argiBookMove = -1;

      // Mark the board as being out of book.
      // This is imortant in other parts of the code where
      // the function ClearHash() is called.
      SetIsInBook( dNo );

      return;
   }

   // Sort the moves according to popularity.
   SortMoves( viMoveList, argsvMoveList, iNumberOfMoves );

   // Some Debugging.
   if ( GetInterfaceBookDebug() )
   {

      gofDebugBook << endl;
      gofDebugBook << "Here is a list of the sorted Moves." << endl
                   << endl;
   }

   // Recalculate total games for the denominator, because we may have filtered some moves out.
   // Use 'iTotalGames' is NO LONGER valid as the denominator if we zeroed out some scores.
   int iFilteredTotalGames = 0;
   for ( int i = 0; i < iNumberOfMoves; ++i )
   {
      iFilteredTotalGames += argsvMoveList[ i ].iScore;
   }
   // Avoid DBZ (though iNumberOfFeasibleMoves > 0 check above handles it)
   if ( iFilteredTotalGames == 0 )
      iFilteredTotalGames = 1;

   // Also calculate the cumulative popularity of the moves.
   for ( iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++ )
   {

      // Calculate the cumulative popularity.
      if ( iMoveCount == 0 )
      {

         // Assign the first value;
         vCumPopularity[ iMoveCount ] =
             (double)( argsvMoveList[ viMoveList[ iMoveCount ] ].iScore ) /
             (double)( iFilteredTotalGames );
      }
      else
      {

         // Assign the cumulative values;
         vCumPopularity[ iMoveCount ] =
             (double)( argsvMoveList[ viMoveList[ iMoveCount ] ].iScore ) /
                 (double)( iFilteredTotalGames ) +
             vCumPopularity[ iMoveCount - 1 ];
      }

      strncpy( strMove, "      ", 6 );
      // Create a book move string.
      CreateAlgebraicMove( strMove, &argsvMoveList[ viMoveList[ iMoveCount ] ], 0 );

      // Some Debugging.
      if ( GetInterfaceBookDebug() )
      {

         gofDebugBook << endl;
         gofDebugBook << "iMoveCount = " << iMoveCount << " Move = " << strMove
                      << " CumPop = " << vCumPopularity[ iMoveCount ] << endl;
      }
   }

   // Some QA/QC
   assert( vCumPopularity[ iNumberOfMoves - 1 ] > 0.999 );
   assert( vCumPopularity[ iNumberOfMoves - 1 ] < 1.001 );

   // Some Debugging.
   if ( GetInterfaceBookDebug() )
   {

      gofDebugBook << "Hash at end = " << GetHash() << endl
                   << endl;
   }

   // Choose the move to make via a random number and normalize to the popularity
   // cutoff. This means that only moves that have a cumulative popularity of 90%
   // will be played. This is done to stop the weirder in the database from being
   // played.
   double dSamplePopularity =
       (double)( rand() ) / (double)(RAND_MAX)*dPopularityCutOff;

   // Some Debugging.
   if ( GetInterfaceBookDebug() )
   {

      gofDebugBook << endl;
      gofDebugBook << "Here is the random number = " << dSamplePopularity << endl
                   << endl;
   }

   // Loop over the cumulative popularity to find a move.
   for ( int iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++ )
   {

      // Some Debugging.
      if ( GetInterfaceBookDebug() )
      {

         gofDebugBook << endl
                      << "iMoveCount = " << iMoveCount << endl
                      << endl;
      }

      // See if we are freshly over the criteria for a good move.
      // As a redundate check, make sure the move is feasable.
      if ( vCumPopularity[ iMoveCount ] >= dSamplePopularity )
      {

         // Some Debugging.
         if ( GetInterfaceBookDebug() )
         {

            gofDebugBook << "Now setting the move." << endl;
         }

         *argiBookMove = viMoveList[ iMoveCount ];
         *argsBestMove = argsvMoveList[ *argiBookMove ];

         // Generate move string early for output
         strncpy( strMove, "      ", 6 );
         CreateAlgebraicMove( strMove, argsBestMove, 0 );

         if ( GetInterfaceMode() == dUCI || GetInterfaceMode() == dVFE )
         {
            int bestIndex = *argiBookMove;
            // Trim move string for cleaner output
            std::string moveStr = strMove;
            moveStr.erase( moveStr.find_last_not_of( " \n\r\t" ) + 1 );

            cout << "info string book_stats Move:" << moveStr << " W:" << fixed << setprecision( 1 ) << vdPercentWhiteWins[ bestIndex ]
                 << " L:" << vdPercentBlackWins[ bestIndex ]
                 << " D:" << vdPercentDraws[ bestIndex ] << endl;
         }

         assert( *argiBookMove >= -1 );
         assert( *argiBookMove <= dNumberOfMoves );

         gofDebugBook << "argiBooMove = " << *argiBookMove << endl;

         // CreateAlgebraicMove was here, moved up

         // Some Debugging.
         if ( GetInterfaceBookDebug() )
         {

            gofDebugBook << endl;
            gofDebugBook << "Just after FindBookMove strMove = " << strMove << endl;
         }

         return;
      }
   }

   // This point should not have been reached.
   // If it is reached, return a book fail.
   *argiBookMove = iNumberOfMoves - 1;
   return;
}

//
//
//---------------------------------------------------------------------
//
//
int FindMaxScore( double *vdWins, int argiNumberOfMoves )
{
   int iMoveNumber = -1;
   int iNumberOfMoves;
   int iMaxWins = -1;

   // Loop over the statistics.
   for ( iNumberOfMoves = 0; iNumberOfMoves < argiNumberOfMoves;
         iNumberOfMoves++ )
   {

      // See if we have a good score and that the score exists.
      if ( ( iMaxWins < vdWins[ iNumberOfMoves ] ) && ( vdWins[ iNumberOfMoves ] > 0 ) )
      {

         iMaxWins    = (int)vdWins[ iNumberOfMoves ];
         iMoveNumber = iNumberOfMoves;
      }
   }

   // Look for a failure.
   if ( iMaxWins <= dWinCutOff )
   {

      iMoveNumber = -1;
   }

   assert( iMoveNumber >= -1 );
   assert( iMoveNumber <= dNumberOfMoves );

   return iMoveNumber;
}

//
//
//---------------------------------------------------------------------
//
//
//
//
//---------------------------------------------------------------------
//
//
std::mutex gBookMergeMutex;

// Unused / Duplicate MergeOpeningBooks
/*
void MergeOpeningBooks(struct Board *argsBoard, OpeningBook *dest, OpeningBook *src, GeneralMove *gm) {
  long long mergeCount = 0;
  for (BitBoard i = 0; i < src->bbOpeningBookSize; i++) {
    BitBoard srcElement = src->mbbHashTable[i];
    if (srcElement == 0)
      continue;

    mergeCount++;
    g_MergeOps++;

    BitBoard srcHash = src->mbbHash[i];
    BitBoard destHash = dest->mbbHash[i];

    if (destHash == 0) {
      // New entry
      dest->mbbHash[i] = srcHash;
      dest->mbbHashTable[i] = srcElement;
    } else if (destHash == srcHash) {
      // Same position, merge stats
      BitBoard destElement = dest->mbbHashTable[i];

      BitBoard w1 = (srcElement & gm->bbWhiteScore) >> gm->iWhiteScoreShift;
      BitBoard b1 = (srcElement & gm->bbBlackScore) >> gm->iBlackScoreShift;
      BitBoard d1 = (srcElement & gm->bbDrawScore) >> gm->iDrawScoreShift;

      BitBoard w2 = (destElement & gm->bbWhiteScore) >> gm->iWhiteScoreShift;
      BitBoard b2 = (destElement & gm->bbBlackScore) >> gm->iBlackScoreShift;
      BitBoard d2 = (destElement & gm->bbDrawScore) >> gm->iDrawScoreShift;

      BitBoard w = w1 + w2;
      BitBoard b = b1 + b2;
      BitBoard d = d1 + d2;

      // Reconstruct element
      BitBoard newElement = 0;
      newElement |= (w << gm->iWhiteScoreShift);
      newElement |= (b << gm->iBlackScoreShift);
      newElement |= (d << gm->iDrawScoreShift);

      dest->mbbHashTable[i] = newElement;
    }
  }
}
*/

//
//
//---------------------------------------------------------------------
//
//
void WriteOutOpeningBook( const char *argstrBook, int iPlyIndex, int iPrune )
{
   // Write out the opening book to a file.
   // Modified to write in BINARY format for compression and speed.
   // Format: [Header "VBOOKv1" 8 bytes] [Hash 8 bytes] [Data 8 bytes] ...

   // For backward compatibility or debugging, we could keep text output if a flag is set,
   // but primarily we want to replace it.
   // Let's modify the filename to .bin if it ends in .txt, or just append .bin?
   // The argument comes in as "C:\\VioletTools\\Book.txt" usually.
   // We will try to write to "Book.bin" in the same directory.

   std::string strTxtFilename = argstrBook;
   std::string strBinFilename = strTxtFilename;
   size_t      extPos         = strBinFilename.rfind( ".txt" );
   if ( extPos != std::string::npos )
   {
      strBinFilename.replace( extPos, 4, ".bin" );
   }
   else
   {
      strBinFilename += ".bin";
   }

   cout << "Writing Binary Book to " << strBinFilename << endl;
   cout << "Total Updates: " << g_BookUpdates << endl;
   cout << "Total Merges: " << g_MergeOps << endl;

   // open the output file in BINARY mode.
   ofstream ofBook( strBinFilename, ofstream::binary );
   if ( ofBook.fail() )
   {
      cout << "Output file failed to open." << endl;
      return;
   }

   // Write Header
   const char *header = "VBOOKv1 ";
   ofBook.write( header, 8 );

   long long writtenEntries = 0;
   long long prunedEntries  = 0;

   // Helper for unpacking (needed for frequency check)
   GeneralMove gm;
   GenerateGeneralMove( &gm );

   // Determine threshold
   int iMinVisits = 1; // Default save everything
   if ( iPrune )
   {
      iMinVisits = dMinBookVisits; // 2
   }

   // Buffer for writing to avoid millions of small I/O calls
   const int BUFFER_SIZE = 8192;
   struct BookEntry
   {
      BitBoard hash;
      BitBoard data;
   } buffer[ BUFFER_SIZE ];
   int bufferIdx = 0;

   for ( BitBoard bbHashCount = 0; bbHashCount < gsOpeningBook.bbOpeningBookSize;
         bbHashCount++ )
   {
      BitBoard element = GetBookElement( bbHashCount );
      if ( element != 0 )
      {
         // Frequency Check
         BitBoard w = ( element & gm.bbWhiteScore ) >> gm.iWhiteScoreShift;
         BitBoard b = ( element & gm.bbBlackScore ) >> gm.iBlackScoreShift;
         BitBoard d = ( element & gm.bbDrawScore ) >> gm.iDrawScoreShift;

         if ( ( w + b + d ) >= iMinVisits )
         {
            buffer[ bufferIdx ].hash = GetBookElementHash( bbHashCount );
            buffer[ bufferIdx ].data = element;
            bufferIdx++;

            if ( bufferIdx >= BUFFER_SIZE )
            {
               ofBook.write( (char *)buffer, sizeof( BookEntry ) * BUFFER_SIZE );
               bufferIdx = 0;
            }
            writtenEntries++;
         }
         else
         {
            prunedEntries++;
         }
      }
   }

   // Flush remaining buffer
   if ( bufferIdx > 0 )
   {
      ofBook.write( (char *)buffer, sizeof( BookEntry ) * bufferIdx );
   }

   cout << "Book Entries Written: " << writtenEntries << " (Pruned: " << prunedEntries << ")" << endl;
   cout << "Saved to " << strBinFilename << endl;

   ofBook.close();
}

//
//
//---------------------------------------------------------------------
//
//
void ReadOpeningBook( const char         *argstrBookName,
                      struct GeneralMove *argsGeneralMoves )
{

   // Debug the inputs.
   assert( argstrBookName >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Try to find the BINARY version first.
   std::string strFilename    = argstrBookName;
   std::string strBinFilename = strFilename;
   size_t      extPos         = strBinFilename.rfind( ".txt" );
   bool        triedBinary    = false;

   if ( extPos != std::string::npos )
   {
      strBinFilename.replace( extPos, 4, ".bin" );
   }
   else
   {
      // If input is already .bin or something else, try it directly
      if ( strBinFilename.find( ".bin" ) == std::string::npos )
         strBinFilename += ".bin";
   }

   // Check if binary file exists
   ifstream ifBin( strBinFilename, ifstream::binary );
   if ( ifBin.good() )
   {
      if ( GetInterfaceMode() != dUCI )
         cout << "Loading compressed opening book: " << strBinFilename << " ... " << endl;

      // Check Header
      char header[ 9 ];
      ifBin.read( header, 8 );
      header[ 8 ] = 0;

      if ( strcmp( header, "VBOOKv1 " ) == 0 )
      {
         // Fast Binary Load
         // Determine file size to estimate progress?
         ifBin.seekg( 0, ifBin.end );
         long long fileSize = ifBin.tellg();
         ifBin.seekg( 8, ifBin.beg ); // Skip header

         long long numEntries  = ( fileSize - 8 ) / 16;
         long long entriesRead = 0;

         const int BUFFER_SIZE = 8192;
         struct BookEntry
         {
            BitBoard hash;
            BitBoard data;
         } buffer[ BUFFER_SIZE ];

         while ( ifBin )
         {
            ifBin.read( (char *)buffer, sizeof( BookEntry ) * BUFFER_SIZE );
            streamsize bytesRead        = ifBin.gcount();
            int        entriesThisChunk = bytesRead / sizeof( BookEntry );

            for ( int i = 0; i < entriesThisChunk; ++i )
            {
               BitBoard bbHash = buffer[ i ].hash;
               BitBoard bbData = buffer[ i ].data;

               // Calculate the key (masked index)
               BitBoard bbKey = bbHash & gsOpeningBook.bbMaskIndex;

               // Validation (optional, but good for safety)
               assert( bbKey < gsOpeningBook.bbOpeningBookSize );

               SetBookElement( bbKey, bbData );
               SetBookElementHash( bbKey, bbHash );
               gsOpeningBook.bbNumberOfPositionsInBook++;
               entriesRead++;
            }
         }

         if ( GetInterfaceMode() != dUCI )
            cout << "Read Complete. Entries: " << entriesRead << endl;
         ifBin.close();
         return; // Success
      }
      else
      {
         cout << "Warning: " << strBinFilename << " has invalid header. Falling back to text mode." << endl;
      }
   }

   cout << "Binary book not found or invalid. Attempting legacy text text load: " << argstrBookName << endl;

   // Fallback to TEXT mode
   ifstream ifBook( argstrBookName ); // Text mode default
   if ( ifBook.fail() )
   {
      // SetUseOpeningBook( dNo ); // Don't disable yet, caller loops through multiple paths
      cout << "Failed to open " << argstrBookName << endl;
      return;
   }

   BitBoard bbKey;
   BitBoard bbData;
   BitBoard bbHash;

   // Loop over the file.
   long long textEntries = 0;
   while ( ifBook >> bbHash ) // Reads first token (Hash)
   {
      // Calculate the key.
      bbKey = bbHash & gsOpeningBook.bbMaskIndex;

      assert( bbKey < gsOpeningBook.bbOpeningBookSize );

      // Get the data.
      ifBook >> bbData;

      // Put the book in the hash table.
      SetBookElement( bbKey, bbData );
      SetBookElementHash( bbKey, bbHash );

      // Count the number of Positions.
      gsOpeningBook.bbNumberOfPositionsInBook++;
      textEntries++;
   }
   if ( GetInterfaceMode() != dUCI )
      cout << "Legacy Text Read Complete. Entries: " << textEntries << endl;
}

//
//
//---------------------------------------------------------------------
//
//
void TrimOpeningBook( struct GeneralMove *argsGeneralMoves, int iMinVisits )
{

   // Debug the inputs.
   assert( argsGeneralMoves >= 0 );

   // This funtion is used to get rid of positions that haven't been reached at
   // least a given amount of time.

   // Use the default if the input is zero.
   if ( iMinVisits <= 0 )
   {
      iMinVisits = dBookCutOff;
   }

   for ( BitBoard bbHashIndex = 0; bbHashIndex < gsOpeningBook.bbOpeningBookSize;
         bbHashIndex++ )
   {

      // Find the number of positions.
      BitBoard bbCountWhite =
          ( GetBookElement( bbHashIndex ) & argsGeneralMoves->bbWhiteScore ) >>
          argsGeneralMoves->iWhiteScoreShift;
      BitBoard bbCountBlack =
          ( GetBookElement( bbHashIndex ) & argsGeneralMoves->bbBlackScore ) >>
          argsGeneralMoves->iBlackScoreShift;
      BitBoard bbCountDraw =
          ( GetBookElement( bbHashIndex ) & argsGeneralMoves->bbDrawScore ) >>
          argsGeneralMoves->iDrawScoreShift;
      BitBoard bbCount = bbCountWhite + bbCountBlack + bbCountDraw;

      // Use the cut off level
      if ( bbCount < iMinVisits )
      {

         SetBookElement( bbHashIndex, 0 );
      }
   }
}

void RunBookTrimming( struct GeneralMove *argsGeneralMoves, int iMinVisits )
{

   // Debug the inputs.
   assert( argsGeneralMoves >= 0 );

   // Read the opening book.
   cout << "Reading Book.txt..." << endl;
   ReadOpeningBook( "Book.txt", argsGeneralMoves );
   cout << "Book Read. Positions: " << gsOpeningBook.bbNumberOfPositionsInBook
        << endl;

   // Trim the book.
   cout << "Trimming Book with min visits: " << iMinVisits << "..." << endl;
   TrimOpeningBook( argsGeneralMoves, iMinVisits );

   // Write out the trimmed book.
   // We use 0 for prune since we already trimmed it in memory.
   cout << "Writing TrimmedBook.txt..." << endl;
   WriteOutOpeningBook( "TrimmedBook.txt", 0, 0 );
   cout << "Book Trimming Complete." << endl;
}

//
//
//---------------------------------------------------------------------
//
//
void AppendBook( struct GeneralMove *argsGeneralMoves,
                 const char         *argstrBookName )
{

   // Debug the inputs.
   assert( argstrBookName >= 0 );
   assert( argsGeneralMoves >= 0 );

   BitBoard bbHash = 0;
   BitBoard bbKey  = 0;
   BitBoard bbData = 0;

   // Read the second opening book and add it in to the first.
   ifstream ifBook( argstrBookName, ifstream::binary );
   if ( ifBook.fail() )
   {

      cout << "Input Book.txt file failed to open." << endl;
   }

   // Loop over the file.
   while ( !ifBook.eof() )
   {

      // Get the key from the file.
      ifBook >> bbHash;

      // Calculate the key.
      bbKey = bbHash & gsOpeningBook.bbMaskIndex;

      assert( bbKey < gsOpeningBook.bbOpeningBookSize );

      // Get the data.
      ifBook >> bbData;

      // Combine the counts here.

      // Extract the current White count.
      BitBoard bbCountWhite1 =
          ( GetBookElement( bbKey ) & argsGeneralMoves->bbWhiteScore ) >>
          argsGeneralMoves->iWhiteScoreShift;

      BitBoard bbCountWhite2 = ( bbData & argsGeneralMoves->bbWhiteScore ) >>
                               argsGeneralMoves->iWhiteScoreShift;

      BitBoard bbCountWhite = bbCountWhite1 + bbCountWhite2;

      // Extract the current Black count.
      BitBoard bbCountBlack1 =
          ( GetBookElement( bbKey ) & argsGeneralMoves->bbBlackScore ) >>
          argsGeneralMoves->iBlackScoreShift;

      BitBoard bbCountBlack2 = ( bbData & argsGeneralMoves->bbBlackScore ) >>
                               argsGeneralMoves->iBlackScoreShift;

      BitBoard bbCountBlack = bbCountBlack1 + bbCountBlack2;

      // Extract the current Draw count.
      BitBoard bbCountDraw1 =
          ( GetBookElement( bbKey ) & argsGeneralMoves->bbDrawScore ) >>
          argsGeneralMoves->iDrawScoreShift;

      BitBoard bbCountDraw2 = ( bbData & argsGeneralMoves->bbDrawScore ) >>
                              argsGeneralMoves->iDrawScoreShift;

      BitBoard bbCountDraw = bbCountDraw1 + bbCountDraw2;

      // Input the data back into the hash.
      SetBookElement( bbKey, 0 );

      bbCountWhite = bbCountWhite << argsGeneralMoves->iWhiteScoreShift;

      bbCountBlack = bbCountBlack << argsGeneralMoves->iBlackScoreShift;

      bbCountDraw = bbCountDraw << argsGeneralMoves->iDrawScoreShift;

      BitBoard bbElement = GetBookElement( bbKey );

      bbElement |= bbCountWhite;

      bbElement |= bbCountBlack;

      bbElement |= bbCountDraw;

      SetBookElement( bbKey, bbElement );
   }

   // Close the input books
   ifBook.close();
}

//
//
//---------------------------------------------------------------------
//
//
//
//
//---------------------------------------------------------------------
//
//
void ExtractOpeningBookStats( OpeningBook *book, BitBoard &argbbWhiteWins,
                              BitBoard &argbbBlackWins, BitBoard &argbbDraws,
                              struct Board       *argsBoard,
                              struct GeneralMove *argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsGeneralMoves >= 0 );

   // Calculate the key to the hash table.
   BitBoard bbKey           = argsBoard->bbHash & book->bbMaskIndex;
   BitBoard bbHash          = GetBookElementHash( book, bbKey );
   BitBoard bbElement       = GetBookElement( book, bbKey );
   BitBoard bbHashFromTable = argsBoard->bbHash;

   if ( bbHash != bbHashFromTable )
   {
      if ( bbHash != 0 )
      {
         // cout << "MISMATCH: BoardHash=" << bbHashFromTable << " BookHash=" << bbHash << " Mask=" << book->bbMaskIndex << " Key=" << bbKey << endl;
      }
   }
   else
   {
      // cout << "MATCH: Hash=" << bbHashFromTable << endl;
   }

   // Check for a collison.
   // If the stored hash is zero, then they match by default!
   // if ( ( bbHash != bbHashFromTable ) &&
   //      ( bbHash != 0 ) )
   if ( bbHash != bbHashFromTable )
   {
      // Debug Mismatch details
      if ( bbHash != 0 )
      {
         // printf("DEBUG: MISMATCH: BoardHash=%llu BookHash=%llu Mask=%llu Key=%llu\n", bbHashFromTable, bbHash, book->bbMaskIndex, bbKey);
      }
      else
      {
         // printf("DEBUG: EMPTY SLOT Key=%llu BoardHash=%llu\n", bbKey, bbHashFromTable);
      }

      argbbWhiteWins = 0;
      argbbBlackWins = 0;
      argbbDraws     = 0;
   }
   else
   {
      // Debug Match
      // printf("DEBUG: MATCH FOUND! Key=%llu Element=%llu WMask=%llu\n", bbKey, bbElement, argsGeneralMoves->bbWhiteScore);

      // Extract the current White count.
      argbbWhiteWins =
          ( GetBookElement( book, bbKey ) & argsGeneralMoves->bbWhiteScore ) >>
          argsGeneralMoves->iWhiteScoreShift;

      // Extract the current Black count.
      argbbBlackWins =
          ( GetBookElement( book, bbKey ) & argsGeneralMoves->bbBlackScore ) >>
          argsGeneralMoves->iBlackScoreShift;

      // Extract the current Draw count.
      argbbDraws =
          ( GetBookElement( book, bbKey ) & argsGeneralMoves->bbDrawScore ) >>
          argsGeneralMoves->iDrawScoreShift;
   }
}

// Wrapper for existing calls (that use global book)
void ExtractOpeningBookStats( BitBoard &argbbWhiteWins, BitBoard &argbbBlackWins,
                              BitBoard           &argbbDraws,
                              struct GeneralMove *argsGeneralMoves,
                              struct Board       *argsBoard )
{

   OpeningBook *book = &gsOpeningBook;

   // Use the board's hash if provided, otherwise fallback to global (legacy support if needed, but we should fix calls)
   BitBoard currentHash = argsBoard ? argsBoard->bbHash : GetHash();
   BitBoard bbKey       = currentHash & book->bbMaskIndex;

   BitBoard bbHash          = GetBookElementHash( book, bbKey );
   BitBoard bbHashFromTable = currentHash;

   if ( bbHash != bbHashFromTable )
   {
      if ( true )
      {
         // Debug mismatch only if we are using a specific board (Explore context)
         cout << "MISMATCH: BoardHash=" << bbHashFromTable << " BookHash=" << bbHash << " Mask=" << book->bbMaskIndex << " Key=" << bbKey << endl;
      }
      argbbWhiteWins = 0;
      argbbBlackWins = 0;
      argbbDraws     = 0;
   }
   else
   {
      argbbWhiteWins =
          ( GetBookElement( book, bbKey ) & argsGeneralMoves->bbWhiteScore ) >>
          argsGeneralMoves->iWhiteScoreShift;
      argbbBlackWins =
          ( GetBookElement( book, bbKey ) & argsGeneralMoves->bbBlackScore ) >>
          argsGeneralMoves->iBlackScoreShift;
      argbbDraws =
          ( GetBookElement( book, bbKey ) & argsGeneralMoves->bbDrawScore ) >>
          argsGeneralMoves->iDrawScoreShift;
   }
}

void UpdateOpeningBook( OpeningBook *book, struct Board *argsBoard,
                        struct GeneralMove *argsGeneralMoves, int iGameResult )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );
   assert( iGameResult >= -1 );
   assert( iGameResult <= 2 );

   // Declare some variables.
   BitBoard bbCount = 0;
   int      iBitCount;

   // Input the data to hash table.
   BitBoard bbKey = argsBoard->bbHash & book->bbMaskIndex;

   // Look for a collision.
   BitBoard bbHash     = argsBoard->bbHash;
   BitBoard bbFullHash = GetBookElementHash( book, bbKey );

   if ( bbFullHash == 0 )
   {
      // This is a new position.
      SetBookElementHash( book, bbKey, bbHash );

      // Increment Book Count
      book->bbNumberOfPositionsInBook++;
   }
   else if ( bbHash != bbFullHash )
   {
      // If a collision, don't store the data.
      return;
   }

   // Input the data to hash table.
   // ... (existing code)

   // Put the opening book into the hash table memory.
   switch ( iGameResult )
   {

   case dWhiteWin:
   {
      g_BookUpdates++;
      // Extract the current White count.

      bbCount = ( GetBookElement( book, bbKey ) & argsGeneralMoves->bbWhiteScore ) >>
                argsGeneralMoves->iWhiteScoreShift;

      // Increment the white win count.
      if ( bbCount < ( ( (BitBoard)1 << dNumberOfBitsPerScore ) - 1 ) )
         bbCount++;

      // Zero out the current count.
      BitBoard bbElement = GetBookElement( book, bbKey );
      // printf( "DEBUG: UpdateOpeningBook Key=%llu OldElem=%llu Result=%d\n", bbKey, bbElement, iGameResult );

      for ( iBitCount = 0; iBitCount < dNumberOfBitsPerScore; iBitCount++ )
      {
         bbElement = SetBitToZero( bbElement, iBitCount + argsGeneralMoves->iWhiteScoreShift );
      }

      // Add the new count.
      bbElement = bbElement | ( bbCount << argsGeneralMoves->iWhiteScoreShift );
      SetBookElement( book, bbKey, bbElement );
      // printf( "DEBUG: UpdateOpeningBook Key=%llu NewElem=%llu\n", bbKey, bbElement );

      break;
   }
   case dBlackWin:
   {

      // Extract the current Black count.
      bbCount = ( GetBookElement( book, bbKey ) & argsGeneralMoves->bbBlackScore ) >>
                argsGeneralMoves->iBlackScoreShift;

      // Increment the Black win count.
      if ( bbCount < ( ( (BitBoard)1 << dNumberOfBitsPerScore ) - 1 ) )
         bbCount++;

      // Zero out the current count.
      BitBoard bbElement = GetBookElement( book, bbKey );
      for ( iBitCount = dNumberOfBitsPerScore;
            iBitCount < 2 * dNumberOfBitsPerScore; iBitCount++ )
      {

         bbElement = SetBitToZero( bbElement, iBitCount );
      }

      // Calculate the depth and enter it.
      bbCount = bbCount << argsGeneralMoves->iBlackScoreShift;
      bbElement |= bbCount;
      SetBookElement( book, bbKey, bbElement );

      break;
   }
   case dDraw:
   {

      // Extract the current Draw count.
      bbCount = ( GetBookElement( book, bbKey ) & argsGeneralMoves->bbDrawScore ) >>
                argsGeneralMoves->iDrawScoreShift;

      // Increment the Draw win count.
      if ( bbCount < ( ( (BitBoard)1 << dNumberOfBitsPerScore ) - 1 ) )
         bbCount++;

      // Zero out the current count.
      BitBoard bbElement = GetBookElement( book, bbKey );
      for ( iBitCount = 2 * dNumberOfBitsPerScore;
            iBitCount < 3 * dNumberOfBitsPerScore; iBitCount++ )
      {

         bbElement = SetBitToZero( bbElement, iBitCount );
      }

      // Calculate the depth and enter it.
      bbCount = bbCount << argsGeneralMoves->iDrawScoreShift;
      bbElement |= bbCount;
      SetBookElement( book, bbKey, bbElement );
      break;
   }
   case dUnknown:
   {
      // In this case do nothing.
      break;
   }
   }
}

// Wrapper
void UpdateOpeningBook( struct Board       *argsBoard,
                        struct GeneralMove *argsGeneralMoves, int iGameResult )
{
   UpdateOpeningBook( &gsOpeningBook, argsBoard, argsGeneralMoves, iGameResult );
}

//
//
//---------------------------------------------------------------------
//
//
void StartCheckBook( struct Board       *argsBoard,
                     struct GeneralMove *argsGeneralMoves )
{

   // Set the counters to zero.
   giTotalCount    = 0;
   giMarginalCount = 0;
   giCutCount      = 0;

   // Open the file for output of the checked book.
   // ifstream ifBook( argstrBookName, ifstream::binary );
   gofCheckedBook.open( "CheckedBook.txt", ofstream::binary );
   if ( gofCheckedBook.fail() )
   {

      cout << "CheckedBook.txt failed to open." << endl;
      system( "Pause" );
      return;
   }

   CheckBook( argsBoard, argsGeneralMoves );

   // Close the checked book.
   gofCheckedBook.close();
}

//
//
//---------------------------------------------------------------------
//
//
/*
void CheckBook( struct Board * argsBoard,
                struct GeneralMove * argsGeneralMoves,
                ofstream * argofCheckedBook )
*/
void CheckBook( struct Board *argsBoard, struct GeneralMove *argsGeneralMoves )
{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );
   assert( CheckBoard( argsBoard ) );

   // This function is called recursively to check the validity of the book.
   int   viPopularMoves[ dNumberOfMoves ];
   int   iNumberOfMoves;
   int   iNumberOfPopularMoves;
   Move *vsMoveList;
   vsMoveList      = (Move *)malloc( dNumberOfMoves * sizeof( Move ) );
   int      iScore = 0;
   int      iAlpha = dAlpha;
   int      iBeta  = dBeta;
   BitBoard bbKey  = 0;
   int      iMoveCount;

   // Calculate the moves for this position.
   CalculateMoves( vsMoveList, argsBoard, argsGeneralMoves );

   // Put the number of moves into a local variable.
   iNumberOfMoves = argsBoard->siNumberOfMoves;

   // Get the popular moves for this board.
   GetPopularMoves( argsBoard, argsGeneralMoves, viPopularMoves,
                    &iNumberOfPopularMoves, vsMoveList, iNumberOfMoves );

   // If the end of the book, return.
   if ( ( iNumberOfPopularMoves == 0 ) || // skip if the end of the book
        ( LookForDraw(
            argsBoard,
            argsGeneralMoves ) ) || // skip if we have seen this position before
        ( ( argsGeneralMoves->bbVerified &
            GetBookElement( GetKey() ) ) ) ) // skip if we have already verified this.
   {

      free( vsMoveList );
      return;
   }

   // Update the counters and publish.
   giTotalCount++;
   giMarginalCount++;
   if ( giMarginalCount > 9 )
   {

      giMarginalCount = 0;
   }

   ///*
   // Perform a search.
   iScore = StartSearch( argsBoard, argsGeneralMoves, iAlpha, iBeta );
   //*/

   // If the score is good, continue and write to checked book file.  If not,
   // return.
   if ( iScore < 0 )
   {

      iScore = -iScore;
   }
   if ( iScore < dOpeningBookScoreCutOff )
   {

      // This is a good position and we will continue.
      gofCheckedBook << GetBookElementHash( GetKey() ) << " "
                     << GetBookElement( GetKey() ) << " " << endl;
   }
   else
   {

      // If we fail, to a research to a deeper depth.
      SetSearchDepth(
          2 *
          dOpeningBookVerificationSearchDepth ); // dInfiniteDepth
                                                 // dOpeningBookVerificationSearchDepth

      SetSearchTimeInMiliSeconds(
          dTenMinutes ); // dInfiniteTime dOneSecond dTwentySeconds dOneMinute
                         // dTenMinutes

      // Perform a search.
      iScore = StartSearch( argsBoard, argsGeneralMoves, iAlpha, iBeta );

      // Set the initial parameters for controling the game
      SetSearchDepth(
          dOpeningBookVerificationSearchDepth ); // dInfiniteDepth
                                                 // dOpeningBookVerificationSearchDepth

      SetSearchTimeInMiliSeconds(
          dInfiniteTime ); // dInfiniteTime dOneSecond dTwentySeconds dOneMinute
                           // dTenMinutes

      // If the score is good, continue and write to checked book file.  If not,
      // return.
      if ( iScore < 0 )
      {
         iScore = -iScore;
      }
      if ( iScore < dOpeningBookScoreCutOff )
      {

         // This is a good position and we will continue.
         gofCheckedBook << GetBookElementHash( GetKey() ) << " "
                        << GetBookElement( GetKey() ) << " " << endl;
      }
      else
      {

         // The score didn't pass the cutoff.  Baill out.
         giCutCount++;
         PrintBoard( argsBoard->mBoard );
         PrintFEN( argsBoard, argsGeneralMoves );
         int iEvalScore = EvaluateBoard( argsBoard, argsGeneralMoves );
         PrintPrincipalVariation( argsBoard, argsGeneralMoves );
         // cout << "iScore = " << iScore << " iEval = " << iEvalScore << endl <<
         // endl; system( "Pause" );
         free( vsMoveList );
         return;
      }
   }

   // If we made it here we are verified.
   // Mark the position as having been checked.
   BitBoard bbElement = GetBookElement( GetKey() );
   BitBoard bbHash    = GetBookElementHash( GetKey() );

   ///*
   // See if the bit is already set to zero.
   if ( !( argsGeneralMoves->bbVerified & bbElement ) )
   {

      bbElement = SetBitToOne( bbElement, 60 );
   }
   //*/
   ///*
   // Put the element back in the book.
   SetBookElement( GetKey(), bbElement );
   //*/

   // Loop over the moves
   for ( iMoveCount = 0; iMoveCount < iNumberOfPopularMoves; iMoveCount++ )
   {

      if ( argsBoard->iMoveHistory == -1 )
      {

         if ( iMoveCount == 0 )
         {

            giTotalCount    = 0;
            giMarginalCount = 0;
         }
      }

      // Make the move.
      MakeMove( vsMoveList, argsBoard, argsGeneralMoves,
                viPopularMoves[ iMoveCount ] );

      CheckBook( argsBoard, argsGeneralMoves );

      // Undo the move.
      UndoMove( argsBoard, argsGeneralMoves );
   }

   free( vsMoveList );

   // put in a final loop here to take out all unverified elemensts.
}

//
//
//---------------------------------------------------------------------
//
//
void GetPopularMoves( struct Board       *argsBoard,
                      struct GeneralMove *argsGeneralMoves, int *viPopularMoves,
                      int *iNumberOfPopularMoves, struct Move *vsMoveList,
                      int iNumberOfMoves )

{

   // Debug the inputs.
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Print the statistics from the open book for the moves from a given
   // position.
   int      viSortOrder[ dNumberOfMoves ];
   int      viPopularity[ dNumberOfMoves ];
   BitBoard bbWhiteWins       = 0;
   BitBoard bbBlackWins       = 0;
   BitBoard bbDraws           = 0;
   double   dPercentWhiteWins = 0;
   double   dPercentBlackWins = 0;
   double   dPercentDraws     = 0;
   int      iMoveCount        = 0;

   // Reset the count of populare moves.
   *iNumberOfPopularMoves = 0;

   // Loop over the moves
   for ( iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++ )
   {

      // Make the move.
      MakeMove( vsMoveList, argsBoard, argsGeneralMoves, iMoveCount );

      // Get the statistics.
      ExtractOpeningBookStats( bbWhiteWins, bbBlackWins, bbDraws, argsGeneralMoves, argsBoard );

      // Undo the move.
      UndoMove( argsBoard, argsGeneralMoves );

      // Collect the popularity.
      viPopularity[ iMoveCount ] = (int)( bbWhiteWins + bbBlackWins + bbDraws );
      viSortOrder[ iMoveCount ]  = iMoveCount;

      // Calculate the number of populare moves.
      if ( viPopularity[ iMoveCount ] > 0 )
      {

         ( *iNumberOfPopularMoves )++;
      }
   }

   // Sort on the popularity
   int iSortFlag = 1;
   while ( iSortFlag )
   {

      // Set the default to bail.
      iSortFlag = 0;

      // Use a cocktail sort and to from top to bottom
      for ( int iMoveIndex = 0; iMoveIndex < iNumberOfMoves - 1; iMoveIndex++ )
      {

         if ( viPopularity[ iMoveIndex + 1 ] > viPopularity[ iMoveIndex ] )
         {

            int iDummyScore                = viPopularity[ iMoveIndex ];
            viPopularity[ iMoveIndex ]     = viPopularity[ iMoveIndex + 1 ];
            viPopularity[ iMoveIndex + 1 ] = iDummyScore;

            int iDummyPosition            = viSortOrder[ iMoveIndex ];
            viSortOrder[ iMoveIndex ]     = viSortOrder[ iMoveIndex + 1 ];
            viSortOrder[ iMoveIndex + 1 ] = iDummyPosition;

            iSortFlag = 1;
         }
      }
   }

   // Extract the populare moves
   for ( int iMoveIndex = 0; iMoveIndex < *iNumberOfPopularMoves; iMoveIndex++ )
   {

      viPopularMoves[ iMoveIndex ] = viSortOrder[ iMoveIndex ];
   }
}

//
//
//---------------------------------------------------------------------
//
//
int GetNumberOfPositionsInOpeningBook()
{
   return (int)( gsOpeningBook.bbNumberOfPositionsInBook );
}

//
//
//---------------------------------------------------------------------
//
//
int GetNumberOfPositionsVerified()
{
   return (int)( gsOpeningBook.bbNumberOfPositionsVerified );
}

//
//
//---------------------------------------------------------------------
//
//

void InitializeBookDebug()
{

   // Open some debugging files.
   gofDebugBook.open( "BookInterfaceLog.txt", ios::out | ios::app );
   gofDebugBook << endl;
   gofDebugBook << "Book log started." << endl
                << endl;
}

//
//
//---------------------------------------------------------------------
// Debug version of ProcessGameTask
void ProcessGameTaskDebug( GameTask &task, Board *board, GeneralMove *gm, OpeningBook *book, int iPlyIndex )
{
   printf( "DEBUG: ProcessGameTaskDebug processing %zu moves. Result: %d\n", task.moves.size(), task.result );
   fflush( stdout );

   Move moveList[ dNumberOfMoves ];

   for ( size_t i = 0; i < task.moves.size(); ++i )
   {
      CalculateMoves( moveList, board, gm );
      int iMoveIndex = GetMoveFromSAN( board, gm, moveList, (char *)task.moves[ i ].c_str() );

      if ( iMoveIndex == -1 )
      {
         printf( "ERROR: Move %s not found at %llu!\n", task.moves[ i ].c_str(), board->bbHash );
         break;
      }

      // Update Book for CURRENT position
      BitBoard bbHash  = board->bbHash;
      BitBoard bbIndex = bbHash & book->bbMaskIndex;
      printf( "Move %zu: %s. Hash: %llu Index: %llu\n", i + 1, task.moves[ i ].c_str(), bbHash, bbIndex );

      UpdateOpeningBook( book, board, gm, task.result );

      // Verify update immediately
      BitBoard stored = GetBookElement( book, bbIndex );
      printf( "   -> Updated Entry: %llu\n", stored );

      MakeMove( moveList, board, gm, iMoveIndex );
   }
   // Update final
   UpdateOpeningBook( book, board, gm, task.result );
}

// Debug Helper for Opening Book Lifecycle
void RunDebugLifecycle()
{
   setvbuf( stdout, NULL, _IONBF, 0 ); // Disable buffering
   printf( "\nDEBUG: RunDebugLifecycle ENTERED\n" );

   // 1. Initialize
   printf( "DEBUG: Destroying old book...\n" );
   DestroyOpeningBook( &gsOpeningBook );

   printf( "DEBUG: Initializing new book...\n" );
   InitializeOpeningBook( &gsOpeningBook );
   InitializeHashTable(); // Critical for Zobrist keys!
   printf( "DEBUG: Initialization Complete.\n" );

   // Use heap to avoid stack overflow (~1MB Board struct)
   Board       *board = new Board();
   GeneralMove *gm    = new GeneralMove();

   GenerateGeneralMove( gm );
   CreateBoard( board, gm );

   printf( "DEBUG: Injecting 10 Games...\n" );

   const char *openingLines[ 10 ][ 8 ] = {
       { "e4", "e5", "Nf3", "Nc6", "Bb5", "a6", "Ba4", "Nf6" },  // Ruy Lopez
       { "e4", "c5", "Nf3", "d6", "d4", "cxd4", "Nxd4", "Nf6" }, // Sicilian
       { "d4", "d5", "c4", "e6", "Nc3", "Nf6", "Bg5", "Be7" },   // QGD
       { "d4", "Nf6", "c4", "g6", "Nc3", "Bg7", "e4", "d6" },    // KID
       { "c4", "e5", "Nc3", "Nf6", "Nf3", "Nc6", "g3", "d5" },   // English
       { "Nf3", "d5", "g3", "Nf6", "Bg2", "e6", "O-O", "Be7" },  // Reti/KIA
       { "e4", "e6", "d4", "d5", "Nc3", "Bb4", "e5", "c5" },     // French
       { "e4", "c6", "d4", "d5", "e5", "Bf5", "Nf3", "e6" },     // Caro-Kann
       { "d4", "f5", "g3", "Nf6", "Bg2", "e6", "Nf3", "Be7" },   // Dutch
       { "b3", "e5", "Bb2", "Nc6", "e3", "Nf6", "Bb5", "Bd6" }   // Nimzo-Larsen
   };

   // We need a local book to match ProcessGameTask signature
   OpeningBook *localBook = new OpeningBook();
   InitializeOpeningBook( localBook );

   for ( int i = 0; i < 10; ++i )
   {
      GameTask task;
      task.result   = ( i % 2 == 0 ) ? dWhiteWin : dBlackWin; // Alternate wins
      task.whiteElo = 2800;
      task.blackElo = 2800;

      printf( "Injecting Game %d: ", i + 1 );
      for ( int m = 0; m < 8; ++m )
      {
         if ( openingLines[ i ][ m ] )
         {
            task.moves.push_back( openingLines[ i ][ m ] );
            printf( "%s ", openingLines[ i ][ m ] );
         }
      }
      printf( "\n" );
      ProcessGameTask( task, board, gm, localBook, 20 );
   }
   printf( "DEBUG: Process 10 Games Complete.\n" );

   // Merge to global
   printf( "DEBUG: Merging Books...\n" );
   MergeOpeningBooks( &gsOpeningBook, localBook, gm );
   printf( "DEBUG: Merge Complete.\n" );

   // Inspect
   printf( "\n=== REPORT START: Inspecting Memory for Start Position ===\n" );
   CreateBoard( board, gm ); // Reset to start
   {
      // 1. Raw Hash Check
      BitBoard storedHash = GetBookElementHash( &gsOpeningBook, board->bbHash & gsOpeningBook.bbMaskIndex );
      printf( "Start Pos Hash:   %llu\n", board->bbHash );
      printf( "Stored Book Hash: %llu\n", storedHash );
      if ( ( board->bbHash & gsOpeningBook.bbMaskIndex ) == ( storedHash & gsOpeningBook.bbMaskIndex ) )
         printf( "MATCH: Hash Key found in book.\n" );
      else
         printf( "FAIL: Hash Key NOT found.\n" );

      // 2. Move Statistics
      printf( "\n--- Available Moves from Book ---\n" );
      // Use standard probing function to list moves
      // PrintOpeningBookMoveStatistics( board, gm );
   }
   printf( "=== REPORT END ===\n" );

   // Write to Disk for Explorer
   printf( "DEBUG: Writing book to disk for Explorer...\n" );
   WriteOutOpeningBook( "C:\\VioletTools\\Book.txt", 0, 0 );
   printf( "DEBUG: Write Complete.\n" );

   // Cleanup
   DestroyOpeningBook( localBook );
   delete localBook;
   delete board;
   delete gm;

   printf( "DEBUG: Step 1 Complete -> Exiting.\n" );
   exit( 0 );
}
//
void CloseBookDebug()
{

   // Close down the communications.
   gofDebugBook << "Closing file." << endl;
   gofDebugBook << endl;
   gofDebugBook.close();
}
