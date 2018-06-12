#include "apy_types.h"

#include <iostream>
#include <sstream>

/*******************************************************************************
 * State
 */
static const char * STATE_CLASS_NAME_STR = "ardalan.State";
static const char * STATE_CLASS_DOCSTR =
"Minimal data needed to represent a position.\n"
"Included data:\n"
" - Positions of all pieces (encoded in a 64-element array)\n"
" - Color to move\n"
" - Castling rights\n"
" - En Passant options\n"
" - Number of ply since the last progress was made\n"
"\n"
"Initialization:\n"
" - No Parameters: initializes to the default chess starting position\n"
"Any of the following parameters may be used in any combination with the others.\n"
"If a parameter is omitted, the default chess starting state will be used.\n"
" - squares: 64-element list containing which type of piece occupies each square\n"
" - white_to_move: boolean of whether it is white's turn to move\n"
" - white_OO: boolean of whether white may kingside castle\n"
" - white_OOO: boolean of whether white may queenside castle\n"
" - black_OO: boolean of whether black may kindside castle\n"
" - black_OOO: boolean of whether black may queenside castle\n"
" - ep: the square where the pawn to be captured is located. 0 if no en passant\n"
" - nply: the number of ply since last progress\n"
"Instead of any of the above parameters, a FEN string may be supplied\n"
" - fen: a FEN string\n";

void APy_State_dealloc(PyObject * self)
{
	Py_TYPE(self)->tp_free((PyObject *)self);
}

PyObject * APy_State_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
	APy_State * self;
	self = (APy_State *)type->tp_alloc(type, 0);
	return (PyObject *)self;
}

int APy_State_init(APy_State * self, PyObject * args, PyObject * kwds)
{
	// Can initialize to: default position, mixed-and-matched parameters, FEN string
	/*
	PyObject * arg_squares = NULL;
	uint8_t arg_white_to_move = true;
	uint8_t arg_white_OO = true;
	uint8_t arg_white_OOO = true;
	uint8_t arg_black_OO = true;
	uint8_t arg_black_OOO = true;
	uint8_t arg_ep = 0;
	uint16_t arg_nply = 0;
	PyObject * arg_fen = NULL;
	*/
	PyObject * arg_squares = NULL;
	PyObject * arg_white_to_move = NULL;
	PyObject * arg_white_OO = NULL;
	PyObject * arg_white_OOO = NULL;
	PyObject * arg_black_OO = NULL;
	PyObject * arg_black_OOO = NULL;
	PyObject * arg_ep = NULL;
	PyObject * arg_nply = NULL;
	char * arg_fen = NULL;
	static char * keywords[] = {
		"squares",
		"white_to_move",
		"white_OO",
		"white_OOO",
		"black_OO",
		"black_OOO",
		"ep",
		"nply",
		"fen",
		NULL
	};
	
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOOOOOOOs", keywords,
			&arg_squares, &arg_white_to_move, &arg_white_OO, &arg_white_OOO, &arg_black_OO, &arg_black_OOO,
			&arg_ep, &arg_nply, &arg_fen)) {
		return -1;
	}
	
	// Validate inputs
	bool custom_init = arg_squares || arg_white_to_move || arg_white_OO || arg_white_OOO
			|| arg_black_OO || arg_black_OOO || arg_ep || arg_nply;
	
	if (custom_init) {
		if (arg_fen) {
			// Error: cannot initialize from both
			PyErr_SetString(PyExc_ValueError, "Cannot initialize from both FEN string and custom parameters");
			return -1;
		}
		else {
			// Initialize from custom parameters
			self->m_state = BoardState();
			
			// Validate parameters
			if (arg_squares) {
				if (PyList_Check(arg_squares) && PyList_Size(arg_squares) == 64) {
					for (int i = 0; i < 64; i++) {
						self->m_state.squares[i] = PyLong_AsLong(PyList_GetItem(arg_squares, i));
					}
				}
				else {
					PyErr_SetString(PyExc_ValueError, "'squares' argument must be a 64-element list of integers");
					return -1;
				}
			}
			
			if (arg_white_to_move) {
				if (PyBool_Check(arg_white_to_move)) {
					self->m_state.white_to_move = PyLong_AsLong(arg_white_to_move);
				}
				else {
					PyErr_SetString(PyExc_TypeError, "'white_to_move' argument must be a boolean");
					return -1;
				}
			}
			
			if (arg_white_OO) {
				if (PyBool_Check(arg_white_OO)) {
					self->m_state.white_OO = PyLong_AsLong(arg_white_OO);
				}
				else {
					PyErr_SetString(PyExc_TypeError, "'white_OO' argument must be a boolean");
					return -1;
				}
			}
			
			if (arg_white_OOO) {
				if (PyBool_Check(arg_white_OOO)) {
					self->m_state.white_OOO = PyLong_AsLong(arg_white_OOO);
				}
				else {
					PyErr_SetString(PyExc_TypeError, "'white_OOO' argument must be a boolean");
					return -1;
				}
			}
			
			if (arg_black_OO) {
				if (PyBool_Check(arg_black_OO)) {
					self->m_state.black_OO = PyLong_AsLong(arg_black_OO);
				}
				else {
					PyErr_SetString(PyExc_TypeError, "'black_OO' argument must be a boolean");
					return -1;
				}
			}
			
			if (arg_black_OOO) {
				if (PyBool_Check(arg_black_OOO)) {
					self->m_state.black_OOO = PyLong_AsLong(arg_black_OOO);
				}
				else {
					PyErr_SetString(PyExc_TypeError, "'black_OOO' argument must be a boolean");
					return -1;
				}
			}
			
			if (arg_ep) {
				if (PyLong_Check(arg_ep)) {
					self->m_state.ep_target = PyLong_AsLong(arg_ep);
				}
				else {
					PyErr_SetString(PyExc_TypeError, "'ep' argument must be an integer");
					return -1;
				}
			}
			
			if (arg_nply) {
				if (PyLong_Check(arg_nply)) {
					self->m_state.n_ply_without_progress = PyLong_AsLong(arg_nply);
				}
				else {
					PyErr_SetString(PyExc_TypeError, "'nply' argument must be an integer");
					return -1;
				}
			}
		}
	}
	else {
		if (arg_fen) {
			// Initialize from FEN string
			self->m_state.InitFromFEN(arg_fen);
		}
		else {
			// Default board state
			self->m_state = BoardState();
		}
	}
	
	return 0;
}

