#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include "Functions.h"
#include "Structures.h"
#include <windows.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>
#include <sstream>
#include <atomic>
#include <map>
#include <fstream>

// =============================================================
// GLOBAL CONSTANTS & DEFINITIONS
// =============================================================

const wchar_t CLASS_NAME[] = L"VioletChessGUI";
const wchar_t APP_TITLE[] = L"Violet Chess Interface";
const int BOARD_SIZE = 600; // Pixels
const int SQUARE_SIZE = BOARD_SIZE / 8;
const int MARGIN = 20;
const int BUTTON_WIDTH = 80;
const int BUTTON_HEIGHT = 30;
const int CONTROL_AREA_HEIGHT = 60; // Extra space at bottom

// Default engine name - change this if your executable is named differently
const std::wstring DEFAULT_ENGINE_PATH = L"violet.exe"; 

// Simple Piece representation
enum PieceType { EMPTY = 0, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
enum PieceColor { NONE = 0, WHITE, BLACK };

struct Piece {
    PieceType type;
    PieceColor color;
};

// Global Pointers to Violet Engine State
struct Board* g_pBoard = nullptr;
struct GeneralMove* g_pGeneralMoves = nullptr;

void Log(std::string msg) {
    std::ofstream logFile("InterfaceLog.txt", std::ios::app);
    if (logFile.is_open()) {
        logFile << msg << std::endl;
        logFile.close();
    }
}

bool ignoreNextMove = false;

// =============================================================
// ENGINE PROCESS HANDLER
// =============================================================

class UciEngine {
private:
    HANDLE hChildStd_IN_Rd = NULL;
    HANDLE hChildStd_IN_Wr = NULL;
    HANDLE hChildStd_OUT_Rd = NULL;
    HANDLE hChildStd_OUT_Wr = NULL;
    HANDLE hProcess = NULL; // Track the engine process
    std::thread readerThread;
    std::atomic<bool> isRunning;
    HWND hWindow; // To post messages back to UI

public:
    UciEngine(HWND hwnd) : hWindow(hwnd), isRunning(false) {}

    ~UciEngine() {
        Stop();
    }

    bool Start(std::wstring path) {
        SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0)) return false;
        if (!SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) return false;
        if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0)) return false;
        if (!SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) return false;

        STARTUPINFO siStartInfo;
        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
        siStartInfo.cb = sizeof(STARTUPINFO);
        siStartInfo.hStdError = hChildStd_OUT_Wr;
        siStartInfo.hStdOutput = hChildStd_OUT_Wr;
        siStartInfo.hStdInput = hChildStd_IN_Rd;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        PROCESS_INFORMATION piProcInfo;
        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

        // Create the child process. 
        // Note: CreateProcessW modifies the command line string, so we need a mutable buffer.
        // Append "--uci" to ensure the engine starts in UCI mode and doesn't spawn another VFE window.
        std::wstring commandLine = path + L" --uci";
        std::vector<wchar_t> cmdLine(commandLine.begin(), commandLine.end());
        cmdLine.push_back(0);

        BOOL bSuccess = CreateProcess(NULL, cmdLine.data(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &siStartInfo, &piProcInfo);

        if (!bSuccess) return false;

        // Keep the process handle so we can terminate it later
        hProcess = piProcInfo.hProcess;
        CloseHandle(piProcInfo.hThread); // Don't need the thread handle
        CloseHandle(hChildStd_OUT_Wr); // Close write end of output pipe
        CloseHandle(hChildStd_IN_Rd); // Close read end of input pipe

        isRunning = true;
        readerThread = std::thread(&UciEngine::ReadLoop, this);
        
        SendCommand("uci");
        // Handshake continues in ProcessLine

        return true;
    }

    void Stop() {
        // Send quit command to engine (UCI protocol)
        if (hChildStd_IN_Wr) {
            SendCommand("quit");
            // Give the engine a moment to process the quit command
            Sleep(100);
        }
        
        // Stop the reader thread
        isRunning = false;
        
        // Close input pipe to unblock any reads
        if (hChildStd_IN_Wr) {
            CloseHandle(hChildStd_IN_Wr);
            hChildStd_IN_Wr = NULL;
        }
        
        // Wait for the reader thread to finish
        if (readerThread.joinable()) {
            readerThread.join();
        }
        
        // Close output pipe
        if (hChildStd_OUT_Rd) {
            CloseHandle(hChildStd_OUT_Rd);
            hChildStd_OUT_Rd = NULL;
        }
        
        // Wait for the process to terminate (up to 2 seconds)
        if (hProcess) {
            DWORD result = WaitForSingleObject(hProcess, 2000);
            if (result == WAIT_TIMEOUT) {
                // Engine didn't exit gracefully, force terminate
                Log("Engine didn't quit gracefully, terminating forcefully");
                TerminateProcess(hProcess, 1);
            }
            CloseHandle(hProcess);
            hProcess = NULL;
        }
    }

    void SendCommand(std::string cmd) {
        if (!hChildStd_IN_Wr) return;
        Log(">> " + cmd);
        cmd += "\n";
        DWORD dwWritten;
        WriteFile(hChildStd_IN_Wr, cmd.c_str(), cmd.length(), &dwWritten, NULL);
    }

