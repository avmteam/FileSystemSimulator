#include "stdafx.h"
#include "FileDescriptor.h"


FileDescriptor::FileDescriptor()
{
  clear();
}

FileDescriptor::FileDescriptor(bool i_is_free)
{
  clear();
  if (!i_is_free)
    file_size = 0;
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

  int last_index = getLastBlockIndex();
  data_blocks[last_index + 1] = i_index;
  return true;
}

void FileDescriptor::clear()
{
  file_size = -1;
  for (size_t i = 0; i < MAX_DATA_BLOCKS; i++)
    data_blocks[i] = -1;
}

bool FileDescriptor::isFree()
{
  return (file_size == -1);
}
