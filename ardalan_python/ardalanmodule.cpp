#include "apy_types.h"

#include <Python.h>

#include <iostream>

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif

static PyMethodDef ardalan_methods[] = {
	{NULL, NULL, 0, NULL}
};

const char * ARDALAN_DOC = "Python wrapper for the DeeperWinkelman chess engine board representation.";

#ifdef PYTHON3
static struct PyModuleDef ardalan_definition = {
	PyModuleDef_HEAD_INIT,
	"ardalan",
	ARDALAN_DOC,
	-1,
	ardalan_methods
};
#endif

PyMODINIT_FUNC
#ifdef PYTHON2
initardalan(void)
#else
#ifdef PYTHON3
PyInit_ardalan(void)
#endif
#endif
{
	
	#ifdef PYTHON2
	std::cout << "Ardalan: Python 2" << std::endl;
	#else
	#ifdef PYTHON3
	std::cout << "Ardalan: Python 3" << std::endl;
	#endif
	#endif
	
	PyObject * m;
	
	#ifdef PYTHON2
	// Prepare Classes
	if (PyType_Ready(&APy_BoardType) < 0) return;
	if (PyType_Ready(&APy_StateType) < 0) return;
	if (PyType_Ready(&APy_MoveType) < 0) return;
	
	// Module Setup
	if (!(m = Py_InitModule3("ardalan", ardalan_methods, ARDALAN_DOC))) return;
	#else
	
	#ifdef PYTHON3
	// Prepare Classes
	if (PyType_Ready(&APy_BoardType) < 0) return NULL;
	if (PyType_Ready(&APy_StateType) < 0) return NULL;
	if (PyType_Ready(&APy_MoveType) < 0) return NULL;
	
	// Module Setup
	if (!(m = PyModule_Create(&ardalan_definition))) return NULL;
	#endif
	#endif
	
	// Move Constants
	PyDict_SetItem(APy_Move_dict,	APy_PyString_FromString("MOVE_NORMAL"),			PyLong_FromLong(Move::NORMAL_MOVE));
	PyDict_SetItem(APy_Move_dict,	APy_PyString_FromString("MOVE_WHITE_OO"),		PyLong_FromLong(Move::WHITE_OO));
	PyDict_SetItem(APy_Move_dict,	APy_PyString_FromString("MOVE_WHITE_OOO"),		PyLong_FromLong(Move::WHITE_OOO));
	PyDict_SetItem(APy_Move_dict,	APy_PyString_FromString("MOVE_BLACK_OO"),		PyLong_FromLong(Move::BLACK_OO));
	PyDict_SetItem(APy_Move_dict,	APy_PyString_FromString("MOVE_BLACK_OOO"),		PyLong_FromLong(Move::BLACK_OOO));
	PyDict_SetItem(APy_Move_dict,	APy_PyString_FromString("MOVE_EN_PASSANT"),		PyLong_FromLong(Move::EN_PASSANT));
	PyDict_SetItem(APy_Move_dict,	APy_PyString_FromString("MOVE_NULL"),			PyLong_FromLong(Move::NULL_MOVE));
	
	// Piece Constants
	PyModule_AddIntConstant(m, "EMPTY",			EMPTY);
	PyModule_AddIntConstant(m, "WHITE_PAWN",	WHITE_PAWN);
	PyModule_AddIntConstant(m, "WHITE_KNIGHT",	WHITE_KNIGHT);
	PyModule_AddIntConstant(m, "WHITE_BISHOP",	WHITE_BISHOP);
	PyModule_AddIntConstant(m, "WHITE_ROOK",	WHITE_ROOK);
	PyModule_AddIntConstant(m, "WHITE_QUEEN",	WHITE_QUEEN);
	PyModule_AddIntConstant(m, "WHITE_KING",	WHITE_KING);
	PyModule_AddIntConstant(m, "BLACK_PAWN",	BLACK_PAWN);
	PyModule_AddIntConstant(m, "BLACK_KNIGHT",	BLACK_KNIGHT);
	PyModule_AddIntConstant(m, "BLACK_BISHOP",	BLACK_BISHOP);
	PyModule_AddIntConstant(m, "BLACK_ROOK",	BLACK_ROOK);
	PyModule_AddIntConstant(m, "BLACK_QUEEN",	BLACK_QUEEN);
	PyModule_AddIntConstant(m, "BLACK_KING",	BLACK_KING);
	
	// Color Constants
	PyModule_AddIntConstant(m, "WHITE", 1);
	PyModule_AddIntConstant(m, "BLACK", 0);
	
	// Classes
	Py_INCREF(&APy_BoardType);
	PyModule_AddObject(m, "Board", (PyObject *)&APy_BoardType);
	
	Py_INCREF(&APy_StateType);
	PyModule_AddObject(m, "State", (PyObject *)&APy_StateType);
	
	Py_INCREF(&APy_MoveType);
	PyModule_AddObject(m, "Move", (PyObject *)&APy_MoveType);
	
	#ifdef PYTHON3
	Py_Initialize();
	return m;
	#endif
}
