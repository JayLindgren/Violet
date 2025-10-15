// Copyright 2006 by Jay Lindgren. All Rights Reserved.
//
//
// Program Violet.
// Please tie your hair up in a ribbon before playing.
//
// This program was written by Jay Lindgren starting 9/2/2003
//
// 

#include "Definitions.h"
#include "Functions.h"
#include "Structures.h"

#include <string.h>
#include <stdio.h>
#include <iostream>

using namespace std;

//
//-----------------------------------------------------------------------
//
int main()
{

    // Allocate the structures.
   struct GeneralMove sGeneralMoves;
   struct Board       sBoard;

   // Initialize that which needs to be.
   Initialize( & sBoard,
               & sGeneralMoves );

   // The main control loop.
   ControlLoop( & sBoard,
                & sGeneralMoves );

   // Free the large chunks of memory as we leave.
   Destroy();

   return 0;
}
