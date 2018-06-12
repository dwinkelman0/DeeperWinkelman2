#include "datatypes.h"
#include "movegen.h"

#include <iostream>

#if 0
static void PrintBitboard(Bitboard_t x) {
	for (int rank = 7; rank >= 0; rank--) {
		for (int file = 0; file < 8; file++) {
			std::cout << (((x >> (rank * 8 + file)) & 1) ? "O " : ". ");
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}
#endif

MoveGenerator::MoveGenerator() {
	Init();
}

MoveList MoveGenerator::GetMoves(const BoardComposite * board) {
	MoveList output;
	GetMoves(board, &output);
	return output;
}

void MoveGenerator::GetMoves(const BoardComposite * board, MoveList * output) {
	Bitboard_t friendly, enemy;
	uint8_t piece;
	
	static int coord_lists_squares[256];
	static CoordList coord_lists[256];
	int n_coord_lists = 0;
	
	static int prom_coord_lists_squares[8];
	static CoordList prom_coord_lists[8];
	int n_prom_coord_lists = 0;
	
	if (board->state.white_to_move)
	{
		friendly = board->white;
		enemy = board->black;
		
		for (int square = 0; square < 64; square++) {
			
			piece = board->state.squares[square];
			
			if (piece == EMPTY || piece > 8) continue;

			if (piece == WHITE_PAWN) {
				// Check for promotion
				if (square / 8 == 6) {
					prom_coord_lists[n_prom_coord_lists] = GetWPMoves(friendly, enemy, square);
					prom_coord_lists_squares[n_prom_coord_lists++] = square;
				} else {
					coord_lists[n_coord_lists] = GetWPMoves(friendly, enemy, square);
					coord_lists_squares[n_coord_lists++] = square;
				}
			}
			else if (piece == WHITE_KNIGHT) {
				coord_lists[n_coord_lists] = GetNMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
			else if (piece == WHITE_BISHOP) {
				coord_lists[n_coord_lists] = GetD1Moves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetD2Moves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
			else if (piece == WHITE_ROOK) {
				coord_lists[n_coord_lists] = GetHMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetVMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
			else if (piece == WHITE_QUEEN) {
				coord_lists[n_coord_lists] = GetD1Moves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetD2Moves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetHMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetVMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
			else if (piece == WHITE_KING) {
				coord_lists[n_coord_lists] = GetKMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
		}
	}
	else
	{
		friendly = board->black;
		enemy = board->white;
		
		for (int square = 0; square < 64; square++) {
			
			piece = board->state.squares[square];
			
			if (piece == EMPTY || piece < 8) continue;

			if (piece == BLACK_PAWN) {
				// Check for promotion
				if (square / 8 == 1) {
					prom_coord_lists[n_prom_coord_lists] = GetBPMoves(friendly, enemy, square);
					prom_coord_lists_squares[n_prom_coord_lists++] = square;
				} else {
					coord_lists[n_coord_lists] = GetBPMoves(friendly, enemy, square);
					coord_lists_squares[n_coord_lists++] = square;
				}
			}
			else if (piece == BLACK_KNIGHT) {
				coord_lists[n_coord_lists] = GetNMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
			else if (piece == BLACK_BISHOP) {
				coord_lists[n_coord_lists] = GetD1Moves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetD2Moves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
			else if (piece == BLACK_ROOK) {
				coord_lists[n_coord_lists] = GetHMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetVMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
			else if (piece == BLACK_QUEEN) {
				coord_lists[n_coord_lists] = GetD1Moves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetD2Moves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetHMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetVMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
			else if (piece == BLACK_KING) {
				coord_lists[n_coord_lists] = GetKMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
		}
	}
	
	Move special_moves[6];
	
	// Castling
	// Conditions:
	//		1. Castling must be available
	//		2. King and rook are in correct place
	//		3. No pieces between king and rook
	Bitboard_t all = friendly | enemy;
	if (board->state.white_to_move) {
		if (board->state.white_OO && board->state.squares[4] == WHITE_KING
				&& board->state.squares[7] == WHITE_ROOK && !(all & 0x0000000000000060))
		{
			special_moves[0].code = Move::WHITE_OO;
		}
		if (board->state.white_OOO && board->state.squares[4] == WHITE_KING
				&& board->state.squares[0] == WHITE_ROOK && !(all & 0x000000000000000e))
		{ 
			special_moves[1].code = Move::WHITE_OOO;
		}
	}
	else {
		if (board->state.black_OO && board->state.squares[60] == BLACK_KING
				&& board->state.squares[63] == BLACK_ROOK && !(all & 0x6000000000000000))
		{
			special_moves[2].code = Move::BLACK_OO;
		}
		if (board->state.black_OOO && board->state.squares[60] == BLACK_KING
				&& board->state.squares[56] == BLACK_ROOK && !(all & 0x0e00000000000000))
		{
			special_moves[3].code = Move::BLACK_OOO;
		}
	}
	
	// En passant
	// Conditions:
	// 		1. En passant target must be set
	//		2. Target square must be not on edge of board
	//		3. Square adjacent to target must be a pawn of the opposite color
	if (board->state.ep_target) {
		// Check for pawn to the right of the target
		if (board->state.ep_target % 8 != 7) {
			if (board->state.white_to_move
					&& board->state.squares[board->state.ep_target + 1] == WHITE_PAWN)
			{
				special_moves[4].start = board->state.ep_target + 1;
				special_moves[4].end = board->state.ep_target + 8;
				special_moves[4].code = Move::EN_PASSANT;
			}
			else if (!board->state.white_to_move
					&& board->state.squares[board->state.ep_target + 1] == BLACK_PAWN)
			{
				special_moves[4].start = board->state.ep_target + 1;
				special_moves[4].end = board->state.ep_target - 8;
				special_moves[4].code = Move::EN_PASSANT;
			}
		}
		// Check for pawn to the left of the target
		if (board->state.ep_target % 8 != 0) {
			if (board->state.white_to_move 
					&& board->state.squares[board->state.ep_target - 1] == WHITE_PAWN)
			{
				special_moves[5].start = board->state.ep_target - 1;
				special_moves[5].end = board->state.ep_target + 8;
				special_moves[5].code = Move::EN_PASSANT;
			}
			else if (!board->state.white_to_move 
					&& board->state.squares[board->state.ep_target - 1] == BLACK_PAWN) 
			{
				special_moves[5].start = board->state.ep_target - 1;
				special_moves[5].end = board->state.ep_target - 8;
				special_moves[5].code = Move::EN_PASSANT;
			}
		}
	}
	
	// Allocate Memory
	uint16_t n_alloc = (n_coord_lists + n_prom_coord_lists) * 8 + 6;
	if (output->moves) {
		// If there is not enough memory, free and allocate later
		if (output->n_alloc < n_alloc) {
			delete[] output->moves;
			output->moves = NULL;
			output->n_moves = 0;
			output->n_alloc = 0;
		}
	}
	// Allocate fresh/new memory
	if (!output->moves) {
		output->moves = new Move[n_alloc];
		output->n_moves = 0;
		output->n_alloc = n_alloc;
	}
	
	// Get variables for assigning output
	Move * move_list = output->moves;
	uint16_t n_moves = 0;
	
	// Normal Moves
	for (int i = 0; i < n_coord_lists; i++) {
		//std::cout << (int)coord_lists_squares[i] << ": " << coord_lists[i] << std::endl;
		
		for (int j = 0; j < 8; j++) {
			move_list[n_moves].start = coord_lists_squares[i];
			move_list[n_moves].end = coord_lists[i].coords[j];
			move_list[n_moves].code = Move::NORMAL_MOVE;
			
			n_moves += coord_lists[i].coords[j] < 64;
		}
	}
	// Promotion Moves
	for (int i = 0; i < n_prom_coord_lists; i++) {
		//std::cout << (int)prom_coord_lists_squares[i] << ": " << prom_coord_lists[i] << std::endl;
		
		for (int j = 0; j < 8; j++) {
			if (board->state.white_to_move) {
				for (int piece = WHITE_KNIGHT; piece <= WHITE_QUEEN; piece++) {
					move_list[n_moves].start = prom_coord_lists_squares[i];
					move_list[n_moves].end = prom_coord_lists[i].coords[j];
					move_list[n_moves].code = piece;
					
					n_moves += prom_coord_lists[i].coords[j] < 64;
				}
			}
			else {
				for (int piece = BLACK_KNIGHT; piece <= BLACK_QUEEN; piece++) {
					move_list[n_moves].start = prom_coord_lists_squares[i];
					move_list[n_moves].end = prom_coord_lists[i].coords[j];
					move_list[n_moves].code = piece;
					
					n_moves += prom_coord_lists[i].coords[j] < 64;
				}
			}
		}
	}
	// Castling and En Passant Moves
	for (int i = 0; i < 6; i++) {
		move_list[n_moves] = special_moves[i];
		n_moves += special_moves[i].code != Move::NULL_MOVE;
	}
	
	output->n_moves = n_moves;
	output->valid = true;
}

MoveList MoveGenerator::GetUnmoves(const BoardComposite * board) {
	MoveList output;
	GetUnmoves(board, &output);
	return output;
}

void MoveGenerator::GetUnmoves(const BoardComposite * board, MoveList * output) {
	Bitboard_t friendly, enemy;
	uint8_t piece;
	
	static int coord_lists_squares[256];
	static CoordList coord_lists[256];
	int n_coord_lists = 0;
	
	int n_prom_coord_lists = 0;
	
	// If black to move, looking for white moves that would have gotten us there
	if (!board->state.white_to_move)
	{
		friendly = board->white | board->black;
		enemy = 0;
		
		for (int square = 0; square < 64; square++) {
			
			piece = board->state.squares[square];
			
			if (piece == EMPTY || piece > 8
				|| piece == WHITE_PAWN || piece == BLACK_PAWN) continue;

			else if (piece == WHITE_KNIGHT) {
				coord_lists[n_coord_lists] = GetNMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
			else if (piece == WHITE_BISHOP) {
				coord_lists[n_coord_lists] = GetD1Moves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetD2Moves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
			else if (piece == WHITE_ROOK) {
				coord_lists[n_coord_lists] = GetHMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetVMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
			else if (piece == WHITE_QUEEN) {
				coord_lists[n_coord_lists] = GetD1Moves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetD2Moves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetHMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetVMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
			else if (piece == WHITE_KING) {
				coord_lists[n_coord_lists] = GetKMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
		}
	}
	// If white to move, looking for black moves that would have gotten us there
	else
	{
		friendly = board->black | board->white;
		enemy = 0;
		
		for (int square = 0; square < 64; square++) {
			
			piece = board->state.squares[square];
			
			if (piece == EMPTY || piece < 8
				|| piece == WHITE_PAWN || piece == BLACK_PAWN) continue;
			
			else if (piece == BLACK_KNIGHT) {
				coord_lists[n_coord_lists] = GetNMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
			else if (piece == BLACK_BISHOP) {
				coord_lists[n_coord_lists] = GetD1Moves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetD2Moves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
			else if (piece == BLACK_ROOK) {
				coord_lists[n_coord_lists] = GetHMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetVMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
			else if (piece == BLACK_QUEEN) {
				coord_lists[n_coord_lists] = GetD1Moves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetD2Moves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetHMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
				coord_lists[n_coord_lists] = GetVMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
			else if (piece == BLACK_KING) {
				coord_lists[n_coord_lists] = GetKMoves(friendly, enemy, square);
				coord_lists_squares[n_coord_lists++] = square;
			}
		}
	}
	
	// Allocate Memory
	uint16_t n_alloc = (n_coord_lists + n_prom_coord_lists) * 8 + 6;
	if (output->moves) {
		// If there is not enough memory, free and allocate later
		if (output->n_alloc < n_alloc) {
			delete[] output->moves;
			output->moves = NULL;
			output->n_moves = 0;
			output->n_alloc = 0;
		}
	}
	// Allocate fresh/new memory
	if (!output->moves) {
		output->moves = new Move[n_alloc];
		output->n_moves = 0;
		output->n_alloc = n_alloc;
	}
	
	// Get variables for assigning output
	Move * move_list = output->moves;
	uint16_t n_moves = 0;
	
	// Normal Moves
	for (int i = 0; i < n_coord_lists; i++) {
		//std::cout << (int)coord_lists_squares[i] << ": " << coord_lists[i] << std::endl;
		
		for (int j = 0; j < 8; j++) {
			move_list[n_moves].start = coord_lists_squares[i];
			move_list[n_moves].end = coord_lists[i].coords[j];
			move_list[n_moves].code = Move::NORMAL_MOVE;
			
			n_moves += coord_lists[i].coords[j] < 64;
		}
	}
	
	output->n_moves = n_moves;
	output->valid = true;
}

bool MoveGenerator::InCheck(const BoardComposite * board, bool is_white) {
	Bitboard_t enemy_pieces = is_white ? board->black : board->white;
	Bitboard_t enemy_pawns = is_white ? board->bpawns : board->wpawns;
	Bitboard_t enemy_pieces_no_pawns = enemy_pieces & ~enemy_pawns;
	Bitboard_t friendly_pieces = is_white ? board->white : board->black;
	
	int king_square = is_white ? board->wking_pos : board->bking_pos;
	
	// For each piece, checks if a check is possible using bitboards
	// If not, then skips; if so, then does a more in-depth check
	
	// Knight checks
	if (n_masks[king_square] & enemy_pieces_no_pawns) {
		CoordList knight_checks = GetNMoves(friendly_pieces, enemy_pieces, king_square);
		for (int i = 0; i < 8; i++) {
			if (knight_checks.coords[i] < 64 && 
				(board->state.squares[knight_checks.coords[i]] & 7) == WHITE_KNIGHT)
			{
				return true;
			}
		}
	}
	
	// King checks
	if (k_masks[king_square] & enemy_pieces_no_pawns) {
		CoordList king_checks = GetKMoves(friendly_pieces, enemy_pieces, king_square);
		for (int i = 0; i < 8; i++) {
			if (king_checks.coords[i] < 64 && 
				(board->state.squares[king_checks.coords[i]] & 7) == WHITE_KING)
			{
				return true;
			}
		}
	}

	// Rook checks
	if (h_masks[king_square] & enemy_pieces_no_pawns) {
		CoordList h_checks = GetHMoves(friendly_pieces, enemy_pieces, king_square);
		for (int i = 0; i < 8; i++) {
			if (h_checks.coords[i] < 64) {
				uint8_t piece = board->state.squares[h_checks.coords[i]] & 7;
				if (piece == WHITE_ROOK || piece == WHITE_QUEEN) return true;
			}
		}
	}
	if (v_masks[king_square] & enemy_pieces_no_pawns) {
		CoordList v_checks = GetVMoves(friendly_pieces, enemy_pieces, king_square);
		for (int i = 0; i < 8; i++) {
			if (v_checks.coords[i] < 64) {
				uint8_t piece = board->state.squares[v_checks.coords[i]] & 7;
				if (piece == WHITE_ROOK || piece == WHITE_QUEEN) return true;
			}
		}
	}
	
	// Bishop checks
	if (d1_masks[king_square] & enemy_pieces_no_pawns) {
		CoordList d1_checks = GetD1Moves(friendly_pieces, enemy_pieces, king_square);
		for (int i = 0; i < 8; i++) {
			if (d1_checks.coords[i] < 64) {
				uint8_t piece = board->state.squares[d1_checks.coords[i]] & 7;
				if (piece == WHITE_BISHOP || piece == WHITE_QUEEN) return true;
			}
		}
	}
	if (d2_masks[king_square] & enemy_pieces_no_pawns) {
		CoordList d2_checks = GetD2Moves(friendly_pieces, enemy_pieces, king_square);
		//std::cout << d2_checks << std::endl;
		for (int i = 0; i < 8; i++) {
			if (d2_checks.coords[i] < 64) {
				uint8_t piece = board->state.squares[d2_checks.coords[i]] & 7;
				if (piece == WHITE_BISHOP || piece == WHITE_QUEEN) return true;
			}
		}
	}
	
	// Pawn checks
	if (king_square % 8 != 0) {
		if ((is_white && king_square < 48) || (!is_white && king_square >= 16)) {
			uint8_t target_square = is_white ? king_square + 7 : king_square - 9;
			if (board->state.squares[target_square] == (is_white ? BLACK_PAWN : WHITE_PAWN)) return true;
		}
	}
	if (king_square % 8 != 7) {
		if ((is_white && king_square < 48) || (!is_white && king_square >= 16)) {
			uint8_t target_square = is_white ? king_square + 9 : king_square - 7;
			if (board->state.squares[target_square] == (is_white ? BLACK_PAWN : WHITE_PAWN)) return true;
		}
	}
	
	return false;
}

CoordList MoveGenerator::row2list_table[256];
CoordList MoveGenerator::friendly_table[256][8];
CoordList MoveGenerator::enemy_table[256][8];

Bitboard_t MoveGenerator::h_masks[64];
Bitboard_t MoveGenerator::v_masks[64];
Bitboard_t MoveGenerator::r_masks[64];
Bitboard_t MoveGenerator::d1_masks[64];
Bitboard_t MoveGenerator::d2_masks[64];
Bitboard_t MoveGenerator::b_masks[64];
Bitboard_t MoveGenerator::q_masks[64];
Bitboard_t MoveGenerator::n_masks[64];
Bitboard_t MoveGenerator::k_masks[64];

Bitboard_t MoveGenerator::CoordList2Bitboard(CoordList coords) {
	Bitboard_t output = 0;
	for (int i = 0; i < 8; i++) {
		if (coords.coords[i] < 64) output |= (Bitboard_t)1 << coords.coords[i];
	}
	return output;
}

void MoveGenerator::Init() {
	// Row-to-list table initialization
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 8; j++) {
			if (i & (1 << j)) row2list_table[i].coords[j] = j;
			else row2list_table[i].coords[j] = 0x7f;
		}
		
		for (int j = 0; j < 8; j++) {
			enemy_table[i][j].data = 0x7f7f7f7f7f7f7f7f;
			friendly_table[i][j].data = 0x7f7f7f7f7f7f7f7f;
			for (int k = j - 1; k >= 0; k--) {
				enemy_table[i][j].coords[k] = k;
				if (i & (1 << k)) break;
				else friendly_table[i][j].coords[k] = k;
			}
			
			for (int k = j + 1; k < 8; k++) {
				enemy_table[i][j].coords[k] = k;
				if (i & (1 << k)) break;
				else friendly_table[i][j].coords[k] = k;
			}
		}
	}
	
	// Piece mask initialization
	for (int i = 0; i < 64; i++) {
		h_masks[i] = CoordList2Bitboard(GetHMoves(0, 0, i));
		v_masks[i] = CoordList2Bitboard(GetVMoves(0, 0, i));
		r_masks[i] = h_masks[i] | v_masks[i];
		d1_masks[i] = CoordList2Bitboard(GetD1Moves(0, 0, i));
		d2_masks[i] = CoordList2Bitboard(GetD2Moves(0, 0, i));
		b_masks[i] = d1_masks[i] | d2_masks[i];
		q_masks[i] = r_masks[i] | b_masks[i];
		n_masks[i] = CoordList2Bitboard(GetNMoves(0, 0, i));
		k_masks[i] = CoordList2Bitboard(GetKMoves(0, 0, i));
	}
}

CoordList MoveGenerator::GetHMoves(Bitboard_t friendly, Bitboard_t enemy, uint8_t square) {
	int shift = square & 56;
	int file = square & 7;
	CoordList f_shifted = friendly_table[(friendly >> shift) & 0xff][file];
	CoordList e_shifted = enemy_table[(enemy >> shift) & 0xff][file];
	CoordList output;
	output.data = f_shifted.data | e_shifted.data; // Has same effect as | since empty bytes are 0xff
	output.data += 0x0101010101010101 * shift;
	return output;
}

CoordList MoveGenerator::GetVMoves(Bitboard_t friendly, Bitboard_t enemy, uint8_t square) {
	int file = square & 7;
	int rank = square / 8;
	CoordList f_shifted = friendly_table[V2Row_Transform(friendly >> file)][rank];
	CoordList e_shifted = enemy_table[V2Row_Transform(enemy >> file)][rank];
	CoordList output;
	output.data = f_shifted.data | e_shifted.data;
	output.data += 0x312a231c150e0700;
	output.data += 0x0101010101010101 * file;
	return output;
}

CoordList MoveGenerator::GetD1Moves(Bitboard_t friendly, Bitboard_t enemy, uint8_t square) {
	int file = square & 7;
	int rank = square / 8;
	
	Bitboard_t f_shifted = friendly;
	Bitboard_t e_shifted = enemy;
	
	// Variables depend upon the coord being on the top/bottom half of diagonal
	int diff = 0;
	Bitboard_t shift_back = 0;
	int table_index = file;
	
	if (rank > file) {
		diff = rank - file;
		f_shifted = friendly >> (diff * 8);
		e_shifted = enemy >> (diff * 8);
		shift_back = 0x0808080808080808 * diff;
		table_index = file;
	} else if (file > rank) {
		diff = file - rank;
		f_shifted = friendly >> diff;
		e_shifted = enemy >> diff;
		shift_back = 0x0101010101010101 * diff;
		table_index = rank;
	}
	
	CoordList f_row = friendly_table[D12Row_Transform(f_shifted)][table_index];
	CoordList e_row = enemy_table[D12Row_Transform(e_shifted)][table_index];
	CoordList output;
	output.data = f_row.data | e_row.data;
	//			   Row to diagonal	    Offset from diag
	output.data += 0x3830282018100800 + shift_back;
	// Restrict the domain of the output
	output.data |= ~(0xffffffffffffffff >> (diff * 8));
	return output;
}

CoordList MoveGenerator::GetD2Moves(Bitboard_t friendly, Bitboard_t enemy, uint8_t square) {
	int file = square & 7;
	int rank = square / 8;
	int shift_coef = rank + file - 7;
	int table_index = file;
	
	Bitboard_t f_shifted = friendly;
	Bitboard_t e_shifted = enemy;

	/*
	if (shift_coef > 0) {
		f_shifted = friendly >> (shift_coef * 8);
		e_shifted = enemy >> (shift_coef * 8);
	} else if (shift_coef < 0) {
		f_shifted = friendly << (-shift_coef * 8);
		e_shifted = enemy << (-shift_coef * 8);
	}
	
	CoordList f_row = friendly_table[D22Row_Transform(f_shifted)][file];
	CoordList e_row = enemy_table[D22Row_Transform(e_shifted)][file];
	CoordList output;
	output.data = f_row.data | e_row.data;
	//			   Row to diagonal	    Offset from diag
	output.data += 0x0008101820283038;
	if (shift_coef > 0) {
		output.data += 0x0808080808080808 * shift_coef;
		output.data |= ~(0xffffffffffffffff << (shift_coef * 8));
	}
	else {
		output.data += 0x0808080808080808 * -shift_coef;
		output.data |= ~(0xffffffffffffffff >> (-shift_coef * 8));
	}
	std::cout << output << std::endl;
	return output;
	*/
	
	if (shift_coef > 0) {
		f_shifted = friendly >> shift_coef;
		e_shifted = enemy >> shift_coef;
		table_index = 7 - rank;
	} else if (shift_coef < 0) {
		f_shifted = friendly << (-shift_coef * 8);
		e_shifted = enemy << (-shift_coef * 8);
		table_index = file;
	}
	
	//PrintBitboard(e_shifted);
	
	CoordList f_row = friendly_table[D22Row_Transform(f_shifted)][table_index];
	CoordList e_row = enemy_table[D22Row_Transform(e_shifted)][table_index];
	
	//std::cout << e_row << std::endl;
	
	CoordList output;
	output.data = f_row.data | e_row.data;
	output.data += 0x0008101820283038;
	
	//std::cout << output << std::endl;
	
	if (shift_coef > 0) {
		output.data += 0x0101010101010101 * shift_coef;
		output.data |= ~(0xffffffffffffffff >> ((shift_coef) * 8));
		//CoordList mask;
		//mask.data = ~(0xffffffffffffffff >> ((shift_coef) * 8));
		//std::cout << mask << std::endl;
		//std::cout << output << std::endl;
	} else if (shift_coef < 0) {
		output.data -= 0x0808080808080808 * (-shift_coef);
		output.data |= ~(0xffffffffffffffff >> ((-shift_coef) * 8));
		//CoordList mask;
		//mask.data = ~(0xffffffffffffffff >> ((shift_coef) * 8));
		//std::cout << mask << std::endl;
		//std::cout << output << std::endl;
	}
	
	//std::cout << output << std::endl;
	
	return output;
}

CoordList MoveGenerator::GetNMoves(Bitboard_t friendly, Bitboard_t enemy, uint8_t square) {
	int file = square & 7;
	int rank = square / 8;
	
	int shift = 0;
	
	shift += 2 - file;
	shift += (2 - rank) * 8;
	
	const Bitboard_t rank_limits[8] = {
		0x0000000000ffffff, 0x00000000ffffffff, 0x000000ffffffffff, 0x0000ffffffffff00,
		0x00ffffffffff0000, 0xffffffffff000000, 0xffffffff00000000, 0xffffff0000000000 };
	const Bitboard_t file_limits[8] = {
		0x0707070707070707, 0x0f0f0f0f0f0f0f0f, 0x1f1f1f1f1f1f1f1f, 0x3e3e3e3e3e3e3e3e,
		0x7c7c7c7c7c7c7c7c, 0xf8f8f8f8f8f8f8f8, 0xf0f0f0f0f0f0f0f0, 0x7070707070707070 };
	
	Bitboard_t shifted = ~friendly & rank_limits[rank] & file_limits[file];
	
	if (shift < 0) {
		shifted = shifted >> -shift;
	} else {
		shifted = shifted << shift;
	}
	
	CoordList row = row2list_table[N2Row_Transform(shifted)];
	row.data += 0x151d071d15010701;
	if (shifted < 0) {
		row.data += 0x0101010101010101 * -shift;
	} else {
		row.data -= 0x0101010101010101 * shift;
	}
	
	return row;
}

CoordList MoveGenerator::GetKMoves(Bitboard_t friendly, Bitboard_t enemy, uint8_t square) {
	int file = square & 7;
	int rank = square / 8;
	
	int shift = 0;
	
	shift += 1 - file;
	shift += (1 - rank) * 8;
	
	const Bitboard_t rank_limits[8] = {
		0x000000000000ffff, 0x0000000000ffffff, 0x00000000ffffff00, 0x000000ffffff0000,
		0x0000ffffff000000, 0x00ffffff00000000, 0xffffff0000000000, 0xffff000000000000 };
	const Bitboard_t file_limits[8] = {
		0x0303030303030303, 0x0707070707070707, 0x0e0e0e0e0e0e0e0e, 0x1c1c1c1c1c1c1c1c,
		0x3838383838383838, 0x7070707070707070, 0xe0e0e0e0e0e0e0e0, 0xc0c0c0c0c0c0c0c0 };
	
	Bitboard_t shifted = ~friendly & rank_limits[rank] & file_limits[file];
	
	if (shift < 0) {
		shifted = shifted >> -shift;
	} else {
		shifted = shifted << shift;
	}
	
	CoordList row = row2list_table[K2Row_Transform(shifted)];
	row.data += 0x0b0b0b0605000000;
	if (shifted < 0) {
		row.data += 0x0101010101010101 * -shift;
	} else {
		row.data -= 0x0101010101010101 * shift;
	}
	
	return row;
}

CoordList MoveGenerator::GetWPMoves(Bitboard_t friendly, Bitboard_t enemy, uint8_t square) {
	int file = square & 7;
	int rank = square / 8;
	
	CoordList output;
	if (~(friendly | enemy) & ((Bitboard_t)1 << (square + 8))) {
		output.coords[0] = square + 8;
		if (rank == 1 && (~(friendly | enemy) & ((Bitboard_t)1 << (square + 16)))) {
			output.coords[1] = square + 16;
		}
	}
	
	if (file != 0 && (enemy & ((Bitboard_t)1 << (square + 7)))) {
		output.coords[2] = square + 7;
	}
	if (file != 7 && (enemy & ((Bitboard_t)1 << (square + 9)))) {
		output.coords[3] = square + 9;
	}
	
	return output;
}

CoordList MoveGenerator::GetBPMoves(Bitboard_t friendly, Bitboard_t enemy, uint8_t square) {
	int file = square & 7;
	int rank = square / 8;
	
	CoordList output;
	if (~(friendly | enemy) & ((Bitboard_t)1 << (square - 8))) {
		output.coords[0] = square - 8;
		if (rank == 6 && (~(friendly | enemy) & ((Bitboard_t)1 << (square - 16)))) {
			output.coords[1] = square - 16;
		}
	}
	
	if (file != 0 && (enemy & ((Bitboard_t)1 << (square - 9)))) {
		output.coords[2] = square - 9;
	}
	if (file != 7 && (enemy & ((Bitboard_t)1 << (square - 7)))) {
		output.coords[3] = square - 7;
	}
	
	return output;
}

Bitboard_t MoveGenerator::V2Row_Transform(Bitboard_t x) {
	x &= 0x0101010101010101;
	x |= (x & 0xffffffff00000000) >> 28;
	x |= (x & 0x00000000ffff0000) >> 14;
	x |= (x & 0x000000000000ff00) >> 7;
	return x & 0xff;
}

Bitboard_t MoveGenerator::D12Row_Transform(Bitboard_t x) {
	x &= 0x8040201008040201;
	x |= (x & 0xffffffff00000000) >> 32;
	x |= (x & 0x00000000ffff0000) >> 16;
	x |= (x & 0x000000000000ff00) >> 8;
	return x & 0xff;
}

Bitboard_t MoveGenerator::D22Row_Transform(Bitboard_t x) {
	x &= 0x0102040810204080;
	x |= (x & 0xffffffff00000000) >> 32;
	x |= (x & 0x00000000ffff0000) >> 16;
	x |= (x & 0x000000000000ff00) >> 8;
	return x & 0xff;
}

Bitboard_t MoveGenerator::N2Row_Transform(Bitboard_t x) {
	return
		((x >> 1) & 0x05) |
		((x >> 7) & 0x22) |
		((x >> 21) & 0x88) |
		((x >> 29) & 0x50);
}

Bitboard_t MoveGenerator::K2Row_Transform(Bitboard_t x) {
	return
		(x & 0x07) |
		((x >> 5) & 0x08) |
		((x >> 6) & 0x10) |
		((x >> 11) & 0xe0);
}



