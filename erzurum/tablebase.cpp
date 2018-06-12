#include "tablebase.h"

#include <movegen.h>

#include <forward_list>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>

TableBase::~TableBase() {
	NodeIterator node_i = positions.begin();
	NodeIterator node_end = positions.end();
	for (; node_i != node_end; node_i++) {
		delete node_i->second;
		node_i->second = NULL;
	}
	
	ClearFrontier();
}

int TableBase::Count(uint8_t status) {
	int count = 0;
	NodeIterator node_i = positions.begin();
	NodeIterator node_end = positions.end();
	for (; node_i != node_end; node_i++) {
		if (node_i->second->status == status) count++;
	}
	return count;
}

/**
 * @brief Get the node with solution data for the given position
 * @param state
 * @return NULL if there is no node for the position
 */
TableBase::Node * TableBase::Evaluate(BoardState state) {
	const NodeIterator node_i = positions.find(state);
	if (node_i == positions.end()) return NULL;
	else if (node_i->second->status != TableBase::Node::STATUS_SOLVED) return NULL;
	else return node_i->second;
}

/**
 * @brief Get the node with solution data for the given position
 * @param state
 * @param path Parent folder for table base files
 * @return NULL if there is no node for the position
 * 
 * First checks the existing loaded positions to see is it was already read.
 * If not, loads the necessary file and retrieves the position (if it exists).
 */
const TableBase::Node * TableBase::EvaluateFromFile(BoardState state, const char * path) {
	// 1. Check existing positions
	const Node * node = Evaluate(state);
	if (node) {
		return node;
	}
	
	// 2. Load file with king/color configuration
	uint8_t wking, bking;
	GetKingPositions(state, &wking, &bking);
	LoadFile(path, wking, bking, state.white_to_move);
	
	// 3. Check positions again; return NULL if not found
	return Evaluate(state);
}

TableBase::MoveSequence TableBase::EvaluateSequence(BoardState state) {
	MoveSequence output(state);
	
	// Get the first node
	const Node * current = Evaluate(state);
	if (!current)  {
		// Return an empty, unevaluated state if not found
		return output;
	}
	
	// Set the initial conditions
	output.Alloc(current->distance);
	output.result = current->result;
	
	// Step through move sequence
	Board board;
	board.SetCurrent(state);
	Move * move_i = output.moves;
	const Move * move_end = output.moves + output.n_alloc;
	while (current && move_i < move_end) {
		if (board.Make(current->move_to_next)) {
			// Add the move to the output
			*(move_i++) = current->move_to_next;
			output.n_moves++;
			
			// Go to the next position
			current = Evaluate(board.GetCurrent());
		}
		else {
			// If there is a problem making the move, return early
			std::cout << "Error making move " << current->move_to_next << std::endl;
			return output;
		}
	}
	
	return output;
}

TableBase::MoveSequence TableBase::EvaluateSequenceFromFile(BoardState state, const char * path) {
	MoveSequence output(state);
	
	// Get the first node
	const Node * current = EvaluateFromFile(state, path);
	if (!current)  {
		// Return an empty, unevaluated state if not found
		std::cout << "Not found" << std::endl;
		return output;
	}
	
	// Set the initial conditions
	output.Alloc(current->distance);
	output.result = current->result;
	
	// Step through move sequence
	Board board;
	board.SetCurrent(state);
	Move * move_i = output.moves;
	const Move * move_end = output.moves + output.n_alloc;
	while (current && move_i < move_end) {
		if (board.Make(current->move_to_next)) {
			// Add the move to the output
			*(move_i++) = current->move_to_next;
			output.n_moves++;
			
			// Go to the next position
			current = EvaluateFromFile(board.GetCurrent(), path);
		}
		else {
			// If there is a problem making the move, return early
			std::cout << "Error making move " << current->move_to_next << std::endl;
			return output;
		}
	}
	
	return output;
}

void TableBase::AddTableBase(TableBase & tb) {
	positions.insert(tb.positions.begin(), tb.positions.end());
}

