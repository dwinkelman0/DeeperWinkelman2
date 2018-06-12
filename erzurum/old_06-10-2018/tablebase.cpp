#include "tablebase.h"

#include <movegen.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>

/**
 * @brief Blank initialization
 */
TableBase::Node::Node() {
	this->state = BoardState();
	this->result = RESULT_UNDETERMINED;
	this->distance = 0;
	this->next = NULL;
	this->move_to_next = Move(0, 0, Move::NULL_MOVE);
}

/**
 * @brief Initialize as a static, terminal, solved position
 * @param state Evaluated position state
 * @param result Terminal result of the position
 */
TableBase::Node::Node(BoardState state, uint8_t result) {
	this->state = state;
	this->result = result;
	this->distance = 0;
	
	// Since this is a terminal position, no need for future linked node
	this->next = NULL;
	this->move_to_next = Move(0, 0, Move::NULL_MOVE);
}

/**
 * @brief Initialize as a node linked to the next position
 * @param state Evaluated position state
 * @param next Pointer to the next node
 * @param move_to_next Move to reach the position of the next node
 */
TableBase::Node::Node(BoardState state, TableBase::Node * next, Move move_to_next) {
	this->state = state;
	
	// Since this is a linked position, link to next position
	this->next = next;
	this->move_to_next = move_to_next;
	this->result = next->result;
	this->distance = next->distance + 1;
}

/**
 * @brief Initialize from a line in a saved file
 * @param line
 */
TableBase::Node::Node(std::string line) {
	char * copy = strdup(line.c_str());
	char * pch = NULL;

	pch = strtok(copy, "|");
	this->state.InitFromFEN(pch);

	pch = strtok(NULL, "|");
	if (pch[0] == 'w') this->result = TableBase::Node::RESULT_WHITE_WIN;
	else if (pch[0] == 'b') this->result = TableBase::Node::RESULT_BLACK_WIN;
	else if (pch[0] == 'd') this->result = TableBase::Node::RESULT_DRAW;
	else this->result = TableBase::Node::RESULT_UNDETERMINED;

	pch = strtok(NULL, "|");
	this->distance = atoi(pch);

	pch = strtok(NULL, "|");
	if (distance != 0) {
		this->move_to_next = Move(pch);
	}
	
	free(copy);
}

/**
 * @brief Create a specially-formatted line for file output
 * @return 
 */
std::string TableBase::Node::ToString() {
	std::stringstream ss;
	ss << state.GetFEN() << '|';
	if (result == TableBase::Node::RESULT_WHITE_WIN) ss << "w|";
	else if (result == TableBase::Node::RESULT_BLACK_WIN) ss << "b|";
	else if (result == TableBase::Node::RESULT_DRAW) ss << "d|";
	else ss << "u|";
	ss << distance << '|';
	ss << move_to_next;
	return ss.str();
}

const MoveList * TableBase::Node::GetMoves() {
	static BoardComposite bc;
	static MoveGenerator mgen;
	
	// If moves not already generated, generate moves
	if (!this->moves.IsValid()) {
		bc.Init(this->state);
		this->moves = mgen.GetMoves(&bc);
	}
	
	return &this->moves;
}

std::ostream & operator <<(std::ostream & os, const TableBase::Node & node) {
	os << "+==============================================================================+" << std::endl;
	os << "| Table Base Node (" << &node << ")" << std::endl;
	os << node.state;
	os << "| Result: ";
	if (node.result == TableBase::Node::RESULT_WHITE_WIN) os << "White Win";
	else if (node.result == TableBase::Node::RESULT_BLACK_WIN) os << "Black Win";
	else if (node.result == TableBase::Node::RESULT_DRAW) os << "Draw";
	else os << "Undetermined";
	os << std::endl;
	os << "| Distance: " << node.distance << std::endl;
	os << "| Next: " << node.next << std::endl;
	os << "| Move " << node.move_to_next << std::endl;
	os << "+==============================================================================+" << std::endl;
	return os;
}

TableBase::MoveSequence::MoveSequence() {
	this->moves = NULL;
	this->n_moves = 0;
	this->n_alloc = 0;
	this->initial_state = BoardState();
	this->result = TableBase::Node::RESULT_UNDETERMINED;
}

