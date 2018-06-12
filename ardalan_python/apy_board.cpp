#include "apy_types.h"

#include <iostream>
#include <sstream>

/*******************************************************************************
 * Board
 */
static const char * BOARD_CLASS_NAME_STR = "ardalan.Board";
static const char * BOARD_CLASS_DOCSTR =
"Complete board representation.\n"
" - Moves can be made and unmade\n"
" - Possible pseudo-legal moves can be generated\n"
" - Checks can be found\n"
" - Checkmate, stalemate, and types of draws can be detected\n"
"\n"
"Initialization:\n"
" - No Parameters: initializes to the default chess starting position\n"
" - fen: initializes to a FEN string\n"
" - state: initializes to an existing ardalan.State object\n"
"NOTE: only one parameter maximum may be passed\n";

void APy_Board_dealloc(APy_Board * self)
{
	delete self->m_board;
	Py_TYPE(self)->tp_free((PyObject *)self);
}

PyObject * APy_Board_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
	APy_Board * self;
	
	self = (APy_Board *)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->m_board = NULL;
	}
	
	return (PyObject *)self;
}

int APy_Board_init(APy_Board * self, PyObject * args, PyObject * kwds)
{
	// Can initialize to: default position, FEN string, existing board state
	char * arg_fen = NULL;
	PyObject * arg_state = NULL;
	static char * keywords[] = {
		"fen",
		"state",
		NULL
	};
	
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|sO", keywords, &arg_fen, &arg_state)) {
		return -1;
	}
	
	if (self->m_board) delete self->m_board;
	self->m_board = new Board();
	
	
	if (arg_fen) {
		if (arg_state) {
			// Error: cannot initialize from both
			PyErr_SetString(PyExc_TypeError, "Cannot initialize both 'fen' and 'state' arguments simultaneously");
			return -1;
		}
		else {
			// Initialize from FEN string			
			// Scan string into new instance
			BoardState state;
			if (!state.InitFromFEN(arg_fen)) {
				PyErr_SetString(PyExc_ValueError, "The FEN string was not successfully processed. Please verify the string");
				return -1;
			}
			self->m_board->SetCurrent(state);
		}
	}
	else {
		if (arg_state) {
			// Initialize from state object
			if (APy_Board_SET_current((PyObject *)self, arg_state, NULL)) {
				PyErr_SetString(PyExc_TypeError, "'state' argument must be an instance of ardalan.State");
				return -1;
			}
		}
		else {
			// Default starting position
			// Do nothing special
			self->m_board->SetCurrent(BoardState());
		}
	}
	
	return 0;
}

PyObject * APy_Board_str(PyObject * self_arg)
{
	APy_Board * self = (APy_Board *)self_arg;
	
	// Put C++ output into a stream and convert into a Python string
	std::ostringstream stream;
	stream << *self->m_board;
	std::string stdstr = stream.str();
	const char * cstr = stdstr.c_str();
	PyObject * strout = APy_PyString_FromString(cstr);
	return strout;
}

PyObject* APy_Board_richcmp(PyObject * self_arg, PyObject * other_arg, int op)
{
	// Check for type matching
	if (!PyObject_TypeCheck(self_arg, &APy_BoardType) ||
		!PyObject_TypeCheck(other_arg, &APy_BoardType)) {
		return Py_NotImplemented;
	}
	
	APy_Board * self = (APy_Board *)self_arg;
	APy_Board * other = (APy_Board *)other_arg;
	
	bool equality = self->m_board->GetCurrent() == other->m_board->GetCurrent();
	if (op == Py_EQ) {
		if (equality) Py_RETURN_TRUE;
		else Py_RETURN_FALSE;
	}
	else if (op == Py_NE) {
		if (equality) Py_RETURN_FALSE;
		else Py_RETURN_TRUE;
	}
	else return Py_NotImplemented;
}

static const char * BOARD_GETSET_CURRENT_DOCSTR =
"Set the current state of the board";
PyObject * APy_Board_GET_current(PyObject * self_arg, void * closure)
{
	APy_Board * self = (APy_Board *)self_arg;
	APy_State * state = (APy_State *)APy_State_new(&APy_StateType, NULL, NULL);
	state->m_state = self->m_board->GetCurrent();
	return (PyObject *)state;
}

int APy_Board_SET_current(PyObject * self_arg, PyObject * value, void * closure)
{
	APy_Board * self = (APy_Board *)self_arg;
	
	// Validate type
	if (!PyObject_TypeCheck(value, &APy_StateType)) {
		PyErr_SetString(PyExc_TypeError, "Must set to an instance of ardalan.State");
		return -1;
	}
	
	APy_State * state = (APy_State *)value;
	self->m_board->SetCurrent(state->m_state);
	
	return 0;
}

static const char * BOARD_GET_INITIAL_DOCSTR = 
"Get the original position for this board";
PyObject * APy_Board_GET_initial(PyObject * self_arg, void * closure)
{
	APy_Board * self = (APy_Board *)self_arg;
	APy_State * state = (APy_State *)APy_State_new(&APy_StateType, NULL, NULL);
	state->m_state = self->m_board->GetInitial();
	return (PyObject *)state;
}

