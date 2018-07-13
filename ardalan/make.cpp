#include "board.h"

#include <iostream>
#include <string.h>

/**
 * Data to Update in Move-Making
 * 
 * 
 * BOARD STATE
 *-----------------
 * Board Squares [complete]
 * Color [main]
 * Castling Rights [complete]
 * En Passant [complete]
 * Moves Without Progress [individual]
 * 
 * 
 * BOARD COMPOSITE
 *-----------------
 * Bitboards [complete]
 * King Positions [complete]
 * Piece Roster [complete]
 * Move Cache [main]
 * Next and Last Positions [main]
 * Move to Next and Last Positions [main]
 */

bool Board::Make(Move move) {
	bool status = true;
	
	// Get Board Composites for move making
	if (!current->next) {
		// Allocate a next link in the list if it does not exist
		current->next = new BoardComposite();
		current->next->last = current;
	}
	
	// Reset move to next/from last linkage
	current->move_to_next = Move(0, 0, Move::NULL_MOVE);
	current->next->move_from_last = Move(0, 0, Move::NULL_MOVE);
	
	// Contextualize position linking
	const BoardComposite * orig = current;
	BoardComposite * target = current->next;
	
	// Decide which type of move to make
	if (move.code == Move::NORMAL_MOVE) {
		status = MakeNormal(orig, target, move);
	}
	else if (move.code == Move::EN_PASSANT) {
		status = MakeEnPassant(orig, target, move);
	}
	else if (
			move.code == Move::WHITE_OO || move.code == Move::WHITE_OOO ||
			move.code == Move::BLACK_OO || move.code == Move::BLACK_OOO) {
		status = MakeCastling(orig, target, move);
	}
	else if ((move.code >= WHITE_KNIGHT && move.code <= WHITE_QUEEN) || (move.code >= BLACK_KNIGHT && move.code <= BLACK_QUEEN)) {
		status = MakePromotion(orig, target, move);
	}
	else {
		std::cout << "Invalid Move Type" << std::endl;
		return false;
	}
	
	if (status) {
		// Color to Move
		target->state.white_to_move = !orig->state.white_to_move;
		
		// Move Cache
		target->move_cache.Clear();
		target->unmove_cache.Clear();
		
		// Link moves
		current->move_to_next = move;
		current->next->move_from_last = move;
		
		// Increment Current Position
		current = current->next;
		depth++;
	}
	
	return status;
}

bool Board::MakeNormal(const BoardComposite * orig, BoardComposite * target, Move move) {
	bool status = MakeComplete(orig, target, move.start, move.end,
			orig->state.squares[move.start], orig->state.squares[move.end], orig->state.squares[move.start]);
	if (!status) return false;
	if (orig->state.squares[move.start] == WHITE_PAWN || orig->state.squares[move.start] == BLACK_PAWN || orig->state.squares[move.end] != EMPTY) {
		target->state.n_ply_without_progress = 0;
	}
	else {
		target->state.n_ply_without_progress = orig->state.n_ply_without_progress + 1;
	}
	
	return status;
}

