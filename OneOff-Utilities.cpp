#include <iostream>
#include <fstream>
#include <string.h>
#include <assert.h>
#include <iomanip>

#include "Definitions.h"
#include "Structures.h"
#include "Functions.h"

using namespace std;

// Simplified computer test routine
void TestComputer( struct Board * argsBoard,
                   struct GeneralMove * argsGeneralMoves )
{
	// Validate pointers.
	assert( argsBoard != nullptr );
	assert( argsGeneralMoves != nullptr );

	// Perform the search using literal constants to avoid macro issues
	// dGo = 1, dAlpha = -99999, dBeta = 99999 (from Definitions.h)
	SetStopGo( 1 );
	int iScore = StartSearch( argsBoard,
							   argsGeneralMoves,
							   -99999,
							   99999 );

	cout << "Score = " << iScore << endl;
	cout << "Number of nodes searched = " << (long long)GetNumberOfNodes() << endl;

	// Print the principal variation (function already exists elsewhere in codebase)
	PrintPrincipalVariation( argsBoard,
							  argsGeneralMoves );
}
