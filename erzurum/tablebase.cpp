#include "tablebase.h"

std::ostream & operator << (std::ostream & os, TableBase::Evaluation eval) {
	if (eval.result == TableBase::Evaluation::RESULT_DRAW) {
		os << "Draw";
	}
	else if (eval.result == TableBase::Evaluation::RESULT_WHITE_WIN) {
			os << "+#" << (eval.distance + 1) / 2 << " " << eval.move_to_next;
	}
	else if (eval.result == TableBase::Evaluation::RESULT_BLACK_WIN) {
		os << "+#" << (eval.distance + 1) / 2 << " " << eval.move_to_next;
	}
	else {
		os << "???";
	}
	
	return os;
}

TableBase::TableBase() {
	
}

TableBase::~TableBase() {
	PosIterator pos_i = positions.begin();
	PosIterator pos_end = positions.end();
	for (; pos_i != pos_end; pos_i++) {
		Node * node = pos_i->second;
		if (node) {
			delete node;
		}
	}
}

bool TableBase::AddStaticallySolved(BoardState state, uint8_t result) {
	
	// Check that node does not already exist
	PosIterator pos_i = positions.find(state);
	if (pos_i != positions.end()) {
		Node * node = pos_i->second;
		// If already exists, just modify
		if (node) {
			node->status = Node::Status::STATUS_SOLVED;
			node->result = result;
			node->distance = 0;
			node->next = NULL;
			node->move_to_next = Move();
		}
	}
	
	// Insert new node (in new memory) into table
	Node * new_node = new Node {
		.status = Node::Status::STATUS_SOLVED,
		.result = result,
		.distance = 0,
		.next = NULL,
		.move_to_next = Move()
	};
	positions.insert(std::pair<BoardState, Node *>(state, new_node));
	
	// Add unmoves to frontier
	AddUnmovesToFrontier(state);
	
	return true;
}

bool TableBase::AddFrontier(BoardState state) {
	
	// Check that node does not already exist
	if (positions.find(state) != positions.end()) {
		return false;
	}
	
	// Insert new node (in new memory) into table
	Node * new_node = new Node {
		.status = Node::STATUS_FRONTIER,
		.result = Node::RESULT_UNDETERMINED,
		.distance = 0,
		.next = NULL,
		.move_to_next = Move()
	};
	positions.insert(std::pair<BoardState, Node *>(state, new_node));
	return true;
}

bool TableBase::AddLinkedSolved(BoardState state, uint8_t result, Node * next, BoardState next_state, Move move_to_next) {
	
	// Find positions in table, make sure already exist
	PosIterator pos_i = positions.find(state);
	PosIterator next_i = positions.find(next_state);
	if (pos_i == positions.end() || next_i == positions.end()) {
		return false;
	}
	
	// Modify solved position
	Node * node = pos_i->second;
	Node * next_node = next_i->second;
	if (!node || !next_node) {
		return false;
	}
	node->status = Node::Status::STATUS_SOLVED;
	node->result = result;
	node->distance = next_node->distance + 1;
	node->next = next_node;
	node->move_to_next = move_to_next;
	
	// Add unmoves to frontier
	AddUnmovesToFrontier(state);
	
	return true;
}

bool TableBase::AddUnmovesToFrontier(BoardState state) {
	
	bool orig_color = state.white_to_move;
	
	// Board for finding moves
	static Board unmoves_board;
	unmoves_board.SetCurrent(state);
	const MoveList * unmoves = unmoves_board.GetUnmoves();
	
	// Board for making moves
	static Board make_board;
	state.white_to_move = !state.white_to_move;
	make_board.SetCurrent(state);
	
	// Iterate through unmoves
	const Move * unmoves_i = unmoves->Begin();
	const Move * unmoves_end = unmoves->End();
	for (; unmoves_i != unmoves_end; unmoves_i++) {
		if (make_board.Make(*unmoves_i)) {
			// Check that the unmove is legal
			// (i.e. the color that did not make the unmove is not in check
			// before the move was made)
			if (!make_board.InCheck(orig_color)) {
				BoardState state_to_add = make_board.GetCurrent();
				state_to_add.white_to_move = !state_to_add.white_to_move;
				AddFrontier(state_to_add);
			}
			make_board.Unmake(1);
		}
	}
	
	return true;
}