private:
    void ReadLoop() {
        DWORD dwRead;
        CHAR chBuf[4096];
        std::string buffer = "";

        while (isRunning) {
            BOOL bSuccess = ReadFile(hChildStd_OUT_Rd, chBuf, 4096, &dwRead, NULL);
            if (!bSuccess || dwRead == 0) break;

            buffer.append(chBuf, dwRead);

            size_t pos = 0;
            while ((pos = buffer.find('\n')) != std::string::npos) {
                std::string line = buffer.substr(0, pos);
                // Remove carriage return if present
                if (!line.empty() && line.back() == '\r') line.pop_back();
                
                ProcessLine(line);
                buffer.erase(0, pos + 1);
            }
        }
    }

    void ProcessLine(std::string line) {
        Log("<< " + line);
        // Basic parsing for "bestmove"
        if (line.find("bestmove") == 0) {
            std::stringstream ss(line);
            std::string segment, moveStr;
            ss >> segment; // "bestmove"
            ss >> moveStr; // e2e4 or similar
            
            // Send custom message to UI thread with the move
            // We use WM_USER + 1
            // We need to allocate a string to pass it safely, UI thread must delete it
            std::string* pMove = new std::string(moveStr);
            PostMessage(hWindow, WM_USER + 1, 0, (LPARAM)pMove);
        }
        else if (line == "uciok") {
            SendCommand("isready");
        }
        else if (line == "readyok") {
            SendCommand("ucinewgame");
            // Signal that engine is ready for the test
            // PostMessage(hWindow, WM_USER + 2, 0, 0); // Auto-play removed
        }
    }
};

// =============================================================
// GAME LOGIC & STATE
// =============================================================

class VFEGame {
public:
    Piece board[8][8];
    PieceColor sideToMove;
    std::vector<std::string> moveHistory;
    std::vector<std::string> redoStack;
    
    // UI Selection State
    int selectedX = -1;
    int selectedY = -1;

    VFEGame() {
        Reset();
    }

    void Reset() {
        // Standard Setup
        // Clear board
        for(int y=0; y<8; y++) 
            for(int x=0; x<8; x++) 
                board[y][x] = {EMPTY, NONE};

        // Pawns
        for(int x=0; x<8; x++) {
            board[1][x] = {PAWN, WHITE};
            board[6][x] = {PAWN, BLACK};
        }

        // Pieces
        PieceType backRank[] = {ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK};
        for(int x=0; x<8; x++) {
            board[0][x] = {backRank[x], WHITE};
            board[7][x] = {backRank[x], BLACK};
        }

        sideToMove = WHITE;
        moveHistory.clear();
        redoStack.clear();
        selectedX = -1;
        selectedY = -1;
        
        // Clear FEN log and log initial position
        std::ofstream fenLog("FEN_Debug.txt", std::ios::trunc);
        fenLog.close();
        
        // Sync Internal Engine Board
        if (g_pBoard && g_pGeneralMoves) {
             CreateBoard(g_pBoard, g_pGeneralMoves);
        }

        LogFEN("Initial position");
    }

