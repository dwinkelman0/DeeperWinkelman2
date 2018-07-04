#include "tablebase.h"

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
		.status = Node::Status::STATUS_FRONTIER,
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
	Board unmoves_board;
	unmoves_board.SetCurrent(state);
	const MoveList * unmoves = unmoves_board.GetUnmoves();
	
	// Board for making moves
	Board make_board;
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

void TableBase::AddStaticPositions(TableBase::GeneratePositionsFunction generate_function) {
	(this->*generate_function)(&TableBase::AddStaticallySolved);
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
		
		// Iterate through positions in frontier
		PosIterator frontier_i = frontier.begin(), frontier_end = frontier.end();
		for (; frontier_i != frontier_end; frontier_i++) {
			
			BoardState state = frontier_i->first;
			Node * node = frontier_i->second;
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
			const MoveList * moves = board.GetMoves();
			const Move * move_i = moves->Begin();
			const Move * move_end = moves->End();
			for (; move_i != move_end; move_i++) {
				if (board.Make(*move_i)) {
					// Verify that not moved into check
					if (board.InCheck(state.white_to_move)) goto unmake;
					
					// Find position in solved
					PosIterator next_i = positions.find(board.GetCurrent());
					if (next_i == positions.end()) {
						any_undetermined = true;
						goto unmake;
					}
					Node * next_node = next_i->second;
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
					
					unmake:
					board.Unmake(1);
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
	}
}