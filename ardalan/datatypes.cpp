#include "datatypes.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string.h>

uint8_t Text2Coord(const char * text) {
	return (text[0] - 'a') + 8 * (text[1] - '1');
}

std::string Coord2Text(uint8_t coord) {
	char out[3];
	out[0] = 'a' + (coord % 8);
	out[1] = '1' + (coord / 8);
	out[2] = 0;
	return std::string(out);
}

CoordList::CoordList() {
	data = 0xffffffffffffffff;
}

uint8_t CoordList::Count() {
	Bitboard_t x = ~((data >> 6) | (data >> 7)) & 0x0101010101010101;
	x += x >> 32;
	x += x >> 16;
	x += x >> 8;
	return x & 0x0f;
}

std::ostream & operator << (std::ostream & os, CoordList cl) {
	// Count number of moves
	/*
	int count = 0;
	for (int i = 0; i < 8; i++) count += (cl.coords[i] < 64);
	*/
	int count = cl.Count();

	// Write output
	os << "{[" << count << " coords]: ";
	for (int i = 0; i < 8; i++) {
		if (cl.coords[i] < 64) {
			os << Coord2Text(cl.coords[i]) << " ";
		}
		else if (false) {
			os << "-- ";
		}
	}
	os << "} ";
	
	return os;
}

Move::Move() {
	code = Move::NULL_MOVE;
}

Move::Move(uint8_t start, uint8_t end, uint8_t code = NORMAL_MOVE) {
	this->start = start;
	this->end = end;
	this->code = code;
}

Move::Move(const char * str) {
	const char * wOO = "wO-O";
	const char * wOOO = "wO-O-O";
	const char * bOO = "bO-O";
	const char * bOOO = "bO-O-O";
	/*
	if (0 == strcmp(str, "wO-O")) code = WHITE_OO;
	else if (0 == strcmp(str, "wO-O-O")) code = WHITE_OOO;
	else if (0 == strcmp(str, "bO-O")) code = BLACK_OO;
	else if (0 == strcmp(str, "bO-O-O")) code = BLACK_OOO;
	*/
	
	if (0 == strcmp(str, wOO)) code = WHITE_OO;
	else if (0 == strcmp(str, wOOO)) code = WHITE_OOO;
	else if (0 == strcmp(str, bOO)) code = BLACK_OO;
	else if (0 == strcmp(str, bOOO)) code = BLACK_OOO;
	else {
		start = (str[0] - 'a') + (str[1] - '1') * 8;
		end = (str[3] - 'a') + (str[4] - '1') * 8;

		if (str[5] == 'e') code = EN_PASSANT;
		else if (str[5] == '=') {
			uint8_t BASE = (str[6] == 'w') ? 0 : 8;
			switch (str[7]) {
				case 'N': BASE += WHITE_KNIGHT;	break;
				case 'B': BASE += WHITE_BISHOP; break;
				case 'R': BASE += WHITE_ROOK;	break;
				case 'Q': BASE += WHITE_QUEEN;	break;
			}
			code = BASE;
		} else code = NORMAL_MOVE;
	}
}

bool Move::operator ==(const Move & other) const {
	return 
		this->code == other.code &&
		this->start == other.start &&
		this->end == other.end;
}

std::ostream & operator << (std::ostream & os, Move m) {
	
	if (m.code == Move::NULL_MOVE) {
		os << "NULL";
		return os;
	}
	else if (m.code == Move::WHITE_OO || m.code == Move::BLACK_OO) {
		os << "O-O";
		return os;
	}
	else if (m.code == Move::WHITE_OOO || m.code == Move::BLACK_OOO) {
		os << "O-O-O";
		return os;
	}
	
	os << Coord2Text(m.start) << "-" << Coord2Text(m.end);
	if (m.code == Move::EN_PASSANT) {
		os << "e.p.";
	}
	switch (m.code) {
		case WHITE_KNIGHT:	os << "=N"; break;
		case WHITE_BISHOP:	os << "=B"; break;
		case WHITE_ROOK:	os << "=R"; break;
		case WHITE_QUEEN:	os << "=Q"; break;
		case BLACK_KNIGHT:	os << "=N"; break;
		case BLACK_BISHOP:	os << "=B"; break;
		case BLACK_ROOK:	os << "=R"; break;
		case BLACK_QUEEN:	os << "=Q"; break;
	}
	return os;
}

