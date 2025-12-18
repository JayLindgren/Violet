//
// Filename: HashTable.cpp
//
// Purpose: Everything with hashes and the hash table.
//

// Define switches.
#if defined( dDeepMode )
   #include <omp.h>
#endif

// Define the external headers.
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <fstream>
#include <cstring>
#include <string>

// Define the internal headers.
#include "Functions.h"
#include "Definitions.h"
#include "Structures.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Define global variables for keeping track of the number of nodes counted.
// Global variables suck, but are awsome for allowing for Deep Violet.
// access to the table:
// Note that the scope for the globe variables is only this file.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   HashTable gsHashTable;

// Helper: round down to the highest power-of-two not exceeding x
static BitBoard RoundDownToPowerOfTwo(BitBoard x)
{
   if (x == 0) return 0;
   // Make all bits below the highest set bit also set
   x |= (x >> 1);
   x |= (x >> 2);
   x |= (x >> 4);
   x |= (x >> 8);
   x |= (x >> 16);
   x |= (x >> 32);
   // Keep only the highest bit
   return (x + 1) >> 1;
}

// Internal: allocate hash arrays for a given entry count (must be power of two)
static void AllocateHashWithCount(BitBoard entryCount)
{
   // Free previous allocations if any
   if (gsHashTable.mbbHash)      free(gsHashTable.mbbHash);
   if (gsHashTable.mbbHashTable) free(gsHashTable.mbbHashTable);

   // Store the element count and mask
   gsHashTable.bbNumberOfHashElements = entryCount; // number of entries
   gsHashTable.bbMaskIndex = entryCount ? (entryCount - 1) : 0; // index mask

   // Allocate the memory
   if (entryCount > 0)
   {
      gsHashTable.mbbHash = ( std::atomic<BitBoard> * ) malloc( entryCount * sizeof( std::atomic<BitBoard> ) );
      gsHashTable.mbbHashTable = ( std::atomic<BitBoard> * ) malloc( entryCount * sizeof( std::atomic<BitBoard> ) ); 

      // Loop over the hash table entries and set them to zero.
      for ( BitBoard bbHashIndex = 0; bbHashIndex < gsHashTable.bbNumberOfHashElements; bbHashIndex++ )
      {
         gsHashTable.mbbHashTable[ bbHashIndex ].store(0, std::memory_order_relaxed);
         gsHashTable.mbbHash[ bbHashIndex ].store(0, std::memory_order_relaxed);
      }
   }
   else
   {
      gsHashTable.mbbHash = nullptr;
      gsHashTable.mbbHashTable = nullptr;
   }
}

// Public API: set hash size by number of index bits (entry count = 2^bits)
void SetHashTableSizeBits(int bits)
{
   if (bits < 1) bits = 1;
   if (bits > 30) bits = 30; // guard against overflow / excessive allocations
   BitBoard count = (BitBoard)1 << bits; // entry count
   AllocateHashWithCount(count);
}

// Public API: set hash size by megabytes (across both arrays)
// Each entry takes 16 bytes (two 64-bit values)
void SetHashTableSizeMB(int megabytes)
{
   if (megabytes <= 0) { AllocateHashWithCount(0); return; }
   const BitBoard bytes = (BitBoard)megabytes * 1024ULL * 1024ULL;
   const BitBoard bytesPerEntry = sizeof(BitBoard) * 2ULL; // key + data
   BitBoard maxEntries = bytes / bytesPerEntry;
   // Round down to nearest power of two for clean masking
   BitBoard entryCount = RoundDownToPowerOfTwo(maxEntries);
   AllocateHashWithCount(entryCount);
}

//
//-------------------------------------------------------------------------------------------------------------
//
void ClearHashTable()
{

   // Rest the hash table.
   for ( BitBoard bbHashIndex = 0; bbHashIndex < gsHashTable.bbNumberOfHashElements; bbHashIndex++ )
   {

      gsHashTable.mbbHashTable[ bbHashIndex ].store(0, std::memory_order_relaxed);
      gsHashTable.mbbHash[ bbHashIndex ].store(0, std::memory_order_relaxed);

   }

}


//
//-------------------------------------------------------------------------------------------------------------
//
void InitializeHashTable()
{
// This function is used to initialize the hash table.  The function is stored in the search file because the 
// hash table is a global variable and is needed the most in the search routines.
//

//# if defined( dUseHash )
   // Allocate the memory using the compile-time default.
   // Use entry count = 2^bits; mask = count - 1
   SetHashTableSizeBits(dNumberOfBitsInHash);
//# endif

   // Switch on using persistant or new keys.
   if ( GetPersistantKeys() == dNewKeys )
   {

      // Set up the hash keys.
      int iSquareIndex = 0;
      for ( iSquareIndex = 0; iSquareIndex < 64; iSquareIndex++ )
      {

         for ( int iPieceIndex = 0; iPieceIndex < 12; iPieceIndex++ )
         {

           gsHashTable.mbbHashKeys[ iPieceIndex ][ iSquareIndex ] = RandomBB();

         }

         gsHashTable.vbbEnPassant[ iSquareIndex ] = RandomBB();

      }

      for ( int iStateIndex = 0; iStateIndex < 6; iStateIndex++ )
      {

         gsHashTable.vbbHashKeysStates[ iStateIndex ] = RandomBB();

      }

      gsHashTable.vbbCasteling[ 0 ] = RandomBB();
      gsHashTable.vbbCasteling[ 1 ] = RandomBB();
      gsHashTable.vbbCasteling[ 2 ] = RandomBB();
      gsHashTable.vbbCasteling[ 3 ] = RandomBB();

      gsHashTable.vbbColorToMove[ 0 ] = RandomBB();
      gsHashTable.vbbColorToMove[ 1 ] = RandomBB();


      // Set up the initial hash.
      gsHashTable.bbHash = RandomBB();

   }
   else if ( GetPersistantKeys() == dFile )
   {

      ReadRandomKeyFile();
      //cout << "Initial Hash = " << gsHashTable.bbHashInitial << endl;

   }
   else if ( GetPersistantKeys() == dCode )
   {

      // Read the keys
      GetAllKeys();
      AssignRandomKeys();
      //cout << "Initial Hash = " << gsHashTable.bbHashInitial << endl;

   }

//# if defined( dUseHash )  
   // Already zeroed in AllocateHashWithCount
//# endif
   
   // Set up the mask for extracting the index.
   // Already set as (count - 1) in AllocateHashWithCount

   // Set the initial hash for later reference
   gsHashTable.bbHashInitial = gsHashTable.bbHash;

}

//
//-----------------------------------------------------------------------
//
void UpdateHash1( struct Move * argsMove,
                 struct Board * argsBoard,
                 int iMakeUnmake,
                 struct GeneralMove * argsGeneralMoves )

{

   // Debug the inputs
   assert( argsMove >= 0 );
   assert( argsBoard >= 0 );
   assert( iMakeUnmake >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Update the hash.
   int iToSquare   = argsMove->iToSquare;
   int iFromSquare = argsMove->iFromSquare;
   int iPiece      = argsMove->iPiece;
   int iCapture    = argsMove->iCapture;
   int iMoveType   = argsMove->iMoveType;

   assert( iToSquare >= 0 );
   assert( iToSquare <= 64 );
   assert( iFromSquare >= 0 );
   assert( iFromSquare <= 64 );
   assert( iPiece >= 0 );
   assert( iPiece <= dBlackKing );
   assert( iCapture >= 0 );
   assert( iCapture <= 64 );
   assert( iMoveType >= 0 );


   // Do a normal move.
   argsBoard->bbHash = argsBoard->bbHash ^
                        gsHashTable.mbbHashKeys[ iPiece ][ iToSquare ] ^
                        gsHashTable.mbbHashKeys[ iPiece ][ iFromSquare ];

   // Do a capture.
   if ( iMoveType == dCapture )
   {

     argsBoard->bbHash = argsBoard->bbHash ^
                          gsHashTable.mbbHashKeys[ iCapture ][ iToSquare ];

   }

   // Do a promotion.
   if ( ( iMoveType == dPromote ) ||
        ( iMoveType == dCaptureAndPromote ) )
   {

     argsBoard->bbHash = argsBoard->bbHash ^
                          gsHashTable.mbbHashKeys[ iPiece ][ iToSquare ] ^
                          gsHashTable.mbbHashKeys[ 1 ][ iFromSquare ];

   }

   // Do that EP thing.
   if ( iMoveType == dEnPassant )
   {

     argsBoard->bbHash = argsBoard->bbHash ^
                          gsHashTable.vbbEnPassant[ iToSquare ];

   }

   // Handle the castling possibilities.
   if ( iMoveType == dWhiteKingSideCastle )
   {

      argsBoard->bbHash = argsBoard->bbHash ^
                           gsHashTable.vbbHashKeysStates[ 0 ];

   }
   else if ( iMoveType == dWhiteQueenSideCastle )   
   {

      argsBoard->bbHash = argsBoard->bbHash ^
                           gsHashTable.vbbHashKeysStates[ 1 ];

   }
   else if ( iMoveType == dBlackKingSideCastle )   
   {

      argsBoard->bbHash = argsBoard->bbHash ^
                           gsHashTable.vbbHashKeysStates[ 2 ];

   }
   else if ( iMoveType == dBlackQueenSideCastle )   
   {

      argsBoard->bbHash = argsBoard->bbHash ^
                           gsHashTable.vbbHashKeysStates[ 3 ];

   }

   // Switch on the colors, note that since we need to remove on color and add the other
   // this operation will always be performed.
   if ( iPiece < dBlackPawn )
   {

       argsBoard->bbHash = argsBoard->bbHash ^
                            gsHashTable.vbbHashKeysStates[ 0 ];

   }
   else
   {
       argsBoard->bbHash = argsBoard->bbHash ^
                            gsHashTable.vbbHashKeysStates[ 1 ];
   }

}

//
//-----------------------------------------------------------------------
//
void UpdateHash( struct Move * argsMove,
                 struct Board * argsBoard,
                 int iMakeUnmake,
                 struct GeneralMove * argsGeneralMoves )

{
   // Debug the inputs
   assert(argsMove >= 0);
   assert(argsBoard >= 0);
   assert(iMakeUnmake >= 0);
   assert(argsGeneralMoves >= 0);

   // Update the hash.
   int iToSquare = argsMove->iToSquare;
   int iFromSquare = argsMove->iFromSquare;
   int iPiece = argsMove->iPiece;
   int iCapture = argsMove->iCapture;
   int iMoveType = argsMove->iMoveType;

   assert(iToSquare >= 0);
   assert(iToSquare <= 64);
   assert(iFromSquare >= 0);
   assert(iFromSquare <= 64);
   assert(iPiece >= 0);
   assert(iPiece <= dBlackKing);
   assert(iCapture >= 0);
   assert(iCapture <= 64);
   assert(iMoveType >= 0);

   // ---------------------------------------------------------
   // 1. Side to Move
   // ---------------------------------------------------------
   // XOR out current side, XOR in next side.
   // ---------------------------------------------------------
   // ---------------------------------------------------------
   // ---------------------------------------------------------
   // 1. Side to Move
   // ---------------------------------------------------------

   // XOR out current side, XOR in next side.
   argsBoard->bbHash ^= gsHashTable.vbbColorToMove[ argsBoard->siColorToMove ];
   argsBoard->bbHash ^= gsHashTable.vbbColorToMove[ argsBoard->siColorToMove ^ 1 ];

   // ---------------------------------------------------------
   // 2. En Passant Opportunity (Remove old)
   // ---------------------------------------------------------
   if ( argsBoard->bbEP )
   {
      int vPos[1];
      Find( argsBoard->bbEP, vPos, argsGeneralMoves );
      argsBoard->bbHash ^= gsHashTable.vbbEnPassant[ vPos[0] ];
   }

   // ---------------------------------------------------------
   // 3. En Passant Opportunity (Add new)
   // ---------------------------------------------------------
   if ( iMoveType == dTwoSquare )
   {
      int vPos[1];
      Find( argsMove->bbEPSquare, vPos, argsGeneralMoves );
      argsBoard->bbHash ^= gsHashTable.vbbEnPassant[ vPos[0] ];
   }

   // ---------------------------------------------------------
   // 4. Castling Rights (Remove lost rights)
   // ---------------------------------------------------------
   BitBoard bbLostRights = 0;
   
   // Moving King or Rook
   if ( iPiece == dWhiteKing ) bbLostRights |= (1ULL << 0) | (1ULL << 2);
   if ( iPiece == dBlackKing ) bbLostRights |= (1ULL << 1) | (1ULL << 3);
   
   if ( iPiece == dWhiteRook )
   {
      if ( iFromSquare == dH1 ) bbLostRights |= (1ULL << 0);
      if ( iFromSquare == dA1 ) bbLostRights |= (1ULL << 2);
   }
   if ( iPiece == dBlackRook )
   {
      if ( iFromSquare == dH8 ) bbLostRights |= (1ULL << 1);
      if ( iFromSquare == dA8 ) bbLostRights |= (1ULL << 3);
   }

   // Capturing Rook
   if ( iCapture == dWhiteRook )
   {
      if ( iToSquare == dH1 ) bbLostRights |= (1ULL << 0);
      if ( iToSquare == dA1 ) bbLostRights |= (1ULL << 2);
   }
   if ( iCapture == dBlackRook )
   {
      if ( iToSquare == dH8 ) bbLostRights |= (1ULL << 1);
      if ( iToSquare == dA8 ) bbLostRights |= (1ULL << 3);
   }

   // Only XOR if the right was actually present
   BitBoard bbRightsToRemove = bbLostRights & argsBoard->bbCastle;
   if ( bbRightsToRemove )
   {
       if ( bbRightsToRemove & (1ULL << 0) ) argsBoard->bbHash ^= gsHashTable.vbbCasteling[0];
       if ( bbRightsToRemove & (1ULL << 1) ) argsBoard->bbHash ^= gsHashTable.vbbCasteling[1];
       if ( bbRightsToRemove & (1ULL << 2) ) argsBoard->bbHash ^= gsHashTable.vbbCasteling[2];
       if ( bbRightsToRemove & (1ULL << 3) ) argsBoard->bbHash ^= gsHashTable.vbbCasteling[3];
   }