void TableBase::Expand() {
	
	Board board;
	
	// Iterate through positions until frontier is empty
	for (int pass = 0;; pass++) {
		
		// Generate frontier list
		// NOTE: std::map iterators are not invalidated by insertion
		std::vector<PosIterator> frontier;
		PosIterator pos_i = positions.begin(), pos_end = positions.end();
		for (; pos_i != pos_end; pos_i++) {
			if (pos_i->second->status == Node::STATUS_FRONTIER) {
				frontier.push_back(pos_i);
			}
		}
		
		// Exit if frontier is empty
		if (frontier.size() == 0) break;
		
		// Iterate through positions in frontier
		auto frontier_i = frontier.begin(), frontier_end = frontier.end();
		for (; frontier_i != frontier_end; frontier_i++) {
			
			BoardState state = (*frontier_i)->first;
			Node * node = (*frontier_i)->second;
			if (!node) continue;
			
			// Determine results needed for an outcome
			uint8_t cond_win  = state.white_to_move ? Node::RESULT_WHITE_WIN : Node::RESULT_BLACK_WIN;
			uint8_t cond_lose = state.white_to_move ? Node::RESULT_BLACK_WIN : Node::RESULT_WHITE_WIN;
			uint8_t cond_draw = Node::RESULT_DRAW;
			
			// Check whether there was a missed stalemate/checkmate
			// Prevents error conditions later, and pre-generates moves
			board.SetCurrent(state);
			if (board.IsCheckmate()) {
				AddStaticallySolved(state, cond_win);
			}
			else if (board.IsStalemate()) {
				AddStaticallySolved(state, cond_draw);
			}
			
			// Hold results for positive outcomes
			bool any_undetermined = false;
			uint8_t best_cond = cond_lose;
			PosIterator best_i;
			Move best_move;
			
			// Filter through possible moves from this position
			// If any win conditions are found, exit immediately
			if (!node->move_cache.IsValid()) {
				node->move_cache = *board.GetMoves();
			}
			const MoveList * moves = &node->move_cache;
			const Move * move_i = moves->Begin();
			const Move * move_end = moves->End();
			for (; move_i != move_end; move_i++) {
				if (board.Make(*move_i)) {
					Node * next_node = NULL;
					PosIterator next_i;
					
					// Verify that not moved into check
					if (board.InCheck(state.white_to_move)) goto unmake;
					
					// Find position in solved
					next_i = positions.find(board.GetCurrent());
					if (next_i == positions.end()) {
						any_undetermined = true;
						goto unmake;
					}
					next_node = next_i->second;
					if (!next_node) goto unmake;
					
					// If the node is not solved or undetermined,
					// nothing can be done at this time
					if (next_node->result == Node::RESULT_UNDETERMINED ||
						next_node->status == Node::STATUS_INVALID)
					{
						any_undetermined = true;
						goto unmake;
					}
					
					// If the node is a win, we are done
					if (next_node->result == cond_win) {
						best_cond = cond_win;
						best_i = next_i;
						best_move = *move_i;
						break;
					}
					
					// If the node is a draw, record as best yet
					if (next_node->result == cond_draw) {
						best_cond = cond_draw;
						best_i = next_i;
						best_move = *move_i;
					}
					
					// If the node is a loss, fill data for later
					if (best_cond == cond_lose) {
						best_i = next_i;
						best_move = *move_i;
					}
					
					unmake: {
						board.Unmake(1);
					}
				}
			}
			
			// Process if node was conclusively solved
			// If there was a win, automatically solved
			if (best_cond == cond_win) {
				AddLinkedSolved(state, cond_win, best_i->second, best_i->first, best_move);
			}
			// If all nodes are determined, possible to solve otherwise
			else if (!any_undetermined) {
				AddLinkedSolved(state, best_cond, best_i->second, best_i->first, best_move);
			}
			// If there are undetermined nodes, there could still be an
			// undiscovered win, so do nothing with it
			else {
				
			}
		}
		
		std::cout << "After pass " << pass << ":" << std::endl;
		std::cout << *this << std::endl;
	}
}