MoveList::MoveList() {
	this->moves = NULL;
	this->n_alloc = 0;
	this->n_moves = 0;
	this->valid = false;
}

MoveList::MoveList(const MoveList & other) {
	this->n_alloc = other.n_alloc;
	this->n_moves = other.n_moves;
	this->valid = other.valid;
	
	this->moves = new Move[other.n_alloc];
	memcpy(this->moves, other.moves, other.n_alloc * sizeof(Move));
}

MoveList & MoveList::operator =(const MoveList & other) {
	Move * temp_moves = new Move[other.n_alloc];
	delete[] this->moves;
	
	this->n_alloc = other.n_alloc;
	this->n_moves = other.n_moves;
	this->valid = other.valid;
	
	this->moves = temp_moves;
	memcpy(this->moves, other.moves, other.n_alloc * sizeof(Move));
	
	return *this;
}

MoveList::~MoveList() {
	if (this->moves) {
		delete[] this->moves;
		this->moves = NULL;
	}
	this->n_alloc = 0;
	this->n_moves = 0;
}

std::ostream & operator << (std::ostream & os, const MoveList & ml) {
	if (!ml.valid) {
		os << "{Uninitialized Move List}";
		return os;
	}
	
	os << "{[" << ml.n_moves << " moves]";
	for (int i = 0; i < ml.n_moves; i++) {
		os << " " << ml.moves[i];
	}
	os << "}";
	return os;
}

uint8_t BoardState::DEFAULT_SQUARES[64] = {
	4, 2, 3, 5, 6, 3, 2, 4,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	9, 9, 9, 9, 9, 9, 9, 9,
	12,10,11,13,14,11,10,12
};

bool BoardState::InitFromFEN(const char * fen) {
	const char * i = fen;
	
	// Get pieces
	int square = 56;
	while (*i != ' ') {
		switch (*i) {
			case 'P': squares[square] = WHITE_PAWN; break;
			case 'N': squares[square] = WHITE_KNIGHT; break;
			case 'B': squares[square] = WHITE_BISHOP; break;
			case 'R': squares[square] = WHITE_ROOK; break;
			case 'Q': squares[square] = WHITE_QUEEN; break;
			case 'K': squares[square] = WHITE_KING; break;
			case 'p': squares[square] = BLACK_PAWN; break;
			case 'n': squares[square] = BLACK_KNIGHT; break;
			case 'b': squares[square] = BLACK_BISHOP; break;
			case 'r': squares[square] = BLACK_ROOK; break;
			case 'q': squares[square] = BLACK_QUEEN; break;
			case 'k': squares[square] = BLACK_KING; break;
			case '/': square -= 17; break;
			default:
				if (*i >= '1' && *i <= '8') {
					for (int j = 0; j < *i - '1' + 1; j++) {
						squares[square++] = EMPTY;
					}
					square--;
				}
				else {
					return false;
				}
				break;
		}
		i++;
		square++;
	}
	
	// Get color to move
	while (*i == ' ') i++;
	while (*i != ' ') {
		if (*i == 'w') white_to_move = true;
		else if (*i == 'b') white_to_move = false;
		else return false;
		i++;
	}
	
	// Get castling rights
	while (*i == ' ') i++;
	white_OO = white_OOO = black_OO = black_OOO = false;
	while (*i != ' ') {
		switch (*i) {
			case 'K': white_OO = true; break;
			case 'Q': white_OOO = true; break;
			case 'k': black_OO = true; break;
			case 'q': black_OOO = true; break;
			case '-': break;
			default: return false;
		}
		i++;
	}
	
	// Get en passant
	while (*i == ' ') i++;
	if (*i == '-') {
		ep_target = 0;
		i++;
	}
	else {
		if (*i >= 'a' && *i <= 'h' && *(i+1) >= '1' && *(i+1) <= '8') {
			ep_target = (*i - 'a') + (*(i+1) - '1') * 8;
			ep_target += white_to_move ? -8 : 8;
			i += 2;
		}
		else return false;
	}
	
	// Get moves since progress
	while (*i == ' ') i++;
	n_ply_without_progress = atoi(i);
	
	return true;
}

