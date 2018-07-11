#include "tests.h"

#include <board.h>

#include <iostream>

struct TestPositionA {
	const char * fen;
	bool is_check;
	bool is_checkmate;
	bool is_stalemate;
};

const int N_TEST_POSITIONS_A = 13;
TestPositionA TEST_POSITIONS_A[N_TEST_POSITIONS_A] = {
	{"8/8/7k/4K3/8/8/8/8 w - - 0 1",				false,	false,	false},
	{"4k1R1/8/4K3/8/8/8/8/8 b - - 0 1",				true,	true,	false},
	{"7k/6R1/6K1/8/8/8/8/8 b - - 0 1",				false,	false,	true},
	{"7k/6R1/6K1/8/8/8/8/8 w - - 0 1",				false,	false,	false},
	{"7k/6R1/6K1/8/7Q/8/8/8 b - - 0 1",				true,	true,	false},
	{"7k/6R1/6K1/8/7Q/5b2/8/8 b - - 0 1",			true,	false,	false},
	{"7k/6R1/6K1/4n3/7Q/5b2/8/8 w - - 0 1",			true,	false,	false},
	{"6RK/1k3nBQ/4b3/8/8/7r/8/8 w - - 0 1",			true,	true,	false},
	{"6RK/1k2n1BQ/4b3/8/8/7r/8/8 w - - 0 1",		false,	false,	false},
	{"8/4b3/2k4q/6RB/4r1NK/6PP/8/8 w - - 2 2",		false,	false,	true},
	{"8/5b2/2k4q/6RB/5rNK/6PP/8/8 w - - 0 1",		false,	false,	false},
	{"8/8/2k2bnq/6RB/5rNK/6PP/8/8 w - - 0 1",		true,	true,	false},
	{"8/8/2k2bnq/6RB/5rNK/6P1/8/8 w - - 0 1",		true,	false,	false}
};

struct TestPositionB {
	const char * fen_init;
	const char * move;
	const char * fen_final;
};

const int N_TEST_POSITIONS_B = 4;
TestPositionB TEST_POSITIONS_B[N_TEST_POSITIONS_B] = {
	{
		"r1bqkb1r/1ppp1ppp/p1n2n2/4p3/B3P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1",
		"wO-O",
		"r1bqkb1r/1ppp1ppp/p1n2n2/4p3/B3P3/5N2/PPPP1PPP/RNBQ1RK1 b kq - 1 1"
	},
	{
		"rnbq1rk1/1pp1bppp/p2p1n2/3Pp3/B3P3/2N1BN2/PPP2PPP/R2Q1RK1 b - - 2 5",
		"c7-c5",
		"rnbq1rk1/1p2bppp/p2p1n2/2pPp3/B3P3/2N1BN2/PPP2PPP/R2Q1RK1 w - c6 0 6"
	},
	{
		"rnbq1rk1/1p2bppp/p2p1n2/2pPp3/B3P3/2N1BN2/PPP2PPP/R2Q1RK1 w - c6 0 6",
		"d5-c6e.p.",
		"rnbq1rk1/1p2bppp/p1Pp1n2/4p3/B3P3/2N1BN2/PPP2PPP/R2Q1RK1 b - - 0 6"
	},
	{
		"3k3r/3P2P1/5K2/8/8/8/8/8 w - - 0 1",
		"g7-h8=wQ",
		"3k3Q/3P4/5K2/8/8/8/8/8 b - - 0 1"
	}
};

void Test_BoardStateFEN() {
	const char * fen = "8/8/8/5nPP/5RNK/6BN/8/8 b KQk e6 56 1";
	BoardState state;
	state.InitFromFEN(fen);
	std::cout << state;
}

void Test_BoardCompositeInit() {
	BoardComposite bc;
	bc.Init(BoardState());
	std::cout << bc;
	
	std::cout << "BoardState Size: " << sizeof(BoardState) << std::endl;
	std::cout << "BoardComposite Size: " << sizeof(BoardComposite) << std::endl;
	std::cout << "MoveList Size: " << sizeof(MoveList) << std::endl;
}


