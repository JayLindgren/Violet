// Copyright 2006 by Jay Lindgren. All Rights Reserved.
//
// This file contains all definitions except for those associated iwth the evaluation.
// Please see Evaluaction.cpp for those values.
//
//
// Some Board notes.
// Cols ---->
// R
// o a1 a2 a3 a4 a5 a6 a7 a8
// w b1 b2 b3 b4 b5 b6 b7 b8
// s c1 c2 c3 c4 c5 c6 c7 c8
// | d1 d2 d3 d4 d5 d6 d7 d8
// | e1 e2 e3 e4 e5 e6 e7 e87
// V f1 f2 f3 f4 f5 f6 f7 f8
//   g1 g2 g3 g4 g5 g6 g7 g8
//   h1 h2 3h h4 h5 h6 h7 h8
//
// North is increasing cols
// South is decreasing cols
// East is increasing rows
// West is decreasing rows

///////////////////////////////
// Game control definitions
///////////////////////////////

//# include <vld.h> 

//# define NDEBUG 
#ifndef NDEBUG
# define NDEBUG 
#endif
# include <assert.h>

// Turn on deep mode
# define dDeepMode    1
# define dUseHash     1
# define dUseNull     1
# define dUseFutility 0
# define dAspiration  1

// Define the version
# define dsVersion "3.0"

// Define the interface
# define dConsole 1
# define dUCI     2
# define dXboard  3

// Define computer numbers
# define dComputerWhite 1
# define dComputerBlack 0

// Define some game control parameters.
# define dGame          1  // Plays a man vs. machine game
# define dSetUp         2  // Reads an FEN file
# define dComputerGame  3  // Has Violet play herself
# define dBookProbe     4  // User pushes game with help of the book.
# define dTestSuite     5  // Runs through EPD files like Win at chess
# define dTestEval      6  // Simple check EVAL() for a position.
# define dTestDraw      7  // A simple test to check the draw code.
# define dCheckBook     8  // Runs a shallow search on ever positoin in the book.
# define dTestHash      9  // Runs a user defined test of the hash.
# define dUserSetUp    10  // Allows the user to push positions forward.
# define dTestNull     11  // Test a null move.
# define dOpeningBook  12  // Create an opening book from a collection of games.
# define dTune         13  // Used for tuning parameters

// How frequently to check the time.
# define dNodeCheck 100000000

///////////////////////////////////
// Chess basics
///////////////////////////////////

// Some real basics
# define dYes 1
# define dNo 0

//  The idea of this calculation is to precalculate all of the possible moves for every piece
// on every square.  Thus, saving later calculations of the same.
//

# define dA1  0
# define dB1  1
# define dC1  2
# define dD1  3
# define dE1  4
# define dF1  5
# define dG1  6
# define dH1  7
# define dA2  8
# define dB2  9
# define dC2 10
# define dD2 11
# define dE2 12
# define dF2 13
# define dG2 14
# define dH2 15
# define dA3 16
# define dB3 17
# define dC3 18
# define dD3 19
# define dE3 20
# define dF3 21
# define dG3 22
# define dH3 23
# define dA4 24
# define dB4 25
# define dC4 26
# define dD4 27
# define dE4 28
# define dF4 29
# define dG4 30
# define dH4 31
# define dA5 32
# define dB5 33
# define dC5 34
# define dD5 35
# define dE5 36
# define dF5 37
# define dG5 38
# define dH5 39
# define dA6 40
# define dB6 41
# define dC6 42
# define dD6 43
# define dE6 44
# define dF6 45
# define dG6 46
# define dH6 47
# define dA7 48
# define dB7 49
# define dC7 50
# define dD7 51
# define dE7 52
# define dF7 53
# define dG7 54
# define dH7 55
# define dA8 56
# define dB8 57
# define dC8 58
# define dD8 59
# define dE8 60
# define dF8 61
# define dG8 62
# define dH8 63

