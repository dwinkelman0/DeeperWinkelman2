#ifndef _ERZURUM_TABLEBASE2_H_
#define _ERZURUM_TABLEBASE2_H_

#include <board.h>

#include <queue>
#include <set>
#include <vector>

class TableBase {
/*******************************************************************************
 * Output Structures
 */
public:
	const static uint8_t RESULT_UNDETERMINED = 0;
	const static uint8_t RESULT_WHITE_WIN = 1;
	const static uint8_t RESULT_BLACK_WIN = 2;
	const static uint8_t RESULT_DRAW = 3;

	struct Evaluation {
	public:
		uint8_t result : 2;
		uint16_t distance : 12;
		Move move_to_next;
	};
	typedef std::vector<Evaluation> Sequence;
	
	friend std::ostream & operator << (std::ostream & os, const Evaluation & eval);
	friend std::ostream & operator << (std::ostream & os, const Sequence & seq);

/*******************************************************************************
 * Internal Structures
 */
protected:
	struct Node {
	public:
		const static uint8_t STATUS_UNINIT = 0;
		const static uint8_t STATUS_FRONTIER = 1;
		const static uint8_t STATUS_SOLVED = 2;
	/***************************************************************************
	 * Data Fields
	 */
	public:
		BoardState state;
		
		uint8_t status : 2;
		uint8_t result : 2;
		uint16_t distance : 12;
		
		Move move_to_next;
		Node * next;
		
		MoveList moves;
	
	/***************************************************************************
	 * Constructors
	 */
	public:
		Node();
		~Node();
		
		static Node * NewStatic(const BoardState state, uint8_t result);
		static Node * NewLinked(const BoardState state, Node * next, Move move_to_next);
		static Node * NewFromLine(const std::string line);
		
	protected:
		Node(const Node & other);
		Node & operator =(const Node & other);
		
	/***************************************************************************
	 * Special Comparisons
	 * (for insertion into sets instead of as map iterators)
	 */
	static inline bool operator == (const Node & left, const Node & right) {
		return left.state == right.state;
	}
	static inline bool operator <  (const Node & left, const Node & right) {
		return left.state < right.state;
	}
		
	/***************************************************************************
	 * String-Based Output
	 */
	public:
		std::string ToLine() const;
		friend std::ostream & operator << (std::ostream & os, const Node & node);
		
	/***************************************************************************
	 * Miscellaneous
	 */
	public:
		const MoveList * GetMoves();
	};
	
/*******************************************************************************
 * Data Fields
 */
protected:
	std::queue<Node *> frontier;
	std::set<Node *> solved;
	
	std::string parent_dir_path;
	
public:
	int CountFrontier() const;
	int CountSolved() const;
	
/*******************************************************************************
 * Constructors
 */
public:
	TableBase(const char * path);
	~TableBase();
	
protected:
	TableBase(const TableBase & other) = delete;
	TableBase & operator =(const TableBase & other) = delete;
	
/*******************************************************************************
 * Evaluation
 */
public:
	Evaluation Evaluate(BoardState state);
	Evaluation EvaluateFromFile(BoardState state);
	
	Sequence EvaluateSequence(BoardState state);
	Sequence EvaluateSequenceFromFile(BoardState state);
	
/*******************************************************************************
 * Generation
 */
public:
	void Add_Kk_InitialPositions();
	void Add_KRk_InitialPositions();

public:
	void Expand();
	void Optimize();
	
	void AddTableBaseSolved(TableBase & tablebase);
	void AddTableBaseFrontier(TableBase & tablebase);
	
protected:
	bool ExpandNode();
	bool OptimizeNode();
	
	Node * AddStaticallySolved(const BoardState state, uint8_t result);
	Node * AddLinkedSolved(Node * position, Node * next, Move move_to_next);
	Node * AddToFrontier(const BoardState state);
	void AddUnmovesToFrontier(const BoardState state);
	
/*******************************************************************************
 * File Input/Output
 */
public:
	void Load();
	void Save();
	
protected:
	bool LoadFile(uint8_t wking, uint8_t bking);
	bool SaveFile(uint8_t wking, uint8_t bking);
	
	void GetKingPositions(BoardState board, uint8_t * wking, uint8_t * bking);
	const char * GetFilename(uint8_t wking, uint8_t bking, bool white_to_move);
};

#endif