    // Convert UCI move string to SAN (Standard Algebraic Notation)
    std::string MoveToSAN(std::string moveStr) {
        if (moveStr.length() < 4) return moveStr;

        int fX = moveStr[0] - 'a';
        int fY = moveStr[1] - '1';
        int tX = moveStr[2] - 'a';
        int tY = moveStr[3] - '1';

        // Range safety
        if (fX < 0 || fX > 7 || fY < 0 || fY > 7 || tX < 0 || tX > 7 || tY < 0 || tY > 7) {
            return moveStr;
        }

        Piece p = board[fY][fX];
        if (p.type == EMPTY) return moveStr; // Fallback if source is empty (prevents garbage output)

        Piece t = board[tY][tX];

        // Castling
        if (p.type == KING && abs(tX - fX) == 2) {
            return (tX > fX) ? "O-O" : "O-O-O";
        }

        std::string san = "";

        // Piece Letter
        if (p.type != PAWN) {
            switch (p.type) {
                case KNIGHT: san += "N"; break;
                case BISHOP: san += "B"; break;
                case ROOK:   san += "R"; break;
                case QUEEN:  san += "Q"; break;
                case KING:   san += "K"; break;
                default: break;
            }
        }

        // Capture
        bool isCapture = (t.type != EMPTY);
        // Pawn diagonal is always capture (handles standard and en passant logic visually)
        if (p.type == PAWN && fX != tX) isCapture = true;

        // Pawn Capture needs source file
        if (p.type == PAWN && isCapture) {
            san += moveStr[0];
        }

        if (isCapture) san += "x";

        // Destination
        san += moveStr.substr(2, 2);

        // Promotion
        if (moveStr.length() >= 5) {
            san += "=";
            san += toupper(moveStr[4]);
        }

        return san;
    }

    bool IsMoveLegal(std::string moveStr) {
        if (!g_pBoard || !g_pGeneralMoves) {
            Log("IsMoveLegal: Board not initialized, assuming true");
            return true;
        }

        struct Move moveList[dNumberOfMoves];
        CalculateMoves(moveList, g_pBoard, g_pGeneralMoves);
        
        int iMoveIndex = -1;
        // FindAlgebraicMove expects char*
        char m[64];
        strcpy(m, moveStr.c_str());
        FindAlgebraicMove(g_pBoard, g_pGeneralMoves, m, moveList, &iMoveIndex);

        if (iMoveIndex == -1) {
            Log("IsMoveLegal: Illegal Move " + moveStr);
            return false;
        }
        return true;
    }