void Test_MoveGeneration() {
	Board board;
	for (int a = 0; a < 10000; a++) {
		if (a % 1000 == 0) std::cout << a << std::endl;
		for (int i = 0; i < N_TEST_POSITIONS_B; i++) {
			BoardState state_init, state_final;
			state_init.InitFromFEN(TEST_POSITIONS_B[i].fen_init);
			state_final.InitFromFEN(TEST_POSITIONS_B[i].fen_final);
			board.SetCurrent(state_init);
			
			const MoveList * moves = board.GetMoves();
			//std::cout << board;
			
			bool status = board.Make(Move(TEST_POSITIONS_B[i].move));
			if (!status) {
				//std::cout << "Error making the move " << TEST_POSITIONS_B[i].move << std::endl;
				continue;
			}
			
			/*
			//std::cout << board;
			board.SetCurrent(state_final);
			//std::cout << board;
			
			bool equal = board.GetCurrent() == state_final;
			if (equal) std::cout << "Positions match" << std::endl;
			else {
				std::cout << "Positions DO NOT match" << std::endl;
				std::cout << "Position after making move:" << std::endl;
				std::cout << board.GetCurrent();
				std::cout << "Position desired:" << std::endl;
				std::cout << state_final;
			}
			//std::cout << std::endl << std::endl << std::endl;
			*/
			
			status = board.Unmake(1);
			//if (!status) {
			//	std::cout << "Error unmaking the move " << TEST_POSITIONS_B[i].move << std::endl;
			//}
		}
	}
}

void Test_MoveMaking() {
	Board board;
	bool status;
	
	BoardState state;
	//state.InitFromFEN()
}

void Test_CheckDetection() {
	Board board;
	for (int i = 0; i < N_TEST_POSITIONS_A; i++) {
		BoardState state;
		state.InitFromFEN(TEST_POSITIONS_A[i].fen);
		board.SetCurrent(state);
		
		bool is_check = board.InCheck(state.white_to_move);
		bool is_checkmate = board.IsCheckmate();
		bool is_stalemate = board.IsStalemate();
		
		std::cout << board;
		
		if (is_check != TEST_POSITIONS_A[i].is_check || 
			is_checkmate != TEST_POSITIONS_A[i].is_checkmate ||
			is_stalemate != TEST_POSITIONS_A[i].is_stalemate) {
			std::cout << "Discrepancy: " << TEST_POSITIONS_A[i].fen << std::endl;
			std::cout << state;
		}
		
		std::cout << (is_check ? "In Check" : "Not In Check") << std::endl;
		std::cout << (is_checkmate ? "In Checkmate" : "Not In Checkmate") << std::endl;
		std::cout << (is_stalemate ? "In Stalemate" : "Not In Stalemate") << std::endl;
		std::cout << "Original: " << TEST_POSITIONS_A[i].fen << std::endl;
		std::cout << "Output:   " << state.GetFEN() << std::endl << std::endl;
	}
}

void Test_UnmoveGeneration() {
	Board board;
	for (int i = 0; i < N_TEST_POSITIONS_A; i++) {
		BoardState state;
		state.InitFromFEN(TEST_POSITIONS_A[i].fen);
		board.SetCurrent(state);
		
		const MoveList * unmoves = board.GetUnmoves();
		std::cout << board << std::endl;
	}
}

void Test_PGN() {
	const char * game = "1. e4 d6 2. Bb5+ Nd7 3. d4 Nf6";
	/*
	    1. e4 e5 \
		2. Nf3 Nf6 \
		3. d4 exd4 \
		4. e5 Ne4 \
		5. Qxd4 d5 \
		6. exd6";
	*/
	/*
		1. e4 e5 2. Nf3 Nc6 3. Bb5 a6 4. Ba4 Nf6 5. O-O Be7 6. Re1 b5 7. Bb3 d6 8. c3 \
		O-O 9. h3 Nb8 10. d4 Nbd7 11. c4 c6 12. cxb5 axb5 13. Nc3 Bb7 14. Bg5 b4 15. \
		Nb1 h6 16. Bh4 c5 17. dxe5 Nxe4 18. Bxe7 Qxe7 19. exd6 Qf6 20. Nbd2 Nxd6 21. \
		Nc4 Nxc4 22. Bxc4 Nb6 23. Ne5 Rae8 24. Bxf7+ Rxf7 25. Nxf7 Rxe1+ 26. Qxe1 Kxf7 \
		27. Qe3 Qg5 28. Qxg5 hxg5 29. b3 Ke6 30. a3 Kd6 31. axb4 cxb4 32. Ra5 Nd5 33. \
		f3 Bc8 34. Kf2 Bf5 35. Ra7 g6 36. Ra6+ Kc5 37. Ke1 Nf4 38. g3 Nxh3 39. Kd2 Kb5 \
		40. Rd6 Kc5 41. Ra6 Nf2 42. g4 Bd3 43. Re6 1/2-1/2";
	*/
	Board board;
	board.SetCurrent(BoardState());
	board.MakePGNMoves(game);
}