/**
 * @brief Add a node to the tablebase with a known result
 * @param state The board state of the final position
 * @param result The code indicating whether white/black won or draw
 * 
 * All unmoves that could have led to this position are added to frontier
 * Used to seed terminal endgame positions
 */
const TableBase::Node * TableBase::AddStaticallySolved(BoardState state, uint8_t result) {
	
	// Find position in table (if already exists)
	NodeIterator node_i = positions.find(state);
	if (node_i != positions.end()) {
		
		// If the position already exists, overwrite so it is solved
		node_i->second->result = result;
		node_i->second->distance = 0;
		node_i->second->status = TableBase::Node::STATUS_SOLVED;
		
		return node_i->second;
	}
	
	// Add position to the solved positions table
	Node * new_node = new TableBase::Node(result);
	positions.insert(NodePair(state, new_node));
	
	std::cout << "Added solved" << std::endl;
	
	// Add positions that led to this position to the frontier
	AddUnmovesToFrontier(state);
	
	return new_node;
}

/**
 * @brief Add a solved node to the solved positions table
 * @param state The board state of the solved position
 * @param next The position which the best move leads to next
 * @param move_to_next The move to get to the next position
 */
const TableBase::Node * TableBase::AddLinkedSolved(BoardState state, Node * next, Move move_to_next) {
	
	// Add position to the solved positions table
	Node * new_node = new TableBase::Node(next, move_to_next);
	positions.insert(NodePair(state, new_node));
	
	// Add positions that led to this position to the frontier
	AddUnmovesToFrontier(state);
	
	return new_node;
}

/**
 * @brief Add a node to the frontier whose solution is not known
 * @param state The board state of the unsolved position
 */
bool TableBase::AddUnsolvedToFrontier(BoardState state) {
	
	// Find position in table
	NodeIterator node_i = positions.find(state);
	if (node_i != positions.end()) {
	
		// Check that the position is not already solved
		if (node_i->second->status == TableBase::Node::STATUS_SOLVED) {
			// Position already solved
			return false;
		}
		
		// Check that the position is not already in the queue
		if (node_i->second->status == TableBase::Node::STATUS_FRONTIER) {
			// Position already exists
			return false;
		}
		
		node_i->second->status = TableBase::Node::STATUS_FRONTIER;
	}
	
	// Add position to the frontier
	Node * new_node = new Node();
	new_node->status = TableBase::Node::STATUS_FRONTIER;
	positions.insert(NodePair(state, new_node));
	
	return true;
}

/**
 * @brief Add all the nodes coming from unmoves to the frontier
 * @param state The board state of the solved root position
 */
void TableBase::AddUnmovesToFrontier(BoardState state) {
		
	// Initialize board and generate unmoves
	Board movegen_board;
	movegen_board.SetCurrent(state);
	const MoveList * unmoves = movegen_board.GetUnmoves();
	
	// Initialize board for making/unmaking unmoves
	Board make_board;
	state.white_to_move = !state.white_to_move;
	make_board.SetCurrent(state);
	
	// Iterate through each unmove
	const Move * unmoves_i = unmoves->Begin();
	const Move * unmoves_end = unmoves->End();
	for (; unmoves_i < unmoves_end; unmoves_i++) {
		if (make_board.Make(*unmoves_i)) {
			// Check that the unmove is legal
			// (i.e. the color that did not make the unmove was not in check originally)
			if (!make_board.InCheck(make_board.GetCurrent().white_to_move)) {
				BoardState state_to_add = make_board.GetCurrent();
				state_to_add.white_to_move = !state_to_add.white_to_move;
				AddUnsolvedToFrontier(state_to_add);
			}
			make_board.Unmake(1);
		}
	}
}

const MoveList * TableBase::GetNodeMoves(NodeIterator node_i) {
	static BoardComposite bc;
	static MoveGenerator mgen;
	
	// If moves not already generated, generate moves
	if (!node_i->second->moves.IsValid()) {
		bc.Init(node_i->first);
		node_i->second->moves = mgen.GetMoves(&bc);
	}
	
	return &node_i->second->moves;
}

/**
 * @brief Evaluate all nodes in the frontier until everything is solved
 */
