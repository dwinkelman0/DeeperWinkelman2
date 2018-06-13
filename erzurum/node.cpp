#include "tablebase.h"

TableBase::Node::Node() {
	this->status = TableBase::Node::STATUS_UNINIT;
	this->result = TableBase::RESULT_UNDETERMINED;
	this->distance = 0;
	this->next = NULL;
}

TableBase::Node * TableBase::Node::NewStatic(const BoardState state, uint8_t result) {
	TableBase::Node * new_node = new TableBase::Node();
	new_node->state = state;
	new_node->status = TableBase::Node::STATUS_SOLVED;
	new_node->result = result;
	return new_node;
}

TableBase::Node * TableBase::Node::NewFrontier(const BoardState state) {
	TableBase::Node * new_node = new TableBase::Node();
	new_node->state = state;
	new_node->status = TableBase::Node::STATUS_FRONTIER;
	return new_node;
}

TableBase::Node * TableBase::Node::NewFromLine(const std::string line) {
	/// TODO: Implement
	return NULL;
}

std::string TableBase::Node::ToLine() const {
	/// TODO: Implement
	return "";
}

std::ostream & operator << (std::ostream & os, const TableBase::Node * node) {
	os << "Node "
	if (node->status == TableBase::Node::STATUS_UNINIT) os << "(UNINIT)";
	else if (node->status == TableBase::Node::STATUS_FRONTIER) os << "(FRONTIER)";
	else if (node->status == TableBase::Node::STATUS_SOLVED) os << "(SOLVED)";
	os << "[";
	if (node->result == TableBase::RESULT_WHITE_WIN) os << "White Wins in " << node->distance;
	else if (node->result == TableBase::RESULT_BLACK_WIN) os << "Black Wins in " << node->distance;
	else if (node->result == TableBase::RESULT_DRAW) os << "Draw";
	else os << "Undetermined";
	os << "] " << node->state.GetFEN();
	return os;
}