PyObject * APy_State_str(PyObject * self_arg)
{
	APy_State * self = (APy_State *)self_arg;
	
	// Put C++ output into a stream and convert into a Python string
	std::ostringstream stream;
	stream << self->m_state;
	std::string stdstr = stream.str();
	const char * cstr = stdstr.c_str();
	PyObject * strout = APy_PyString_FromString(cstr);
	return strout;
}

PyObject* APy_State_richcmp(PyObject * self_arg, PyObject * other_arg, int op)
{
	// Check for type matching
	if (!PyObject_TypeCheck(self_arg, &APy_StateType) ||
		!PyObject_TypeCheck(other_arg, &APy_StateType)) {
		return Py_NotImplemented;
	}
	
	APy_State * self = (APy_State *)self_arg;
	APy_State * other = (APy_State *)other_arg;
	
	bool equality = self->m_state == other->m_state;
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

static const char * STATE_GET_FEN_DOCSTR =
"Get the FEN string representing the position\n"
"This value is calculated as-needed; it is not stored in a cache";
PyObject * APy_State_GET_fen(PyObject * self_arg, void * closure)
{
	APy_State * self = (APy_State *)self_arg;
	
	// Convert C++ string into a Python string
	std::string stdstr = self->m_state.GetFEN();
	const char * cstr = stdstr.c_str();
	PyObject * strout = APy_PyString_FromString(cstr);
	return strout;
}

static const char * STATE_GET_WHITE_TO_MOVE_DOCSTR = 
"Whether white is the next color to move in the current position";
PyObject * APy_State_GET_white_to_move(PyObject * self_arg, void * closure)
{
	APy_State * self = (APy_State *)self_arg;
	if (self->m_state.white_to_move) Py_RETURN_TRUE;
	else Py_RETURN_FALSE;
}

static const char * STATE_GET_WHITE_OO_DOCSTR = 
"Whether white may castle kingside";
PyObject * APy_State_GET_white_OO(PyObject * self_arg, void * closure)
{
	APy_State * self = (APy_State *)self_arg;
	if (self->m_state.white_OO) Py_RETURN_TRUE;
	else Py_RETURN_FALSE;
}

static const char * STATE_GET_WHITE_OOO_DOCSTR = 
"Whether white may castle queenside";
PyObject * APy_State_GET_white_OOO(PyObject * self_arg, void * closure)
{
	APy_State * self = (APy_State *)self_arg;
	if (self->m_state.white_OOO) Py_RETURN_TRUE;
	else Py_RETURN_FALSE;
}

static const char * STATE_GET_BLACK_OO_DOCSTR = 
"Whether black may castle kingside";
PyObject * APy_State_GET_black_OO(PyObject * self_arg, void * closure)
{
	APy_State * self = (APy_State *)self_arg;
	if (self->m_state.black_OO) Py_RETURN_TRUE;
	else Py_RETURN_FALSE;
}

static const char * STATE_GET_BLACK_OOO_DOCSTR = 
"Whether black may castle queenside";
PyObject * APy_State_GET_black_OOO(PyObject * self_arg, void * closure)
{
	APy_State * self = (APy_State *)self_arg;
	if (self->m_state.black_OOO) Py_RETURN_TRUE;
	else Py_RETURN_FALSE;
}

static const char * STATE_GET_EP_TARGET_DOCSTR =
"The square to which a pawn may en passant; returns None if no e.p.";
PyObject * APy_State_GET_ep_target(PyObject * self_arg, void * closure)
{
	APy_State * self = (APy_State *)self_arg;
	PyObject * outint = PyLong_FromLong(self->m_state.ep_target);
	Py_INCREF(outint);
	return outint;
}

static const char * STATE_GET_NPLY_DOCSTR =
"The number of ply since the last move that made progress";
PyObject * APy_State_GET_nply(PyObject * self_arg, void * closure)
{
	APy_State * self = (APy_State *)self_arg;
	PyObject * outint = PyLong_FromLong(self->m_state.n_ply_without_progress);
	Py_INCREF(outint);
	return outint;
}


PyGetSetDef APy_State_properties[] = {
	{"fen",				APy_State_GET_fen,				NULL,		(char *)STATE_GET_FEN_DOCSTR,				NULL},
	{"white_to_move",	APy_State_GET_white_to_move,	NULL,		(char *)STATE_GET_WHITE_TO_MOVE_DOCSTR,		NULL},
	{"white_OO",		APy_State_GET_white_OO,			NULL,		(char *)STATE_GET_WHITE_OO_DOCSTR,			NULL},
	{"white_OOO",		APy_State_GET_white_OOO,		NULL,		(char *)STATE_GET_WHITE_OOO_DOCSTR,			NULL},
	{"black_OO",		APy_State_GET_black_OO,			NULL,		(char *)STATE_GET_BLACK_OO_DOCSTR,			NULL},
	{"black_OOO",		APy_State_GET_black_OOO,		NULL,		(char *)STATE_GET_BLACK_OOO_DOCSTR,			NULL},
	{"ep_target",		APy_State_GET_ep_target,		NULL,		(char *)STATE_GET_EP_TARGET_DOCSTR,			NULL},
	{"nply",			APy_State_GET_nply,				NULL,		(char *)STATE_GET_NPLY_DOCSTR,				NULL},
	{NULL,				NULL,							NULL,		NULL,										NULL}
};

PyObject* APy_State_dict = PyDict_New();

PyTypeObject APy_StateType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    STATE_CLASS_NAME_STR,           /* tp_name */
    sizeof(APy_State),        		/* tp_basicsize */
    0,                         		/* tp_itemsize */
    APy_State_dealloc, 				/* tp_dealloc */
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
    APy_State_str,                  /* tp_str */
    0,                         		/* tp_getattro */
    0,                         		/* tp_setattro */
    0,                         		/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |   			/* tp_flags */
        Py_TPFLAGS_BASETYPE,
    STATE_CLASS_DOCSTR,				/* tp_doc */
    0,                         		/* tp_traverse */
    0,                         		/* tp_clear */
    APy_State_richcmp,				/* tp_richcompare */
    0,                         		/* tp_weaklistoffset */
    0,                         		/* tp_iter */
    0,                         		/* tp_iternext */
    NULL,             				/* tp_methods */
    NULL,             				/* tp_members */
    APy_State_properties, 			/* tp_getset */
    0,                         		/* tp_base */
    APy_State_dict,                 /* tp_dict */
    0,                         		/* tp_descr_get */
    0,                         		/* tp_descr_set */
    0,                         		/* tp_dictoffset */
    (initproc)APy_State_init,      	/* tp_init */
    0,                         		/* tp_alloc */
    APy_State_new,		            /* tp_new */
};