/*
void TableBase::Expand() {	
	Board board;
	int n_eval = 0;
	
	// Process all nodes
	while (!frontier.empty())
	{		
		if ((n_eval % 100000) == 0) {
			std::cout << n_eval << " iterations, " << solved.size() << " nodes found" << std::endl;
		}
		
		// Choose the next node in the frontier
		BoardState state = frontier.front();
		frontier.pop();
		frontier_unique.erase(state);
		board.SetCurrent(state);
		
		// Conditions for winning and losing
		uint8_t cond_win, cond_lose;
		uint8_t cond_draw = Node::RESULT_DRAW;
		if (state.white_to_move) {
			cond_win = Node::RESULT_WHITE_WIN;
			cond_lose = Node::RESULT_BLACK_WIN;
		}
		else {
			cond_win = Node::RESULT_BLACK_WIN;
			cond_lose = Node::RESULT_WHITE_WIN;
		}
		
		// Verify that position is not already discovered
		if (solved.find(state) != solved.end()) {
			continue;
		}
		
		// Figure out what the best possible option is
		bool 		any_win = false, 	any_draw = false,	any_loss = false;
		Move 		move_win,			move_draw,			move_loss;
		Node 		* node_win = NULL,	* node_draw = NULL,	* node_loss = NULL;
		bool any_undetermined = false;
		
		const MoveList * moves = board.GetMoves();
		const Move * move_i = moves->Begin();
		const Move * move_end = moves->End();
		for (; move_i < move_end; move_i++) {
			
			if (board.Make(*move_i)) {
				
				// Ensure that not moving into check
				if (board.InCheck(!board.GetCurrent().white_to_move)) {
					board.Unmake(1);
					continue;
				}
				
				auto solved_i = solved.find(board.GetCurrent());
				if (solved_i != solved.end()) {
					
					Node * next_node = solved_i->second;
					if (next_node->result == cond_win) {
						any_win = true;
						move_win = *move_i;
						node_win = next_node;
						board.Unmake(1);
						break;
					}
					else if (next_node->result == cond_draw) {
						if (!any_draw) {
							any_draw = true;
							move_draw = *move_i;
							node_draw = next_node;
						}
					}
					else if (next_node->result == cond_lose) {
						if (!any_loss) {
							any_loss = true;
							move_loss = *move_i;
							node_loss = next_node;
						}
					}
					else {
						any_undetermined = true;
					}
				}
				else {
					any_undetermined = true;
				}
				
				board.Unmake(1);
			}
		}
		
		// Automatic win if there is a single winning continuation
		// Not all options have to be evaluated
		// Any winning continuation is sufficient
		n_eval++;
		if (any_win) {
			AddLinkedSolved(state, node_win, move_win);
		}
		else {
			// If there is no discovered path to win, all options are searched
			if (!any_undetermined) {
				// If there are any draws, this position is drawing
				// Any drawing continuation is sufficient
				if (any_draw) {
					AddLinkedSolved(state, node_draw, move_draw);
				}
				// If there are no draws, this position is losing
				// Any option is sufficient
				else {
					AddLinkedSolved(state, node_loss, move_loss);
				}
			}
			else {
				// Must revisit this node later
				AddUnsolvedToFrontier(state);
				continue;
			}
		}
	}
	
	std::cout << "***** Processed " << n_eval << " iterations *****" << std::endl;
	std::cout << *this << std::endl;
}

void TableBase::Expand() {
	int last_frontier_size = 0;
	int last_positions_size = positions.size();
	
	// Iterate until frontier is empty or no progress is made
	while (1) {
		// Copy frontier nodes into fresh memory
		// This is necessary because memory shifts around as new nodes are added
		std::forward_list<NodePair> frontier;
		NodeIterator node_i = positions.begin();
		NodeIterator node_end = positions.end();
		for (; node_i != node_end; node_i++) {
			frontier.push_front(NodePair(node_i->first, node_i->second));
		}
		
		// Terminate when frontier is empty or no progress is made
		if (frontier.empty()) return;
		
		// Iterate through frontier
		
	}
}

void TableBase::ExpandNode(NodeIterator node_i) {
	
}
*/

