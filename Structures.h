
#ifndef STRUCTURES_H
#define STRUCTURES_H

// Copyright 2006 by Jay Lindgren. All Rights Reserved.
//
#include <time.h>
#include "Definitions.h"

//////////////////////////////////////////////////////////////////////
// The major data structures.
//////////////////////////////////////////////////////////////////////

struct GeneralMove 
{

   // Some utility selectors.
   BitBoard bbColZero;
   BitBoard bbColOne;
   BitBoard bbColSix;
   BitBoard bbColSeven;
   BitBoard bbRowZero;
   BitBoard bbRowSeven;
   BitBoard bbKingSideWhite;
   BitBoard bbKingSideBlack;
   BitBoard bbQueenSideWhite;
   BitBoard bbQueenSideBlack;
   BitBoard bbKingSideWhiteCastleArea;
   BitBoard bbKingSideBlackCastleArea;
   BitBoard bbQueenSideWhiteCastleArea;
   BitBoard bbQueenSideBlackCastleArea;

   // Allocate the memory.
   // Some short hand.
   // FM  - found moves.
   // FA  - found attackes.
   // NOM - number of moves.
   // NOA - number of attacks.

   // White Pawns
   BitBoard bbWPMove[ 64 ];
   BitBoard bbWPAttack[ 64 ];
   int mWPFM[ 64 ][ 2 ];
   int mWPFA[ 64 ][ 2 ];
   int vWPNOM[ 64 ];
   int vWPNOA[ 64 ];

   // Black Pawns
   BitBoard bbBPMove[ 64 ];
   BitBoard bbBPAttack[ 64 ];
   int mBPFM[ 64 ][ 2 ];
   int mBPFA[ 64 ][ 2 ];
   int vBPNOM[ 64 ];
   int vBPNOA[ 64 ];

   // The Rooks.
   BitBoard bbRMove[ 64 ];
   BitBoard bbRMoveN[ 64 ];
   BitBoard bbRMoveW[ 64 ];
   BitBoard bbRMoveE[ 64 ];
   BitBoard bbRMoveS[ 64 ];
   int mRN[ 64 ][ 8 ];
   int mRS[ 64 ][ 8 ];
   int mRE[ 64 ][ 8 ];
   int mRW[ 64 ][ 8 ];
   int vRNOMN[ 64 ];
   int vRNOMS[ 64 ];
   int vRNOME[ 64 ];
   int vRNOMW[ 64 ];

   // The knights.
   BitBoard bbNMove[ 64 ];
   int mNMove[ 64 ][ 8 ];
   int vNNOM[ 64 ];

   // The Bishops.
   BitBoard bbBMove[ 64 ];
   BitBoard bbBMoveNE[ 64 ];
   BitBoard bbBMoveNW[ 64 ];
   BitBoard bbBMoveSE[ 64 ];
   BitBoard bbBMoveSW[ 64 ];
   int mBNE[ 64 ][ 8 ];
   int mBNW[ 64 ][ 8 ];
   int mBSE[ 64 ][ 8 ];
   int mBSW[ 64 ][ 8 ];
   int vBNOMNE[ 64 ];
   int vBNOMNW[ 64 ];
   int vBNOMSE[ 64 ];
   int vBNOMSW[ 64 ];

   // The Queen.
   BitBoard bbQMove[ 64 ];
   BitBoard bbQMoveNE[ 64 ];
   BitBoard bbQMoveNW[ 64 ];
   BitBoard bbQMoveSE[ 64 ];
   BitBoard bbQMoveSW[ 64 ];
   BitBoard bbQMoveN[ 64 ];
   BitBoard bbQMoveW[ 64 ];
   BitBoard bbQMoveS[ 64 ];
   BitBoard bbQMoveE[ 64 ];
   int mQN[ 64 ][ 8 ];
   int mQS[ 64 ][ 8 ];
   int mQE[ 64 ][ 8 ];
   int mQW[ 64 ][ 8 ];
   int mQNE[ 64 ][ 8 ];
   int mQNW[ 64 ][ 8 ];
   int mQSE[ 64 ][ 8 ];
   int mQSW[ 64 ][ 8 ];
   int vQNOMN[ 64 ];
   int vQNOMS[ 64 ];
   int vQNOME[ 64 ];
   int vQNOMW[ 64 ];
   int vQNOMNE[ 64 ];
   int vQNOMNW[ 64 ];
   int vQNOMSE[ 64 ];
   int vQNOMSW[ 64 ];

