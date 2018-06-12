#ifndef _ERZURUM_TABLEBASE_H_
#define _ERZURUM_TABLEBASE_H_

#include <board.h>

#include <map>
#include <queue>
#include <set>
#include <string>

/**
 * @class TableBase
 * @author Daniel Winkelman
 * @date 03/05/18
 * @file tablebase.h
 * @brief Algorithm for generating endgame tablebases
 */
class TableBase {
public:
	/**
	 * @class Node
	 * @author Daniel Winkelman
	 * @date 03/05/18
	 * @file tablebase.h
	 * @brief 
	 */
	struct Node {
	public:
		const static uint8_t RESULT_WHITE_WIN = 1;
		const static uint8_t RESULT_DRAW = 2;
		const static uint8_t RESULT_BLACK_WIN = 3;
		const static uint8_t RESULT_UNDETERMINED = 0;
		
		BoardState state;
		uint8_t result : 2;
		uint16_t distance : 14;
		Node * next;
		Move move_to_next;
		MoveList moves;
		
		Node();
		Node(BoardState state, uint8_t result);
		Node(BoardState state, Node * next, Move move_to_next);
		Node(std::string line);
		
		std::string ToString();
		
		const MoveList * GetMoves();
		
		friend std::ostream & operator << (std::ostream & os, const Node & node);
	};
	
	/**
	 * @class MoveSequence
	 * @author Daniel Winkelman
	 * @date 08/06/18
	 * @file tablebase.h
	 * @brief 
	 */
	struct MoveSequence {
	protected:
		friend class TableBase;

		Move * moves = NULL;
		uint16_t n_alloc = 0;
		uint16_t n_moves = 0;
		BoardState initial_state;
		uint8_t result;
		
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
		inline uint8_t Result() const {
			return result;
		}
		inline void Alloc(int n_moves) {
			this->moves = new Move[n_moves];
			this->n_alloc = n_moves;
		}
		
	public:
		MoveSequence();
		MoveSequence(BoardState state);
		MoveSequence(const MoveSequence & other);
		MoveSequence & operator =(const MoveSequence & other);
		~MoveSequence();
		
	public:
		friend std::ostream & operator << (std::ostream & os, const MoveSequence & ml);
	};

/*******************************************************************************
 * Data Storage
 */
protected:
	typedef std::pair<BoardState, Node*> SolvedPosition;
	
	std::map<BoardState, Node*> solved;
	std::queue<BoardState> frontier;
	std::set<BoardState> frontier_unique;
	
public:
	~TableBase();
	
/*******************************************************************************
 * Evaluation
 */
public:
	Node * Evaluate(BoardState state);
	const Node * EvaluateFromFile(BoardState state, const char * path);
	
	MoveSequence EvaluateSequence(BoardState state);
	MoveSequence EvaluateSequenceFromFile(BoardState state, const char * path);
	
/*******************************************************************************
 * Node Management
 */
public:
	void AddTableBase(TableBase & tb);
	const Node * AddStaticallySolved(BoardState state, uint8_t result);
	
protected:
	const Node * AddLinkedSolved(BoardState state, Node * next, Move move_to_next);
	bool AddUnsolvedToFrontier(BoardState state);
	void AddUnmovesToFrontier(BoardState state);
	
/*******************************************************************************
 * Node Generation Declarations
 */
protected:
	friend TableBase GenerateKings();
	friend TableBase GenerateRookAndKing();
	friend TableBase GenerateTwoBishopsAndKing();
	
	friend void PatchRookAndKing(TableBase & tb);

/*******************************************************************************
 * Generation Algorithms
 */
public:
	void Expand();
	void Optimize();
	bool OptimizeNode(Node * node);
	
	void ClearFrontier();
	
/*******************************************************************************
 * File Input/Output
 */
public:
	void Save(const char * path);
	void Load(const char * path);
	void LoadFile(const char * path, uint8_t wking, uint8_t bking, bool white_to_move);
	
protected:
	static const char * GetFilename(uint8_t wking, uint8_t bking, bool white_to_move);
	static void GetKingPositions(BoardState board, uint8_t * wking, uint8_t * bking);
	
/*******************************************************************************
 * Console Display
 */
public:
	friend std::ostream & operator << (std::ostream & os, const TableBase & tb);
};

#endif