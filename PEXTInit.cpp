// Copyright 2025 - Violet Chess Engine
// PEXT-based Move Generation Implementation

#include "MoveGen.h"
#include "Functions.h"
#include "Structures.h"
#include <immintrin.h> // For _pext_u64 and _pdep_u64

// Global arrays for PEXT tables
BitBoard RookMasks[ 64 ];
BitBoard BishopMasks[ 64 ];
BitBoard RookAttacks[ 64 ][ 4096 ];
BitBoard BishopAttacks[ 64 ][ 512 ];

// Helper: Calculate rook mask (relevant occupancy bits, excluding edges)
BitBoard CalcRookMask( int sq )
{
   BitBoard attacks = 0;
   int      r = sq / 8, c = sq % 8;

   // North (excluding top edge)
   for ( int r2 = r + 1; r2 < 7; r2++ )
      attacks |= ( 1ULL << ( r2 * 8 + c ) );

   // South (excluding bottom edge)
   for ( int r2 = r - 1; r2 > 0; r2-- )
      attacks |= ( 1ULL << ( r2 * 8 + c ) );

   // East (excluding right edge)
   for ( int c2 = c + 1; c2 < 7; c2++ )
      attacks |= ( 1ULL << ( r * 8 + c2 ) );

   // West (excluding left edge)
   for ( int c2 = c - 1; c2 > 0; c2-- )
      attacks |= ( 1ULL << ( r * 8 + c2 ) );

   return attacks;
}

// Helper: Calculate bishop mask (relevant occupancy bits, excluding edges)
BitBoard CalcBishopMask( int sq )
{
   BitBoard attacks = 0;
   int      r = sq / 8, c = sq % 8;

   // North-East
   for ( int r2 = r + 1, c2 = c + 1; r2 < 7 && c2 < 7; r2++, c2++ )
      attacks |= ( 1ULL << ( r2 * 8 + c2 ) );

   // North-West
   for ( int r2 = r + 1, c2 = c - 1; r2 < 7 && c2 > 0; r2++, c2-- )
      attacks |= ( 1ULL << ( r2 * 8 + c2 ) );

   // South-East
   for ( int r2 = r - 1, c2 = c + 1; r2 > 0 && c2 < 7; r2--, c2++ )
      attacks |= ( 1ULL << ( r2 * 8 + c2 ) );

   // South-West
   for ( int r2 = r - 1, c2 = c - 1; r2 > 0 && c2 > 0; r2--, c2-- )
      attacks |= ( 1ULL << ( r2 * 8 + c2 ) );

   return attacks;
}

// Classical blocker loop to generate truth table for rook
BitBoard RookAttacksSlow( int sq, BitBoard block )
{
   BitBoard attacks = 0;
   int      r = sq / 8, c = sq % 8;

   // North
   for ( int r2 = r + 1; r2 <= 7; r2++ )
   {
      attacks |= ( 1ULL << ( r2 * 8 + c ) );
      if ( block & ( 1ULL << ( r2 * 8 + c ) ) )
         break;
   }

   // South
   for ( int r2 = r - 1; r2 >= 0; r2-- )
   {
      attacks |= ( 1ULL << ( r2 * 8 + c ) );
      if ( block & ( 1ULL << ( r2 * 8 + c ) ) )
         break;
   }

   // East
   for ( int c2 = c + 1; c2 <= 7; c2++ )
   {
      attacks |= ( 1ULL << ( r * 8 + c2 ) );
      if ( block & ( 1ULL << ( r * 8 + c2 ) ) )
         break;
   }

   // West
   for ( int c2 = c - 1; c2 >= 0; c2-- )
   {
      attacks |= ( 1ULL << ( r * 8 + c2 ) );
      if ( block & ( 1ULL << ( r * 8 + c2 ) ) )
         break;
   }

   return attacks;
}

// Classical blocker loop to generate truth table for bishop
BitBoard BishopAttacksSlow( int sq, BitBoard block )
{
   BitBoard attacks = 0;
   int      r = sq / 8, c = sq % 8;

   // North-East
   for ( int r2 = r + 1, c2 = c + 1; r2 <= 7 && c2 <= 7; r2++, c2++ )
   {
      attacks |= ( 1ULL << ( r2 * 8 + c2 ) );
      if ( block & ( 1ULL << ( r2 * 8 + c2 ) ) )
         break;
   }

   // North-West
   for ( int r2 = r + 1, c2 = c - 1; r2 <= 7 && c2 >= 0; r2++, c2-- )
   {
      attacks |= ( 1ULL << ( r2 * 8 + c2 ) );
      if ( block & ( 1ULL << ( r2 * 8 + c2 ) ) )
         break;
   }

   // South-East
   for ( int r2 = r - 1, c2 = c + 1; r2 >= 0 && c2 <= 7; r2--, c2++ )
   {
      attacks |= ( 1ULL << ( r2 * 8 + c2 ) );
      if ( block & ( 1ULL << ( r2 * 8 + c2 ) ) )
         break;
   }

   // South-West
   for ( int r2 = r - 1, c2 = c - 1; r2 >= 0 && c2 >= 0; r2--, c2-- )
   {
      attacks |= ( 1ULL << ( r2 * 8 + c2 ) );
      if ( block & ( 1ULL << ( r2 * 8 + c2 ) ) )
         break;
   }

   return attacks;
}

// Main Init function - populates PEXT attack tables
void InitPEXT()
{
   for ( int sq = 0; sq < 64; sq++ )
   {
      // Calculate masks
      RookMasks[ sq ]   = CalcRookMask( sq );
      BishopMasks[ sq ] = CalcBishopMask( sq );

      // Populate rook attack table
      BitBoard rookMask   = RookMasks[ sq ];
      int      numBits    = (int)Count( rookMask );
      int      numIndices = 1 << numBits;

      for ( int i = 0; i < numIndices; i++ )
      {
         // _pdep_u64 maps the index bits back to the mask bits to form occupancy
         BitBoard occupancy         = _pdep_u64( i, rookMask );
         int      index             = (int)_pext_u64( occupancy, rookMask );
         RookAttacks[ sq ][ index ] = RookAttacksSlow( sq, occupancy );
      }

      // Populate bishop attack table
      BitBoard bishopMask = BishopMasks[ sq ];
      numBits             = (int)Count( bishopMask );
      numIndices          = 1 << numBits;

      for ( int i = 0; i < numIndices; i++ )
      {
         BitBoard occupancy           = _pdep_u64( i, bishopMask );
         int      index               = (int)_pext_u64( occupancy, bishopMask );
         BishopAttacks[ sq ][ index ] = BishopAttacksSlow( sq, occupancy );
      }
   }
}