   // The King.
   BitBoard bbKMove[ 64 ];
   int mKMove[ 64 ][ 8 ];
   int vKNOM[ 64 ];
   BitBoard bbKingBubble[ 64 ]; // Used for King safety.

   // The promotion pieces.
   int vPromotionPieces[ 4 ];

   // These are used in the Find() function.   
   BitBoard bbCol1;
   BitBoard bbCol2;
   BitBoard bbCol3;
   BitBoard bbCol4;
   BitBoard bbCol5;
   BitBoard bbCol6;
   BitBoard bbCol7;
   BitBoard bbCol8;
   BitBoard bbRow1;
   BitBoard bbRow2;
   BitBoard bbRow3;
   BitBoard bbRow4;
   BitBoard bbRow5;
   BitBoard bbRow6;
   BitBoard bbRow7;
   BitBoard bbRow8;
   BitBoard bbUpperHalf;
   BitBoard bbLowerHalf;
   BitBoard bbFourthQuarter;
   BitBoard bbThirdQuarter;
   BitBoard bbSecondQuarter;
   BitBoard bbFirstQuarter;
   BitBoard bbRightHalf;
   BitBoard bbLeftHalf;
   BitBoard bbFarRight;
   BitBoard bbNearRight;
   BitBoard bbFarLeft;
   BitBoard bbNearLeft;

   // The hash key stuff.
   BitBoard bbScore;
   BitBoard bbScoreSign;
   BitBoard bbDepth;
   BitBoard bbBestMove;
   BitBoard bbDanger;
   BitBoard bbTypeOfScore;
   BitBoard bbAge;
   BitBoard bbWhiteScore;
   BitBoard bbBlackScore;
   BitBoard bbDrawScore;
   BitBoard bbVerified;

   int iScoreShift;
   int iSignShift;
   int iDepthShift;
   int iBestMoveShift;
   int iDangerShift;
   int iTypeOfScoreShift;
   int iAgeShift;
   int iWhiteScoreShift;
   int iBlackScoreShift;
   int iDrawScoreShift;
   int iOpenBookMoveShift;

   // These hold the rows and columns of a given square.
   // Used at least for passed pawns.
   int viCol[ 64 ]; // This is not calculate with a zero offset but represents the real row or col.
   int viRow[ 64 ];
   BitBoard vbbCol[ 64 ];
   BitBoard vbbRow[ 64 ];

   // For passed pawns
   // Masks of the board in from the expressed columns
   BitBoard vbbWhitePPBoardMask[ 8 ];
   BitBoard vbbBlackPPBoardMask[ 8 ];
   BitBoard vbbWhitePPVector[ 64 ]; // the space in front the pawn
   BitBoard vbbBlackPPVector[ 64 ];
   BitBoard vbbWhitePPWideVector[ 64 ]; // the space in front the pawn and the cols on either side
   BitBoard vbbBlackPPWideVector[ 64 ]; // the space in front the pawn and the cols on either side

   // Define the move score here.  This allows for them to be set dynamically and optimized.
   // Define the move scores.
   // ms notaion = move score.
   // Heuristic move scores
   int msRegular;
   int msPawnTwo;
   int msPiece;
   int msQueen;
   int msBishop;
   int msRook;
   int msKnight;
   int msKing;  
   int msCaptureDown;
   int msCaptureSide;
   int msCaptureUp;
   int msCheck;
   int msPromotion;
   int msCastle;
   int msKillerMove;
   int msHH;
   int msKingCapture;
   int msBestMove;
   int msPVMove;

