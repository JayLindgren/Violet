#include "Thread.h"
#include "Functions.h"
#include <iostream>

std::vector<Thread *> gThreads;
// std::atomic<long long> gTotalNodesSearched(0);

ThreadData::ThreadData() {
  id = -1;
  nodesSearched = 0;

  // Allocate memory for move lists
  vsMoveList = new Move[dNumberOfPlys][dNumberOfMoves];
  vsiMoveOrder = new int[dNumberOfPlys][dNumberOfMoves];
  viMoveScore = new int[dNumberOfPlys][dNumberOfMoves];

  rngState = 12345; // Default seed, will be updated with ID
}

ThreadData::~ThreadData() {
  if (vsMoveList)
    delete[] vsMoveList;
  if (vsiMoveOrder)
    delete[] vsiMoveOrder;
  if (viMoveScore)
    delete[] viMoveScore;
}

Thread::Thread(int id) {
  m_data = new ThreadData();
  m_data->id = id;
  // Seed RNG with ID and some constants to ensure diversity
  m_data->rngState = 12345 + (id * 67890);
  if (m_data->rngState == 0)
    m_data->rngState = 1; // Xorshift must not be 0
  m_isSearching = false;
  m_stopRequest = false;
  m_thread = nullptr; // We might launch the thread in StartSearch or have a
                      // persistent loop.
  // For Lazy SMP, persistent loop is better.

  m_thread = new std::thread(&Thread::SearchLoop, this);
}

Thread::~Thread() {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stopRequest = true;
  }
  m_cv.notify_one();

  if (m_thread && m_thread->joinable()) {
    m_thread->join();
  }
  delete m_thread;
  delete m_data;
}

void Thread::StartSearch(Board *board, GeneralMove *generalMoves, int alpha,
                         int beta, int depth) {
  // Debug print
  // std::cout << "Thread::StartSearch id=" << m_data->id << " depth=" << depth
  // << std::endl;
  if (depth > 100)
    std::cout << "WARNING: Thread::StartSearch depth=" << depth << std::endl;
  // Wait if already searching
  while (m_isSearching) {
    std::this_thread::yield();
  }

  {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Copy board and parameters
    memcpy(&m_searchBoard, board, sizeof(Board));
    memcpy(&m_searchGeneralMoves, generalMoves, sizeof(GeneralMove));

    m_searchBoard.iMaxPlys =
        depth; // CRITICAL FIX: Set the search depth for the helper thread

    m_alpha = alpha;
    m_beta = beta;
    m_depth = depth;

    m_isSearching = true;
  }
  m_cv.notify_one();
}

void Thread::SearchLoop() {
  while (true) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(lock, [this] { return m_isSearching || m_stopRequest; });

    if (m_stopRequest && !m_isSearching)
      break;

    if (m_isSearching) {
      // Lazy SMP: Do iterative deepening like main thread
      // Each helper thread searches same position independently
      for (int depth = 0; depth < m_depth && GetStopGo() == dGo; depth++) {
        m_searchBoard.iMaxPlys = depth;
        m_searchBoard.iNumberOfPlys = -1;

        // Perform search at this depth
        int alpha = m_alpha;
        int beta = m_beta;
        int bestMove = -1;
        int bestMoveSearched = 0;

        int score = FirstSearch(&m_searchBoard, &m_searchGeneralMoves, alpha,
                                beta, &bestMove, &bestMoveSearched, m_data);

        // Update shared PV if we found a better move
        if (m_searchBoard.vmPrincipalVariation[0][0].iFromSquare >= 0) {
          UpdateSharedPV(m_searchBoard.vmPrincipalVariation[0], score, depth);
        }
      }

      m_isSearching = false;
    }
  }
}

void InitializeThreads(int count) {
  std::cout << "InitializeThreads: Requested count=" << count << std::endl;
  std::cout << "InitializeThreads: Hardware concurrency="
            << std::thread::hardware_concurrency() << std::endl;

  DestroyThreads();

  // Create N-1 helper threads (main thread is thread 0)
  int successCount = 0;
  for (int i = 1; i < count; i++) {
    try {
      std::cout << "InitializeThreads: Creating thread " << i << "...";
      gThreads.push_back(new Thread(i));
      successCount++;
      std::cout << " SUCCESS" << std::endl;
    } catch (const std::exception &e) {
      std::cerr << " FAILED: " << e.what() << std::endl;
    }
  }
  std::cout << "InitializeThreads: Created " << successCount
            << " helper threads (total threads: " << (successCount + 1) << ")"
            << std::endl;
  std::cout.flush();
}

void DestroyThreads() {
  for (auto thread : gThreads) {
    delete thread;
  }
  gThreads.clear();
}

long long GetTotalNodes() {
  // Aggregate counters from all threads
  long long total = 0;
  if (gMainThreadData) {
    total += gMainThreadData->nodesSearched;
  }
  for (auto *thread : gThreads) {
    if (thread && thread->GetData()) {
      total += thread->GetData()->nodesSearched;
    }
  }
  return total;
}

void PrintPerformanceStats() {
  long long total = 0;
  if (gMainThreadData) {
    std::cout << "Thread 0 (Main): " << gMainThreadData->nodesSearched
              << " nodes" << std::endl;
    total += gMainThreadData->nodesSearched;
  }
  for (size_t i = 1; i < gThreads.size(); ++i) {
    long long nodes = gThreads[i]->GetData()->nodesSearched;
    std::cout << "Thread " << i << ": " << nodes << " nodes" << std::endl;
    total += nodes;
  }
  std::cout << "Total Nodes: " << total << std::endl;
}