    // Convert e.g., "e2e4" to board update
    bool MakeMove(std::string moveStr, bool preserveRedo = false, bool validate = true) {
        Log("MakeMove: " + moveStr);
        if (moveStr.length() < 4) return false;

        // Validation against Internal Engine
        if (validate && g_pBoard && g_pGeneralMoves) {
            if (!IsMoveLegal(moveStr)) {
                return false;
            }
        }
        
        // If legal or skipping validation, update internal engine state first
        if (g_pBoard && g_pGeneralMoves) {
            // We need to fetch the move index again to execute it
            struct Move moveList[dNumberOfMoves];
            CalculateMoves(moveList, g_pBoard, g_pGeneralMoves);
            int iMoveIndex = -1;
            char m[64];
            strcpy(m, moveStr.c_str());
            FindAlgebraicMove(g_pBoard, g_pGeneralMoves, m, moveList, &iMoveIndex);
            
            if (iMoveIndex != -1) {
                ::MakeMove(moveList, g_pBoard, g_pGeneralMoves, iMoveIndex); // Calls Violet's MakeMove
            }
        }

        // --- Execute Visual Update ---

        int fromX = moveStr[0] - 'a';
        int fromY = moveStr[1] - '1';
        int toX = moveStr[2] - 'a';
        int toY = moveStr[3] - '1';

        Log("Coords: " + std::to_string(fromX) + "," + std::to_string(fromY) + " -> " + std::to_string(toX) + "," + std::to_string(toY));
        Log("From Piece: Type=" + std::to_string(board[fromY][fromX].type) + " Color=" + std::to_string(board[fromY][fromX].color));

        // Check if source is empty (fix for duplicate move commands)
        if (board[fromY][fromX].type == EMPTY) {
            Log("MakeMove Ignored: Source square is empty.");
            return false;
        }

        // Check for castling
        if (board[fromY][fromX].type == KING && abs(toX - fromX) == 2) {
            // Kingside
            if (toX > fromX) {
                board[toY][5] = board[toY][7]; // Move Rook to f-file (index 5)
                board[toY][7] = {EMPTY, NONE}; // Clear h-file rook
            }
            // Queenside
            else {
                board[toY][3] = board[toY][0]; // Move Rook to d-file (index 3)
                board[toY][0] = {EMPTY, NONE}; // Clear a-file rook
            }
        }

        // Move piece
        board[toY][toX] = board[fromY][fromX];
        board[fromY][fromX] = {EMPTY, NONE};

        Log("To Piece After: Type=" + std::to_string(board[toY][toX].type) + " Color=" + std::to_string(board[toY][toX].color));
        if (moveStr.length() == 5) {
            board[toY][toX].type = QUEEN;
        }

        // Toggle side
        sideToMove = (sideToMove == WHITE) ? BLACK : WHITE;
        moveHistory.push_back(moveStr);
        
        if (!preserveRedo) {
            redoStack.clear();
        }

        // Log FEN for debugging
        LogFEN("After move: " + moveStr);
        return true;
    }

    std::string GetPositionCommand() {
        std::string cmd = "position startpos moves";
        for (const auto& m : moveHistory) {
            cmd += " " + m;
        }
        return cmd;
    }

    // Generate FEN string from current board state
    std::string GetFEN() {
        std::string fen = "";
        
        // 1. Piece placement (from rank 8 to rank 1)
        for (int y = 7; y >= 0; y--) {
            int emptyCount = 0;
            for (int x = 0; x < 8; x++) {
                Piece p = board[y][x];
                if (p.type == EMPTY) {
                    emptyCount++;
                } else {
                    if (emptyCount > 0) {
                        fen += std::to_string(emptyCount);
                        emptyCount = 0;
                    }
                    // Add piece character
                    char piece = ' ';
                    switch (p.type) {
                        case PAWN:   piece = 'p'; break;
                        case KNIGHT: piece = 'n'; break;
                        case BISHOP: piece = 'b'; break;
                        case ROOK:   piece = 'r'; break;
                        case QUEEN:  piece = 'q'; break;
                        case KING:   piece = 'k'; break;
                        default: break;
                    }
                    if (p.color == WHITE) {
                        piece = toupper(piece);
                    }
                    fen += piece;
                }
            }
            if (emptyCount > 0) {
                fen += std::to_string(emptyCount);
            }
            if (y > 0) fen += "/";
        }
        
        // 2. Active color
        fen += (sideToMove == WHITE) ? " w " : " b ";
        
        // 3. Castling availability (simplified - assume all castling rights until moved)
        // For a full implementation, you'd need to track king/rook movements
        fen += "KQkq ";
        
        // 4. En passant target square (not tracked in this simple implementation)
        fen += "- ";
        
        // 5. Halfmove clock (not tracked)
        fen += "0 ";
        
        // 6. Fullmove number
        int fullmove = (moveHistory.size() / 2) + 1;
        fen += std::to_string(fullmove);
        
        return fen;
    }

