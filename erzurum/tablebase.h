#ifndef _ERZURUM_TABLEBASE_H_
#define _ERZURUM_TABLEBASE_H_

#include <board.h>

#include <map>
#include <string>

/**
 * @class TableBase
 * @author Daniel Winkelman
 * @date 03/05/18
 * @file tablebase.h
 * @brief Algorithm for generating endgame tablebases
 */
class TableBase {
/*******************************************************************************
 * Node
 */
public:
	struct Node {
		friend class TableBase;
		
	/***************************************************************************
	 * Position Evaluation
	 */
	public:
		const static uint8_t RESULT_WHITE_WIN = 1;
		const static uint8_t RESULT_DRAW = 2;
		const static uint8_t RESULT_BLACK_WIN = 3;
		const static uint8_t RESULT_UNDETERMINED = 0;
	
		uint8_t result : 2;
		uint16_t distance : 12;
		Node * next;
		Move move_to_next;
		
	/***************************************************************************
	 * Node Status Data
	 */
	public:
		const static uint8_t STATUS_UNINIT = 0;
		const static uint8_t STATUS_FRONTIER = 1;
		const static uint8_t STATUS_SOLVED = 2;
	
		uint8_t status : 2;
		MoveList moves;
		
	/***************************************************************************
	 * Initialization, Input/Output
	 */
	public:
		Node();
		Node(uint8_t result);
		Node(Node * next, Move move_to_next);
	};
	
/*******************************************************************************
 * Move Sequence
 */
public:
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
	typedef std::map<BoardState, Node *>::iterator NodeIterator;
	typedef std::pair<BoardState, Node *> NodePair;
	std::map<BoardState, Node *> positions;
	
public:
	~TableBase();
	int Count(uint8_t status);
	
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
	
	static const MoveList * GetNodeMoves(NodeIterator node_i);
	
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
	void ExpandNode(NodeIterator node_i);
	void Optimize();
	bool OptimizeNode(NodeIterator node);
	
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
	
	static std::string NodeToLine(const NodeIterator node_i);
	static NodePair InitNodeFromLine(const std::string line);
	
/*******************************************************************************
 * Console Display
 */
public:
	friend std::ostream & operator << (std::ostream & os, TableBase & tb);
	friend std::ostream & operator << (std::ostream & os, NodeIterator & ni);
};

#endif