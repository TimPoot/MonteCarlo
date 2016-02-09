
//
// C++ program to play simple chess:
// white king (WK) and white queen (WQ) against black king (BK)
// Compile: g++ -Wall -O2 -o chess chess.cc
// Run:     ./chess <thesize> <simulations> <rounds>
//
// Walter Kosters, January 25, 2016; w.a.kosters@liacs.leidenuniv.nl
//
// For Monte Carlo add the following two member functions:
//   int randomplayout ( ), 10 lines
//     starting with black, play a random game from the current situation
//     (looks like playagame (...)), of length at most 20
//   void whiteMCmove ( ), 50 lines
//     looks like randomwhitemove ( ), but now do all possible moves
//     on a fresh copy of the board (use *this) and select the one where
//     for 100 random playouts the most wins did occur;
//     remember to restore the old situation 
// And add a function int playaMCgame (int somesize, int rounds), 10 lines,
// that resembles playagame (...), but now white uses whiteMCmove ( );
// and of course use this function in main
//


#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

const int MAX = 30;

class Board {
  public:
    char A[MAX][MAX];    // game board (pretty useless)
    int thesize;         // height = width of game board
    int xBK, yBK;        // position of black king
    int xWK, yWK;        // position of white king
    int xWQ, yWQ;        // position of white queen (or rook)
    bool queencaptured;  // is white queen captured?
    int countmoves;      // number of moves so far
    // member functions, see below for comments
    Board ( );
    Board (int somesize);
    void print ( );
    bool legalforblackking (int i, int j);
    bool legalforwhiteking (int i, int j);
    bool canwhitequeenreach (int i, int j);
    bool legalforwhitequeen (int i, int j);
    bool incheck (int i, int j);
    bool checkmate ( );
    int numberofblackmoves ( );
    int numberofwhitequeenmoves ( );
    int numberofwhitekingmoves ( );
    int randomblackmove ( );
    void randomwhitemove ( );
    void humanwhitemove ( );
    int randomplayout ( );
    void whiteMCmove ( );
    void whitetactmove ( );
    double rate (bool qMovef);
};//Board

// get first non-enter
char getfirstchar ( ) {
  char c = cin.get ( );
  while ( c == '\n' )
    c = cin.get ( );
  return c;
}//getfirstchar

// default constructor
Board::Board ( ) {

}//Board::Board

// constructor
Board::Board (int somesize) {
  thesize = somesize;
  int i, j;
  for ( i = 0; i < MAX; i++ )
    for ( j = 0; j < MAX; j++ )
      if ( 0 < i && i <= thesize && 0 < j && j <= thesize )
        A[i][j] = '.';
      else
        A[i][j] = 'O'; // outside board
  xWK = rand ( ) % thesize + 1;
  yWK = rand ( ) % thesize + 1;
  do {
    xBK = rand ( ) % thesize + 1;
    yBK = rand ( ) % thesize + 1;
  } while ( xBK - xWK <= 1 && xWK - xBK <= 1 
            && yBK - yWK <= 1 && yWK - yBK <= 1 );
  do {
    xWQ = rand ( ) % thesize + 1;
    yWQ = rand ( ) % thesize + 1;
  } while ( ( xWQ == xWK && yWQ == yWK ) 
            || incheck (xBK,yBK) || numberofblackmoves ( ) == 0 );
  queencaptured = false;
  countmoves = 0;
}//Board::Board

// can BK move from (xBK,yBK) to (i,j)?
bool Board::legalforblackking (int i, int j) {
  if ( i <= 0 || j <= 0 || i > thesize || j > thesize )
    return false;
  if ( i == xBK && j == yBK )
    return false;
  if ( i > xBK+1 || i < xBK-1 || j > yBK+1 || j < yBK-1 )
    return false;
  if ( i - xWK <= 1 && xWK - i <= 1 && j - yWK <= 1 && yWK - j <= 1 )
    return false;
  if ( i == xWQ && j == yWQ )
    return true;  // king captures queen
  if ( incheck (i,j) )
    return false;
  return true;
}//legalforblackking

