#pragma once

#include "../IOSystem/IOSystem.h"

class OpenFileTable
{
public:
  struct OFTEntry {
    char buffer[Sector::BLOCK_SIZE];
    size_t cur_pos = 0;
    int fd_index = -1; //means its free
  };

public:
  const static size_t MAX_TABLE_LENGTH = 32;

public:
  OpenFileTable();
  ~OpenFileTable();

  int addNewEntry(size_t i_fd_index);
  OFTEntry* getEntry(size_t i_index);
  bool freeEntry(size_t i_index);
  bool isExist(size_t i_index);

private:
  OFTEntry entries[MAX_TABLE_LENGTH];
};

