#ifndef _APY_BOARD_H_
#define _APY_BOARD_H_

#include <board.h>

#include <Python.h>

/*******************************************************************************
 * Macros for Python version interoperability
 */
#ifdef PYTHON2
// Python 2.x
#define APy_PyString_FromString		PyString_FromString
#define APy_PyString_FromFormat		PyString_FromFormat

#else

#ifdef PYTHON3
// Python 3.x
#define APy_PyString_FromString		PyUnicode_FromString
#define APy_PyString_FromFormat		PyUnicode_FromFormat

#endif
#endif

/*******************************************************************************
 * Board
 */
// Data Structure
typedef struct {
	PyObject_HEAD
	Board * m_board;
} APy_Board;


// Python Methods
void 		APy_Board_dealloc	(APy_Board * self);
PyObject* 	APy_Board_new		(PyTypeObject * type, PyObject * args, PyObject * kwds);
int 		APy_Board_init		(APy_Board * self, PyObject * args, PyObject * kwds);
PyObject*	APy_Board_str		(PyObject * self_arg);
PyObject*	APy_Board_richcmp	(PyObject * self_arg, PyObject * other_arg, int op);


// Properties
PyObject*	APy_Board_GET_depth		(PyObject * self_arg, void * closure);
PyObject*	APy_Board_GET_current	(PyObject * self_arg, void * closure);
PyObject*	APy_Board_GET_initial	(PyObject * self_arg, void * closure);

int			APy_Board_SET_current	(PyObject * self_arg, PyObject * value, void * closure);


// Methods
PyObject*	APy_Board_Make			(PyObject * self_arg, PyObject * args, PyObject * kwds);
PyObject*	APy_Board_Unmake		(PyObject * self_arg, PyObject * args, PyObject * kwds);
PyObject*	APy_Board_GetMoves		(PyObject * self_arg, PyObject * args, PyObject * kwds);
PyObject*	APy_Board_GetLegalMoves	(PyObject * self_arg, PyObject * args, PyObject * kwds);
PyObject*	APy_Board_InCheck		(PyObject * self_arg, PyObject * args, PyObject * kwds);
PyObject*	APy_Board_IsCheckmate	(PyObject * self_arg, PyObject * args, PyObject * kwds);
PyObject*	APy_Board_IsStalemate	(PyObject * self_arg, PyObject * args, PyObject * kwds);
PyObject*	APy_Board_IsDraw		(PyObject * self_arg, PyObject * args, PyObject * kwds);


// Python Type
extern PyGetSetDef	APy_Board_properties[];
extern PyMethodDef	APy_Board_methods[];

extern PyObject*	APy_Board_dict;

extern PyTypeObject APy_BoardType;


/*******************************************************************************
 * Board State
 */
// Data Structure
typedef struct {
	PyObject_HEAD
	BoardState m_state;
} APy_State;


// Python Methods
void 		APy_State_dealloc	(PyObject * self);
PyObject* 	APy_State_new		(PyTypeObject * type, PyObject * args, PyObject * kwds);
int 		APy_State_init		(APy_State * self, PyObject * args, PyObject * kwds);
PyObject*	APy_State_str		(PyObject * self_arg);
PyObject*	APy_State_repr		(PyObject * self_arg);
PyObject*	APy_State_richcmp	(PyObject * self_arg, PyObject * other_arg, int op);


// Properties
PyObject*	APy_State_GET_fen			(PyObject * self_arg, void * closure);
PyObject*	APy_State_GET_white_to_move	(PyObject * self_arg, void * closure);
PyObject*	APy_State_GET_white_OO		(PyObject * self_arg, void * closure);
PyObject*	APy_State_GET_white_OOO		(PyObject * self_arg, void * closure);
PyObject*	APy_State_GET_black_OO		(PyObject * self_arg, void * closure);
PyObject*	APy_State_GET_black_OOO		(PyObject * self_arg, void * closure);
PyObject*	APy_State_GET_ep_target		(PyObject * self_arg, void * closure);
PyObject*	APy_State_GET_nply			(PyObject * self_arg, void * closure);


// Methods


// Python Type
extern PyGetSetDef	APy_State_properties[];

extern PyObject*	APy_State_dict;

extern PyTypeObject APy_StateType;


/*******************************************************************************
 * Move
 */
// Data Structure
typedef struct {
	PyObject_HEAD
	Move m_move;
} APy_Move;


// Python Methods
void 		APy_Move_dealloc	(APy_Move * self);
PyObject* 	APy_Move_new		(PyTypeObject * type, PyObject * args, PyObject * kwds);
int 		APy_Move_init		(APy_Move * self, PyObject * args, PyObject * kwds);
PyObject*	APy_Move_str		(PyObject * self_arg);
PyObject*	APy_Move_repr		(PyObject * self_arg);
PyObject*	APy_Move_richcmp	(PyObject * self_arg, PyObject * other_arg, int op);


// Properties


// Methods


// Python Type
extern PyObject*	APy_Move_dict;

extern PyTypeObject	APy_MoveType;

#endif