// can WK move from (xWK,yWK) to (i,j)?
bool Board::legalforwhiteking (int i, int j) {
  if ( i <= 0 || j <= 0 || i > thesize || j > thesize )
    return false;
  if ( i == xWK && j == yWK )
    return false;
  if ( i > xWK+1 || i < xWK-1 || j > yWK+1 || j < yWK-1 )
    return false;
  if ( i - xBK <= 1 && xBK - i <= 1 && j - yBK <= 1 && yBK - j <= 1 )
    return false;
  if ( i == xWQ && j == yWQ )
    return false;
  return true;
}//legalforwhiteking

// can WQ reach (i,j) from (xWQ,yWQ)?
bool Board::canwhitequeenreach (int i, int j) {
  if ( i <= 0 || j <= 0 || i > thesize || j > thesize )
    return false;
  if ( i != xWQ && j != yWQ && i + j != xWQ + yWQ && i - j != xWQ - yWQ )
    return false;
  if ( i == xWQ && i == xWK 
       && ( ( yWQ < yWK && yWK < j ) || ( j < yWK && yWK < yWQ ) ) )
    return false;
  if ( j == yWQ && j == yWK 
       && ( ( xWQ < xWK && xWK < i ) || ( i < xWK && xWK < xWQ ) ) )
    return false;
  if ( i + j == xWQ + yWQ && i + j == xWK + yWK 
       && ( ( xWQ - yWQ < xWK - yWK && xWK - yWK < i - j ) 
            || ( i - j < xWK - yWK && xWK - yWK < xWQ - yWQ ) ) )
    return false;
  if ( i - j == xWQ - yWQ && i - j == xWK - yWK 
       && ( ( xWQ + yWQ < xWK + yWK && xWK + yWK < i + j ) 
            || ( i + j < xWK + yWK && xWK + yWK < xWQ + yWQ ) ) )
    return false;
  return true;
}//canwhitequeenrach

// can WQ move from (xWQ,yWQ) to (i,j)?
bool Board::legalforwhitequeen (int i, int j) {
  if ( i <= 0 || j <= 0 || i > thesize || j > thesize )
    return false;
  if ( i == xWQ && j == yWQ )
    return false;
  if ( i == xWK && j == yWK )
    return false;
  return canwhitequeenreach (i,j);
}//legalforwhitequeen

// is BK at (i,j) in check?
bool Board::incheck (int i, int j) {
  return canwhitequeenreach (i,j);
}//Board::incheck

// black cannot move and is in check
bool Board::checkmate ( ) {
  return ( numberofblackmoves ( ) == 0 && incheck (xBK,yBK) );
}//checkmate

// how many moves are available for BK?
int Board::numberofblackmoves ( ) {
  int howmany = 0;
  int i, j;
  for ( i = -1; i <= 1; i++ )
    for ( j = -1; j <= 1; j++ )
      howmany += legalforblackking (xBK+i,yBK+j);
  return howmany;
}//Board::numberofblackmoves

// how many moves are available for WK?
int Board::numberofwhitekingmoves ( ) {
  int howmany = 0;
  int i, j;
  for ( i = -1; i <= 1; i++ )
    for ( j = -1; j <= 1; j++ )
      howmany += legalforwhiteking (xWK+i,yWK+j);
  return howmany;
}//Board::numberofwhitekingmoves

// how many moves are available for WQ?
int Board::numberofwhitequeenmoves ( ) {
  int howmany = 0;
  int i, j;
  for ( i = 1; i <= thesize; i++ )
    for ( j = 1; j <= thesize; j++ )
      howmany += legalforwhitequeen (i,j);
  return howmany;
}//Board::numberofwhitequeenmoves

