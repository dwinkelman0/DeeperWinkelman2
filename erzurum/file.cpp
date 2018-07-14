#include "tablebase.h"

const Hash_t FILE_BITMASK = 0x0000000000003fff;
const Hash_t N_FILES      = 0x0000000000004000;

struct PositionData {
	
};

bool TableBase::ImportFromDirectory(const char * dir) {
	
	char buffer = new char[strlen(dir) + 32];
	for (int i = 0; i <= N_FILES; i++) {
		sprintf(buffer, "%s/%x.erz", dir, i);
		positions.insert(ImportFromFile(buffer));
	}
	delete[] buffer;
	return true;
}

bool TableBase::SaveToDirectory(const char * dir) {
	
	std::map<Hash_t, 
	
	char buffer = new char[strlen(dir) + 32];
	for (int i = 0; i < N_FILES; i++) {
		sprintf(buffer, "%s/%x.erz", dir, i);
		if (!SaveToFile())
	}
}

bool TableBase::MergeToDirectory(const char * dir) {
	
}

std::map<BoardState, Node *> TableBase::ImportFromFile(const char * dir) {
	
}

bool TableBase::SaveToFile(const char * dir, std::map<BoardState, Node *> positions) {
	
}