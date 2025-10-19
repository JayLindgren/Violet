![Violet logo](assets/images/violet-logo.png)
# Violet
The Violet chess engine

Violet Overview and Language

Violet is a program written by Dr. Jay Lindgren starting on 9/2/2003.  The reason for undertaking such an endeavor is that it is a cheap and entertaining hobby that can be engaged while traveling for business or wait for children at activities.  I am writing this introduction on what is about the 10th anniversary of my starting to write Violet.  A few tidbits:
I have never beaten Violet
Violet has played people and computers from all over the planet
Her biggest fans are in Germany
I have read over 4,000 pages off technical documents researching her
As a child of the Cold War, I hate to admit it, but the Soviet research into bit boards during the 1960’s was breath taking
Violet is named after the heroin of the Lemmony Snicket novels, “A Series of Unfortunate Events” as she reminded me of my daughter Morgan
When I watch her play games, I frequently can’t tell if she is winning or losing
My relationship with Violet has lasted longer than any romantic relationship I have had with a woman
Violet doesn’t talk either
This documentation is not meant to be exhaustive, but to jog my memory as needed.
Violet is written in C++ but she looks a lot like regular C.  There are rumors that all of the deferenceing of using objects would slow down the program, but  there are even more rumors that it doesn’t.  At this point, I don’t have any value in developing the objects, but I might someday if I get time.
I have tried to consistently use Hungarian notation in my variable names.  An example is argiCounter, is a variable passed into the function that is an integer variable that is a counter.  My notation is below.
i – Ingeter
s – structure
arg – any variables passed in as an argument to subroutine.
bb – BitBoard – a variable with 64 integers defined explicitly in Violet
v – vector or array
m – matrix
g – a global variable (I have tried to limit the use of these)
si – short integer
Function names are capitalized
d – a defined variable (see the header  file “Definitions.h” for most of them except for those defined in the evaluation files

Data structures

One of the most important decisions for a chess program is the data structure.
Board
The board structure relies heavily on the BitBoard construction as well as such old fashioned things as an  array with all of the pieces on it. Most of these variables should be self-evident.
 It also contains a history stack which are all of the moves the board has made to reach its current state.  It also contains the current Principal Variation matrix, which is a matrix of moves.
General Moves	

Violet has a data structure called sGeneralMoves.  Strategically, sGeneralMoves contains all of the precalculations that can be performed for moves.  For example, for a given square, sGeneralMoves will have all of the square to which a knight can move.  The same would go for the other pieces.  These are used extensively for calculating the moves for sliding pieces.
sGeneralMoves also has handy tools, such as BitBoards to calculate if a rook is on the 7th rank and so forth.
I have considered making sGeneralMoves a global variables as it is probably passed around hundreds of times.  But alas, here we are.
Moves
There is move structure that contains things such as the “from square”, the “to square”, a BitBoard to contain the En Passant square, castle info (is it a castle), piece type, is it a capture, a promotion, etc.
Hash Table

The hash table structure hold the hash table or transposition table.  It contains the largest of the data fields and also handles the hash keys for the Zobrist hasing (see hash table section).
Opening Book
The opening book structure simply holds a hash table.
Evaluator
The evaluator structure holds many of the key scoring items such as the piece position value tables.  The definitions for the values that go into the tables are defined at the start of the “Evaluator.cpp” file.
Utilities

There are several utilities spread throughout Violet. Some of them warrant some additional comments.  These include the count and find routines which operate on BitBoards.
Count
Count is used to count how many of the 64 fields are set to one.  The routine uses a trick from “Hacker’s Delight” by Hank Warren.
Find
The find routine uses a binary search structure to find a single bit on a bit board.  The routine is blown out for a full search in the code.

Move Generation

As much precalculation for move generation as can be done is and is stored in sGeneralMoves.  See the structure section for more details.
Rotated Bitboards was considered for Violet, but they didn’t seem to add anything and were a bit of a pain, so I used the Slider routine extensively.
Magic bitboards might be an idea for knight moves in the future, but I’m not very interested in them right now.
Attacks

Evaluation

Violet has two basic sets up evaluations. one for the opening and midgame and the second for the end game.  A phase of game state variable is used to smoothly transition from one to the other.
The evaluation function has table piece valuation as well as mobility valuation.  While Fruit claims to play great with only the mobility, Violet needs the piece tables.
This is definitely one of the greatest theoretical holes in Violet.  There has been to science around how I have assigned the value to the tables.  I have closely followed some other programs (not copying of course) and added some ideas of my own.  But in the end, it’s a SWAG.
Some other feathers in the valuation include:
Penalty for doubled pawns
Penalty for tripled pawns
King safety
Pieces lining up on oponents king
Protected pieces [NTD: check this and implement if not]
Pawn structure
Search

Quisessence search

Things that don't work

I have tried many programming ideas for Violet that just did not bear fruit.  I have tested, stepped, worked and reworked these potential enhancements and they just slowed the program down or did nothing.
Null Move

Violet has a verified Null Move approach.  The null move is made give certain conditions and then if a cutoff is found it is researched to see if we are in Zugschwein.
The heuristic seems to neither hurt nor help.  In my measurements, it seems to have slowed down the program by increasing the amount of notes searched by 1%.  That being the case I just left it in.
There are potentially some reasons as to why it might not work which I will document here.
The evaluation function is such that it is not friendly to the NMH
LMR may interact.  Although I have turned LMR off and tried NMH, it just didn’t work
I need to do more research to improve my chess intuition as to when it should cut off and make sure that it does
Hash Table

The hash table has been most heart breaking in it’s not helping.  It slows the program down by about 20% rather speeding it up.  
It should be noted that the use of this technology will depend on the speed of the CPU’s and the threads. 
CurrentlyViolet replaces the entry whenever it comes to a new entry.


Deep

Violet uses OpenMP for optimization.
At the time of this writing, there are bugs in the interface and the counter that need to be ironed out.
With the Developer Studio 2005 that I am currently using, it cannot optimizing the compiling and use OpenMP.  The benefit of Open MP was stronger than the pick up from the compiler.  Last checked Dec. 2012.
Complier Optimization

Time Control

LMR

Late Move Reduction was a great improvement over depth and the best feather added to date.  Violet score all moves and forces full searches for moves such as captuers, check, etc.  She also forces at least one move to be searched to full depth.  The other moves are searched to a lower horizon, typically three ply shorter.  If one of these moves causes, a cut off, it is researched to full depth.
Because this controls the exponential branching of the tree, if Violet were a boxer, she would have very long arms.
Game Control

Most of the game control parameters and variables are stored in a structure called “Tempus”, a name I liked from the Latin “Tempus Fugit”.  In hind sight, it doesn’t really fit, but the name stuck for game and time controls.
Interface notes

Opening book

Violet has an opening book created from her reading and summarizing several hundred thousand games.  The routines for this is found in the file “OpeningBook.C”.
The opening book is stored in the hash table.
Most of the games came from a “Mentor” website that had the games stored and freely accessible.
Violet reads the first move from every game in the book, summarizes the statistics, and then goes back over all of the second moves and summarizes the statistics.  This is the long way to do it, but it was the only way I could find to give good statistics.
The parameters for when to just out of the book is in “define.h”.  For example, stop using the book when white only has a 40% chance historically of winning from this positions.  Note, a typical statistic might be White wins 40%, Black wins 40%, Drawn 20%.
I have not read the opening book, nor do I have the chess skills to do a good job of analyzing it.  I just hope I hop out of the lines soon enough.
Table Bases

Test suites

There are several suites of chess problems that Violet can run over.  Testing with and without new features allows the following:
Is there something horribly wrong with a new feature?
Did the program improve?
The improvement in the scores give an indication of the improvement in the score.
There are several things to consider.  
Many of these suites were developed by humans and the solutions can be suspect
Some of these solutions may change as our knowledge of chess improves.
I have been shocked at how bad Violet has been in solving some of these suites.  
Suites that have been worked covered include:
EPD
Arasan 15
New WAC (Win at Chess)
BT 2630
La Puce 2
IQ 4
PET
EET
