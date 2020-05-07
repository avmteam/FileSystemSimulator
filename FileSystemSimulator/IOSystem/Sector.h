#pragma once
class Sector
{
public:
	static const int NUMBER_OF_BLOCKS = 1;
	static const int BLOCK_SIZE = 64;

	char **blocks;

	Sector() {
		blocks = new char*[NUMBER_OF_BLOCKS];
		for (int i = 0; i < NUMBER_OF_BLOCKS; i++) {
			blocks[i] = new char[BLOCK_SIZE];
		}
	};

	~Sector() {
		for (int i = 0; i < NUMBER_OF_BLOCKS; i++)
			delete blocks[i];
		delete[] blocks;
	};
};