void TableBase::Optimize() {
	bool all_optimal = false;
	int n_iterations = 0;
	int n_operations = 0;
	
	while (!all_optimal) {
		all_optimal = true;
		
		int n_total = 0;
		int n_not_optimal = 0;
		
		// Iterate through each position
		NodeIterator node_i = positions.begin();
		NodeIterator node_end = positions.end();
		for (; node_i != node_end; node_i++) {
			
			// Check if the node optimization changes when the process is called
			bool result = OptimizeNode(node_i);
			
			all_optimal = all_optimal && !result;
			if (result) n_not_optimal++;
			n_operations++;
			n_total++;
			
			if (n_total % 10000 == 0) {
				std::cout << n_not_optimal << " changed, " << n_total << " total" << std::endl;
			}
		}
		n_iterations++;
		std::cout << n_iterations << " passes, this pass " << n_not_optimal << " nodes changed" << std::endl;
		std::cout << *this << std::endl;
	}
}

/**
 * @brief Modify the next node so that winning/losing positions arrive optimally
 * @param node Node to modify
 * @return Returns true if the node was modified, false if not
 */
bool TableBase::OptimizeNode(NodeIterator node_i) {
	static Board board;
	BoardState state = node_i->first;
	Node * node = node_i->second;
	
	// Check if the position is solved
	if (node->status != TableBase::Node::STATUS_SOLVED) {
		return false;
	}
	
	// If the position is terminal, no further opimization is possible
	if (node->distance == 0) {
		return false;
	}
	
	// If the position is drawn, there is no need for optimization
	if (node->result == TableBase::Node::RESULT_DRAW) {
		return false;
	}
	
	// Decide whether minimizing or maximizing
	bool minimize =
		( state.white_to_move && node->result == TableBase::Node::RESULT_WHITE_WIN) ||
		(!state.white_to_move && node->result == TableBase::Node::RESULT_BLACK_WIN);
		
	// Store data for finding best available continuation
	Node * best_node = NULL;
	int best_distance = minimize ? 16383 : 0;
	Move best_move;
	
	// Iterate through all possible pseudo-legal moves in the position
	board.SetCurrent(state);
	const MoveList * moves = board.GetMoves();
	const Move * move_i = moves->Begin();
	const Move * move_end = moves->End();
	for (; move_i < move_end; move_i++) {
		if (board.Make(*move_i)) {
			do {
				// Check that not moving into check
				if (board.InCheck(state.white_to_move)) {
					continue;
				}
				
				// Retrieve the current node from the solved positions
				NodeIterator next_i = positions.find(board.GetCurrent());
				if (next_i == positions.end()) continue;
				
				// Check that the node has the same result
				Node * next_node = next_i->second;
				if (next_node->result != node->result) continue;
				
				// See if this node is better than the best yet discovered
				if (( minimize && next_node->distance < best_distance) ||
					(!minimize && next_node->distance > best_distance))
				{
					best_node = next_node;
					best_distance = next_node->distance;
					best_move = *move_i;
				}
			} while(0);
			board.Unmake(1);
		}
	}
	
	// Update distance just in case the best node does not change but its
	// distance does
	node->distance = best_node->distance + 1;
	
	// If the best node is better than the current next position, then change
	if (( minimize && best_distance < node->next->distance) ||
		(!minimize && best_distance > node->next->distance))
	{
		node->next = best_node;
		node->move_to_next = best_move;
		return true;
	}
	else return false;
}

/**
 * @brief Remove all nodes from the search frontier
 */
void TableBase::ClearFrontier() {
	NodeIterator node_i = positions.begin();
	NodeIterator node_end = positions.end();
	for (; node_i != node_end; node_i++) {
		if (node_i->second->status == TableBase::Node::STATUS_FRONTIER) {
			if (node_i->second) {
				delete node_i->second;
				node_i->second = NULL;
				positions.erase(node_i);
			}
		}
	}
}

/**
 * @brief Save all solved nodes to files within the specified directory
 * @param path Path to a folder (does not have to be pre-made)
 */