std::string BoardState::GetFEN() const {
	std::stringstream ss;
	
	// Pieces
	for (int rank = 7; rank >= 0; rank--) {
		for (int file = 0; file < 8; file++) {
			switch(squares[rank * 8 + file]) {
				case WHITE_PAWN: ss << 'P'; break;
				case WHITE_KNIGHT: ss << 'N'; break;
				case WHITE_BISHOP: ss << 'B'; break;
				case WHITE_ROOK: ss << 'R'; break;
				case WHITE_QUEEN: ss << 'Q'; break;
				case WHITE_KING: ss << 'K'; break;
				case BLACK_PAWN: ss << 'p'; break;
				case BLACK_KNIGHT: ss << 'n'; break;
				case BLACK_BISHOP: ss << 'b'; break;
				case BLACK_ROOK: ss << 'r'; break;
				case BLACK_QUEEN: ss << 'q'; break;
				case BLACK_KING: ss << 'k'; break;
				case EMPTY:
					int n_empty = 0;
					while (file < 8 && squares[rank * 8 + file] == EMPTY) {
						file++;
						n_empty++;
					}
					file--;
					ss << n_empty;
					break;
			}
		}
		if (rank != 0) ss << '/';
	}
	
	// Color to move
	ss << ' ' << (white_to_move ? 'w' : 'b');
	ss << ' ';
	
	// Castling Rights
	if (!(white_OO || white_OOO || black_OO || black_OOO)) ss << '-';
	else {
		if (white_OO) ss << 'K';
		if (white_OOO) ss << 'Q';
		if (black_OO) ss << 'k';
		if (black_OOO) ss << 'q';
	}
	
	// En Passant
	ss << ' ';
	if (ep_target == 0) ss << '-';
	else ss << Coord2Text(ep_target);
	
	// Moves Without Progress
	ss << ' ' << (int)n_ply_without_progress;
	
	// Moves in Game
	ss << ' ' << 1;
	
	return ss.str();
}

Hash_t BoardState::GetHash() const {
	Hash_t output = white_to_move ? ZOBRIST_WHITE_TO_MOVE : 0;
	output ^= ZOBRIST_EN_PASSANT[ep_target];
	output ^= ZOBRIST_WHITE_OO[white_OO];
	output ^= ZOBRIST_WHITE_OOO[white_OOO];
	output ^= ZOBRIST_BLACK_OO[black_OO];
	output ^= ZOBRIST_BLACK_OOO[black_OOO];
	for (int i = 0; i < 64; i++) {
		output ^= ZOBRIST_SQUARES[squares[i]][i];
	}
	return output;
}

bool BoardState::operator == (BoardState other) const {
	other.n_ply_without_progress = this->n_ply_without_progress;
	uint8_t * this_bytes = (uint8_t *)this;
	uint8_t * other_bytes = (uint8_t *)&other;
	return memcmp(this_bytes, other_bytes, sizeof(BoardState)) == 0;
}

bool BoardState::operator < (BoardState other) const {
	other.n_ply_without_progress = this->n_ply_without_progress;
	uint8_t * this_bytes = (uint8_t *)this;
	uint8_t * other_bytes = (uint8_t *)&other;
	return memcmp(this_bytes, other_bytes, sizeof(BoardState)) < 0;
}

std::ostream & operator << (std::ostream & os, const BoardState & bs) {
	os << "+---+-----------------+-------------------------+" << std::endl;
	for (int rank = 7; rank >= 0; rank--) {
		os << "| "  << (rank + 1) << " | ";
		for (int file = 0; file < 8; file++) {
			uint8_t piece = bs.squares[rank * 8 + file];
			if (piece >= WHITE_PAWN && piece <= WHITE_KING) {
				os << (char)('1' - WHITE_PAWN + piece);
			} else if (piece >= BLACK_PAWN && piece <= BLACK_KING) {
				os << (char)('A' - BLACK_PAWN + piece);
			} else {
				os << '.';
			}
			os << ' ';
		}
		os << "|";
		
		if (rank == 7)		os << "   " << (bs.white_to_move ? "White" : "Black") << " to move";
		else if (rank == 5)	os << "   " << "White O-O:     " << (bs.white_OO ? "Yes" : "No");
		else if (rank == 4) os << "   " << "White O-O-O:   " << (bs.white_OOO ? "Yes" : "No");
		else if (rank == 3)	os << "   " << "Black O-O:     " << (bs.black_OO ? "Yes" : "No");
		else if (rank == 2) os << "   " << "Black O-O-O:   " << (bs.black_OOO ? "Yes" : "No");
		else if (rank == 1) os << "   " << "En Passant:    " << (bs.ep_target ? Coord2Text(bs.ep_target) : "--");
		else if (rank == 0) os << "   " << "Last Progress: " << (int)bs.n_ply_without_progress;
		
		os << std::endl;
	}
	os << "+---+-----------------+-------------------------+" << std::endl;
	os << "| " << bs.GetFEN() << std::endl;
	os << "+---+-----------------+-------------------------+" << std::endl;
	return os;
}

