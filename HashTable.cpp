
#include <iostream>
//#include <stdlib.h>
#include <fstream>
#include "Functions.h"
#include "Structures.h"
// *include <omp.h>
#include "Definitions.h"
#include <math.h>
#include <string.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Define global variables for keeping track of the number of nodes counted.
// Global variables suck, but are awsome for allowing for Deep Violet.
// access to the table:
// Note that the scope for the globe variables is only this file.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   HashTable gsHashTable;

//
//-------------------------------------------------------------------------------------------------------------
//
void ClearHashTable()
{

   // Rest the hash table.
   for ( BitBoard bbHashIndex = 0; bbHashIndex < gsHashTable.bbNumberOfHashElements; bbHashIndex++ )
   {

      gsHashTable.mbbHashTable[ bbHashIndex ] = 0;
      gsHashTable.mbbHash[ bbHashIndex ]      = 0;

   }

}


//
//-------------------------------------------------------------------------------------------------------------
//
void InitializeHashTable()
{
// This function is used to initialize the hash table.  The function is stored in the search file because the 
// hash table is a global variable and is needed the most in the search routines.
//

//# if defined( dUseHash )
   // Allocate the memory.
   gsHashTable.bbNumberOfHashElements = Power( 2, dNumberOfBitsInHash ) - 1;
   //gsHashTable.bbNumberOfHashElements = 1048575;
   gsHashTable.mbbHash = ( BitBoard * ) malloc( gsHashTable.bbNumberOfHashElements * sizeof( BitBoard ) );
   gsHashTable.mbbHashTable = ( BitBoard * ) malloc( gsHashTable.bbNumberOfHashElements * sizeof( BitBoard ) ); 
//# endif

   // Switch on using persistant or new keys.
   if ( GetPersistantKeys() == dNewKeys )
   {

      // Set up the hash keys.
      int iSquareIndex = 0;
      for ( iSquareIndex = 0; iSquareIndex < 64; iSquareIndex++ )
      {

         for( int iPieceIndex = 0; iPieceIndex < 12; iPieceIndex++ )
         {

           gsHashTable.mbbHashKeys[ iPieceIndex ][ iSquareIndex ] = RandomBB();

         }

         gsHashTable.vbbEnPassant[ iSquareIndex ] = RandomBB();

      }

      for ( int iStateIndex = 0; iStateIndex < 6; iStateIndex++ )
      {

         gsHashTable.vbbHashKeysStates[ iStateIndex ] = RandomBB();

      }

      gsHashTable.vbbCasteling[ 0 ] = RandomBB();
      gsHashTable.vbbCasteling[ 1 ] = RandomBB();
      gsHashTable.vbbCasteling[ 2 ] = RandomBB();
      gsHashTable.vbbCasteling[ 3 ] = RandomBB();

      gsHashTable.vbbColorToMove[ 0 ] = RandomBB();
      gsHashTable.vbbColorToMove[ 1 ] = RandomBB();


      // Set up the initial hash.
      gsHashTable.bbHash = RandomBB();

   }
   else if ( GetPersistantKeys() == dFile )
   {

      ReadRandomKeyFile();
      //cout << "Initial Hash = " << gsHashTable.bbHashInitial << endl;

   }
   else if ( GetPersistantKeys() == dCode )
   {

      // Read the keys
      GetAllKeys();
      AssignRandomKeys();
      //cout << "Initial Hash = " << gsHashTable.bbHashInitial << endl;

   }

//# if defined( dUseHash )  
   // Loop over the hash table entries and set them to zero.
   // #pragma omp parallel for schedule( dynamic, 1 )
   for ( BitBoard bbHashIndex = 0; bbHashIndex < gsHashTable.bbNumberOfHashElements; bbHashIndex++ )
   {

      gsHashTable.mbbHashTable[ bbHashIndex ] = 0;
      gsHashTable.mbbHash[ bbHashIndex ]      = 0;

   }
//# endif
   
   // Set up the mask for extracting the index.
   // Note the +1 is there for rounding up.
   //int numberOfBits = (int)( log( (double) gsHashTable.bbNumberOfHashElements ) / log( 2.0 ) + 1 );
   gsHashTable.bbMaskIndex = 0;
   for ( int iBitIndex = 0; iBitIndex < dNumberOfBitsInHash; iBitIndex++ )
   {
   
      gsHashTable.bbMaskIndex = SetBitToOne( gsHashTable.bbMaskIndex, iBitIndex );
      
   }

   // Set the initial hash for later reference
   gsHashTable.bbHashInitial = gsHashTable.bbHash;

}

//
//-----------------------------------------------------------------------
//
void UpdateHash( struct Move * argsMove,
                 struct Board * argsBoard,
                 int iMakeUnmake,
                 struct GeneralMove * argsGeneralMoves )

{

   // Debug the inputs
   assert( argsMove >= 0 );
   assert( argsBoard >= 0 );
   assert( iMakeUnmake >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Update the hash.
   int iToSquare   = argsMove->iToSquare;
   int iFromSquare = argsMove->iFromSquare;
   int iPiece      = argsMove->iPiece;
   int iCapture    = argsMove->iCapture;
   int iMoveType   = argsMove->iMoveType;

   assert( iToSquare >= 0 );
   assert( iToSquare <= 64 );
   assert( iFromSquare >= 0 );
   assert( iFromSquare <= 64 );
   assert( iPiece >= 0 );
   assert( iPiece <= dBlackKing );
   assert( iCapture >= 0 );
   assert( iCapture <= 64 );
   assert( iMoveType >= 0 );


   // Do a normal move.
   gsHashTable.bbHash = gsHashTable.bbHash ^
                        gsHashTable.mbbHashKeys[ iPiece ][ iToSquare ] ^
                        gsHashTable.mbbHashKeys[ iPiece ][ iFromSquare ];

   // Do a capture.
   if ( iMoveType == dCapture )
   {

     gsHashTable.bbHash = gsHashTable.bbHash ^
                          gsHashTable.mbbHashKeys[ iCapture ][ iToSquare ];

   }

   // Do a promotion.
   if ( ( iMoveType == dPromote ) ||
        ( iMoveType == dCaptureAndPromote ) )
   {

     gsHashTable.bbHash = gsHashTable.bbHash ^
                          gsHashTable.mbbHashKeys[ iPiece ][ iToSquare ] ^
                          gsHashTable.mbbHashKeys[ 1 ][ iFromSquare ];

   }

   // Do that EP thing.
   if ( iMoveType == dEnPassant )
   {

     gsHashTable.bbHash = gsHashTable.bbHash ^
                          gsHashTable.vbbEnPassant[ iToSquare ];

   }

   // Handle the castling possibilities.
   if ( iMoveType == dWhiteKingSideCastle )
   {

      gsHashTable.bbHash = gsHashTable.bbHash ^
                           gsHashTable.vbbHashKeysStates[ 0 ];

   }
   else if ( iMoveType == dWhiteQueenSideCastle )   
   {

      gsHashTable.bbHash = gsHashTable.bbHash ^
                           gsHashTable.vbbHashKeysStates[ 1 ];

   }
   else if ( iMoveType == dBlackKingSideCastle )   
   {

      gsHashTable.bbHash = gsHashTable.bbHash ^
                           gsHashTable.vbbHashKeysStates[ 2 ];

   }
   else if ( iMoveType == dBlackQueenSideCastle )   
   {

      gsHashTable.bbHash = gsHashTable.bbHash ^
                           gsHashTable.vbbHashKeysStates[ 3 ];

   }

   // Switch on the colors, note that since we need to remove on color and add the other
   // this operation will always be performed.
   if ( iPiece < dBlackPawn )
   {

       gsHashTable.bbHash = gsHashTable.bbHash ^
                            gsHashTable.vbbHashKeysStates[ 0 ];

   }
   else
   {

       gsHashTable.bbHash = gsHashTable.bbHash ^
                            gsHashTable.vbbHashKeysStates[ 1 ];

   }

}

//
//--------------------------------------------------------------------------
//
void ExtractFromHashTable( struct Board * argsBoard,
                           struct GeneralMove * argsGeneralMoves )
{
// This function will return gsHashTable.iQueryState =.
// -1 = not found.
//  0 = empty element.
//  1 = not found.

   // Debug the inputs
   assert( argsBoard >= 0 );
   assert( argsGeneralMoves >= 0 );

   // Reset the best move.
   argsBoard->iBestMove = 128;

   // Extract the data from hash table.
   BitBoard bbKey = gsHashTable.bbHash & gsHashTable.bbMaskIndex;   
   
   // Extract the depth.
   gsHashTable.bbDepth = ( gsHashTable.mbbHashTable[ bbKey ] & 
                           argsGeneralMoves->bbDepth ) >>
                           argsGeneralMoves->iDepthShift;
 
   // Debugging.
   assert( bbKey >= 0 );   
   assert( bbKey <=  gsHashTable.bbNumberOfHashElements );

#if defined( dDeepMode )
//   #pragma omp critical // only let this be executed one thread at a time.
#endif
   { 

      // Check if we are using the table.
      if ( GetUseHashTable() == dNo )
      {

         // We have not touched this move before or the keys don't match.
         gsHashTable.bbScore       = 0;
         gsHashTable.bbDepth       = 0;
         gsHashTable.bbBestMove    = 0;
         gsHashTable.bbDanger      = 0;
         gsHashTable.bbTypeOfScore = 0;
         gsHashTable.bbAge         = 0;
         gsHashTable.iQueryState   = 0;
         //return;

      }
      // Check the position.
      // See if it has been found before.
      // Also check to see if we have a key collision.
      else if ( ( gsHashTable.mbbHash[ bbKey ] == 0 ) ||
                ( gsHashTable.mbbHash[ bbKey ] != gsHashTable.bbHash ) ||
                ( gsHashTable.bbDepth < ( argsBoard->iMaxPlys - argsBoard->iNumberOfPlys )  ) )
      {

         // We have not touched this move before or the keys don't match.
         gsHashTable.bbScore       = 0;
         gsHashTable.bbDepth       = 0;
         gsHashTable.bbBestMove    = 0;
         gsHashTable.bbDanger      = 0;
         gsHashTable.bbTypeOfScore = 0;
         gsHashTable.bbAge         = 0;
         gsHashTable.iQueryState   = 0;
         //return;

      }
      else
      {
   
         // We have seen this position before:
         // Extract the score and the sign.
         BitBoard bbSign = argsGeneralMoves->bbScoreSign & gsHashTable.mbbHashTable[ bbKey ];  
         bbSign = bbSign >> argsGeneralMoves->iSignShift; 
         gsHashTable.iScore =  (int)( gsHashTable.mbbHashTable[ bbKey ] & argsGeneralMoves->bbScore );

         if ( bbSign >= 1 )
         {

            gsHashTable.iScore = - gsHashTable.iScore;

         }

         // Extract the best move.
         gsHashTable.bbBestMove = ( gsHashTable.mbbHashTable[ bbKey ] & 
                                    argsGeneralMoves->bbBestMove ) >>
                                    argsGeneralMoves->iBestMoveShift;

      /*
         // Extract the danger.
         gsHashTable.bbDanger = ( gsHashTable.mbbHashTable[ bbKey ] & 
                                  argsGeneralMoves->bbDanger ) >>
                                  argsGeneralMoves->iDangerShift;
    
      */
         // Extract the age.
         gsHashTable.bbAge = ( gsHashTable.mbbHashTable[ bbKey ] & 
                               argsGeneralMoves->bbAge ) >>
                               argsGeneralMoves->iAgeShift;

         // Return a found position.
        gsHashTable.iQueryState = 1;

      }

   } // end Pragma Critical

}

//
//--------------------------------------------------------------------------
//
void InputToHashTable( struct Board * argsBoard,
                       struct GeneralMove * argsGeneralMoves,
                       int argiAlpha,
                       int argiBeta,
                       int argiScore,
                       struct Move * argsMove )
{
// Put the score and the best move into the hash table.