TableBase::MoveSequence::MoveSequence(BoardState state) {
	this->moves = NULL;
	this->n_moves = 0;
	this->n_alloc = 0;
	this->initial_state = state;
	this->result = TableBase::Node::RESULT_UNDETERMINED;
}

TableBase::MoveSequence::MoveSequence(const MoveSequence & other) {
	this->n_moves = other.n_moves;
	this->n_alloc = other.n_alloc;
	this->initial_state = other.initial_state;
	this->result = other.result;
	
	this->moves = new Move[other.n_alloc];
	memcpy(this->moves, other.moves, other.n_alloc * sizeof(Move));
}

TableBase::MoveSequence & TableBase::MoveSequence::operator =(const TableBase::MoveSequence & other) {
	Move * temp_moves = new Move[other.n_alloc];
	delete[] this->moves;
	
	this->n_moves = other.n_moves;
	this->n_alloc = other.n_alloc;
	this->initial_state = other.initial_state;
	this->result = other.result;
	
	this->moves = temp_moves;
	memcpy(this->moves, other.moves, other.n_alloc * sizeof(Move));
	
	return *this;
}

TableBase::MoveSequence::~MoveSequence() {
	if (this->moves) {
		delete[] this->moves;
		this->moves = NULL;
	}
	this->n_moves = 0;
	this->n_alloc = 0;
}

std::ostream & operator << (std::ostream & os, const TableBase::MoveSequence & ms) {
	os << "MoveSequence [Result: ";
	if (ms.result == TableBase::Node::RESULT_WHITE_WIN) os << "White Win";
	else if (ms.result == TableBase::Node::RESULT_BLACK_WIN) os << "Black Win";
	else if (ms.result == TableBase::Node::RESULT_DRAW) os << "Draw";
	else os << "Undetermined";
	os << ", Length: " << ms.Length();
	os << ", Initial Position: \"" << ms.initial_state.GetFEN() << "\"] ";
	
	if (ms.n_moves == 0) {
		os << "No Moves";
		return os;
	}
	
	int move_number = 1;
	bool white_to_move = false;
	const Move * move_i = ms.Begin();
	const Move * move_end = ms.End();
	os << "1. ";
	if (!ms.initial_state.white_to_move) {
		os << "... ";
	}
	else {
		os << *(move_i++) << " ";
	}
	for (; move_i < move_end; move_i++) {
		if (white_to_move) {
			move_number++;
			std::cout << move_number << ". ";
		}
		white_to_move = !white_to_move;
		os << *move_i << " ";
	}
	
	return os;
}

TableBase::~TableBase() {
	auto solved_i = solved.begin();
	auto solved_end = solved.end();
	for (; solved_i != solved_end; solved_i++) {
		delete solved_i->second;
		solved_i->second = NULL;
	}
	
	ClearFrontier();
}

/**
 * @brief Get the node with solution data for the given position
 * @param state
 * @return NULL if there is no node for the position
 */
TableBase::Node * TableBase::Evaluate(BoardState state) {
	const auto iter = solved.find(state);
	if (iter == solved.end()) return NULL;
	else return iter->second;
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
	solved.insert(tb.solved.begin(), tb.solved.end());
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
	
	// Add position to the solved positions table
	Node * new_node = new TableBase::Node(state, result);
	solved.insert(SolvedPosition(state, new_node));
	
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
	Node * new_node = new TableBase::Node(state, next, move_to_next);
	solved.insert(SolvedPosition(state, new_node));
	
	// Add positions that led to this position to the frontier
	AddUnmovesToFrontier(state);
	
	return new_node;
}

/**
 * @brief Add a node to the frontier whose solution is not known
 * @param state The board state of the unsolved position
 */