    // Log FEN to a debug file
    void LogFEN(const std::string& context = "") {
        std::ofstream fenLog("FEN_Debug.txt", std::ios::app);
        if (fenLog.is_open()) {
            std::string fen = GetFEN();
            if (!context.empty()) {
                fenLog << "[" << context << "] ";
            }
            fenLog << fen << std::endl;
            fenLog.close();
        }
    }
    
    std::string CoordsToMove(int fx, int fy, int tx, int ty) {
        char fFile = 'a' + fx;
        char fRank = '1' + fy;
        char tFile = 'a' + tx;
        char tRank = '1' + ty;
        
        std::string move = "";
        move += fFile; move += fRank;
        move += tFile; move += tRank;

        // Auto-promote to Queen for GUI simplicity logic
        if (board[fy][fx].type == PAWN) {
            if ((board[fy][fx].color == WHITE && ty == 7) ||
                (board[fy][fx].color == BLACK && ty == 0)) {
                move += "q";
            }
        }
        return move;
    }

    void UndoLastMove() {
        if (moveHistory.empty()) return;

        // Save for redo
        std::string lastMove = moveHistory.back();
        redoStack.push_back(lastMove);

        // Create copy of history without the last move
        std::vector<std::string> tempHistory = moveHistory;
        tempHistory.pop_back();

        // Reset board to initial state
        // Visual Reset
        for(int y=0; y<8; y++) for(int x=0; x<8; x++) board[y][x] = {EMPTY, NONE};
        for(int x=0; x<8; x++) { board[1][x] = {PAWN, WHITE}; board[6][x] = {PAWN, BLACK}; }
        PieceType backRank[] = {ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK};
        for(int x=0; x<8; x++) { board[0][x] = {backRank[x], WHITE}; board[7][x] = {backRank[x], BLACK}; }
        sideToMove = WHITE;
        moveHistory.clear();
        selectedX = -1; selectedY = -1;

        // Engine Reset
        if (g_pBoard && g_pGeneralMoves) {
             ::UndoMove(g_pBoard, g_pGeneralMoves);
        }

        // Replay all moves except the last one for VISUAL persistence?
        // Wait, Undoing just one move on internal board works via UndoMove.
        // But Visual uses Reset + Replay.
        // Replay visual:
        for (const auto& move : tempHistory) {
            MakeMove(move, true, false); // preserveRedo = true, validate = false (we assume history was valid)
        }
    }

    void RedoMove() {
        if (redoStack.empty()) return;
        std::string move = redoStack.back();
        redoStack.pop_back();
        MakeMove(move, true, true); // preserveRedo = true, validate = true
    }
};

VFEGame game;
UciEngine* engine = nullptr;
bool isThinking = false;
std::string lastEngineMove = "";