   // Redefine the captures.
   // Specific move scores.
   int msPawnTakesQueen;
   int msPawnTakesRook;
   int msPawnTakesBishop;
   int msPawnTakesKnight;
   int msPawnTakesPawn;
   int msKnightTakesPawn;
   int msKnightTakesKnight;
   int msKnightTakesBishop;
   int msKnightTakesRook;
   int msKnightTakesQueen;
   int msBishopTakesPawn;
   int msBishopTakesKnight;
   int msBishopTakesBishop;
   int msBishopTakesRook;
   int msBishopTakesQueen;
   int msRookTakesPawn;
   int msRookTakesKnight;
   int msRookTakesBishop;
   int msRookTakesRook;
   int msRookTakesQueen;
   int msQueenTakesPawn;
   int msQueenTakesKnight;
   int msQueenTakesBishop;
   int msQueenTakesRook;
   int msQueenTakesQueen;

};

struct SearchParameters
{

   int miSearchReduction[ dNumberOfPlys ][ dNumberOfMoves ]; // Depth and move 

   // Defind the global variables for the heuristics table. 
   int mWhiteHistoryHeuristic[ dNumberOfSquares ][ dNumberOfSquares ];
   int mBlackHistoryHeuristic[ dNumberOfSquares ][ dNumberOfSquares ];
   int iWhiteHHCount;
   int iBlackHHCount;
   int mKillerMove[ dNumberOfMoves ][ dNumberOfSquares ][ dNumberOfSquares ];
   int vKillerMoveCount[ dNumberOfMoves ];

   int iTried;
   int iFailed;
   int iZug;

   // Here are the forward pruning parameters
   // This is the material bonus that the search must be down, below Alpha
   // before pruning.  Similar to Crafty.
   int iPruningSchedule[ 10 ];
};

// Extern declaration for the global instance of SearchParameters
// This tells other files that gsSearchParameters exists somewhere.
// =========================================================================
extern SearchParameters gsSearchParameters;

struct Move 
{

   BitBoard bbFromSquare;
   BitBoard bbToSquare;   
   BitBoard bbEPSquare;
   int      iPiece;
   int      iFromSquare;
   int      iToSquare;
   int      iPromote;
   int      iMoveType;
   BitBoard bbCastle;
   int      iCapture;
   int      iScore;
	
};

struct HistoryStack
{

   BitBoard bbFromSquare;
   BitBoard bbToSquare;   
   BitBoard bbEPSquare;
   int      iPiece;
   int      iFromSquare;
   int      iToSquare;
   int      iPromote;
   int      iMoveType;
   BitBoard bbCastle;
   BitBoard bbCheck;
   //int      iFifty;
   //int      iHash;
   int      iCapture;
   BitBoard bbHash;
	
};

struct HashTable
{
   
   // The hash table with have two large arrays that hold bit boards.  Each
   // hash entry will essentially contain two 64 bit intigers.  One full integer
   // will be dedicated to the entire 64 bit key and can then be checked for 
   // correctness of the key.  The idea being that not all 64 bits will be used
   // for indexing, the rest can be used for checking.
   //
   // For example, to store a million positions, a binary number need only be 19 bits long.
   // The key array is 64 bits longs.  This leaves an additional 64 - 19 = 45 bit for checking.
   //
   // The other integer will contain info for:
   //
   // The best definition of the data structure is found in the creator for general moves.
   //
   // Key - 64 bits, used to see if the match is close to exact.
   //
   // Score       - 17, 17 - Holds the value of the score.
   // Depth       - 15, 32 - Holds the depth the position has been searched.
   // BestMove    - 21, 53 - Holds the best move previously found.
   // Danger      -  1, 54 - One if the position is dangerous and needs to be searched deeper.
   // TypeOfScore -  2, 56 - 0 Exact, 1 Upper bound, 2 Lower bound.
   // Age         -  3, 59 - Holds the age of the search.

   // Used in calculating the new hash.
   BitBoard mbbHashKeys[ 12 ][ 64 ];
   BitBoard vbbHashKeysStates[ 6 ]; // white, black, WCKS, WCQS, BCKS, BCQS, 
   BitBoard vbbEnPassant[ 64 ];
   BitBoard vbbColorToMove[ 2 ];
   BitBoard vbbCasteling[ 4 ];
   
