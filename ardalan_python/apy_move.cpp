#include "apy_types.h"

#include <iostream>
#include <sstream>

/*******************************************************************************
 * Move
 */
static const char * MOVE_CLASS_NAME_STR = "ardalan.Move";
static const char * MOVE_CLASS_DOCSTR =
"Move that can be made to the board state.";

void APy_Move_dealloc(PyObject * self)
{
	Py_TYPE(self)->tp_free((PyObject *)self);
}

PyObject * APy_Move_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
	APy_Move * self;
	self = (APy_Move *)type->tp_alloc(type, 0);
	return (PyObject *)self;
}

int APy_Move_init(APy_Move * self, PyObject * args, PyObject * kwds)
{
	char * arg_str = NULL;
	int8_t arg_start = -1;
	int8_t arg_end = -1;
	int8_t arg_code = -1;
	static char * keywords[] = {
		"string",
		"start",
		"end",
		"code",
		NULL
	};
	
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|sBBB", keywords,
			&arg_str, &arg_start, &arg_end, &arg_code)) {
		return -1;
	}
	
	if (arg_str) {
		// Initialize by parsing string
		
		// Cannot have any other arguments initialized
		if (arg_start != -1 || arg_end != -1 || arg_code != -1) {
			PyErr_SetString(PyExc_TypeError, "Cannot initialize to both a string and specified arguments");
			return -1;
		}
		
		self->m_move = Move(arg_str);
	}
	else {
		// Initialize by explicit arguments
		if (arg_code != -1) {
			// If only code, then must be castling or null move
			// Check and set code and do nothing else
			if (arg_start == -1 && arg_end == -1) {
				if (arg_code == Move::WHITE_OO || arg_code == Move::WHITE_OOO ||
					arg_code == Move::BLACK_OO || arg_code == Move::BLACK_OOO ||
					arg_code == Move::NULL_MOVE)
				{
					self->m_move = Move(0, 0, arg_code);
				}
				else {
					PyErr_SetString(PyExc_TypeError, "If only the 'code' argument is supplied, then the code must be for castling");
					return -1;
				}
			}
			// En Passant, explicit Normal, Promotion moves
			else if (arg_start != -1 && arg_end != -1) {
				if (arg_code == Move::EN_PASSANT || arg_code == Move::NORMAL_MOVE ||
					arg_code == WHITE_KNIGHT || arg_code == WHITE_BISHOP ||
					arg_code == WHITE_ROOK || arg_code == WHITE_QUEEN ||
					arg_code == BLACK_KNIGHT || arg_code == BLACK_BISHOP ||
					arg_code == BLACK_ROOK || arg_code == BLACK_QUEEN)
				{
					// Check that start and end are in range
					if (!(arg_start >= 0 && arg_start <= 63) || !(arg_end >= 0 && arg_end <= 63)) {
						PyErr_SetString(PyExc_TypeError, "'start' and 'end' arguments must be between 0 and 63 inclusive");
						return -1;
					}
					self->m_move = Move(arg_start, arg_end, arg_code);
				}
			}
			else {
				PyErr_SetString(PyExc_TypeError, "Invalid combination of 'start', 'end', and 'code' arguments");
				return -1;
			}
				
		}
		else {
			// If nothing, then null move
			if (arg_start == -1 && arg_end == -1) {
				self->m_move = Move(0, 0, Move::NULL_MOVE);
			}
			// Check for start and end
			else {
				if (arg_start == -1 || arg_end == -1) {
			
					PyErr_SetString(PyExc_TypeError, "'start' and 'end' arguments must be supplied for a normal move");
					return -1;
				}
				// Check that start and end are in range
				if (!(arg_start >= 0 && arg_start <= 63) || !(arg_end >= 0 && arg_end <= 63)) {
					PyErr_SetString(PyExc_TypeError, "'start' and 'end' arguments must be between 0 and 63 inclusive");
					return -1;
				}
				
				self->m_move.start = arg_start;
				self->m_move.end = arg_end;
			}
		}
	}
	
	return 0;
}

PyObject * APy_Move_str(PyObject * self_arg)
{
	APy_Move * self = (APy_Move *)self_arg;
	
	// Put C++ output into a stream and convert into a Python string
	std::ostringstream stream;
	stream << self->m_move;
	std::string stdstr = stream.str();
	const char * cstr = stdstr.c_str();
	PyObject * strout = APy_PyString_FromString(cstr);
	return strout;
}

PyObject * APy_Move_repr(PyObject * self_arg) {
	APy_Move * self = (APy_Move *)self_arg;
	
	// Put C++ output into a stream and convert into a Python string
	std::ostringstream stream;
	stream << self->m_move;
	std::string stdstr = stream.str();
	const char * cstr = stdstr.c_str();
	PyObject * strout = APy_PyString_FromFormat("<ardalan.Move %s>", cstr);
	return strout;
}

PyObject* APy_Move_richcmp(PyObject * self_arg, PyObject * other_arg, int op) {
	// Check for type matching
	if (!PyObject_TypeCheck(self_arg, &APy_MoveType) ||
		!PyObject_TypeCheck(other_arg, &APy_MoveType)) {
		return Py_NotImplemented;
	}
	
	APy_Move * self = (APy_Move *)self_arg;
	APy_Move * other = (APy_Move *)other_arg;
	
	bool equality = self->m_move == other->m_move;
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

PyObject* APy_Move_dict = PyDict_New();

PyTypeObject APy_MoveType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    MOVE_CLASS_NAME_STR,			/* tp_name */
    sizeof(APy_Move),        		/* tp_basicsize */
    0,                         		/* tp_itemsize */
    (destructor)APy_Move_dealloc, 	/* tp_dealloc */
    0,                         		/* tp_print */
    0,                         		/* tp_getattr */
    0,                         		/* tp_setattr */
    NULL,							/* tp_compare */
    APy_Move_repr,					/* tp_repr */
    0,                         		/* tp_as_number */
    0,                         		/* tp_as_sequence */
    0,                         		/* tp_as_mapping */
    0,                         		/* tp_hash */
    0,                         		/* tp_call */
    APy_Move_str,					/* tp_str */
    0,                         		/* tp_getattro */
    0,                         		/* tp_setattro */
    0,                         		/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |   			/* tp_flags */
        Py_TPFLAGS_BASETYPE,
    MOVE_CLASS_DOCSTR,				/* tp_doc */
    0,                         		/* tp_traverse */
    0,                         		/* tp_clear */
    APy_Move_richcmp,				/* tp_richcompare */
    0,                         		/* tp_weaklistoffset */
    0,                         		/* tp_iter */
    0,                         		/* tp_iternext */
    NULL,             				/* tp_methods */
    NULL,             				/* tp_members */
    NULL,							/* tp_getset */
    0,                         		/* tp_base */
    APy_Move_dict,					/* tp_dict */
    0,                         		/* tp_descr_get */
    0,                         		/* tp_descr_set */
    0,                         		/* tp_dictoffset */
    (initproc)APy_Move_init,      	/* tp_init */
    0,                         		/* tp_alloc */
    APy_Move_new,                 	/* tp_new */
};