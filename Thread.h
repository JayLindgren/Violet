#ifndef THREAD_H
#define THREAD_H

#include "Definitions.h"
#include "Structures.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

// Forward declarations
struct Board;
struct GeneralMove;

// Data specific to a single search thread
struct ThreadData
{
   int   id;
   Board board; // Thread-local board copy

   // Thread-local move lists (replacing global gvsMoveList etc.)
   // We use pointers to arrays to match existing usage, but allocated per
   // thread.
   Move ( *vsMoveList )[ dNumberOfMoves ];
   int ( *vsiMoveOrder )[ dNumberOfMoves ];
   int ( *viMoveScore )[ dNumberOfMoves ];

   SearchParameters
       searchParameters; // Thread-local search parameters (History, Killers)

   // Search statistics for this thread
   long long nodesSearched;

   // Thread-local RNG state
   unsigned int rngState;

   ThreadData();
   ~ThreadData();
};

class Thread
{
public:
   Thread( int argid );
   ~Thread();

   void StartSearch( Board *argsBoard,
                     GeneralMove *argsGeneralMoves,
                     int argiAlpha,
                     int argiBeta,
                     int argiDepth );
   void Join();

   // Check if thread is currently searching
   bool IsSearching() const
   {
      return mIsSearching;
   }

   // Get thread data
   ThreadData *GetData()
   {
      return mData;
   }

private:
   void SearchLoop(); // Main loop for the thread

   ThreadData       *mData;
   std::thread      *mThread;
   std::atomic<bool> mIsSearching;
   std::atomic<bool> mStopRequest;

   // Parameters for the current search
   Board       mSearchBoard; // Copy of board to search
   GeneralMove mSearchGeneralMoves;
   int         mAlpha;
   int         mBeta;
   int         mDepth;

   std::mutex              mMutex;
   std::condition_variable mCV;
};

// Global thread pool
extern std::vector<Thread *> gThreads;

// Global atomic node counter for fast access
// extern std::atomic<long long> gTotalNodesSearched;

// Function to initialize/resize thread pool
void InitializeThreads( int argiCount );
void DestroyThreads();
long long GetTotalNodes();
void PrintPerformanceStats();

#endif // THREAD_H
