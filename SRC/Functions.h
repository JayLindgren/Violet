#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// Copyright 2006 by Jay Lindgren. All Rights Reserved.
//
//#include <fstream>
#include "Definitions.h"
#include "Structures.h"
#include "time.h"
#include "windows.h"
#include <string>

// Global array for pre-calculated placement scores
extern int g_PlacementScore[13][64][64];

///////////////////////////////////////////////////////////////////////////
// BitBoard.cpp functions.
///////////////////////////////////////////////////////////////////////////

// Prototype the board functions.
void CreateBoard( struct Board * argsBoard,
                  struct GeneralMove * argsGeneralMoves );

void ClearPV( struct Board * argsBoard );

// Board Functions
BitBoard Count( BitBoard b );

int Find(  BitBoard b, 
           int * vPosition,
           struct GeneralMove * argsGeneralMoves );
                
int FindRow( BitBoard b, 
             int * vPosition,
             struct GeneralMove * argsGeneralMoves,
             int siCount,
             int siCol );
                  
void CalculateSquares( int * vRow, int * vCol, int * vSquare, int * length );

// Bit Functions
BitBoard SetBitToOne(  BitBoard b, int v );

BitBoard SetBitToZero( BitBoard b, int v );

BitBoard FlipBit(      BitBoard b, int v );

int CheckBoard( struct Board * argsBoard );

// Help with the PV search.
void CreatePV( struct Board * argsBoard );

// Debug help.
void PrintBitBoard( BitBoard b );

void PrintBoard( int * b );

void PrintFullBoard( struct Board * argsBoard );

void PrintFEN( struct Board * argsBoard,
               struct GeneralMove * argsGeneralMoves );

// FEN and board Set Up.
void ReadFEN( const char strFileName[ 64 ],
              struct Board * argsBoard,
              struct GeneralMove * argsGeneralMoves,
              int iInitialBoardFlag ); // 0 if using the initial position.

// Random generator for a bit board.
BitBoard RandomBB();

// Set the starting position.
void SetUpStartingPosition( struct Board * argsBoard,
                            struct Move * argsMoves,
                            struct GeneralMove * argsGeneralMoves );

/////////////////////////////////////////////////////////////////////////////////
// Moves.cpp functions.
////////////////////////////////////////////////////////////////////////////////

// This file is used for calculating all of the potential moves for a board.
// 

void InitializeMoveDebug();
void CloseMoveDebug();
void InitializeBookDebug();
void CloseBookDebug();

void InitializeMoves( struct Move * argsMoveList,
                      int iMoveIndex );

void MakeMove( struct Move  * sMoves, 
               struct Board * sBoard,
               struct GeneralMove * argsGeneralMoves,
               int       moveNumber );
               
void UndoMove( struct Board * argsBoard, 
               struct GeneralMove * argsGeneralMoves );

