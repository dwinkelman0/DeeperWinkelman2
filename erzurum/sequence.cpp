#include "tablebase.h"

#include <iostream>
#include <string.h>

TableBase::MoveSequence::MoveSequence() {
	this->moves = NULL;
	this->n_moves = 0;
	this->n_alloc = 0;
	this->initial_state = BoardState();
	this->result = TableBase::Node::RESULT_UNDETERMINED;
}

TableBase::MoveSequence::MoveSequence(BoardState state) {
	this->moves = NULL;
	this->n_moves = 0;
	this->n_alloc = 0;
	this->initial_state = state;
	this->result = TableBase::Node::RESULT_UNDETERMINED;
}

TableBase::MoveSequence::MoveSequence(const MoveSequence & other) {
	this->n_moves = other.n_moves;
	this->n_alloc = other.n_alloc;
	this->initial_state = other.initial_state;
	this->result = other.result;
	
	this->moves = new Move[other.n_alloc];
	memcpy(this->moves, other.moves, other.n_alloc * sizeof(Move));
}

TableBase::MoveSequence & TableBase::MoveSequence::operator =(const TableBase::MoveSequence & other) {
	Move * temp_moves = new Move[other.n_alloc];
	delete[] this->moves;
	
	this->n_moves = other.n_moves;
	this->n_alloc = other.n_alloc;
	this->initial_state = other.initial_state;
	this->result = other.result;
	
	this->moves = temp_moves;
	memcpy(this->moves, other.moves, other.n_alloc * sizeof(Move));
	
	return *this;
}

TableBase::MoveSequence::~MoveSequence() {
	if (this->moves) {
		delete[] this->moves;
		this->moves = NULL;
	}
	this->n_moves = 0;
	this->n_alloc = 0;
}

std::ostream & operator << (std::ostream & os, const TableBase::MoveSequence & ms) {
	os << "MoveSequence [Result: ";
	if (ms.result == TableBase::Node::RESULT_WHITE_WIN) os << "White Win";
	else if (ms.result == TableBase::Node::RESULT_BLACK_WIN) os << "Black Win";
	else if (ms.result == TableBase::Node::RESULT_DRAW) os << "Draw";
	else os << "Undetermined";
	os << ", Length: " << ms.Length();
	os << ", Initial Position: \"" << ms.initial_state.GetFEN() << "\"] ";
	
	if (ms.n_moves == 0) {
		os << "No Moves";
		return os;
	}
	
	int move_number = 1;
	bool white_to_move = false;
	const Move * move_i = ms.Begin();
	const Move * move_end = ms.End();
	os << "1. ";
	if (!ms.initial_state.white_to_move) {
		os << "... ";
	}
	else {
		os << *(move_i++) << " ";
	}
	for (; move_i < move_end; move_i++) {
		if (white_to_move) {
			move_number++;
			std::cout << move_number << ". ";
		}
		white_to_move = !white_to_move;
		os << *move_i << " ";
	}
	
	return os;
}