// do a random move for White
void Board::randomwhitemove ( ) {
  int move, i, j;
  int numberK = numberofwhitekingmoves ( );
  int numberQ = numberofwhitequeenmoves ( );
  move = rand ( ) % ( numberK + numberQ );
  if ( move < numberK )
    for ( i = -1; i <= 1; i++ )
      for ( j = -1; j <= 1; j++ )
        if ( legalforwhiteking (xWK+i,yWK+j) ) {
          if ( move == 0 ) {
            xWK = xWK+i; yWK = yWK+j;
	    countmoves++;
	    return;
	  }//if
	  move--;
        }//if
  move -= numberK;
  for ( i = 1; i <= thesize; i++ )
    for ( j = 1; j <= thesize; j++ )
        if ( legalforwhitequeen (i,j) ) {
          if ( move == 0 ) {
            xWQ = i; yWQ = j;
	    countmoves++;
	    return;
	  }//if
	  move--;
        }//if
}//Board::randomwhitemove

// do a random move for BK
// return 0 if checkmate, 1 if stalemate, 2 if queen captured,
// 3 otherwise
int Board::randomblackmove ( ) {
  int move, i, j;
  int number = numberofblackmoves ( );
  if ( number == 0 ) {
    if ( incheck (xBK,yBK) )
      return 0;
    else
      return 1;
  }//if
  move = rand ( ) % number;
  for ( i = -1; i <= 1; i++ )
    for ( j = -1; j <= 1; j++ )
      if ( legalforblackking (xBK+i,yBK+j) ) {
        if ( move == 0 ) {
          xBK = xBK+i; yBK = yBK+j;
	        if ( xBK == xWQ && yBK == yWQ ) {
	           queencaptured = true;
	           return 2;
	         }//if
        return 3;
	     }//if
	     move--;
      }//if
  return 999;
}//Board::randomblackmove

// do a move for White: human player
void Board::humanwhitemove ( ) {
  char choice;
  bool OK = false;
  int i, j;
  do {
    print ( );
    cout << "A (white) move please (Q or K) .. ";
    choice = getfirstchar ( );
    if ( choice == 'q' || choice == 'Q' ) {
      cout << "Row number for queen (1..)     .. ";
      i = getfirstchar ( ) - '0';
      cout << "Column number for queen (1..)  .. ";
      j = getfirstchar ( ) - '0';
      if ( legalforwhitequeen (i,j) ) {
        xWQ = i; yWQ = j;
	OK = true;
      }//if
      else {
        cout << "Illegal queen move ..." << endl;
      }//else
    }//if
    else {
      cout << "Row number for king (1..)      .. ";
      i = getfirstchar ( ) - '0';
      cout << "Column number for king (1..)   .. ";
      j = getfirstchar ( ) - '0';
      if ( legalforwhiteking (i,j) ) {
        xWK = i; yWK = j;
	OK = true;
      }//if
      else {
        cout << "Illegal king move ..." << endl;
      }//else
    }//if
  } while ( ! OK );
  countmoves++;
}//Board::humanwhitemove

// print board
void Board::print ( ) {
  int i, j;
  A[xBK][yBK] = 'z';
  A[xWK][yWK] = 'K';
  if ( ! queencaptured )
    A[xWQ][yWQ] = 'Q';
  cout << "  ";
  for ( j = 1; j <= thesize; j++ )
    cout << " " << j % 10;
  cout << endl;
  for ( i = 1; i <= thesize; i++ ) {
    cout << ( i < 10 ? " " : "" ) << i << " ";
    for ( j = 1; j <= thesize; j++ )
      cout << A[i][j] << " ";
    cout << endl;
  }//for
  A[xBK][yBK] = '.';
  A[xWK][yWK] = '.';
  A[xWQ][yWQ] = '.';
}//Board::print

