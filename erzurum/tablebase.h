#ifndef _ERZURUM_TABLEBASE2_H_
#define _ERZURUM_TABLEBASE2_H_

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
			RESULT_INDETERMINED
		} Result;
		
		uint16_t result : 2;
		uint16_t distance : 14;
		Move move_to_next;
		
		friend std::ostream & operator << (std::ostream & os, Evaluation eval);
		friend std::ostream & operator << (std::ostream & os, std::vector<Evaluation> path);
	} __attribute__((__packed__));

protected:
	struct Node {
		typedef enum : uint16_t {
			RESULT_WHITE_WIN,
			RESULT_BLACK_WIN,
			RESULT_DRAW,
			RESULT_INDETERMINED
		} Result;
		
		typedef enum : uint16_t {
			STATUS_INVALID,
			STATUS_FRONTIER,
			STATUS_SOLVED
		} Status;
		uint16_t status : 2, result : 2, distance : 12;
		
		Node * next;
		Move move_to_next;
		
		friend std::ostream & operator << (std::ostream & os, Node node);
	} __attribute__((__packed__));
	
protected:
	std::map<BoardState, Node *> positions;
	std::vector<std::string> search_dirs;

/*******************************************************************************
 * Constructors
 */
public:
	TableBase();
	
	// Disable copy and assignment constructors to avoid internal pointer issues
	TableBase(TableBase & other) = delete;
	TableBase & operator = (TableBase other) = delete;
	
/*******************************************************************************
 * Generation
 */
public:
	void AddStaticPositions(void (*generation_function)(
		void (TableBase::*add_function)(BoardState state, uint8_t result)));
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
	void AddSearchDirectory(const char * dir);
	bool LoadFromDirectory(const char * dir);
	bool SaveToDirectory(const char * dir);
	
protected:

};

#endif