#ifndef _ARDALAN_MOVEGEN_H_
#define _ARDALAN_MOVEGEN_H_

#include "datatypes.h"

class MoveGenerator {
public:
	MoveGenerator();
	
	MoveList GetMoves(const BoardComposite * board);
	void GetMoves(const BoardComposite * board, MoveList * output);
	MoveList GetUnmoves(const BoardComposite * board);
	void GetUnmoves(const BoardComposite * board, MoveList * output);
	bool InCheck(const BoardComposite * board, bool is_white);
	
protected:
	CoordList GetHMoves(Bitboard_t friendly, Bitboard_t enemy, uint8_t square);
	CoordList GetVMoves(Bitboard_t friendly, Bitboard_t enemy, uint8_t square);
	CoordList GetD1Moves(Bitboard_t friendly, Bitboard_t enemy, uint8_t square);
	CoordList GetD2Moves(Bitboard_t friendly, Bitboard_t enemy, uint8_t square);
	CoordList GetNMoves(Bitboard_t friendly, Bitboard_t enemy, uint8_t square);
	CoordList GetKMoves(Bitboard_t friendly, Bitboard_t enemy, uint8_t square);
	CoordList GetWPMoves(Bitboard_t friendly, Bitboard_t enemy, uint8_t square);
	CoordList GetBPMoves(Bitboard_t friendly, Bitboard_t enemy, uint8_t square);
	
protected:
	static CoordList row2list_table[256];
	static CoordList friendly_table[256][8];
	static CoordList enemy_table[256][8];
	
	static Bitboard_t h_masks[64];
	static Bitboard_t v_masks[64];
	static Bitboard_t r_masks[64];
	static Bitboard_t d1_masks[64];
	static Bitboard_t d2_masks[64];
	static Bitboard_t b_masks[64];
	static Bitboard_t q_masks[64];
	static Bitboard_t n_masks[64];
	static Bitboard_t k_masks[64];
	
	static Bitboard_t CoordList2Bitboard(CoordList coords);
	
	void Init();
	
	static Bitboard_t V2Row_Transform(Bitboard_t x);
	static Bitboard_t D12Row_Transform(Bitboard_t x);
	static Bitboard_t D22Row_Transform(Bitboard_t x);
	static Bitboard_t N2Row_Transform(Bitboard_t x);
	static Bitboard_t K2Row_Transform(Bitboard_t x);
};

#endif