   // ---------------------------------------------------------
   // 5. Piece Moves (Standard Zobrist)
   // ---------------------------------------------------------
   if ( ( iMoveType == dPromote ) || ( iMoveType == dCaptureAndPromote ) )
   {
       // Promotion: Remove Pawn, Add Promoted Piece
       int pawnPiece = ( iPiece < dBlackPawn ) ? dWhitePawn : dBlackPawn;
       argsBoard->bbHash ^= gsHashTable.mbbHashKeys[ pawnPiece ][ iFromSquare ];
       argsBoard->bbHash ^= gsHashTable.mbbHashKeys[ iPiece ][ iToSquare ];
       
       // Capture
       if ( iMoveType == dCaptureAndPromote )
       {
           argsBoard->bbHash ^= gsHashTable.mbbHashKeys[ iCapture ][ iToSquare ];
       }
   }
   else
   {
       // Normal Move
       argsBoard->bbHash ^= gsHashTable.mbbHashKeys[ iPiece ][ iFromSquare ];
       argsBoard->bbHash ^= gsHashTable.mbbHashKeys[ iPiece ][ iToSquare ];

       // Capture
       if ( iMoveType == dCapture )
       {
          argsBoard->bbHash ^= gsHashTable.mbbHashKeys[ iCapture ][ iToSquare ];
       }
       
       // En Passant Capture
       if ( iMoveType == dEnPassant )
       {
           // Remove the captured pawn.
           // Capture square is iToSquare +/- 8.
           int captureSquare = ( argsBoard->siColorToMove == dWhite ) ? ( iToSquare - 8 ) : ( iToSquare + 8 );
           int capturedPawn = ( argsBoard->siColorToMove == dWhite ) ? dBlackPawn : dWhitePawn;
           argsBoard->bbHash ^= gsHashTable.mbbHashKeys[ capturedPawn ][ captureSquare ];
       }

       // Castling Move (Move the Rook)
       if ( iMoveType == dWhiteKingSideCastle )
       {
           argsBoard->bbHash ^= gsHashTable.mbbHashKeys[ dWhiteRook ][ dH1 ];
           argsBoard->bbHash ^= gsHashTable.mbbHashKeys[ dWhiteRook ][ dF1 ];
       }
       else if ( iMoveType == dWhiteQueenSideCastle )
       {
           argsBoard->bbHash ^= gsHashTable.mbbHashKeys[ dWhiteRook ][ dA1 ];
           argsBoard->bbHash ^= gsHashTable.mbbHashKeys[ dWhiteRook ][ dD1 ];
       }
       else if ( iMoveType == dBlackKingSideCastle )
       {
           argsBoard->bbHash ^= gsHashTable.mbbHashKeys[ dBlackRook ][ dH8 ];
           argsBoard->bbHash ^= gsHashTable.mbbHashKeys[ dBlackRook ][ dF8 ];
       }
       else if ( iMoveType == dBlackQueenSideCastle )
       {
           argsBoard->bbHash ^= gsHashTable.mbbHashKeys[ dBlackRook ][ dA8 ];
           argsBoard->bbHash ^= gsHashTable.mbbHashKeys[ dBlackRook ][ dD8 ];
       }
   }

}

//
//--------------------------------------------------------------------------
//
HashQueryResult ExtractFromHashTable( struct Board * argsBoard,
                                      struct GeneralMove * argsGeneralMoves )
{
// This function will return a HashQueryResult struct.

   HashQueryResult result;
   result.iQueryState = 0;
   result.iScore = 0;
   result.iDepth = 0;
   result.iBestMove = 128; // Default invalid move
   result.iAge = 0;
   result.iTypeOfScore = 0;

   // Debug the inputs
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Extract the data from hash table.
   BitBoard bbKey = argsBoard->bbHash & gsHashTable.bbMaskIndex;   
   
   // Extract the depth.
   result.iDepth = (int)(( gsHashTable.mbbHashTable[ bbKey ] & 
                           argsGeneralMoves->bbDepth ) >>
                           argsGeneralMoves->iDepthShift);
 
   // Debugging.
   assert( bbKey >= 0 );   
   assert( bbKey <  gsHashTable.bbNumberOfHashElements );

   // Thread-safe hash table access (Lockless with Atomics)
   {
      // Check if we are using the table.
      if ( GetUseHashTable() == dNo )
      {
         // Not using hash table
         return result;
      }
      
      // Load the key with ACQUIRE semantics to ensure we see the data written before it
      BitBoard storedKey = gsHashTable.mbbHash[ bbKey ].load(std::memory_order_acquire);
      
      // Check the position.
      // See if it has been found before.
      // Also check to see if we have a key collision.
      if ( ( storedKey == 0 ) ||
           ( storedKey != argsBoard->bbHash ) ||
           ( result.iDepth < ( argsBoard->iMaxPlys - argsBoard->iNumberOfPlys )  ) )
      {
         // Not found or collision or insufficient depth
         return result;
      }
      else
      {
         // We have seen this position before:
         // Load the data (Relaxed is fine because of the Acquire on the key)
         BitBoard storedData = gsHashTable.mbbHashTable[ bbKey ].load(std::memory_order_relaxed);
         
         // Extract the score and the sign.
         BitBoard bbSign = argsGeneralMoves->bbScoreSign & storedData;  
         bbSign = bbSign >> argsGeneralMoves->iSignShift; 
         result.iScore =  (int)( storedData & argsGeneralMoves->bbScore );

         if ( bbSign >= 1 )
         {
            result.iScore = - result.iScore;
         }

         // Extract the best move.
         result.iBestMove = (int)(( storedData & 
                                    argsGeneralMoves->bbBestMove ) >>
                                    argsGeneralMoves->iBestMoveShift);

         // Extract the age.
         result.iAge = (int)(( storedData & 
                               argsGeneralMoves->bbAge ) >>
                               argsGeneralMoves->iAgeShift);

         // Return a found position.
         result.iQueryState = 1;

      }

   } 

   return result;

}

//
//--------------------------------------------------------------------------
//
void InputToHashTable( struct Board * argsBoard,
                       struct GeneralMove * argsGeneralMoves,
                       int argiAlpha,
                       int argiBeta,
                       int argiScore,
                       struct Move * argsMove )
{
// Put the score and the best move into the hash table.

   // If the hash table isn't being used, jump out.
   if ( GetUseHashTable() == dNo )
   {
      return;
   }

   // Thread-safe hash table access (Lockless with Atomics)
   {
      // Calculate key
      BitBoard bbKey = argsBoard->bbHash & gsHashTable.bbMaskIndex;
      
      // Optimistic check without lock (Double-Checked Locking Pattern)
      // We can read the key atomically (64-bit aligned read on x64)
      // If the key matches and the depth is sufficient, we don't need to write.
      // This avoids taking the unique lock for the vast majority of probes.
      BitBoard bbExistingKeyOptimistic = gsHashTable.mbbHash[ bbKey ].load(std::memory_order_relaxed);
      
      // Extract the old depth currently stored at this index.
      BitBoard bbDepthOldOptimistic = ( gsHashTable.mbbHashTable[ bbKey ].load(std::memory_order_relaxed) & 
                             argsGeneralMoves->bbDepth ) >>
                             argsGeneralMoves->iDepthShift;

      // Calculate the new depth (remaining search depth from current ply).
      int iDepth = argsBoard->iMaxPlys - argsBoard->iNumberOfPlys;
      if ( iDepth < 0 )
      {
         iDepth = 0;
      }
      BitBoard bbDepth = (BitBoard)iDepth; 

      bool bSameKeyOptimistic = ( bbExistingKeyOptimistic == argsBoard->bbHash );
      bool bShouldReplaceOptimistic = ( !bSameKeyOptimistic ) || ( bbDepthOldOptimistic <= bbDepth );

      if ( !bShouldReplaceOptimistic )
      {
          return; // Early exit without locking
      }

      // Some scoring correction to make sure the score fits.
      if ( argiAlpha < dMate ) { argiAlpha = dMate; }
      if ( argiBeta  < dMate ) { argiBeta  = dMate; }
      if ( argiAlpha > - dMate ) { argiAlpha = - dMate; }
      if ( argiBeta  > - dMate ) { argiBeta  = - dMate; }

      assert( argsBoard >= 0 );
      assert( argsGeneralMoves >= 0 );
      assert( argiAlpha >= dMate );
      assert( argiBeta  >= dMate );
      assert( argiScore >= dMate );
      assert( argsMove  >= 0 );

      // Input the data to hash table.
      // BitBoard bbKey = argsBoard->bbHash & gsHashTable.bbMaskIndex; // Moved up
      assert( bbKey < gsHashTable.bbNumberOfHashElements );

      // Prepare the new data entry
      BitBoard bbNewData = 0;

      // Enter the depth.
      BitBoard bbDepthField = bbDepth << argsGeneralMoves->iDepthShift;
      bbNewData |= bbDepthField;

      // Input the score and handle the sign.
      BitBoard bbScore = 0;
      if ( argiScore < 0 )
      {
         // Put the sign in the first bit of the score field as expected by masks/shifts.
         bbScore = - argiScore;
         bbScore = SetBitToOne( bbScore, 17 );
      }
      else
      {
         bbScore = argiScore;
      }
      bbScore = bbScore << argsGeneralMoves->iScoreShift;
      bbNewData |= bbScore;

      // Input the best move if present.
      if ( argsBoard->iBestMove < 128 )
      {
         BitBoard bbBestMove = argsBoard->iBestMove;
         bbBestMove = bbBestMove << argsGeneralMoves->iBestMoveShift;
         bbNewData |= bbBestMove;   
      }

      // Enter the age (ply into the game when observed).
      BitBoard bbAge = 0;
      if ( argsBoard->iMoveHistory > 0 )
      {
         bbAge = argsBoard->iMoveHistory;
      }
      bbAge = bbAge << argsGeneralMoves->iAgeShift;
      bbNewData |= bbAge;
      
      // LOCKLESS WRITE:
      // 1. Write Data first (Relaxed)
      // 2. Write Key second (Release)
      // This ensures that if a reader sees the new Key, they will likely see the new Data.
      
      gsHashTable.mbbHashTable[ bbKey ].store(bbNewData, std::memory_order_relaxed);
      gsHashTable.mbbHash[ bbKey ].store(argsBoard->bbHash, std::memory_order_release);
  
   } 

}

//
//-------------------------------------------------------------------------------------------------
//
void AssignRandomKeys()
{

// This function is used to create the random hash keys that will be persistant.
//

   int iKeyIndex = 0;

   // Set up the hash keys.
   int iSquareIndex = 0;
   for ( iSquareIndex = 0; iSquareIndex < 64; iSquareIndex++ )
   {

      for ( int iPieceIndex = 0; iPieceIndex < 12; iPieceIndex++ )
      {

        gsHashTable.mbbHashKeys[ iPieceIndex ][ iSquareIndex ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];

      }

      gsHashTable.vbbEnPassant[ iSquareIndex ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];;

   }

   for ( int iStateIndex = 0; iStateIndex < 6; iStateIndex++ )
   {

      gsHashTable.vbbHashKeysStates[ iStateIndex ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];;

   }

   gsHashTable.vbbCasteling[ 0 ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];
   gsHashTable.vbbCasteling[ 1 ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];
   gsHashTable.vbbCasteling[ 2 ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];
   gsHashTable.vbbCasteling[ 3 ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];

   gsHashTable.vbbColorToMove[ 0 ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];
   gsHashTable.vbbColorToMove[ 1 ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];


   // Set up the initial hash.
   gsHashTable.bbHash = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];;

   // Keep an initial hash for reference
   gsHashTable.bbHashInitial = gsHashTable.bbHash;

}

//
//-------------------------------------------------------------------------------------------------
//
void ReadRandomKeyFile()
{

// This function is used to create the random hash keys that will be persistant.
//

   // Read the opening book.
   ifstream ifKeys( "RandomKeys.txt" );
   if ( ifKeys.fail() )
   {
   
      cout << "Input RandomKeys.txt file failed to open." << endl;
      
   }

   // Set up the hash keys.
   int iSquareIndex = 0;
   for ( iSquareIndex = 0; iSquareIndex < 64; iSquareIndex++ )
   {

      for ( int iPieceIndex = 0; iPieceIndex < 12; iPieceIndex++ )
      {

        ifKeys >> gsHashTable.mbbHashKeys[ iPieceIndex ][ iSquareIndex ];

      }

      ifKeys >> gsHashTable.vbbEnPassant[ iSquareIndex ];

   }

   for ( int iStateIndex = 0; iStateIndex < 6; iStateIndex++ )
   {

      ifKeys >> gsHashTable.vbbHashKeysStates[ iStateIndex ];

   }

   ifKeys >> gsHashTable.vbbCasteling[ 0 ];
   ifKeys >> gsHashTable.vbbCasteling[ 1 ];
   ifKeys >> gsHashTable.vbbCasteling[ 2 ];
   ifKeys >> gsHashTable.vbbCasteling[ 3 ];

   ifKeys >> gsHashTable.vbbColorToMove[ 0 ];
   ifKeys >> gsHashTable.vbbColorToMove[ 1 ];


   // Set up the initial hash.
   ifKeys >> gsHashTable.bbHash;

   // Keep an initial hash for reference
   gsHashTable.bbHashInitial = gsHashTable.bbHash;

   // Close the file.
   ifKeys.close();

}

//
//------------------------------------------------------------------------------------------
//
void DestroyHashTable()
{
// This function releases the memory taken by the hash table.

   free( gsHashTable.mbbHash );
   free( gsHashTable.mbbHashTable );

}

// Define some hash sets.
void SetHash( BitBoard bbHash )
{
   gsHashTable.bbHash = bbHash;
}
void SetHashElement( BitBoard bbKey,
                     BitBoard bbElement )
{
   gsHashTable.mbbHashTable[ bbKey ] = bbElement;
} 

