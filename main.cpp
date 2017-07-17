#include <iostream>
#include <deque>
#include <tuple>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <climits>
#include <algorithm>
#include "crow_all.h"

using namespace std;

struct node {
	int alpha;
	int beta;
	std::vector<node*> children;

	node(int a, int b) :
		alpha(a), beta(b) {}

};

int XX = -1;
int YY = -1;

char board[8][8] = {
	{ 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E' },
	{ 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E' },
	{ 'E', 'E', 'B', 'E', 'E', 'B', 'E', 'E' },
	{ 'E', 'E', 'E', 'B', 'B', 'B', 'E', 'W' },
	{ 'E', 'E', 'E', 'B', 'B', 'B', 'W', 'E' },
	{ 'E', 'E', 'E', 'W', 'W', 'W', 'E', 'E' },
	{ 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E' },
	{ 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E' } };


int CornerW = 128;
int WeakCornerW = -8;
int GoodEdgeW = 16;
int BadInW = -4;
int maxDeph = 7;

int Corners[] = { 0, 7, 56, 63 };
int WeakCorners[4][3] = { { 1,  8,  9 },
{ 6, 14, 15 },
{ 48, 49, 57 },
{ 54, 55, 62 } };

int GoodEdges[16] = { 2,  3,  4,  5,
58, 59, 60, 61,
23, 31, 39, 47,
16, 24, 32, 40 };

int BadIns[16] = { 10, 11, 12, 13,
50, 51, 52, 53,
22, 30, 38, 46,
17, 25, 33, 41 };

int ExtraLookAheadCode[64] = {
	1, 2, 1, 0,   0, 1, 2, 1,
	2, 3, 1, 1,   1, 1, 3, 2,
	1, 1, 0, 0,   0, 0, 1, 1,
	0, 1, 0, 0,   0, 0, 1, 0,

	0, 1, 0, 0,   0, 0, 1, 0,
	1, 1, 0, 0,   0, 0, 1, 1,
	2, 3, 1, 1,   1, 1, 3, 2,
	1, 2, 1, 0,   0, 1, 2, 1
};

int boardValue[8][8] = {
	{ 200, -50, 5, 30, 30, 5, -50, 200 },
	{ -50, -50,20, 20, 20, 20,-50, -50 },
	{ 70, 20,  20, 20, 20, 20,  20,  70 },
	{ 30 , 20,  20, 0, 0, 20,  20,  30 },
	{ 30 , 20,  20, 0, 0, 20,  20,  30 },
	{ 70 , 20,  20, 20, 20, 20,  20,  70 },
	{ -50,-50, 20, 20, 20, 20,-50, -50 },
	{ 200, -50, 70, 30, 30, 70, -50, 200 } };

int moveRow[] = { -1, -1, -1,  0,  0,  1,  1,  1 };
int moveCol[] = { -1,  0,  1, -1,  1, -1,  0,  1 };

int ExtraLookAhead(int x, int y, int Depth) {
	int Position = x * 7 + (y + x);
	int Code = ExtraLookAheadCode[Position];
	return min(6, max(0, Code - Depth));
}

std::vector<std::pair<int, int>>findValidMove(char piece);

int heuristic(char piece) {
	int my_tiles = 0, opp_tiles = 0, i, j, k, my_front_tiles = 0, opp_front_tiles = 0, x, y;
	double p = 0, c = 0, l = 0, m = 0, f = 0, d = 0;

	int X1[] = {-1, -1, 0, 1, 1, 1, 0, -1};
	int Y1[] = {0, 1, 1, 1, 0, -1, -1, -1};
	int V[8][8] =
		{ {20, -3, 11, 8, 8, 11, -3, 20},
    	 {-3, -7, -4, 1, 1, -4, -7, -3},
    	 {11, -4, 2, 2, 2, 2, -4, 11},
    	 {8, 1, 2, -3, -3, 2, 1, 8},
    	 {8, 1, 2, -3, -3, 2, 1, 8},
    	 {11, -4, 2, 2, 2, 2, -4, 11},
    	 {-3, -7, -4, 1, 1, -4, -7, -3},
    	 {20, -3, 11, 8, 8, 11, -3, 20}};

// Piece difference, frontier disks and disk squares
	for(i=0; i<8; i++)
		for(j=0; j<8; j++)  {
			if(board[i][j] == 'W')  {
				d += V[i][j];
				my_tiles++;
			} else if(board[i][j] == 'B')  {
				d -= V[i][j];
				opp_tiles++;
			}
			if(board[i][j] != 'E')   {
				for(k=0; k<8; k++)  {
					x = i + X1[k]; y = j + Y1[k];
					if(x >= 0 && x < 8 && y >= 0 && y < 8 && board[x][y] == 'E') {
						if(board[i][j] == 'W')  my_front_tiles++;
						else opp_front_tiles++;
						break;
					}
				}
			}
		}
	if(my_tiles > opp_tiles)
		p = (100.0 * my_tiles)/(my_tiles + opp_tiles);
	else if(my_tiles < opp_tiles)
		p = -(100.0 * opp_tiles)/(my_tiles + opp_tiles);
	else p = 0;

	if(my_front_tiles > opp_front_tiles)
		f = -(100.0 * my_front_tiles)/(my_front_tiles + opp_front_tiles);
	else if(my_front_tiles < opp_front_tiles)
		f = (100.0 * opp_front_tiles)/(my_front_tiles + opp_front_tiles);
	else f = 0;

// Corner occupancy
	my_tiles = opp_tiles = 0;
	if(board[0][0] == 'W') my_tiles++;
	else if(board[0][0] == 'B') opp_tiles++;
	if(board[0][7] == 'W') my_tiles++;
	else if(board[0][7] == 'B') opp_tiles++;
	if(board[7][0] == 'W') my_tiles++;
	else if(board[7][0] == 'B') opp_tiles++;
	if(board[7][7] == 'W') my_tiles++;
	else if(board[7][7] == 'B') opp_tiles++;
	c = 25 * (my_tiles - opp_tiles);

// Corner closeness
	my_tiles = opp_tiles = 0;
	if(board[0][0] == 'E')   {
		if(board[0][1] == 'W') my_tiles++;
		else if(board[0][1] == 'B') opp_tiles++;
		if(board[1][1] == 'W') my_tiles++;
		else if(board[1][1] == 'B') opp_tiles++;
		if(board[1][0] == 'W') my_tiles++;
		else if(board[1][0] == 'B') opp_tiles++;
	}
	if(board[0][7] == 'E')   {
		if(board[0][6] == 'W') my_tiles++;
		else if(board[0][6] == 'B') opp_tiles++;
		if(board[1][6] == 'W') my_tiles++;
		else if(board[1][6] == 'B') opp_tiles++;
		if(board[1][7] == 'W') my_tiles++;
		else if(board[1][7] == 'B') opp_tiles++;
	}
	if(board[7][0] == 'E')   {
		if(board[7][1] == 'W') my_tiles++;
		else if(board[7][1] == 'B') opp_tiles++;
		if(board[6][1] == 'W') my_tiles++;
		else if(board[6][1] == 'B') opp_tiles++;
		if(board[6][0] == 'W') my_tiles++;
		else if(board[6][0] == 'B') opp_tiles++;
	}
	if(board[7][7] == 'E')   {
		if(board[6][7] == 'W') my_tiles++;
		else if(board[6][7] == 'B') opp_tiles++;
		if(board[6][6] == 'W') my_tiles++;
		else if(board[6][6] == 'B') opp_tiles++;
		if(board[7][6] == 'W') my_tiles++;
		else if(board[7][6] == 'B') opp_tiles++;
	}
	l = -12.5 * (my_tiles - opp_tiles);

// Mobility
	my_tiles = findValidMove('W').size();
	opp_tiles = findValidMove('B').size();
	if(my_tiles > opp_tiles)
		m = (100.0 * my_tiles)/(my_tiles + opp_tiles);
	else if(my_tiles < opp_tiles)
		m = -(100.0 * opp_tiles)/(my_tiles + opp_tiles);
	else m = 0;

// final weighted score
	double score = (10 * p) + (801.724 * c) + (382.026 * l) + (78.922 * m) + (74.396 * f) + (10 * d);
	return score;

}



int heuristic1(int x, int y, char piece) {

	//  int black_c = 0;
	//  int white_c = 0;
	//  int myBoard[64];
	//
	//  int k = 0;
	//  for (int r = 0; r < 8; ++r)
	//  	for (int c = 0; c < 8; ++c) {
	//  		if (board[r][c] == 'B') {
	//  			++black_c;
	//  			myBoard[k++] = -1;
	//  		}
	//  		else
	//  			if (board[r][c] == 'W') {
	//  				++white_c;
	//  				myBoard[k++] = 1;
	//  			}
	//  			else
	//  				myBoard[k++] = 0;
	//  	}
	//
	// //cout << "b: " << black_c << " w: " << white_c << endl;
	//  int filled = white_c + black_c;
	//  int balance = white_c - black_c;
	//
	//  auto result = balance << (filled >> 4);
	//
	//  for (int i = 0; i < 4; i++) {
	//  	int space = myBoard[Corners[i]];
	//  	if (space != 0)
	//  		result += space * CornerW;
	//  	else {
	//  		for (int j = 0; j < 3; j++)
	//  			result += myBoard[WeakCorners[i][j]] * WeakCornerW;
	//  	}
	//  }
	//
	//  for (int i = 0; i < 16; i++)
	//  	result += myBoard[GoodEdges[i]] * GoodEdgeW + myBoard[BadIns[i]] * BadInW;



	int result = 0;

	for (int r = 0; r < 8; ++r) {
		for (int c = 0; c < 8; ++c) {
			if (board[r][c] == 'W')
				result += boardValue[r][c];
			 else if (board[r][c] == 'B')
			 	result -= boardValue[r][c];
		}
	}
	auto z = findValidMove(piece).size() * 5;
	if (piece == 'W')
		result += z;
	else
		result -= z;
	//result += findValidMove(piece).size();

	//cout << "result: " << result << endl;
	return result;

	//  int score = 0;
	//  char oppPiece = (piece == 'B') ? 'W' : 'B';
	//
	//  for (int r = 0; r < 8; ++r) {
	//  	for (int c = 0; c < 8; ++c) {
	//  		if (board[r][c] == piece)
	//  			score += boardValue[r][c];
	//  		//else if (board[r][c] == oppPiece)
	//  			//score -= boardValue[r][c];
	//  	}
	//  }
	//  score += findValidMove(piece).size();
	//  return score;
}


bool isValidMove(char piece, int x, int y) {
	if (board[x][y] != 'E')
		return false;

	char oppPiece = (piece == 'B') ? 'W' : 'B';
	bool valid = false;

	for (int i = 0; i < 8; ++i) {
		int curRow = x + moveRow[i];
		int curCol = y + moveCol[i];
		bool hasOppPieceBetween = false;
		while (curRow >= 0 && curRow < 8 && curCol >= 0 && curCol < 8) {
			if (board[curRow][curCol] == oppPiece)
				hasOppPieceBetween = true;
			else if ((board[curRow][curCol] == piece) && hasOppPieceBetween)
			{
				valid = true;
				break;
			}
			else
				break;

			curRow += moveRow[i];
			curCol += moveCol[i];
		}
		if (valid)
			break;
	}

	return valid;

}

std::vector<std::pair<int, int>> effectMove(char piece, int x, int y) {

	std::vector<std::pair<int, int>> pieces;

	for (int i = 0; i < 8; ++i) {
		int curRow = x + moveRow[i];
		int curCol = y + moveCol[i];

		if (curRow < 0 || curRow >= 8 || curCol < 0 || curCol >= 8 || board[curRow][curCol] == piece)
			continue;

		bool hasOppPieceBetween = false;
		while (curRow >= 0 && curRow < 8 && curCol >= 0 && curCol < 8) {
			if (board[curRow][curCol] == 'E')
				break;

			if (board[curRow][curCol] != piece)
				hasOppPieceBetween = true;

			if ((board[curRow][curCol] == piece) && hasOppPieceBetween)
			{
				int effectPieceRow = x + moveRow[i];
				int effectPieceCol = y + moveCol[i];
				while (effectPieceRow != curRow || effectPieceCol != curCol)
				{
					pieces.push_back(std::pair<int, int>(effectPieceRow, effectPieceCol));
					effectPieceRow += moveRow[i];
					effectPieceCol += moveCol[i];
				}

				break;
			}

			curRow += moveRow[i];
			curCol += moveCol[i];
		}
	}

	return pieces;
}

std::vector<std::pair<int, int>>findValidMove(char piece) {

	std::vector<std::pair<int, int>> movesList;

	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j) {
			if (isValidMove(piece, i, j))
			{
				movesList.push_back(std::pair<int, int>(i, j));

			}
		}
	return movesList;
}

int AlphaBetaMINCR(int alpha, int beta, int deph, int player);


int AlphaBetaMAXCR(int alpha, int beta, int deph, char piece) {
	char oppPiece = (piece == 'B') ? 'W' : 'B';
	if (deph == maxDeph)
		return heuristic(piece);

	int score = alpha;

	auto moves = findValidMove(piece);

	if (moves.size() == 0)
		score = AlphaBetaMINCR(score, beta, deph + 1, oppPiece);

	for (int i = 0; i < moves.size(); ++i) {

		int x = moves[i].first;
		int y = moves[i].second;


		board[x][y] = piece;
		auto effectPieces = effectMove(piece, x, y);

		for (int i = 0; i < effectPieces.size(); ++i)
			board[effectPieces[i].first][effectPieces[i].second] = piece;

		int s = AlphaBetaMINCR(score, beta, deph + 1, oppPiece);

		for (int i = 0; i < effectPieces.size(); ++i)
			board[effectPieces[i].first][effectPieces[i].second] = oppPiece;

		board[x][y] = 'E';

		if (s > score) {
			if (deph == 0){
				XX = x;
				YY = y;
			}
			score = s;
		}
		if (score >= beta) {
			if (deph == 0){
				XX = x;
				YY = y;
			}
			return score;
		}

	}
	return score;
}

int AlphaBetaMINCR(int alpha, int beta, int deph, int piece) {

	char oppPiece = (piece == 'B') ? 'W' : 'B';
	if (deph == maxDeph)
		return heuristic(piece);

	int score = beta;

	auto moves = findValidMove(piece);

	if (moves.size() == 0)
		score = AlphaBetaMAXCR(alpha, score, deph + 1, oppPiece);

	for (int i = 0; i < moves.size(); ++i) {

		int x = moves[i].first;
		int y = moves[i].second;


		board[x][y] = piece;
		auto effectPieces = effectMove(piece, x, y);

		for (int i = 0; i < effectPieces.size(); ++i)
			board[effectPieces[i].first][effectPieces[i].second] = piece;

		int s = AlphaBetaMAXCR(alpha, score, deph + 1, oppPiece);

		for (int i = 0; i < effectPieces.size(); ++i)
			board[effectPieces[i].first][effectPieces[i].second] = oppPiece;

		board[x][y] = 'E';

		if (s < score)
			score = s;
		if (score <= alpha) {
			return score;
		}
	}
	return score;
}


// int AlphaBetaMinMax(int alpha, int beta, int deph, int piece, int look, int X, int Y, bool pass) {
//
// 	char pieceChar = (piece == 1) ? 'B' : 'W';
// 	char oppPieceChar = (piece == 1) ? 'W' : 'B';
//
// 	//look += ExtraLookAhead(X, Y, deph);
// 	if (look <= 1) return heuristic(deph, 0, 0);
//
// 	++deph;
// 	auto moves = findValidMove(pieceChar);
// 	int high = INT_MIN + 100;
//
// 	if (moves.size() == 0){
// 		if (pass)
// 			return -heuristic(0, 0, 0);
// 		else
// 			return -AlphaBetaMinMax(-beta, -alpha, deph, piece * (-1), look - 1, -1, -1, true);
// 	}
//
// 	for (int i = 0; i < moves.size(); ++i) {
//
// 		int x = moves[i].first;
// 		int y = moves[i].second;
//
// 		board[x][y] = pieceChar;
// 		auto effectPieces = effectMove(pieceChar, x, y);
//
// 		for (int i = 0; i < effectPieces.size(); ++i)
// 			board[effectPieces[i].first][effectPieces[i].second] = pieceChar;
//
// 		int s = AlphaBetaMinMax(-beta, -alpha, deph, piece * (-1), look - 1, x, y, pass);
// 		// cout << "d: " << deph << " x: " << x << " y: " << y  << " ! "<<  x * 7 + (y + x) << " result: " << s << endl;
//
//
// 		for (int i = 0; i < effectPieces.size(); ++i)
// 			board[effectPieces[i].first][effectPieces[i].second] = oppPieceChar;
//
// 		board[x][y] = 'E';
//
// 		if (s > alpha) {
// 			alpha = s;
//
// 			if (alpha >= beta) {
// 				return -alpha;
// 			}
// 		}
// 		if (s > high) {
// 			high = s;
// 		}
// 	}
// 	return -high;
// }

void start(char piece) {

	int high = INT_MIN + 100;
	// auto moves = findValidMove(piece);
	// char oppPiece = (piece == 'B') ? 'W' : 'B';
	// for (int i = 0; i < moves.size(); ++i) {
	//
	// 	int x = moves[i].first;
	// 	int y = moves[i].second;
	//
	//
	//
	// 	board[x][y] = piece;
	// 	auto effectPieces = effectMove(piece, x, y);
	//
	// 	for (int i = 0; i < effectPieces.size(); ++i)
	// 		board[effectPieces[i].first][effectPieces[i].second] = piece;

		int m = AlphaBetaMAXCR(high, INT_MAX - 100, 0, 'W');
		//cout << endl << "x: " << x << " y: " << y << " ! " << x * 7 + (y + x) << " result: " << m << endl;

	// 	for (int i = 0; i < effectPieces.size(); ++i)
	// 		board[effectPieces[i].first][effectPieces[i].second] = oppPiece;
	//
	// 	board[x][y] = 'E';
	//
	// 	if (m > high) {
	// 		high = m;
	// 		XX = x;
	// 		YY = y;
	// 	}
	// }
}

void setBoardAndStart(string str_board) {
	int k = 0;
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j)
			board[i][j] = str_board[k++];

	char p = 'W';
	start(p);

}



int main() {


	//   for (int i = 0; i < 8; ++i) {
	//   	for (int j = 0; j < 8; ++j)
	//   		std::cout << board[i][j] << " ";
	//   	std::cout << std::endl;
	//   }
	//
	//   start('W');
	//   std::cout << std::endl;
	//
	//
	//
	//   for (int i = 0; i < 8; ++i) {
	//   	for (int j = 0; j < 8; ++j)
	//   		std::cout << board[i][j] << " ";
	//   	std::cout << std::endl;
	//   }
	//
	//   std::cout << XX << " " << YY << std::endl;

	crow::SimpleApp app;

	CROW_ROUTE(app, "/").methods("GET"_method)
		([](const crow::request& req)
	{
		std::ifstream myhtml("reversi.html");
		string site((istreambuf_iterator<char>(myhtml)), istreambuf_iterator<char>());
		myhtml.close();
		return site;
	});

	CROW_ROUTE(app, "/images/<string>").methods("GET"_method)
		([](const crow::request& req, string img)
	{
		std::ifstream myimg(img);
		string str((istreambuf_iterator<char>(myimg)), istreambuf_iterator<char>());
		myimg.close();

		return str;
	});


	CROW_ROUTE(app, "/echo/<string>").methods("GET"_method)
		([](const crow::request& req, string str_board) {

		setBoardAndStart(str_board);
		return crow::response{ to_string(XX) + to_string(YY) };
	});


	app.port(8080).multithreaded().run();
}