bool BoardComposite::Init(const BoardState state) {
	const Bitboard_t ONE = 1;
	
	this->state = state;
	
	// Reset the state
	white = black = wpawns = bpawns = 0;
	wking_pos = bking_pos = 255;
	hash = state.GetHash();
	#ifdef ARDALAN_DISCRETE_SCORING
	material_score = 0;
	#endif
	memset(roster, 0, 16);
	move_cache.Clear();
	unmove_cache.Clear();
	move_to_next = Move(0, 0, Move::NULL_MOVE);
	move_from_last = Move(0, 0, Move::NULL_MOVE);
	
	// Iterate through board and build up states
	for (int i = 0; i < 64; i++) {
		uint8_t piece = state.squares[i];
		if (piece >= WHITE_PAWN && piece <= WHITE_KING) {
			white |= ONE << i;
			if (piece == WHITE_PAWN) wpawns |= ONE << i;
			else if (piece == WHITE_KING) wking_pos = i;
			roster[piece]++;
		}
		else if (piece >= BLACK_PAWN && piece <= BLACK_KING) {
			black |= ONE << i;
			if (piece == BLACK_PAWN) bpawns |= ONE << i;
			else if (piece == BLACK_KING) bking_pos = i;
			roster[piece]++;
		}
	}
	
	return true;
}

std::ostream & operator << (std::ostream & os, const BoardComposite & bc) {
	os << "+=========================================================================+" << std::endl;
	os << bc.state;
	os << "| THIS: " << &bc << std::endl;
	os << "| Move From Last: " << bc.move_from_last << " (" << bc.last << ")" << std::endl;
	os << "| Move To Next:   " << bc.move_to_next << " (" << bc.next << ")" << std::endl;
	os << std::hex;
	os << "| White:       " << std::setfill('0') << std::setw(16) << bc.white << std::endl;
	os << "| White Pawns: " << std::setfill('0') << std::setw(16) << bc.wpawns << std::endl;
	os << "| Black:       " << std::setfill('0') << std::setw(16) << bc.black << std::endl;
	os << "| Black Pawns: " << std::setfill('0') << std::setw(16) << bc.bpawns << std::endl;
	os << "| Hash:        " << std::setfill('0') << std::setw(16) << bc.hash << std::endl;
	os << std::dec;
	os << "| Roster: " << std::endl;
	os << "|     White: [" << (int)bc.roster[WHITE_PAWN] << " pawns, " << (int)bc.roster[WHITE_KNIGHT] << " knights, " << (int)bc.roster[WHITE_BISHOP] << " bishops, " << (int)bc.roster[WHITE_ROOK] << " rooks, " << (int)bc.roster[WHITE_QUEEN] << " queens, " << (int)bc.roster[WHITE_KING] << " kings]" << std::endl;
	os << "|     Black: [" << (int)bc.roster[BLACK_PAWN] << " pawns, " << (int)bc.roster[BLACK_KNIGHT] << " knights, " << (int)bc.roster[BLACK_BISHOP] << " bishops, " << (int)bc.roster[BLACK_ROOK] << " rooks, " << (int)bc.roster[BLACK_QUEEN] << " queens, " << (int)bc.roster[BLACK_KING] << " kings]" << std::endl;
	os << "| White King: " << Coord2Text(bc.wking_pos) << std::endl;
	os << "| Black King: " << Coord2Text(bc.bking_pos) << std::endl;
	os << "| Move cache:  " << bc.move_cache << std::endl;
	os << "| Unmove cache: " << bc.unmove_cache << std::endl;
	os << "+=========================================================================+" << std::endl;
	return os;
}