void TableBase::Save(const char * path) {
	char cmd[1024];
	FILE * pipe = NULL;
	
	// Remove old table base of same name
	sprintf(cmd, "rm -r %s", path);
    pipe = popen(cmd, "r");
    pclose(pipe);
	
	// Add back parent folder
	sprintf(cmd, "mkdir %s", path);
	pipe = popen(cmd, "r");
	pclose(pipe);
	
	// Populate new files
	std::forward_list<NodeIterator> grid[64][64][2];
	NodeIterator node_i = positions.begin();
	NodeIterator node_end = positions.end();
	for (; node_i != node_end; node_i++) {
		uint8_t wking, bking;
		GetKingPositions(node_i->first, &wking, &bking);
		grid[wking][bking][node_i->first.white_to_move].push_front(node_i);
	}
	
	// Generate files
	char fpath[1024];
	const char * fname = NULL;
	for (int wking = 0; wking < 64; wking++) {
		for (int bking = 0; bking < 64; bking++) {
			for (int white_to_move = 0; white_to_move < 2; white_to_move++) {
				fname = GetFilename(wking, bking, white_to_move);
				
				auto list_i = grid[wking][bking][white_to_move].begin();
				auto list_end = grid[wking][bking][white_to_move].end();
				if (list_i == list_end) continue;
				
				sprintf(fpath, "%s/%s", path, fname);
				std::fstream ss(fpath, std::fstream::out);
				
				for (; list_i != list_end; list_i++) {
					ss << NodeToLine(*list_i) << '\n';
				}
				
				ss.close();
			}
		}
	}
}

/**
 * @brief Load all solved nodes from a folder saved by this library
 * @param path Path to the folder containing the output of a "Save()"
 */
void TableBase::Load(const char * path) {
	// Generate files
	for (int wking = 0; wking < 64; wking++) {
		for (int bking = 0; bking < 64; bking++) {
			for (int white_to_move = 0; white_to_move < 2; white_to_move++) {
				LoadFile(path, wking, bking, white_to_move);
			}
		}
	}
	
	// Link nodes
	Board board;
	NodeIterator node_i = positions.begin();
	NodeIterator node_end = positions.end();
	for (; node_i != node_end; node_i++) {
		
		// Check that the node has a next move
		if (node_i->second->distance == 0) continue;
		
		board.SetCurrent(node_i->first);
		if (board.Make(node_i->second->move_to_next)) {
			// Find the node for the next move
			NodeIterator next_i = positions.find(board.GetCurrent());
			if (next_i == positions.end()) continue;
			
			// Link nodes
			node_i->second->next = next_i->second;
		}
	}
}

/**
 * @brief Load an individual file
 * @param path Path to the parent folder
 * @param wking Square containing white king
 * @param bking Square containing black king
 * @param white_to_move Whether white is to move
 * 
 * This function accepts inputs, generates the file name according to the
 * format, opens it, and reads all positions into the solved positions table
 */
void TableBase::LoadFile(const char * path, uint8_t wking, uint8_t bking, bool white_to_move) {
	const char * fname = GetFilename(wking, bking, white_to_move);
	
	char fpath[1024];
	sprintf(fpath, "%s/%s", path, fname);
	std::fstream ss(fpath, std::fstream::in);
	
	for (std::string line; std::getline(ss, line); ) {
		positions.insert(InitNodeFromLine(line));
	}
}

/**
 * @brief Generate the file name to look for a specific piece configuration
 * @param wking Square containing white king
 * @param bking Square containing black king
 * @param white_to_move Whether white is to move
 * @return Statically-allocated string (use right away or copy)
 */
const char * TableBase::GetFilename(uint8_t wking, uint8_t bking, bool white_to_move) {
	static char fname[] = "  -  - .erz";
	fname[0] = 'a' + (wking % 8);
	fname[1] = '1' + (wking / 8);
	fname[3] = 'a' + (bking % 8);
	fname[4] = '1' + (bking / 8);
	fname[6] = white_to_move ? 'w' : 'b';
	return fname;
}

/**
 * @brief Get the positions of the kings in a board state
 * @param board
 * @param wking Output
 * @param bking Output
 */
