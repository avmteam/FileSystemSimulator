#include "stdafx.h"
#include "FileSystem.h"


FileSystem::FileSystem(IOSystem* i_iosystem) :
  iosystem(i_iosystem)
{
  // init bitmap with true (is_free)
  char bitmap[Sector::BLOCK_SIZE];
  std::memset(bitmap, true, Sector::BLOCK_SIZE);
  for (size_t i = 0; i < BITMAP_BLOCKS_NUMBER; i++)
    iosystem->write_block(i, bitmap);

  // init file descriptors with is_free = true
  size_t fd_in_block = Sector::BLOCK_SIZE / sizeof(FileDescriptor);
  char block[Sector::BLOCK_SIZE];
  for (size_t i = 0; i < fd_in_block; i++) {
    FileDescriptor* fd = (FileDescriptor*)block + i;
    *fd = FileDescriptor(true);
  }
  for (size_t i = BITMAP_BLOCKS_NUMBER; i < FIRST_DATA_BLOCK_INDEX; i++)
    iosystem->write_block(i, block);

  // dir file descriptor (index 0) is not free
  writeFileDescriptorToIO(FileDescriptor(false), 0);
}


FileSystem::~FileSystem()
{
}

bool FileSystem::create(const std::string & i_file_name)
{
  int fd_index = findFreeFileDescriptor();
  if (fd_index == -1)
    return false;

  bool success = recordFileToDir(i_file_name, fd_index);
  if (!success)
    return false;

  FileDescriptor fd(false);
  writeFileDescriptorToIO(fd, fd_index);

  return true;
}

int FileSystem::findFreeFileDescriptor()
{
  size_t fd_in_block = Sector::BLOCK_SIZE / sizeof(FileDescriptor);
  size_t first_fd_index = BITMAP_BLOCKS_NUMBER;

  for (size_t i = first_fd_index; i < FIRST_DATA_BLOCK_INDEX; i++) {
    char block[Sector::BLOCK_SIZE];
    iosystem->read_block(i, block);
    for (size_t j = 0; j < fd_in_block; j++) {
      FileDescriptor* fd = (FileDescriptor*)block + j;
      if (fd->is_free)
        return (i - first_fd_index)*fd_in_block + j;
    }
  }

  return -1;
}

bool FileSystem::recordFileToDir(const std::string & i_file_name, size_t i_fd_index)
{
  FileDescriptor dir_fd = getFileDescriptor(0);
  size_t entries_number = dir_fd.file_size / sizeof(DirEntry);
  size_t entries_in_last_block = entries_number % ENTRIES_IN_BLOCK;

  // search for existing file with name i_file_name
  int first_free_index = -1;
  for (size_t i = 0; i <= dir_fd.getLastBlockIndex(); i++) {
    char block[Sector::BLOCK_SIZE];
    iosystem->read_block(dir_fd.data_blocks[i], block);

    for (size_t j = 0; j < ENTRIES_IN_BLOCK; j++) {
      DirEntry* entry = (DirEntry*)block + j;
      if (strcmp(entry->file_name, i_file_name.c_str()) == 0)
        return false;

      // check if this entry is free
      if (strcmp(entry->file_name, "") == 0 && first_free_index == -1)
        first_free_index = i * ENTRIES_IN_BLOCK + j;

      if (i*ENTRIES_IN_BLOCK + j == entries_number)
        break;
    }
  }
  if (first_free_index == -1) {
    first_free_index = entries_number;

    dir_fd.file_size += sizeof(DirEntry);
    writeFileDescriptorToIO(dir_fd, 0);
  }

  // add one more data block for dir file if needed
  if (entries_in_last_block == 0 && first_free_index != -1) {
    bool success = allocateDataBlock(0);
    if (!success)
      return false;
    // update dir_fd after allocation
    dir_fd = getFileDescriptor(0);
  }

  size_t block_number = dir_fd.data_blocks[first_free_index / ENTRIES_IN_BLOCK];
  size_t block_index = first_free_index % ENTRIES_IN_BLOCK;

  char block[Sector::BLOCK_SIZE];
  iosystem->read_block(block_number, block);
  DirEntry* entry = (DirEntry*)block + block_index;

  *entry = { i_file_name.c_str(), i_fd_index };
  iosystem->write_block(block_number, block);

  return true;
}

FileSystem::FileDescriptor FileSystem::getFileDescriptor(size_t i_index)
{
  char block[Sector::BLOCK_SIZE];

  size_t fd_in_block = Sector::BLOCK_SIZE / sizeof(FileDescriptor);
  size_t first_fd_index = BITMAP_BLOCKS_NUMBER;

  size_t block_number = i_index / fd_in_block;
  size_t fd_number = i_index % fd_in_block;

  iosystem->read_block(first_fd_index + block_number, block);
  FileDescriptor* fd = (FileDescriptor*)block + fd_number;

  return *fd;
}

bool FileSystem::writeFileDescriptorToIO(const FileDescriptor& i_fd, size_t i_index)
{
  if (i_index >= Disk::NUMBER_OF_BLOCKS)
    return false;

  char block[Sector::BLOCK_SIZE];

  size_t fd_in_block = Sector::BLOCK_SIZE / sizeof(FileDescriptor);
  size_t first_fd_index = BITMAP_BLOCKS_NUMBER;

  size_t block_number = i_index / fd_in_block;
  size_t fd_number = i_index % fd_in_block;

  iosystem->read_block(first_fd_index + block_number, block);
  FileDescriptor* fd = (FileDescriptor*)block + fd_number;

  *fd = i_fd;
  iosystem->write_block(i_index, block);

  return true;
}

bool FileSystem::allocateDataBlock(size_t i_index)
{
  if (i_index >= Disk::NUMBER_OF_BLOCKS)
    return false;

  int db_index = findFreeDataBlock();
  if (db_index == -1)
    return false;

  FileDescriptor fd = getFileDescriptor(i_index);
  bool success = fd.addBlock(db_index);
  if (!success)
    return false;

  writeFileDescriptorToIO(fd, i_index);
  setBit(db_index, false);

  return true;
}

int FileSystem::findFreeDataBlock()
{
  for (size_t i = 0; i < BITMAP_BLOCKS_NUMBER; i++) {
    char block[Sector::BLOCK_SIZE];
    iosystem->read_block(i, block);

    for (size_t j = 0; j < Sector::BLOCK_SIZE; j++) {
      bool* bit = (bool*)block + j;
      size_t index = i * Sector::BLOCK_SIZE + j;

      if (*bit)
        return (index >= Disk::NUMBER_OF_BLOCKS ? -1 : index);
    }
  }

  return -1;
}

bool FileSystem::setBit(size_t i_index, bool i_is_free)
{
  if (i_index >= Disk::NUMBER_OF_BLOCKS)
    return false;

  size_t block_number = i_index / Sector::BLOCK_SIZE;
  size_t block_index = i_index % Sector::BLOCK_SIZE;

  char block[Sector::BLOCK_SIZE];
  iosystem->read_block(block_number, block);

  bool* bit = (bool*)block + block_index;
  *bit = i_is_free;

  iosystem->write_block(i_index, block);
  return true;
}