// Define the pieces.
// The code assumes that the black pieces have a higher number than the white pieces.
// Further it assumes that the white king is the highest numbered white piece and that
// the black pawn is the lowest numbered black piece.
# define dEmpty         0
# define dWhitePawn     1
# define dWhiteRook     2
# define dWhiteKnight   3
# define dWhiteBishop   4
# define dWhiteQueen    5
# define dWhiteKing     6
# define dBlackPawn     7
# define dBlackRook     8
# define dBlackKnight   9
# define dBlackBishop  10
# define dBlackQueen   11
# define dBlackKing    12

// Define the rows.
# define dA 1
# define dB 2
# define dC 3
# define dD 4
# define dE 5
# define dF 6
# define dG 7
# define dH 8

// Define the directions.
# define dNorth     1
# define dNorthEast 2
# define dEast      3
# define dSouthEast 4
# define dSouth     5 
# define dSouthWest 6
# define dWest      7
# define dNorthWest 8

// Define the colors ;
# define dWhite 1
# define dBlack 0

//  The various type are defined here.
//

// Here are some useful board descriptions.
//
// Some Board notes.
// Cols ---->
// R
// o a1 a2 a3 a4 a5 a6 a7 a8
// w b1 b2 b3 b4 b5 b6 b7 b8
// s c1 c2 c3 c4 c5 c6 c7 c8
// | d1 d2 d3 d4 d5 d6 d7 d8
// | e1 e2 e3 e4 e5 e6 e7 e8
// V f1 f2 f3 f4 f5 f6 f7 f8
//   g1 g2 g3 g4 g5 g6 g7 g8
//   h1 h2 3h h4 h5 h6 h7 h8
//
// North is increasing cols
// South is decreasing cols
// East is increasing rows
// West is decreasing rows
//
// Board[ 64 ] = { a1, b1, c1, d1, e1, f1, g1, h1,
//                 a2, b2, c2, d2, e2, f2, g2, h2
//						 a3, b3, c3, d3, e3, f3, g3, h3
//						 a4, b4, c4, d4, e4, f4, g4, h4
//						 a5, b5, c5, d5, e5, f5, g5, h5
//						 a6, b6, c6, d6, e6, f6, g6, h6
//						 a7, b7, c7, d7, e7, f7, g7, h7
//						 a8, b8, c8, d8, e8, f8, g8, h8 );
//

//////////////////////////////////////////////////////////////////////////////////////
// Define some useful substitutions to make the programming more intuitive.
//////////////////////////////////////////////////////////////////////////////////////


// Define the squares.
//
// The numbering scheme is. square = rowIndex + ( colIndex ) * 8
//

// Define the rows.
# define dA 1
# define dB 2
# define dC 3
# define dD 4
# define dE 5
# define dF 6
# define dG 7
# define dH 8

// Define the directions.
# define dNorth     1
# define dNorthEast 2
# define dEast      3
# define dSouthEast 4
# define dSouth     5 
# define dSouthWest 6
# define dWest      7
# define dNorthWest 8

// Define the colors ;
# define dWhite 1
# define dBlack 0
// Define the row and column opperators.
# define dRow( iSquare ) ( iSquare & 7 )
# define dCol( iSquare )	( iSquare >> 3 )

// Misc parameters.
# define dNumberOfMoves       200
# define dNumberOfMoveHistory 300
# define dNumberOfPlys         60 
# define dNumberOfSquares      64

// Define the move types for numerical reference.
# define dRegular                 0
# define dCapture                 1
# define dEnPassant               4
# define dTwoSquare               8
# define dOneSquare              16 
# define dPromote                32
# define dCastle                 64
# define dWhiteKingSideCastle   128
# define dWhiteQueenSideCastle  256
# define dBlackKingSideCastle   512
# define dBlackQueenSideCastle 1024
# define dCaptureAndPromote    2048

////////////////////////////////////////
// Hash parameters
////////////////////////////////////////

// For the hash table.
// Must be a power of x^2 - 1
// This will use up to x number of 1's to create the full entry.
// Memory used by violet is:
// 2 * 2^x * 64 / 1000000 for the value in megs.
# define dNumberOfBitsInHash        20
# define dNumberOfBitsInOpeningBook 20 // Changing this requres redoing the entire opening book.
//# define dHashTableSize   ( dNumberOfBitsInHash^2 - 1 ) // 2^20 - 1  This should be controlled by sGeneralMove->bbKey1 check bbKey2.
//# define dOpeningBookSize ( dNumberOfBitsInHash^2 - 1 ) // 2^20 - 1
//# define dHashTableSize 1048575 // 2^20 - 1  
//# define dOpeningBookSize 1048575 // 2^20 - 1

