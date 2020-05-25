#include "stdafx.h"
#include "OpenFileTable.h"


OpenFileTable::OpenFileTable()
{
}


OpenFileTable::~OpenFileTable()
{
}

int OpenFileTable::addNewEntry(size_t i_fd_index)
{
  int first_free_entry = -1;
  for (size_t i = 0; i < MAX_TABLE_LENGTH; i++) {
    if (entries->fd_index == i_fd_index)
      return -1;

	

    if (entries[i].fd_index == -1 && first_free_entry == -1)
      first_free_entry = i;
  }
  if (first_free_entry == -1)
    return false;

  entries[first_free_entry].fd_index = i_fd_index;
  return first_free_entry;
}

OpenFileTable::OFTEntry* OpenFileTable::getEntry(size_t i_index)
{
  if (!isExist(i_index))
    return nullptr;

  return &entries[i_index];
}

bool OpenFileTable::freeEntry(size_t i_index)
{
  if (!isExist(i_index))
    return false;

  OFTEntry* entry = getEntry(i_index);
  entry->fd_index = -1;
  entry->cur_pos = 0;

  return true;
}

bool OpenFileTable::isExist(size_t i_index)
{
  if (i_index >= MAX_TABLE_LENGTH)
    return false;

  OFTEntry entry = entries[i_index];
  return (entry.fd_index != -1);
}

bool OpenFileTable::checkOpenFD(size_t fd_index)
{
	for (int i = 0; i < MAX_TABLE_LENGTH; i++) {
		if (entries[i].fd_index == fd_index) return true;
	}
	return false;
}