static const char * BOARD_GET_DEPTH_DOCSTR =
"Get the index of the current position relative to the initial position";
PyObject * APy_Board_GET_depth(PyObject * self_arg, void * closure)
{
	APy_Board * self = (APy_Board *)self_arg;
	PyObject * output = PyLong_FromLong(self->m_board->GetDepth());
	Py_INCREF(output);
	return output;
}

static const char * BOARD_MAKE_DOCSTR =
"Make a move to the board state.\n"
"Returns True if the move was successfully made.\n"
"Returns False if the move was not successful; the board state does not change if this is the case";
PyObject * APy_Board_Make(PyObject * self_arg, PyObject * args, PyObject * kwds) {
	APy_Board * self = (APy_Board *)self_arg;
	PyObject * move_arg = NULL;
	if (!PyArg_ParseTuple(args, "O", &move_arg)) {
		return NULL;
	}
	
	// Validate type
	if (!PyObject_TypeCheck(move_arg, &APy_MoveType)) {
		PyErr_SetString(PyExc_TypeError, "Must set to an instance of ardalan.Move");
		return NULL;
	}
	
	APy_Move * move = (APy_Move *)move_arg;
	bool result = self->m_board->Make(move->m_move);
	if (result) Py_RETURN_TRUE;
	else Py_RETURN_FALSE;
}

static const char * BOARD_UNMAKE_DOCSTR =
"Undo a number of moves from the board state.\n"
"Returns true if the requested depth was unmade.\n"
"Returns false if the requested depth exceeds the board's depth.\n"
"In those cases, as many moves as possible are unmade.";
PyObject * APy_Board_Unmake(PyObject * self_arg, PyObject * args, PyObject * kwds) {
	APy_Board * self = (APy_Board *)self_arg;
	int n_moves = -1;
	if (!PyArg_ParseTuple(args, "i", &n_moves)) {
		return NULL;
	}
	
	if (n_moves <= 0) {
		PyErr_SetString(PyExc_TypeError, "Must unmake a positive number of moves");
		return NULL;
	}
	
	bool result = self->m_board->Unmake(n_moves);
	if (result) Py_RETURN_TRUE;
	else Py_RETURN_FALSE;
}

static const char * BOARD_GETMOVES_DOCSTR =
"Get the moves available in the current position.\n"
"Illegal moves (like moving into check) are not filtered out.\n";
PyObject * APy_Board_GetMoves(PyObject * self_arg, PyObject * args, PyObject * kwds) {
	APy_Board * self = (APy_Board *)self_arg;
	const MoveList * moves = self->m_board->GetMoves();
	if (!moves->IsValid()) {
		Py_RETURN_NONE;
	}
	
	int n_moves = moves->Length();
	const Move * move_i = moves->Begin();
	PyObject * output = PyList_New(n_moves);
	for (int i = 0; i < n_moves; i++, move_i++) {
		APy_Move * new_move = (APy_Move *)APy_Move_new(&APy_MoveType, NULL, NULL);
		new_move->m_move = *move_i;
		PyList_SetItem(output, i, (PyObject *)new_move);
	}
	
	return output;
}

static const char * BOARD_GETLEGALMOVES_DOCSTR =
"Get the legal moves available in the current position.\n";
PyObject * APy_Board_GetLegalMoves(PyObject * self_arg, PyObject * args, PyObject * kwds) {
	APy_Board * self = (APy_Board *)self_arg;
	const MoveList * moves = self->m_board->GetMoves();
	if (!moves->IsValid()) {
		Py_RETURN_NONE;
	}
	
	int n_moves = moves->Length();
	Move * legal = new Move[n_moves];
	int n_legal = 0;
	
	bool white_to_move = self->m_board->GetCurrent().white_to_move;
	
	const Move * move_i = moves->Begin();
	for (int i = 0; i < n_moves; i++, move_i++) {
		if (self->m_board->Make(*move_i)) {
			if (!self->m_board->InCheck(white_to_move)) {
				legal[n_legal++] = *move_i;
			}
			self->m_board->Unmake(1);
		}
	}
	
	PyObject * output = PyList_New(n_legal);
	for (int i = 0; i < n_legal; i++) {
		APy_Move * move = (APy_Move *)APy_Move_new(&APy_MoveType, NULL, NULL);
		move->m_move = legal[i];
		PyList_SetItem(output, i, (PyObject *)move);
	}
	
	return output;
}

static const char * BOARD_INCHECK_DOCSTR =
"Determine whether the specified color is in check.\n";
PyObject * APy_Board_InCheck(PyObject * self_arg, PyObject * args, PyObject * kwds) {
	APy_Board * self = (APy_Board *)self_arg;
	uint8_t arg_is_white = -1;
	if (!PyArg_ParseTuple(args, "B", &arg_is_white)) {
		PyErr_SetString(PyExc_TypeError, "Argument must a bool of whether to check for white");
		return NULL;
	}
	
	bool is_white = arg_is_white;
	bool result = self->m_board->InCheck(is_white);
	if (result) Py_RETURN_TRUE;
	else Py_RETURN_FALSE;
}

