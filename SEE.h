#ifndef SEE_H
#define SEE_H

#include "Structures.h"

//
//---------------------------------------------------------------------
//
// Static Exchange Evaluation
// Returns the approximate material gain/loss of the capture 'move'.
// If the move is not a capture, it returns 0 (or the value of the promotion).
int See( struct Board * argsBoard, 
         struct GeneralMove * argsGeneralMoves, 
         struct Move * argsMove );

//
//---------------------------------------------------------------------
//
// Helper for internal use or direct calls
int SeeCapture( struct Board * argsBoard, 
                struct GeneralMove * argsGeneralMoves, 
                int iFrom, 
                int iTo, 
                int siSide, 
                int iCapturedPieceType );

//
//---------------------------------------------------------------------
//
// Helper to find the least valuable attacker
int GetSmallestAttacker( struct Board * argsBoard, 
                         struct GeneralMove * argsGeneralMoves, 
                         int iSquare, 
                         int siSide, 
                         BitBoard bbOccupied, 
                         int & iAttackerSq, 
                         int & iAttackerPiece );

#endif // SEE_H
