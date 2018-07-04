//#include <seed.h>

#include <tablebase.h>
#include <seed.h>

#include <iostream>
#include <string.h>

/*
void Test_SequenceFromFile() {
	BoardState state;
	state.InitFromFEN("8/8/8/8/8/2k5/1R6/K7 w - - 0 1");
	
	TableBase KRk;
	//std::cout <<
	KRk.EvaluateSequenceFromFile(state, "KR-k_Optimized");
}

void Test_KRkGeneration() {
	//TableBase KRk = GenerateRookAndKing();
	//std::cout << KRk;
	TableBase Kk = GenerateKings();
	std::cout << Kk;
	//KRk.AddTableBase(Kk);
	//KRk.Expand();
}

void Test_Optimization() {
	TableBase KRk;
	KRk.Load("KR-k_Raw");
	KRk.Optimize();
	std::cout << KRk;
}
*/

int main(int argc, char ** argv) {
	TableBase tb;
	
	std::cout << sizeof(Move) << std::endl;
	
	Generate_KvK(&tb);
	
	std::cout << tb;
	
	return 0;
}
