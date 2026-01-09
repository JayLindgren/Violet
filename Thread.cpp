#include "Thread.h"
#include "Functions.h"
#include <iostream>

std::vector<Thread *> gThreads;
// std::atomic<long long> gTotalNodesSearched(0);

ThreadData::ThreadData()
{
   id            = -1;
   nodesSearched = 0;

   // Allocate memory for move lists
   vsMoveList   = new Move[ dNumberOfPlys ][ dNumberOfMoves ];
   vsiMoveOrder = new int[ dNumberOfPlys ][ dNumberOfMoves ];
   viMoveScore  = new int[ dNumberOfPlys ][ dNumberOfMoves ];

   rngState = 12345; // Default seed, will be updated with ID
}

ThreadData::~ThreadData()
{
   if ( vsMoveList )
      delete[] vsMoveList;
   if ( vsiMoveOrder )
      delete[] vsiMoveOrder;
   if ( viMoveScore )
      delete[] viMoveScore;
}

Thread::Thread( int argid )
{
   mData     = new ThreadData();
   mData->id = argid;
   // Seed RNG with ID and some constants to ensure diversity
   mData->rngState = 12345 + ( argid * 67890 );
   if ( mData->rngState == 0 )
      mData->rngState = 1; // Xorshift must not be 0
   mIsSearching = false;
   mStopRequest = false;
   mThread      = nullptr; // We might launch the thread in StartSearch or have a
                           // persistent loop.
   // For Lazy SMP, persistent loop is better.

   mThread = new std::thread( &Thread::SearchLoop, this );
}

Thread::~Thread()
{
   {
      std::lock_guard<std::mutex> lock( mMutex );
      mStopRequest = true;
   }
   mCV.notify_one();

   if ( mThread && mThread->joinable() )
   {
      mThread->join();
   }
   delete mThread;
   delete mData;
}

void Thread::StartSearch( Board *argsBoard, GeneralMove *argsGeneralMoves,
                          int argiAlpha, int argiBeta, int argiDepth )
{
   // Debug print
   // std::cout << "Thread::StartSearch id=" << m_data->id << " depth=" << depth
   // << std::endl;
   if ( argiDepth > 100 )
      std::cout << "WARNING: Thread::StartSearch depth=" << argiDepth
                << std::endl;
   // Wait if already searching
   while ( mIsSearching )
   {
      std::this_thread::yield();
   }

   {
      std::lock_guard<std::mutex> lock( mMutex );

      // Copy board and parameters
      CopyBoard( &mSearchBoard, argsBoard );
      memcpy( &mSearchGeneralMoves, argsGeneralMoves, sizeof( GeneralMove ) );

      mSearchBoard.iMaxPlys = argiDepth; // CRITICAL FIX: Set the search depth for the helper thread

      mAlpha = argiAlpha;
      mBeta  = argiBeta;
      mDepth = argiDepth;

      mIsSearching = true;
   }
   mCV.notify_one();
}

void Thread::SearchLoop()
{
   while ( true )
   {
      std::unique_lock<std::mutex> lock( mMutex );
      mCV.wait( lock, [ this ]
                { return mIsSearching || mStopRequest; } );

      if ( mStopRequest && !mIsSearching )
         break;

      if ( mIsSearching )
      {
         // Lazy SMP: Do iterative deepening like main thread
         // Each helper thread searches same position independently
         for ( int iDepth = 0; iDepth < mDepth && GetStopGo() == dGo; iDepth++ )
         {
            mSearchBoard.iMaxPlys      = iDepth;
            mSearchBoard.iNumberOfPlys = -1;

            // Perform search at this depth
            int iAlpha            = mAlpha;
            int iBeta             = mBeta;
            int iBestMove         = -1;
            int iBestMoveSearched = 0;

            int iScore = FirstSearch( &mSearchBoard, &mSearchGeneralMoves, iAlpha,
                                      iBeta, &iBestMove, &iBestMoveSearched, mData );

            // Update shared PV if we found a better move
            if ( mSearchBoard.vmPrincipalVariation[ 0 ][ 0 ].iFromSquare >= 0 )
            {
               UpdateSharedPV( mSearchBoard.vmPrincipalVariation[ 0 ], iScore, iDepth );
            }
         }

         mIsSearching = false;
      }
   }
}

void InitializeThreads( int argiCount )
{
   if ( GetInterfaceMode() != dUCI )
   {
      //      std::cout << "InitializeThreads: Requested count=" << argiCount << std::endl;
      //      std::cout << "InitializeThreads: Hardware concurrency="
      //                << std::thread::hardware_concurrency() << std::endl;
   }

   DestroyThreads();

   // Create N-1 helper threads (main thread is thread 0)
   int iSuccessCount = 0;
   for ( int i = 1; i < argiCount; i++ )
   {
      try
      {
         //         if ( GetInterfaceMode() != dUCI )
         //            std::cout << "InitializeThreads: Creating thread " << i << "...";
         gThreads.push_back( new Thread( i ) );
         iSuccessCount++;
         //         if ( GetInterfaceMode() != dUCI )
         //            std::cout << " SUCCESS" << std::endl;
      }
      catch ( const std::exception &e )
      {
         if ( GetInterfaceMode() != dUCI )
            std::cerr << " FAILED: " << e.what() << std::endl;
      }
   }
   if ( GetInterfaceMode() != dUCI )
   {
      //      std::cout << "InitializeThreads: Created " << iSuccessCount
      //                << " helper threads (total threads: " << ( iSuccessCount + 1 ) << ")"
      //                << std::endl;
      std::cout.flush();
   }
}

void DestroyThreads()
{
   for ( auto thread : gThreads )
   {
      delete thread;
   }
   gThreads.clear();
}

long long GetTotalNodes()
{
   // Aggregate counters from all threads
   long long iTotal = 0;
   if ( gMainThreadData )
   {
      iTotal += gMainThreadData->nodesSearched;
   }
   for ( auto *thread : gThreads )
   {
      if ( thread && thread->GetData() )
      {
         iTotal += thread->GetData()->nodesSearched;
      }
   }
   return iTotal;
}

void PrintPerformanceStats()
{
   long long iTotal = 0;
   if ( gMainThreadData )
   {
      std::cout << "Thread 0 (Main): " << gMainThreadData->nodesSearched
                << " nodes" << std::endl;
      iTotal += gMainThreadData->nodesSearched;
   }
   for ( size_t i = 1; i < gThreads.size(); ++i )
   {
      long long iNodes = gThreads[ i ]->GetData()->nodesSearched;
      std::cout << "Thread " << i << ": " << iNodes << " nodes" << std::endl;
      iTotal += iNodes;
   }
   std::cout << "Total Nodes: " << iTotal << std::endl;
}
