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

void Generate_KRvK(TableBase * tb_instance) {
	
	const char * king_arrangements[48][2] = {
		// Kings against top/bottom of board [0, 20)
		{"a1", "a3"}, {"a1", "b3"}, {"b1", "b3"}, {"c1", "c3"}, {"d1", "d3"},
		{"e1", "e3"}, {"f1", "f3"}, {"g1", "g3"}, {"h1", "g3"}, {"h1", "h3"},
		{"a8", "a6"}, {"a8", "b6"}, {"b8", "b6"}, {"c8", "c6"}, {"d8", "d6"},
		{"e8", "e6"}, {"f8", "f6"}, {"g8", "g6"}, {"h8", "g6"}, {"h8", "h6"},
		// Kings on sides of board [20, 40)
		{"a1", "c1"}, {"a1", "c2"}, {"a2", "c2"}, {"a3", "c3"}, {"a4", "c4"},
		{"a5", "c5"}, {"a6", "c6"}, {"a7", "c7"}, {"a8", "c7"}, {"a8", "c8"},
		{"h1", "f1"}, {"h1", "f2"}, {"h2", "f2"}, {"h3", "f3"}, {"h4", "f4"},
		{"h5", "f5"}, {"h6", "f6"}, {"h7", "f7"}, {"h8", "f7"}, {"h8", "f8"},
		// Kings in the corners (for stalemate) [40, 48)
		{"a1", "a3"}, {"a8", "a6"}, {"h1", "h3"}, {"h8", "h6"},
		{"a1", "c1"}, {"h1", "f1"}, {"a8", "c8"}, {"h8", "f8"}
	};
	
	BoardState state;
	memset(state.squares, 0, 64);
	state.white_to_move = false;
	state.white_OO = state.white_OOO = state.black_OO = state.black_OOO = false;
	state.ep_target = 0;
	
	Board board;
	
	for (int i_king = 0; i_king < 48; i_king++) {
		
		uint8_t black_king = Text2Coord(king_arrangements[i_king][0]);
		uint8_t white_king = Text2Coord(king_arrangements[i_king][1]);
		
		state.squares[white_king] = WHITE_KING;
		state.squares[black_king] = BLACK_KING;
		
		uint8_t white_rook, white_rook_end, white_rook_inc;
		if (i_king < 20) {
			white_rook = (i_king < 10) ? 0 : 56;
			white_rook_end = (i_king < 10) ? 8 : 64;
			white_rook_inc = 1;
		}
		else if (i_king < 40) {
			white_rook = (i_king < 30) ? 0 : 7;
			white_rook_end = 64;
			white_rook_inc = 8;
		}
		else if (i_king < 44) {
			white_rook = (i_king < 42) ? 1 : 6;
			white_rook_end = 64;
			white_rook_inc = 8;
		}
		else {
			white_rook = (i_king < 46) ? 8 : 48;
			white_rook_end = (i_king < 46) ? 16 : 56;
			white_rook_inc = 1;
		}
		
		for (; white_rook < white_rook_end; white_rook += white_rook_inc) {
			if (white_rook == black_king) {
				continue;
			}
			
			state.squares[white_rook] = WHITE_ROOK;
			
			// Add state to table base
			uint8_t result;
			bool valid = true;
			
			// Verify validity of position
			board.SetCurrent(state);
			if (i_king < 40) {
				if (!board.IsCheckmate()) valid = false;
				result = TableBase::Node::RESULT_WHITE_WIN;
			}
			else {
				if (!board.IsStalemate()) valid = false;
				result = TableBase::Node::RESULT_DRAW;
			}
			
			if (valid) {
				tb_instance->AddStaticallySolved(state, result);
			}
			
			state.squares[white_rook] = EMPTY;
		}
		
		state.squares[white_king] = EMPTY;
		state.squares[black_king] = EMPTY;
	}
	
	// Special Draws
	int res_draw = TableBase::Node::RESULT_DRAW;
	state.InitFromFEN("8/8/8/8/8/8/1RK5/k7 b - - 0 1");
	tb_instance->AddStaticallySolved(state, res_draw);
	state.InitFromFEN("8/8/8/8/8/1K6/1R6/k7 b - - 0 1");
	tb_instance->AddStaticallySolved(state, res_draw);
	state.InitFromFEN("8/8/8/8/8/2K5/1R6/k7 b - - 0 1");
	tb_instance->AddStaticallySolved(state, res_draw);
	state.InitFromFEN("k7/1R6/1K6/8/8/8/8/8 b - - 0 1");
	tb_instance->AddStaticallySolved(state, res_draw);
	state.InitFromFEN("k7/1R6/2K5/8/8/8/8/8 b - - 0 1");
	tb_instance->AddStaticallySolved(state, res_draw);
	state.InitFromFEN("k7/1RK5/8/8/8/8/8/8 b - - 0 1");
	tb_instance->AddStaticallySolved(state, res_draw);
	state.InitFromFEN("7k/5KR1/8/8/8/8/8/8 b - - 0 1");
	tb_instance->AddStaticallySolved(state, res_draw);
	state.InitFromFEN("7k/6R1/5K2/8/8/8/8/8 b - - 0 1");
	tb_instance->AddStaticallySolved(state, res_draw);
	state.InitFromFEN("7k/6R1/6K1/8/8/8/8/8 b - - 0 1");
	tb_instance->AddStaticallySolved(state, res_draw);
	state.InitFromFEN("8/8/8/8/8/6K1/6R1/7k b - - 0 1");
	tb_instance->AddStaticallySolved(state, res_draw);
	state.InitFromFEN("8/8/8/8/8/5K2/6R1/7k b - - 0 1");
	tb_instance->AddStaticallySolved(state, res_draw);
	state.InitFromFEN("8/8/8/8/8/8/5KR1/7k b - - 0 1");
	tb_instance->AddStaticallySolved(state, res_draw);
}

void Patch_KRvK(TableBase * tb_instance) {
	BoardState state;
	memset(state.squares, 0, 64);
	state.white_to_move = false;
	state.white_OO = state.white_OOO = state.black_OO = state.black_OOO = false;
	state.ep_target = 0;
	
	Board board;
	
	for (int wking = 0; wking < 64; wking++) {
		for (int bking = 0; bking < 64; bking++) {
			if (wking == bking) continue;
			
			for (int wrook = 0; wrook < 64; wrook++) {
				if (wrook == wking || wrook == bking) continue;
				
				state.squares[wking] = WHITE_KING;
				state.squares[bking] = BLACK_KING;
				state.squares[wrook] = WHITE_ROOK;
				
				state.white_to_move = false;
				board.SetCurrent(state);
				if (!board.InCheck(true)) {
					TableBase::Evaluation eval = tb_instance->Evaluate(state);
					if (eval.result == TableBase::Evaluation::RESULT_UNDETERMINED) {
						tb_instance->AddStaticallySolved(
							state, TableBase::Evaluation::RESULT_DRAW);
					}
				}				
				
				state.squares[wking] = EMPTY;
				state.squares[bking] = EMPTY;
				state.squares[wrook] = EMPTY;
			}
		}
	}
}