#include "stdafx.h"
#include "FileDescriptor.h"


FileDescriptor::FileDescriptor(bool i_is_free) :
  is_free(i_is_free)
{
  std::memset(data_blocks, -1, MAX_DATA_BLOCKS);
}

bool FileDescriptor::isFull(){
  return data_blocks[MAX_DATA_BLOCKS - 1] != -1;
}

int FileDescriptor::getLastBlockIndex() {
  if (isFull())
    return MAX_DATA_BLOCKS - 1;

  int i = 0;
  while (data_blocks[i] != -1)
    i++;

  return i - 1;
}

bool FileDescriptor::addBlock(size_t i_index) {
  if (isFull())
    return false;

  size_t last_index = getLastBlockIndex();
  data_blocks[last_index + 1] = i_index;
  return true;
}
