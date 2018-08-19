#ifndef _ERZURUM_TABLEBASE_H_
#define _ERZURUM_TABLEBASE_H_

#include "ibst.h"

#include <board.h>

#include <iostream>
#include <map>
#include <vector>

class TableBase {
public:
	struct Evaluation {
		typedef enum : uint16_t {
			RESULT_WHITE_WIN,
			RESULT_BLACK_WIN,
			RESULT_DRAW,
			RESULT_UNDETERMINED
		} Result;
		
		uint16_t result : 2, distance : 13, white_to_move : 1;
		Move move_to_next;
		
		friend std::ostream & operator << (std::ostream & os, Evaluation eval);
		friend std::ostream & operator << (std::ostream & os, std::vector<Evaluation> path);
	} __attribute__((__packed__));

public:
	struct Node {
		typedef enum : uint16_t {
			RESULT_WHITE_WIN,
			RESULT_BLACK_WIN,
			RESULT_DRAW,
			RESULT_UNDETERMINED
		} Result;
		
		typedef enum : uint16_t {
			STATUS_INVALID,
			STATUS_FRONTIER,
			STATUS_SOLVED
		} Status;
		
		uint16_t status : 2, result : 2, distance : 12;
		Node * next;
		Move move_to_next;
		
		MoveList move_cache;
		Hash_t hash;
		
		friend std::ostream & operator << (std::ostream & os, Node node);
	} __attribute__((__packed__));
	
protected:
	#ifdef USE_STD_MAP
	typedef std::map<BoardState, Node *>::iterator PosIterator;
	std::map<BoardState, Node *> positions;
	#else
	static int positions_cmp_func(BoardState a, BoardState b) {
		return a.Compare(b);
	}
	typedef BST<BoardState, Node *>::Node * PosIterator;
	BST<BoardState, Node *> * positions;
	#endif

/*******************************************************************************
 * Constructors
 */
public:
	TableBase();
	~TableBase();
	
	// Disable copy and assignment constructors to avoid internal pointer issues
	//TableBase(TableBase & other) = delete;
	//TableBase & operator = (TableBase other) = delete;
	
/*******************************************************************************
 * Generation
 */
public:
	bool AddStaticallySolved(BoardState state, uint8_t result);
protected:
	bool AddFrontier(BoardState state);
	bool AddLinkedSolved(BoardState state, uint8_t result, Node * next, BoardState next_state, Move move_to_next);
	bool AddUnmovesToFrontier(BoardState state);
	
public:
	void Expand();
	void Optimize();
	
/*******************************************************************************
 * Evaluation
 */
public:
	Evaluation Evaluate(BoardState state);
	Evaluation EvaluateFromFile(BoardState state);
	std::vector<Evaluation> EvaluateSequence(BoardState state);
	std::vector<Evaluation> EvaluateSequenceFromFile(BoardState state);
	
/*******************************************************************************
 * File Input/Output
 */
public:
	bool ImportFromDirectory(const char * dir);
	bool SaveToDirectory(const char * dir);
	bool MergeToDirectory(const char * dir);
	
protected:
	std::map<BoardState, Node *> ImportFromFile(const char * dir);
	bool SaveToFile(const char * dir, std::map<BoardState, Node *> positions);
	
/*******************************************************************************
 * Utility and Display
 */
public:
	friend std::ostream & operator << (std::ostream & os, TableBase & tb);
	void PrintStrata();
};

#endif