// Here are the function prototypes.
void CalculateMoves(         struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateMovesForCheck( struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhitePawns(    struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhiteRooks(    struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhiteKnights(  struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhiteBishops(  struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhiteQueens(   struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhiteKing(     struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackPawns(    struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackRooks(    struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackKnights(  struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackBishops(  struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackQueens(   struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackKing(     struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CastleWhite(            struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CastleBlack(            struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateCheck(         struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );

// Calculate the attacks.
void CalculateAttacks(              struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhitePawnsAttacks(    struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhiteRooksAttacks(    struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhiteKnightsAttacks(  struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhiteBishopsAttacks(  struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhiteQueensAttacks(   struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhiteKingAttacks(     struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackPawnsAttacks(    struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackRooksAttacks(    struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackKnightsAttacks(  struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackBishopsAttacks(  struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackQueensAttacks(   struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackKingAttacks(     struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );

// Calculate the non-attacks (quiet).
void CalculateQuiets(              struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhitePawnsQuiets(    struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhiteRooksQuiets(    struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhiteKnightsQuiets(  struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhiteBishopsQuiets(  struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhiteQueensQuiets(   struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateWhiteKingQuiets(     struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackPawnsQuiets(    struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackRooksQuiets(    struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackKnightsQuiets(  struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackBishopsQuiets(  struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackQueensQuiets(   struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );
void CalculateBlackKingQuiets(     struct Move * sMoves, struct Board * sBoard, struct GeneralMove * sGeneralMoves );


// Utility Functions.
void CheckMoves( struct Move * argsMoves );

void PrintMoves( struct Move * sMoves,
                 int iNumberOfMoves,
					  struct Board * sBoard );
					  
int PrintMove( struct Board * argsBoard,
               struct GeneralMove * argsGeneralMoves,
               Move * vsMoveList,                     // This is only one move, not an array
               char * argstrMove );
					  
// Same funciton as above only doesn't detect things such as checkmate or check which require expenseive searches.
// The moves are not strongly verified.
int PrintMoveFast( struct Board * argsBoard,
                   struct GeneralMove * argsGeneralMoves,
                   Move * vsMoveList,                     // This is only one move, not an array
                   char * argstrMove );
               
void ConvertIntToCol( int iCol,
                      char * strCol );
void ConvertIntToRow( int iRow,
                      char * strRow );
void ConvertColToInt( char * strCol,
                      int * iCol );
void ConvertRowToInt( char * strRow,
                      int * iRow );


void Slider( struct Move * argsMoves, 
             struct Board * argsBoard,
             struct GeneralMove * argsGeneralMoves,
             int * pMoves,
             int numberOfGeneralMoves,
             int startSquare,
             int pieceType );
             
void PrintPrincipalVariation( struct Board * argsBoard,
                              struct GeneralMove * argsGeneralMoves );

void InputUserMove( struct Board * argsBoard,
                    struct GeneralMove * argsGeneralMoves );
                    
void ComputerMove( struct Board * argsBoard,
                   struct GeneralMove * argsGeneralMoves );

void Ponder( struct Board * argsBoard,
             struct GeneralMove * argsGeneralMoves );

// Point an incoming algebraic move to a move in an array
void FindAlgebraicMove( struct Board * argsBoard,
                        struct GeneralMove * argsGeneralMoves,
                        char * argstrMove,
                        struct Move * argsMoveList,
                        int * argiMoveNumber );

// Take move and translate it into an algebraic interface move.
void CreateAlgebraicMove( //struct Board * argsBoard,
                          //struct GeneralMove * argsGeneralMoves,
                          char * argstrMove,
                          struct Move * argsMoveList,
                          int argiMoveNumber );

// Check to see if the board is in a legal state.
void LegalState( struct Board * argsBoard,
                 struct GeneralMove * argsGeneralMoves );

int GetMoveNumber( struct Board * argsBoard,
                   struct GeneralMove * argsGeneralMoves,
                   struct Move * argsMoveList,
                   char * strMove );

// Same as the above only no check or checkmate is calculated.
// The moves are not validated.
int GetMoveNumberFast( struct Board * argsBoard,
                       struct GeneralMove * argsGeneralMoves,
                       struct Move * argsMoveList,
                       char * strMove );

//////////////////////////////////////////////////////////////////////
// Function in Search.cpp
/////////////////////////////////////////////////////////////////////

int SearchMove(Board* argsBoard,
   GeneralMove* argsGeneralMoves,
   Move* vsMoveList,
   int iMoveIndex,
   int& argiAlpha, // Pass alpha by reference to update it
   int argiBeta,
   int iMoveCount, // The index in the sorted move list (0 for best move)
   int* viMoveScores,
   int& siHaveMove);

void InitializeSearch();

int GetReductionElement( int argiDepth,
                         int argiMove );

// Conduct the search.
int Search( struct Board * sBoard, 
            struct GeneralMove * sGeneralMoves,
            int argiAlpha,
            int argiBeta);

int StartSearch( struct Board * sBoard, 
                 struct GeneralMove * sGeneralMoves,
                 int argiAlpha,
                 int argiBeta);

int FirstSearch( struct Board * sBoard, 
                 struct GeneralMove * sGeneralMoves,
                 int argiAlpha,
                 int argiBeta,
                 int * iBestMove,
                 int * iBestMoveSearched );

int QuiesenceSearch( struct Board * sBoard, 
                     struct GeneralMove * sGeneralMoves,
                     int argiAlpha,
                     int argiBeta);

void SearchMonitor( struct Board * argsBoard,
                    struct Move * vsMoves,
                    struct GeneralMove * sGeneralMoves );

// Perform an update.
void Update( struct Board * argsBoard,
             struct GeneralMove * argsGeneralMove );

// Let the interface take control.
void InterfaceControl( struct Board * argsBoard );

// Look for checkmate.
void LookForCheckmate( struct Board * argsBoard,
                       struct GeneralMove * argsGeneralMoves,
                       int * argsiHaveMove,
                       int * argiAlpha,
                       int * argiBeta );

// Look for Check
int LookForCheck( struct Board * argsBoard );
int LookForSearchExtensions( struct Board * argsBoard,
                             struct GeneralMove * argsGeneralMoves,
                             struct Move * argvsMoveList,
                             int iMoveNumber,
                             int iNewCheck );

// See if we should do the null search.
int DoNullSearch( struct Board * argsBoard,
                  struct GeneralMove * argsGeneralMoves,
                  int argiAlpha,
                  int argiBeta,
                  int iScore,
                  struct Move * argsMove ); 

// See if we should to a LMR (Late Move Reduction)
int DoLMRSearch( int * viScore,
                 int iMoveCount,
                 struct Board * argsBoard,
                 int argiAlpha );

// Look for check and stale mates
int LookForCheckAndStale( struct Board * argsBoard,
                          struct GeneralMove * argsGeneralMoves );

//================================================================================
// Function Declarations for Move Ordering
//================================================================================
#ifndef MOVEORDER_H
#define MOVEORDER_H

//================================================================================
// Function Declarations for Move Ordering
//================================================================================

// Main sorting function using heuristics
void SortMovesHash(int* argvsiMoveOrder,
   Move* argvsMoveList,
   int argiNumberOfMoves,
   Board* argsBoard,
   GeneralMove* argsGeneralMoves,
   int* viScore);

// Simple move sorting (used by Quiescence Search)
void SortMoves( int *, Move *, int );


// Q-search sorting that only keeps captures
int QSortMoves(int* argvsiMoveOrder,
   Move* argvsMoveList,
   int argNumberOfMoves,
   Board * );

// --- Heuristic Management ---

// History Heuristic
void ResetHistoryHeuristic();
void UpdateHH(Board* argsBoard, Move* argsMove);
int UpdateScoreHH(Move* argsMove, GeneralMove* argsGeneralMoves);

// Killer Moves
void ResetKillerMoves();
void UpdateKillerMoves(Board* argsBoard, Move* argsMove);
int UpdateScoreKillerMoves(Board* argsBoard, Move* argsMove, GeneralMove* argsGeneralMoves);

void InitializePlacementScores();

#endif // MOVEORDER_H

///////////////////////////////////////////////////////////////////////
// Hash functions
///////////////////////////////////////////////////////////////////////

void UpdateHash( struct Move * argsMove,
                 struct Board * argsBoard,
                 int iMakeUnmake,
                 struct GeneralMove * argsGeneralMoves );
                 
void InitializeHashTable();

void DestroyHashTable();

void ExtractFromHashTable( struct Board * argsBoard,
                           struct GeneralMove * argsGeneralMoves );
                         
void InputToHashTable( struct Board * argsBoard,
                       struct GeneralMove * argsGeneralMoves,
                       int argiAlpha,
                       int argiBeta,
                       int argiScore,
                       struct Move * argsMove );

void TestHashUpdate();

void TestHashUpdateing();

void TestHashScores( struct Board * argsBoard,
                     struct GeneralMove * argsGeneralMoves );

void TestNullMove( Board * argsBoard,
                   GeneralMove * argsGeneralMoves );

void ReadRandomKeyFile();

void SetHash( BitBoard bbHash );

void SetHashElement( BitBoard bbKey,
                     BitBoard bbElement );

BitBoard GetHash();
BitBoard GetHashInitial();
BitBoard GetHashElement( BitBoard bbKey );
int GetQueryState();
BitBoard GetDepth();
int GetScoreHash();
BitBoard GetBestMove();

void ConvertRandomKeyFileToCode();

void GetAllKeys();
void AssignRandomKeys();

void ClearHashTable();

// New: Allow resizing TT at runtime
void SetHashTableSizeMB(int megabytes);
void SetHashTableSizeBits(int bits);

BitBoard Power( int iBase, 
                int iExponent );

int GetHashScore();
BitBoard GetMaskIndex();
BitBoard GetKey();
BitBoard GetKeyTest();

///////////////////////////////////////////////////////////////////////////
// Opening book functions
////////////////////////////////////////////////////////////////////////////

void OpeningBookAnalysis( struct Board * argsBoard,
                          struct GeneralMove * argsGeneralMoves );

void InitializeOpeningBook();

void DestroyOpeningBook();

BitBoard GetBookElement(     BitBoard bbKey );
BitBoard GetBookElementHash( BitBoard bbKey );

void SetBookElement( BitBoard bbKey,
                     BitBoard bbElement );

void SetBookElementHash( BitBoard bbKey,
                         BitBoard bbElement );

int LookForGameResult( char * strMove, 
                       int * iGameResult ); 

void UpdateOpeningBook( struct Board * argsBoard,
                        struct GeneralMove * argsGeneralMoves,
                        int iGameResult );

void WriteOutOpeningBook( const char * strFileName,
                          int iPlyIndex );

void ReadAPGNMove( int * iGameResult,
                   int * iFlagLine,
                   int * iFlagBook,
                   int * iGoodGame,
                   int * iGoodWhiteELO,
                   int * iGoodBlackELO,
                   char * strMove,
                   struct Board * argsBoard,
                   struct GeneralMove * argsGeneralMoves,
                   int iPlyCount,
                   int iPlyIndex );

void PrintOpeningBookMoveStatistics( struct Board * argsBoard,
                                     struct GeneralMove * argsGeneralMoves );

void ExtractOpeningBookStats( BitBoard & argiWhiteWins,
                              BitBoard & argiBlackWins,
                              BitBoard & argiDraws,
                              struct GeneralMove * argsGeneralMoves );

int FindMaxScore ( double * vdWins,
                   int iNumberOfMoves );

void FindBookMove( struct Board * argsBoard,
                   struct GeneralMove * argsGeneralMoves,
                   struct Move * argsvMoveList,
                   struct Move * argsBestMove,
                   int * iBookMove );

void ReadOpeningBook( const char * strBookName,
                      struct GeneralMove * argsGeneralMoves );

void CombineOpeningBooks( struct Board * argsBoard,
                          struct GeneralMove * argsGeneralMoves );

void AppendBook( struct GeneralMove * argsGeneralMoves,
                 const char * strBookName );

void TrimOpeningBook( struct GeneralMove * argsGeneralMoves );

void CheckBook( struct Board * argsBoard,
                struct GeneralMove * argsGeneralMoves );

void StartCheckBook( struct Board * argsBoard,
                     struct GeneralMove * argsGeneralMoves );

void GetPopularMoves( struct Board * argsBoard,
                      struct GeneralMove * argsGeneralMoves,
                      int * viPopularMoves,
                      int * iNumberOfPopularMoves,
                      struct Move * sMoves,
                      int iNumberOfMoves );

int GetNumberOfPositionsInOpeningBook();
int GetNumberOfPositionsVerified();

////////////////////////////////////////////////////////////////////////////////////////////
// GameControl.CPP
///////////////////////////////////////////////////////////////////////////////////////////

// Set some control parameters.
void SetTempusParameters();

void GameControl( struct Board * argsBoard,
                  struct GeneralMove * argsGeneralMove );

// Calculate the time for a move.
void CalculateTimeForMove( struct Board * argsBoard );

// Switch the side to move.
void SwitchSideToMove( struct Board * argsBoard );

// Set some basic parameters.
void SetInitialParameters();
void SetSearchDepth(              int argiDepth );
void SetSearchTimeInMiliSeconds(  int argiTime );
void SetSearchMonitorTogel(       int iTogel );
void SetPersistantKeys(           int iTogel );
void SetModeOfPlay(               int iTogel );
void SetDebug(                    int iTogel );
void SetUseHashTable(             int iTogel );
void SetOpeningBookDebug(         int iTogel );
void SetUseOpeningBook(           int iTogel );
void SetComputerColor(            int iTogel );
void SetInterfaceMode(            int iTogel );
void SetStopGo(                   int iTogel );
void SetOpeningBookAnalysis(      int iTogel );
void SetLimitELO(                 int iTogel );
void SetBlackIncrementalTime(     int iTime );
void SetBlackTime(                int iTime );
void SetMoveTime(                 int iTime );
void SetWhiteIncrementalTime(     int iTime );
void SetWhiteTime(                int iTime );
void SetMovesToGo(                int iMovesToGo );
//void SetNodes(                    int iNodes );
void SetNodes(long long nodes);
void SetMate(                     int iMate );
void SetInterfaceDebug(           int iTogel );
void SetNumberOfNodes(            BitBoard iNodes );
void SetNumberOfIncrementalNodes( BitBoard iNodes );
void SetNodeCheck(                BitBoard iNode );
void SetNumberOfTranspositions(   BitBoard iNode );
void SetStart(                    int iTime );
void SetEnd(                      int iTime );
void SetSearchStartTime(                    );
void SetSearchStopTime(                     );
void SetPollingCount(             int iCount );
void IncrementPollingCount(                 );
void SetUseNullMove(              int iMode );
void SetUseNullEval(              int iMode );
void SetNullReduction(            int iDepth );
void SetLMRMinimumMoveSearch(     int iMoves );
void SetLMRMinimumMoveScore(      int iScore );
void SetLMRReducedSearchDepth(    int iDepth );
void SetUseLMR(                   int iMode );
void SetScore(                    int iScore );
void SetNumberOfPVs(              int iNumberOfPVs );
void SetPonder(                   int iPonder );
void SetTimeLastMoveUpdate(                   );
void SetConsoleOutput(            int iYesNo );
void SetResetHash(                int iYesNo );
void SetIsInBook(                 int iYesNo );
void SetInterfaceMovesDebug(      int iYesNo ); // dYes dNo
void SetInterfaceBookDebug(       int iYesNo ); 
void SetAspirationSearch(         int iYesNo );
void SetNumberOfNodesSearched(    int iNumber );


// Get some basic parameters.
BitBoard GetNumberOfNodes();
BitBoard GetNumberOfIncrementalNodes();
BitBoard GetNodeCheck();
BitBoard GetNumberOfTranspositions();
BitBoard GetPollingCount();
int GetSearchDepth();
int GetSearchTimeInMiliSeconds();
int GetSearchMonitorTogel();
int GetPersistantKeys();
int GetModeOfPlay();
int GetDebug();
int GetUseHashTable();
int GetOpeningBookDebug();
int GetUseOpeningBook();
int GetComputerColor();
int GetInterfaceMode();
int GetStopGo();
int GetOpeningBookAnalysis();
int GetLimitELO();
int GetBlackIncrementalTime();
int GetBlackTime();
int GetMoveTime();
int GetWhiteIncrementalTime();
int GetWhiteTime();
int GetMovesToGo();
//int GetNodes();
long long GetNodes();
int GetMate();
int GetInterfaceDebug();
int GetStart();
int GetEnd();
int GetSearchStartTime();
int GetSearchStopTime();
int GetUseNullMove();
int GetUseNullEval();
int GetNullReduction();
int GetLMRMinimumMoveSearch();
int GetLMRMinimumMoveScore();
int GetLMRReducedSearchDepth();
int GetUseLMR();
int GetScore();
int GetNumberOfPVs();
int GetPonder();
int GetTimeSinceLastMoveUpdate();
int GetConsoleOutput();
int GetResetHash();
int GetIsInBook();
int GetInterfaceMovesDebug(); // dYes dNo
int GetInterfaceBookDebug(); 
int GetAspirationSearch();
int GetNumberOfNodesSearched();


void SetUp( struct Board * argsBoard,
            struct GeneralMove * argsGeneralMoves );

void Game( struct Board * argsBoard,
           struct GeneralMove * argsGeneralMoves );

void ComputerGame( struct Board * argsBoard,
                   struct GeneralMove * argsGeneralMoves );

void BookProbe( struct Board * argsBoard,
                struct GeneralMove * argsGeneralMoves );


/////////////////////////////////////////////////////////////////////
// These are in the one off utilities file.
/////////////////////////////////////////////////////////////////////
void TestSuite( struct Board * argsBoard,
                struct GeneralMove * argsGeneralMoves );

void TestEval( struct Board * argsBoard,
               struct GeneralMove * argsGeneralMoves );

void TestHash( struct Board * argsBoard,
               struct GeneralMove * argsGeneralMoves );

void TestDraw( struct Board * argsBoard,
               struct GeneralMove * argsGeneralMoves );

void UserSetUp( struct Board * argsBoard,
                struct GeneralMove * argsGeneralMoves );

void RunTestEPDFile( const char * argstrFileName,
                     struct Board * argsBoard,
                     struct GeneralMove * argsGeneralMoves );

void Initialize( struct Board * argsBoard,
                 struct GeneralMove * argsGeneralMoves );

void ControlLoop( struct Board * argsBoard,
                  struct GeneralMove * argsGeneralMoves );

void UpdateMultiPV( struct Board * argsBoard,
                    struct GeneralMove * argsGeneralMoves,
                    int argiScore,
                    int argiMoveNumber,
                    int argiNumberOfMoves );

void InitializeMultiPV();

void PrintMultiPV( struct Board * argsBoard,
                   struct GeneralMove * argsGeneralMoves,
                   int iNumberOfMoves );

void Destroy();

std::string FormatWithCommas(long long value);

/////////////////////////////////////////////////////////////////////
// Functions in Evaluation.cpp
/////////////////////////////////////////////////////////////////////

// Initialize the score information.
void InitializeEvaluator();

// Evaulate a board 
int EvaluateBoard( struct Board * argsBoard,
                   struct GeneralMove * argsGeneralMoves );

// A quick evaluation to see if should perform a null move search.
int EvaluateNull( struct Board * argsBoard,
                  struct GeneralMove * argsGeneralMoves );
                            
int CountPiecesAndEvaluatePosition( struct Board * argsBoard,
                                    struct GeneralMove * argsGeneralMoves,
                                    int iScore );

int SliderMobility( struct Board * argsBoard,
                    int * pMoves,
                    int numberOfGeneralMoves,
                    int startSquare );

int RookMobility( struct Board * argsBoard,
                  struct GeneralMove * argsGeneralMoves,
                  int argiSquareIndex );

int BishopMobility( struct Board * argsBoard,
                    struct GeneralMove * argsGeneralMoves,
                    int argiSquareIndex );

int QueenMobility( struct Board * argsBoard, 
                   struct GeneralMove * argsGeneralMoves,
                   int argiSquareIndex );

int KnightMobility( struct Board * argsBoard, 
                    struct GeneralMove * argsGeneralMoves,
                    int argiSquareIndex );

int LookForDraw( struct Board * argsBoard,
                 struct GeneralMove * argsGeneralMoves );

int GetValueOfDraw();

// Add a score for a passed pawns, doubled pawns, isolated pawns
int PawnMisc( struct Board * argsBoard,
              struct GeneralMove * argsGeneralMoves,
              int iSquareIndex );


// Get parameters for other files.
int GetWhitePawnPlacementScore(   int iSquare );
int GetBlackPawnPlacementScore(   int iSquare );
int GetWhiteRookPlacementScore(   int iSquare );
int GetBlackRookPlacementScore(   int iSquare );
int GetWhiteKnightPlacementScore( int iSquare );
int GetBlackKnightPlacementScore( int iSquare );
int GetWhiteBishopPlacementScore( int iSquare );
int GetBlackBishopPlacementScore( int iSquare );
int GetWhiteQueenPlacementScore(  int iSquare );
int GetBlackQueenPlacementScore(  int iSquare );
int GetWhiteKingPlacementScore(   int iSquare );
int GetBlackKingPlacementScore(   int iSquare );
int GetUseMobility();

void SetUseMobility( int iMobility );

/////////////////////////////////////////////////////////////////////
// Functions in Main.cpp
/////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// Functions in GeneralMoves.cpp
////////////////////////////////////////////////////////////////////////////

// Prototype the functions.
void GenerateGeneralMove( struct GeneralMove * sGeneralMove );
void CalculateSelectors(  struct GeneralMove * sGeneralMove );
void CalculatePawns(      struct GeneralMove * sGeneralMove );
void CalculateRooks(      struct GeneralMove * sGeneralMove, int rowIndex, int colIndex, int square );
void CalculateKnights(    struct GeneralMove * sGeneralMove, int rowIndex, int colIndex, int square );
void CalculateBishops(    struct GeneralMove * sGeneralMove, int rowIndex, int colIndex, int square );
void CalculateQueen(      struct GeneralMove * sGeneralMove, int rowIndex, int colIndex, int square );
void CalculateKing(       struct GeneralMove * sGeneralMove, int rowIndex, int colIndex, int square );

void CreateRandomKeyFile();


///////////////////////////////////////////////////////////////////////////////
// Function in FrontEndInterface.cpp
////////////////////////////////////////////////////////////////////////////////

void CommandFromInterface(std::string argstrCommand, struct Board* argsBoard, struct GeneralMove* argsGeneralMoves);
void PositionCommand(std::string argstrCommand, struct Board* argsBoard, struct GeneralMove* argsGeneralMoves);
void GoCommand(std::string argstrCommand, struct Board* argsBoard, struct GeneralMove* argsGeneralMoves);
// Initialize the communications.
void InitializeCommunications();

void SetNodes(long long nodes);

// Here is the main communications funciton.
void ReadInputAndExecute( struct Board * argsBoard,
                          struct GeneralMove * argsGeneralMove );

// Execute a UCI command.
//void CommandFromInterface( char * argstrCommand,
//                           struct Board * argstrBoard,
//                           struct GeneralMove * argsGeneralMoves );

// Send a command to the interface with an eol included.
extern void SendCommand( const char * argstrCommand );

// To do in case of an event.
extern void event ();

// Parse through and act on a "parse" command from UCI.
//void PositionCommand( char * argstrCommand,
//                      struct Board * argsBoard,
//                      struct GeneralMove * argsGeneralMoves );

// Parse through a "go" command the execute.
//void GoCommand( char * argstrCommand,
//                struct Board * argsBoard,
//                struct GeneralMove * argsGeneralMoves );

// See if there is something in the pipe.
int CheckForInput();

// Shut down communications.
void CloseCommunications();

/////////////////////////////////////////////////////////////////////////////
// Tuning.cpp
// These funcitons are used for tuning parameters.
////////////////////////////////////////////////////////////////////////////

void TuneMoveParameters( struct Board * argsBoard,
                         struct GeneralMove * argsGeneralMoves );

#endif // FUNCTIONS_H