   // Holding the data for later use.
   BitBoard bbNumberOfHashElements;
   BitBoard * mbbHashTable; // Holds the information
   BitBoard * mbbHash;      // Holds the keys
   BitBoard bbHash;         // Holds the current hash
   BitBoard bbHashInitial;  // Holds the initial hash for reference
   BitBoard bbScore;        // Used for holding the data for passing back to the calling function
   BitBoard bbDepth;        // A way to use global variables instead of passing.
   BitBoard bbBestMove;
   BitBoard bbDanger;
   BitBoard bbTypeOfScore;  // 0 Exact, 1 Upper bound, 2 Lower bound.
   BitBoard bbAge; 
   
   // Create masks for easily extracting data from the bit fields.
   BitBoard bbMaskIndex;
   
   // Note, there are several masks for extracting data in GeneralMoves
   
   // Outputs.
   int iQueryState;
   int iScore;
   BitBoard bbKey;

   // The Hash Table.
   int iHashTableSize0;
   int iHashTableSize1;

   // Store the permanant random keys.
   BitBoard vbbRandomKeys[ 845 ];


};

struct OpeningBook
{
   
   // Nearly identiacal to the hash table and even uses the hash table's hash
   
   // Holding the data for later use.
   BitBoard bbOpeningBookSize;
   BitBoard * mbbHashTable; // Holds the information
   BitBoard * mbbHash;      // Holds the keys

   // The Hash Table.
   int iOpeningBookSize0;
   int iOpeningBookSize1;
   BitBoard bbMaskIndex;

   // Set the number of positions in the opening book
   BitBoard bbNumberOfPositionsInBook;
   BitBoard bbNumberOfPositionsVerified;

};

// Holds data for evlauting a board
struct Evaluator
{

   // Here are the evaluation structures.
   int viPlacementPawnWhite[ 64 ];
   int viPlacementPawnBlack[ 64 ];
	int viPlacementRook[ 64 ];
	int viPlacementKnight[ 64 ];
	int viPlacementBishop[ 64 ];
	int viPlacementQueen[ 64 ];
	int viPlacementKingWhite[ 64 ];
   int viPlacementKingBlack[ 64 ];
	int viReverse[ 64 ];
   BitBoard vbbKingFence[ 64 ];
   int iUseMobility;
   int iPhaseOfGame;

};

// Prototype the data structures.
struct Board 
{ 

   BitBoard bbWhitePawn;
   BitBoard bbWhiteRook;
   BitBoard bbWhiteKnight;
   BitBoard bbWhiteBishop;
   BitBoard bbWhiteQueen;
   BitBoard bbWhiteKing;

   BitBoard bbBlackPawn;
   BitBoard bbBlackRook;
   BitBoard bbBlackKnight;
   BitBoard bbBlackBishop;
   BitBoard bbBlackQueen;
   BitBoard bbBlackKing;
 
   BitBoard bbAllPieces;
   BitBoard bbWhitePieces;
   BitBoard bbBlackPieces;

   BitBoard bbWhiteAttack;
   BitBoard bbWhiteMove;
   BitBoard bbBlackAttack;
   BitBoard bbBlackMove;

   BitBoard bbWhiteKingMove;
   BitBoard bbWhiteKingAttack;
   BitBoard bbBlackKingMove;
   BitBoard bbBlackKingAttack;

   int siNumberOfMoves;

   // To Do.
   int iHasWhiteCastled;
   int iHasBlackCastled;
   BitBoard bbCastle;
   // bit 1 - white king side castle.
   // bit 2 - black king side castle.
   // bit 3 - white queen side castle.
   // bit 4 - black queen side castle.
   
   // To Do.
   BitBoard bbCheck;
   // bit 0 - white king is in check
   // bit 1 - black king is in check 
   // bit 2 - white king is in check mate 
   // bit 3 - black king is in check mate
   // bit 4 - white is in stalemate
   // bit 5 - black is in stalemet
   