# define dNewKeys 0
# define dFile    1
# define dCode    2


//////////////////////////////////////
// Define the bit board
//////////////////////////////////////
// Type definitions.
typedef unsigned long long int BitBoard;

/////////////////////////////////////////////////
// Move scoring
/////////////////////////////////////////////////

// Defind the number of scores
# define dNumberHeuristicMoveScores 19
# define dNumberSpecificMoveScores  27
# define dMaxMoveScore              1000
# define dNumberOfSimulations       1000


///* Orignal Scores
// Define the move scores.
# define dsRegular       10
# define dsPawnTwo       4 // 20 226,823, 5 187,069, 4 187,232
# define dsPiece         30
# define dsQueen          5
# define dsBishop        50
# define dsRook          60
# define dsKnight        70
# define dsKing          80  
# define dsCaptureDown   125 // 125 253,890
# define dsCaptureSide  110 // 110, 251,519, 200 251,888, 10 251,427
# define dsCaptureUp    120 
# define dsCheck        200
# define dsPromotion    300
# define dsCastle       99 // 400 253864, 1600 257155, 299 252921, 199 253024, 99 251,519
# define dsKillerMove  1501 // 1501 253864
# define dsHH          1500
# define dsKingCapture 5000
# define dsBestMove    6000
# define dsPVMove      1000 // 7000 251,519, 1000 226,823

// Redefine the captures.
# define dsPawnTakesQueen    190
# define dsPawnTakesRook     180
# define dsPawnTakesBishop   170
# define dsPawnTakesKnight   160
# define dsPawnTakesPawn     111
# define dsKnightTakesPawn   100
# define dsKnightTakesKnight 110
# define dsKnightTakesBishop 111
# define dsKnightTakesRook   170
# define dsKnightTakesQueen  179
# define dsBishopTakesPawn   100
# define dsBishopTakesKnight 110
# define dsBishopTakesBishop 111
# define dsBishopTakesRook   170
# define dsBishopTakesQueen  178
# define dsRookTakesPawn     100
# define dsRookTakesKnight   100
# define dsRookTakesBishop   100
# define dsRookTakesRook     110
# define dsRookTakesQueen    177
# define dsQueenTakesPawn    100
# define dsQueenTakesKnight  100
# define dsQueenTakesBishop  100
# define dsQueenTakesRook    100
# define dsQueenTakesQueen   110
//*/


////////////////////////////////////
// Define some scoring parameters
////////////////////////////////////

// Define the initial alpha and beta.
#define dAlpha    -99999
#define dBeta      99999
#define dMate     -99999

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Opening book parameters
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Define some opening book parameters
# define dWhiteWin 0
# define dBlackWin 1
# define dDraw     2
# define dUnknown  3

# define dNumberOfBitsPerScore 21

// Set the percentage of wins a position has to have before moving out of book.
# define dWinCutOff 30

// Define the number of times a position must be visited before a cut off is taken.
# define dBookCutOff       100 // Used for trimming the book but not for move selection
# define dPopularityCutOff 0.9 // used for going after only the most popular moves.
# define dMoveCutOff       100 // Used for move selection
//# define dWinCutOff        60
# define dNotALoss         0.6

// For the verification of the book.  If the position scores less then this, it will be removed from
// the book
# define dOpeningBookScoreCutOff 70
# define dOpeningBookVerificationSearchDepth 6

// Define the maximum number the rand() funciton generates
//# define dMaxRand 32767

////////////////////////////////////////////////////////////////////////////////////////////////////
// Tempus definitions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Define the Tempus game control variables.
# define dStop   0
# define dGo     1
# define dPonder 2

// Define the input string size for the interface.
# define dStringSize         6400

// Define some helpful times.
# define dInfiniteDepth        60
# define dOneSecond          1000
# define dBufferTime         2000
# define dTwentySeconds     20000
# define dOneMinute         60000
# define dTenMinutes       600000
# define dInfiniteTime  999999999

