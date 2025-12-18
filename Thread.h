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
struct ThreadData {
  int id;
  Board board; // Thread-local board copy

  // Thread-local move lists (replacing global gvsMoveList etc.)
  // We use pointers to arrays to match existing usage, but allocated per
  // thread.
  Move (*vsMoveList)[dNumberOfMoves];
  int (*vsiMoveOrder)[dNumberOfMoves];
  int (*viMoveScore)[dNumberOfMoves];

  SearchParameters
      searchParameters; // Thread-local search parameters (History, Killers)

  // Search statistics for this thread
  long long nodesSearched;

  // Thread-local RNG state
  unsigned int rngState;

  ThreadData();
  ~ThreadData();
};

class Thread {
public:
  Thread(int id);
  ~Thread();

  void StartSearch(Board *board, GeneralMove *generalMoves, int alpha, int beta,
                   int depth);
  void Join();

  // Check if thread is currently searching
  bool IsSearching() const { return m_isSearching; }

  // Get thread data
  ThreadData *GetData() { return m_data; }

private:
  void SearchLoop(); // Main loop for the thread

  ThreadData *m_data;
  std::thread *m_thread;
  std::atomic<bool> m_isSearching;
  std::atomic<bool> m_stopRequest;

  // Parameters for the current search
  Board m_searchBoard; // Copy of board to search
  GeneralMove m_searchGeneralMoves;
  int m_alpha;
  int m_beta;
  int m_depth;

  std::mutex m_mutex;
  std::condition_variable m_cv;
};

// Global thread pool
extern std::vector<Thread *> gThreads;

// Global atomic node counter for fast access
// extern std::atomic<long long> gTotalNodesSearched;

// Function to initialize/resize thread pool
void InitializeThreads(int count);
void DestroyThreads();
long long GetTotalNodes();
void PrintPerformanceStats();

#endif // THREAD_H
