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
std::mutex coutMutex;
std::atomic<int> gProcessedFiles(0);
std::atomic<int> gTotalFiles(0);
std::chrono::steady_clock::time_point gStartTime;

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

//
//---------------------------------------------------------------------------------------------------------------------------------------
//

//
//----------------// Globals for debug
std::atomic<long long> g_BookUpdates(0);
std::atomic<long long> g_MergeOps(0);
std::atomic<int> g_PendingTasks(0); // Tracking active games + markers
std::atomic<long long> g_TotalGamesAnalyzed(0);
const int dTaskMerge = -999;


//
//---------------------------------------------------------------------------------------------
//
void InitializeOpeningBook(OpeningBook *book) {
  // This function is used to initialize the hash table.  The function is stored
  // in the search file because the hash table is a global variable and is
  // needed the most in the search routines.
  //

  // Allocate the memory.
  book->bbOpeningBookSize = Power(2, dNumberOfBitsInOpeningBook) - 1;
  book->mbbHash =
      (BitBoard *)malloc(book->bbOpeningBookSize * sizeof(BitBoard));
  book->mbbHashTable =
      (BitBoard *)malloc(book->bbOpeningBookSize * sizeof(BitBoard));

  // Loop over the hash table entries and set them to zero.
  // #pragma omp parallel for schedule( dynamic, 1 )
  for (BitBoard bbHashIndex = 0; bbHashIndex < book->bbOpeningBookSize;
       bbHashIndex++) {

    book->mbbHashTable[bbHashIndex] = 0;
    book->mbbHash[bbHashIndex] = 0;
  }

  // Set up the mask for extracting the index.
  book->bbMaskIndex = 0;
  for (int iBitIndex = 0; iBitIndex < dNumberOfBitsInOpeningBook; iBitIndex++) {

    book->bbMaskIndex = SetBitToOne(book->bbMaskIndex, iBitIndex);
  }

  // Get ready to count the positions in the book.
  book->bbNumberOfPositionsInBook = 0;

  // Set the initial hash for later reference
  // book->bbHashInitial = book->bbHash;
}

void InitializeOpeningBook() { InitializeOpeningBook(&gsOpeningBook); }

//
//---------------------------------------------------------------------------------------------------------------------------------
//
void DestroyOpeningBook(OpeningBook *book) {
  // This function releases the memory taken by the hash table.

  free(book->mbbHash);
  free(book->mbbHashTable);
}

void DestroyOpeningBook() { DestroyOpeningBook(&gsOpeningBook); }

// Define some book sets.
void SetBookElement(OpeningBook *book, BitBoard bbKey, BitBoard bbElement) {
  book->mbbHashTable[bbKey] = bbElement;
}
void SetBookElementHash(OpeningBook *book, BitBoard bbKey, BitBoard bbElement) {
  book->mbbHash[bbKey] = bbElement;
}

// Wrappers
void SetBookElement(BitBoard bbKey, BitBoard bbElement) {
  SetBookElement(&gsOpeningBook, bbKey, bbElement);
}
void SetBookElementHash(BitBoard bbKey, BitBoard bbElement) {
  SetBookElementHash(&gsOpeningBook, bbKey, bbElement);
}

// Define some input and some output variables.
BitBoard GetBookElement(OpeningBook *book, BitBoard bbKey) {

  assert(bbKey <= book->bbOpeningBookSize);

  return book->mbbHashTable[bbKey];
}
BitBoard GetBookElementHash(OpeningBook *book, BitBoard bbKey) {

  assert(bbKey <= book->bbOpeningBookSize);

  return book->mbbHash[bbKey];
}

// Wrappers
BitBoard GetBookElement(BitBoard bbKey) {
  // Debug
  // cout << "GetBookElement(" << bbKey << ") = " << GetBookElement(&gsOpeningBook, bbKey) << endl;
  return GetBookElement(&gsOpeningBook, bbKey);
}
BitBoard GetBookElementHash(BitBoard bbKey) {
  // Debug
  // cout << "GetBookElementHash(" << bbKey << ") = " << GetBookElementHash(&gsOpeningBook, bbKey) << endl;
  return GetBookElementHash(&gsOpeningBook, bbKey);
}

extern std::atomic<long long> g_MergeOps;