bool Board::MakeCastling(const BoardComposite * orig, BoardComposite * target, Move move) {
	static BoardComposite intermediate1, intermediate2;
	uint8_t king_start, king_inter, king_end, rook_start, rook_end;
	uint8_t king_piece, rook_piece;
	bool color;
	if (move.code == Move::WHITE_OO) {
		if (orig->state.white_to_move != true) return false;
		king_start = 4;
		king_inter = 5;
		king_end = 6;
		rook_start = 7;
		rook_end = 5;
		king_piece = WHITE_KING;
		rook_piece = WHITE_ROOK;
		color = true;
	}
	else if (move.code == Move::WHITE_OOO) {
		if (orig->state.white_to_move != true) return false;
		king_start = 4;
		king_inter = 3;
		king_end = 2;
		rook_start = 0;
		rook_end = 3;
		king_piece = WHITE_KING;
		rook_piece = WHITE_ROOK;
		// Verify the extra empty square
		if (orig->state.squares[1] != EMPTY) return false;
		color = true;
	}
	else if (move.code == Move::BLACK_OO) {
		if (orig->state.white_to_move != false) return false;
		king_start = 60;
		king_inter = 61;
		king_end = 62;
		rook_start = 63;
		rook_end = 61;
		king_piece = BLACK_KING;
		rook_piece = BLACK_ROOK;
		color = false;
	}
	else if (move.code == Move::BLACK_OOO) {
		if (orig->state.white_to_move != false) return false;
		king_start = 60;
		king_inter = 59;
		king_end = 58;
		rook_start = 56;
		rook_end = 59;
		king_piece = BLACK_KING;
		rook_piece = BLACK_ROOK;
		// Verify the extra empty square
		if (orig->state.squares[57] != EMPTY) return false;
		color = false;
	}
	else {
		return false;
	}
	
	// Verify that there is correct piece placement
	if (orig->state.squares[king_start] != king_piece || orig->state.squares[rook_start] != rook_piece) {
		return false;
	}
	
	// Verify that there are enough empty squares
	if (orig->state.squares[king_inter] != EMPTY || orig->state.squares[king_end] != EMPTY) {
		return false;
	}
	
	// Verify that the king is not in check while making the moves in sequence
	bool status = true;
	if (mgen.InCheck(orig, color)) return false;
	status = MakeComplete(orig, &intermediate1, king_start, king_inter, king_piece, (uint8_t)EMPTY, king_piece);
	if (!status) return status;
	if (mgen.InCheck(&intermediate1, color)) return false;
	status = MakeComplete(&intermediate1, &intermediate2, king_inter, king_end, king_piece, (uint8_t)EMPTY, king_piece);
	if (!status) return status;
	if (mgen.InCheck(&intermediate2, color)) return false;
	status = MakeComplete(&intermediate2, target, rook_start, rook_end, rook_piece, (uint8_t)EMPTY, rook_piece);
	if (!status) return status;
	
	target->state.n_ply_without_progress = 0;
	
	return status;
}

bool Board::MakeEnPassant(const BoardComposite * orig, BoardComposite * target, Move move) {
	static BoardComposite intermediate;
	uint8_t start, inter, end;
	uint8_t friendly_pawn, enemy_pawn;
	
	start = move.start;
	end = move.end;
	if (orig->state.white_to_move) {
		inter = move.end - 8;
		friendly_pawn = WHITE_PAWN;
		enemy_pawn = BLACK_PAWN;
	}
	else {
		inter = move.end + 8;
		friendly_pawn = BLACK_PAWN;
		enemy_pawn = WHITE_PAWN;
	}
	
	// Verify that there is correct piece placement
	if (orig->state.squares[start] != friendly_pawn || orig->state.squares[inter] != enemy_pawn || orig->state.squares[end] != EMPTY) {
		return false;
	}
	
	bool status = true;
	status = MakeComplete(orig, &intermediate, start, inter, friendly_pawn, enemy_pawn, friendly_pawn);
	if (!status) return status;
	status = MakeComplete(&intermediate, target, inter, end, friendly_pawn, (uint8_t)EMPTY, friendly_pawn);
	if (!status) return status;
	
	target->state.n_ply_without_progress = 0;
	
	return true;
}

bool Board::MakePromotion(const BoardComposite * orig, BoardComposite * target, Move move) {
	// Check that a pawn is making the move
	if (orig->state.squares[move.start] != WHITE_PAWN && orig->state.squares[move.start] != BLACK_PAWN) {
		std::cout << "Neither are pawns" << std::endl;
		return false;
	}
	
	bool status = MakeComplete(orig, target, move.start, move.end,
			orig->state.squares[move.start], orig->state.squares[move.end], move.code);
	if (!status) return false;
	
	target->state.n_ply_without_progress = 0;
	
	return status;
}