// Helper to draw Unicode Chess Pieces
void DrawPiece(HDC hdc, int x, int y, Piece p) {
    if (p.type == EMPTY) return;

    int base = 0x2654;
    int offset = 0;
    
    if (p.color == BLACK) offset += 6;

    switch(p.type) {
        case KING:   offset += 0; break;
        case QUEEN:  offset += 1; break;
        case ROOK:   offset += 2; break;
        case BISHOP: offset += 3; break;
        case KNIGHT: offset += 4; break;
        case PAWN:   offset += 5; break;
        default: break;
    }

    wchar_t symbol[2];
    symbol[0] = (wchar_t)(base + offset);
    symbol[1] = 0;

    // Center text in square
    RECT rect;
    rect.left = MARGIN + x * SQUARE_SIZE;
    rect.top = MARGIN + (7 - y) * SQUARE_SIZE; // Flip Y for drawing
    rect.right = rect.left + SQUARE_SIZE;
    rect.bottom = rect.top + SQUARE_SIZE;

    SetBkMode(hdc, TRANSPARENT);
    
    if (p.color == WHITE) SetTextColor(hdc, RGB(20, 20, 20)); // White pieces (Black outline)
    else SetTextColor(hdc, RGB(10, 10, 10)); // Black pieces

    DrawTextW(hdc, symbol, 1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void DrawBoard(HDC hdc) {
    // 1. Draw Squares
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int drawY = 7 - y; // Render rank 1 at bottom
            RECT rect;
            rect.left = MARGIN + x * SQUARE_SIZE;
            rect.top = MARGIN + drawY * SQUARE_SIZE;
            rect.right = rect.left + SQUARE_SIZE;
            rect.bottom = rect.top + SQUARE_SIZE;

            // Checkerboard colors
            HBRUSH brush;
            if ((x + y) % 2 != 0) {
                 brush = CreateSolidBrush(RGB(118, 150, 86)); // Greenish
            } else {
                 brush = CreateSolidBrush(RGB(238, 238, 210)); // Cream
            }

            FillRect(hdc, &rect, brush);
            DeleteObject(brush);

            // Highlight selection
            if (game.selectedX == x && game.selectedY == y) {
                HBRUSH selBrush = CreateSolidBrush(RGB(186, 202, 68)); // Yellow highlight
                FrameRect(hdc, &rect, selBrush);
                FillRect(hdc, &rect, selBrush); // Just fill it for visibility
                DeleteObject(selBrush);
            }
        }
    }

    // 2. Setup Font
    HFONT hFont = CreateFont(
        SQUARE_SIZE - 10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Symbol"
    );
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    // 3. Draw Pieces
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            DrawPiece(hdc, x, y, game.board[y][x]);
        }
    }

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);

    // 4. Draw Status
    std::wstring statusText = L"";
    if (isThinking) statusText = L"Thinking...";
    else if (!lastEngineMove.empty()) {
        std::wstring wLastMove(lastEngineMove.begin(), lastEngineMove.end());
        statusText = L"Last Move: " + wLastMove;
    }

    if (!statusText.empty()) {
        RECT statusRect = { MARGIN, BOARD_SIZE + MARGIN + 5, BOARD_SIZE + MARGIN, BOARD_SIZE + MARGIN + 30 };
        
        // Clear previous text
        HBRUSH hBackground = (HBRUSH)(COLOR_WINDOW + 1);
        FillRect(hdc, &statusRect, hBackground);

        SetTextColor(hdc, RGB(0, 0, 0));
        SetBkMode(hdc, TRANSPARENT); // Ensure text background is transparent
        DrawTextW(hdc, statusText.c_str(), -1, &statusRect, DT_LEFT | DT_SINGLELINE);
    }
}