   // If the hash table isn't being used, jump out.
   if ( GetUseHashTable() == dNo )
   {

      return;

   }

#if defined( dDeepMode )
//   #pragma omp critical // only let this be executed one thread at a time.
#endif
   { 

      // Some scoring correction to make sure the score fits.
      if ( argiAlpha < dMate )
      {

         argiAlpha = dMate;

      }
      if ( argiBeta < dMate )
      {

         argiBeta = dMate;

      }
      if ( argiAlpha > - dMate )
      {

         argiAlpha = - dMate;

      }
      if ( argiBeta > - dMate )
      {

         argiBeta = - dMate;

      }

      assert( argsBoard >= 0 );
      assert( argsGeneralMoves >= 0 );
      assert( argiAlpha >= dMate );
      assert( argiBeta >= dMate );
      assert( argiScore >= dMate );
      assert( argsMove >= 0 );

      // Input the data to hash table.
      BitBoard bbKey = gsHashTable.bbHash & gsHashTable.bbMaskIndex;
      //BitBoard bbKey = GetKey();
      assert( bbKey <  gsHashTable.bbNumberOfHashElements );

      // Extract the depth and replace on that criteria.
      BitBoard bbDepthOld = ( gsHashTable.mbbHashTable[ bbKey ] & 
                              argsGeneralMoves->bbDepth ) >>
                              argsGeneralMoves->iDepthShift;

      // Calculate the new depth.
      int iDepth = argsBoard->iMaxPlys - argsBoard->iNumberOfPlys - 1;
      if ( iDepth < 0 )
      {

         iDepth = 0;

      }
      BitBoard bbDepth = iDepth; 

      // Replace only based on depth and the key being the same
      // If the hash is different go ahead and replace.
      if ( ( bbDepthOld <= bbDepth ) &&
           ( gsHashTable.mbbHash[ bbKey ] == gsHashTable.bbHash ) )
      {

         // Assign the full key.
         // Not this will replace any existing position.
         gsHashTable.mbbHash[ bbKey ] = gsHashTable.bbHash;
      
         // Clear the data.
         gsHashTable.mbbHashTable[ bbKey ] = 0;

         // Calculate the depth and enter it.
         bbDepth = bbDepth << argsGeneralMoves->iDepthShift;
         gsHashTable.mbbHashTable[ bbKey ] |= bbDepth;

         // Input the score and handle the sign.
         BitBoard bbScore = 0;
         if ( argiScore < 0 )
         {

            // Put the sign in the first bit.
            bbScore = - argiScore;
            bbScore = SetBitToOne( bbScore, 17 );

         }
         else
         {

            // The first bit will be zero by initialization.
            bbScore = argiScore;

         }
         bbScore = bbScore << argsGeneralMoves->iScoreShift;
         gsHashTable.mbbHashTable[ bbKey ] |= bbScore;

         // Calculate the input the best move.
         if ( argsBoard->iBestMove < 128 )
         {

            BitBoard bbBestMove = argsBoard->iBestMove;
            bbBestMove = bbBestMove << argsGeneralMoves->iBestMoveShift;
            gsHashTable.mbbHashTable[ bbKey ] |= bbBestMove;   

         }

         // Don't worry about promotions at this point, but consider it for the future.


         // holding on inputting a danger rating.

         // Input the type of score.
         // Holding on score type.

         // Enter the age.  By this meaning at what ply into the game was this observed.
         BitBoard bbAge = 0;
         if ( argsBoard->iMoveHistory > 0 )
         {
         
            bbAge = argsBoard->iMoveHistory;

         }
         bbAge = bbAge << argsGeneralMoves->iAgeShift;
         gsHashTable.mbbHashTable[ bbKey ] |= bbAge;
     
      } // End if over depth
  
   } // End pragma

}

//
//-------------------------------------------------------------------------------------------------
//
void AssignRandomKeys()
{

// This function is used to create the random hash keys that will be persistant.
//

   int iKeyIndex = 0;

   // Set up the hash keys.
   int iSquareIndex = 0;
   for ( iSquareIndex = 0; iSquareIndex < 64; iSquareIndex++ )
   {

      for( int iPieceIndex = 0; iPieceIndex < 12; iPieceIndex++ )
      {

        gsHashTable.mbbHashKeys[ iPieceIndex ][ iSquareIndex ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];

      }

      gsHashTable.vbbEnPassant[ iSquareIndex ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];;

   }

   for ( int iStateIndex = 0; iStateIndex < 6; iStateIndex++ )
   {

      gsHashTable.vbbHashKeysStates[ iStateIndex ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];;

   }

   gsHashTable.vbbCasteling[ 0 ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];
   gsHashTable.vbbCasteling[ 1 ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];
   gsHashTable.vbbCasteling[ 2 ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];
   gsHashTable.vbbCasteling[ 3 ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];

   gsHashTable.vbbColorToMove[ 0 ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];
   gsHashTable.vbbColorToMove[ 1 ] = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];


   // Set up the initial hash.
   gsHashTable.bbHash = gsHashTable.vbbRandomKeys[ iKeyIndex++ ];;

   // Keep an initial hash for reference
   gsHashTable.bbHashInitial = gsHashTable.bbHash;

}

//
//-------------------------------------------------------------------------------------------------
//
void ReadRandomKeyFile()
{

// This function is used to create the random hash keys that will be persistant.
//

   // Read the opening book.
   ifstream ifKeys( "RandomKeys.txt" );
   if ( ifKeys.fail() )
   {
   
      cout << "Input RandomKeys.txt file failed to open." << endl;
      
   }

   // Set up the hash keys.
   int iSquareIndex = 0;
   for ( iSquareIndex = 0; iSquareIndex < 64; iSquareIndex++ )
   {

      for( int iPieceIndex = 0; iPieceIndex < 12; iPieceIndex++ )
      {

        ifKeys >> gsHashTable.mbbHashKeys[ iPieceIndex ][ iSquareIndex ];

      }

      ifKeys >> gsHashTable.vbbEnPassant[ iSquareIndex ];

   }

   for ( int iStateIndex = 0; iStateIndex < 6; iStateIndex++ )
   {

      ifKeys >> gsHashTable.vbbHashKeysStates[ iStateIndex ];

   }

   ifKeys >> gsHashTable.vbbCasteling[ 0 ];
   ifKeys >> gsHashTable.vbbCasteling[ 1 ];
   ifKeys >> gsHashTable.vbbCasteling[ 2 ];
   ifKeys >> gsHashTable.vbbCasteling[ 3 ];

   ifKeys >> gsHashTable.vbbColorToMove[ 0 ];
   ifKeys >> gsHashTable.vbbColorToMove[ 1 ];


   // Set up the initial hash.
   ifKeys >> gsHashTable.bbHash;

   // Keep an initial hash for reference
   gsHashTable.bbHashInitial = gsHashTable.bbHash;

   // Close the file.
   ifKeys.close();

}

//
//------------------------------------------------------------------------------------------
//
void DestroyHashTable()
{
// This function releases the memory taken by the hash table.

   free( gsHashTable.mbbHash );
   free( gsHashTable.mbbHashTable );

}

// Define some hash sets.
void SetHash( BitBoard bbHash )
{
   gsHashTable.bbHash = bbHash;
}
void SetHashElement( BitBoard bbKey,
                     BitBoard bbElement )
{
   gsHashTable.mbbHashTable[ bbKey ] = bbElement;
} 