void TableBase::Optimize() {
	
	Board board;
	
	PrintStrata();
	
	// Iterate through all positions until all positions are stable
	for (int pass = 0;; pass++) {
		int n_not_optimal = 0;
		
		PosIterator pos_i = positions.begin();
		PosIterator pos_end = positions.end();
		for (; pos_i != pos_end; pos_i++) {
			
			BoardState state = pos_i->first;
			Node * node = pos_i->second;
			if (!node) {
				n_not_optimal++;
				continue;
			}
			
			// Nothing to be done for draws
			if (node->result == Node::RESULT_DRAW) continue;
			
			// Nothing to be done for terminal positions
			if (node->distance == 0) continue;
			
			// Determine if the side to move is winning
			// Winning sides minimize distance to end, losing sides maximize
			bool is_minimizing =
				( state.white_to_move && node->result == Node::RESULT_WHITE_WIN) ||
				(!state.white_to_move && node->result == Node::RESULT_BLACK_WIN);
				
			uint16_t best_distance = is_minimizing ? 0xffff : 0;
			Node * best_node = NULL;
			Move best_move;
				
			// Iterate through moves to check child positions
			board.SetCurrent(state);
			if (!node->move_cache.IsValid()) {
				node->move_cache = *board.GetMoves();
			}
			const MoveList * moves = &node->move_cache;
			const Move * move_i = moves->Begin();
			const Move * move_end = moves->End();
			for (; move_i != move_end; move_i++) {
				if (board.Make(*move_i)) {
					
					PosIterator next_i;
					Node * next_node;
					
					// Verify that not moved into check
					if (board.InCheck(state.white_to_move)) goto unmake;
					
					// Find position in solved
					next_i = positions.find(board.GetCurrent());
					if (next_i == positions.end()) {
						n_not_optimal++;
						goto unmake;
					}
					next_node = next_i->second;
					if (!next_node) goto unmake;
					
					// Check that position has same result
					if (next_node->result != node->result) goto unmake;
					
					// See if the next position improves upon the others
					if (( is_minimizing && next_node->distance < best_distance) ||
						(!is_minimizing && next_node->distance > best_distance))
					{
						best_distance = next_node->distance;
						best_node = next_node;
						best_move = *move_i;
					}
					
					unmake: {
						board.Unmake(1);
					}
				}
			}
			
			// Update stored distance in case next node changed
			node->distance = node->next->distance + 1;
			
			// See if best node out-performs already-established best node
			if (( is_minimizing && best_distance < node->next->distance) ||
				(!is_minimizing && best_distance > node->next->distance))
			{
				node->next = best_node;
				node->move_to_next = best_move;
				node->distance = best_node->distance + 1;
				n_not_optimal++;
			}
		}
		
		std::cout << "Pass " << pass << ": " << n_not_optimal << " not optimal" << std::endl;
		if (n_not_optimal == 0) break;
	}
	
	PrintStrata();
}

TableBase::Evaluation TableBase::Evaluate(BoardState state) {
	Evaluation output;
	
	PosIterator pos_i = positions.find(state);
	if (pos_i == positions.end() || !pos_i->second) {
		output.result = Evaluation::RESULT_UNDETERMINED;
		return output;
	}
	else {
		Node * node = pos_i->second;
		output.result = node->result;
		output.distance = node->distance;
		output.white_to_move = state.white_to_move;
		output.move_to_next = node->move_to_next;
		return output;
	}
}

std::ostream & operator << (std::ostream & os, TableBase & tb) {
	
	// Count frontier/solved nodes
	int n_frontier = 0, n_solved = 0;
	TableBase::PosIterator pos_i = tb.positions.begin();
	TableBase::PosIterator pos_end = tb.positions.end();
	for (; pos_i != pos_end; pos_i++) {
		TableBase::Node * node = pos_i->second;
		if (node) {
			if (node->status == TableBase::Node::STATUS_FRONTIER) n_frontier++;
			else if (node->status == TableBase::Node::STATUS_SOLVED) n_solved++;
		}
	}
	
	os << "+-------------------------------------------------------------------------------" << std::endl;
	os << "| TABLE BASE" << std::endl;
	os << "| Frontier: " << n_frontier << std::endl;
	os << "| Solved:   " << n_solved << std::endl;
	os << "+--------------------" << std::endl;
	
	return os;
}

void TableBase::PrintStrata() {
	int count[1024];
	for (int i = 0; i < 1024; i++) count[i] = 0;
	
	PosIterator pos_i = positions.begin();
	PosIterator pos_end = positions.end();
	for (; pos_i != pos_end; pos_i++) {
		count[pos_i->second->distance]++;
	}
	
	for (int i = 0; i < 1024; i++) {
		if (count[i] == 0) break;
		std::cout << " | " << i << ": " << count[i] << std::endl;
	}
}