//#include <seed.h>

#include <tablebase.h>
#include <seed.h>

#include <iostream>
#include <string.h>

int main(int argc, char ** argv) {
	TableBase tb;
	
	std::cout << sizeof(Move) << std::endl;
	
	Generate_KvK(&tb);
	Generate_KRvK(&tb);
	std::cout << tb << std::endl;
	
	tb.Expand();
	
	std::cout << tb << std::endl;
	
	Patch_KRvK(&tb);
	
	std::cout << tb << std::endl;
	
	tb.Optimize();
	
	std::cout << tb << std::endl;
	
	return 0;
}
