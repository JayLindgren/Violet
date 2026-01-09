#ifndef MOVEGEN_H
#define MOVEGEN_H

// PEXT-based Move Generation (Haswell+ x86 CPUs)
// Copyright 2025 - Violet Chess Engine
//
// This header provides PEXT (Parallel Bit Extract) based sliding piece attack
// generation. PEXT replaces massive pre-calculated array lookups with a single
// CPU instruction, improving cache locality and performance.

#include "Definitions.h"
#include <immintrin.h> // Required for _pext_u64

// Pre-calculated masks for PEXT
// These masks isolate the "relevant occupancy" bits for a given square
// (excludes edge squares as they don't affect attack generation)
extern BitBoard RookMasks[ 64 ];
extern BitBoard BishopMasks[ 64 ];

// The PEXT attack tables
// These tables store the actual attack bitboards.
// Size: The index is the PEXT result of the occupancy.
// Rook max bits = 12 (4096 entries per square), Bishop max bits = 9 (512 entries).
// We flatten this into a single array for cache efficiency.
extern BitBoard RookAttacks[ 64 ][ 4096 ];
extern BitBoard BishopAttacks[ 64 ][ 512 ];

// Initialize the PEXT tables (Call this once at startup)
void InitPEXT();

// The core PEXT Attack Getters (Inline for speed)
__forceinline BitBoard GetRookAttacksPEXT( int square, BitBoard occupancy )
{
   return RookAttacks[ square ][ _pext_u64( occupancy, RookMasks[ square ] ) ];
}

__forceinline BitBoard GetBishopAttacksPEXT( int square, BitBoard occupancy )
{
   return BishopAttacks[ square ][ _pext_u64( occupancy, BishopMasks[ square ] ) ];
}

__forceinline BitBoard GetQueenAttacksPEXT( int square, BitBoard occupancy )
{
   return GetRookAttacksPEXT( square, occupancy ) | GetBishopAttacksPEXT( square, occupancy );
}

#endif // MOVEGEN_H