void MergeOpeningBooks(OpeningBook *dest, OpeningBook *src, GeneralMove *gm) {
  long long mergeCount = 0;
  for (BitBoard i = 0; i < src->bbOpeningBookSize; i++) {
    BitBoard srcElement = src->mbbHashTable[i];
    if (srcElement == 0)
      continue;
     
    mergeCount++;
    g_MergeOps++;

    // Debug first few merges
    // if (g_MergeOps < 10) {
    //    cout << "Merge Ops: Found src element at index " << i << endl;
    // }

    BitBoard srcHash = src->mbbHash[i];
    BitBoard destHash = dest->mbbHash[i];

    if (destHash == 0) {
      // New entry
      dest->mbbHash[i] = srcHash;
      dest->mbbHashTable[i] = srcElement;
    } else if (destHash == srcHash) {
      // Same position, merge stats
      BitBoard w1 = (srcElement & gm->bbWhiteScore) >> gm->iWhiteScoreShift;
      BitBoard b1 = (srcElement & gm->bbBlackScore) >> gm->iBlackScoreShift;
      BitBoard d1 = (srcElement & gm->bbDrawScore) >> gm->iDrawScoreShift;

      BitBoard destElement = dest->mbbHashTable[i];
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

// Defines
void UpdateOpeningBook(OpeningBook *book, struct Board *argsBoard,
                       struct GeneralMove *argsGeneralMoves, int iGameResult);
void ReadAPGNMove(int *iGameResult, int *iFlagLine, int *iFlagBook,
                  int *iIsGoodGame, int *iGoodWhiteELO, int *iGoodBlackELO,
                  char *strMove, struct Board *argsBoard,
                  struct GeneralMove *argsGeneralMoves, int iPlyCount,
                  int iPlyIndex);

#include <ctype.h>

// Helper to identify file/rank char
bool isSANFile(char c) { return c >= 'a' && c <= 'h'; }
bool isSANRank(char c) { return c >= '1' && c <= '8'; }

int GetMoveFromSAN(struct Board *argsBoard, struct GeneralMove *argsGeneralMoves, struct Move *vsMoveList, string strMove) {

    // 1. Clean string (remove +, #, etc)
    string san = strMove;
    // Remove check/mate/annotation chars
    while (!san.empty() && (san.back() == '+' || san.back() == '#' || san.back() == '!' || san.back() == '?')) {
        san.pop_back();
    }
    
    // 2. Identify castling
    if (san == "O-O" || san == "0-0") {
         // Find Short Castle Move
         for(int i=0; i<argsBoard->siNumberOfMoves; ++i) {
             if (vsMoveList[i].iMoveType & dWhiteKingSideCastle) { return i; } 
             if (vsMoveList[i].iMoveType & dBlackKingSideCastle) { return i; }
             // Also check standard Castle definition if flags are different
             if (vsMoveList[i].iMoveType == dCastle) {
                  if (vsMoveList[i].iFromSquare == dE1 && vsMoveList[i].iToSquare == dG1) { return i; }
                  if (vsMoveList[i].iFromSquare == dE8 && vsMoveList[i].iToSquare == dG8) { return i; }
             }
         }
         return -1;
    }
    if (san == "O-O-O" || san == "0-0-0") {
         // Long Castle
         for(int i=0; i<argsBoard->siNumberOfMoves; ++i) {
             if (vsMoveList[i].iMoveType & dWhiteQueenSideCastle) { return i; }
             if (vsMoveList[i].iMoveType & dBlackQueenSideCastle) { return i; }
             if (vsMoveList[i].iMoveType == dCastle) {
                  if (vsMoveList[i].iFromSquare == dE1 && vsMoveList[i].iToSquare == dC1) { return i; }
                  if (vsMoveList[i].iFromSquare == dE8 && vsMoveList[i].iToSquare == dC8) { return i; }
             }
         }
         return -1;
    }

    // 3. Parse Piece and Destination
    int pieceType = dWhitePawn; // Default Pawn (if no letter)
    if (argsBoard->siColorToMove == dBlack) pieceType = dBlackPawn;
    
    int strIdx = 0;
    char firstChar = san[0];
    
    // Check if first char is a piece letter (and uppercase)
    if (isupper(firstChar)) {
        if (firstChar == 'N') pieceType = (argsBoard->siColorToMove == dWhite) ? dWhiteKnight : dBlackKnight;
        else if (firstChar == 'B') pieceType = (argsBoard->siColorToMove == dWhite) ? dWhiteBishop : dBlackBishop;
        else if (firstChar == 'R') pieceType = (argsBoard->siColorToMove == dWhite) ? dWhiteRook : dBlackRook;
        else if (firstChar == 'Q') pieceType = (argsBoard->siColorToMove == dWhite) ? dWhiteQueen : dBlackQueen;
        else if (firstChar == 'K') pieceType = (argsBoard->siColorToMove == dWhite) ? dWhiteKing : dBlackKing;
        strIdx++;
    }
    
    // Parse Destination (last 2 chars usually, unless promotion)
    // Handle promotion: e8=Q
    char promoteChar = 0;
    size_t eqPos = san.find('=');
    if (eqPos != string::npos) {
        promoteChar = san[eqPos+1]; // e.g. Q
        san = san.substr(0, eqPos);
    }
    
    // Destination is the last 2 format chars of the remaining string 
    // Example: Nbd7 -> d7. Naxb4 -> b4.
    // We need to walk backwards from end of san checking for rank/file
    
    int destFile = -1;
    int destRank = -1;
    
    int len = san.length();
    if (len < 2) return -1;
    
    // The last char should be rank, char before file
    // Check ranges safely
    if (isSANRank(san[len-1]) && isSANFile(san[len-2])) {
        destRank = san[len-1] - '1'; // '1'->0
        destFile = san[len-2] - 'a'; // 'a'->0
    } else {
        return -1; // Invalid format
    }
    
    // Violet: Square = Row + Col*8. Row=File, Col=Rank.
    // Square = File + Rank*8
    int destSquare = destFile + destRank * 8;
    
    // 4. Checking Moves
    int bestMoveIdx = -1;
    int matchCount = 0;
    
    for(int i=0; i<argsBoard->siNumberOfMoves; ++i) {
        // Filter by Piece Type
        // Note: Violet Moves stores the piece type.
        // Special case: Promotion. Violet might store 'Pawn' but 'Promote' type, 
        // OR it might store 'Queen' if it pre-generates.
        // Let's assume Violet stores the moving piece (Pawn).
        // So if we found 'Pawn' but it's a promotion, we still check Pawn.
        // Wait, if input is "N..." pieceType is Knight.
        
        if (vsMoveList[i].iPiece != pieceType) continue;
        
        // Filter by Destination
        if (vsMoveList[i].iToSquare != destSquare) continue;
        
        // Filter by Promotion
        if (promoteChar) {
             if (!(vsMoveList[i].iMoveType & dPromote)) continue;
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
        
        int src = vsMoveList[i].iFromSquare;
        int srcFile = dRow(src); // 0-7 (File)
        int srcRank = dCol(src); // 0-7 (Rank)
        
        // Disambiguation check
        bool match = true;
        
        // If we skipped piece char coverage (strIdx)
        // Verify middle chars
        for(int k=strIdx; k<len-2; ++k) {
            char c = san[k];
            if (c == 'x') continue;
            if (isSANFile(c)) {
                if ((c - 'a') != srcFile) match = false;
            } else if (isSANRank(c)) {
                if ((c - '1') != srcRank) match = false;
            }
        }
        
        if (match) {
            bestMoveIdx = i;
            matchCount++;
        }
    }
    
    if (matchCount >= 1) return bestMoveIdx;
    return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Some analysis elements
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// GameTask structure
struct GameTask {
    vector<string> moves;
    int whiteElo = 0;
    int blackElo = 0;
    int result = 0; // dWhiteWin, dBlackWin, dDraw, dUnknown
};

// Thread-safe Queue
template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable cond_empty;
    std::condition_variable cond_full;
    bool finished = false;
    size_t maxSize;

public:
    ThreadSafeQueue(size_t size = 50000) : maxSize(size) {}

    void push(T item) {
        std::unique_lock<std::mutex> lock(mutex);
        // Block if full
        cond_full.wait(lock, [this](){ return queue.size() < maxSize; });
        queue.push(item);
        cond_empty.notify_one();
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex);
        // Block if empty, unless finished
        cond_empty.wait(lock, [this](){ return !queue.empty() || finished; });
        
        if (queue.empty()) return false;
        
        item = queue.front();
        queue.pop();
        cond_full.notify_one(); // Notify producer that space is available
        return true;
    }

    void setFinished() { 
        std::lock_guard<std::mutex> lock(mutex);
        finished = true;
        cond_empty.notify_all();
    }
    
    void reset() {
        std::lock_guard<std::mutex> lock(mutex);
        finished = false;
        // Queue should ideally be empty when resetting, or we accept leftovers
    }
    
    bool empty() {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }
    
    size_t size() {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }
};

void ResetOpeningBook(OpeningBook* book) {
    // Zero out the hash table without freeing memory
    // #pragma omp parallel for schedule( dynamic, 1 )
    for (BitBoard bbHashIndex = 0; bbHashIndex < book->bbOpeningBookSize; bbHashIndex++) {
        book->mbbHashTable[bbHashIndex] = 0;
        book->mbbHash[bbHashIndex] = 0;
    }
    
    book->bbMaskIndex = 0;
    for (int iBitIndex = 0; iBitIndex < dNumberOfBitsInOpeningBook; iBitIndex++) {
        book->bbMaskIndex = SetBitToOne(book->bbMaskIndex, iBitIndex);
    }
    book->bbNumberOfPositionsInBook = 0;
}

void ProcessGameTask(const GameTask& task, struct Board* argsBoard, struct GeneralMove* argsGeneralMoves, OpeningBook* localBook, int iPlyIndexTarget) {
    
    // Reset Board
    CreateBoard(argsBoard, argsGeneralMoves);

    int iMoveCount = 1;
    int iPlyCount = 1;

    // Check ELO requirements if applicable
    int iIsGoodGame = 1;
     if (GetLimitELO()) {
          if (task.whiteElo > 0 && task.whiteElo < 2500) iIsGoodGame = 0;
          if (task.blackElo > 0 && task.blackElo < 2500) iIsGoodGame = 0;
     }
     
     if (!iIsGoodGame) return;

     Move vsMoveList[dNumberOfMoves];
     char strMoveNumber[64];
    
     for (size_t i = 0; i < task.moves.size(); ++i) {
         const string& strMove = task.moves[i];
         
         // Move Parsing logic similar to ReadAPGNMove but simplified since we have tokens
         sprintf(strMoveNumber, "%d.", iMoveCount);
         
         // Skip move numbers if present (though our parser hopefully handles this, PGNs might have "1. e4" as "1." then "e4")
         if (strMove == strMoveNumber) continue; // Just the number
         if (strMove.find('.') != string::npos && isdigit(strMove[0])) {
             // Form "1.e4" or "1..." - simplistic check, better to catch "1." separately. 
             // Ideally the parser separates "1." and "e4".
             // If token matches number pattern, skip/handle.
             if (strcmp(strMove.c_str(), strMoveNumber) == 0) continue;
         }

             if (iPlyCount <= iPlyIndexTarget + 1) {
                 CalculateMoves(vsMoveList, argsBoard, argsGeneralMoves);
                 int iMoveNumber = GetMoveFromSAN(argsBoard, argsGeneralMoves, vsMoveList, strMove);
                 // int iMoveNumber = GetMoveNumberFast(argsBoard, argsGeneralMoves, vsMoveList, (char*)strMove.c_str());
                 


                 if (iMoveNumber != -1) {
                     // Make the move.
                     MakeMove(vsMoveList, argsBoard, argsGeneralMoves, iMoveNumber);

                     argsBoard->iNumberOfPlys = -1;
                     iPlyCount++;
                     if (i % 2 == 1) iMoveCount++; // Increment move count after black's move
                 } else {
                     // Invalid move found, abort game
                     static int invalidMoveDebugCount = 0;
                     if (invalidMoveDebugCount < 20) {
                         cout << "Abort: Invalid Move '" << strMove << "' Expected Ply: " << iMoveCount << endl;
                         invalidMoveDebugCount++;
                     }
                     return;
                 }
            } else {
                // Reached target depth
                break; 
            }
     }
     
    // Update Book
    int iMoveHistory = argsBoard->iMoveHistory + 1; // Correct limit: Start + moves.
    for (int iReverseIndex = 0; iReverseIndex < iMoveHistory; iReverseIndex++) {
        UpdateOpeningBook(localBook, argsBoard, argsGeneralMoves, task.result);
        if (iReverseIndex < iMoveHistory - 1) {
             UndoMove(argsBoard, argsGeneralMoves);
        }
    }
}

void OpeningBookAnalysis(struct Board *mainArgsBoard,
                         struct GeneralMove *mainArgsGeneralMoves) {

  assert(mainArgsBoard >= 0);
  assert(mainArgsGeneralMoves >= 0);

  char strBookName[256];
  strcpy(strBookName, "C:\\VioletTools\\Book.txt");

  // 1. Discovery
  vector<string> files;
  // Check if directory exists
  if (!std::filesystem::exists("Lichess Elite Database")) {
       cout << "Error: Directory 'Lichess Elite Database' not found." << endl;
       return;
  }
  
  for (const auto &entry : std::filesystem::directory_iterator("Lichess Elite Database")) {
    if (entry.path().extension() == ".pgn") {
      files.push_back(entry.path().string());
    }
  }

  gTotalFiles = files.size();
  gProcessedFiles = 0;
  gStartTime = std::chrono::steady_clock::now();

  int numThreads = std::thread::hardware_concurrency();
  if (numThreads == 0) numThreads = 4;
  
  // Ensure we use all cores (minus one for main thread maybe, or just use all)
  cout << "Starting Opening Book Analysis with " << numThreads
       << " threads." << endl;
  cout << "Found " << gTotalFiles << " files to process." << endl;

  ThreadSafeQueue<GameTask> workQueue(50000); // Buffer 50k games
  vector<thread> threads;
  
  // Start Persistent Workers
  for (int t = 0; t < numThreads; ++t) {
      threads.push_back(thread([&workQueue, t]() { // Capture t for ID
          struct Board *argsBoard = new Board(); // Local board
          struct GeneralMove *argsGeneralMoves = new GeneralMove(); // Local moves
          OpeningBook *localBook = new OpeningBook();
          InitializeOpeningBook(localBook);
          
          GenerateGeneralMove(argsGeneralMoves);
          
          GameTask task;
          // Loop until queue is finished and empty
          while(workQueue.pop(task)) {
               if (task.result == dTaskMerge) {
                   // Synchronization point: Merge and Reset
                   {
		       // Lock the mutex to ensure that only one thread is merging at a time.
		       extern std::mutex gBookMergeMutex;
                       std::lock_guard<std::mutex> lock(gBookMergeMutex);
                       MergeOpeningBooks(&gsOpeningBook, localBook, argsGeneralMoves); 
                   }
                   
                   ResetOpeningBook(localBook);
               } else {
                   ProcessGameTask(task, argsBoard, argsGeneralMoves, localBook, dMaxBookPly);
                   long long currentTotal = ++g_TotalGamesAnalyzed;
                   if (currentTotal % 1000 == 0) {
                       extern std::mutex coutMutex;
                       
                       auto now = std::chrono::steady_clock::now();
                       std::chrono::duration<double> elapsed = now - gStartTime;
                       double timePerGame = elapsed.count() / (double)currentTotal;
                       
                       std::lock_guard<std::mutex> lock(coutMutex);
                       cout << "Analyzed " << currentTotal << " games. Time per game: " << std::fixed << std::setprecision(6) << timePerGame << " seconds." << endl;
                   }
               }
               g_PendingTasks--; // Task complete
          }
          
          // Final Merge (if any leftovers)
          if (localBook->bbNumberOfPositionsInBook > 0) {
              {
                  extern std::mutex gBookMergeMutex;
                  std::lock_guard<std::mutex> lock(gBookMergeMutex);
                  MergeOpeningBooks(&gsOpeningBook, localBook, argsGeneralMoves); 
              }
          }
          
          DestroyOpeningBook(localBook);
          delete localBook;
          delete argsBoard;
          delete argsGeneralMoves;
      }));
  }

  // Main Thread: Producer (File Reader)
  long long totalGamesFound = 0;
  long long g_LastPruneCount = 0; // Track when we last pruned
  
  for (const auto &filename : files) {
      gProcessedFiles++;
      
      {
          std::lock_guard<std::mutex> lock(coutMutex);
          cout << "Reading file " << gProcessedFiles << " of " << gTotalFiles << ": " << filename 
               << " (Queue: " << workQueue.size() << ")" << endl;
      }
      
      ifstream ifBook(filename);
      if (ifBook.fail()) continue;
      
      string token;
      GameTask currentTask;
      bool inTag = false;
      
      int variationDepth = 0; // Track parenthesis depth for variations
      bool inComment = false; // Track curly brace comments
      
      while (ifBook >> token) {
           if (token.empty()) continue;
        
           // Debuging first few tokens of first file
           static int debugTokens = 0;
           if (gProcessedFiles == 1 && debugTokens < 20) {
               // cout << "Token[" << debugTokens << "]: " << token << endl;
               debugTokens++;
           }

           // Handle 'old style' comments ; to end of line?
           // Minimal PGN parser usually primarily cares about {} and ()

           // Check for start of tag
           if (!inComment && variationDepth == 0 && token.front() == '[') {
               inTag = true;
           }

           if (inTag) {
               if (token.back() == ']') inTag = false;
               continue;
           }
           
           // Comment Handling
           if (token.find('{') != string::npos) {
               inComment = true;
               // If the token *ends* with }, comment also ends immediately (e.g. {comment})
               // But we must be careful of multiple braces. 
               // For simplicity, assuming standard spaced PGN or simple nesting if strictly { ... }
               // If token contains '}', we turn off. 
               // NOTE: This basic parser assumes { and } are separated or properly distinct. 
           }
           if (inComment) {
               if (token.find('}') != string::npos) {
                   inComment = false;
               }
               continue; // Skip comment token
           }

           // Variation Handling
           // Count '(' and ')'
           for(char c : token) {
               if (c == '(') variationDepth++;
               else if (c == ')') variationDepth--;
           }
           // If we are inside a variation (depth > 0) or just closed one, skip this token
           // Note: if token is ")", depth decremented, but we still skip that token.
           if (variationDepth > 0 || token.find(')') != string::npos) {
                continue;
           }
           
           // NAGs ($1, $2, etc)
           if (token[0] == '$') continue;

           // Check for result
           int iGameResult = -1;
           if (token == "1-0") iGameResult = dWhiteWin;
           else if (token == "0-1") iGameResult = dBlackWin;
           else if (token == "1/2-1/2") iGameResult = dDraw;
           else if (token == "*") iGameResult = dUnknown;
           
           if (iGameResult != -1) {
               currentTask.result = iGameResult;
               // End of game
               if (!currentTask.moves.empty()) {
                    g_PendingTasks++; // Increment BEFORE push
                    workQueue.push(currentTask); // Blocks if queue is full
                    totalGamesFound++;
               }
               currentTask = GameTask(); // Reset
           } else {
               // Move
               // Filter out move numbers like "1." if adjacent logic fails, but cleaner to just keep them 
               // and let ProcessGameTask handle string matching, OR strip them here.
               // ProcessGameTask expects "1." to check against but skips it. 
               // Let's filter out non-move garbage if possible.
               
               // If token contains '.', it's likely a number "1." or "1.e4"
               // "1.e4" should be split? 
               // Standard PGN has "1. e4". 
               // If "1.e4", we should technically handle it.
               // For now, push raw token.
               
               currentTask.moves.push_back(token);
           }
      }
      ifBook.close();
      
      // End of File: Synchronize
      // Push Markers
      for(int i=0; i<numThreads; ++i) {
          GameTask marker; 
          marker.result = dTaskMerge;
          g_PendingTasks++;
          workQueue.push(marker);
      }
      
      // Wait for all tasks to finish
      // Note: We use a simple spin-wait here. Could use condition variable but 
      // g_PendingTasks changes rapidly, so spin/sleep is fine.
      while(g_PendingTasks > 0) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      
      // Determine if we should prune
      int bPrune = 0;
      // if ((g_TotalGamesAnalyzed - g_LastPruneCount) >= 10000) {
      //     bPrune = 1;
      //     g_LastPruneCount = g_TotalGamesAnalyzed;
      // }

      // Write Book
      cout << "File complete. Total Games Analyzed: " << g_TotalGamesAnalyzed << ". Writing Book..." << endl;
      WriteOutOpeningBook(strBookName, dMaxBookPly, bPrune);
      // break; // DEBUG: Stop after 1 file to quickly regenerate a valid book for testing.
  }
  
  cout << "All files read. Total games pushed: " << totalGamesFound << ". Waiting for workers..." << endl;
  
  // Signal workers that no more work is coming
  workQueue.setFinished(); 
  
  // Join threads
  for (auto &t : threads) {
    if (t.joinable()) t.join();
  }
  threads.clear();
  
  cout << "All workers joined. Writing final book..." << endl;
  WriteOutOpeningBook(strBookName, dMaxBookPly, 0); // Always prune at the end
  cout << "Opening Book Creation Complete." << endl;
}

void ReadAPGNMove(int *iGameResult, int *iFlagLine, int *iFlagBook,
                  int *iIsGoodGame, int *iGoodWhiteELO, int *iGoodBlackELO,
                  char *strMove, struct Board *argsBoard,
                  struct GeneralMove *argsGeneralMoves, int iPlyCount,
                  int iPlyIndex) {

  // Debug the inputs.
  assert(argsBoard >= 0);
  assert(argsGeneralMoves >= 0);

  // This routine reads a move, either black or white from a file.
  Move vsMoveList[dNumberOfMoves];
  int iMoveNumber;

  // Look to see if we are at the end of a game.
  // Look for an ending.
  if (LookForGameResult(strMove, iGameResult) == 1) {

    // Reset the ELO flags.
    *iGoodWhiteELO = 0;
    *iGoodBlackELO = 0;

    *iFlagLine = 0;
    return;
  }
  // Look for an EOF or more precisely an empty strMove.
  if (LookForGameResult(strMove, iGameResult) == -1) {

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
  if (iIsGoodGame && (iPlyCount <= iPlyIndex + 1)) {

    // Calculate the moves.
    CalculateMoves(vsMoveList, argsBoard, argsGeneralMoves);

    // Get the matching move number.
    iMoveNumber =
        GetMoveNumberFast(argsBoard, argsGeneralMoves, vsMoveList, strMove);

    // Look for a bad move in the data.
    if (iMoveNumber == -1) {

      /*
      PrintBoard( argsBoard->mBoard );
      PrintFEN( argsBoard,
                argsGeneralMoves );
      */
      *iIsGoodGame = 0;
    }

    if (*iIsGoodGame) {

      // If the move was found make it move on with life.
      MakeMove(vsMoveList, argsBoard, argsGeneralMoves, iMoveNumber);

      // Reset the current ply depth.
      argsBoard->iNumberOfPlys = -1;
    }
  }
}

int LookForGameResult(char *argstrMove, int *argiGameResult) {

  // Debug the inputs.
  assert(argstrMove >= 0);

  // Declare some variables.
  int iResultFound = 0;

  if (strcmp(argstrMove, "1-0") == 0) {

    *argiGameResult = dWhiteWin;
    iResultFound = 1;
  }
  if (strcmp(argstrMove, "0-1") == 0) {

    *argiGameResult = dBlackWin;
    iResultFound = 1;
  }
  if (strcmp(argstrMove, "1/2-1/2") == 0) {

    *argiGameResult = dDraw;
    iResultFound = 1;
  }
  if (strcmp(argstrMove, "*") == 0) {

    *argiGameResult = dUnknown;
    iResultFound = 1;
  }
  if (strcmp(argstrMove, "") == 0) {

    iResultFound = -1;
  }

  assert(iResultFound >= -1);
  assert(iResultFound <= 1);

  // Return whether or not we found a result
  return iResultFound;
}

//
//----------------------------------------------------------------------------------------------------------
//
void PrintOpeningBookMoveStatistics(struct Board *argsBoard,
                                    struct GeneralMove *argsGeneralMoves) {

  // Debug the inputs.
  assert(argsBoard >= 0);
  assert(argsGeneralMoves >= 0);

  // Print the statistics from the open book for the moves from a given
  // position.
  int iMoveCount;
  Move vsMoveList[dNumberOfMoves];
  int viSortOrder[dNumberOfMoves];
  int viPopularity[dNumberOfMoves];
  BitBoard vbbWhiteWins[dNumberOfMoves];
  BitBoard vbbBlackWins[dNumberOfMoves];
  BitBoard vbbDraws[dNumberOfMoves];
  BitBoard bbWhiteWins = 0;
  BitBoard bbBlackWins = 0;
  BitBoard bbDraws = 0;
  char strMove[64];
  int iNumberOfChars;
  int iCharCount;
  int iNumberOfMoves;
  double dPercentWhiteWins = 0;
  double dPercentBlackWins = 0;
  double dPercentDraws = 0;
  BitBoard bbTotalGames = 0;

  // Calculate the moves for this position.
  CalculateMoves(vsMoveList, argsBoard, argsGeneralMoves);

  iNumberOfMoves = argsBoard->siNumberOfMoves;

  cout << endl;

  for (iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++) {

    // Make the move.
    MakeMove(vsMoveList, argsBoard, argsGeneralMoves, iMoveCount);

    // Get the statistics.
    ExtractOpeningBookStats(&gsOpeningBook, bbWhiteWins, bbBlackWins, bbDraws,
                            argsBoard, argsGeneralMoves);

    // Store the stats for this move
    vbbWhiteWins[iMoveCount] = bbWhiteWins;
    vbbBlackWins[iMoveCount] = bbBlackWins;
    vbbDraws[iMoveCount] = bbDraws;

    // Sum the total games.
    bbTotalGames += bbWhiteWins;
    bbTotalGames += bbBlackWins;
    bbTotalGames += bbDraws;

    // Undo the move.
    UndoMove(argsBoard, argsGeneralMoves);

    // Collect the popularity.
    viPopularity[iMoveCount] = (int)(bbWhiteWins + bbBlackWins + bbDraws);
    viSortOrder[iMoveCount] = iMoveCount;
  }

  // Sort on the popularity
  int iSortFlag = 1;
  while (iSortFlag) {

    // Set the default to bail.
    iSortFlag = 0;

    // Use a cocktail sort and to from top to bottom
    for (int iMoveIndex = 0; iMoveIndex < iNumberOfMoves - 1; iMoveIndex++) {

      if (viPopularity[iMoveIndex + 1] > viPopularity[iMoveIndex]) {

        int iDummyScore = viPopularity[iMoveIndex];
        viPopularity[iMoveIndex] = viPopularity[iMoveIndex + 1];
        viPopularity[iMoveIndex + 1] = iDummyScore;

        int iDummyPosition = viSortOrder[iMoveIndex];
        viSortOrder[iMoveIndex] = viSortOrder[iMoveIndex + 1];
        viSortOrder[iMoveIndex + 1] = iDummyPosition;

        iSortFlag = 1;
      }
    }
  }

  // This loop is for printing out.
  for (iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++) {

    // Only show non-zero popularities.
    if (viPopularity[iMoveCount] > 0) {

      // Get the index of this move
      int idx = viSortOrder[iMoveCount];

      // Print out the move name.
      iNumberOfChars = PrintMove(argsBoard, argsGeneralMoves,
                                 &vsMoveList[idx], strMove);

      // Write out the move
      for (iCharCount = 0; iCharCount < iNumberOfChars; iCharCount++) {
        cout << strMove[iCharCount];
      }

      // Write out spaces to line up the output.
      for (iCharCount = 0; iCharCount < 7 - iNumberOfChars; iCharCount++) {
        cout << " ";
      }

      // Format and print the popularity
      cout << FormatWithCommas((long long)viPopularity[iMoveCount]);

      // Calculate percentages
      BitBoard total = vbbWhiteWins[idx] + vbbBlackWins[idx] + vbbDraws[idx];
      if (total > 0) {
        dPercentWhiteWins = (vbbWhiteWins[idx] * 100.0) / total;
        dPercentBlackWins = (vbbBlackWins[idx] * 100.0) / total;
        dPercentDraws = (vbbDraws[idx] * 100.0) / total;

        cout << " (W:" << std::fixed << std::setprecision(1) << dPercentWhiteWins << "% "
             << "D:" << dPercentDraws << "% "
             << "L:" << dPercentBlackWins << "%)";
      }

      cout << endl;
    }
  }

  // Show total games in the book with commas
  cout << "Total games in book = " << FormatWithCommas((long long)bbTotalGames) << endl << endl;

  // Close the book if no games are found.
  if (bbTotalGames == 0) {
    SetIsInBook(dNo);
  }
}

//
//----------------------------------------------------------------------------------------------------------
//
// This function returns the actual move in the argument argsBestMove.
// The argtiBookMove is -1 if no book move was found.
void FindBookMove(struct Board *argsBoard, struct GeneralMove *argsGeneralMoves,
                  struct Move *argsvMoveList, struct Move *argsBestMove,
                  int *argiBookMove) {

  // Debug the inputs.
  assert(argsBoard >= 0);
  assert(argsGeneralMoves >= 0);

  // Print the statistics from the open book for the moves from a given
  // position.
  BitBoard bbWhiteWins = 0;
  BitBoard bbBlackWins = 0;
  BitBoard bbDraws = 0;
  int iNumberOfMoves;
  int iMoveCount;
  int iWins = 0;
  int iTotalGames = 0;
  int iNumberOfFeasibleMoves = 0;
  double dPercentNotLoss = 0;
  double vdPercentWhiteWins[dNumberOfMoves];
  double vdPercentBlackWins[dNumberOfMoves];
  double vdPercentDraws[dNumberOfMoves];
  double vCumPopularity[dNumberOfMoves];
  int viTotalGames[dNumberOfMoves];
  int viMoveList[dNumberOfMoves];
  char strMove[64];

  // Calculate the moves for this position.
  CalculateMoves(argsvMoveList, argsBoard, argsGeneralMoves);

  // Some Debugging.
  if (GetInterfaceBookDebug()) {

    gofDebugBook << "Hash at start = " << GetHash() << endl << endl;
  }

  // Extract the number of moves.
  iNumberOfMoves = argsBoard->siNumberOfMoves;

  // Some Debugging.
  if (GetInterfaceBookDebug()) {

    gofDebugBook << endl;
    gofDebugBook << "Here are a list of the moves." << endl << endl;
  }

  // Loop over the moves.
  for (iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++) {

    // Set the score of the move to zero, as the score here is based on
    // popularity and not the standard move scoring used for searching
    argsvMoveList[iMoveCount].iScore = 0;

    // Make the move.
    MakeMove(argsvMoveList, argsBoard, argsGeneralMoves, iMoveCount);

    // Get the statistics.
    ExtractOpeningBookStats(&gsOpeningBook, bbWhiteWins, bbBlackWins, bbDraws,
                            argsBoard, argsGeneralMoves);

    // Undo the move.
    UndoMove(argsBoard, argsGeneralMoves);

    // Write out the stats in terms of percentages.
    // Only consider moves above the Violet threshold of quality.
    if ((bbWhiteWins + bbBlackWins + bbDraws) > 0) {

      vdPercentWhiteWins[iMoveCount] =
          (double)(bbWhiteWins) /
          ((double)(bbWhiteWins) + (double)(bbBlackWins) + (double)(bbDraws)) *
          100.0;
      vdPercentBlackWins[iMoveCount] =
          (double)(bbBlackWins) /
          ((double)(bbWhiteWins) + (double)(bbBlackWins) + (double)(bbDraws)) *
          100.0;
      vdPercentDraws[iMoveCount] =
          (double)(bbDraws) /
          ((double)(bbWhiteWins) + (double)(bbBlackWins) + (double)(bbDraws)) *
          100.0;

    } else {

      vdPercentWhiteWins[iMoveCount] = 0;
      vdPercentBlackWins[iMoveCount] = 0;
      vdPercentDraws[iMoveCount] = 0;
    }

    // Here are criteria for rejecting a book move.
    viTotalGames[iMoveCount] =
        (int)bbWhiteWins + (int)bbBlackWins + (int)bbDraws;

    // Calculate the total games for this position.
    iTotalGames += viTotalGames[iMoveCount];

    if (argsBoard->siComputerColor == dComputerWhite) {

      iWins = (int)bbWhiteWins;
      dPercentNotLoss =
          vdPercentWhiteWins[iMoveCount] + vdPercentDraws[iMoveCount];
    }
    if (argsBoard->siComputerColor == dComputerBlack) {

      iWins = (int)bbBlackWins;
      dPercentNotLoss =
          vdPercentBlackWins[iMoveCount] + vdPercentDraws[iMoveCount];
    }

    // See if the move is feasibility.
    // Temporary debug: Show all moves with any games.
    if (viTotalGames[iMoveCount] > 0) {
    // if ((viTotalGames[iMoveCount] >= dMoveCutOff) && (iWins >= dWinCutOff) && (dPercentNotLoss >= dNotALoss)) {

      // Update the number of feasible moves.
      iNumberOfFeasibleMoves++;

      // If the move is feasible, set the move score to the total number of
      // games, this will allow the moves to be sorted by popularity.
      argsvMoveList[iMoveCount].iScore = viTotalGames[iMoveCount];

    } else {

      // Make sure the score is zero.
      argsvMoveList[iMoveCount].iScore = 0;
    }

    strncpy(strMove, "      ", 6);
    // Create a book move string.
    CreateAlgebraicMove(strMove, &argsvMoveList[iMoveCount], 0);

    // Some Debugging.
    if (GetInterfaceBookDebug()) {

      gofDebugBook << endl;
      gofDebugBook << "iMoveCount = " << iMoveCount << " Move = " << strMove
                   << " Score = " << argsvMoveList[iMoveCount].iScore << endl;
    }
  }

  // If not feasible, call the book quits.
  if (iNumberOfFeasibleMoves == 0) {

    // Return a book fail.
    *argiBookMove = -1;

    // Mark the board as being out of book.
    // This is imortant in other parts of the code where
    // the function ClearHash() is called.
    SetIsInBook(dNo);

    return;
  }

  // Sort the moves according to popularity.
  SortMoves(viMoveList, argsvMoveList, iNumberOfMoves);

  // Some Debugging.
  if (GetInterfaceBookDebug()) {

    gofDebugBook << endl;
    gofDebugBook << "Here is a list of the sorted Moves." << endl << endl;
  }

  // Also calculate the cumulative popularity of the moves.
  for (iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++) {

    // Calculate the cumulative popularity.
    if (iMoveCount == 0) {

      // Assign the first value;
      vCumPopularity[iMoveCount] =
          (double)(argsvMoveList[viMoveList[iMoveCount]].iScore) /
          (double)(iTotalGames);

    } else {

      // Assign the cumulative values;
      vCumPopularity[iMoveCount] =
          (double)(argsvMoveList[viMoveList[iMoveCount]].iScore) /
              (double)(iTotalGames) +
          vCumPopularity[iMoveCount - 1];
    }

    strncpy(strMove, "      ", 6);
    // Create a book move string.
    CreateAlgebraicMove(strMove, &argsvMoveList[viMoveList[iMoveCount]], 0);

    // Some Debugging.
    if (GetInterfaceBookDebug()) {

      gofDebugBook << endl;
      gofDebugBook << "iMoveCount = " << iMoveCount << " Move = " << strMove
                   << " CumPop = " << vCumPopularity[iMoveCount] << endl;
    }
  }

  // Some QA/QC
  assert(vCumPopularity[iNumberOfMoves - 1] > 0.999);
  assert(vCumPopularity[iNumberOfMoves - 1] < 1.001);

  // Some Debugging.
  if (GetInterfaceBookDebug()) {

    gofDebugBook << "Hash at end = " << GetHash() << endl << endl;
  }

  // Choose the move to make via a random number and normalize to the popularity
  // cutoff. This means that only moves that have a cumulative popularity of 90%
  // will be played. This is done to stop the weirder in the database from being
  // played.
  double dSamplePopularity =
      (double)(rand()) / (double)(RAND_MAX)*dPopularityCutOff;

  // Some Debugging.
  if (GetInterfaceBookDebug()) {

    gofDebugBook << endl;
    gofDebugBook << "Here is the random number = " << dSamplePopularity << endl
                 << endl;
  }

  // Loop over the cumulative popularity to find a move.
  for (int iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++) {

    // Some Debugging.
    if (GetInterfaceBookDebug()) {

      gofDebugBook << endl << "iMoveCount = " << iMoveCount << endl << endl;
    }

    // See if we are freshly over the criteria for a good move.
    // As a redundate check, make sure the move is feasable.
    if (vCumPopularity[iMoveCount] >= dSamplePopularity) {

      // Some Debugging.
      if (GetInterfaceBookDebug()) {

        gofDebugBook << "Now setting the move." << endl;
      }

      *argiBookMove = viMoveList[iMoveCount];
      *argsBestMove = argsvMoveList[*argiBookMove];
      assert(*argiBookMove >= -1);
      assert(*argiBookMove <= dNumberOfMoves);

      gofDebugBook << "argiBooMove = " << *argiBookMove << endl;

      strncpy(strMove, "      ", 6);
      // Create a book move string.
      CreateAlgebraicMove(strMove, argsBestMove, 0);

      // Some Debugging.
      if (GetInterfaceBookDebug()) {

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
//---------------------------------------------------------------------------------------------------------------
//
int FindMaxScore(double *vdWins, int argiNumberOfMoves) {
  int iMoveNumber = -1;
  int iNumberOfMoves;
  int iMaxWins = -1;

  // Loop over the statistics.
  for (iNumberOfMoves = 0; iNumberOfMoves < argiNumberOfMoves;
       iNumberOfMoves++) {

    // See if we have a good score and that the score exists.
    if ((iMaxWins < vdWins[iNumberOfMoves]) && (vdWins[iNumberOfMoves] > 0)) {

      iMaxWins = (int)vdWins[iNumberOfMoves];
      iMoveNumber = iNumberOfMoves;
    }
  }

  // Look for a failure.
  if (iMaxWins <= dWinCutOff) {

    iMoveNumber = -1;
  }

  assert(iMoveNumber >= -1);
  assert(iMoveNumber <= dNumberOfMoves);

  return iMoveNumber;
}

//
//------------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------------------------------------
//
extern std::atomic<long long> g_BookUpdates;
extern std::atomic<long long> g_MergeOps;

void WriteOutOpeningBook(const char *argstrBook, int iPlyIndex, int iPrune) {
   // Write out the opening book to a file.
   
   cout << "Writing Book to " << argstrBook << endl;
   cout << "Total Updates: " << g_BookUpdates << endl;
   cout << "Total Merges: " << g_MergeOps << endl;
   
   // open the output file.
   ofstream ofBook(argstrBook, ofstream::binary);
   if (ofBook.fail()) {
 
     cout << "Output Book.txt file failed to open." << endl;
   }
   
   long long writtenLines = 0;
   long long prunedLines = 0;
   
   // Helper for unpacking
   GeneralMove gm;
   GenerateGeneralMove(&gm);

   // Determine threshold
   int iMinVisits = 1; // Default save everything
   if (iPrune) {
       iMinVisits = dMinBookVisits; // 2
   }

   for (BitBoard bbHashCount = 0; bbHashCount < gsOpeningBook.bbOpeningBookSize;
        bbHashCount++) {
     
     BitBoard element = GetBookElement(bbHashCount);
     if (element != 0) {
       
       // Frequency Check
       BitBoard w = (element & gm.bbWhiteScore) >> gm.iWhiteScoreShift;
       BitBoard b = (element & gm.bbBlackScore) >> gm.iBlackScoreShift;
       BitBoard d = (element & gm.bbDrawScore) >> gm.iDrawScoreShift;
       
       if ((w + b + d) >= iMinVisits) {
           ofBook << GetBookElementHash(bbHashCount) << " "
                  << element << " " << endl;
           writtenLines++;
       } else {
           prunedLines++;
       }
     }
   }
   
   cout << "Book Lines Written: " << writtenLines << " (Pruned: " << prunedLines << ")" << endl;
 
   ofBook.close();
}

//
//----------------------------------------------------------------------------------------------------------
//
void ReadOpeningBook(const char *argstrBookName,
                     struct GeneralMove *argsGeneralMoves) {

  // Debug the inputs.
  assert(argstrBookName >= 0);
  assert(argsGeneralMoves >= 0);

  // Delcare some variables.
  int iReadMoreFlag = 1;
  BitBoard bbKey;
  BitBoard bbData;
  BitBoard bbHash;

  // Read the opening book.
  ifstream ifBook(argstrBookName, ifstream::binary);
  if (ifBook.fail()) {

    SetUseOpeningBook(dNo);
    return;
    // cout << "Output Book.txt file failed to open." << endl;
  }

  // Loop over the file.
  while (!ifBook.eof()) {

    // Get the key from the file.
    ifBook >> bbHash;

    // Calculate the key.
    bbKey = bbHash & gsOpeningBook.bbMaskIndex;

    assert(bbKey < gsOpeningBook.bbOpeningBookSize);

    // Get the data.
    ifBook >> bbData;

    // Put the book in the hash table.
    SetBookElement(bbKey, bbData);

    SetBookElementHash(bbKey, bbHash);

    // Count the number of Positions.
    gsOpeningBook.bbNumberOfPositionsInBook++;
  }

  if (GetInterfaceMode() != dUCI) {
    cout << "Reading Book Complete. Positions: " << gsOpeningBook.bbNumberOfPositionsInBook << endl;
  }

  ifBook.close();
}

//
//------------------------------------------------------------------------------------------------------------------
//
void TrimOpeningBook(struct GeneralMove *argsGeneralMoves, int iMinVisits) {

  // Debug the inputs.
  assert(argsGeneralMoves >= 0);

  // This funtion is used to get rid of positions that haven't been reached at
  // least a given amount of time.

  // Use the default if the input is zero.
  if (iMinVisits <= 0) {
    iMinVisits = dBookCutOff;
  }

  for (BitBoard bbHashIndex = 0; bbHashIndex < gsOpeningBook.bbOpeningBookSize;
       bbHashIndex++) {

    // Find the number of positions.
    BitBoard bbCountWhite =
        (GetBookElement(bbHashIndex) & argsGeneralMoves->bbWhiteScore) >>
        argsGeneralMoves->iWhiteScoreShift;
    BitBoard bbCountBlack =
        (GetBookElement(bbHashIndex) & argsGeneralMoves->bbBlackScore) >>
        argsGeneralMoves->iBlackScoreShift;
    BitBoard bbCountDraw =
        (GetBookElement(bbHashIndex) & argsGeneralMoves->bbDrawScore) >>
        argsGeneralMoves->iDrawScoreShift;
    BitBoard bbCount = bbCountWhite + bbCountBlack + bbCountDraw;

    // Use the cut off level
    if (bbCount < iMinVisits) {

      SetBookElement(bbHashIndex, 0);
    }
  }
}

void RunBookTrimming(struct GeneralMove *argsGeneralMoves, int iMinVisits) {

  // Debug the inputs.
  assert(argsGeneralMoves >= 0);

  // Read the opening book.
  cout << "Reading Book.txt..." << endl;
  ReadOpeningBook("Book.txt", argsGeneralMoves);
  cout << "Book Read. Positions: " << gsOpeningBook.bbNumberOfPositionsInBook
       << endl;

  // Trim the book.
  cout << "Trimming Book with min visits: " << iMinVisits << "..." << endl;
  TrimOpeningBook(argsGeneralMoves, iMinVisits);

  // Write out the trimmed book.
  // We use 0 for prune since we already trimmed it in memory.
  cout << "Writing TrimmedBook.txt..." << endl;
  WriteOutOpeningBook("TrimmedBook.txt", 0, 0);
  cout << "Book Trimming Complete." << endl;
}

//
//----------------------------------------------------------------------------------------------------------------------------
//
void AppendBook(struct GeneralMove *argsGeneralMoves,
                const char *argstrBookName) {

  // Debug the inputs.
  assert(argstrBookName >= 0);
  assert(argsGeneralMoves >= 0);

  BitBoard bbHash = 0;
  BitBoard bbKey = 0;
  BitBoard bbData = 0;

  // Read the second opening book and add it in to the first.
  ifstream ifBook(argstrBookName, ifstream::binary);
  if (ifBook.fail()) {

    cout << "Input Book.txt file failed to open." << endl;
  }

  // Loop over the file.
  while (!ifBook.eof()) {

    // Get the key from the file.
    ifBook >> bbHash;

    // Calculate the key.
    bbKey = bbHash & gsOpeningBook.bbMaskIndex;

    assert(bbKey < gsOpeningBook.bbOpeningBookSize);

    // Get the data.
    ifBook >> bbData;

    // Combine the counts here.

    // Extract the current White count.
    BitBoard bbCountWhite1 =
        (GetBookElement(bbKey) & argsGeneralMoves->bbWhiteScore) >>
        argsGeneralMoves->iWhiteScoreShift;

    BitBoard bbCountWhite2 = (bbData & argsGeneralMoves->bbWhiteScore) >>
                             argsGeneralMoves->iWhiteScoreShift;

    BitBoard bbCountWhite = bbCountWhite1 + bbCountWhite2;

    // Extract the current Black count.
    BitBoard bbCountBlack1 =
        (GetBookElement(bbKey) & argsGeneralMoves->bbBlackScore) >>
        argsGeneralMoves->iBlackScoreShift;

    BitBoard bbCountBlack2 = (bbData & argsGeneralMoves->bbBlackScore) >>
                             argsGeneralMoves->iBlackScoreShift;

    BitBoard bbCountBlack = bbCountBlack1 + bbCountBlack2;

    // Extract the current Draw count.
    BitBoard bbCountDraw1 =
        (GetBookElement(bbKey) & argsGeneralMoves->bbDrawScore) >>
        argsGeneralMoves->iDrawScoreShift;

    BitBoard bbCountDraw2 = (bbData & argsGeneralMoves->bbDrawScore) >>
                            argsGeneralMoves->iDrawScoreShift;

    BitBoard bbCountDraw = bbCountDraw1 + bbCountDraw2;

    // Input the data back into the hash.
    SetBookElement(bbKey, 0);

    bbCountWhite = bbCountWhite << argsGeneralMoves->iWhiteScoreShift;

    bbCountBlack = bbCountBlack << argsGeneralMoves->iBlackScoreShift;

    bbCountDraw = bbCountDraw << argsGeneralMoves->iDrawScoreShift;

    BitBoard bbElement = GetBookElement(bbKey);

    bbElement |= bbCountWhite;

    bbElement |= bbCountBlack;

    bbElement |= bbCountDraw;

    SetBookElement(bbKey, bbElement);
  }

  // Close the input books
  ifBook.close();
}

//
//----------------------------------------------------------------------------------------
//
//
//----------------------------------------------------------------------------------------
//
void ExtractOpeningBookStats(OpeningBook *book, BitBoard &argbbWhiteWins,
                             BitBoard &argbbBlackWins, BitBoard &argbbDraws,
                             struct Board *argsBoard,
                             struct GeneralMove *argsGeneralMoves) {

  // Debug the inputs.
  assert(argsGeneralMoves >= 0);

  // Calculate the key to the hash table.
  BitBoard bbKey = argsBoard->bbHash & book->bbMaskIndex;
  BitBoard bbHash = GetBookElementHash(book, bbKey);
  BitBoard bbElement = GetBookElement(book, bbKey);
  BitBoard bbHashFromTable = argsBoard->bbHash;

  if (bbHash != bbHashFromTable) {
     if (bbHash != 0) {
        // cout << "MISMATCH: BoardHash=" << bbHashFromTable << " BookHash=" << bbHash << " Mask=" << book->bbMaskIndex << " Key=" << bbKey << endl;
     }
  } else {
     // cout << "MATCH: Hash=" << bbHashFromTable << endl;
  }

  // Check for a collison.
  // If the stored hash is zero, then they match by default!
  // if ( ( bbHash != bbHashFromTable ) &&
  //      ( bbHash != 0 ) )
  if (bbHash != bbHashFromTable) {
    // Debug Mismatch details
    if (bbHash != 0) {
         // cout << "MISMATCH: BoardHash=" << bbHashFromTable << " BookHash=" << bbHash << " Mask=" << book->bbMaskIndex << " Key=" << bbKey << endl;
    }

    argbbWhiteWins = 0;
    argbbBlackWins = 0;
    argbbDraws = 0;

  } else {
    // Debug Match
    // cout << "MATCH FOUND! Key=" << bbKey << " Element=" << bbElement << " WMask=" << argsGeneralMoves->bbWhiteScore << endl;

    // Extract the current White count.
    argbbWhiteWins =
        (GetBookElement(book, bbKey) & argsGeneralMoves->bbWhiteScore) >>
        argsGeneralMoves->iWhiteScoreShift;

    // Extract the current Black count.
    argbbBlackWins =
        (GetBookElement(book, bbKey) & argsGeneralMoves->bbBlackScore) >>
        argsGeneralMoves->iBlackScoreShift;

    // Extract the current Draw count.
    argbbDraws =
        (GetBookElement(book, bbKey) & argsGeneralMoves->bbDrawScore) >>
        argsGeneralMoves->iDrawScoreShift;
  }
}

// Wrapper for existing calls (that use global book)
void ExtractOpeningBookStats(BitBoard &argbbWhiteWins, BitBoard &argbbBlackWins,
                             BitBoard &argbbDraws,
                             struct GeneralMove *argsGeneralMoves,
                             struct Board *argsBoard) {
  
  OpeningBook *book = &gsOpeningBook;
  
  // Use the board's hash if provided, otherwise fallback to global (legacy support if needed, but we should fix calls)
  BitBoard currentHash = argsBoard ? argsBoard->bbHash : GetHash();
  BitBoard bbKey = currentHash & book->bbMaskIndex;

  BitBoard bbHash = GetBookElementHash(book, bbKey);
  BitBoard bbHashFromTable = currentHash;

  if (bbHash != bbHashFromTable) {
    if (argsBoard) {
         // Debug mismatch only if we are using a specific board (Explore context)
         // cout << "MISMATCH: BoardHash=" << bbHashFromTable << " BookHash=" << bbHash << " Mask=" << book->bbMaskIndex << " Key=" << bbKey << endl;
    }
    argbbWhiteWins = 0;
    argbbBlackWins = 0;
    argbbDraws = 0;
  } else {
    argbbWhiteWins =
        (GetBookElement(book, bbKey) & argsGeneralMoves->bbWhiteScore) >>
        argsGeneralMoves->iWhiteScoreShift;
    argbbBlackWins =
        (GetBookElement(book, bbKey) & argsGeneralMoves->bbBlackScore) >>
        argsGeneralMoves->iBlackScoreShift;
    argbbDraws =
        (GetBookElement(book, bbKey) & argsGeneralMoves->bbDrawScore) >>
        argsGeneralMoves->iDrawScoreShift;
  }
}

void UpdateOpeningBook(OpeningBook *book, struct Board *argsBoard,
                       struct GeneralMove *argsGeneralMoves, int iGameResult) {

  // Debug the inputs.
  assert(argsBoard >= 0);
  assert(argsGeneralMoves >= 0);
  assert(iGameResult >= -1);
  assert(iGameResult <= 2);

  // Declare some variables.
  BitBoard bbCount = 0;
  int iBitCount;

  // Input the data to hash table.
  BitBoard bbKey = argsBoard->bbHash & book->bbMaskIndex;

  // Look for a collision.
  BitBoard bbHash = argsBoard->bbHash;
  BitBoard bbFullHash = GetBookElementHash(book, bbKey);

  if (bbFullHash == 0) {
    // This is a new position.
    SetBookElementHash(book, bbKey, bbHash);
    
    // Increment Book Count
    book->bbNumberOfPositionsInBook++;
  } else if (bbHash != bbFullHash) {
    // If a collision, don't store the data.
    return;
  }



// Input the data to hash table.
// ... (existing code)

  // Put the opening book into the hash table memory.
  switch (iGameResult) {

  case dWhiteWin: {
    g_BookUpdates++;
    // Extract the current White count.

    bbCount = (GetBookElement(book, bbKey) & argsGeneralMoves->bbWhiteScore) >>
              argsGeneralMoves->iWhiteScoreShift;

    // Increment the white win count.
    bbCount++;

    // Zero out the current count.
    BitBoard bbElement = GetBookElement(book, bbKey);
    for (iBitCount = 0; iBitCount < dNumberOfBitsPerScore; iBitCount++) {

      bbElement = SetBitToZero(bbElement, iBitCount);
    }

    // Calculate the depth and enter it.
    bbCount = bbCount << argsGeneralMoves->iWhiteScoreShift;
    bbElement |= bbCount;
    SetBookElement(book, bbKey, bbElement);

    break;
  }
  case dBlackWin: {

    // Extract the current Black count.
    bbCount = (GetBookElement(book, bbKey) & argsGeneralMoves->bbBlackScore) >>
              argsGeneralMoves->iBlackScoreShift;

    // Increment the Black win count.
    bbCount++;

    // Zero out the current count.
    BitBoard bbElement = GetBookElement(book, bbKey);
    for (iBitCount = dNumberOfBitsPerScore;
         iBitCount < 2 * dNumberOfBitsPerScore; iBitCount++) {

      bbElement = SetBitToZero(bbElement, iBitCount);
    }

    // Calculate the depth and enter it.
    bbCount = bbCount << argsGeneralMoves->iBlackScoreShift;
    bbElement |= bbCount;
    SetBookElement(book, bbKey, bbElement);

    break;
  }
  case dDraw: {

    // Extract the current Draw count.
    bbCount = (GetBookElement(book, bbKey) & argsGeneralMoves->bbDrawScore) >>
              argsGeneralMoves->iDrawScoreShift;

    // Increment the Draw win count.
    bbCount++;

    // Zero out the current count.
    BitBoard bbElement = GetBookElement(book, bbKey);
    for (iBitCount = 2 * dNumberOfBitsPerScore;
         iBitCount < 3 * dNumberOfBitsPerScore; iBitCount++) {

      bbElement = SetBitToZero(bbElement, iBitCount);
    }

    // Calculate the depth and enter it.
    bbCount = bbCount << argsGeneralMoves->iDrawScoreShift;
    bbElement |= bbCount;
    SetBookElement(book, bbKey, bbElement);
    break;
  }
  case dUnknown: {
    // In this case do nothing.
    break;
  }
  }
}

// Wrapper
void UpdateOpeningBook(struct Board *argsBoard,
                       struct GeneralMove *argsGeneralMoves, int iGameResult) {
  UpdateOpeningBook(&gsOpeningBook, argsBoard, argsGeneralMoves, iGameResult);
}

//
//------------------------------------------------------------------------------------------
//
void StartCheckBook(struct Board *argsBoard,
                    struct GeneralMove *argsGeneralMoves) {

  // Set the counters to zero.
  giTotalCount = 0;
  giMarginalCount = 0;
  giCutCount = 0;

  // Open the file for output of the checked book.
  // ifstream ifBook( argstrBookName, ifstream::binary );
  gofCheckedBook.open("CheckedBook.txt", ofstream::binary);
  if (gofCheckedBook.fail()) {

    cout << "CheckedBook.txt failed to open." << endl;
    system("Pause");
    return;
  }

  CheckBook(argsBoard, argsGeneralMoves);

  // Close the checked book.
  gofCheckedBook.close();
}

//
//-------------------------------------------------------------------------------------------
//
/*
void CheckBook( struct Board * argsBoard,
                struct GeneralMove * argsGeneralMoves,
                ofstream * argofCheckedBook )
*/
void CheckBook(struct Board *argsBoard, struct GeneralMove *argsGeneralMoves) {

  // Debug the inputs.
  assert(argsBoard >= 0);
  assert(argsGeneralMoves >= 0);
  assert(CheckBoard(argsBoard));

  // This function is called recursively to check the validity of the book.
  int viPopularMoves[dNumberOfMoves];
  int iNumberOfMoves;
  int iNumberOfPopularMoves;
  Move *vsMoveList;
  vsMoveList = (Move *)malloc(dNumberOfMoves * sizeof(Move));
  int iScore = 0;
  int iAlpha = dAlpha;
  int iBeta = dBeta;
  BitBoard bbKey = 0;
  int iMoveCount;

  // Calculate the moves for this position.
  CalculateMoves(vsMoveList, argsBoard, argsGeneralMoves);

  // Put the number of moves into a local variable.
  iNumberOfMoves = argsBoard->siNumberOfMoves;

  // Get the popular moves for this board.
  GetPopularMoves(argsBoard, argsGeneralMoves, viPopularMoves,
                  &iNumberOfPopularMoves, vsMoveList, iNumberOfMoves);

  // If the end of the book, return.
  if ((iNumberOfPopularMoves == 0) || // skip if the end of the book
      (LookForDraw(
          argsBoard,
          argsGeneralMoves)) || // skip if we have seen this position before
      ((argsGeneralMoves->bbVerified &
        GetBookElement(GetKey())))) // skip if we have already verified this.
  {

    free(vsMoveList);
    return;
  }

  // Update the counters and publish.
  giTotalCount++;
  giMarginalCount++;
  if (giMarginalCount > 9) {


    giMarginalCount = 0;
  }

  ///*
  // Perform a search.
  iScore = StartSearch(argsBoard, argsGeneralMoves, iAlpha, iBeta);
  //*/

  // If the score is good, continue and write to checked book file.  If not,
  // return.
  if (iScore < 0) {

    iScore = -iScore;
  }
  if (iScore < dOpeningBookScoreCutOff) {

    // This is a good position and we will continue.
    gofCheckedBook << GetBookElementHash(GetKey()) << " "
                   << GetBookElement(GetKey()) << " " << endl;

  } else {

    // If we fail, to a research to a deeper depth.
    SetSearchDepth(
        2 *
        dOpeningBookVerificationSearchDepth); // dInfiniteDepth
                                              // dOpeningBookVerificationSearchDepth

    SetSearchTimeInMiliSeconds(
        dTenMinutes); // dInfiniteTime dOneSecond dTwentySeconds dOneMinute
                      // dTenMinutes

    // Perform a search.
    iScore = StartSearch(argsBoard, argsGeneralMoves, iAlpha, iBeta);

    // Set the initial parameters for controling the game
    SetSearchDepth(
        dOpeningBookVerificationSearchDepth); // dInfiniteDepth
                                              // dOpeningBookVerificationSearchDepth

    SetSearchTimeInMiliSeconds(
        dInfiniteTime); // dInfiniteTime dOneSecond dTwentySeconds dOneMinute
                        // dTenMinutes

    // If the score is good, continue and write to checked book file.  If not,
    // return.
    if (iScore < 0) {
      iScore = -iScore;
    }
    if (iScore < dOpeningBookScoreCutOff) {

      // This is a good position and we will continue.
      gofCheckedBook << GetBookElementHash(GetKey()) << " "
                     << GetBookElement(GetKey()) << " " << endl;

    } else {

      // The score didn't pass the cutoff.  Baill out.
      giCutCount++;
      PrintBoard(argsBoard->mBoard);
      PrintFEN(argsBoard, argsGeneralMoves);
      int iEvalScore = EvaluateBoard(argsBoard, argsGeneralMoves);
      PrintPrincipalVariation(argsBoard, argsGeneralMoves);
      // cout << "iScore = " << iScore << " iEval = " << iEvalScore << endl <<
      // endl; system( "Pause" );
      free(vsMoveList);
      return;
    }
  }

  // If we made it here we are verified.
  // Mark the position as having been checked.
  BitBoard bbElement = GetBookElement(GetKey());
  BitBoard bbHash = GetBookElementHash(GetKey());

  ///*
  // See if the bit is already set to zero.
  if (!(argsGeneralMoves->bbVerified & bbElement)) {

    bbElement = SetBitToOne(bbElement, 60);
  }
  //*/
  ///*
  // Put the element back in the book.
  SetBookElement(GetKey(), bbElement);
  //*/

  // Loop over the moves
  for (iMoveCount = 0; iMoveCount < iNumberOfPopularMoves; iMoveCount++) {

    if (argsBoard->iMoveHistory == -1) {

      if (iMoveCount == 0) {

        giTotalCount = 0;
        giMarginalCount = 0;
      }


    }

    // Make the move.
    MakeMove(vsMoveList, argsBoard, argsGeneralMoves,
             viPopularMoves[iMoveCount]);

    CheckBook(argsBoard, argsGeneralMoves);

    // Undo the move.
    UndoMove(argsBoard, argsGeneralMoves);
  }

  free(vsMoveList);

  // put in a final loop here to take out all unverified elemensts.
}

//
//----------------------------------------------------------------------------------------------------------
//
void GetPopularMoves(struct Board *argsBoard,
                     struct GeneralMove *argsGeneralMoves, int *viPopularMoves,
                     int *iNumberOfPopularMoves, struct Move *vsMoveList,
                     int iNumberOfMoves)

{

  // Debug the inputs.
  assert(argsBoard >= 0);
  assert(argsGeneralMoves >= 0);

  // Print the statistics from the open book for the moves from a given
  // position.
  int viSortOrder[dNumberOfMoves];
  int viPopularity[dNumberOfMoves];
  BitBoard bbWhiteWins = 0;
  BitBoard bbBlackWins = 0;
  BitBoard bbDraws = 0;
  double dPercentWhiteWins = 0;
  double dPercentBlackWins = 0;
  double dPercentDraws = 0;
  int iMoveCount = 0;

  // Reset the count of populare moves.
  *iNumberOfPopularMoves = 0;

  // Loop over the moves
  for (iMoveCount = 0; iMoveCount < iNumberOfMoves; iMoveCount++) {

    // Make the move.
    MakeMove(vsMoveList, argsBoard, argsGeneralMoves, iMoveCount);

    // Get the statistics.
    ExtractOpeningBookStats(bbWhiteWins, bbBlackWins, bbDraws, argsGeneralMoves, argsBoard);

    // Undo the move.
    UndoMove(argsBoard, argsGeneralMoves);

    // Collect the popularity.
    viPopularity[iMoveCount] = (int)(bbWhiteWins + bbBlackWins + bbDraws);
    viSortOrder[iMoveCount] = iMoveCount;

    // Calculate the number of populare moves.
    if (viPopularity[iMoveCount] > 0) {

      (*iNumberOfPopularMoves)++;
    }
  }

  // Sort on the popularity
  int iSortFlag = 1;
  while (iSortFlag) {

    // Set the default to bail.
    iSortFlag = 0;

    // Use a cocktail sort and to from top to bottom
    for (int iMoveIndex = 0; iMoveIndex < iNumberOfMoves - 1; iMoveIndex++) {

      if (viPopularity[iMoveIndex + 1] > viPopularity[iMoveIndex]) {

        int iDummyScore = viPopularity[iMoveIndex];
        viPopularity[iMoveIndex] = viPopularity[iMoveIndex + 1];
        viPopularity[iMoveIndex + 1] = iDummyScore;

        int iDummyPosition = viSortOrder[iMoveIndex];
        viSortOrder[iMoveIndex] = viSortOrder[iMoveIndex + 1];
        viSortOrder[iMoveIndex + 1] = iDummyPosition;

        iSortFlag = 1;
      }
    }
  }

  // Extract the populare moves
  for (int iMoveIndex = 0; iMoveIndex < *iNumberOfPopularMoves; iMoveIndex++) {

    viPopularMoves[iMoveIndex] = viSortOrder[iMoveIndex];
  }
}

//
//------------------------------------------------------------------------------------------------------------------------
//
int GetNumberOfPositionsInOpeningBook() {
  return (int)(gsOpeningBook.bbNumberOfPositionsInBook);
}

//
//------------------------------------------------------------------------------------------------------------------------
//
int GetNumberOfPositionsVerified() {
  return (int)(gsOpeningBook.bbNumberOfPositionsVerified);
}

//
//------------------------------------------------------------------------------------------------------------------------
//

void InitializeBookDebug() {

  // Open some debugging files.
  gofDebugBook.open("BookInterfaceLog.txt", ios::out | ios::app);
  gofDebugBook << endl;
  gofDebugBook << "Book log started." << endl << endl;
}

//
//---------------------------------------------------------------------------------------------------------
//
void CloseBookDebug() {

  // Close down the communications.
  gofDebugBook << "Closing file." << endl;
  gofDebugBook << endl;
  gofDebugBook.close();
}