double Board::rate(bool qMove) {
  if (checkmate())
    return 100000000;
  if (legalforblackking(xWQ, yWQ)) 
    return -9999999;
  if (numberofblackmoves() == 0 && !incheck(xBK, yBK))
    return -77777777;

  //Maak rechthoek BK kleiner met WQ
  //eerste kwadrant
  if (xBK > xWQ && yBK < yWQ && qMove) { 
   return 3000 / ((thesize - xWQ) * yWQ);
  }
  //tweede kwadrant
  if (xBK < xWQ && yBK < yWQ && qMove) {
    return 3000 / (xWQ * yWQ);
  }
  //derde kwadrant
  if (xBK < xWQ && yBK > yWQ && qMove) {
    return 3000 / (xWQ * (thesize - yWQ));
  }
  //vierde kwadrant
  if (xBK > xWQ && yBK > yWQ && qMove) {
    return 3000 / ((thesize - xWQ) * (thesize - yWQ));
  }

  //zet WK zo dicht mogeljik bij BK
  //eerste kwadrant
  if (xBK > xWK && yBK < yWK && !qMove) { 
   return 1000 / ((xBK - xWK) + (yWK - yBK));
  }
  //tweede kwadrant
  if (xBK < xWK && yBK < yWK && !qMove) {
    return 1000 / ((xWK - xBK) + (yWK - yBK));
  }
  //derde kwadrant
  if (xBK < xWK && yBK > yWK && !qMove) {
    return 1000 / ((xWK - xBK) + (yBK - yWK));
  }
  //vierde kwadrant
  if (xBK < xWK && yBK > yWK && !qMove) {
    return 1000 / ((xWK - xBK) + (yBK - yWK));
  }
  

  return -1;
}




void Board::whitetactmove () {
  int score, i, j, k, l;
  int bestmovesofar = -1;
  int bestscoresofar = -1000;
  int numberK = numberofwhitekingmoves ( );
  int numberQ = numberofwhitequeenmoves ( );
  int move = numberQ + numberK;
  Board kopie;
  for ( i = -1; i <= 1; i++ ){
    for ( j = -1; j <= 1; j++ ){
      if ( legalforwhiteking (xWK+i,yWK+j) ) {
        score = 0;
        kopie = *this;
        kopie.xWK = kopie.xWK+i; kopie.yWK = kopie.yWK+j;
        //scoring
        score = (int)kopie.rate(false);
        
        if (score > bestscoresofar){
          bestscoresofar = score;
          bestmovesofar = move;
        }
	      move--;
      }//if
    }
  }
  for ( i = 1; i <= thesize; i++ ){
    for ( j = 1; j <= thesize; j++ ){
      if ( legalforwhitequeen (i,j) ) {
        score = 0;
        kopie = *this; 
        kopie.xWQ = i; kopie.yWQ = j;
        //scoring
        score = (int)kopie.rate(true);
        if (score > bestscoresofar){
          bestscoresofar = score;
          bestmovesofar = move;
        }
	    move--;
      }//if
    }
  }
  cout << "move: " << bestmovesofar << endl;
  cout << "score: " << bestscoresofar << endl;
  move = numberK + numberQ;
  for ( i = -1; i <= 1; i++ ){
    for ( j = -1; j <= 1; j++ ){
      if ( legalforwhiteking (xWK+i,yWK+j) ) {
        if (move == bestmovesofar){
          xWK = xWK+i; yWK = yWK+j;
          countmoves++;
          return;
        }
      move--;
      }
    }     
  }
  for ( i = 1; i <= thesize; i++ ){
    for ( j = 1; j <= thesize; j++ ){
      if ( legalforwhitequeen (i,j) ) {    
        if (move == bestmovesofar){
          xWQ = i; yWQ = j;
          countmoves++;
          return;
        }
        move--;
      }
    }
  }
}

