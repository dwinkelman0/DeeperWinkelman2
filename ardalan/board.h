#ifndef _ARDALAN_BOARD_H_
#define _ARDALAN_BOARD_H_

#include "datatypes.h"
#include "movegen.h"

/**
 * @class Board
 * @author Daniel-Winkelman
 * @date 31/03/18
 * @file board.h
 * @brief A data structure for maintaining a complete game of chess.
 */
class Board {
protected:
	BoardComposite * current = NULL;
	BoardComposite * history_begin = NULL;
	uint16_t depth = 0;
	
	MoveGenerator mgen;
	
public:
	Board();
	Board(const Board & other) = delete;
	Board & operator = (const Board & other) = delete;
	~Board();
	
	/**
	 * @brief Completely change the current board state.
	 * @param state State to apply.
	 * @return Returns whether Board Composite initialization succeeded.
	 */
	inline bool SetCurrent(BoardState state) {
		// Current is guaranteed to exist because it is allocated in constructor
		return current->Init(state);
	}
	
	/**
	 * @brief Get the current board state.
	 * @return A fresh copy of the state.
	 */
	inline BoardState GetCurrent() const {
		// Current is guaranteed to exist because it is allocated in constructor
		return current->state;
	}
	
	/**
	 * @brief Get the initial board state.
	 * @return A fresh copy of the state.
	 */
	inline BoardState GetInitial() const {
		// History is guaranteed to exist because it is allocated in constructor
		return history_begin->state;
	}
	
	/**
	 * @brief Get the depth of the current position from the initial position.
	 * @return
	 */
	inline int GetDepth() const {
		return depth;
	}
	
	/**
	 * @brief Make a move that changes the board state.
	 * @param move Move to make.
	 * @return Returns whether the move was legal and if the state was changed.
	 * 
	 * Checks for the following conditions of move legality:
	 *  - The move is of a valid type (normal, en passant, castling, promotion)
	 *  - Either king is not captured
	 *  - The moved piece matches the color of the turn
	 *  - The piece is moved to a square that does not contain a color of the same piece
	 *  - CASTLING:
	 *     - The correct color is making the move
	 *     - The involved squares have the correct pieces or emptiness
	 *     - The king does not begin in check, travel through check, or land in check
	 *  - EN PASSANT:
	 *     - The correct color is making the move
	 *     - The original board state has a valid en passant target assigned
	 *     - The involved squares have the correct pieces or emptiness
	 * 
	 * NOTE: Only castling moves are verified for not landing in check (due to
	 * the requirement that the king not begin in or travel through check). All
	 * other moves must be validated separately.
	 */
	bool Make(Move move);
	
	/**
	 * @brief Shift the current position backwards through the move sequence.
	 * @param n_moves Number of moves to go back.
	 * @return Returns false if the requested number is greater than the depth.
	 */
	bool Unmake(uint16_t n_moves);
	
	/**
	 * @brief Generate the available pseudo-legal moves for whichever color to move.
	 * @return Returns an unalterable pointer to the cached moves associated with the current Board Composite.
	 * 
	 * Be careful to not refer to the returned pointer to a list unless the
	 * board is currently in a state such that the generated moves are valid.
	 */
	const MoveList * GetMoves();
	
	/**
	 * @brief Generate the available pseudo-legal moves that could have been just made by the other color.
	 * @return Returns an unalterable pointer to the cached moves associated with the current Board Composite.
	 * 
	 * Be careful to not refer to the returned pointer to a list unless the
	 * board is currently in a state such that the generated moves are valid.
	 */
	const MoveList * GetUnmoves();
	
	/**
	 * @brief Determine whether the color to move has legal options.
	 * @return 
	 */
	bool HasLegalMoves();
	
	/**
	 * @brief Determine whether the specified color is in check.
	 * @param is_white True to check for white being in check, false for black.
	 * @return 
	 */
	bool InCheck(bool is_white);
	
	/**
	 * @brief Determine whether the color to move is in checkmate.
	 * @return 
	 */
	bool IsCheckmate();
	
	/**
	 * @brief Determine whether the color to move is in stalemate.
	 * @return 
	 */
	bool IsStalemate();
	
	/**
	 * @brief Determine whether a draw can be claimed from insufficient material.
	 * @return 
	 */
	bool IsDrawByInsufficientMaterial();
	
	/**
	 * @brief Determine whether a draw can be claimed from lack of progress.
	 * @return 
	 */
	bool IsDrawByNoProgress();
	
	/**
	 * @brief Determine whether the current position is a repetition.
	 * @return 
	 */
	bool IsDrawByRepetition();
	
	/**
	 * @brief Check if any draw conditions are met.
	 * @return 
	 * 
	 * Draw conditions include Stalemate, Insufficient Material, Lack of
	 * Progress, and Repetition (2-fold included).
	 */
	bool IsDraw();
	
protected:
	bool MakeComplete(const BoardComposite * orig, BoardComposite * target,
		uint8_t start, uint8_t end, uint8_t start_piece, uint8_t end_piece, uint8_t promotion_piece);
	bool MakeNormal(const BoardComposite * orig, BoardComposite * target, Move move);
	bool MakeCastling(const BoardComposite * orig, BoardComposite * target, Move move);
	bool MakeEnPassant(const BoardComposite * orig, BoardComposite * target, Move move);
	bool MakePromotion(const BoardComposite * orig, BoardComposite * target, Move move);
	
public:
	friend std::ostream & operator << (std::ostream & os, const Board & board);
};

#endif