void DrawControls(HDC hdc) {
    // Button Positions (below status text)
    int yPos = BOARD_SIZE + MARGIN + 40;
    RECT rcBack = { MARGIN, yPos, MARGIN + BUTTON_WIDTH, yPos + BUTTON_HEIGHT };
    RECT rcFwd = { MARGIN + BUTTON_WIDTH + 10, yPos, MARGIN + BUTTON_WIDTH * 2 + 10, yPos + BUTTON_HEIGHT };

    // Draw Back Button
    bool canUndo = !game.moveHistory.empty();
    SetBkMode(hdc, TRANSPARENT);
    
    // Background
    HBRUSH bgBrush = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    if (!canUndo) bgBrush = (HBRUSH)GetStockObject(LTGRAY_BRUSH); // Simulating disabled
    
    // Draw button chrome
    DrawFrameControl(hdc, &rcBack, DFC_BUTTON, DFCS_BUTTONPUSH | (canUndo ? 0 : DFCS_INACTIVE));
    
    // Text/Icon
    SetTextColor(hdc, canUndo ? RGB(0,0,0) : RGB(128,128,128));
    DrawTextW(hdc, L"◀", -1, &rcBack, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Draw Forward Button
    bool canRedo = !game.redoStack.empty();
    DrawFrameControl(hdc, &rcFwd, DFC_BUTTON, DFCS_BUTTONPUSH | (canRedo ? 0 : DFCS_INACTIVE));
    
    SetTextColor(hdc, canRedo ? RGB(0,0,0) : RGB(128,128,128));
    DrawTextW(hdc, L"▶", -1, &rcFwd, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void SavePGN(HWND hwnd) {
    OPENFILENAME ofn;
    wchar_t szFile[260];
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"PGN Files\0*.pgn\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = L"pgn";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn)) {
        HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            std::string header = "[Event \"Casual Game\"]\n[Site \"Violet GUI\"]\n[Date \"????.??.??\"]\n[Round \"1\"]\n[White \"User\"]\n[Black \"Violet\"]\n[Result \"*\"]\n\n";
            DWORD dwWritten;
            WriteFile(hFile, header.c_str(), header.length(), &dwWritten, NULL);

            std::string pgnText = "";
            for (size_t i = 0; i < game.moveHistory.size(); i++) {
                if (i % 2 == 0) {
                    pgnText += std::to_string((i / 2) + 1) + ". ";
                }
                pgnText += game.moveHistory[i] + " ";
            }
            pgnText += "*";
            WriteFile(hFile, pgnText.c_str(), pgnText.length(), &dwWritten, NULL);
            CloseHandle(hFile);
            MessageBox(hwnd, L"Game saved.", L"Info", MB_OK);
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        engine = new UciEngine(hwnd);
        // Try to load default engine
        if (!engine->Start(DEFAULT_ENGINE_PATH)) {
            // Optional: Alert user engine wasn't found immediately
        }
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        DrawBoard(hdc);
        DrawControls(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        // Prevent interaction if engine is thinking
        if (isThinking) return 0;

        int xPos = LOWORD(lParam);
        int yPos = HIWORD(lParam);

        // Check Button Clicks
        int btnY = BOARD_SIZE + MARGIN + 40;
        RECT rcBack = { MARGIN, btnY, MARGIN + BUTTON_WIDTH, btnY + BUTTON_HEIGHT };
        RECT rcFwd = { MARGIN + BUTTON_WIDTH + 10, btnY, MARGIN + BUTTON_WIDTH * 2 + 10, btnY + BUTTON_HEIGHT };
        
        POINT pt = { xPos, yPos };

        // Undo Click
        if (PtInRect(&rcBack, pt)) {
             if (game.moveHistory.empty()) return 0;

             // Logic from Menu
             if (isThinking) {
                if (engine) engine->SendCommand("stop");
                ignoreNextMove = true;
                isThinking = false;
             }
             game.UndoLastMove();
             lastEngineMove = "";
             if (engine) engine->SendCommand(game.GetPositionCommand());
             InvalidateRect(hwnd, NULL, FALSE);
             return 0;
        }

        // Redo Click
        if (PtInRect(&rcFwd, pt)) {
             if (game.redoStack.empty()) return 0;
             
             game.RedoMove();
             lastEngineMove = "";
             if (engine) engine->SendCommand(game.GetPositionCommand());
             InvalidateRect(hwnd, NULL, FALSE);
             return 0;
        }

        // Convert mouse to board coords
        int gridX = (xPos - MARGIN) / SQUARE_SIZE;
        int gridY = 7 - ((yPos - MARGIN) / SQUARE_SIZE); // Flip Y

        if (gridX >= 0 && gridX < 8 && gridY >= 0 && gridY < 8) {
            
            if (game.selectedX == -1) {
                // Select
                if (game.board[gridY][gridX].type != EMPTY && 
                    game.board[gridY][gridX].color == game.sideToMove) {
                    game.selectedX = gridX;
                    game.selectedY = gridY;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else {
                // Move Attempt
                // 1. Check if clicking same square (deselect)
                if (gridX == game.selectedX && gridY == game.selectedY) {
                    game.selectedX = -1;
                    game.selectedY = -1;
                }
                else {
                    // 2. Construct UCI move string
                    std::string moveStr = game.CoordsToMove(game.selectedX, game.selectedY, gridX, gridY);
                    
                    // 3. Try to make move (includes legality check)
                    if (game.MakeMove(moveStr)) {
                        // 4. Send to engine
                        if (engine) {
                            engine->SendCommand(game.GetPositionCommand());
                            engine->SendCommand("go"); 
                            isThinking = true;
                        }
                        
                        // 5. Deselect
                        game.selectedX = -1;
                        game.selectedY = -1;
                    } else {
                        // Invalid Move
                        // Just deselect or maybe flash?
                         game.selectedX = -1;
                         game.selectedY = -1;
                         Log("Invalid Move: " + moveStr);
                         MessageBox(hwnd, L"Illegal Move", L"Invalid Move", MB_OK);
                    }
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        return 0;
    }

    case WM_USER + 1: { // Engine sent a move
        std::string* moveStr = (std::string*)lParam;
        if (moveStr) {
            if (ignoreNextMove) {
                Log("Ignoring move due to Undo: " + *moveStr);
                ignoreNextMove = false;
                isThinking = false;
                delete moveStr;
                return 0;
            }

            // Calculate SAN before making the move (we need the board state)
            std::string san = game.MoveToSAN(*moveStr);
            
            // Engine moves are assumed legal, but we can set validate=false to be safe or true to double check
            if (game.MakeMove(*moveStr, false, true)) {
                lastEngineMove = san; 
                isThinking = false;
                InvalidateRect(hwnd, NULL, FALSE);
            } else {
                Log("Duplicate or invalid move received from engine: " + *moveStr);
            }
            delete moveStr;
        }
        return 0;
    }

    case WM_RBUTTONUP: {
        HMENU hMenu = CreatePopupMenu();
        AppendMenu(hMenu, MF_STRING, 1, L"New Game");
        AppendMenu(hMenu, MF_STRING, 4, L"Backward (Undo)");
        AppendMenu(hMenu, MF_STRING, 5, L"Forward (Redo)");
        AppendMenu(hMenu, MF_STRING, 2, L"Load Engine (.exe)...");
        AppendMenu(hMenu, MF_STRING, 3, L"Save PGN...");
        
        POINT pt;
        GetCursorPos(&pt);
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
        DestroyMenu(hMenu);
        return 0;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case 1: // New Game
            game.Reset();
            lastEngineMove = "";
            ignoreNextMove = false;
            isThinking = false;
            if (engine) engine->SendCommand("ucinewgame");
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case 4: // Undo Last Move
            if (isThinking) {
                // If engine is thinking, stop it and ignore the result
                if (engine) engine->SendCommand("stop");
                ignoreNextMove = true;
                isThinking = false;
            }
            game.UndoLastMove();
            lastEngineMove = ""; // Clear last move display as it might be invalid
            
            // Sync engine with new position
            if (engine) {
                engine->SendCommand(game.GetPositionCommand());
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case 5: // Forward (Redo)
            if (isThinking) break; // Can't redo while thinking
            game.RedoMove();
            lastEngineMove = "";
             // Sync engine with new position
            if (engine) {
                engine->SendCommand(game.GetPositionCommand());
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case 2: { // Load Engine
            OPENFILENAME ofn;
            wchar_t szFile[260];
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            ofn.lpstrFile[0] = '\0';
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"Executables\0*.exe\0All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            
            if (GetOpenFileName(&ofn)) {
                if(engine) delete engine;
                engine = new UciEngine(hwnd);
                if(engine->Start(szFile)) {
                    MessageBox(hwnd, L"Engine Loaded Successfully", L"Info", MB_OK);
                }
            }
            break;
        }
        case 3: // Save PGN
            SavePGN(hwnd);
            break;
        }
        return 0;
    }

    case WM_DESTROY:
        if (engine) delete engine;
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void RunVFE(struct Board *argsBoard, struct GeneralMove *argsGeneralMoves) {
    g_pBoard = argsBoard;
    g_pGeneralMoves = argsGeneralMoves;

    // Reset game with attached board
    game.Reset();

    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    // Calculate window size based on board size + margins
    RECT wr = { 0, 0, BOARD_SIZE + (MARGIN * 2), BOARD_SIZE + (MARGIN * 2) + CONTROL_AREA_HEIGHT };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, APP_TITLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 
        wr.right - wr.left, wr.bottom - wr.top,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return;

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}