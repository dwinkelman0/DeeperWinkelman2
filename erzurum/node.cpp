#include "tablebase.h"

/**
 * @brief Blank initialization
 */
TableBase::Node::Node() {
	// Structs are initialized by default to invalid
	this->result = RESULT_UNDETERMINED;
	this->distance = 0;
	this->next = NULL;
	this->status = STATUS_UNINIT;
}

/**
 * @brief Initialize as a static, terminal, solved position
 * @param result Terminal result of the position
 */
TableBase::Node::Node(uint8_t result) {
	// Structs are initialized by default to invalid
	this->result = RESULT_UNDETERMINED;
	this->distance = 0;
	this->status = STATUS_SOLVED;
	
	// Since this is a terminal position, no need for future linked node
	this->next = NULL;
	this->move_to_next = Move(0, 0, Move::NULL_MOVE);
}

/**
 * @brief Initialize as a node linked to the next position
 * @param next Pointer to the next node
 * @param move_to_next Move to reach the position of the next node
 */
TableBase::Node::Node(TableBase::Node * next, Move move_to_next) {
	// Structs are initialized by default to invalid
	this->status = STATUS_SOLVED;
	
	// Since this is a linked position, link to next position
	this->result = next->result;
	this->distance = next->distance + 1;
	this->next = next;
	this->move_to_next = move_to_next;
}