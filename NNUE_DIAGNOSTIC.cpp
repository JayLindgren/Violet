#include <iostream>
#include "Definitions.h"
#include "Structures.h"
#include "Functions.h"

using namespace std;

// Diagnostic Test for NNUE Evaluation
// Place this in your test/debug code

void TestNNUEEvaluation(struct Board *argsBoard, struct GeneralMove *argsGeneralMoves) {
    cout << "=== NNUE Evaluation Diagnostic ===" << endl;
    cout << "iMoveHistory: " << argsBoard->iMoveHistory << endl;
    cout << "siColorToMove: " << (argsBoard->siColorToMove == dWhite ? "White" : "Black") << endl;
    
    // Check accumulator state
    cout << "Accumulator computedAccumulation[" << argsBoard->iMoveHistory << "]: " 
         << argsBoard->sNNUEHistory[argsBoard->iMoveHistory].accumulator.computedAccumulation << endl;
    
    // Force refresh and evaluate
    argsBoard->sNNUEHistory[argsBoard->iMoveHistory].accumulator.computedAccumulation = 0;
    int scoreRefreshed = EvaluateBoardDirectNNUE(argsBoard, argsGeneralMoves);
    cout << "Score (after refresh): " << scoreRefreshed << endl;
    
    // Evaluate again without refresh
    int scoreIncremental = EvaluateBoardDirectNNUE(argsBoard, argsGeneralMoves);
    cout << "Score (incremental): " << scoreIncremental << endl;
    
    if (scoreRefreshed != scoreIncremental) {
        cout << "WARNING: Scores differ! NNUE state may be corrupt." << endl;
    }
    
    // Print FEN for reference
    char fenString[256];
    GenerateFENFromBoard(argsBoard, argsGeneralMoves, fenString);
    cout << "FEN: " << fenString << endl;
    cout << "=================================" << endl;
}