bool TableBase::AddUnsolvedToFrontier(BoardState state) {
	
	// Check that the position is not already solved
	if (solved.find(state) != solved.end()) return false;
	
	// Check that the position is not already in the queue
	if (frontier_unique.find(state) != frontier_unique.end()) {
		// Position already exists
		return false;
	}
	
	// Add position to the frontier
	frontier.push(state);
	frontier_unique.insert(state);
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

/**
 * @brief Evaluate all nodes in the frontier until everything is solved
 */
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

void TableBase::Optimize() {
	bool all_optimal = false;
	int n_iterations = 0;
	int n_operations = 0;
	
	while (!all_optimal) {
		all_optimal = true;
		
		int n_total = 0;
		int n_not_optimal = 0;
		
		// Iterate through each position
		auto solved_i = solved.begin();
		auto solved_end = solved.end();
		for (; solved_i != solved_end; solved_i++) {
			Node * node = solved_i->second;
			
			bool result = OptimizeNode(node);
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
bool TableBase::OptimizeNode(Node * node) {
	// If the position is already optimized, no further possible
	
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
		( node->state.white_to_move && node->result == TableBase::Node::RESULT_WHITE_WIN) ||
		(!node->state.white_to_move && node->result == TableBase::Node::RESULT_BLACK_WIN);
		
	// Store data for finding best available continuation
	Node * best_node = NULL;
	int best_distance = minimize ? 16383 : 0;
	Move best_move;
	
	// Iterate through all possible pseudo-legal moves in the position
	Board board;
	board.SetCurrent(node->state);
	const MoveList * moves = board.GetMoves();
	const Move * move_i = moves->Begin();
	const Move * move_end = moves->End();
	for (; move_i < move_end; move_i++) {
		if (board.Make(*move_i)) {
			do {
				// Check that not moving into check
				if (board.InCheck(node->state.white_to_move)) {
					continue;
				}
				
				// Retrieve the current node from the solved positions
				auto node_i = solved.find(board.GetCurrent());
				if (node_i == solved.end()) continue;
				
				// Check that the node has the same result
				Node * next_node = node_i->second;
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
	std::queue<BoardState> empty_queue;
	frontier.swap(empty_queue);
	std::set<BoardState> empty_set;
	frontier_unique.swap(empty_set);
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
	std::vector<Node *> grid[64][64][2];
	std::map<BoardState, Node *>::iterator solved_i = solved.begin();
	auto solved_end = solved.end();
	for (; solved_i != solved_end; solved_i++) {
		uint8_t wking, bking;
		GetKingPositions(solved_i->second->state, &wking, &bking);
		grid[wking][bking][solved_i->second->state.white_to_move].push_back(solved_i->second);
	}
	
	// Generate files
	char fpath[1024];
	const char * fname = NULL;
	for (int wking = 0; wking < 64; wking++) {
		for (int bking = 0; bking < 64; bking++) {
			for (int white_to_move = 0; white_to_move < 2; white_to_move++) {
				fname = GetFilename(wking, bking, white_to_move);
				
				auto node_i = grid[wking][bking][white_to_move].begin();
				auto node_end = grid[wking][bking][white_to_move].end();
				if (node_i == node_end) continue;
				
				sprintf(fpath, "%s/%s", path, fname);
				std::fstream ss(fpath, std::fstream::out);
				
				for (; node_i != node_end; node_i++) {
					Node * node = *node_i;
					ss << node->ToString() << '\n';
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
	auto solved_i = solved.begin();
	auto solved_end = solved.end();
	for (; solved_i != solved_end; solved_i++) {
		Node * node = solved_i->second;
		
		// Check that the node has a next move
		if (node->distance == 0) continue;
		
		board.SetCurrent(node->state);
		if (board.Make(node->move_to_next)) {
			// Find the node for the next move
			auto next_i = solved.find(board.GetCurrent());
			if (next_i == solved.end()) continue;
			
			// Link nodes
			node->next = next_i->second;
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
		Node * new_node = new TableBase::Node(line);
		solved.insert(SolvedPosition(new_node->state, new_node));
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

std::ostream & operator << (std::ostream & os, const TableBase & tb) {
	os << "+==============================================================================+" << std::endl;
	os << "| Table Base" << std::endl;
	os << "| " << tb.solved.size() << " positions solved" << std::endl;
	os << "| " << tb.frontier.size() << " positions enqueued in the frontier" << std::endl;
	os << "+-------------------------------------------------------------------------------" << std::endl;
	int count[1024] = {0};
	auto solved_i = tb.solved.begin(), solved_end = tb.solved.end();
	for (; solved_i != solved_end; solved_i++) {
		const TableBase::Node * node = solved_i->second;
		count[node->distance]++;
	}
	for (int depth = 0; depth < 1024; depth++) {
		if (count[depth] == 0) break;
		std::cout << "| " << count[depth] << " positions at depth " << depth << std::endl;
	}
	os << "+==============================================================================+" << std::endl;
	
	return os;
}