// Define some input and some output variables.
BitBoard GetHash()
{
   return gsHashTable.bbHash;
}
BitBoard GetHashInitial()
{
   return gsHashTable.bbHashInitial;
}
BitBoard GetHashElement( BitBoard bbKey )
{
   return gsHashTable.mbbHashTable[ bbKey ];
} 
int GetQueryState()
{
   return gsHashTable.iQueryState;
}
BitBoard GetDepth()
{
   return gsHashTable.bbDepth;
}
int GetScoreHash()
{
   return gsHashTable.iScore;
}
BitBoard GetBestMove()
{
   return gsHashTable.bbBestMove;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Test Suites are below.




//
//----------------------------------------------------------------------------------------------
//
// I was storing the persistant random keys in a text file.  This converts it to code for better
// "permanant" storage.
void ConvertRandomKeyFileToCode()
{

   ifstream ifKeyFile( "RandomKeys.txt" );
   ofstream ofKeyCode( "RandomKeyCode.txt" );

   if ( ifKeyFile.fail() )
   {
     
      cout << "Input file failed to open." << endl;
      system( "pause" );

   }
   if ( ofKeyCode.fail() )
   {

      cout << "Output file failed to open." << endl;
      system( "pause" );

   }

   int iCounter = -1;
   int iFlag = 1;
   BitBoard bbKey = 0;
   BitBoard bbKeyOld = 0;
   while( iFlag )
   {

      // Get the key and increment the counter.
      ifKeyFile >> bbKey;
      if ( bbKey == bbKeyOld )
      {

         iFlag = 0;
         break;

      }

      // Keep the old key.
      bbKeyOld = bbKey;

      // Increment the counter
      iCounter++;

      // Let the folks at home see the count.
      cout << "iCounter = " << iCounter << " Key = " << bbKey << endl;

      // Write the code.
      ofKeyCode << "gsHashTable.vbbRandomKeys[ " << iCounter << " ] = " << bbKey << ";" << endl;
      

   }

   ifKeyFile.close();
   ofKeyCode.close();

}


//
//--------------------------------------------------------------------------------------
//
void GetAllKeys()
{

   // Here are the permanant keys.
   gsHashTable.vbbRandomKeys[ 0 ] = 4935135370079648666;
   gsHashTable.vbbRandomKeys[ 1 ] = 8760541055110964466;
   gsHashTable.vbbRandomKeys[ 2 ] = 1373468545248413492;
   gsHashTable.vbbRandomKeys[ 3 ] = 17424003862454590535;
   gsHashTable.vbbRandomKeys[ 4 ] = 7046370612499155837;
   gsHashTable.vbbRandomKeys[ 5 ] = 5922200747593917863;
   gsHashTable.vbbRandomKeys[ 6 ] = 553153244974754788;
   gsHashTable.vbbRandomKeys[ 7 ] = 17152115956460010486;
   gsHashTable.vbbRandomKeys[ 8 ] = 8813278035371429741;
   gsHashTable.vbbRandomKeys[ 9 ] = 13610028684416652972;
   gsHashTable.vbbRandomKeys[ 10 ] = 7320583005235833889;
   gsHashTable.vbbRandomKeys[ 11 ] = 18428241321350849386;
   gsHashTable.vbbRandomKeys[ 12 ] = 12501768878060745096;
   gsHashTable.vbbRandomKeys[ 13 ] = 2301449675581005687;
   gsHashTable.vbbRandomKeys[ 14 ] = 7959306954535313715;
   gsHashTable.vbbRandomKeys[ 15 ] = 17311297250047461341;
   gsHashTable.vbbRandomKeys[ 16 ] = 7647947029509561407;
   gsHashTable.vbbRandomKeys[ 17 ] = 6245404973388758094;
   gsHashTable.vbbRandomKeys[ 18 ] = 6901276093037625841;
   gsHashTable.vbbRandomKeys[ 19 ] = 7912728514252366347;
   gsHashTable.vbbRandomKeys[ 20 ] = 1242976831634006302;
   gsHashTable.vbbRandomKeys[ 21 ] = 6424960976185133159;
   gsHashTable.vbbRandomKeys[ 22 ] = 12233926170583448611;
   gsHashTable.vbbRandomKeys[ 23 ] = 9464641311136192561;
   gsHashTable.vbbRandomKeys[ 24 ] = 10995414420296893337;
   gsHashTable.vbbRandomKeys[ 25 ] = 3583907980795662100;
   gsHashTable.vbbRandomKeys[ 26 ] = 12949840557515789192;
   gsHashTable.vbbRandomKeys[ 27 ] = 7247304256905388901;
   gsHashTable.vbbRandomKeys[ 28 ] = 2350353823024099498;
   gsHashTable.vbbRandomKeys[ 29 ] = 13056814523014245207;
   gsHashTable.vbbRandomKeys[ 30 ] = 18094338687727304282;
   gsHashTable.vbbRandomKeys[ 31 ] = 3528422694172788572;
   gsHashTable.vbbRandomKeys[ 32 ] = 11479469170138869657;
   gsHashTable.vbbRandomKeys[ 33 ] = 9870638376459579589;
   gsHashTable.vbbRandomKeys[ 34 ] = 9726242749197214417;
   gsHashTable.vbbRandomKeys[ 35 ] = 17575098152022733246;
   gsHashTable.vbbRandomKeys[ 36 ] = 18207978509386268757;
   gsHashTable.vbbRandomKeys[ 37 ] = 12935208460127086199;
   gsHashTable.vbbRandomKeys[ 38 ] = 3822731913716169222;
   gsHashTable.vbbRandomKeys[ 39 ] = 50957834674229182;
   gsHashTable.vbbRandomKeys[ 40 ] = 14745584112208288187;
   gsHashTable.vbbRandomKeys[ 41 ] = 9150751690725558562;
   gsHashTable.vbbRandomKeys[ 42 ] = 10729842981047422978;
   gsHashTable.vbbRandomKeys[ 43 ] = 16389271633590366922;
   gsHashTable.vbbRandomKeys[ 44 ] = 167595253981702758;
   gsHashTable.vbbRandomKeys[ 45 ] = 6198823806034427955;
   gsHashTable.vbbRandomKeys[ 46 ] = 1860974486140708579;
   gsHashTable.vbbRandomKeys[ 47 ] = 16033988530684246712;
   gsHashTable.vbbRandomKeys[ 48 ] = 13701360435820042510;
   gsHashTable.vbbRandomKeys[ 49 ] = 17143430502818090469;
   gsHashTable.vbbRandomKeys[ 50 ] = 18237011452591495607;
   gsHashTable.vbbRandomKeys[ 51 ] = 3111763172835065622;
   gsHashTable.vbbRandomKeys[ 52 ] = 12795754322230409885;
   gsHashTable.vbbRandomKeys[ 53 ] = 10079630097836378147;
   gsHashTable.vbbRandomKeys[ 54 ] = 13701139478742659113;
   gsHashTable.vbbRandomKeys[ 55 ] = 8192683469179632153;
   gsHashTable.vbbRandomKeys[ 56 ] = 71127209098305206;
   gsHashTable.vbbRandomKeys[ 57 ] = 14716305965773944179;
   gsHashTable.vbbRandomKeys[ 58 ] = 3498877393685310710;
   gsHashTable.vbbRandomKeys[ 59 ] = 13786632312116635800;
   gsHashTable.vbbRandomKeys[ 60 ] = 14491929101080280037;
   gsHashTable.vbbRandomKeys[ 61 ] = 13622923582448834427;
   gsHashTable.vbbRandomKeys[ 62 ] = 3421413570544993410;
   gsHashTable.vbbRandomKeys[ 63 ] = 1975174938213371089;
   gsHashTable.vbbRandomKeys[ 64 ] = 15631236566978425604;
   gsHashTable.vbbRandomKeys[ 65 ] = 11269118174198131671;
   gsHashTable.vbbRandomKeys[ 66 ] = 5649831898311974713;
   gsHashTable.vbbRandomKeys[ 67 ] = 4221767994904442824;
   gsHashTable.vbbRandomKeys[ 68 ] = 8783724825689332167;
   gsHashTable.vbbRandomKeys[ 69 ] = 115441443530029035;
   gsHashTable.vbbRandomKeys[ 70 ] = 17981259982952871599;
   gsHashTable.vbbRandomKeys[ 71 ] = 5986860220222532696;
   gsHashTable.vbbRandomKeys[ 72 ] = 16699741404889962921;
   gsHashTable.vbbRandomKeys[ 73 ] = 14263378855515925081;
   gsHashTable.vbbRandomKeys[ 74 ] = 10911432543843778548;
   gsHashTable.vbbRandomKeys[ 75 ] = 13385088849281539019;
   gsHashTable.vbbRandomKeys[ 76 ] = 15148358834241027995;
   gsHashTable.vbbRandomKeys[ 77 ] = 15958591445201563278;
   gsHashTable.vbbRandomKeys[ 78 ] = 1835134974859575952;
   gsHashTable.vbbRandomKeys[ 79 ] = 7961956497720207380;
   gsHashTable.vbbRandomKeys[ 80 ] = 12885358504040285515;
   gsHashTable.vbbRandomKeys[ 81 ] = 14777842447256309910;
   gsHashTable.vbbRandomKeys[ 82 ] = 6287924485600403893;
   gsHashTable.vbbRandomKeys[ 83 ] = 5560211958396438137;
   gsHashTable.vbbRandomKeys[ 84 ] = 18426050403864099492;
   gsHashTable.vbbRandomKeys[ 85 ] = 17739561510201804606;
   gsHashTable.vbbRandomKeys[ 86 ] = 8143248515724258973;
   gsHashTable.vbbRandomKeys[ 87 ] = 976845348288513367;
   gsHashTable.vbbRandomKeys[ 88 ] = 2662748885775523881;
   gsHashTable.vbbRandomKeys[ 89 ] = 2709088665022941349;
   gsHashTable.vbbRandomKeys[ 90 ] = 10684462989696293633;
   gsHashTable.vbbRandomKeys[ 91 ] = 7199042308857076158;
   gsHashTable.vbbRandomKeys[ 92 ] = 6582027593128207596;
   gsHashTable.vbbRandomKeys[ 93 ] = 8189636606085609321;
   gsHashTable.vbbRandomKeys[ 94 ] = 13892710499392706057;
   gsHashTable.vbbRandomKeys[ 95 ] = 6658428705083593830;
   gsHashTable.vbbRandomKeys[ 96 ] = 16219376670579898698;
   gsHashTable.vbbRandomKeys[ 97 ] = 12949376600498868586;
   gsHashTable.vbbRandomKeys[ 98 ] = 17284857596287908000;
   gsHashTable.vbbRandomKeys[ 99 ] = 8781347084849184461;
   gsHashTable.vbbRandomKeys[ 100 ] = 11685849876061585859;
   gsHashTable.vbbRandomKeys[ 101 ] = 14014166370621317114;
   gsHashTable.vbbRandomKeys[ 102 ] = 1853551609357334224;
   gsHashTable.vbbRandomKeys[ 103 ] = 2464656721224973028;
   gsHashTable.vbbRandomKeys[ 104 ] = 15862732107410517397;
   gsHashTable.vbbRandomKeys[ 105 ] = 14634335726527321380;
   gsHashTable.vbbRandomKeys[ 106 ] = 1386847398902725516;
   gsHashTable.vbbRandomKeys[ 107 ] = 5257757431874371424;
   gsHashTable.vbbRandomKeys[ 108 ] = 283053926336771110;
   gsHashTable.vbbRandomKeys[ 109 ] = 12435510189066653500;
   gsHashTable.vbbRandomKeys[ 110 ] = 5946244702078260643;
   gsHashTable.vbbRandomKeys[ 111 ] = 16589105116111004170;
   gsHashTable.vbbRandomKeys[ 112 ] = 16184132748602693483;
   gsHashTable.vbbRandomKeys[ 113 ] = 12623532889688181458;
   gsHashTable.vbbRandomKeys[ 114 ] = 17352043466912814336;
   gsHashTable.vbbRandomKeys[ 115 ] = 1688525684216809943;
   gsHashTable.vbbRandomKeys[ 116 ] = 3024352167265828814;
   gsHashTable.vbbRandomKeys[ 117 ] = 3791468294708056053;
   gsHashTable.vbbRandomKeys[ 118 ] = 17757703511814922732;
   gsHashTable.vbbRandomKeys[ 119 ] = 11280700411732542229;
   gsHashTable.vbbRandomKeys[ 120 ] = 11322008415086027197;
   gsHashTable.vbbRandomKeys[ 121 ] = 14886180053689320936;
   gsHashTable.vbbRandomKeys[ 122 ] = 1890121798009321925;
   gsHashTable.vbbRandomKeys[ 123 ] = 151366907903016629;
   gsHashTable.vbbRandomKeys[ 124 ] = 11820963556777049053;
   gsHashTable.vbbRandomKeys[ 125 ] = 465967135617106496;
   gsHashTable.vbbRandomKeys[ 126 ] = 1173955523072332698;
   gsHashTable.vbbRandomKeys[ 127 ] = 5916079091746291288;
   gsHashTable.vbbRandomKeys[ 128 ] = 8653772674023417462;
   gsHashTable.vbbRandomKeys[ 129 ] = 12748387629534233511;
   gsHashTable.vbbRandomKeys[ 130 ] = 12252994478093091835;
   gsHashTable.vbbRandomKeys[ 131 ] = 12461857806472829458;
   gsHashTable.vbbRandomKeys[ 132 ] = 1709199067063832321;
   gsHashTable.vbbRandomKeys[ 133 ] = 6668344926470687540;
   gsHashTable.vbbRandomKeys[ 134 ] = 15815600854590138520;
   gsHashTable.vbbRandomKeys[ 135 ] = 12695502796830378126;
   gsHashTable.vbbRandomKeys[ 136 ] = 12153540460622051322;
   gsHashTable.vbbRandomKeys[ 137 ] = 7175423485251423620;
   gsHashTable.vbbRandomKeys[ 138 ] = 1897136785990571731;
   gsHashTable.vbbRandomKeys[ 139 ] = 463407426052211390;
   gsHashTable.vbbRandomKeys[ 140 ] = 6894668742045483267;
   gsHashTable.vbbRandomKeys[ 141 ] = 16694474718386414950;
   gsHashTable.vbbRandomKeys[ 142 ] = 5176599940548795448;
   gsHashTable.vbbRandomKeys[ 143 ] = 8044525157766773404;
   gsHashTable.vbbRandomKeys[ 144 ] = 15769634638604376500;
   gsHashTable.vbbRandomKeys[ 145 ] = 17131441059169170643;
   gsHashTable.vbbRandomKeys[ 146 ] = 15883127658452287307;
   gsHashTable.vbbRandomKeys[ 147 ] = 11749621619917248544;
   gsHashTable.vbbRandomKeys[ 148 ] = 7504472520185788887;
   gsHashTable.vbbRandomKeys[ 149 ] = 6153965901076564017;
   gsHashTable.vbbRandomKeys[ 150 ] = 928445363607578965;
   gsHashTable.vbbRandomKeys[ 151 ] = 14563452617670965426;
   gsHashTable.vbbRandomKeys[ 152 ] = 16709359376109501421;
   gsHashTable.vbbRandomKeys[ 153 ] = 1618228431386014929;
   gsHashTable.vbbRandomKeys[ 154 ] = 5608701459468607089;
   gsHashTable.vbbRandomKeys[ 155 ] = 759281347879418618;
   gsHashTable.vbbRandomKeys[ 156 ] = 12756160760667167844;
   gsHashTable.vbbRandomKeys[ 157 ] = 14714245548412428991;
   gsHashTable.vbbRandomKeys[ 158 ] = 13427732340878524584;
   gsHashTable.vbbRandomKeys[ 159 ] = 11032144747053841450;
   gsHashTable.vbbRandomKeys[ 160 ] = 2154534722210676336;
   gsHashTable.vbbRandomKeys[ 161 ] = 5122340863223504183;
   gsHashTable.vbbRandomKeys[ 162 ] = 17400864600770803864;
   gsHashTable.vbbRandomKeys[ 163 ] = 13206475456165470062;
   gsHashTable.vbbRandomKeys[ 164 ] = 4717918801002464053;
   gsHashTable.vbbRandomKeys[ 165 ] = 2057546224846249930;
   gsHashTable.vbbRandomKeys[ 166 ] = 16415205182510692527;
   gsHashTable.vbbRandomKeys[ 167 ] = 387281517301493335;
   gsHashTable.vbbRandomKeys[ 168 ] = 9770797717351808290;
   gsHashTable.vbbRandomKeys[ 169 ] = 4330782717792696916;
   gsHashTable.vbbRandomKeys[ 170 ] = 17190510560590230790;
   gsHashTable.vbbRandomKeys[ 171 ] = 14963743919639659770;
   gsHashTable.vbbRandomKeys[ 172 ] = 5563952063838493762;
   gsHashTable.vbbRandomKeys[ 173 ] = 12010887833691968949;
   gsHashTable.vbbRandomKeys[ 174 ] = 2292988140517973496;
   gsHashTable.vbbRandomKeys[ 175 ] = 17655904365783758241;
   gsHashTable.vbbRandomKeys[ 176 ] = 3489468710120115707;
   gsHashTable.vbbRandomKeys[ 177 ] = 4430640583843063438;
   gsHashTable.vbbRandomKeys[ 178 ] = 2916893870121568004;
   gsHashTable.vbbRandomKeys[ 179 ] = 18405371841364968435;
   gsHashTable.vbbRandomKeys[ 180 ] = 15591421984591757264;
   gsHashTable.vbbRandomKeys[ 181 ] = 1883594594455465681;
   gsHashTable.vbbRandomKeys[ 182 ] = 9066226989811406923;
   gsHashTable.vbbRandomKeys[ 183 ] = 5542731965854158349;
   gsHashTable.vbbRandomKeys[ 184 ] = 3073770483931687892;
   gsHashTable.vbbRandomKeys[ 185 ] = 1842248390712116653;
   gsHashTable.vbbRandomKeys[ 186 ] = 15382259679143197757;
   gsHashTable.vbbRandomKeys[ 187 ] = 17160178335322656369;
   gsHashTable.vbbRandomKeys[ 188 ] = 6212373600052167078;
   gsHashTable.vbbRandomKeys[ 189 ] = 14581671181268223261;
   gsHashTable.vbbRandomKeys[ 190 ] = 12658485879843455472;
   gsHashTable.vbbRandomKeys[ 191 ] = 11858908553690145779;
   gsHashTable.vbbRandomKeys[ 192 ] = 13846527487904627475;
   gsHashTable.vbbRandomKeys[ 193 ] = 17730873393267436611;
   gsHashTable.vbbRandomKeys[ 194 ] = 3340900340542698407;
   gsHashTable.vbbRandomKeys[ 195 ] = 16287619699920330388;
   gsHashTable.vbbRandomKeys[ 196 ] = 13981268605706299935;
   gsHashTable.vbbRandomKeys[ 197 ] = 9590035561708822464;
   gsHashTable.vbbRandomKeys[ 198 ] = 13661930529097191454;
   gsHashTable.vbbRandomKeys[ 199 ] = 9288643624708755173;
   gsHashTable.vbbRandomKeys[ 200 ] = 8911775425816798898;
   gsHashTable.vbbRandomKeys[ 201 ] = 1200599218775333085;
   gsHashTable.vbbRandomKeys[ 202 ] = 3563294164938379510;
   gsHashTable.vbbRandomKeys[ 203 ] = 11302611871828470153;
   gsHashTable.vbbRandomKeys[ 204 ] = 14547525872618001768;
   gsHashTable.vbbRandomKeys[ 205 ] = 13686249026255986070;
   gsHashTable.vbbRandomKeys[ 206 ] = 9256186612199032643;
   gsHashTable.vbbRandomKeys[ 207 ] = 946525180231834586;
   gsHashTable.vbbRandomKeys[ 208 ] = 10112394073579361573;
   gsHashTable.vbbRandomKeys[ 209 ] = 7570316668447289715;
   gsHashTable.vbbRandomKeys[ 210 ] = 15033307218694241963;
   gsHashTable.vbbRandomKeys[ 211 ] = 1116132907257488754;
   gsHashTable.vbbRandomKeys[ 212 ] = 6443806979863662448;
   gsHashTable.vbbRandomKeys[ 213 ] = 7129975447970775031;
   gsHashTable.vbbRandomKeys[ 214 ] = 48751080626640451;
   gsHashTable.vbbRandomKeys[ 215 ] = 10238184278289966472;
   gsHashTable.vbbRandomKeys[ 216 ] = 15075081649572177884;
   gsHashTable.vbbRandomKeys[ 217 ] = 15558937251010864845;
   gsHashTable.vbbRandomKeys[ 218 ] = 11967937246072603505;
   gsHashTable.vbbRandomKeys[ 219 ] = 15516838899388883080;
   gsHashTable.vbbRandomKeys[ 220 ] = 15513139612648968612;
   gsHashTable.vbbRandomKeys[ 221 ] = 2222896532179260816;
   gsHashTable.vbbRandomKeys[ 222 ] = 8489483746095221736;
   gsHashTable.vbbRandomKeys[ 223 ] = 4330299715914659173;
   gsHashTable.vbbRandomKeys[ 224 ] = 11472696857521874412;
   gsHashTable.vbbRandomKeys[ 225 ] = 10312284560818484338;
   gsHashTable.vbbRandomKeys[ 226 ] = 1867815146558769700;
   gsHashTable.vbbRandomKeys[ 227 ] = 600249699052379429;
   gsHashTable.vbbRandomKeys[ 228 ] = 14040612342337482318;
   gsHashTable.vbbRandomKeys[ 229 ] = 16274543711566352996;
   gsHashTable.vbbRandomKeys[ 230 ] = 1895785172263718604;
   gsHashTable.vbbRandomKeys[ 231 ] = 16661291268273584085;
   gsHashTable.vbbRandomKeys[ 232 ] = 5148874725156059137;
   gsHashTable.vbbRandomKeys[ 233 ] = 6582123155087367143;
   gsHashTable.vbbRandomKeys[ 234 ] = 15807340424866308897;
   gsHashTable.vbbRandomKeys[ 235 ] = 17903031853855775229;
   gsHashTable.vbbRandomKeys[ 236 ] = 1085916237822101987;
   gsHashTable.vbbRandomKeys[ 237 ] = 2126800388291214182;
   gsHashTable.vbbRandomKeys[ 238 ] = 8442697267907828706;
   gsHashTable.vbbRandomKeys[ 239 ] = 13453965647616689582;
   gsHashTable.vbbRandomKeys[ 240 ] = 17582224846569265504;
   gsHashTable.vbbRandomKeys[ 241 ] = 4279028351987032376;
   gsHashTable.vbbRandomKeys[ 242 ] = 4425400614262658792;
   gsHashTable.vbbRandomKeys[ 243 ] = 4803395348995701790;
   gsHashTable.vbbRandomKeys[ 244 ] = 6608131653019013916;
   gsHashTable.vbbRandomKeys[ 245 ] = 2438431347235665323;
   gsHashTable.vbbRandomKeys[ 246 ] = 4051885490660939060;
   gsHashTable.vbbRandomKeys[ 247 ] = 436438476925012381;
   gsHashTable.vbbRandomKeys[ 248 ] = 6460020123108651076;
   gsHashTable.vbbRandomKeys[ 249 ] = 11173625625074531694;
   gsHashTable.vbbRandomKeys[ 250 ] = 2185348500754117524;
   gsHashTable.vbbRandomKeys[ 251 ] = 8480723964983527620;
   gsHashTable.vbbRandomKeys[ 252 ] = 2457303879165271190;
   gsHashTable.vbbRandomKeys[ 253 ] = 13104600897885889221;
   gsHashTable.vbbRandomKeys[ 254 ] = 4173045397401310305;
   gsHashTable.vbbRandomKeys[ 255 ] = 11365133834883247761;
   gsHashTable.vbbRandomKeys[ 256 ] = 63804126074410096;
   gsHashTable.vbbRandomKeys[ 257 ] = 11161906508689137836;
   gsHashTable.vbbRandomKeys[ 258 ] = 15016599928566609653;
   gsHashTable.vbbRandomKeys[ 259 ] = 16269029863924382692;
   gsHashTable.vbbRandomKeys[ 260 ] = 8162753980902932322;
   gsHashTable.vbbRandomKeys[ 261 ] = 1849333294556510852;
   gsHashTable.vbbRandomKeys[ 262 ] = 7252153461718139666;
   gsHashTable.vbbRandomKeys[ 263 ] = 3936383861584655204;
   gsHashTable.vbbRandomKeys[ 264 ] = 11054933813022310257;
   gsHashTable.vbbRandomKeys[ 265 ] = 151807195329410555;
   gsHashTable.vbbRandomKeys[ 266 ] = 2370590319620426760;
   gsHashTable.vbbRandomKeys[ 267 ] = 14699287662653919329;
   gsHashTable.vbbRandomKeys[ 268 ] = 6723716622327502200;
   gsHashTable.vbbRandomKeys[ 269 ] = 2212040317801727931;
   gsHashTable.vbbRandomKeys[ 270 ] = 9114942237622257130;
   gsHashTable.vbbRandomKeys[ 271 ] = 10408762504841682934;
   gsHashTable.vbbRandomKeys[ 272 ] = 6905119390488045507;
   gsHashTable.vbbRandomKeys[ 273 ] = 8209185801436321534;
   gsHashTable.vbbRandomKeys[ 274 ] = 10613424755411090100;
   gsHashTable.vbbRandomKeys[ 275 ] = 17172956176212519599;
   gsHashTable.vbbRandomKeys[ 276 ] = 1037664165871214517;
   gsHashTable.vbbRandomKeys[ 277 ] = 15620908131366839787;
   gsHashTable.vbbRandomKeys[ 278 ] = 10804425254167825044;
   gsHashTable.vbbRandomKeys[ 279 ] = 15438184041890710517;
   gsHashTable.vbbRandomKeys[ 280 ] = 16640332963902778823;
   gsHashTable.vbbRandomKeys[ 281 ] = 5230787705124406241;
   gsHashTable.vbbRandomKeys[ 282 ] = 8622332490324061299;
   gsHashTable.vbbRandomKeys[ 283 ] = 8356086214475649940;
   gsHashTable.vbbRandomKeys[ 284 ] = 12046796118178695396;
   gsHashTable.vbbRandomKeys[ 285 ] = 8180403113820970145;
   gsHashTable.vbbRandomKeys[ 286 ] = 15402660407473215632;
   gsHashTable.vbbRandomKeys[ 287 ] = 7018207125954560227;
   gsHashTable.vbbRandomKeys[ 288 ] = 11683389668466580844;
   gsHashTable.vbbRandomKeys[ 289 ] = 18087717269769926850;
   gsHashTable.vbbRandomKeys[ 290 ] = 13760796120271330061;
   gsHashTable.vbbRandomKeys[ 291 ] = 5400261231525542676;
   gsHashTable.vbbRandomKeys[ 292 ] = 3563156082204254616;
   gsHashTable.vbbRandomKeys[ 293 ] = 8498995909576272292;
   gsHashTable.vbbRandomKeys[ 294 ] = 10202104521817394451;
   gsHashTable.vbbRandomKeys[ 295 ] = 9909852847406304237;
   gsHashTable.vbbRandomKeys[ 296 ] = 10769805475914925418;
   gsHashTable.vbbRandomKeys[ 297 ] = 14990567919135282638;
   gsHashTable.vbbRandomKeys[ 298 ] = 5333032281237107047;
   gsHashTable.vbbRandomKeys[ 299 ] = 16769638380885260543;
   gsHashTable.vbbRandomKeys[ 300 ] = 139768483873232302;
   gsHashTable.vbbRandomKeys[ 301 ] = 11256621163219843430;
   gsHashTable.vbbRandomKeys[ 302 ] = 3583932678020305713;
   gsHashTable.vbbRandomKeys[ 303 ] = 1826468160408788736;
   gsHashTable.vbbRandomKeys[ 304 ] = 5847292296893332391;
   gsHashTable.vbbRandomKeys[ 305 ] = 1577832050664049892;
   gsHashTable.vbbRandomKeys[ 306 ] = 18001897581465246260;
   gsHashTable.vbbRandomKeys[ 307 ] = 3749208007554556854;
   gsHashTable.vbbRandomKeys[ 308 ] = 4041834233113872302;
   gsHashTable.vbbRandomKeys[ 309 ] = 15495920083068704086;
   gsHashTable.vbbRandomKeys[ 310 ] = 4299826711031831966;
   gsHashTable.vbbRandomKeys[ 311 ] = 14275445904488267730;
   gsHashTable.vbbRandomKeys[ 312 ] = 12619069218030256672;
   gsHashTable.vbbRandomKeys[ 313 ] = 6123482047278803083;
   gsHashTable.vbbRandomKeys[ 314 ] = 7831896195366358724;
   gsHashTable.vbbRandomKeys[ 315 ] = 9355867766055400602;
   gsHashTable.vbbRandomKeys[ 316 ] = 6360457979159859757;
   gsHashTable.vbbRandomKeys[ 317 ] = 13845980212380040629;
   gsHashTable.vbbRandomKeys[ 318 ] = 6741183028744970162;
   gsHashTable.vbbRandomKeys[ 319 ] = 13920484680734084087;
   gsHashTable.vbbRandomKeys[ 320 ] = 2978948904727153209;
   gsHashTable.vbbRandomKeys[ 321 ] = 12926677860505600659;
   gsHashTable.vbbRandomKeys[ 322 ] = 8304497923989126972;
   gsHashTable.vbbRandomKeys[ 323 ] = 7127763959372691259;
   gsHashTable.vbbRandomKeys[ 324 ] = 7228913571138267757;
   gsHashTable.vbbRandomKeys[ 325 ] = 5440300756491697597;
   gsHashTable.vbbRandomKeys[ 326 ] = 10643663359368845678;
   gsHashTable.vbbRandomKeys[ 327 ] = 2287225428107476467;
   gsHashTable.vbbRandomKeys[ 328 ] = 6049777262784860281;
   gsHashTable.vbbRandomKeys[ 329 ] = 12334542464283704697;
   gsHashTable.vbbRandomKeys[ 330 ] = 10403174850142488320;
   gsHashTable.vbbRandomKeys[ 331 ] = 5607608845535956315;
   gsHashTable.vbbRandomKeys[ 332 ] = 15264547612286183582;
   gsHashTable.vbbRandomKeys[ 333 ] = 841816041152082174;
   gsHashTable.vbbRandomKeys[ 334 ] = 7391815064494572602;
   gsHashTable.vbbRandomKeys[ 335 ] = 18367820465571178166;
   gsHashTable.vbbRandomKeys[ 336 ] = 15006891588643797097;
   gsHashTable.vbbRandomKeys[ 337 ] = 6822335996487016210;
   gsHashTable.vbbRandomKeys[ 338 ] = 17801208619149606569;
   gsHashTable.vbbRandomKeys[ 339 ] = 1995376138012955932;
   gsHashTable.vbbRandomKeys[ 340 ] = 16407241925450618970;
   gsHashTable.vbbRandomKeys[ 341 ] = 4311128145234531303;
   gsHashTable.vbbRandomKeys[ 342 ] = 146135625377058522;
   gsHashTable.vbbRandomKeys[ 343 ] = 2232778168035379839;
   gsHashTable.vbbRandomKeys[ 344 ] = 6825540131995277676;
   gsHashTable.vbbRandomKeys[ 345 ] = 480716391780806;
   gsHashTable.vbbRandomKeys[ 346 ] = 12534200477889622857;
   gsHashTable.vbbRandomKeys[ 347 ] = 387792468727484856;
   gsHashTable.vbbRandomKeys[ 348 ] = 14519474273395506879;
   gsHashTable.vbbRandomKeys[ 349 ] = 12931318539001361278;
   gsHashTable.vbbRandomKeys[ 350 ] = 9318211821033394160;
   gsHashTable.vbbRandomKeys[ 351 ] = 15752028604028896253;
   gsHashTable.vbbRandomKeys[ 352 ] = 14345729832159189576;
   gsHashTable.vbbRandomKeys[ 353 ] = 16500706173742864724;
   gsHashTable.vbbRandomKeys[ 354 ] = 15178895521184224361;
   gsHashTable.vbbRandomKeys[ 355 ] = 6573370076250518824;
   gsHashTable.vbbRandomKeys[ 356 ] = 12157161219685457186;
   gsHashTable.vbbRandomKeys[ 357 ] = 16237073210848684183;
   gsHashTable.vbbRandomKeys[ 358 ] = 3916536503640903980;
   gsHashTable.vbbRandomKeys[ 359 ] = 11968579260449086672;
   gsHashTable.vbbRandomKeys[ 360 ] = 16826350101892209356;
   gsHashTable.vbbRandomKeys[ 361 ] = 8708524667094192573;
   gsHashTable.vbbRandomKeys[ 362 ] = 4080300613527203560;
   gsHashTable.vbbRandomKeys[ 363 ] = 3187595835993090121;
   gsHashTable.vbbRandomKeys[ 364 ] = 9983166862425865875;
   gsHashTable.vbbRandomKeys[ 365 ] = 13821589933414074524;
   gsHashTable.vbbRandomKeys[ 366 ] = 1117302542172311464;
   gsHashTable.vbbRandomKeys[ 367 ] = 8007288322808096406;
   gsHashTable.vbbRandomKeys[ 368 ] = 11440349626545726072;
   gsHashTable.vbbRandomKeys[ 369 ] = 6801486488047538494;
   gsHashTable.vbbRandomKeys[ 370 ] = 12132389990626565662;
   gsHashTable.vbbRandomKeys[ 371 ] = 8146843448520574479;
   gsHashTable.vbbRandomKeys[ 372 ] = 7360417401812417961;
   gsHashTable.vbbRandomKeys[ 373 ] = 4519962351738970045;
   gsHashTable.vbbRandomKeys[ 374 ] = 13583528725134400826;
   gsHashTable.vbbRandomKeys[ 375 ] = 1690852946580328919;
   gsHashTable.vbbRandomKeys[ 376 ] = 4011536040352710982;
   gsHashTable.vbbRandomKeys[ 377 ] = 2932754063635089095;
   gsHashTable.vbbRandomKeys[ 378 ] = 8523267475921324151;
   gsHashTable.vbbRandomKeys[ 379 ] = 12214262480892667683;
   gsHashTable.vbbRandomKeys[ 380 ] = 16391108672921117265;
   gsHashTable.vbbRandomKeys[ 381 ] = 1109729969817899442;
   gsHashTable.vbbRandomKeys[ 382 ] = 6840216412034469787;
   gsHashTable.vbbRandomKeys[ 383 ] = 6678154308523457823;
   gsHashTable.vbbRandomKeys[ 384 ] = 15462195022270978258;
   gsHashTable.vbbRandomKeys[ 385 ] = 7932211405693026916;
   gsHashTable.vbbRandomKeys[ 386 ] = 5521111321402668195;
   gsHashTable.vbbRandomKeys[ 387 ] = 6273060676892249842;
   gsHashTable.vbbRandomKeys[ 388 ] = 5379077922461368958;
   gsHashTable.vbbRandomKeys[ 389 ] = 5961179897323766817;
   gsHashTable.vbbRandomKeys[ 390 ] = 16938785341374763767;
   gsHashTable.vbbRandomKeys[ 391 ] = 16823897401208150167;
   gsHashTable.vbbRandomKeys[ 392 ] = 11207800658017012220;
   gsHashTable.vbbRandomKeys[ 393 ] = 1983641547682912676;
   gsHashTable.vbbRandomKeys[ 394 ] = 16732851709703935604;
   gsHashTable.vbbRandomKeys[ 395 ] = 12333321330626399551;
   gsHashTable.vbbRandomKeys[ 396 ] = 12557239717278413627;
   gsHashTable.vbbRandomKeys[ 397 ] = 711921472326200139;
   gsHashTable.vbbRandomKeys[ 398 ] = 1248958513585921226;
   gsHashTable.vbbRandomKeys[ 399 ] = 12550937280854436547;
   gsHashTable.vbbRandomKeys[ 400 ] = 14669033909621100883;
   gsHashTable.vbbRandomKeys[ 401 ] = 9466268398626418938;
   gsHashTable.vbbRandomKeys[ 402 ] = 13432810597552324109;
   gsHashTable.vbbRandomKeys[ 403 ] = 5343148220256586368;
   gsHashTable.vbbRandomKeys[ 404 ] = 12628375941092352036;
   gsHashTable.vbbRandomKeys[ 405 ] = 6274488562225975663;
   gsHashTable.vbbRandomKeys[ 406 ] = 10091146402508099078;
   gsHashTable.vbbRandomKeys[ 407 ] = 6735366233725645197;
   gsHashTable.vbbRandomKeys[ 408 ] = 14070396363361601407;
   gsHashTable.vbbRandomKeys[ 409 ] = 5691273042477788423;
   gsHashTable.vbbRandomKeys[ 410 ] = 9899249355509703130;
   gsHashTable.vbbRandomKeys[ 411 ] = 4168575018978437633;
   gsHashTable.vbbRandomKeys[ 412 ] = 7123371073236794656;
   gsHashTable.vbbRandomKeys[ 413 ] = 5230587847850043228;
   gsHashTable.vbbRandomKeys[ 414 ] = 3333180680299038160;
   gsHashTable.vbbRandomKeys[ 415 ] = 6814513973395387372;
   gsHashTable.vbbRandomKeys[ 416 ] = 2495707657782111241;
   gsHashTable.vbbRandomKeys[ 417 ] = 10764142983642287384;
   gsHashTable.vbbRandomKeys[ 418 ] = 16243691210665612633;
   gsHashTable.vbbRandomKeys[ 419 ] = 370495672740109600;
   gsHashTable.vbbRandomKeys[ 420 ] = 9548695876324166232;
   gsHashTable.vbbRandomKeys[ 421 ] = 13291990783127785922;
   gsHashTable.vbbRandomKeys[ 422 ] = 4691852551306627882;
   gsHashTable.vbbRandomKeys[ 423 ] = 377789437597219442;
   gsHashTable.vbbRandomKeys[ 424 ] = 12637302334845728008;
   gsHashTable.vbbRandomKeys[ 425 ] = 15745184260535432129;
   gsHashTable.vbbRandomKeys[ 426 ] = 9251852216076414260;
   gsHashTable.vbbRandomKeys[ 427 ] = 13109527194562171534;
   gsHashTable.vbbRandomKeys[ 428 ] = 4712687622708765825;
   gsHashTable.vbbRandomKeys[ 429 ] = 11897937273543026496;
   gsHashTable.vbbRandomKeys[ 430 ] = 10384266748740935321;
   gsHashTable.vbbRandomKeys[ 431 ] = 2339769022993372408;
   gsHashTable.vbbRandomKeys[ 432 ] = 9864906115327565609;
   gsHashTable.vbbRandomKeys[ 433 ] = 16961856502834081231;
   gsHashTable.vbbRandomKeys[ 434 ] = 15564245899588621595;
   gsHashTable.vbbRandomKeys[ 435 ] = 1135328958288613848;
   gsHashTable.vbbRandomKeys[ 436 ] = 9558620628584231074;
   gsHashTable.vbbRandomKeys[ 437 ] = 632884043052786381;
   gsHashTable.vbbRandomKeys[ 438 ] = 8532326043787566476;
   gsHashTable.vbbRandomKeys[ 439 ] = 1359433726221306278;
   gsHashTable.vbbRandomKeys[ 440 ] = 10710205753731219419;
   gsHashTable.vbbRandomKeys[ 441 ] = 17046264454822289123;
   gsHashTable.vbbRandomKeys[ 442 ] = 12555020774262218399;
   gsHashTable.vbbRandomKeys[ 443 ] = 5294579529791718129;
   gsHashTable.vbbRandomKeys[ 444 ] = 12447274105395453054;
   gsHashTable.vbbRandomKeys[ 445 ] = 16672369431365133693;
   gsHashTable.vbbRandomKeys[ 446 ] = 11701847768641363200;
   gsHashTable.vbbRandomKeys[ 447 ] = 17170550045396202732;
   gsHashTable.vbbRandomKeys[ 448 ] = 4490542930025184466;
   gsHashTable.vbbRandomKeys[ 449 ] = 15834100041171453383;
   gsHashTable.vbbRandomKeys[ 450 ] = 15270988561449989866;
   gsHashTable.vbbRandomKeys[ 451 ] = 5668451761626092185;
   gsHashTable.vbbRandomKeys[ 452 ] = 17161290564051505833;
   gsHashTable.vbbRandomKeys[ 453 ] = 7564278081283818305;
   gsHashTable.vbbRandomKeys[ 454 ] = 12729810027329144832;
   gsHashTable.vbbRandomKeys[ 455 ] = 6406004772211203805;
   gsHashTable.vbbRandomKeys[ 456 ] = 17801225335955684145;
   gsHashTable.vbbRandomKeys[ 457 ] = 3363231397499148717;
   gsHashTable.vbbRandomKeys[ 458 ] = 14172287922227380680;
   gsHashTable.vbbRandomKeys[ 459 ] = 11822437593268974367;
   gsHashTable.vbbRandomKeys[ 460 ] = 17962550678788775782;
   gsHashTable.vbbRandomKeys[ 461 ] = 4861620716644554428;
   gsHashTable.vbbRandomKeys[ 462 ] = 663479320492724640;
   gsHashTable.vbbRandomKeys[ 463 ] = 15038372726560660296;
   gsHashTable.vbbRandomKeys[ 464 ] = 15334056276650248315;
   gsHashTable.vbbRandomKeys[ 465 ] = 12877855713504224472;
   gsHashTable.vbbRandomKeys[ 466 ] = 7400323965941855121;
   gsHashTable.vbbRandomKeys[ 467 ] = 7374846568171888216;
   gsHashTable.vbbRandomKeys[ 468 ] = 16219934151174535670;
   gsHashTable.vbbRandomKeys[ 469 ] = 6819090430266045104;
   gsHashTable.vbbRandomKeys[ 470 ] = 5915793383058770127;
   gsHashTable.vbbRandomKeys[ 471 ] = 5481328324649163174;
   gsHashTable.vbbRandomKeys[ 472 ] = 13537741837911538281;
   gsHashTable.vbbRandomKeys[ 473 ] = 11420984484079821882;
   gsHashTable.vbbRandomKeys[ 474 ] = 14384981225261819145;
   gsHashTable.vbbRandomKeys[ 475 ] = 10433351171427581842;
   gsHashTable.vbbRandomKeys[ 476 ] = 8454217209990498148;
   gsHashTable.vbbRandomKeys[ 477 ] = 6936220899744219434;
   gsHashTable.vbbRandomKeys[ 478 ] = 17410798448172204801;
   gsHashTable.vbbRandomKeys[ 479 ] = 5540752735179067256;
   gsHashTable.vbbRandomKeys[ 480 ] = 6683291264837942198;
   gsHashTable.vbbRandomKeys[ 481 ] = 10604430502535205467;
   gsHashTable.vbbRandomKeys[ 482 ] = 3834750191244057521;
   gsHashTable.vbbRandomKeys[ 483 ] = 4102573974495202389;
   gsHashTable.vbbRandomKeys[ 484 ] = 17614451825660521333;
   gsHashTable.vbbRandomKeys[ 485 ] = 12773414607989170461;
   gsHashTable.vbbRandomKeys[ 486 ] = 10641228511198858870;
   gsHashTable.vbbRandomKeys[ 487 ] = 16196391165890649377;
   gsHashTable.vbbRandomKeys[ 488 ] = 1304077062221260649;
   gsHashTable.vbbRandomKeys[ 489 ] = 18123750108214158251;
   gsHashTable.vbbRandomKeys[ 490 ] = 7745484898088222317;
   gsHashTable.vbbRandomKeys[ 491 ] = 18150787232046897296;
   gsHashTable.vbbRandomKeys[ 492 ] = 1611596254753085333;
   gsHashTable.vbbRandomKeys[ 493 ] = 8045216616385771651;
   gsHashTable.vbbRandomKeys[ 494 ] = 13808549922858978704;
   gsHashTable.vbbRandomKeys[ 495 ] = 16108378890346828216;
   gsHashTable.vbbRandomKeys[ 496 ] = 1407213294348378099;
   gsHashTable.vbbRandomKeys[ 497 ] = 13388761525887756758;
   gsHashTable.vbbRandomKeys[ 498 ] = 10288668673086868716;
   gsHashTable.vbbRandomKeys[ 499 ] = 12062998194622282169;
   gsHashTable.vbbRandomKeys[ 500 ] = 2168967169336839131;
   gsHashTable.vbbRandomKeys[ 501 ] = 10763200532822696293;
   gsHashTable.vbbRandomKeys[ 502 ] = 8505786302485354209;
   gsHashTable.vbbRandomKeys[ 503 ] = 17820179250848101614;
   gsHashTable.vbbRandomKeys[ 504 ] = 6437884345877787836;
   gsHashTable.vbbRandomKeys[ 505 ] = 5266453928088178996;
   gsHashTable.vbbRandomKeys[ 506 ] = 11731306319480219669;
   gsHashTable.vbbRandomKeys[ 507 ] = 16912954515892114024;
   gsHashTable.vbbRandomKeys[ 508 ] = 10217713217374574526;
   gsHashTable.vbbRandomKeys[ 509 ] = 5610692145543001066;
   gsHashTable.vbbRandomKeys[ 510 ] = 7140083000022179288;
   gsHashTable.vbbRandomKeys[ 511 ] = 17133361304417786769;
   gsHashTable.vbbRandomKeys[ 512 ] = 6735787271417561845;
   gsHashTable.vbbRandomKeys[ 513 ] = 15879336771283630301;
   gsHashTable.vbbRandomKeys[ 514 ] = 7858931196142548205;
   gsHashTable.vbbRandomKeys[ 515 ] = 12862420241811297053;
   gsHashTable.vbbRandomKeys[ 516 ] = 12285133650635991785;
   gsHashTable.vbbRandomKeys[ 517 ] = 17417646803644721457;
   gsHashTable.vbbRandomKeys[ 518 ] = 936216325614902133;
   gsHashTable.vbbRandomKeys[ 519 ] = 13454994280465408065;
   gsHashTable.vbbRandomKeys[ 520 ] = 17770511996638374287;
   gsHashTable.vbbRandomKeys[ 521 ] = 1066095093704430692;
   gsHashTable.vbbRandomKeys[ 522 ] = 3269142729367898223;
   gsHashTable.vbbRandomKeys[ 523 ] = 18403649462481367650;
   gsHashTable.vbbRandomKeys[ 524 ] = 13627802623331568067;
   gsHashTable.vbbRandomKeys[ 525 ] = 12782592477921769766;
   gsHashTable.vbbRandomKeys[ 526 ] = 4214677265381411434;
   gsHashTable.vbbRandomKeys[ 527 ] = 4064654417285393272;
   gsHashTable.vbbRandomKeys[ 528 ] = 5962280267623677146;
   gsHashTable.vbbRandomKeys[ 529 ] = 17507077312458502872;
   gsHashTable.vbbRandomKeys[ 530 ] = 11036129989614529025;
   gsHashTable.vbbRandomKeys[ 531 ] = 3693567730248790600;
   gsHashTable.vbbRandomKeys[ 532 ] = 7190905165140119873;
   gsHashTable.vbbRandomKeys[ 533 ] = 17581380900557916465;
   gsHashTable.vbbRandomKeys[ 534 ] = 5333492033637306699;
   gsHashTable.vbbRandomKeys[ 535 ] = 14624742332086753900;
   gsHashTable.vbbRandomKeys[ 536 ] = 18350938733083317667;
   gsHashTable.vbbRandomKeys[ 537 ] = 10905527976632169590;
   gsHashTable.vbbRandomKeys[ 538 ] = 4801456446983871146;
   gsHashTable.vbbRandomKeys[ 539 ] = 3266050529504486320;
   gsHashTable.vbbRandomKeys[ 540 ] = 5865160423984896601;
   gsHashTable.vbbRandomKeys[ 541 ] = 1223417422661166166;
   gsHashTable.vbbRandomKeys[ 542 ] = 434381393777217448;
   gsHashTable.vbbRandomKeys[ 543 ] = 7301578196664402864;
   gsHashTable.vbbRandomKeys[ 544 ] = 4597902016408901072;
   gsHashTable.vbbRandomKeys[ 545 ] = 13288351202132573098;
   gsHashTable.vbbRandomKeys[ 546 ] = 10328150304000951788;
   gsHashTable.vbbRandomKeys[ 547 ] = 16154358768539163100;
   gsHashTable.vbbRandomKeys[ 548 ] = 8341483093453518288;
   gsHashTable.vbbRandomKeys[ 549 ] = 15019937017881069024;
   gsHashTable.vbbRandomKeys[ 550 ] = 3199922319887440068;
   gsHashTable.vbbRandomKeys[ 551 ] = 9466685674597639495;
   gsHashTable.vbbRandomKeys[ 552 ] = 2260843730451057181;
   gsHashTable.vbbRandomKeys[ 553 ] = 1824238150244808735;
   gsHashTable.vbbRandomKeys[ 554 ] = 11229003979400982185;
   gsHashTable.vbbRandomKeys[ 555 ] = 3962316491299950406;
   gsHashTable.vbbRandomKeys[ 556 ] = 7145429124507414799;
   gsHashTable.vbbRandomKeys[ 557 ] = 6993884963598058289;
   gsHashTable.vbbRandomKeys[ 558 ] = 874119196077229802;
   gsHashTable.vbbRandomKeys[ 559 ] = 3162874772651062127;
   gsHashTable.vbbRandomKeys[ 560 ] = 14957852747194196030;
   gsHashTable.vbbRandomKeys[ 561 ] = 13926061009820464390;
   gsHashTable.vbbRandomKeys[ 562 ] = 5479841853734785281;
   gsHashTable.vbbRandomKeys[ 563 ] = 16262473193769399526;
   gsHashTable.vbbRandomKeys[ 564 ] = 14204598813043204049;
   gsHashTable.vbbRandomKeys[ 565 ] = 8493869582040325805;
   gsHashTable.vbbRandomKeys[ 566 ] = 4392363342027311998;
   gsHashTable.vbbRandomKeys[ 567 ] = 8236730659329627992;
   gsHashTable.vbbRandomKeys[ 568 ] = 2486787887784337045;
   gsHashTable.vbbRandomKeys[ 569 ] = 1438259652878979479;
   gsHashTable.vbbRandomKeys[ 570 ] = 12910592520114890708;
   gsHashTable.vbbRandomKeys[ 571 ] = 14089457223824596794;
   gsHashTable.vbbRandomKeys[ 572 ] = 13374234815557086233;
   gsHashTable.vbbRandomKeys[ 573 ] = 7320620899483278565;
   gsHashTable.vbbRandomKeys[ 574 ] = 3978693214514368795;
   gsHashTable.vbbRandomKeys[ 575 ] = 3717236585699311422;
   gsHashTable.vbbRandomKeys[ 576 ] = 2608941221941403513;
   gsHashTable.vbbRandomKeys[ 577 ] = 14006703223334955255;
   gsHashTable.vbbRandomKeys[ 578 ] = 1881255124488659029;
   gsHashTable.vbbRandomKeys[ 579 ] = 3800903636494582527;
   gsHashTable.vbbRandomKeys[ 580 ] = 11204942675800277427;
   gsHashTable.vbbRandomKeys[ 581 ] = 17825417559917674922;
   gsHashTable.vbbRandomKeys[ 582 ] = 9174535531020623146;
   gsHashTable.vbbRandomKeys[ 583 ] = 14213211179127315137;
   gsHashTable.vbbRandomKeys[ 584 ] = 14507101922313192387;
   gsHashTable.vbbRandomKeys[ 585 ] = 15543127870003203059;
   gsHashTable.vbbRandomKeys[ 586 ] = 14704485344828382208;
   gsHashTable.vbbRandomKeys[ 587 ] = 8799618389701523964;
   gsHashTable.vbbRandomKeys[ 588 ] = 7994512334209114400;
   gsHashTable.vbbRandomKeys[ 589 ] = 12199618317420952464;
   gsHashTable.vbbRandomKeys[ 590 ] = 11137551833257844433;
   gsHashTable.vbbRandomKeys[ 591 ] = 1677938927126126656;
   gsHashTable.vbbRandomKeys[ 592 ] = 14621391259534309951;
   gsHashTable.vbbRandomKeys[ 593 ] = 11297169633756526499;
   gsHashTable.vbbRandomKeys[ 594 ] = 6569564832007778117;
   gsHashTable.vbbRandomKeys[ 595 ] = 339659959559306900;
   gsHashTable.vbbRandomKeys[ 596 ] = 10264066308000784394;
   gsHashTable.vbbRandomKeys[ 597 ] = 1161055478009014470;
   gsHashTable.vbbRandomKeys[ 598 ] = 12149343690814368795;
   gsHashTable.vbbRandomKeys[ 599 ] = 5994390635656134610;
   gsHashTable.vbbRandomKeys[ 600 ] = 13238856641081644408;
   gsHashTable.vbbRandomKeys[ 601 ] = 16033302142659539088;
   gsHashTable.vbbRandomKeys[ 602 ] = 14388684507424835392;
   gsHashTable.vbbRandomKeys[ 603 ] = 1777267578553527241;
   gsHashTable.vbbRandomKeys[ 604 ] = 18041613971552494045;
   gsHashTable.vbbRandomKeys[ 605 ] = 12472235958415576895;
   gsHashTable.vbbRandomKeys[ 606 ] = 9710678362901299840;
   gsHashTable.vbbRandomKeys[ 607 ] = 11739291760427509428;
   gsHashTable.vbbRandomKeys[ 608 ] = 1216225364132997779;
   gsHashTable.vbbRandomKeys[ 609 ] = 3941727980584103401;
   gsHashTable.vbbRandomKeys[ 610 ] = 14634031960547357335;
   gsHashTable.vbbRandomKeys[ 611 ] = 15395901354824670013;
   gsHashTable.vbbRandomKeys[ 612 ] = 11467994748494269665;
   gsHashTable.vbbRandomKeys[ 613 ] = 13889241926071813622;
   gsHashTable.vbbRandomKeys[ 614 ] = 739035727687969921;
   gsHashTable.vbbRandomKeys[ 615 ] = 17231804267709361002;
   gsHashTable.vbbRandomKeys[ 616 ] = 4215373595798798494;
   gsHashTable.vbbRandomKeys[ 617 ] = 10504150901472949749;
   gsHashTable.vbbRandomKeys[ 618 ] = 8679193723467083578;
   gsHashTable.vbbRandomKeys[ 619 ] = 11803348387223915686;
   gsHashTable.vbbRandomKeys[ 620 ] = 2379359756692057252;
   gsHashTable.vbbRandomKeys[ 621 ] = 4605482384475317035;
   gsHashTable.vbbRandomKeys[ 622 ] = 1010353084515816675;
   gsHashTable.vbbRandomKeys[ 623 ] = 2497517606085496582;
   gsHashTable.vbbRandomKeys[ 624 ] = 15616420830894623793;
   gsHashTable.vbbRandomKeys[ 625 ] = 3434319739494326420;
   gsHashTable.vbbRandomKeys[ 626 ] = 17069741457771118638;
   gsHashTable.vbbRandomKeys[ 627 ] = 5981782967497335006;
   gsHashTable.vbbRandomKeys[ 628 ] = 5001122469643909553;
   gsHashTable.vbbRandomKeys[ 629 ] = 11599905566257442065;
   gsHashTable.vbbRandomKeys[ 630 ] = 17727351619505203727;
   gsHashTable.vbbRandomKeys[ 631 ] = 16604602388845362308;
   gsHashTable.vbbRandomKeys[ 632 ] = 7431935967310015915;
   gsHashTable.vbbRandomKeys[ 633 ] = 11212381932580776872;
   gsHashTable.vbbRandomKeys[ 634 ] = 2638043397942328569;
   gsHashTable.vbbRandomKeys[ 635 ] = 11918251849269776811;
   gsHashTable.vbbRandomKeys[ 636 ] = 3941018871267158112;
   gsHashTable.vbbRandomKeys[ 637 ] = 14755077219103732577;
   gsHashTable.vbbRandomKeys[ 638 ] = 38943976481445955;
   gsHashTable.vbbRandomKeys[ 639 ] = 13565538522546702499;
   gsHashTable.vbbRandomKeys[ 640 ] = 9705870979313148148;
   gsHashTable.vbbRandomKeys[ 641 ] = 16731429477385924949;
   gsHashTable.vbbRandomKeys[ 642 ] = 802754698401058373;
   gsHashTable.vbbRandomKeys[ 643 ] = 5859328887625108595;
   gsHashTable.vbbRandomKeys[ 644 ] = 8573349314945064048;
   gsHashTable.vbbRandomKeys[ 645 ] = 1366082109386191881;
   gsHashTable.vbbRandomKeys[ 646 ] = 4864362512511525524;
   gsHashTable.vbbRandomKeys[ 647 ] = 1151964852379331268;
   gsHashTable.vbbRandomKeys[ 648 ] = 17392045688673152227;
   gsHashTable.vbbRandomKeys[ 649 ] = 17190322757803815441;
   gsHashTable.vbbRandomKeys[ 650 ] = 13638322019474538514;
   gsHashTable.vbbRandomKeys[ 651 ] = 13393743891064251465;
   gsHashTable.vbbRandomKeys[ 652 ] = 9543889586995202454;
   gsHashTable.vbbRandomKeys[ 653 ] = 8395935506392517064;
   gsHashTable.vbbRandomKeys[ 654 ] = 14415930661618391446;
   gsHashTable.vbbRandomKeys[ 655 ] = 5355501773312198872;
   gsHashTable.vbbRandomKeys[ 656 ] = 10934661199436056295;
   gsHashTable.vbbRandomKeys[ 657 ] = 7659608848262917338;
   gsHashTable.vbbRandomKeys[ 658 ] = 16553112922340276880;
   gsHashTable.vbbRandomKeys[ 659 ] = 10572143000794430237;
   gsHashTable.vbbRandomKeys[ 660 ] = 10497647641077701813;
   gsHashTable.vbbRandomKeys[ 661 ] = 1207807617012930323;
   gsHashTable.vbbRandomKeys[ 662 ] = 8179076230942276001;
   gsHashTable.vbbRandomKeys[ 663 ] = 17277093565345336337;
   gsHashTable.vbbRandomKeys[ 664 ] = 6847275515939942774;
   gsHashTable.vbbRandomKeys[ 665 ] = 14196185070498631750;
   gsHashTable.vbbRandomKeys[ 666 ] = 467287672779915272;
   gsHashTable.vbbRandomKeys[ 667 ] = 13233732212875929072;
   gsHashTable.vbbRandomKeys[ 668 ] = 16578657490136270162;
   gsHashTable.vbbRandomKeys[ 669 ] = 9774463890126697154;
   gsHashTable.vbbRandomKeys[ 670 ] = 13958428880568520635;
   gsHashTable.vbbRandomKeys[ 671 ] = 4019287109102408651;
   gsHashTable.vbbRandomKeys[ 672 ] = 17152213106257554727;
   gsHashTable.vbbRandomKeys[ 673 ] = 3166365097778196402;
   gsHashTable.vbbRandomKeys[ 674 ] = 7316566627464373330;
   gsHashTable.vbbRandomKeys[ 675 ] = 13683390819964593770;
   gsHashTable.vbbRandomKeys[ 676 ] = 7028976381171577227;
   gsHashTable.vbbRandomKeys[ 677 ] = 2715071596548175905;
   gsHashTable.vbbRandomKeys[ 678 ] = 2546794203174789776;
   gsHashTable.vbbRandomKeys[ 679 ] = 4131168688374455886;
   gsHashTable.vbbRandomKeys[ 680 ] = 12042854394675755374;
   gsHashTable.vbbRandomKeys[ 681 ] = 17503579178160605270;
   gsHashTable.vbbRandomKeys[ 682 ] = 10728948009755923058;
   gsHashTable.vbbRandomKeys[ 683 ] = 17278076772175334761;
   gsHashTable.vbbRandomKeys[ 684 ] = 6449223468297355373;
   gsHashTable.vbbRandomKeys[ 685 ] = 7543872698209144130;
   gsHashTable.vbbRandomKeys[ 686 ] = 17077464670670506504;
   gsHashTable.vbbRandomKeys[ 687 ] = 10924120912844579657;
   gsHashTable.vbbRandomKeys[ 688 ] = 12703420672138699025;
   gsHashTable.vbbRandomKeys[ 689 ] = 12927819252207940502;
   gsHashTable.vbbRandomKeys[ 690 ] = 7462414154901210654;
   gsHashTable.vbbRandomKeys[ 691 ] = 13838019486917932965;
   gsHashTable.vbbRandomKeys[ 692 ] = 6472500327961808229;
   gsHashTable.vbbRandomKeys[ 693 ] = 9866902577321020832;
   gsHashTable.vbbRandomKeys[ 694 ] = 1563797348849008616;
   gsHashTable.vbbRandomKeys[ 695 ] = 15633184522225135279;
   gsHashTable.vbbRandomKeys[ 696 ] = 4411398937464551177;
   gsHashTable.vbbRandomKeys[ 697 ] = 1734017110485417527;
   gsHashTable.vbbRandomKeys[ 698 ] = 12441314272213774324;
   gsHashTable.vbbRandomKeys[ 699 ] = 1066407786230842984;
   gsHashTable.vbbRandomKeys[ 700 ] = 6478156725563690490;
   gsHashTable.vbbRandomKeys[ 701 ] = 17086378179193669309;
   gsHashTable.vbbRandomKeys[ 702 ] = 14572068555270591374;
   gsHashTable.vbbRandomKeys[ 703 ] = 6886808589363256775;
   gsHashTable.vbbRandomKeys[ 704 ] = 11755290753610531065;
   gsHashTable.vbbRandomKeys[ 705 ] = 17131188977051811206;
   gsHashTable.vbbRandomKeys[ 706 ] = 16524071407999343412;
   gsHashTable.vbbRandomKeys[ 707 ] = 10373996705263706714;
   gsHashTable.vbbRandomKeys[ 708 ] = 11503115787288575475;
   gsHashTable.vbbRandomKeys[ 709 ] = 9683850348888951678;
   gsHashTable.vbbRandomKeys[ 710 ] = 7277229627429420262;
   gsHashTable.vbbRandomKeys[ 711 ] = 16245005074696261451;
   gsHashTable.vbbRandomKeys[ 712 ] = 7674682503392413631;
   gsHashTable.vbbRandomKeys[ 713 ] = 10139370587039834767;
   gsHashTable.vbbRandomKeys[ 714 ] = 15416465182628392692;
   gsHashTable.vbbRandomKeys[ 715 ] = 14244584779681740221;
   gsHashTable.vbbRandomKeys[ 716 ] = 9972580351325836508;
   gsHashTable.vbbRandomKeys[ 717 ] = 7279797674797966103;
   gsHashTable.vbbRandomKeys[ 718 ] = 11825581315205784744;
   gsHashTable.vbbRandomKeys[ 719 ] = 16945833380386602550;
   gsHashTable.vbbRandomKeys[ 720 ] = 6791896365715253599;
   gsHashTable.vbbRandomKeys[ 721 ] = 6945313208353081657;
   gsHashTable.vbbRandomKeys[ 722 ] = 16294808480662096093;
   gsHashTable.vbbRandomKeys[ 723 ] = 12221239568569296496;
   gsHashTable.vbbRandomKeys[ 724 ] = 11247417372709685536;
   gsHashTable.vbbRandomKeys[ 725 ] = 7938873558142830553;
   gsHashTable.vbbRandomKeys[ 726 ] = 3088317014682678078;
   gsHashTable.vbbRandomKeys[ 727 ] = 9111214889041263155;
   gsHashTable.vbbRandomKeys[ 728 ] = 1265991644320173425;
   gsHashTable.vbbRandomKeys[ 729 ] = 16378441826843806725;
   gsHashTable.vbbRandomKeys[ 730 ] = 9987141752145990129;
   gsHashTable.vbbRandomKeys[ 731 ] = 4712911999066775696;
   gsHashTable.vbbRandomKeys[ 732 ] = 1091683086315350892;
   gsHashTable.vbbRandomKeys[ 733 ] = 13727136165381515233;
   gsHashTable.vbbRandomKeys[ 734 ] = 15795530794364513535;
   gsHashTable.vbbRandomKeys[ 735 ] = 8388682948336516610;
   gsHashTable.vbbRandomKeys[ 736 ] = 16246031262521150924;
   gsHashTable.vbbRandomKeys[ 737 ] = 14550513926044067208;
   gsHashTable.vbbRandomKeys[ 738 ] = 13538296560505696104;
   gsHashTable.vbbRandomKeys[ 739 ] = 16748709196863972138;
   gsHashTable.vbbRandomKeys[ 740 ] = 10122687395401356471;
   gsHashTable.vbbRandomKeys[ 741 ] = 3372511915377055679;
   gsHashTable.vbbRandomKeys[ 742 ] = 13616712054906680649;
   gsHashTable.vbbRandomKeys[ 743 ] = 14175791590636824995;
   gsHashTable.vbbRandomKeys[ 744 ] = 1389405298971986549;
   gsHashTable.vbbRandomKeys[ 745 ] = 10426426967119908001;
   gsHashTable.vbbRandomKeys[ 746 ] = 12358648985599658077;
   gsHashTable.vbbRandomKeys[ 747 ] = 16526806724558558692;
   gsHashTable.vbbRandomKeys[ 748 ] = 14854968871305145772;
   gsHashTable.vbbRandomKeys[ 749 ] = 8509033789540071550;
   gsHashTable.vbbRandomKeys[ 750 ] = 6203636036589020240;
   gsHashTable.vbbRandomKeys[ 751 ] = 2385203061819987805;
   gsHashTable.vbbRandomKeys[ 752 ] = 8205234402520923534;
   gsHashTable.vbbRandomKeys[ 753 ] = 14129550963374704796;
   gsHashTable.vbbRandomKeys[ 754 ] = 15815748951467491001;
   gsHashTable.vbbRandomKeys[ 755 ] = 14443034831437213696;
   gsHashTable.vbbRandomKeys[ 756 ] = 14450737780254747528;
   gsHashTable.vbbRandomKeys[ 757 ] = 310135492839025137;
   gsHashTable.vbbRandomKeys[ 758 ] = 14939551031538396458;
   gsHashTable.vbbRandomKeys[ 759 ] = 700160981297204979;
   gsHashTable.vbbRandomKeys[ 760 ] = 5464631219857791783;
   gsHashTable.vbbRandomKeys[ 761 ] = 10998694675053622467;
   gsHashTable.vbbRandomKeys[ 762 ] = 17736739849700738874;
   gsHashTable.vbbRandomKeys[ 763 ] = 16210024720852160608;
   gsHashTable.vbbRandomKeys[ 764 ] = 2260830677670491392;
   gsHashTable.vbbRandomKeys[ 765 ] = 14326085389032563505;
   gsHashTable.vbbRandomKeys[ 766 ] = 15381665477938131459;
   gsHashTable.vbbRandomKeys[ 767 ] = 10204650436469811106;
   gsHashTable.vbbRandomKeys[ 768 ] = 13310277797079362845;
   gsHashTable.vbbRandomKeys[ 769 ] = 4486253165076233351;
   gsHashTable.vbbRandomKeys[ 770 ] = 3214134056478690477;
   gsHashTable.vbbRandomKeys[ 771 ] = 12072811996655449274;
   gsHashTable.vbbRandomKeys[ 772 ] = 16986164094371584164;
   gsHashTable.vbbRandomKeys[ 773 ] = 16990403225960217102;
   gsHashTable.vbbRandomKeys[ 774 ] = 2500847829156690369;
   gsHashTable.vbbRandomKeys[ 775 ] = 14578363180205550801;
   gsHashTable.vbbRandomKeys[ 776 ] = 2189601008890531998;
   gsHashTable.vbbRandomKeys[ 777 ] = 18113605750591796951;
   gsHashTable.vbbRandomKeys[ 778 ] = 8808929484567163142;
   gsHashTable.vbbRandomKeys[ 779 ] = 5447322282283184429;
   gsHashTable.vbbRandomKeys[ 780 ] = 17953006113929159523;
   gsHashTable.vbbRandomKeys[ 781 ] = 4206650648250308834;
   gsHashTable.vbbRandomKeys[ 782 ] = 764005333152791095;
   gsHashTable.vbbRandomKeys[ 783 ] = 6802054851484128021;
   gsHashTable.vbbRandomKeys[ 784 ] = 10186959537498457575;
   gsHashTable.vbbRandomKeys[ 785 ] = 6013786835328322120;
   gsHashTable.vbbRandomKeys[ 786 ] = 14355486565686183313;
   gsHashTable.vbbRandomKeys[ 787 ] = 12737695732105077560;
   gsHashTable.vbbRandomKeys[ 788 ] = 577070768361750432;
   gsHashTable.vbbRandomKeys[ 789 ] = 17836294213057326572;
   gsHashTable.vbbRandomKeys[ 790 ] = 6292287486412957021;
   gsHashTable.vbbRandomKeys[ 791 ] = 10102663156850393321;
   gsHashTable.vbbRandomKeys[ 792 ] = 15131189335648891829;
   gsHashTable.vbbRandomKeys[ 793 ] = 8444560394021001992;
   gsHashTable.vbbRandomKeys[ 794 ] = 5393496927744779613;
   gsHashTable.vbbRandomKeys[ 795 ] = 3692841011119961827;
   gsHashTable.vbbRandomKeys[ 796 ] = 11224494698699900707;
   gsHashTable.vbbRandomKeys[ 797 ] = 14190738106914624288;
   gsHashTable.vbbRandomKeys[ 798 ] = 15057800283914283200;
   gsHashTable.vbbRandomKeys[ 799 ] = 3787145909357773919;
   gsHashTable.vbbRandomKeys[ 800 ] = 14272432467076396516;
   gsHashTable.vbbRandomKeys[ 801 ] = 10246858249508536232;
   gsHashTable.vbbRandomKeys[ 802 ] = 15303887680985898288;
   gsHashTable.vbbRandomKeys[ 803 ] = 15996367696030052102;
   gsHashTable.vbbRandomKeys[ 804 ] = 17211754503046290657;
   gsHashTable.vbbRandomKeys[ 805 ] = 4768643932683150514;
   gsHashTable.vbbRandomKeys[ 806 ] = 10941609835690510853;
   gsHashTable.vbbRandomKeys[ 807 ] = 14579149437321725037;
   gsHashTable.vbbRandomKeys[ 808 ] = 2793758436475448277;
   gsHashTable.vbbRandomKeys[ 809 ] = 5921838086377620211;
   gsHashTable.vbbRandomKeys[ 810 ] = 14212899169182171597;
   gsHashTable.vbbRandomKeys[ 811 ] = 8873563698559260035;
   gsHashTable.vbbRandomKeys[ 812 ] = 18035910650225975026;
   gsHashTable.vbbRandomKeys[ 813 ] = 12507473593614146414;
   gsHashTable.vbbRandomKeys[ 814 ] = 12712632422104824629;
   gsHashTable.vbbRandomKeys[ 815 ] = 303017059599393366;
   gsHashTable.vbbRandomKeys[ 816 ] = 12699130826967550991;
   gsHashTable.vbbRandomKeys[ 817 ] = 7588112800533069575;
   gsHashTable.vbbRandomKeys[ 818 ] = 16197751598086055561;
   gsHashTable.vbbRandomKeys[ 819 ] = 10992498009490893610;
   gsHashTable.vbbRandomKeys[ 820 ] = 5684436995755082471;
   gsHashTable.vbbRandomKeys[ 821 ] = 13333597431820927435;
   gsHashTable.vbbRandomKeys[ 822 ] = 15774691191107388800;
   gsHashTable.vbbRandomKeys[ 823 ] = 9864548368542560451;
   gsHashTable.vbbRandomKeys[ 824 ] = 14776618675940843523;
   gsHashTable.vbbRandomKeys[ 825 ] = 12543802211375948263;
   gsHashTable.vbbRandomKeys[ 826 ] = 16898926597733160885;
   gsHashTable.vbbRandomKeys[ 827 ] = 11367807395364471738;
   gsHashTable.vbbRandomKeys[ 828 ] = 12601222305507396866;
   gsHashTable.vbbRandomKeys[ 829 ] = 2635365032461521742;
   gsHashTable.vbbRandomKeys[ 830 ] = 14935026512025795195;
   gsHashTable.vbbRandomKeys[ 831 ] = 14502999676582431288;
   gsHashTable.vbbRandomKeys[ 832 ] = 11325998485063304336;
   gsHashTable.vbbRandomKeys[ 833 ] = 9179009070155872364;
   gsHashTable.vbbRandomKeys[ 834 ] = 7920872266765757528;
   gsHashTable.vbbRandomKeys[ 835 ] = 15741154000658882844;
   gsHashTable.vbbRandomKeys[ 836 ] = 10333935457558719007;
   gsHashTable.vbbRandomKeys[ 837 ] = 1861598860104210920;
   gsHashTable.vbbRandomKeys[ 838 ] = 11325941964653808226;
   gsHashTable.vbbRandomKeys[ 839 ] = 9916147574033958671;
   gsHashTable.vbbRandomKeys[ 840 ] = 3253782930022084337;
   gsHashTable.vbbRandomKeys[ 841 ] = 17910717658979510453;
   gsHashTable.vbbRandomKeys[ 842 ] = 5344980397914292860;
   gsHashTable.vbbRandomKeys[ 843 ] = 15788993092722404193;
   gsHashTable.vbbRandomKeys[ 844 ] = 17208086862359937279;

}


//
//---------------------------------------------------------------------------------------------------------
//
// Switch the side to move.
void SwitchSideToMove( struct Board * argsBoard )
{

   // Update the Hash
   gsHashTable.bbHash = gsHashTable.bbHash ^
                        gsHashTable.vbbHashKeysStates[ 1 ];

   // Update the Hash
   gsHashTable.bbHash = gsHashTable.bbHash ^
                        gsHashTable.vbbHashKeysStates[ 0 ];

   // Switch the side to move.
   switch ( argsBoard->siColorToMove )
   {

      case dWhite :
      {
   
         // Switch the color
         argsBoard->siColorToMove = dBlack;

         // Take away the en passant
         argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare = 0;

         break;

      }
      case dBlack :
      {
      
         // Switch the side to move.
         argsBoard->siColorToMove = dWhite;

         // Take away the en passant
         argsBoard->sHistoryStack[ argsBoard->iMoveHistory ].bbEPSquare = 0;

      }

   }   

}

//
//--------------------------------------------------------------------------------------------------
//
BitBoard Power( int iBase, int iExponent )
{

   BitBoard bbAnswer = iBase;

   if ( iExponent == 0 )
   {

      return 1;

   }

   for ( int iCounter = 0; iCounter < iExponent - 1; iCounter++ )
   {

      bbAnswer *= iBase;

   }

   return bbAnswer;

}

//
//--------------------------------------------------------------------------------------------------
//
int GetHashScore()
{
   return gsHashTable.iScore;
}
BitBoard GetMaskIndex()
{
   return gsHashTable.bbMaskIndex;
}
BitBoard GetKey()
{
   return GetHash() & GetMaskIndex();
}
BitBoard GetKeyTest()
{
   return gsHashTable.mbbHashKeys[ 1 ][ 16 ];
}