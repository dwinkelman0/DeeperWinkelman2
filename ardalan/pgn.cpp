#include "board.h"

#include <iostream>
#include <string.h>

bool Board::MakePGNMoves(const char * pgn) {
	
	bool white_to_move = true;
	uint8_t start_piece = 0;
	uint8_t end_square = 0;
	
	// Delimit string on whitespace
	char * copy = strdup(pgn);
	const char * pch = strtok(copy, " \n\t");
	while (pch) {
		// Check that not a move number...
		if (pch[0] >= '0' && pch[0] <= '9') goto next;
		
		// ...or a game termination
		if (pch[0] == '*') goto next;
		
		// Get piece type
		switch (pch[0]) {
			case 'K': start_piece = white_to_move ? WHITE_KING : BLACK_KING; break;
			case 'Q': start_piece = white_to_move ? WHITE_QUEEN : BLACK_QUEEN; break;
			case 'R': start_piece = white_to_move ? WHITE_ROOK : BLACK_ROOK; break;
			case 'B': start_piece = white_to_move ? WHITE_BISHOP : BLACK_BISHOP; break;
			case 'N': start_piece = white_to_move ? WHITE_KNIGHT : BLACK_KNIGHT; break;
			default:  start_piece = white_to_move ? WHITE_PAWN : BLACK_PAWN; break;
		}
		
		white_to_move = !white_to_move;
		std::cout << "Parsed " << (int)start_piece << ": " << pch << std::endl;
		
		next: {
			pch = strtok(NULL, " \n\t");
		}
	}
	
	free(copy);
	
	return true;
}