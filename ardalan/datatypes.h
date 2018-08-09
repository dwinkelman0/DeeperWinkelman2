#ifndef _ARDALAN_DATATYPES_H_
#define _ARDALAN_DATATYPES_H_

#include "hash.h"

#include <stdint.h>
#include <string>

#define EMPTY			0
#define WHITE_PAWN		1
#define WHITE_KNIGHT	2
#define WHITE_BISHOP	3
#define WHITE_ROOK		4
#define WHITE_QUEEN		5
#define WHITE_KING		6
#define BLACK_PAWN		9
#define BLACK_KNIGHT	10
#define BLACK_BISHOP	11
#define BLACK_ROOK		12
#define BLACK_QUEEN		13
#define BLACK_KING		14

typedef uint64_t Bitboard_t;
typedef int16_t Score_t;
typedef uint64_t Hash_t;

uint8_t Text2Coord(const char * text);
std::string Coord2Text(uint8_t coord);

/**
 * @class CoordList
 * @author Daniel-Winkelman
 * @date 25/03/18
 * @file datatypes.h
 * @brief 64-bit representation of a set of up to 8 moves. Used only internally.
 * 
 * Long integers are loaded into this data type where they can be manipulated
 * using bitwise/addition/subtraction operators. Individual char-sized square
 * coordinates can be accessed through the union array.
 * 
 * The convention is that valid squares have values between 0 and 63 inclusive,
 * while invalid squares can have ANYTHING else.
 */
struct CoordList {
public:
	union {
		uint8_t coords[8];
		uint64_t data;
	};
	
	CoordList();
	uint8_t Count();
	friend std::ostream & operator << (std::ostream & os, CoordList cl);
};

/**
 * @class Move
 * @author Daniel-Winkelman
 * @date 25/03/18
 * @file datatypes.h
 * @brief The move of a piece from a start coord to an end coord/special moves.
 * 
 * code == NULL_MOVE: none of the other states matter. Default state.
 * code == NORMAL_MOVE: start and end squares function normally.
 * code == [WHITE/BLACK]_[OO/OOO]: castling, none of the other states matter.
 * code == EN_PASSANT: start and end are the start and end of the pawn to move.
 * code == 14: invalid state
 * code == anything else: pawn moves from start to end and promotes to <code>.
 */
struct Move {
public:
	uint8_t start : 6, end : 6, code : 4;
	
	Move();
	Move(uint8_t start, uint8_t end, uint8_t code);
	Move(const char * str);
	bool operator == (const Move & other) const;
	friend std::ostream & operator << (std::ostream & os, Move m);
	
	static const uint8_t NORMAL_MOVE = 0;
	static const uint8_t EN_PASSANT = 1;
	static const uint8_t WHITE_OO = 6;
	static const uint8_t WHITE_OOO = 7;
	static const uint8_t BLACK_OO = 8;
	static const uint8_t BLACK_OOO = 9;
	static const uint8_t NULL_MOVE = 15;
} __attribute__((__packed__));

/**
 * @class MoveList
 * @author Daniel-Winkelman
 * @date 25/03/18
 * @file datatypes.h
 * @brief Stores all the pseudo-legal moves that can be made in a position.
 * 
 * Moves are listed serially. Memory is dynamically (but excessively) allocated
 * by the move generator, but can be freed whenever convenient. Moves in this
 * array are not guaranteed to be legal insofar as concerning check rules (like
 * moving into check, castling through check, undoing pins, etc.).
 * 
 * The list is accessed globally using STL-style begin/end functions, and direct
 * writing access is limited to the move generator.
 */
struct MoveList {
protected:
	friend class MoveGenerator;
	
	Move * moves = NULL;
	uint16_t n_alloc = 0;
	uint16_t n_moves = 0;
	bool valid = false;
	
public:
	inline const Move * Begin() const {
		return moves;
	}
	inline const Move * End() const {
		return moves + n_moves;
	}
	inline int Length() const {
		return n_moves;
	}
	inline void Clear() {
		this->n_moves = 0;
		this->valid = false;
	}
	inline bool IsValid() const {
		return this->valid;
	}
	
public:
	MoveList();
	MoveList(const MoveList & other);
	MoveList & operator =(const MoveList & other);
	~MoveList();
	
public:
	friend std::ostream & operator << (std::ostream & os, const MoveList & ml);
} __attribute__((__packed__));

