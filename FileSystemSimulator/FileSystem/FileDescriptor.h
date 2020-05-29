#pragma once
#include <cstring>

struct FileDescriptor {
	static const size_t MAX_DATA_BLOCKS = 3;

	FileDescriptor();
	FileDescriptor(bool i_is_free);
	bool isFull();
	int getLastBlockIndex();
	bool addBlock(size_t i_index);
	void clear();
	bool isFree();

	int file_size = -1; //-1 means its free
	int data_blocks[MAX_DATA_BLOCKS];
};