void TableBase::GetKingPositions(BoardState board, uint8_t * wking, uint8_t * bking) {
	*wking = 64;
	*bking = 64;
	for (int i = 0; i < 64; i++) {
		if (board.squares[i] == WHITE_KING) {
			*wking = i;
			if (*bking != 64) break;
		}
		else if (board.squares[i] == BLACK_KING) {
			*bking = i;
			if (*wking != 64) break;
		}
	}
}

/**
 * @brief Create a specially-formatted line for file output
 * @return 
 */
std::string TableBase::NodeToLine(const NodeIterator node_i) {
	std::stringstream ss;
	ss << node_i->first.GetFEN() << '|';
	if (node_i->second->result == TableBase::Node::RESULT_WHITE_WIN) ss << "w|";
	else if (node_i->second->result == TableBase::Node::RESULT_BLACK_WIN) ss << "b|";
	else if (node_i->second->result == TableBase::Node::RESULT_DRAW) ss << "d|";
	else ss << "u|";
	ss << node_i->second->distance << '|';
	ss << node_i->second->move_to_next;
	return ss.str();
}

/**
 * @brief Initialize from a line in a saved file
 * @param line
 */
TableBase::NodePair TableBase::InitNodeFromLine(const std::string line) {
	BoardState state;
	TableBase::Node * node = new TableBase::Node();
	
	char * copy = strdup(line.c_str());
	char * pch = NULL;

	pch = strtok(copy, "|");
	state.InitFromFEN(pch);
	
	pch = strtok(NULL, "|");
	if (pch[0] == 'w') node->result = TableBase::Node::RESULT_WHITE_WIN;
	else if (pch[0] == 'b') node->result = TableBase::Node::RESULT_BLACK_WIN;
	else if (pch[0] == 'd') node->result = TableBase::Node::RESULT_DRAW;
	else node->result = TableBase::Node::RESULT_UNDETERMINED;

	pch = strtok(NULL, "|");
	node->distance = atoi(pch);

	pch = strtok(NULL, "|");
	if (node->distance != 0) {
		node->move_to_next = Move(pch);
	}
	
	free(copy);
	
	return TableBase::NodePair(state, node);
}

std::ostream & operator << (std::ostream & os, TableBase & tb) {
	os << "+==============================================================================+" << std::endl;
	os << "| Table Base" << std::endl;
	os << "|   Positions Total:       " << tb.positions.size() << std::endl;
	os << "|   Positions Solved:      " << tb.Count(TableBase::Node::STATUS_SOLVED) << std::endl;
	os << "|   Positions in Frontier: " << tb.Count(TableBase::Node::STATUS_FRONTIER) << std::endl;
	os << "+-------------------------------------------------------------------------------" << std::endl;
	int count[1024] = {0};
	TableBase::NodeIterator node_i = tb.positions.begin();
	TableBase::NodeIterator node_end = tb.positions.end();
	for (; node_i != node_end; node_i++) {
		count[node_i->second->distance]++;
	}
	for (int depth = 0; depth < 1024; depth++) {
		if (count[depth] == 0) break;
		std::cout << "| " << count[depth] << " positions at depth " << depth << std::endl;
	}
	os << "+==============================================================================+" << std::endl;
	
	return os;
}

std::ostream & operator <<(std::ostream & os, TableBase::NodeIterator & ni) {
	os << "+==============================================================================+" << std::endl;
	os << "| Table Base Node (" << ni->second << ")" << std::endl;
	os << ni->first;
	os << "| Result: ";
	if (ni->second->result == TableBase::Node::RESULT_WHITE_WIN) os << "White Win";
	else if (ni->second->result == TableBase::Node::RESULT_BLACK_WIN) os << "Black Win";
	else if (ni->second->result == TableBase::Node::RESULT_DRAW) os << "Draw";
	else os << "Undetermined";
	os << std::endl;
	os << "| Distance: " << ni->second->distance << std::endl;
	os << "| Next: " << ni->second->next << std::endl;
	os << "| Move " << ni->second->move_to_next << std::endl;
	os << "+==============================================================================+" << std::endl;
	return os;
}