// Define some input and some output variables.
BitBoard GetHash()
{
   return gsHashTable.bbHash;
}
BitBoard GetHashInitial()
{
   return gsHashTable.bbHashInitial;
}
BitBoard GetHashElement( BitBoard bbKey )
{
   return gsHashTable.mbbHashTable[ bbKey ];
} 
int GetQueryState()
{
   return gsHashTable.iQueryState;
}
BitBoard GetDepth()
{
   return gsHashTable.bbDepth;
}
int GetScoreHash()
{
   return gsHashTable.iScore;
}
BitBoard GetBestMove()
{
   return gsHashTable.bbBestMove;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Test Suites are below.




//
//----------------------------------------------------------------------------------------------
//
// I was storing the persistant random keys in a text file.  This converts it to code for better
// "permanant" storage.
void ConvertRandomKeyFileToCode()
{

   ifstream ifKeyFile( "RandomKeys.txt" );
   ofstream ofKeyCode( "RandomKeyCode.txt" );

   if ( ifKeyFile.fail() )
   {
     
      cout << "Input file failed to open." << endl;
      system( "pause" );

   }
   if ( ofKeyCode.fail() )
   {

      cout << "Output file failed to open." << endl;
      system( "pause" );

   }

   int iCounter = -1;
   int iFlag = 1;
   BitBoard bbKey = 0;
   BitBoard bbKeyOld = 0;
   while ( iFlag )
   {

      // Get the key and increment the counter.
      ifKeyFile >> bbKey;
      if ( bbKey == bbKeyOld )
      {

         iFlag = 0;
         break;

      }

      // Keep the old key.
      bbKeyOld = bbKey;

      // Increment the counter
      iCounter++;

      // Let the folks at home see the count.
      cout << "iCounter = " << iCounter << " Key = " << bbKey << endl;

      // Write the code.
      ofKeyCode << "gsHashTable.vbbRandomKeys[ " << iCounter << " ] = " << bbKey << ";" << endl;
      

   }

   ifKeyFile.close();
   ofKeyCode.close();

}


//
//--------------------------------------------------------------------------------------
//
void GetAllKeys()
{

   // Here are the permanant keys.
   gsHashTable.vbbRandomKeys[ 0 ] = 4935135370079648666ULL;
   gsHashTable.vbbRandomKeys[ 1 ] = 8760541055110964466ULL;
   gsHashTable.vbbRandomKeys[ 2 ] = 1373468545248413492ULL;
   gsHashTable.vbbRandomKeys[ 3 ] = 17424003862454590535ULL;
   gsHashTable.vbbRandomKeys[ 4 ] = 7046370612499155837ULL;
   gsHashTable.vbbRandomKeys[ 5 ] = 5922200747593917863ULL;
   gsHashTable.vbbRandomKeys[ 6 ] = 553153244974754788ULL;
   gsHashTable.vbbRandomKeys[ 7 ] = 17152115956460010486ULL;
   gsHashTable.vbbRandomKeys[ 8 ] = 8813278035371429741ULL;
   gsHashTable.vbbRandomKeys[ 9 ] = 13610028684416652972ULL;
   gsHashTable.vbbRandomKeys[ 10 ] = 7320583005235833889ULL;
   gsHashTable.vbbRandomKeys[ 11 ] = 18428241321350849386ULL;
   gsHashTable.vbbRandomKeys[ 12 ] = 12501768878060745096ULL;
   gsHashTable.vbbRandomKeys[ 13 ] = 2301449675581005687ULL;
   gsHashTable.vbbRandomKeys[ 14 ] = 7959306954535313715ULL;
   gsHashTable.vbbRandomKeys[ 15 ] = 17311297250047461341ULL;
   gsHashTable.vbbRandomKeys[ 16 ] = 7647947029509561407ULL;
   gsHashTable.vbbRandomKeys[ 17 ] = 6245404973388758094ULL;
   gsHashTable.vbbRandomKeys[ 18 ] = 6901276093037625841ULL;
   gsHashTable.vbbRandomKeys[ 19 ] = 7912728514252366347ULL;
   gsHashTable.vbbRandomKeys[ 20 ] = 1242976831634006302ULL;
   gsHashTable.vbbRandomKeys[ 21 ] = 6424960976185133159ULL;
   gsHashTable.vbbRandomKeys[ 22 ] = 12233926170583448611ULL;
   gsHashTable.vbbRandomKeys[ 23 ] = 9464641311136192561ULL;
   gsHashTable.vbbRandomKeys[ 24 ] = 10995414420296893337ULL;
   gsHashTable.vbbRandomKeys[ 25 ] = 3583907980795662100ULL;
   gsHashTable.vbbRandomKeys[ 26 ] = 12949840557515789192ULL;
   gsHashTable.vbbRandomKeys[ 27 ] = 7247304256905388901ULL;
   gsHashTable.vbbRandomKeys[ 28 ] = 2350353823024099498ULL;
   gsHashTable.vbbRandomKeys[ 29 ] = 13056814523014245207ULL;
   gsHashTable.vbbRandomKeys[ 30 ] = 18094338687727304282ULL;
   gsHashTable.vbbRandomKeys[ 31 ] = 3528422694172788572ULL;
   gsHashTable.vbbRandomKeys[ 32 ] = 11479469170138869657ULL;
   gsHashTable.vbbRandomKeys[ 33 ] = 9870638376459579589ULL;
   gsHashTable.vbbRandomKeys[ 34 ] = 9726242749197214417ULL;
   gsHashTable.vbbRandomKeys[ 35 ] = 17575098152022733246ULL;
   gsHashTable.vbbRandomKeys[ 36 ] = 18207978509386268757ULL;
   gsHashTable.vbbRandomKeys[ 37 ] = 12935208460127086199ULL;
   gsHashTable.vbbRandomKeys[ 38 ] = 3822731913716169222ULL;
   gsHashTable.vbbRandomKeys[ 39 ] = 50957834674229182ULL;
   gsHashTable.vbbRandomKeys[ 40 ] = 14745584112208288187ULL;
   gsHashTable.vbbRandomKeys[ 41 ] = 9150751690725558562ULL;
   gsHashTable.vbbRandomKeys[ 42 ] = 10729842981047422978ULL;
   gsHashTable.vbbRandomKeys[ 43 ] = 16389271633590366922ULL;
   gsHashTable.vbbRandomKeys[ 44 ] = 167595253981702758ULL;
   gsHashTable.vbbRandomKeys[ 45 ] = 6198823806034427955ULL;
   gsHashTable.vbbRandomKeys[ 46 ] = 1860974486140708579ULL;
   gsHashTable.vbbRandomKeys[ 47 ] = 16033988530684246712ULL;
   gsHashTable.vbbRandomKeys[ 48 ] = 13701360435820042510ULL;
   gsHashTable.vbbRandomKeys[ 49 ] = 17143430502818090469ULL;
   gsHashTable.vbbRandomKeys[ 50 ] = 18237011452591495607ULL;
   gsHashTable.vbbRandomKeys[ 51 ] = 3111763172835065622ULL;
   gsHashTable.vbbRandomKeys[ 52 ] = 12795754322230409885ULL;
   gsHashTable.vbbRandomKeys[ 53 ] = 10079630097836378147ULL;
   gsHashTable.vbbRandomKeys[ 54 ] = 13701139478742659113ULL;
   gsHashTable.vbbRandomKeys[ 55 ] = 8192683469179632153ULL;
   gsHashTable.vbbRandomKeys[ 56 ] = 71127209098305206ULL;
   gsHashTable.vbbRandomKeys[ 57 ] = 14716305965773944179ULL;
   gsHashTable.vbbRandomKeys[ 58 ] = 3498877393685310710ULL;
   gsHashTable.vbbRandomKeys[ 59 ] = 13786632312116635800ULL;
   gsHashTable.vbbRandomKeys[ 60 ] = 14491929101080280037ULL;
   gsHashTable.vbbRandomKeys[ 61 ] = 13622923582448834427ULL;
   gsHashTable.vbbRandomKeys[ 62 ] = 3421413570544993410ULL;
   gsHashTable.vbbRandomKeys[ 63 ] = 1975174938213371089ULL;
   gsHashTable.vbbRandomKeys[ 64 ] = 15631236566978425604ULL;
   gsHashTable.vbbRandomKeys[ 65 ] = 11269118174198131671ULL;
   gsHashTable.vbbRandomKeys[ 66 ] = 5649831898311974713ULL;
   gsHashTable.vbbRandomKeys[ 67 ] = 4221767994904442824ULL;
   gsHashTable.vbbRandomKeys[ 68 ] = 8783724825689332167ULL;
   gsHashTable.vbbRandomKeys[ 69 ] = 115441443530029035ULL;
   gsHashTable.vbbRandomKeys[ 70 ] = 17981259982952871599ULL;
   gsHashTable.vbbRandomKeys[ 71 ] = 5986860220222532696ULL;
   gsHashTable.vbbRandomKeys[ 72 ] = 16699741404889962921ULL;
   gsHashTable.vbbRandomKeys[ 73 ] = 14263378855515925081ULL;
   gsHashTable.vbbRandomKeys[ 74 ] = 10911432543843778548ULL;
   gsHashTable.vbbRandomKeys[ 75 ] = 13385088849281539019ULL;
   gsHashTable.vbbRandomKeys[ 76 ] = 15148358834241027995ULL;
   gsHashTable.vbbRandomKeys[ 77 ] = 15958591445201563278ULL;
   gsHashTable.vbbRandomKeys[ 78 ] = 1835134974859575952ULL;
   gsHashTable.vbbRandomKeys[ 79 ] = 7961956497720207380ULL;
   gsHashTable.vbbRandomKeys[ 80 ] = 12885358504040285515ULL;
   gsHashTable.vbbRandomKeys[ 81 ] = 14777842447256309910ULL;
   gsHashTable.vbbRandomKeys[ 82 ] = 6287924485600403893ULL;
   gsHashTable.vbbRandomKeys[ 83 ] = 5560211958396438137ULL;
   gsHashTable.vbbRandomKeys[ 84 ] = 18426050403864099492ULL;
   gsHashTable.vbbRandomKeys[ 85 ] = 17739561510201804606ULL;
   gsHashTable.vbbRandomKeys[ 86 ] = 8143248515724258973ULL;
   gsHashTable.vbbRandomKeys[ 87 ] = 976845348288513367ULL;
   gsHashTable.vbbRandomKeys[ 88 ] = 2662748885775523881ULL;
   gsHashTable.vbbRandomKeys[ 89 ] = 2709088665022941349ULL;
   gsHashTable.vbbRandomKeys[ 90 ] = 10684462989696293633ULL;
   gsHashTable.vbbRandomKeys[ 91 ] = 7199042308857076158ULL;
   gsHashTable.vbbRandomKeys[ 92 ] = 6582027593128207596ULL;
   gsHashTable.vbbRandomKeys[ 93 ] = 8189636606085609321ULL;
   gsHashTable.vbbRandomKeys[ 94 ] = 13892710499392706057ULL;
   gsHashTable.vbbRandomKeys[ 95 ] = 6658428705083593830ULL;
   gsHashTable.vbbRandomKeys[ 96 ] = 16219376670579898698ULL;
   gsHashTable.vbbRandomKeys[ 97 ] = 12949376600498868586ULL;
   gsHashTable.vbbRandomKeys[ 98 ] = 17284857596287908000ULL;
   gsHashTable.vbbRandomKeys[ 99 ] = 8781347084849184461ULL;
   gsHashTable.vbbRandomKeys[ 100 ] = 11685849876061585859ULL;
   gsHashTable.vbbRandomKeys[ 101 ] = 14014166370621317114ULL;
   gsHashTable.vbbRandomKeys[ 102 ] = 1853551609357334224ULL;
   gsHashTable.vbbRandomKeys[ 103 ] = 2464656721224973028ULL;
   gsHashTable.vbbRandomKeys[ 104 ] = 15862732107410517397ULL;
   gsHashTable.vbbRandomKeys[ 105 ] = 14634335726527321380ULL;
   gsHashTable.vbbRandomKeys[ 106 ] = 1386847398902725516ULL;
   gsHashTable.vbbRandomKeys[ 107 ] = 5257757431874371424ULL;
   gsHashTable.vbbRandomKeys[ 108 ] = 283053926336771110ULL;
   gsHashTable.vbbRandomKeys[ 109 ] = 12435510189066653500ULL;
   gsHashTable.vbbRandomKeys[ 110 ] = 5946244702078260643ULL;
   gsHashTable.vbbRandomKeys[ 111 ] = 16589105116111004170ULL;
   gsHashTable.vbbRandomKeys[ 112 ] = 16184132748602693483ULL;
   gsHashTable.vbbRandomKeys[ 113 ] = 12623532889688181458ULL;
   gsHashTable.vbbRandomKeys[ 114 ] = 17352043466912814336ULL;
   gsHashTable.vbbRandomKeys[ 115 ] = 1688525684216809943ULL;
   gsHashTable.vbbRandomKeys[ 116 ] = 3024352167265828814ULL;
   gsHashTable.vbbRandomKeys[ 117 ] = 3791468294708056053ULL;
   gsHashTable.vbbRandomKeys[ 118 ] = 17757703511814922732ULL;
   gsHashTable.vbbRandomKeys[ 119 ] = 11280700411732542229ULL;
   gsHashTable.vbbRandomKeys[ 120 ] = 11322008415086027197ULL;
   gsHashTable.vbbRandomKeys[ 121 ] = 14886180053689320936ULL;
   gsHashTable.vbbRandomKeys[ 122 ] = 1890121798009321925ULL;
   gsHashTable.vbbRandomKeys[ 123 ] = 151366907903016629ULL;
   gsHashTable.vbbRandomKeys[ 124 ] = 11820963556777049053ULL;
   gsHashTable.vbbRandomKeys[ 125 ] = 465967135617106496ULL;
   gsHashTable.vbbRandomKeys[ 126 ] = 1173955523072332698ULL;
   gsHashTable.vbbRandomKeys[ 127 ] = 5916079091746291288ULL;
   gsHashTable.vbbRandomKeys[ 128 ] = 8653772674023417462ULL;
   gsHashTable.vbbRandomKeys[ 129 ] = 12748387629534233511ULL;
   gsHashTable.vbbRandomKeys[ 130 ] = 12252994478093091835ULL;
   gsHashTable.vbbRandomKeys[ 131 ] = 12461857806472829458ULL;
   gsHashTable.vbbRandomKeys[ 132 ] = 1709199067063832321ULL;
   gsHashTable.vbbRandomKeys[ 133 ] = 6668344926470687540ULL;
   gsHashTable.vbbRandomKeys[ 134 ] = 15815600854590138520ULL;
   gsHashTable.vbbRandomKeys[ 135 ] = 12695502796830378126ULL;
   gsHashTable.vbbRandomKeys[ 136 ] = 12153540460622051322ULL;
   gsHashTable.vbbRandomKeys[ 137 ] = 7175423485251423620ULL;
   gsHashTable.vbbRandomKeys[ 138 ] = 1897136785990571731ULL;
   gsHashTable.vbbRandomKeys[ 139 ] = 463407426052211390ULL;
   gsHashTable.vbbRandomKeys[ 140 ] = 6894668742045483267ULL;
   gsHashTable.vbbRandomKeys[ 141 ] = 16694474718386414950ULL;
   gsHashTable.vbbRandomKeys[ 142 ] = 5176599940548795448ULL;
   gsHashTable.vbbRandomKeys[ 143 ] = 8044525157766773404ULL;
   gsHashTable.vbbRandomKeys[ 144 ] = 15769634638604376500ULL;
   gsHashTable.vbbRandomKeys[ 145 ] = 17131441059169170643ULL;
   gsHashTable.vbbRandomKeys[ 146 ] = 15883127658452287307ULL;
   gsHashTable.vbbRandomKeys[ 147 ] = 11749621619917248544ULL;
   gsHashTable.vbbRandomKeys[ 148 ] = 7504472520185788887ULL;
   gsHashTable.vbbRandomKeys[ 149 ] = 6153965901076564017ULL;
   gsHashTable.vbbRandomKeys[ 150 ] = 928445363607578965ULL;
   gsHashTable.vbbRandomKeys[ 151 ] = 14563452617670965426ULL;
   gsHashTable.vbbRandomKeys[ 152 ] = 16709359376109501421ULL;
   gsHashTable.vbbRandomKeys[ 153 ] = 1618228431386014929ULL;
   gsHashTable.vbbRandomKeys[ 154 ] = 5608701459468607089ULL;
   gsHashTable.vbbRandomKeys[ 155 ] = 759281347879418618ULL;
   gsHashTable.vbbRandomKeys[ 156 ] = 12756160760667167844ULL;
   gsHashTable.vbbRandomKeys[ 157 ] = 14714245548412428991ULL;
   gsHashTable.vbbRandomKeys[ 158 ] = 13427732340878524584ULL;
   gsHashTable.vbbRandomKeys[ 159 ] = 11032144747053841450ULL;
   gsHashTable.vbbRandomKeys[ 160 ] = 2154534722210676336ULL;
   gsHashTable.vbbRandomKeys[ 161 ] = 5122340863223504183ULL;
   gsHashTable.vbbRandomKeys[ 162 ] = 17400864600770803864ULL;
   gsHashTable.vbbRandomKeys[ 163 ] = 13206475456165470062ULL;
   gsHashTable.vbbRandomKeys[ 164 ] = 4717918801002464053ULL;
   gsHashTable.vbbRandomKeys[ 165 ] = 2057546224846249930ULL;
   gsHashTable.vbbRandomKeys[ 166 ] = 16415205182510692527ULL;
   gsHashTable.vbbRandomKeys[ 167 ] = 387281517301493335ULL;
   gsHashTable.vbbRandomKeys[ 168 ] = 9770797717351808290ULL;
   gsHashTable.vbbRandomKeys[ 169 ] = 4330782717792696916ULL;
   gsHashTable.vbbRandomKeys[ 170 ] = 17190510560590230790ULL;
   gsHashTable.vbbRandomKeys[ 171 ] = 14963743919639659770ULL;
   gsHashTable.vbbRandomKeys[ 172 ] = 5563952063838493762ULL;
   gsHashTable.vbbRandomKeys[ 173 ] = 12010887833691968949ULL;
   gsHashTable.vbbRandomKeys[ 174 ] = 2292988140517973496ULL;
   gsHashTable.vbbRandomKeys[ 175 ] = 17655904365783758241ULL;
   gsHashTable.vbbRandomKeys[ 176 ] = 3489468710120115707ULL;
   gsHashTable.vbbRandomKeys[ 177 ] = 4430640583843063438ULL;
   gsHashTable.vbbRandomKeys[ 178 ] = 2916893870121568004ULL;
   gsHashTable.vbbRandomKeys[ 179 ] = 18405371841364968435ULL;
   gsHashTable.vbbRandomKeys[ 180 ] = 15591421984591757264ULL;
   gsHashTable.vbbRandomKeys[ 181 ] = 1883594594455465681ULL;
   gsHashTable.vbbRandomKeys[ 182 ] = 9066226989811406923ULL;
   gsHashTable.vbbRandomKeys[ 183 ] = 5542731965854158349ULL;
   gsHashTable.vbbRandomKeys[ 184 ] = 3073770483931687892ULL;
   gsHashTable.vbbRandomKeys[ 185 ] = 1842248390712116653ULL;
   gsHashTable.vbbRandomKeys[ 186 ] = 15382259679143197757ULL;
   gsHashTable.vbbRandomKeys[ 187 ] = 17160178335322656369ULL;
   gsHashTable.vbbRandomKeys[ 188 ] = 6212373600052167078ULL;
   gsHashTable.vbbRandomKeys[ 189 ] = 14581671181268223261ULL;
   gsHashTable.vbbRandomKeys[ 190 ] = 12658485879843455472ULL;
   gsHashTable.vbbRandomKeys[ 191 ] = 11858908553690145779ULL;
   gsHashTable.vbbRandomKeys[ 192 ] = 13846527487904627475ULL;
   gsHashTable.vbbRandomKeys[ 193 ] = 17730873393267436611ULL;
   gsHashTable.vbbRandomKeys[ 194 ] = 3340900340542698407ULL;
   gsHashTable.vbbRandomKeys[ 195 ] = 16287619699920330388ULL;
   gsHashTable.vbbRandomKeys[ 196 ] = 13981268605706299935ULL;
   gsHashTable.vbbRandomKeys[ 197 ] = 9590035561708822464ULL;
   gsHashTable.vbbRandomKeys[ 198 ] = 13661930529097191454ULL;
   gsHashTable.vbbRandomKeys[ 199 ] = 9288643624708755173ULL;
   gsHashTable.vbbRandomKeys[ 200 ] = 8911775425816798898ULL;
   gsHashTable.vbbRandomKeys[ 201 ] = 1200599218775333085ULL;
   gsHashTable.vbbRandomKeys[ 202 ] = 3563294164938379510ULL;
   gsHashTable.vbbRandomKeys[ 203 ] = 11302611871828470153ULL;
   gsHashTable.vbbRandomKeys[ 204 ] = 14547525872618001768ULL;
   gsHashTable.vbbRandomKeys[ 205 ] = 13686249026255986070ULL;
   gsHashTable.vbbRandomKeys[ 206 ] = 9256186612199032643ULL;
   gsHashTable.vbbRandomKeys[ 207 ] = 946525180231834586ULL;
   gsHashTable.vbbRandomKeys[ 208 ] = 10112394073579361573ULL;
   gsHashTable.vbbRandomKeys[ 209 ] = 7570316668447289715ULL;
   gsHashTable.vbbRandomKeys[ 210 ] = 15033307218694241963ULL;
   gsHashTable.vbbRandomKeys[ 211 ] = 1116132907257488754ULL;
   gsHashTable.vbbRandomKeys[ 212 ] = 6443806979863662448ULL;
   gsHashTable.vbbRandomKeys[ 213 ] = 7129975447970775031ULL;
   gsHashTable.vbbRandomKeys[ 214 ] = 48751080626640451ULL;
   gsHashTable.vbbRandomKeys[ 215 ] = 10238184278289966472ULL;
   gsHashTable.vbbRandomKeys[ 216 ] = 15075081649572177884ULL;
   gsHashTable.vbbRandomKeys[ 217 ] = 15558937251010864845ULL;
   gsHashTable.vbbRandomKeys[ 218 ] = 11967937246072603505ULL;
   gsHashTable.vbbRandomKeys[ 219 ] = 15516838899388883080ULL;
   gsHashTable.vbbRandomKeys[ 220 ] = 15513139612648968612ULL;
   gsHashTable.vbbRandomKeys[ 221 ] = 2222896532179260816ULL;
   gsHashTable.vbbRandomKeys[ 222 ] = 8489483746095221736ULL;
   gsHashTable.vbbRandomKeys[ 223 ] = 4330299715914659173ULL;
   gsHashTable.vbbRandomKeys[ 224 ] = 11472696857521874412ULL;
   gsHashTable.vbbRandomKeys[ 225 ] = 10312284560818484338ULL;
   gsHashTable.vbbRandomKeys[ 226 ] = 1867815146558769700ULL;
   gsHashTable.vbbRandomKeys[ 227 ] = 600249699052379429ULL;
   gsHashTable.vbbRandomKeys[ 228 ] = 14040612342337482318ULL;
   gsHashTable.vbbRandomKeys[ 229 ] = 16274543711566352996ULL;
   gsHashTable.vbbRandomKeys[ 230 ] = 1895785172263718604ULL;
   gsHashTable.vbbRandomKeys[ 231 ] = 16661291268273584085ULL;
   gsHashTable.vbbRandomKeys[ 232 ] = 5148874725156059137ULL;
   gsHashTable.vbbRandomKeys[ 233 ] = 6582123155087367143ULL;
   gsHashTable.vbbRandomKeys[ 234 ] = 15807340424866308897ULL;
   gsHashTable.vbbRandomKeys[ 235 ] = 17903031853855775229ULL;
   gsHashTable.vbbRandomKeys[ 236 ] = 1085916237822101987ULL;
   gsHashTable.vbbRandomKeys[ 237 ] = 2126800388291214182ULL;
   gsHashTable.vbbRandomKeys[ 238 ] = 8442697267907828706ULL;
   gsHashTable.vbbRandomKeys[ 239 ] = 13453965647616689582ULL;
   gsHashTable.vbbRandomKeys[ 240 ] = 17582224846569265504ULL;
   gsHashTable.vbbRandomKeys[ 241 ] = 4279028351987032376ULL;
   gsHashTable.vbbRandomKeys[ 242 ] = 4425400614262658792ULL;
   gsHashTable.vbbRandomKeys[ 243 ] = 4803395348995701790ULL;
   gsHashTable.vbbRandomKeys[ 244 ] = 6608131653019013916ULL;
   gsHashTable.vbbRandomKeys[ 245 ] = 2438431347235665323ULL;
   gsHashTable.vbbRandomKeys[ 246 ] = 4051885490660939060ULL;
   gsHashTable.vbbRandomKeys[ 247 ] = 436438476925012381ULL;
   gsHashTable.vbbRandomKeys[ 248 ] = 6460020123108651076ULL;
   gsHashTable.vbbRandomKeys[ 249 ] = 11173625625074531694ULL;
   gsHashTable.vbbRandomKeys[ 250 ] = 2185348500754117524ULL;
   gsHashTable.vbbRandomKeys[ 251 ] = 8480723964983527620ULL;
   gsHashTable.vbbRandomKeys[ 252 ] = 2457303879165271190ULL;
   gsHashTable.vbbRandomKeys[ 253 ] = 13104600897885889221ULL;
   gsHashTable.vbbRandomKeys[ 254 ] = 4173045397401310305ULL;
   gsHashTable.vbbRandomKeys[ 255 ] = 11365133834883247761ULL;
   gsHashTable.vbbRandomKeys[ 256 ] = 63804126074410096ULL;
   gsHashTable.vbbRandomKeys[ 257 ] = 11161906508689137836ULL;
   gsHashTable.vbbRandomKeys[ 258 ] = 15016599928566609653ULL;
   gsHashTable.vbbRandomKeys[ 259 ] = 16269029863924382692ULL;
   gsHashTable.vbbRandomKeys[ 260 ] = 8162753980902932322ULL;
   gsHashTable.vbbRandomKeys[ 261 ] = 1849333294556510852ULL;
   gsHashTable.vbbRandomKeys[ 262 ] = 7252153461718139666ULL;
   gsHashTable.vbbRandomKeys[ 263 ] = 3936383861584655204ULL;
   gsHashTable.vbbRandomKeys[ 264 ] = 11054933813022310257ULL;
   gsHashTable.vbbRandomKeys[ 265 ] = 151807195329410555ULL;
   gsHashTable.vbbRandomKeys[ 266 ] = 2370590319620426760ULL;
   gsHashTable.vbbRandomKeys[ 267 ] = 14699287662653919329ULL;
   gsHashTable.vbbRandomKeys[ 268 ] = 6723716622327502200ULL;
   gsHashTable.vbbRandomKeys[ 269 ] = 2212040317801727931ULL;
   gsHashTable.vbbRandomKeys[ 270 ] = 9114942237622257130ULL;
   gsHashTable.vbbRandomKeys[ 271 ] = 10408762504841682934ULL;
   gsHashTable.vbbRandomKeys[ 272 ] = 6905119390488045507ULL;
   gsHashTable.vbbRandomKeys[ 273 ] = 8209185801436321534ULL;
   gsHashTable.vbbRandomKeys[ 274 ] = 10613424755411090100ULL;
   gsHashTable.vbbRandomKeys[ 275 ] = 17172956176212519599ULL;
   gsHashTable.vbbRandomKeys[ 276 ] = 1037664165871214517ULL;
   gsHashTable.vbbRandomKeys[ 277 ] = 15620908131366839787ULL;
   gsHashTable.vbbRandomKeys[ 278 ] = 10804425254167825044ULL;
   gsHashTable.vbbRandomKeys[ 279 ] = 15438184041890710517ULL;
   gsHashTable.vbbRandomKeys[ 280 ] = 16640332963902778823ULL;
   gsHashTable.vbbRandomKeys[ 281 ] = 5230787705124406241ULL;
   gsHashTable.vbbRandomKeys[ 282 ] = 8622332490324061299ULL;
   gsHashTable.vbbRandomKeys[ 283 ] = 8356086214475649940ULL;
   gsHashTable.vbbRandomKeys[ 284 ] = 12046796118178695396ULL;
   gsHashTable.vbbRandomKeys[ 285 ] = 8180403113820970145ULL;
   gsHashTable.vbbRandomKeys[ 286 ] = 15402660407473215632ULL;
   gsHashTable.vbbRandomKeys[ 287 ] = 7018207125954560227ULL;
   gsHashTable.vbbRandomKeys[ 288 ] = 11683389668466580844ULL;
   gsHashTable.vbbRandomKeys[ 289 ] = 18087717269769926850ULL;
   gsHashTable.vbbRandomKeys[ 290 ] = 13760796120271330061ULL;
   gsHashTable.vbbRandomKeys[ 291 ] = 5400261231525542676ULL;
   gsHashTable.vbbRandomKeys[ 292 ] = 3563156082204254616ULL;
   gsHashTable.vbbRandomKeys[ 293 ] = 8498995909576272292ULL;
   gsHashTable.vbbRandomKeys[ 294 ] = 10202104521817394451ULL;
   gsHashTable.vbbRandomKeys[ 295 ] = 9909852847406304237ULL;
   gsHashTable.vbbRandomKeys[ 296 ] = 10769805475914925418ULL;
   gsHashTable.vbbRandomKeys[ 297 ] = 14990567919135282638ULL;
   gsHashTable.vbbRandomKeys[ 298 ] = 5333032281237107047ULL;
   gsHashTable.vbbRandomKeys[ 299 ] = 16769638380885260543ULL;
   gsHashTable.vbbRandomKeys[ 300 ] = 139768483873232302ULL;
   gsHashTable.vbbRandomKeys[ 301 ] = 11256621163219843430ULL;
   gsHashTable.vbbRandomKeys[ 302 ] = 3583932678020305713ULL;
   gsHashTable.vbbRandomKeys[ 303 ] = 1826468160408788736ULL;
   gsHashTable.vbbRandomKeys[ 304 ] = 5847292296893332391ULL;
   gsHashTable.vbbRandomKeys[ 305 ] = 1577832050664049892ULL;
   gsHashTable.vbbRandomKeys[ 306 ] = 18001897581465246260ULL;
   gsHashTable.vbbRandomKeys[ 307 ] = 3749208007554556854ULL;
   gsHashTable.vbbRandomKeys[ 308 ] = 4041834233113872302ULL;
   gsHashTable.vbbRandomKeys[ 309 ] = 15495920083068704086ULL;
   gsHashTable.vbbRandomKeys[ 310 ] = 4299826711031831966ULL;
   gsHashTable.vbbRandomKeys[ 311 ] = 14275445904488267730ULL;
   gsHashTable.vbbRandomKeys[ 312 ] = 12619069218030256672ULL;
   gsHashTable.vbbRandomKeys[ 313 ] = 6123482047278803083ULL;
   gsHashTable.vbbRandomKeys[ 314 ] = 7831896195366358724ULL;
   gsHashTable.vbbRandomKeys[ 315 ] = 9355867766055400602ULL;
   gsHashTable.vbbRandomKeys[ 316 ] = 6360457979159859757ULL;
   gsHashTable.vbbRandomKeys[ 317 ] = 13845980212380040629ULL;
   gsHashTable.vbbRandomKeys[ 318 ] = 6741183028744970162ULL;
   gsHashTable.vbbRandomKeys[ 319 ] = 13920484680734084087ULL;
   gsHashTable.vbbRandomKeys[ 320 ] = 2978948904727153209ULL;
   gsHashTable.vbbRandomKeys[ 321 ] = 12926677860505600659ULL;
   gsHashTable.vbbRandomKeys[ 322 ] = 8304497923989126972ULL;
   gsHashTable.vbbRandomKeys[ 323 ] = 7127763959372691259ULL;
   gsHashTable.vbbRandomKeys[ 324 ] = 7228913571138267757ULL;
   gsHashTable.vbbRandomKeys[ 325 ] = 5440300756491697597ULL;
   gsHashTable.vbbRandomKeys[ 326 ] = 10643663359368845678ULL;
   gsHashTable.vbbRandomKeys[ 327 ] = 2287225428107476467ULL;
   gsHashTable.vbbRandomKeys[ 328 ] = 6049777262784860281ULL;
   gsHashTable.vbbRandomKeys[ 329 ] = 12334542464283704697ULL;
   gsHashTable.vbbRandomKeys[ 330 ] = 10403174850142488320ULL;
   gsHashTable.vbbRandomKeys[ 331 ] = 5607608845535956315ULL;
   gsHashTable.vbbRandomKeys[ 332 ] = 15264547612286183582ULL;
   gsHashTable.vbbRandomKeys[ 333 ] = 841816041152082174ULL;
   gsHashTable.vbbRandomKeys[ 334 ] = 7391815064494572602ULL;
   gsHashTable.vbbRandomKeys[ 335 ] = 18367820465571178166ULL;
   gsHashTable.vbbRandomKeys[ 336 ] = 15006891588643797097ULL;
   gsHashTable.vbbRandomKeys[ 337 ] = 6822335996487016210ULL;
   gsHashTable.vbbRandomKeys[ 338 ] = 17801208619149606569ULL;
   gsHashTable.vbbRandomKeys[ 339 ] = 1995376138012955932ULL;
   gsHashTable.vbbRandomKeys[ 340 ] = 16407241925450618970ULL;
   gsHashTable.vbbRandomKeys[ 341 ] = 4311128145234531303ULL;
   gsHashTable.vbbRandomKeys[ 342 ] = 146135625377058522ULL;
   gsHashTable.vbbRandomKeys[ 343 ] = 2232778168035379839ULL;
   gsHashTable.vbbRandomKeys[ 344 ] = 6825540131995277676ULL;
   gsHashTable.vbbRandomKeys[ 345 ] = 480716391780806ULL;
   gsHashTable.vbbRandomKeys[ 346 ] = 12534200477889622857ULL;
   gsHashTable.vbbRandomKeys[ 347 ] = 387792468727484856ULL;
   gsHashTable.vbbRandomKeys[ 348 ] = 14519474273395506879ULL;
   gsHashTable.vbbRandomKeys[ 349 ] = 12931318539001361278ULL;
   gsHashTable.vbbRandomKeys[ 350 ] = 9318211821033394160ULL;
   gsHashTable.vbbRandomKeys[ 351 ] = 15752028604028896253ULL;
   gsHashTable.vbbRandomKeys[ 352 ] = 14345729832159189576ULL;
   gsHashTable.vbbRandomKeys[ 353 ] = 16500706173742864724ULL;
   gsHashTable.vbbRandomKeys[ 354 ] = 15178895521184224361ULL;
   gsHashTable.vbbRandomKeys[ 355 ] = 6573370076250518824ULL;
   gsHashTable.vbbRandomKeys[ 356 ] = 12157161219685457186ULL;
   gsHashTable.vbbRandomKeys[ 357 ] = 16237073210848684183ULL;
   gsHashTable.vbbRandomKeys[ 358 ] = 3916536503640903980ULL;
   gsHashTable.vbbRandomKeys[ 359 ] = 11968579260449086672ULL;
   gsHashTable.vbbRandomKeys[ 360 ] = 16826350101892209356ULL;
   gsHashTable.vbbRandomKeys[ 361 ] = 8708524667094192573ULL;
   gsHashTable.vbbRandomKeys[ 362 ] = 4080300613527203560ULL;
   gsHashTable.vbbRandomKeys[ 363 ] = 3187595835993090121ULL;
   gsHashTable.vbbRandomKeys[ 364 ] = 9983166862425865875ULL;
   gsHashTable.vbbRandomKeys[ 365 ] = 13821589933414074524ULL;
   gsHashTable.vbbRandomKeys[ 366 ] = 1117302542172311464ULL;
   gsHashTable.vbbRandomKeys[ 367 ] = 8007288322808096406ULL;
   gsHashTable.vbbRandomKeys[ 368 ] = 11440349626545726072ULL;
   gsHashTable.vbbRandomKeys[ 369 ] = 6801486488047538494ULL;
   gsHashTable.vbbRandomKeys[ 370 ] = 12132389990626565662ULL;
   gsHashTable.vbbRandomKeys[ 371 ] = 8146843448520574479ULL;
   gsHashTable.vbbRandomKeys[ 372 ] = 7360417401812417961ULL;
   gsHashTable.vbbRandomKeys[ 373 ] = 4519962351738970045ULL;
   gsHashTable.vbbRandomKeys[ 374 ] = 13583528725134400826ULL;
   gsHashTable.vbbRandomKeys[ 375 ] = 1690852946580328919ULL;
   gsHashTable.vbbRandomKeys[ 376 ] = 4011536040352710982ULL;
   gsHashTable.vbbRandomKeys[ 377 ] = 2932754063635089095ULL;
   gsHashTable.vbbRandomKeys[ 378 ] = 8523267475921324151ULL;
   gsHashTable.vbbRandomKeys[ 379 ] = 12214262480892667683ULL;
   gsHashTable.vbbRandomKeys[ 380 ] = 16391108672921117265ULL;
   gsHashTable.vbbRandomKeys[ 381 ] = 1109729969817899442ULL;
   gsHashTable.vbbRandomKeys[ 382 ] = 6840216412034469787ULL;
   gsHashTable.vbbRandomKeys[ 383 ] = 6678154308523457823ULL;
   gsHashTable.vbbRandomKeys[ 384 ] = 15462195022270978258ULL;
   gsHashTable.vbbRandomKeys[ 385 ] = 7932211405693026916ULL;
   gsHashTable.vbbRandomKeys[ 386 ] = 5521111321402668195ULL;
   gsHashTable.vbbRandomKeys[ 387 ] = 6273060676892249842ULL;
   gsHashTable.vbbRandomKeys[ 388 ] = 5379077922461368958ULL;
   gsHashTable.vbbRandomKeys[ 389 ] = 5961179897323766817ULL;
   gsHashTable.vbbRandomKeys[ 390 ] = 16938785341374763767ULL;
   gsHashTable.vbbRandomKeys[ 391 ] = 16823897401208150167ULL;
   gsHashTable.vbbRandomKeys[ 392 ] = 11207800658017012220ULL;
   gsHashTable.vbbRandomKeys[ 393 ] = 1983641547682912676ULL;
   gsHashTable.vbbRandomKeys[ 394 ] = 16732851709703935604ULL;
   gsHashTable.vbbRandomKeys[ 395 ] = 12333321330626399551ULL;
   gsHashTable.vbbRandomKeys[ 396 ] = 12557239717278413627ULL;
   gsHashTable.vbbRandomKeys[ 397 ] = 711921472326200139ULL;
   gsHashTable.vbbRandomKeys[ 398 ] = 1248958513585921226ULL;
   gsHashTable.vbbRandomKeys[ 399 ] = 12550937280854436547ULL;
   gsHashTable.vbbRandomKeys[ 400 ] = 14669033909621100883ULL;
   gsHashTable.vbbRandomKeys[ 401 ] = 9466268398626418938ULL;
   gsHashTable.vbbRandomKeys[ 402 ] = 13432810597552324109ULL;
   gsHashTable.vbbRandomKeys[ 403 ] = 5343148220256586368ULL;
   gsHashTable.vbbRandomKeys[ 404 ] = 12628375941092352036ULL;
   gsHashTable.vbbRandomKeys[ 405 ] = 6274488562225975663ULL;
   gsHashTable.vbbRandomKeys[ 406 ] = 10091146402508099078ULL;
   gsHashTable.vbbRandomKeys[ 407 ] = 6735366233725645197ULL;
   gsHashTable.vbbRandomKeys[ 408 ] = 14070396363361601407ULL;
   gsHashTable.vbbRandomKeys[ 409 ] = 5691273042477788423ULL;
   gsHashTable.vbbRandomKeys[ 410 ] = 9899249355509703130ULL;
   gsHashTable.vbbRandomKeys[ 411 ] = 4168575018978437633ULL;
   gsHashTable.vbbRandomKeys[ 412 ] = 7123371073236794656ULL;
   gsHashTable.vbbRandomKeys[ 413 ] = 5230587847850043228ULL;
   gsHashTable.vbbRandomKeys[ 414 ] = 3333180680299038160ULL;
   gsHashTable.vbbRandomKeys[ 415 ] = 6814513973395387372ULL;
   gsHashTable.vbbRandomKeys[ 416 ] = 2495707657782111241ULL;
   gsHashTable.vbbRandomKeys[ 417 ] = 10764142983642287384ULL;
   gsHashTable.vbbRandomKeys[ 418 ] = 16243691210665612633ULL;
   gsHashTable.vbbRandomKeys[ 419 ] = 370495672740109600ULL;
   gsHashTable.vbbRandomKeys[ 420 ] = 9548695876324166232ULL;
   gsHashTable.vbbRandomKeys[ 421 ] = 13291990783127785922ULL;
   gsHashTable.vbbRandomKeys[ 422 ] = 4691852551306627882ULL;
   gsHashTable.vbbRandomKeys[ 423 ] = 377789437597219442ULL;
   gsHashTable.vbbRandomKeys[ 424 ] = 12637302334845728008ULL;
   gsHashTable.vbbRandomKeys[ 425 ] = 15745184260535432129ULL;
   gsHashTable.vbbRandomKeys[ 426 ] = 9251852216076414260ULL;
   gsHashTable.vbbRandomKeys[ 427 ] = 13109527194562171534ULL;
   gsHashTable.vbbRandomKeys[ 428 ] = 4712687622708765825ULL;
   gsHashTable.vbbRandomKeys[ 429 ] = 11897937273543026496ULL;
   gsHashTable.vbbRandomKeys[ 430 ] = 10384266748740935321ULL;
   gsHashTable.vbbRandomKeys[ 431 ] = 2339769022993372408ULL;
   gsHashTable.vbbRandomKeys[ 432 ] = 9864906115327565609ULL;
   gsHashTable.vbbRandomKeys[ 433 ] = 16961856502834081231ULL;
   gsHashTable.vbbRandomKeys[ 434 ] = 15564245899588621595ULL;
   gsHashTable.vbbRandomKeys[ 435 ] = 1135328958288613848ULL;
   gsHashTable.vbbRandomKeys[ 436 ] = 9558620628584231074ULL;
   gsHashTable.vbbRandomKeys[ 437 ] = 632884043052786381ULL;
   gsHashTable.vbbRandomKeys[ 438 ] = 8532326043787566476ULL;
   gsHashTable.vbbRandomKeys[ 439 ] = 1359433726221306278ULL;
   gsHashTable.vbbRandomKeys[ 440 ] = 10710205753731219419ULL;
   gsHashTable.vbbRandomKeys[ 441 ] = 17046264454822289123ULL;
   gsHashTable.vbbRandomKeys[ 442 ] = 12555020774262218399ULL;
   gsHashTable.vbbRandomKeys[ 443 ] = 5294579529791718129ULL;
   gsHashTable.vbbRandomKeys[ 444 ] = 12447274105395453054ULL;
   gsHashTable.vbbRandomKeys[ 445 ] = 16672369431365133693ULL;
   gsHashTable.vbbRandomKeys[ 446 ] = 11701847768641363200ULL;
   gsHashTable.vbbRandomKeys[ 447 ] = 17170550045396202732ULL;
   gsHashTable.vbbRandomKeys[ 448 ] = 4490542930025184466ULL;
   gsHashTable.vbbRandomKeys[ 449 ] = 15834100041171453383ULL;
   gsHashTable.vbbRandomKeys[ 450 ] = 15270988561449989866ULL;
   gsHashTable.vbbRandomKeys[ 451 ] = 5668451761626092185ULL;
   gsHashTable.vbbRandomKeys[ 452 ] = 17161290564051505833ULL;
   gsHashTable.vbbRandomKeys[ 453 ] = 7564278081283818305ULL;
   gsHashTable.vbbRandomKeys[ 454 ] = 12729810027329144832ULL;
   gsHashTable.vbbRandomKeys[ 455 ] = 6406004772211203805ULL;
   gsHashTable.vbbRandomKeys[ 456 ] = 17801225335955684145ULL;
   gsHashTable.vbbRandomKeys[ 457 ] = 3363231397499148717ULL;
   gsHashTable.vbbRandomKeys[ 458 ] = 14172287922227380680ULL;
   gsHashTable.vbbRandomKeys[ 459 ] = 11822437593268974367ULL;
   gsHashTable.vbbRandomKeys[ 460 ] = 17962550678788775782ULL;
   gsHashTable.vbbRandomKeys[ 461 ] = 4861620716644554428ULL;
   gsHashTable.vbbRandomKeys[ 462 ] = 663479320492724640ULL;
   gsHashTable.vbbRandomKeys[ 463 ] = 15038372726560660296ULL;
   gsHashTable.vbbRandomKeys[ 464 ] = 15334056276650248315ULL;
   gsHashTable.vbbRandomKeys[ 465 ] = 12877855713504224472ULL;
   gsHashTable.vbbRandomKeys[ 466 ] = 7400323965941855121ULL;
   gsHashTable.vbbRandomKeys[ 467 ] = 7374846568171888216ULL;
   gsHashTable.vbbRandomKeys[ 468 ] = 16219934151174535670ULL;
   gsHashTable.vbbRandomKeys[ 469 ] = 6819090430266045104ULL;
   gsHashTable.vbbRandomKeys[ 470 ] = 5915793383058770127ULL;
   gsHashTable.vbbRandomKeys[ 471 ] = 5481328324649163174ULL;
   gsHashTable.vbbRandomKeys[ 472 ] = 13537741837911538281ULL;
   gsHashTable.vbbRandomKeys[ 473 ] = 11420984484079821882ULL;
   gsHashTable.vbbRandomKeys[ 474 ] = 14384981225261819145ULL;
   gsHashTable.vbbRandomKeys[ 475 ] = 10433351171427581842ULL;
   gsHashTable.vbbRandomKeys[ 476 ] = 8454217209990498148ULL;
   gsHashTable.vbbRandomKeys[ 477 ] = 6936220899744219434ULL;
   gsHashTable.vbbRandomKeys[ 478 ] = 17410798448172204801ULL;
   gsHashTable.vbbRandomKeys[ 479 ] = 5540752735179067256ULL;
   gsHashTable.vbbRandomKeys[ 480 ] = 6683291264837942198ULL;
   gsHashTable.vbbRandomKeys[ 481 ] = 10604430502535205467ULL;
   gsHashTable.vbbRandomKeys[ 482 ] = 3834750191244057521ULL;
   gsHashTable.vbbRandomKeys[ 483 ] = 4102573974495202389ULL;
   gsHashTable.vbbRandomKeys[ 484 ] = 17614451825660521333ULL;
   gsHashTable.vbbRandomKeys[ 485 ] = 12773414607989170461ULL;
   gsHashTable.vbbRandomKeys[ 486 ] = 10641228511198858870ULL;
   gsHashTable.vbbRandomKeys[ 487 ] = 16196391165890649377ULL;
   gsHashTable.vbbRandomKeys[ 488 ] = 1304077062221260649ULL;
   gsHashTable.vbbRandomKeys[ 489 ] = 18123750108214158251ULL;
   gsHashTable.vbbRandomKeys[ 490 ] = 7745484898088222317ULL;
   gsHashTable.vbbRandomKeys[ 491 ] = 18150787232046897296ULL;
   gsHashTable.vbbRandomKeys[ 492 ] = 1611596254753085333ULL;
   gsHashTable.vbbRandomKeys[ 493 ] = 8045216616385771651ULL;
   gsHashTable.vbbRandomKeys[ 494 ] = 13808549922858978704ULL;
   gsHashTable.vbbRandomKeys[ 495 ] = 16108378890346828216ULL;
   gsHashTable.vbbRandomKeys[ 496 ] = 1407213294348378099ULL;
   gsHashTable.vbbRandomKeys[ 497 ] = 13388761525887756758ULL;
   gsHashTable.vbbRandomKeys[ 498 ] = 10288668673086868716ULL;
   gsHashTable.vbbRandomKeys[ 499 ] = 12062998194622282169ULL;
   gsHashTable.vbbRandomKeys[ 500 ] = 2168967169336839131ULL;
   gsHashTable.vbbRandomKeys[ 501 ] = 10763200532822696293ULL;
   gsHashTable.vbbRandomKeys[ 502 ] = 8505786302485354209ULL;
   gsHashTable.vbbRandomKeys[ 503 ] = 17820179250848101614ULL;
   gsHashTable.vbbRandomKeys[ 504 ] = 6437884345877787836ULL;
   gsHashTable.vbbRandomKeys[ 505 ] = 5266453928088178996ULL;
   gsHashTable.vbbRandomKeys[ 506 ] = 11731306319480219669ULL;
   gsHashTable.vbbRandomKeys[ 507 ] = 16912954515892114024ULL;
   gsHashTable.vbbRandomKeys[ 508 ] = 10217713217374574526ULL;
   gsHashTable.vbbRandomKeys[ 509 ] = 5610692145543001066ULL;
   gsHashTable.vbbRandomKeys[ 510 ] = 7140083000022179288ULL;
   gsHashTable.vbbRandomKeys[ 511 ] = 17133361304417786769ULL;
   gsHashTable.vbbRandomKeys[ 512 ] = 6735787271417561845ULL;
   gsHashTable.vbbRandomKeys[ 513 ] = 15879336771283630301ULL;
   gsHashTable.vbbRandomKeys[ 514 ] = 7858931196142548205ULL;
   gsHashTable.vbbRandomKeys[ 515 ] = 12862420241811297053ULL;
   gsHashTable.vbbRandomKeys[ 516 ] = 12285133650635991785ULL;
   gsHashTable.vbbRandomKeys[ 517 ] = 17417646803644721457ULL;
   gsHashTable.vbbRandomKeys[ 518 ] = 936216325614902133ULL;
   gsHashTable.vbbRandomKeys[ 519 ] = 13454994280465408065ULL;
   gsHashTable.vbbRandomKeys[ 520 ] = 17770511996638374287ULL;
   gsHashTable.vbbRandomKeys[ 521 ] = 1066095093704430692ULL;
   gsHashTable.vbbRandomKeys[ 522 ] = 3269142729367898223ULL;
   gsHashTable.vbbRandomKeys[ 523 ] = 18403649462481367650ULL;
   gsHashTable.vbbRandomKeys[ 524 ] = 13627802623331568067ULL;
   gsHashTable.vbbRandomKeys[ 525 ] = 12782592477921769766ULL;
   gsHashTable.vbbRandomKeys[ 526 ] = 4214677265381411434ULL;
   gsHashTable.vbbRandomKeys[ 527 ] = 4064654417285393272ULL;
   gsHashTable.vbbRandomKeys[ 528 ] = 5962280267623677146ULL;
   gsHashTable.vbbRandomKeys[ 529 ] = 17507077312458502872ULL;
   gsHashTable.vbbRandomKeys[ 530 ] = 11036129989614529025ULL;
   gsHashTable.vbbRandomKeys[ 531 ] = 3693567730248790600ULL;
   gsHashTable.vbbRandomKeys[ 532 ] = 7190905165140119873ULL;
   gsHashTable.vbbRandomKeys[ 533 ] = 17581380900557916465ULL;
   gsHashTable.vbbRandomKeys[ 534 ] = 5333492033637306699ULL;
   gsHashTable.vbbRandomKeys[ 535 ] = 14624742332086753900ULL;
   gsHashTable.vbbRandomKeys[ 536 ] = 18350938733083317667ULL;
   gsHashTable.vbbRandomKeys[ 537 ] = 10905527976632169590ULL;
   gsHashTable.vbbRandomKeys[ 538 ] = 4801456446983871146ULL;
   gsHashTable.vbbRandomKeys[ 539 ] = 3266050529504486320ULL;
   gsHashTable.vbbRandomKeys[ 540 ] = 5865160423984896601ULL;
   gsHashTable.vbbRandomKeys[ 541 ] = 1223417422661166166ULL;
   gsHashTable.vbbRandomKeys[ 542 ] = 434381393777217448ULL;
   gsHashTable.vbbRandomKeys[ 543 ] = 7301578196664402864ULL;
   gsHashTable.vbbRandomKeys[ 544 ] = 4597902016408901072ULL;
   gsHashTable.vbbRandomKeys[ 545 ] = 13288351202132573098ULL;
   gsHashTable.vbbRandomKeys[ 546 ] = 10328150304000951788ULL;
   gsHashTable.vbbRandomKeys[ 547 ] = 16154358768539163100ULL;
   gsHashTable.vbbRandomKeys[ 548 ] = 8341483093453518288ULL;
   gsHashTable.vbbRandomKeys[ 549 ] = 15019937017881069024ULL;
   gsHashTable.vbbRandomKeys[ 550 ] = 3199922319887440068ULL;
   gsHashTable.vbbRandomKeys[ 551 ] = 9466685674597639495ULL;
   gsHashTable.vbbRandomKeys[ 552 ] = 2260843730451057181ULL;
   gsHashTable.vbbRandomKeys[ 553 ] = 1824238150244808735ULL;
   gsHashTable.vbbRandomKeys[ 554 ] = 11229003979400982185ULL;
   gsHashTable.vbbRandomKeys[ 555 ] = 3962316491299950406ULL;
   gsHashTable.vbbRandomKeys[ 556 ] = 7145429124507414799ULL;
   gsHashTable.vbbRandomKeys[ 557 ] = 6993884963598058289ULL;
   gsHashTable.vbbRandomKeys[ 558 ] = 874119196077229802ULL;
   gsHashTable.vbbRandomKeys[ 559 ] = 3162874772651062127ULL;
   gsHashTable.vbbRandomKeys[ 560 ] = 14957852747194196030ULL;
   gsHashTable.vbbRandomKeys[ 561 ] = 13926061009820464390ULL;
   gsHashTable.vbbRandomKeys[ 562 ] = 5479841853734785281ULL;
   gsHashTable.vbbRandomKeys[ 563 ] = 16262473193769399526ULL;
   gsHashTable.vbbRandomKeys[ 564 ] = 14204598813043204049ULL;
   gsHashTable.vbbRandomKeys[ 565 ] = 8493869582040325805ULL;
   gsHashTable.vbbRandomKeys[ 566 ] = 4392363342027311998ULL;
   gsHashTable.vbbRandomKeys[ 567 ] = 8236730659329627992ULL;
   gsHashTable.vbbRandomKeys[ 568 ] = 2486787887784337045ULL;
   gsHashTable.vbbRandomKeys[ 569 ] = 1438259652878979479ULL;
   gsHashTable.vbbRandomKeys[ 570 ] = 12910592520114890708ULL;
   gsHashTable.vbbRandomKeys[ 571 ] = 14089457223824596794ULL;
   gsHashTable.vbbRandomKeys[ 572 ] = 13374234815557086233ULL;
   gsHashTable.vbbRandomKeys[ 573 ] = 7320620899483278565ULL;
   gsHashTable.vbbRandomKeys[ 574 ] = 3978693214514368795ULL;
   gsHashTable.vbbRandomKeys[ 575 ] = 3717236585699311422ULL;
   gsHashTable.vbbRandomKeys[ 576 ] = 2608941221941403513ULL;
   gsHashTable.vbbRandomKeys[ 577 ] = 14006703223334955255ULL;
   gsHashTable.vbbRandomKeys[ 578 ] = 1881255124488659029ULL;
   gsHashTable.vbbRandomKeys[ 579 ] = 3800903636494582527ULL;
   gsHashTable.vbbRandomKeys[ 580 ] = 11204942675800277427ULL;
   gsHashTable.vbbRandomKeys[ 581 ] = 17825417559917674922ULL;
   gsHashTable.vbbRandomKeys[ 582 ] = 9174535531020623146ULL;
   gsHashTable.vbbRandomKeys[ 583 ] = 14213211179127315137ULL;
   gsHashTable.vbbRandomKeys[ 584 ] = 14507101922313192387ULL;
   gsHashTable.vbbRandomKeys[ 585 ] = 15543127870003203059ULL;
   gsHashTable.vbbRandomKeys[ 586 ] = 14704485344828382208ULL;
   gsHashTable.vbbRandomKeys[ 587 ] = 8799618389701523964ULL;
   gsHashTable.vbbRandomKeys[ 588 ] = 7994512334209114400ULL;
   gsHashTable.vbbRandomKeys[ 589 ] = 12199618317420952464ULL;
   gsHashTable.vbbRandomKeys[ 590 ] = 11137551833257844433ULL;
   gsHashTable.vbbRandomKeys[ 591 ] = 1677938927126126656ULL;
   gsHashTable.vbbRandomKeys[ 592 ] = 14621391259534309951ULL;
   gsHashTable.vbbRandomKeys[ 593 ] = 11297169633756526499ULL;
   gsHashTable.vbbRandomKeys[ 594 ] = 6569564832007778117ULL;
   gsHashTable.vbbRandomKeys[ 595 ] = 339659959559306900ULL;
   gsHashTable.vbbRandomKeys[ 596 ] = 10264066308000784394ULL;
   gsHashTable.vbbRandomKeys[ 597 ] = 1161055478009014470ULL;
   gsHashTable.vbbRandomKeys[ 598 ] = 12149343690814368795ULL;
   gsHashTable.vbbRandomKeys[ 599 ] = 5994390635656134610ULL;
   gsHashTable.vbbRandomKeys[ 600 ] = 13238856641081644408ULL;
   gsHashTable.vbbRandomKeys[ 601 ] = 16033302142659539088ULL;
   gsHashTable.vbbRandomKeys[ 602 ] = 14388684507424835392ULL;
   gsHashTable.vbbRandomKeys[ 603 ] = 1777267578553527241ULL;
   gsHashTable.vbbRandomKeys[ 604 ] = 18041613971552494045ULL;
   gsHashTable.vbbRandomKeys[ 605 ] = 12472235958415576895ULL;
   gsHashTable.vbbRandomKeys[ 606 ] = 9710678362901299840ULL;
   gsHashTable.vbbRandomKeys[ 607 ] = 11739291760427509428ULL;
   gsHashTable.vbbRandomKeys[ 608 ] = 1216225364132997779ULL;
   gsHashTable.vbbRandomKeys[ 609 ] = 3941727980584103401ULL;
   gsHashTable.vbbRandomKeys[ 610 ] = 14634031960547357335ULL;
   gsHashTable.vbbRandomKeys[ 611 ] = 15395901354824670013ULL;
   gsHashTable.vbbRandomKeys[ 612 ] = 11467994748494269665ULL;
   gsHashTable.vbbRandomKeys[ 613 ] = 13889241926071813622ULL;
   gsHashTable.vbbRandomKeys[ 614 ] = 739035727687969921ULL;
   gsHashTable.vbbRandomKeys[ 615 ] = 17231804267709361002ULL;
   gsHashTable.vbbRandomKeys[ 616 ] = 4215373595798798494ULL;
   gsHashTable.vbbRandomKeys[ 617 ] = 10504150901472949749ULL;
   gsHashTable.vbbRandomKeys[ 618 ] = 8679193723467083578ULL;
   gsHashTable.vbbRandomKeys[ 619 ] = 11803348387223915686ULL;
   gsHashTable.vbbRandomKeys[ 620 ] = 2379359756692057252ULL;
   gsHashTable.vbbRandomKeys[ 621 ] = 4605482384475317035ULL;
   gsHashTable.vbbRandomKeys[ 622 ] = 1010353084515816675ULL;
   gsHashTable.vbbRandomKeys[ 623 ] = 2497517606085496582ULL;
   gsHashTable.vbbRandomKeys[ 624 ] = 15616420830894623793ULL;
   gsHashTable.vbbRandomKeys[ 625 ] = 3434319739494326420ULL;
   gsHashTable.vbbRandomKeys[ 626 ] = 17069741457771118638ULL;
   gsHashTable.vbbRandomKeys[ 627 ] = 5981782967497335006ULL;
   gsHashTable.vbbRandomKeys[ 628 ] = 5001122469643909553ULL;
   gsHashTable.vbbRandomKeys[ 629 ] = 11599905566257442065ULL;
   gsHashTable.vbbRandomKeys[ 630 ] = 17727351619505203727ULL;
   gsHashTable.vbbRandomKeys[ 631 ] = 16604602388845362308ULL;
   gsHashTable.vbbRandomKeys[ 632 ] = 7431935967310015915ULL;
   gsHashTable.vbbRandomKeys[ 633 ] = 11212381932580776872ULL;
   gsHashTable.vbbRandomKeys[ 634 ] = 2638043397942328569ULL;
   gsHashTable.vbbRandomKeys[ 635 ] = 11918251849269776811ULL;
   gsHashTable.vbbRandomKeys[ 636 ] = 3941018871267158112ULL;
   gsHashTable.vbbRandomKeys[ 637 ] = 14755077219103732577ULL;
   gsHashTable.vbbRandomKeys[ 638 ] = 38943976481445955ULL;
   gsHashTable.vbbRandomKeys[ 639 ] = 13565538522546702499ULL;
   gsHashTable.vbbRandomKeys[ 640 ] = 9705870979313148148ULL;
   gsHashTable.vbbRandomKeys[ 641 ] = 16731429477385924949ULL;
   gsHashTable.vbbRandomKeys[ 642 ] = 802754698401058373ULL;
   gsHashTable.vbbRandomKeys[ 643 ] = 5859328887625108595ULL;
   gsHashTable.vbbRandomKeys[ 644 ] = 8573349314945064048ULL;
   gsHashTable.vbbRandomKeys[ 645 ] = 1366082109386191881ULL;
   gsHashTable.vbbRandomKeys[ 646 ] = 4864362512511525524ULL;
   gsHashTable.vbbRandomKeys[ 647 ] = 1151964852379331268ULL;
   gsHashTable.vbbRandomKeys[ 648 ] = 17392045688673152227ULL;
   gsHashTable.vbbRandomKeys[ 649 ] = 17190322757803815441ULL;
   gsHashTable.vbbRandomKeys[ 650 ] = 13638322019474538514ULL;
   gsHashTable.vbbRandomKeys[ 651 ] = 13393743891064251465ULL;
   gsHashTable.vbbRandomKeys[ 652 ] = 9543889586995202454ULL;
   gsHashTable.vbbRandomKeys[ 653 ] = 8395935506392517064ULL;
   gsHashTable.vbbRandomKeys[ 654 ] = 14415930661618391446ULL;
   gsHashTable.vbbRandomKeys[ 655 ] = 5355501773312198872ULL;
   gsHashTable.vbbRandomKeys[ 656 ] = 10934661199436056295ULL;
   gsHashTable.vbbRandomKeys[ 657 ] = 7659608848262917338ULL;
   gsHashTable.vbbRandomKeys[ 658 ] = 16553112922340276880ULL;
   gsHashTable.vbbRandomKeys[ 659 ] = 10572143000794430237ULL;
   gsHashTable.vbbRandomKeys[ 660 ] = 10497647641077701813ULL;
   gsHashTable.vbbRandomKeys[ 661 ] = 1207807617012930323ULL;
   gsHashTable.vbbRandomKeys[ 662 ] = 8179076230942276001ULL;
   gsHashTable.vbbRandomKeys[ 663 ] = 17277093565345336337ULL;
   gsHashTable.vbbRandomKeys[ 664 ] = 6847275515939942774ULL;
   gsHashTable.vbbRandomKeys[ 665 ] = 14196185070498631750ULL;
   gsHashTable.vbbRandomKeys[ 666 ] = 467287672779915272ULL;
   gsHashTable.vbbRandomKeys[ 667 ] = 13233732212875929072ULL;
   gsHashTable.vbbRandomKeys[ 668 ] = 16578657490136270162ULL;
   gsHashTable.vbbRandomKeys[ 669 ] = 9774463890126697154ULL;
   gsHashTable.vbbRandomKeys[ 670 ] = 13958428880568520635ULL;
   gsHashTable.vbbRandomKeys[ 671 ] = 4019287109102408651ULL;
   gsHashTable.vbbRandomKeys[ 672 ] = 17152213106257554727ULL;
   gsHashTable.vbbRandomKeys[ 673 ] = 3166365097778196402ULL;
   gsHashTable.vbbRandomKeys[ 674 ] = 7316566627464373330ULL;
   gsHashTable.vbbRandomKeys[ 675 ] = 13683390819964593770ULL;
   gsHashTable.vbbRandomKeys[ 676 ] = 7028976381171577227ULL;
   gsHashTable.vbbRandomKeys[ 677 ] = 2715071596548175905ULL;
   gsHashTable.vbbRandomKeys[ 678 ] = 2546794203174789776ULL;
   gsHashTable.vbbRandomKeys[ 679 ] = 4131168688374455886ULL;
   gsHashTable.vbbRandomKeys[ 680 ] = 12042854394675755374ULL;
   gsHashTable.vbbRandomKeys[ 681 ] = 17503579178160605270ULL;
   gsHashTable.vbbRandomKeys[ 682 ] = 10728948009755923058ULL;
   gsHashTable.vbbRandomKeys[ 683 ] = 17278076772175334761ULL;
   gsHashTable.vbbRandomKeys[ 684 ] = 6449223468297355373ULL;
   gsHashTable.vbbRandomKeys[ 685 ] = 7543872698209144130ULL;
   gsHashTable.vbbRandomKeys[ 686 ] = 17077464670670506504ULL;
   gsHashTable.vbbRandomKeys[ 687 ] = 10924120912844579657ULL;
   gsHashTable.vbbRandomKeys[ 688 ] = 12703420672138699025ULL;
   gsHashTable.vbbRandomKeys[ 689 ] = 12927819252207940502ULL;
   gsHashTable.vbbRandomKeys[ 690 ] = 7462414154901210654ULL;
   gsHashTable.vbbRandomKeys[ 691 ] = 13838019486917932965ULL;
   gsHashTable.vbbRandomKeys[ 692 ] = 6472500327961808229ULL;
   gsHashTable.vbbRandomKeys[ 693 ] = 9866902577321020832ULL;
   gsHashTable.vbbRandomKeys[ 694 ] = 1563797348849008616ULL;
   gsHashTable.vbbRandomKeys[ 695 ] = 15633184522225135279ULL;
   gsHashTable.vbbRandomKeys[ 696 ] = 4411398937464551177ULL;
   gsHashTable.vbbRandomKeys[ 697 ] = 1734017110485417527ULL;
   gsHashTable.vbbRandomKeys[ 698 ] = 12441314272213774324ULL;
   gsHashTable.vbbRandomKeys[ 699 ] = 1066407786230842984ULL;
   gsHashTable.vbbRandomKeys[ 700 ] = 6478156725563690490ULL;
   gsHashTable.vbbRandomKeys[ 701 ] = 17086378179193669309ULL;
   gsHashTable.vbbRandomKeys[ 702 ] = 14572068555270591374ULL;
   gsHashTable.vbbRandomKeys[ 703 ] = 6886808589363256775ULL;
   gsHashTable.vbbRandomKeys[ 704 ] = 11755290753610531065ULL;
   gsHashTable.vbbRandomKeys[ 705 ] = 17131188977051811206ULL;
   gsHashTable.vbbRandomKeys[ 706 ] = 16524071407999343412ULL;
   gsHashTable.vbbRandomKeys[ 707 ] = 10373996705263706714ULL;
   gsHashTable.vbbRandomKeys[ 708 ] = 11503115787288575475ULL;
   gsHashTable.vbbRandomKeys[ 709 ] = 9683850348888951678ULL;
   gsHashTable.vbbRandomKeys[ 710 ] = 7277229627429420262ULL;
   gsHashTable.vbbRandomKeys[ 711 ] = 16245005074696261451ULL;
   gsHashTable.vbbRandomKeys[ 712 ] = 7674682503392413631ULL;
   gsHashTable.vbbRandomKeys[ 713 ] = 10139370587039834767ULL;
   gsHashTable.vbbRandomKeys[ 714 ] = 15416465182628392692ULL;
   gsHashTable.vbbRandomKeys[ 715 ] = 14244584779681740221ULL;
   gsHashTable.vbbRandomKeys[ 716 ] = 9972580351325836508ULL;
   gsHashTable.vbbRandomKeys[ 717 ] = 7279797674797966103ULL;
   gsHashTable.vbbRandomKeys[ 718 ] = 11825581315205784744ULL;
   gsHashTable.vbbRandomKeys[ 719 ] = 16945833380386602550ULL;
   gsHashTable.vbbRandomKeys[ 720 ] = 6791896365715253599ULL;
   gsHashTable.vbbRandomKeys[ 721 ] = 6945313208353081657ULL;
   gsHashTable.vbbRandomKeys[ 722 ] = 16294808480662096093ULL;
   gsHashTable.vbbRandomKeys[ 723 ] = 12221239568569296496ULL;
   gsHashTable.vbbRandomKeys[ 724 ] = 11247417372709685536ULL;
   gsHashTable.vbbRandomKeys[ 725 ] = 7938873558142830553ULL;
   gsHashTable.vbbRandomKeys[ 726 ] = 3088317014682678078ULL;
   gsHashTable.vbbRandomKeys[ 727 ] = 9111214889041263155ULL;
   gsHashTable.vbbRandomKeys[ 728 ] = 1265991644320173425ULL;
   gsHashTable.vbbRandomKeys[ 729 ] = 16378441826843806725ULL;
   gsHashTable.vbbRandomKeys[ 730 ] = 9987141752145990129ULL;
   gsHashTable.vbbRandomKeys[ 731 ] = 4712911999066775696ULL;
   gsHashTable.vbbRandomKeys[ 732 ] = 1091683086315350892ULL;
   gsHashTable.vbbRandomKeys[ 733 ] = 13727136165381515233ULL;
   gsHashTable.vbbRandomKeys[ 734 ] = 15795530794364513535ULL;
   gsHashTable.vbbRandomKeys[ 735 ] = 8388682948336516610ULL;
   gsHashTable.vbbRandomKeys[ 736 ] = 16246031262521150924ULL;
   gsHashTable.vbbRandomKeys[ 737 ] = 14550513926044067208ULL;
   gsHashTable.vbbRandomKeys[ 738 ] = 13538296560505696104ULL;
   gsHashTable.vbbRandomKeys[ 739 ] = 16748709196863972138ULL;
   gsHashTable.vbbRandomKeys[ 740 ] = 10122687395401356471ULL;
   gsHashTable.vbbRandomKeys[ 741 ] = 3372511915377055679ULL;
   gsHashTable.vbbRandomKeys[ 742 ] = 13616712054906680649ULL;
   gsHashTable.vbbRandomKeys[ 743 ] = 14175791590636824995ULL;
   gsHashTable.vbbRandomKeys[ 744 ] = 1389405298971986549ULL;
   gsHashTable.vbbRandomKeys[ 745 ] = 10426426967119908001ULL;
   gsHashTable.vbbRandomKeys[ 746 ] = 12358648985599658077ULL;
   gsHashTable.vbbRandomKeys[ 747 ] = 16526806724558558692ULL;
   gsHashTable.vbbRandomKeys[ 748 ] = 14854968871305145772ULL;
   gsHashTable.vbbRandomKeys[ 749 ] = 8509033789540071550ULL;
   gsHashTable.vbbRandomKeys[ 750 ] = 6203636036589020240ULL;
   gsHashTable.vbbRandomKeys[ 751 ] = 2385203061819987805ULL;
   gsHashTable.vbbRandomKeys[ 752 ] = 8205234402520923534ULL;
   gsHashTable.vbbRandomKeys[ 753 ] = 14129550963374704796ULL;
   gsHashTable.vbbRandomKeys[ 754 ] = 15815748951467491001ULL;
   gsHashTable.vbbRandomKeys[ 755 ] = 14443034831437213696ULL;
   gsHashTable.vbbRandomKeys[ 756 ] = 14450737780254747528ULL;
   gsHashTable.vbbRandomKeys[ 757 ] = 310135492839025137ULL;
   gsHashTable.vbbRandomKeys[ 758 ] = 14939551031538396458ULL;
   gsHashTable.vbbRandomKeys[ 759 ] = 700160981297204979ULL;
   gsHashTable.vbbRandomKeys[ 760 ] = 5464631219857791783ULL;
   gsHashTable.vbbRandomKeys[ 761 ] = 10998694675053622467ULL;
   gsHashTable.vbbRandomKeys[ 762 ] = 17736739849700738874ULL;
   gsHashTable.vbbRandomKeys[ 763 ] = 16210024720852160608ULL;
   gsHashTable.vbbRandomKeys[ 764 ] = 2260830677670491392ULL;
   gsHashTable.vbbRandomKeys[ 765 ] = 14326085389032563505ULL;
   gsHashTable.vbbRandomKeys[ 766 ] = 15381665477938131459ULL;
   gsHashTable.vbbRandomKeys[ 767 ] = 10204650436469811106ULL;
   gsHashTable.vbbRandomKeys[ 768 ] = 13310277797079362845ULL;
   gsHashTable.vbbRandomKeys[ 769 ] = 4486253165076233351ULL;
   gsHashTable.vbbRandomKeys[ 770 ] = 3214134056478690477ULL;
   gsHashTable.vbbRandomKeys[ 771 ] = 12072811996655449274ULL;
   gsHashTable.vbbRandomKeys[ 772 ] = 16986164094371584164ULL;
   gsHashTable.vbbRandomKeys[ 773 ] = 16990403225960217102ULL;
   gsHashTable.vbbRandomKeys[ 774 ] = 2500847829156690369ULL;
   gsHashTable.vbbRandomKeys[ 775 ] = 14578363180205550801ULL;
   gsHashTable.vbbRandomKeys[ 776 ] = 2189601008890531998ULL;
   gsHashTable.vbbRandomKeys[ 777 ] = 18113605750591796951ULL;
   gsHashTable.vbbRandomKeys[ 778 ] = 8808929484567163142ULL;
   gsHashTable.vbbRandomKeys[ 779 ] = 5447322282283184429ULL;
   gsHashTable.vbbRandomKeys[ 780 ] = 17953006113929159523ULL;
   gsHashTable.vbbRandomKeys[ 781 ] = 4206650648250308834ULL;
   gsHashTable.vbbRandomKeys[ 782 ] = 764005333152791095ULL;
   gsHashTable.vbbRandomKeys[ 783 ] = 6802054851484128021ULL;
   gsHashTable.vbbRandomKeys[ 784 ] = 10186959537498457575ULL;
   gsHashTable.vbbRandomKeys[ 785 ] = 6013786835328322120ULL;
   gsHashTable.vbbRandomKeys[ 786 ] = 14355486565686183313ULL;
   gsHashTable.vbbRandomKeys[ 787 ] = 12737695732105077560ULL;
   gsHashTable.vbbRandomKeys[ 788 ] = 577070768361750432ULL;
   gsHashTable.vbbRandomKeys[ 789 ] = 17836294213057326572ULL;
   gsHashTable.vbbRandomKeys[ 790 ] = 6292287486412957021ULL;
   gsHashTable.vbbRandomKeys[ 791 ] = 10102663156850393321ULL;
   gsHashTable.vbbRandomKeys[ 792 ] = 15131189335648891829ULL;
   gsHashTable.vbbRandomKeys[ 793 ] = 8444560394021001992ULL;
   gsHashTable.vbbRandomKeys[ 794 ] = 5393496927744779613ULL;
   gsHashTable.vbbRandomKeys[ 795 ] = 3692841011119961827ULL;
   gsHashTable.vbbRandomKeys[ 796 ] = 11224494698699900707ULL;
   gsHashTable.vbbRandomKeys[ 797 ] = 14190738106914624288ULL;
   gsHashTable.vbbRandomKeys[ 798 ] = 15057800283914283200ULL;
   gsHashTable.vbbRandomKeys[ 799 ] = 3787145909357773919ULL;
   gsHashTable.vbbRandomKeys[ 800 ] = 14272432467076396516ULL;
   gsHashTable.vbbRandomKeys[ 801 ] = 10246858249508536232ULL;
   gsHashTable.vbbRandomKeys[ 802 ] = 15303887680985898288ULL;
   gsHashTable.vbbRandomKeys[ 803 ] = 15996367696030052102ULL;
   gsHashTable.vbbRandomKeys[ 804 ] = 17211754503046290657ULL;
   gsHashTable.vbbRandomKeys[ 805 ] = 4768643932683150514ULL;
   gsHashTable.vbbRandomKeys[ 806 ] = 10941609835690510853ULL;
   gsHashTable.vbbRandomKeys[ 807 ] = 14579149437321725037ULL;
   gsHashTable.vbbRandomKeys[ 808 ] = 2793758436475448277ULL;
   gsHashTable.vbbRandomKeys[ 809 ] = 5921838086377620211ULL;
   gsHashTable.vbbRandomKeys[ 810 ] = 14212899169182171597ULL;
   gsHashTable.vbbRandomKeys[ 811 ] = 8873563698559260035ULL;
   gsHashTable.vbbRandomKeys[ 812 ] = 18035910650225975026ULL;
   gsHashTable.vbbRandomKeys[ 813 ] = 12507473593614146414ULL;
   gsHashTable.vbbRandomKeys[ 814 ] = 12712632422104824629ULL;
   gsHashTable.vbbRandomKeys[ 815 ] = 303017059599393366ULL;
   gsHashTable.vbbRandomKeys[ 816 ] = 12699130826967550991ULL;
   gsHashTable.vbbRandomKeys[ 817 ] = 7588112800533069575ULL;
   gsHashTable.vbbRandomKeys[ 818 ] = 16197751598086055561ULL;
   gsHashTable.vbbRandomKeys[ 819 ] = 10992498009490893610ULL;
   gsHashTable.vbbRandomKeys[ 820 ] = 5684436995755082471ULL;
   gsHashTable.vbbRandomKeys[ 821 ] = 13333597431820927435ULL;
   gsHashTable.vbbRandomKeys[ 822 ] = 15774691191107388800ULL;
   gsHashTable.vbbRandomKeys[ 823 ] = 9864548368542560451ULL;
   gsHashTable.vbbRandomKeys[ 824 ] = 14776618675940843523ULL;
   gsHashTable.vbbRandomKeys[ 825 ] = 12543802211375948263ULL;
   gsHashTable.vbbRandomKeys[ 826 ] = 16898926597733160885ULL;
   gsHashTable.vbbRandomKeys[ 827 ] = 11367807395364471738ULL;
   gsHashTable.vbbRandomKeys[ 828 ] = 12601222305507396866ULL;
   gsHashTable.vbbRandomKeys[ 829 ] = 2635365032461521742ULL;
   gsHashTable.vbbRandomKeys[ 830 ] = 14935026512025795195ULL;
   gsHashTable.vbbRandomKeys[ 831 ] = 14502999676582431288ULL;
   gsHashTable.vbbRandomKeys[ 832 ] = 11325998485063304336ULL;
   gsHashTable.vbbRandomKeys[ 833 ] = 9179009070155872364ULL;
   gsHashTable.vbbRandomKeys[ 834 ] = 7920872266765757528ULL;
   gsHashTable.vbbRandomKeys[ 835 ] = 15741154000658882844ULL;
   gsHashTable.vbbRandomKeys[ 836 ] = 10333935457558719007ULL;
   gsHashTable.vbbRandomKeys[ 837 ] = 1861598860104210920ULL;
   gsHashTable.vbbRandomKeys[ 838 ] = 11325941964653808226ULL;
   gsHashTable.vbbRandomKeys[ 839 ] = 9916147574033958671ULL;
   gsHashTable.vbbRandomKeys[ 840 ] = 3253782930022084337ULL;
   gsHashTable.vbbRandomKeys[ 841 ] = 17910717658979510453ULL;
   gsHashTable.vbbRandomKeys[ 842 ] = 5344980397914292860ULL;
   gsHashTable.vbbRandomKeys[ 843 ] = 15788993092722404193ULL;
   gsHashTable.vbbRandomKeys[ 844 ] = 17208086862359937279ULL;

}


//
//---------------------------------------------------------------------------------------------------------
//
// Switch the side to move.
void SwitchSideToMove( struct Board * argsBoard )
{

   // Update the Hash (FIX: Use thread-local hash)
   argsBoard->bbHash = argsBoard->bbHash ^
                        gsHashTable.vbbHashKeysStates[ 1 ];

   // Update the Hash (FIX: Use thread-local hash)
   argsBoard->bbHash = argsBoard->bbHash ^
                        gsHashTable.vbbHashKeysStates[ 0 ];

   // Switch the side to move.
   switch ( argsBoard->siColorToMove )
   {

      case dWhite :
      {
   
         // Switch the color
         argsBoard->siColorToMove = dBlack;

         // Take away the en passant
         argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare = 0;

         break;

      }
      case dBlack :
      {
      
         // Switch the side to move.
         argsBoard->siColorToMove = dWhite;

         // Take away the en passant
         argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare = 0;

      }

   }   

}

//
//--------------------------------------------------------------------------------------------------
//
BitBoard Power( int iBase, int iExponent )
{

   BitBoard bbAnswer = iBase;

   if ( iExponent == 0 )
   {

      return 1;

   }

   for ( int iCounter = 0; iCounter < iExponent - 1; iCounter++ )
   {

      bbAnswer *= iBase;

   }

   return bbAnswer;

}

//
//--------------------------------------------------------------------------------------------------
//
int GetHashScore()
{
   return gsHashTable.iScore;
}
BitBoard GetMaskIndex()
{
   return gsHashTable.bbMaskIndex;
}
BitBoard GetKey()
{
   return GetHash() & GetMaskIndex();
}
BitBoard GetKeyTest()
{
   return gsHashTable.mbbHashKeys[ 1 ][ 16 ];
}

//
//-----------------------------------------------------------------------
//
void GenerateHashFromBoard( struct Board * argsBoard,
                            struct GeneralMove * argsGeneralMoves )
{
   // This function calculates the hash from the board state.
   // It is used after setting up a position from FEN.

   // Debug the inputs
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Reset the hash.
   argsBoard->bbHash = 0;

   // 1. Pieces
   for ( int iSquare = 0; iSquare < 64; iSquare++ )
   {
      int iPiece = argsBoard->mBoard[ iSquare ];
      if ( iPiece != dEmpty )
      {
         argsBoard->bbHash ^= gsHashTable.mbbHashKeys[ iPiece ][ iSquare ];
      }
   }

   // 2. Side to Move
   argsBoard->bbHash ^= gsHashTable.vbbColorToMove[ argsBoard->siColorToMove ];

   // 3. Castling Rights
   if ( argsBoard->bbCastle & (1ULL << 0) ) argsBoard->bbHash ^= gsHashTable.vbbCasteling[0];
   if ( argsBoard->bbCastle & (1ULL << 1) ) argsBoard->bbHash ^= gsHashTable.vbbCasteling[1];
   if ( argsBoard->bbCastle & (1ULL << 2) ) argsBoard->bbHash ^= gsHashTable.vbbCasteling[2];
   if ( argsBoard->bbCastle & (1ULL << 3) ) argsBoard->bbHash ^= gsHashTable.vbbCasteling[3];

   // 4. En Passant
   if ( argsBoard->bbEP )
   {
      int vPos[1];
      Find( argsBoard->bbEP, vPos, argsGeneralMoves );
      argsBoard->bbHash ^= gsHashTable.vbbEnPassant[ vPos[0] ];

   }
   else
   {

   }
   
   // Store as initial hash
   gsHashTable.bbHashInitial = argsBoard->bbHash;
}