bool Board::MakeComplete(const BoardComposite * orig, BoardComposite * target,
		uint8_t start, uint8_t end, uint8_t start_piece, uint8_t end_piece, uint8_t promotion_piece) {

	/***************************************************************************
	 * Error Checking
	 */
	// Cannot capture king
	if (end_piece == WHITE_KING || end_piece == BLACK_KING) std::cout << "King is end piece" << std::endl;
	if (end_piece == WHITE_KING || end_piece == BLACK_KING) return false;
	
	// Cannot move a piece that does not belong to the color to move
	// Cannot capture a piece that belongs to the color to move
	if (orig->state.white_to_move) {
		if (!(start_piece >= WHITE_PAWN && start_piece <= WHITE_KING)) std::cout << "Moving wrong colored piece" << std::endl;
		if (!(start_piece >= WHITE_PAWN && start_piece <= WHITE_KING)) return false;
		if (end_piece >= WHITE_PAWN && end_piece <= WHITE_KING) std::cout << "Capturing wrong colored piece" << std::endl;
		if (end_piece >= WHITE_PAWN && end_piece <= WHITE_KING) return false;
	}
	else {
		if (!(start_piece >= BLACK_PAWN && start_piece <= BLACK_KING)) std::cout << "Moving wrong colored piece" << std::endl;
		if (!(start_piece >= BLACK_PAWN && start_piece <= BLACK_KING)) return false;
		if (end_piece >= BLACK_PAWN && end_piece <= BLACK_KING) std::cout << "Capturing wrong colored piece" << std::endl;
		if (end_piece >= BLACK_PAWN && end_piece <= BLACK_KING) return false;
	}
	
	/***************************************************************************
	 * Apply States
	 */
	// Board Squares
	memcpy(target->state.squares, orig->state.squares, 64);
	target->state.squares[start] = EMPTY;
	target->state.squares[end] = promotion_piece;
	
	// Bitboards
	Bitboard_t factor = ((Bitboard_t)1 << end) | ((Bitboard_t)1 << start);
	if (orig->state.white_to_move) {
		target->white = orig->white ^ factor;
		if (start_piece == WHITE_PAWN) {
			target->wpawns = orig->wpawns ^ factor;
			if (promotion_piece != WHITE_PAWN) {
				target->wpawns &= ~factor;
			}
		}
		else {
			target->wpawns = orig->wpawns;
		}
		target->black = orig->black & ~factor;
		target->bpawns = orig->bpawns & ~factor;
	}
	else {
		target->black = orig->black ^ factor;
		if (start_piece == BLACK_PAWN) {
			target->bpawns = orig->bpawns ^ factor;
			if (promotion_piece != BLACK_PAWN) {
				target->bpawns &= ~factor;
			}
		}
		else {
			target->bpawns = orig->bpawns;
		}
		target->white = orig->white & ~factor;
		target->wpawns = orig->wpawns & ~factor;
	}
	
	// King Positions
	target->wking_pos = start_piece == WHITE_KING ? end : orig->wking_pos;
	target->bking_pos = start_piece == BLACK_KING ? end : orig->bking_pos;
	
	// Piece Roster
	memcpy(target->roster, orig->roster, 16);
	target->roster[start_piece]--;
	target->roster[end_piece]--;
	target->roster[promotion_piece]++;
	
	// En Passant
	if (	(start_piece == WHITE_PAWN && end - start == 16) ||
			(start_piece == BLACK_PAWN && start - end == 16)) {
		target->state.ep_target = end;
	}
	else {
		target->state.ep_target = 0;
	}
	
	// Castling Rights
	target->state.white_OO = orig->state.white_OO &&
		target->state.squares[4] == WHITE_KING && target->state.squares[7] == WHITE_ROOK;
	target->state.white_OOO = orig->state.white_OOO &&
		target->state.squares[4] == WHITE_KING && target->state.squares[0] == WHITE_ROOK;
	target->state.black_OO = orig->state.black_OO &&
		target->state.squares[60] == BLACK_KING && target->state.squares[63] == BLACK_ROOK;
	target->state.black_OOO = orig->state.black_OOO &&
		target->state.squares[60] == BLACK_KING && target->state.squares[56] == BLACK_ROOK;
		
	// Hash
	target->hash = orig->hash
		// Squares
		^ ZOBRIST_SQUARES[start_piece][start]
		^ ZOBRIST_SQUARES[EMPTY][start]
		^ ZOBRIST_SQUARES[end_piece][end]
		^ ZOBRIST_SQUARES[promotion_piece][end]
		// Castling
		^ ZOBRIST_WHITE_OO[target->state.white_OO]
		^ ZOBRIST_WHITE_OO[orig->state.white_OO]
		^ ZOBRIST_WHITE_OOO[target->state.white_OOO]
		^ ZOBRIST_WHITE_OOO[orig->state.white_OOO]
		^ ZOBRIST_BLACK_OO[target->state.black_OO]
		^ ZOBRIST_BLACK_OO[orig->state.black_OO]
		^ ZOBRIST_BLACK_OOO[target->state.black_OOO]
		^ ZOBRIST_BLACK_OOO[orig->state.black_OOO]
		// En Passant
		^ ZOBRIST_EN_PASSANT[target->state.ep_target]
		^ ZOBRIST_EN_PASSANT[orig->state.ep_target]
		// Color
		^ ZOBRIST_WHITE_TO_MOVE;
		
	// Transfer color to next
	target->state.white_to_move = orig->state.white_to_move;
	
	return true;
}

bool Board::Unmake(uint16_t n_moves) {
	for (int i = 0; i < n_moves; i++) {
		if (current->last) {
			current = current->last;
			depth--;
		}
		else return false;
	}
	return true;
}