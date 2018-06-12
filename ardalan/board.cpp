#include "board.h"

#include <iostream>

//MoveGenerator Board::mgen;

Board::Board() {
	current = new BoardComposite();
	history_begin = current;
	depth = 0;
}

Board::~Board() {
	// Go to beginning of history linked-list
	current = history_begin;
	
	// Delete each Board Composite
	BoardComposite * next = current->next;
	while (current) {
		next = current->next;
		delete current;
		current = next;
	}
}

const MoveList * Board::GetMoves() {
	if (!current->move_cache.IsValid()) {
		mgen.GetMoves(current, &(current->move_cache));
	}
	return &current->move_cache;
}

const MoveList * Board::GetUnmoves() {
	if (!current->unmove_cache.IsValid()) {
		mgen.GetUnmoves(current, &(current->unmove_cache));
	}
	return &current->unmove_cache;
}

bool Board::HasLegalMoves() {
	GetMoves();
	bool color = current->state.white_to_move;
	const Move * move = current->move_cache.Begin(); 
	const Move * end_move = current->move_cache.End();
	for (; move < end_move; move++) {
		if (Make(*move)) {
			bool in_check = InCheck(color);
			Unmake(1);
			if (!in_check) return true;
		}
	}
	return false;
}

bool Board::InCheck(bool is_white) {
	return mgen.InCheck(current, is_white);
}

bool Board::IsCheckmate() {
	return InCheck(current->state.white_to_move) && !HasLegalMoves();
}

bool Board::IsStalemate() {
	return !InCheck(current->state.white_to_move) && !HasLegalMoves();
}

bool Board::IsDrawByNoProgress() {
	const int MAX_PLY = 100;
	return current->state.n_ply_without_progress > MAX_PLY;
}

bool Board::IsDrawByRepetition() {
	const BoardComposite * board = current;
	
	// Repetition is impossible with progress in the last four moves
	if (current->state.n_ply_without_progress < 4) return false;
	
	// Go back four ply: is minimum distance at which a repetition could occur
	for (int i = 0; i < 4; i++) {
		if (board->last) board = board->last;
		else return false;
	}
	
	// Go back two until the linked list is exhausted or the last progress
	const BoardComposite * recent;
	for (int i = 4; i < current->state.n_ply_without_progress;) {
		recent = board;
		if (board->last) board = board->last;
		else return false;
		
		if (board->last) board = board->last;
		else return false;
		
		if (*recent == *board) return true;
		else i += 2;
	}
	
	return false;
}

bool Board::IsDraw() {
	return IsDrawByNoProgress() || IsDrawByRepetition() || IsStalemate();
}

/*
bool Board::IsDrawByInsufficientMaterial() {
	uint64_t roster[16];
	memcpy(roster, current->roster, 16);
	// General conditions
	if (roster[WHITE_PAWN] || roster[BLACK_PAWN]) return false;
	else if (roster[WHITE_QUEEN] || roster[BLACK_QUEEN]) return false;
	else if (roster[WHITE_ROOK] || roster[BLACK_ROOK]) return false;
}
*/

std::ostream & operator << (std::ostream & os, const Board & board) {
	os << "+=========================================================================+" << std::endl;
	os << "| Board: Depth " << (int)(board.depth + 1) << "  (Index " << (int)board.depth << ")" << std::endl;
	os << *(board.current);
	return os;
}