// Set how frequently we poll for input.
# define dPollingCount   1000
# define dMoveUpdateTime 2000 // Defines how frequently we update the move being searched to the interface.  

// Some parameters for when the null move is used.
# define dMaxPieces 31
# define dMinPieces 0
# define dMinPiecesToVerify 32

///////////////////////////////////////////////////////////////
// Search parameters
///////////////////////////////////////////////////////////////

// This sets the value below Alpha that must be found before pruning.
///*
# define dForwardPrune0 300
# define dForwardPrune1 400
# define dForwardPrune2 450
# define dForwardPrune3 500
# define dForwardPrune4 550
# define dForwardPrune5 600
# define dForwardPrune6 700
# define dForwardPrune7 800
# define dForwardPrune8 900
# define dForwardPrune9 1000
//*/

// Define the value of the remaining plys to the Q search with no forward prunning
# define dPruneCutOff 6

// Define the window for the aspirational search window.
# define dAspirationalWindowWidth 30


///////////////////////////////////////////////////////////////////////////////////////////////////////
// Define some evaluation parameters
///////////////////////////////////////////////////////////////////////////////////////////////////////

// Define the doubled pawn score. - Note, these score get counted twice (one for each pawn).
# define dDoubleCol1 10
# define dDoubleCol2 10
# define dDoubleCol3 10
# define dDoubleCol4 10
# define dDoubleCol5 10
# define dDoubleCol6 10
# define dDoubleCol7 10
# define dDoubleCol8 10

// Define the piece values.
# define dEmpty                  0
# define dValuePawnOpening     100
# define dValueRookOpening     500
# define dValueKnightOpening   325
# define dValueBishopOpening   325
# define dValueQueenOpening    900
# define dValueKingOpening    9999

# define dValuePawnEndGame     100
# define dValueRookEndGame     500
# define dValueKnightEndGame   325
# define dValueBishopEndGame   325
# define dValueQueenEndGame    901
# define dValueKingEndGame    9999

// Define some phase numbers.
# define dPhasePawn   1
# define dPhaseKnight 10
# define dPhaseBishop 10
# define dPhaseRook   20
# define dPhaseQueen  40

// Defind the maximum phase
// Max Phase = 16 * 1 + 4 * 20 + 4 * 10 + 4 * 10 + 2 * 40
# define dMaxPhase 256

// Define penality for not casteling :
# define dNoCastlePenality 20

// Define the score for one square of mobility.
# define dMobilityScore 4

// Mobility in the opening.
# define dMobilityKnightOpening 4
# define dMobilityBishopOpening 5
# define dMobiltiyRookOpening   2
# define dMobilityQueenOpening  1
# define dMobilityKingOpening   0

// Mobility in the end game.
# define dMobilityKnightEndGame 4
# define dMobilityBishopEndGame 5
# define dMobiltiyRookEndGame   4
# define dMobilityQueenEndGame  1
# define dMobilityKingEndGame   0


// Define the bonus for having a double bishop
# define dBishopPair 25 

// King safety, king potential attack.
// This is be counted if a piece can potentially attack the king from
// from their current square.
//# define dKingOpaqueAttack 2
# define dKingOpaqueAttack 1

// The value of having a king fence.
# define dKingFence 15

// Define the value of a draw, a contempt for draw maybe placed here.
# define dDrawValue 0

// Define the value of a passed pawn
// And the value of a passed pawn but blocked.
# define dPassedPawnIntercept 10
# define dPassedPawnSlope 10
# define dPassedPawnBlocked
# define dIsolatedPawn 10
# define dDoubledPawn

// Add in for rooks on open and semi open files.
# define dRookOnSemiOpenFileOpening     10
# define dRookOnOpenFileOpening         20
# define dRookOnSemiOpenKingFileOpening 10
# define dRookOnOpenKingFileOpening     20
# define dRookOnSemiOpenFileEndGame     10
# define dRookOnOpenFileEndGame         20
# define dRookOnSemiOpenKingFileEndGame 10
# define dRookOnOpenKingFileEndGame     20