void Board::whiteMCmove () {
  int score, i, j, k;
  int bestmovesofar = -1;
  int bestscoresofar = -1;
  int numberK = numberofwhitekingmoves ( );
  int numberQ = numberofwhitequeenmoves ( );
  int move = numberQ + numberK;
  Board kopie;
  for ( i = -1; i <= 1; i++ ){
    for ( j = -1; j <= 1; j++ ){
      if ( legalforwhiteking (xWK+i,yWK+j) ) {
        score = 0;
        for (k = 0; k < 100; k++){
          kopie = *this;
          kopie.xWK = kopie.xWK+i; kopie.yWK = kopie.yWK+j;
          if (kopie.randomplayout() == 0){
            score++;
          }
        }
        if (score > bestscoresofar){
          bestscoresofar = score;
          bestmovesofar = move;
        }
	      move--;
      }//if
    }
  }
  for ( i = 1; i <= thesize; i++ ){
    for ( j = 1; j <= thesize; j++ ){
      if ( legalforwhitequeen (i,j) ) {
        score = 0;
        for (k = 0; k < 100; k++){
          kopie = *this; 
          kopie.xWQ = i; kopie.yWQ = j;
          if (kopie.randomplayout() == 0){
            score++;
          }
        }
        if (score > bestscoresofar){
          bestscoresofar = score;
          bestmovesofar = move;
        }
	    move--;
      }//if
    }
  }
  move = numberK + numberQ;
  for ( i = -1; i <= 1; i++ ){
    for ( j = -1; j <= 1; j++ ){
      if ( legalforwhiteking (xWK+i,yWK+j) ) {
        if (move == bestmovesofar){
          xWK = xWK+i; yWK = yWK+j;
          countmoves++;
          return;
        }
      move--;
      }
    }     
  }
  for ( i = 1; i <= thesize; i++ ){
    for ( j = 1; j <= thesize; j++ ){
      if ( legalforwhitequeen (i,j) ) {    
        if (move == bestmovesofar){
          xWQ = i; yWQ = j;
          countmoves++;
          return;
        }
        move--;
      }
    }
  }
}

// play a random game
// return 0 if checkmate, 1 if stalemate, 2 if simple tie,
// 3 if stopped
int Board::randomplayout(){
  int i= 0;
  int themove = 3;
  while ( themove == 3 && i < 20 ) {
    themove = randomblackmove ( ); 
    if (themove == 3) {
      randomwhitemove ( );
    }
    i++;
  }//while
  return themove;
}

// play one game
// return 0 if checkmate, 1 if stalemate, 2 if simple tie,
// 3 if stopped
int playagame (int somesize, int rounds) {
  Board board (somesize);
  int themove = 3;
  while ( themove == 3 && board.countmoves < rounds ) {
    board.randomwhitemove ( );
    themove = board.randomblackmove ( ); 
  }//while
  return themove;
}//playagame

// play one game
// return 0 if checkmate, 1 if stalemate, 2 if simple tie,
// 3 if stopped
int playaMCgame (int somesize, int rounds) {
  Board board (somesize);
  int themove = 3; 
  while ( themove == 3 && board.countmoves < rounds ) {
    board.whiteMCmove();
    themove = board.randomblackmove();
  }
  //cout << "result: " << themove << endl;
  return themove;
}//playaMCgame


// play one game
// return 0 if checkmate, 1 if stalemate, 2 if simple tie,
// 3 if stopped
int playatactgame (int somesize, int rounds) {
  Board board (somesize);
  board.print();
  cout << "---------------------" << endl;
  int themove = 3;
  while ( themove == 3 && board.countmoves < rounds ) {
    board.whitetactmove ( );
    themove = board.randomblackmove ( );
    board.print(); 
  }//while
  return themove;
}//playatactgame


int main (int argc, char* argv[ ]) {
  int i;
  int stats[4];
  int somesize;
  int simulations;
  int rounds;
  if ( argc >= 4 ) {
    somesize = atoi (argv[1]);
    if ( somesize > MAX - 2 )
      somesize = MAX - 2;
    if ( somesize <= 4 )
      somesize = 5;
    simulations = atoi (argv[2]);
    rounds = atoi (argv[3]);
  }//if
  else {
    somesize = 8;
    simulations = 1;
    rounds = 20;
  }//else
  srand (time(NULL));  // seed random generator
  stats[0] = stats[1] = stats[2] = stats[3] = 0;
  for ( i = 0; i < simulations; i++ ){
     //cout << "simulation: " << i << endl;
     //stats[playagame (somesize,rounds)]++;
     //stats[playaMCgame (somesize, rounds)]++;
     stats[playatactgame (somesize, rounds)]++;
  }
  cout << "Board size:  " << somesize << endl
       << "Rounds:      " << rounds << endl
       << "Wins:        " << stats[0] << endl
       << "Stalemates:  " << stats[1] << endl
       << "Simple ties: " << stats[2] << endl
       << "Stopped:     " << stats[3] << endl
       << "Total:       " << simulations << endl;
  return 0;
}//main

