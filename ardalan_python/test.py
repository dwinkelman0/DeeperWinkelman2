# Correct syntax for loading this library
from sys import version_info
if version_info.major == 2:
	from ardalan import *
elif version_info.major == 3:
	import ardalan

cases = [
	("8/8/7k/4K3/8/8/8/8 w - - 0 1",				False,	False,	False),
	("4k1R1/8/4K3/8/8/8/8/8 b - - 0 1",				True,	True,	False),
	("7k/6R1/6K1/8/8/8/8/8 b - - 0 1",				False,	False,	True),
	("7k/6R1/6K1/8/8/8/8/8 w - - 0 1",				False,	False,	False),
	("7k/6R1/6K1/8/7Q/8/8/8 b - - 0 1",				True,	True,	False),
	("7k/6R1/6K1/8/7Q/5b2/8/8 b - - 0 1",			True,	False,	False),
	("7k/6R1/6K1/4n3/7Q/5b2/8/8 w - - 0 1",			True,	False,	False),
	("6RK/1k3nBQ/4b3/8/8/7r/8/8 w - - 0 1",			True,	True,	False),
	("6RK/1k2n1BQ/4b3/8/8/7r/8/8 w - - 0 1",		False,	False,	False),
	("8/4b3/2k4q/6RB/4r1NK/6PP/8/8 w - - 2 2",		False,	False,	True),
	("8/5b2/2k4q/6RB/5rNK/6PP/8/8 w - - 0 1",		False,	False,	False),
	("8/8/2k2bnq/6RB/5rNK/6PP/8/8 w - - 0 1",		True,	True,	False),
	("8/8/2k2bnq/6RB/5rNK/6P1/8/8 w - - 0 1",		True,	False,	False)
]

for fen, check, checkmate, stalemate in cases:
	s = ardalan.State(fen=fen)
	b = ardalan.Board(state=s)
	
	if b.InCheck(b.current.white_to_move): print("In Check")
	else: print("Not In Check")
	if b.IsCheckmate(): print("Checkmate")
	else: print("Not Checkmate")
	if b.IsStalemate(): print("Stalemate")
	else: print("Not Stalemate")
	print(b.GetLegalMoves())
	print(b.current.fen)
	
	print("")