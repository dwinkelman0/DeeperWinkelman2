#include "seed.h"

#include <string.h>

void Generate_KvK(TableBase * tb_instance) {
	
	BoardState state;
	memset(state.squares, 0, 64);
	state.white_to_move = false;
	state.white_OO = state.white_OOO = state.black_OO = state.black_OOO = false;
	state.ep_target = 0;
	
	Board board;
	
	for (int white_king = 0; white_king < 64; white_king++) {
		for (int black_king = 0; black_king < 64; black_king++) {
			
			if (white_king == black_king) continue;
			
			state.squares[white_king] = WHITE_KING;
			state.squares[black_king] = BLACK_KING;
			
			board.SetCurrent(state);
			if (!board.InCheck(true)) {
				state.white_to_move = true;
				tb_instance->AddStaticallySolved(state, TableBase::Node::RESULT_DRAW);
				state.white_to_move = false;
				tb_instance->AddStaticallySolved(state, TableBase::Node::RESULT_DRAW);
			}
			
			state.squares[white_king] = EMPTY;
			state.squares[black_king] = EMPTY;
		}
	}
}