static const char * BOARD_ISCHECKMATE_DOCSTR =
"Determine whether the color to move is in checkmate.\n";
PyObject * APy_Board_IsCheckmate(PyObject * self_arg, PyObject * args, PyObject * kwds) {
	APy_Board * self = (APy_Board *)self_arg;
	bool result = self->m_board->IsCheckmate();
	if (result) Py_RETURN_TRUE;
	else Py_RETURN_FALSE;
}

static const char * BOARD_ISSTALEMATE_DOCSTR =
"Determine whether the color to move is in stalemate.\n";
PyObject * APy_Board_IsStalemate(PyObject * self_arg, PyObject * args, PyObject * kwds) {
	APy_Board * self = (APy_Board *)self_arg;
	bool result = self->m_board->IsStalemate();
	if (result) Py_RETURN_TRUE;
	else Py_RETURN_FALSE;
}

static const char * BOARD_ISDRAW_DOCSTR =
"Determine whether any draw conditions are met.\n";
PyObject * APy_Board_IsDraw(PyObject * self_arg, PyObject * args, PyObject * kwds) {
	APy_Board * self = (APy_Board *)self_arg;
	bool result = self->m_board->IsDraw();
	if (result) Py_RETURN_TRUE;
	else Py_RETURN_FALSE;
}

PyGetSetDef APy_Board_properties[] = {
	{"current",		APy_Board_GET_current,	APy_Board_SET_current,	(char *)BOARD_GETSET_CURRENT_DOCSTR,	NULL},
	{"initial",		APy_Board_GET_initial,	NULL,					(char *)BOARD_GET_INITIAL_DOCSTR,		NULL},
	{"depth",		APy_Board_GET_depth,	NULL,					(char *)BOARD_GET_DEPTH_DOCSTR,			NULL},
	{NULL,	NULL,	NULL,	NULL,	NULL}
};

PyMethodDef APy_Board_methods[] = {
	{"Make",			(PyCFunction)APy_Board_Make,				METH_VARARGS,	(char *)BOARD_MAKE_DOCSTR},
	{"Unmake",			(PyCFunction)APy_Board_Unmake,				METH_VARARGS,	(char *)BOARD_UNMAKE_DOCSTR},
	{"GetMoves",		(PyCFunction)APy_Board_GetMoves,			METH_NOARGS,	(char *)BOARD_GETMOVES_DOCSTR},
	{"GetLegalMoves",	(PyCFunction)APy_Board_GetLegalMoves,		METH_NOARGS,	(char *)BOARD_GETLEGALMOVES_DOCSTR},
	{"InCheck",			(PyCFunction)APy_Board_InCheck,				METH_VARARGS,	(char *)BOARD_INCHECK_DOCSTR},
	{"IsCheckmate",		(PyCFunction)APy_Board_IsCheckmate,			METH_NOARGS,	(char *)BOARD_ISCHECKMATE_DOCSTR},
	{"IsStalemate",		(PyCFunction)APy_Board_IsStalemate,			METH_NOARGS,	(char *)BOARD_ISSTALEMATE_DOCSTR},
	{"IsDraw",			(PyCFunction)APy_Board_IsDraw,				METH_NOARGS,	(char *)BOARD_ISDRAW_DOCSTR},
	{NULL,				NULL,						0,				NULL}
};

PyObject* APy_Board_dict = PyDict_New();

PyTypeObject APy_BoardType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    BOARD_CLASS_NAME_STR,           /* tp_name */
    sizeof(APy_Board),        		/* tp_basicsize */
    0,                         		/* tp_itemsize */
    (destructor)APy_Board_dealloc, 	/* tp_dealloc */
    0,                         		/* tp_print */
    0,                         		/* tp_getattr */
    0,                         		/* tp_setattr */
    NULL,							/* tp_compare */
    NULL,	        				/* tp_repr */
    0,                         		/* tp_as_number */
    0,                         		/* tp_as_sequence */
    0,                         		/* tp_as_mapping */
    0,                         		/* tp_hash */
    0,                         		/* tp_call */
    APy_Board_str,                  /* tp_str */
    0,                         		/* tp_getattro */
    0,                         		/* tp_setattro */
    0,                         		/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |   			/* tp_flags */
        Py_TPFLAGS_BASETYPE,
    BOARD_CLASS_DOCSTR,				/* tp_doc */
    0,                         		/* tp_traverse */
    0,                         		/* tp_clear */
    APy_Board_richcmp,				/* tp_richcompare */
    0,                         		/* tp_weaklistoffset */
    0,                         		/* tp_iter */
    0,                         		/* tp_iternext */
    APy_Board_methods,             	/* tp_methods */
    NULL,             				/* tp_members */
    APy_Board_properties, 			/* tp_getset */
    0,                         		/* tp_base */
    APy_Board_dict,                 /* tp_dict */
    0,                         		/* tp_descr_get */
    0,                         		/* tp_descr_set */
    0,                         		/* tp_dictoffset */
    (initproc)APy_Board_init,      	/* tp_init */
    0,                         		/* tp_alloc */
    APy_Board_new,                 	/* tp_new */
};