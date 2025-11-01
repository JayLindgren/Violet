//
// Program Violet.
// Please tie your hair up in a ribbon before playing.
//
// This program was written by Jay Lindgren starting 9/2/2003
//
// The purpose and goals of this program are as follows.
// 1) Have fun
// 2) Write a program that is better than I am at chess - done
// 3) Keep it simple
// 4) Enjoy on boards and trees and searches
//
// 4/7/2006
// 5) Do chicks like chess programs?
//
// 8/5/2006
// 6) Changed the name of the program from DrFaust to Violet
//
// 8/26/2006
// On a cold and cloudy day in August where the high reached only 70
// and I slept under my heaveast blanket at night, Violet played her
// first game.  She won, giving checkmate with a combination of four
// pieces.
//
// A new goal.  It costs about $18 to play in the computer chess world
// championship.  I would like to play and draw one game.
//
// 10/15/2006
// The End is out and Team Violet proved they are true Baudelaires
// Working on the hash table.
//
// 10/31/2006 - 11/01/2006
// Trick-Or-Treating was a great success.  The kids got complete bags full,
// thanks to some power TOT in the car.  I read The End to them at the library
// and really enjoyed doing it.  Chris has shown me how to use malloc and free
// the hash table is coming along nicely.
//
// 11/12/2006
// A snowy day.  I have created www.VioletChess.com and VioletChess LLC.  Hopefully a tax write off
// I have also formed The Lindgren Group LLC, and Queequeg LLC.  The plan is for Queequeg to become
// a hedge fund or somthing like it.  It should not be a problem to have funding come into this
// company, as long as there are fewer than 15 investors.
//
// 12/7/2006
// Violet played Holly Myers at work over lunch time.  Violet won a convincing victory.
//
// 3/14/2007
// After taking a couple of months off, I'm back on the job.  It sure feels good to have my friend back.
// I'm working on getting my lcd projector or a large screen tv going in the condo to make working on
// it easier.  BTW, I had a good weekend at the Chalet this weekend.
//
// 4/3/2007
// I am on a role.  I'm starting a database for the all the candidates.  My theories of 
// objectification are really paying off.
//
// 9/24/2007
// I'm on an airplane flying to Houston with my new job working on making Violet into Deep Violet.
// I'm worried about getting enough time with my kids and how to make sure their homework gets
// done when I'm not in town.  Violet use two processors for the first time on this flight.
//
// 10/7/2007
// While on a flight to Houston, I was able to get the move ordering code finally wired up
// and Violet reached out further in plys than she ever has before.  She easily went out
// seven plys.  That will soon be a laugh.
//
// 12/7/2007
// Benched marked the idea of simply making a copy of the board structure rather and abandanoning it
// rather than undoing a move.  It cost almost 20 as much CPU time.
//
// 4/20/3008
// My mother died today.  I didn't morn.  It feels like a dark force has been lifted from the world.
// She died on Hitler's birthday...
//
// 4/30/2008
// Found the bug in the Q search that has been bugging me for a year and a half.
// For some reason, the code is hitting errors when using the multiprocessor version.
// Added the calculation for doubled and higher order pawns
//
// 5/4/2008
// Make great strides in cleaning up Violet.  I'm finally ready to focus on the hash table again.
// I'm also thinking about how to work on Violet full time.  I worry about putting the kids through
// college and probably shouldn't.  Life is short.
// 
// 5/27/2008
// The moves for the attacks only are not being calculated correctly.  The code needs a good cleaning for
// how the attacks are handeled.
//
// 6/24/2008
// The kids caught their first trout last weekend and we took it to Baur's to have Chef Jimmy cook them.
// Violet is finally playing well with a Q-search and I'm working on writing out moves in algebraic notation.
//
// 8/3/2008
// I'm waiting for a flight to the airport to head to Tulsa.  I have fixed a bug in the reported score and 
// seem to have checkmate under control.  I'll start a series of test next.  It was a good weekend with
// with sucessful playing, physics and chess.  It doesn't get any better.  The kids are growing, becomeing
// big and displaying behaviors according to their age.
//
// 10/7/2008
// After a break I'm back to work on my girl.  I'm trying fto fix the castling problem.
//
// 11/26/2008
// I'm flying to New Orelans with the kids.  They are very excited and we are expecting
// to eat quite a bit of food!  We are also going to try a swamp tour, a haunted tour
// and go see the cemetaries.
//
// 12/7/2008
// Flying to Tulsa working on finally getting the hash table set up and working.  Once
// the hash table the move sorting are working, Violet should be strong.  The kids and I
// are going to London for Christmas.  The economy sucks, but if you don't live life, what's
// the point.  Attack like there is no tomorrow.  Money is just a tool.
//
// 2/24/2009 - Tulsa
// Violet's hash table seems to be working.  She now has a brain!  Off to iterative deeping.
//
// 3/21/2010 - Denver
// After taking almost a year off I'm back.  I was rather sad about how bad I broke Violet
// while implementing iterative deepening.  I'm going to be more cautious this time.
// In the mean time I have purchased a scooter and a motorcycle.
//
// 6/9/2010 - Denver
// Got the first level search to use the hash table and it works well.  It really increased
// her depth.  Working on getting the full search to use the hash table.
//
// 8/30/2010 - Denver
// On the way to Houston.  Violet is playing very well.  Maybe time to write in the interface.
// Violet beats her first chess engine - MSCP (at 4 plys)
//
// 9/13/2010 - NYC
// Violet beats Arasan at 1 ply
//
// 9/15/2010 - Houston
// Added history historic to Violet
//
// 9/22/2010 - London
// Need to score move to search minor pieces capturing bigger pieces first.
// ICCA Vol. 9, No. 4, December 1986 P Bettadapur
//
// 9/23/2010 - London
// Good mobility and scoring in:
// ICCA Vol. 10, No. 1, March 1987 Hartmann
//
// 9/24/2010 - Train to Scotland
// Trying to fix castling bug.
//
// 12/12/2010 - Copper Mountain.  Sitting in the ski lodge with a sick Maxwell working on a memory leak.
//
// 12/28/2010 - Still getting rid of bugs by reading games for the opening book.
//
// 12/8/2011 - After a year, Violet is surrendering her bugs.  I'm in the basement of the Clock tower
// at Lannie's caberet working on Violet.
//
// 12/13/2011 - At DIA on my way to Houston to interview with Accenture.  Working on the interface.  Hoping to
// keep employeed :-)
//
// 12/28/2011 - At the Byers library, being a basketball dad for Morgan.  Working on the interface and cleaning
// up the functions that are needed for communicating.  Rewriting some of the routines for more times than I
// care to admit :-)  The overall design of violet is letting that happen pretty quickly.
//
// 1/5/2012 - Starting to lay the ground work for my interface code.  I now have a full beard and am growing my hair long on top.
// Tonight I'm going to one the performances at the Clock Tower.
//
// 1/26/2012 - I went to see Tinker, Tailor, Soldier, Spy.  I have an offer from Accenture and may resign from PA tomorrow.  
// The interface and game playing code is coming along.  Redoing the opening book too.
//
// 4/16/2012 - I have been stuck on the interface for awhile.  Damn the MicroSoft corporation.  Im going to try to rework the opening 
// book this week.  Loop over the games and enter the moves one at a time to avoid collisions.
//
// 4/27/2012 - Sitting in the Red Carpet club in NYC working on Violet.  The interface may finally be starting to surrender.  Damn Microsoft.
// I would like to thank the Accenture corporation for giving me the time to write this :-)
//
// 4/30/2012 - Dateline Denver - Some of my problems have been the MS corporation, but some have been mine :-)  I would be moving
// a lot faster with two pairs of eyes.  But you might be suprised how hard it is to get people intersted in chess programming.
//
// 5/2/2012 - Violet played a game of chess with me through a UCI Interface!!!  Internet, here she comes.
//
// 5/8/2012 - In Houston cleaning the code.  Need a girl friend.
//
// 5/21/2012 - Dateline Denver.  Played an engine match against Fritz.  Lost 60 games in a row.  Need to work on the evaluation a bit.
//             Trying to fix the deep bug tonight.
//
// 5/27/2012 - Dateline Canon City, CO.  The kids solved a murder mystery on a train while enjoying a gourmet meal.  Just another
//             weekend.  Violet gets her deep back.
//
// 5/29/2012 - Dateline Denver, CO.  Implemented draw and contempt of draw.
//
// 6/6/2012 - Dateline Denver, CO.  Working on my own Late Move Reduction.  Jack is spending a few summer days with me.
//
// 6/7/2012 - Dateline Denver, CO.  God bless LMR.  Violet just got some long arms.  Below are the number of nodes
//            required to search 6 ply deep from the starting position as white.
//             6 ply - no null    206781
//             6 ply - null       609752
//             6 ply - fixed hash 206781
//             6 ply - LMR         18533
//             6 ply - LMR 3 move  11745
//             6 ply - LMR sort     6371
//
// 12/22/2012 - Just took the Hattrick to see White Christmas.  Working on the PV routine.  A definate bug in there
//              some where.
//
// 12/30/2012 - In first class flying to NOLA for NYE.  Life is good.  Working on getting the PV through the interface.
//
// 1/1/2012 - Fixed the interface bug and stalemate.  Good times in NOLA.
//
// 1/5/2012 - Dateline: Byers.  Waiting for the kids to finish basketball practice.  Fixeed the PV and Stalemate issues.  Now starting on
//            revamping the evaluation routine.  Tonight the Denver Nuggest and tomorrow, jump street.
//
// 1/8/2012 - Violet played consistently on PlayChess.com.  She booked over 30 games.  She lost them all, but the play was consistent.
//
// 1/21/2012 - Found massive problems in th end game with LMR.  It needs a better search.
//
// 1/22/2012 - Actually this problem is just that Violet thinks stalemate is checkmate.  Silly girl.
//
// 1/25/2012 - Up in first, flying to KC for some bushwacking.
//             // First deep run nodes for 6 ply = 28005 after e4
//
// 1/27/2013 - Turned LMR back on.  Violet has some long arms. // First run, 6 ply 2038 after e4.
//
// 2/13/2013 - Use the alpha betas for all node when Violet is thinking on her time
//             Don't use that when Violet is thinking on the oponents time. - this preserves the Multi PVs.
//
// 3/13/2013 - On the way to New York City for a GARP conference.  Sitting in first, working on Violet.
//
// 4/1/2013 - Working in the ACN office while the condo is cleaned.  Trying to get this interface working with
//            mulitple input commands.  Thinking of taking a motorcycle trip to Santa Fe this weekend.  I want to check out
//            a civil war battle site.
//
// 4/23/2012 - strcpy is pure evil.  strncpy should be used.  In the third strait weeks of snow storms in April here in Denver.
//
// 7/14/2013 - Dateline: Wilmington, DE.  Fly in on a Sunday night.  I have a larege credit card client here.  Staying in the DoubleTree
//             and earning Hilton Points.  I went to Gettysburg on the 4th of July.
//
// 7/25/2013 - in First Class flying back from Wilmington to Denver.
//
// 8/4/2013 - Flying to Wilmington, DE.  Hat Trick went to a Rush concert with me.  Jack and Morgan fell asleep.  We were
//            up at 5:00am to volunteer at a homeless shelter.  Morgan went to running camp.  Max has a new shotgun and Ka Bar.
//            Jack is still cute :-)
//
// 8/18/2013 - Flyingto Wilmington, DE.  Morgan got my car this weekend. I went to an airshow.  The kids and I volunteered for three
//             hours at the Denver Rescue Mission.
//
// 9/2/2013 - Watch Max fun for yards and tackle many people on Friday night.  Went cowboy shooting with Jack and hew new Henry rifle.  Morgan
//            bought a prom dress.  I think I have fixed the LMR problem.
//
// 9/8/2013 - Upfront and flying to Columbus.  Violet is now move than 10 years old.
//
// 9/22/2013 - Upfront and flying to Columbus.  I watched Max play football Friday night in Rocky Ford.  He played running back, defendive safety
//             and quarterback.  I spend the night in Pueblo on the way back.  A fun city, I call if Victorian Beat.  Went to the local races 
//             Saturday night.  Currently (in flight) looking out the window at the Mississippi, on the East side of Iowa.
//
// 10/6/2013 - Starting the documentation of Violet while flying to Columbus.  Reorganizing some of the files.
//
// 10/27/2013 - Up front (first class) flying to Columbus, OH wworking on tuning the Null Move heuristic that I just got working.  Max had a football
//              game in Calahan this weekend that I rode my motorcycle to.  I got drive highway 24 from Colorado Springs to Limon which had been on 
//              my to do list for awhile.  An interesting and strange piece of rode.  Also rode the loop around Denver (E-470, C470) as much as possible
//              on the Moto Guzzi.
//
// 11/11/2013 - Veterans day.  I'm in the Denver airport flying out to Columbus. Having a Sunday night in Denver felt great!  Tonight I think I
//              got the null move working.  Violet is better than ever.
//
// 11/17/2013 - In Denver in the United lounge as a flight is delayed.  LMR and NMH are working well.  I can use them as agressively as I would like,
//              but she has greatly improved.  I'm going to clean up a bug in the interface and then revisit the hash table.  I'm thinking of
//              getting the kids giant sequoia trees for Christmas.
//
// 11/21/2013 - Waiting for Max to have surgery on his finger.  The hash table is finally looking promising.
//
// 12/5/2013 - Stranded in Columbus, It is -15 degrees in Denver. My plane needed a new tire, they had to get the wheel from Cincinatti.  I may be here
//             over night.
// 12/17/2013 - After several years, I finally got a bug out of the alpha beta search.  In the main search routine, I was return iScore instead of iBeta.
//
// 4/8/2014 - Irvine, CA, On the client site and still working on the Null Move.  It will surrender.  Or I might die before I get it right :-)
//
// 4/10/2014 - Irvine, CA, I can see a B-17 flying fortress from my hotel room on the tarmac of the John Wayne airport.
//
// 11/11/2014 - Irvine, CA, Armistice day.  WWI is a hundred years old, am trying follow historical progress since August.  Death Valley on the GSA last night.  Tonight
//              I got the check mate bug out of Violet, only to have her produce a "Deep" issue.  But progress none the less.
//
// 7/18/2015 - Irvine, CA, Fixes what might be causing Violet to "dissappear" once she finds checkmate.  A simple probelem.
//             Morgan starts school in two weeks.  I wrote my first tuition check yesterday.  Thinking of getting a second replacement scooter.
//             Feeling the need to get the GSA out and open her up, confined to the coast in the California summer.  Fear and loathing in Irvine.
//
// 7/31/2015 - Up in first flying home to say good bye to Morganas she leaves for college.  Fixed the computer opening move bug.
//             need to recalculate the book.
//
// 8/2/2015 - Met with Max and Morgan.  Let Morgan go to College.  Now sitting next to Neal Patrick Harris in first 
//            and working on Violet.  He is very skinny.
//
// 8/23/2015 - My friend Manesh got fired on Friday.  He really helped me a year ago and I'm sad to see him go.
//             He bought a Ducati and is constantly having it repaired and asking to store his helmet in my Vespa, lesson learned.
//             I'm getting close to hunting down the bug in the opening book move finder.
//
// 9/1/2015 - I lead the discussion on Saint Augustines, "The City of God" at the Irvine Great Books Reading Group.  I
//            really don't like him.  I got a cheap version of the Younger Brother Wait working tonight.
//
// 9/28/2015 - I have an implementation of the Null Move that seems to be working.  I based it on the TSCP program's implementation with many improvements
//             from other ICCA papers.  I'm in the process of tuning.  Work is having some volatility, high risk, low profits.  I would love to
//             get back to Denver! - Nope, this doesn't work.
//
// 5/16/2016 - Nega Scout and Aspiration search are paying big dividends.  Much more so than the null search.
//             I continue my horological studies.  My role at work has switched to market risk, which is interesting, but I have no staff.
//             I pine for the Holy city of Denver.
//
// 6/16/2016 - I'm back at work after a vacation motorcycle ride.  I have a picture of my bike in Times Square.  I held the sword of Joshua Chamberlain.
//             I saw the burrial site of the left arm of Stonewall Jackson.  I might be getting a little old for such hard riding.
//             I'm working on fixing a new version of the "deep" code and seem to have problems when the program stops because of time.
//
// 6/28/2016   Move to generating moves on command. pawns knights bishops rooks queen king
//             Generation on command didn't work, because the PV move (the one to be searched) can't be found unless all of the moves are generated.
//             Even the bisected production is suspect, but seems to work ok.  I couldn't detect a speed up, but I'm keeping it anyway.
//             On another note, I'm noticing that as I age, I'm becoming more grumpy.  I like it.
//
// 10/3/2016   I have parted with with Hyuyndai and feel very free.  I'm hoping Violet gets the love she so dearly deserves.
//
// 10/19 2017  I have e-mail from Germany telling of some bugs in Violet.  It's nice to get some feed back.  My job at Wells has been keeping me programming which
//             means poor Violet has not been getting the love.  I can't tell you how good it feels to be back with her.
//
// 12/31/2017  The Hat Trick and friends are in town.  We are having a very fun NYE party with Dominos and hot chocolate.  I just got my swagger back.
//
//
// 10/8/2025  Firing Violet back up.  Using AI this time.  I'm shocked at how it recommended replacing some of my well developed bitboard funcions
//            with some simple MicroSoft functions.  It kind of breaks my heart, but she is faster.  The AI is doing well at improving the code and finding bugs.
//
// 10/11/2025 At Sun Motorsports in Denver getting a new rear tire on the Honda Super Cub.  Having quite a few AI diasters.  Going to try to solve smaller problems.
//



// Violet ToDo:
//
// Change the time stopped move to only use the previous plys results for now.
// See if aI can make the search.cpp deep better than omp
// Replace raw malloc in InitializeHashTable and OpeningBook.cpp with std::vector<BitBoard>. This handles memory allocation and deallocation automatically, preventing leaks.
// Real deep - This is probably top priority.
// Calculate attacks only, then, if no cut off is found, calculate the regular moves.
// Counter-move heuristic
// Generate non-capturing moves
// update null search to incorporate killer moves/HH and other effects from the literature.
// Moving to storing moves in integers would allow for much faster copying of boards <-----
// futility puuning
// Implement table bases.
// Work on move ordering emperically
// Work on move ordering from other codes
// Write time controls.
// Allow thinking on oponents time - ponder
// PGO (Profile Guided Optimization)
// Try ripping out piece tables.
//
