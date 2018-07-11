#include "board.h"

#include <iostream>
#include <string.h>

bool Board::MakePGNMoves(const char * pgn) {
	
	bool white_to_move = true;
	
	// Delimit string on whitespace
	char * copy = strdup(pgn);
	const char * pch = strtok(copy, " \n\t");
	while (pch) {
		// Check that not a move number or a game termination
		if ((pch[0] >= '0' && pch[0] <= '9') || pch[0] == '*') {
			pch = strtok(NULL, " \n\t");
			continue;
		}
		
		// Check for castling; if found, do it
		if (!strcmp(pch, "O-O") || !strcmp(pch, "O-O-O")) {
			Move move;
			if (!strcmp(pch, "O-O")) {
				move = Move(0, 0, white_to_move ? Move::WHITE_OO : Move::BLACK_OO);
			}
			else {
				move = Move(0, 0, white_to_move ? Move::WHITE_OOO : Move::BLACK_OOO);
			}
			if (!Make(move)) {
				return false;
			}
		}
		else {
			uint8_t start_piece = 0;
			uint8_t end_square = 0;
			int n_row = 0, n_col = 0, n_capture = 0;
			const char * first_pos = NULL;
			
			// Get piece type
			switch (pch[0]) {
				case 'K': start_piece = white_to_move ? WHITE_KING : BLACK_KING; break;
				case 'Q': start_piece = white_to_move ? WHITE_QUEEN : BLACK_QUEEN; break;
				case 'R': start_piece = white_to_move ? WHITE_ROOK : BLACK_ROOK; break;
				case 'B': start_piece = white_to_move ? WHITE_BISHOP : BLACK_BISHOP; break;
				case 'N': start_piece = white_to_move ? WHITE_KNIGHT : BLACK_KNIGHT; break;
				default:  start_piece = white_to_move ? WHITE_PAWN : BLACK_PAWN; break;
			}
			
			// Scan for 'x' and how many coordinate characters
			const char * i = pch;
			while (*i) {
				if (*i >= '1' && *i <= '8') {
					n_row++;
					if (!first_pos) first_pos = i;
				}
				else if (*i >= 'a' && *i <= 'h') {
					n_col++;
					if (!first_pos) first_pos = i;
				}
				else if (*i == 'x') {
					n_capture++;
					if (!first_pos) first_pos = i;
				}
				i++;
			}
			int n_pos_char = n_row + n_col + n_capture;
			
			// Get ending coordinate
			end_square = Text2Coord(first_pos - 2 + n_pos_char);
			
			// Get moves to find which one matches the constraints
			bool any_moves = false;
			BoardState state = GetCurrent();
			const MoveList * moves = GetMoves();
			const Move * move_i = moves->Begin();
			const Move * move_end = moves->End();
			for (; move_i != move_end; move_i++) {
				// Make sure start piece matches
				if (state.squares[move_i->start] != start_piece) continue;
				
				// Make sure end square matches
				if (move_i->end != end_square) continue;
				
				// If there are further constraints, check those
				if (n_row == 2 && n_col == 2) {
					uint8_t start_square = Text2Coord(first_pos);
					if (move_i->start != start_square) continue;
				}
				else if (n_row == 2) {
					if (move_i->start / 8 != *first_pos - '1') continue;
				}
				else if (n_col == 2) {
					if (move_i->start % 8 != *first_pos - 'a') continue;
				}
				
				// Passed all the tests, make the move
				if (Make(*move_i)) {
					// Make sure move is legal
					if (InCheck(state.white_to_move)) {
						Unmake(1);
					}
					else {
						any_moves = true;
						break;
					}
				}
				else return false;
			}
			
			if (!any_moves) return false;
		}
		
		white_to_move = !white_to_move;
		pch = strtok(NULL, " \n\t");
	}
	
	free(copy);
	
	return true;
}