/**
 * @class BoardState
 * @author Daniel-Winkelman
 * @date 25/03/18
 * @file datatypes.h
 * @brief The minimal set of data that defines the current game state.
 * 
 * This structure has four sets of essential data (which can be used for direct
 * comparisons to check for positional equality) and one set of non-essential
 * data (which is necessary for a complete positional representation but is
 * misleading for comparison).
 * 
 * ESSENTIAL: square array, color to move, castling rights, en passant status
 * NON-ESSENTIAL: number of ply since last progress.
 * 
 * En passant status lists the pawn that can be captured via en passant. It is
 * 0 if there is no target square.
 */
struct BoardState {
protected:
	static uint8_t DEFAULT_SQUARES[64];
	
public:
	// Information that can be used to determine identical positions
	uint8_t squares[64] = {
		4, 2, 3, 5, 6, 3, 2, 4,
		1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		9, 9, 9, 9, 9, 9, 9, 9,
		12,10,11,13,14,11,10,12
	};
	bool white_to_move = true;
	bool white_OO = true, white_OOO = true, black_OO = true, black_OOO = true;
	uint8_t ep_target = 0;
	
	// Information that is still essential for a valid game state but does not
	// contribute to positional uniqueness
	uint8_t n_ply_without_progress = 0;
	
public:
	// Compare critical states
	int Compare(BoardState other) const;
	bool operator == (BoardState other) const;
	bool operator < (BoardState other) const;
	bool operator > (BoardState other) const;
	bool operator <= (BoardState other) const;
	bool operator >= (BoardState other) const;
	
public:
	bool InitFromFEN(const char * fen);
	std::string GetFEN() const;
	
	Hash_t GetHash() const;
	
	// Output for visualization
	friend std::ostream & operator << (std::ostream & os, const BoardState & bc);
};

/**
 * @class BoardComposite
 * @author Daniel-Winkelman
 * @date 25/03/18
 * @file datatypes.h
 * @brief Stores essential data and associated data for other purposes.
 * 
 * This structure contains a board state, 64-bit bitboards for move generation,
 * the positions of both kings for check detection, a roster of the counts of
 * each type of piece on the board, a cache of moves generated from this
 * position, and optionally a Zobrist hashing system to accelerate comparison
 * and a field for storing incremental score data.
 */
struct BoardComposite {
	#ifdef ARDALAN_DISCRETE_SCORING
	static const Score_t PIECE_SCORES[16];
	#endif

public:
	BoardState state;
	
	// Maintain bitboards representing white and black pieces
	Bitboard_t white = 0, black = 0;
	Bitboard_t wpawns = 0, bpawns = 0;
	
	// Maintain the positions of white and black kings to help check detection
	uint8_t wking_pos = 255, bking_pos = 255;
	
	// Maintain a 64-bit hash to accelerate position comparison
	Hash_t hash = 0;
	
	#ifdef ARDALAN_DISCRETE_SCORING
	// Maintain a running total of material to accelerate evaluation
	Score_t material_score = 0;
	#endif
	
	// Maintain a roster of the types of pieces on the board
	uint8_t roster[16] = { 0 };
	
	// Cache moves that have been found in the position
	MoveList move_cache;
	MoveList unmove_cache;
	
	// Linked-list-style move stack
	BoardComposite * next = NULL;
	BoardComposite * last = NULL;
	Move move_from_last;
	Move move_to_next;
	
public:

	bool Init(const BoardState state);
	
	inline bool operator == (const BoardComposite & other) const {
		// Short-circuit compare the hash
		return this->hash == other.hash && this->state == other.state;
	}
	
public:
	friend std::ostream & operator << (std::ostream & os, const BoardComposite & bc);

};

#endif