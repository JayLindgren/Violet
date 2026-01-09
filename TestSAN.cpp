
#include "Definitions.h"
#include "Functions.h"
#include "Structures.h"
#include "Thread.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>

std::ofstream g_Report;

// Helper to convert Square to String
static std::string ReportSquareToString( int sq )
{
   int         file = sq & 7;
   int         rank = ( sq >> 3 ); // 0-7
   char        f    = 'a' + file;
   char        r    = '1' + rank;
   std::string s;
   s += f;
   s += r;
   return s;
}

static std::string ReportMoveToString( Move *m )
{
   return ReportSquareToString( m->iFromSquare ) + ReportSquareToString( m->iToSquare );
}

void Explore( Board *board, GeneralMove *gm, int currentPly, int maxPly )
{
   if ( currentPly >= maxPly )
      return;

   Move moveList[ dNumberOfMoves ];
   CalculateMoves( moveList, board, gm );

   // We want to list moves that are IN THE BOOK.
   // Violet's "ExtractOpeningBookStats" works on the CURRENT board hash.
   // So we must MAKE the move to check if the RESULT is in the book.

   struct BookMove
   {
      int      listIndex;
      BitBoard white, black, drawing;
      BitBoard total;
   };

   std::vector<BookMove> bookMoves;

   for ( int i = 0; i < board->siNumberOfMoves; i++ )
   {
      MakeMove( moveList, board, gm, i );

      // Probe
      BitBoard w, b, d;
      w = 0;
      b = 0;
      d = 0;

      // Check if hash matches ANY entry.
      // We use ExtractOpeningBookStats logic manually or call it.
      // If hash is not in book, Extract returns 0,0,0 usually (unless collision logic handled differently).
      // Let's use the function.
      ExtractOpeningBookStats( w, b, d, gm, board );

      if ( w > 0 || b > 0 || d > 0 )
      {
         BookMove bm;
         bm.listIndex = i;
         bm.white     = w;
         bm.black     = b;
         bm.drawing   = d;
         bm.total     = w + b + d;
         bookMoves.push_back( bm );
      }

      UndoMove( board, gm );
   }

   if ( bookMoves.empty() )
   {
      // g_Report << std::string(currentPly * 2, ' ') << "<End of Book Line>\n";
      return;
   }

   // Process book moves
   for ( const auto &bm : bookMoves )
   {
      // Print
      MakeMove( moveList, board, gm, bm.listIndex );

      // Indentation
      std::string indent( currentPly * 2, ' ' );

      // Move Name
      std::string moveStr = ReportMoveToString( &moveList[ bm.listIndex ] );

      g_Report << indent << "Ply " << ( currentPly + 1 ) << ": " << moveStr;

      // Calculate Percentages
      double total = (double)bm.total;
      double wp    = 100.0 * (double)bm.white / total;
      double bp    = 100.0 * (double)bm.black / total;
      double dp    = 100.0 * (double)bm.drawing / total;

      g_Report << std::fixed << std::setprecision( 1 )
               << " (W:" << wp << "% B:" << bp << "% D:" << dp << "% N:" << bm.total << ")\n";

      // Recurse
      Explore( board, gm, currentPly + 1, maxPly );

      UndoMove( board, gm );
   }
}

int main()
{
   // 1. Initialize
   InitializeThreads( 1 ); // Minimal threads

   Board       *board = new Board();
   GeneralMove *gm    = new GeneralMove();

   InitializeOpeningBook();
   // CRITICAL: Initialize Hash Keys properly
   InitializeHashTable(); // Use standard initialization
   // But InitializeHashTable uses "SetHashTableSizeBits" which allocates memory?
   // And calls AssignRandomKeys().
   // We must ensure the keys match what was used to CREATE the book.
   // The book was created with "Code" persistent keys.
   // SetPersistantKeys(dCode) before InitializeHashTable?
   SetPersistantKeys( dCode );
   InitializeHashTable();

   GenerateGeneralMove( gm );
   CreateBoard( board, gm );

   // Load Book
   ReadOpeningBook( "C:\\VioletTools\\Book.txt", gm );

   // 2. Setup Ruy Lopez
   // 1. e4 e5 2. Nf3 Nc6 3. Bb5
   const char *moves[] = { "e4", "e5", "Nf3", "Nc6", "Bb5" };
   Move        moveList[ dNumberOfMoves ];

   std::cout << "Setting up Ruy Lopez...\n";
   for ( int i = 0; i < 5; i++ )
   {
      CalculateMoves( moveList, board, gm );
      int idx = GetMoveFromSAN( board, gm, moveList, (char *)moves[ i ] );
      if ( idx == -1 )
      {
         std::cout << "Failed to find move " << moves[ i ] << "\n";
         return 1;
      }
      MakeMove( moveList, board, gm, idx );
   }

   // 3. Generate Report
   g_Report.open( "RuyLopezReport.txt" );
   g_Report << "Violet Opening Book Exploration: Ruy Lopez (5 Moves / 10 Ply Deep)\n";
   g_Report << "Start Position: After 1. e4 e5 2. Nf3 Nc6 3. Bb5\n";
   g_Report << "------------------------------------------------------------\n";

   Explore( board, gm, 0, 10 ); // 10 Ply Depth

   g_Report.close();
   std::cout << "Report generated: RuyLopezReport.txt\n";

   return 0;
}