   // Game controls.
   int siColorToMove;
   BitBoard bbEP;
   int mBoard[ dNumberOfSquares ];
   int iMoveHistory;
   struct HistoryStack sHistoryStack[ dNumberOfMoveHistory ];
   int siLegalMove;
   int iNumberOfPlys;
   int iMaxPlys;
   int iMaxPlysReached;

   // The principal variation data sturctures, see David Levy, "How Computers Play Chess"
   // rows, moved at ply of cols
   Move vmPrincipalVariation[       dNumberOfPlys ][ dNumberOfPlys ];
   int  viPrincipalVariationLength[ dNumberOfPlys ];

   // Some time keeping variables.
   clock_t cStart; // can probably move to tempus.
   clock_t cFinish;

   // Game control.
   int iPlayGame;
   int siComputerColor;

   // Store the best move for storage in the hash table
   // this variable is the best in the standard matrix and defines which move is
   // searched first.fs
   int iBestMove;

   // Set the clock since the last pawn capture
   int iHalfMoveClock;

   // Used in the function DoNullSearch to ensure at least one move is NOT Null Searched.
   int iMoveOrder;

   // For null moves - if the last move was a null, don't perform another null move.
   int iLastMoveNull;

   // This variable is used to determine if verificaton of the null move is used in a sub tree.
   int iUseNullVerification;

};


// Prototype the time control and move control structure.
// This will be a global structure in the search file.
struct Tempus 
{ 

   // This variable will control whather to search or stop
   int giStopGo;
   int giTimeStart;
   int giTimeLeft;
   int giSearchDepth;
   int giSearchTime;
   int giSearchStartTime;
   int giSearchStopTime;

   // Various game control parameters
   int giSearchMonitorTogel;
   int giPersistantKeys;
   int giModeOfPlay;
   int giDebug;
   int iInterfaceBookDebug;
   int iInterfaceMoveDebug;
   int giUseHashTable;
   int giOpeningBookDebug;
   int giUseOpeningBook;
   int giComputerColor;
   int giInterfaceMode;
   int giOpeningBookAnalysis;
   int giLimitELO;
   int giMovesToGo;
   int giNodes;
   int giMate;
   int giInterfaceDebug;
   int giConsoleOutput;
   int giResetHash;

   // The Null-Move heurisitc.
   int iUseNullMove;
   int iUseNullEval;
   int iNullReduction;

   // The Late Move Reduction parameters.
   int iUseLMR;
   int iMinimumMovesSearched;
   int iMinimumScoreSearched;
   int iReducedSearchDepth;

   // Control the aspriation search
   int iAspirationSearch;

   // Here are the move time elements (in Milli seconds).
   int giBlackIncrementalTime;
   int giBlackTime;
   int giMoveTime;
   int giWhiteIncrementalTime;
   int giWhiteTime;

   // Some Nodal counts
   BitBoard gbbNumberOfNodes;
   BitBoard gbbNumberOfIncrementalNodes;
   BitBoard gbbiNodeCheck;
   BitBoard gnniNumberOfTranspositions;
   clock_t  gcStart;
   clock_t  gcFinish;
   BitBoard gbbPollingCount;

   // Store some score info here.
   int iScore;

   // This handles the multiple PVs
   Move vmPrincipalVariation[    dNumberOfPlys ][ dNumberOfPlys ];
   Move vmPrincipalVariationOld[ dNumberOfPlys ][ dNumberOfPlys ];
   int viScore[     dNumberOfMoves ];
   int viScoreOld[  dNumberOfMoves ];
   int viHaveScore[ dNumberOfMoves ];
   int viDepth[     dNumberOfMoves ];
   int iNumberOfPVs;

   // A bolean for pondering or not.
   int iPonder;
   
   // This timer is used to control sending updates on what move is being searched.
   int iTimeOfLastMoveUpdate;

    // A holder for if Violet is still in book.
   int iInBook; // 1 if in book and 0 if searching.

   // Store how many nodes were searched.
   int iNumberOfNodesSearched;

};

#endif // STRUCTURES_H