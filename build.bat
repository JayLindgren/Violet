@echo off
echo Building Violet...
g++ -static -O3 -mbmi2 -std=c++17 -pthread -I Fathom-master/src -o Violet.exe main.cpp AspirationTest.cpp Bitboard.cpp Evaluation.cpp FrontEndInterface.cpp GameControl.cpp GeneralMoves.cpp HashTable.cpp HashTableTest.cpp MoveOrder.cpp Moves.cpp NullMoveTest.cpp OneOffUtilities.cpp OpeningBook.cpp PEXTInit.cpp SEE.cpp SEETest.cpp Search.cpp TestSuites.cpp Thread.cpp Tuning.cpp misc.cpp nnue.cpp VFE.cpp Fathom-master/src/tbprobe.c -lgdi32 -lcomdlg32
if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b %errorlevel%
)
echo Build successful! Violet.exe created.
