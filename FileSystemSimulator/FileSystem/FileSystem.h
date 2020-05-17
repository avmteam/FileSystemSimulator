#pragma once

#include "../IOSystem/IOSystem.h"

// TODO: add errors code
class FileSystem
{
public:

  // TODO: move impl to cpp file
  struct FileDescriptor {
    static const size_t MAX_DATA_BLOCKS = 3;

    FileDescriptor(bool i_is_free) :
      is_free(i_is_free)
    {
      std::memset(data_blocks, -1, MAX_DATA_BLOCKS);
    }

    bool isFull() {
      return data_blocks[MAX_DATA_BLOCKS - 1] != -1;
    }

    size_t getLastBlockIndex() {
      if (isFull())
        return MAX_DATA_BLOCKS - 1;

      size_t i = 0;
      while (data_blocks[i] != -1)
        i++;

      return i;
    }

    size_t getLastBlock() {
      return data_blocks[getLastBlockIndex()];
    }

    bool addBlock(size_t i_index) {
      if (isFull())
        return false;

      size_t last_index = getLastBlockIndex();
      data_blocks[last_index + 1] = i_index;
      return true;
    }

    bool is_free = true;
    size_t file_size = 0;
    int data_blocks[MAX_DATA_BLOCKS];
  };

  struct DirEntry {
    static const size_t MAX_FILE_NAME_LENGTH = 16;

    DirEntry(const std::string& i_file_name, size_t i_index) {
      std::memcpy(file_name, i_file_name.c_str(), MAX_FILE_NAME_LENGTH);
      file_descr_index = i_index;
    }

    char file_name[MAX_FILE_NAME_LENGTH];
    size_t file_descr_index;
  };

public:
  FileSystem(IOSystem* i_iosystem);
  ~FileSystem();

  bool create(const std::string& i_file_name);
  bool destroy(const std::string& i_file_name);
  int open(const std::string& i_file_name);
  bool close(int index);
  bool read(int index, char* mem_area, int count);
  bool write(int index, char* mem_area, int count);
  bool lseek(int index, int pos);

private:
  int findFreeFileDescriptor();
  bool recordFileToDir(const std::string & i_file_name, size_t i_fd_index);
  FileDescriptor getFileDescriptor(size_t i_index);
  bool writeFileDescriptorToIO(const FileDescriptor& i_fd, size_t i_index);
  bool allocateDataBlock(size_t i_index);
  int findFreeDataBlock();
  bool setBit(size_t i_index, bool i_is_free);

private:
  IOSystem* iosystem;

  static const size_t DATA_BLOCKS_NUMBER = Disk::NUMBER_OF_BLOCKS * Sector::BLOCK_SIZE / (Sector::BLOCK_SIZE + sizeof(FileDescriptor) + 1) - 1;
  static const size_t BITMAP_BLOCKS_NUMBER = DATA_BLOCKS_NUMBER / Sector::BLOCK_SIZE;
  static const size_t FIRST_DATA_BLOCK_INDEX = Disk::NUMBER_OF_BLOCKS - DATA_BLOCKS_NUMBER;
  static const size_t ENTRIES_IN_BLOCK = Sector::BLOCK_SIZE / sizeof(DirEntry);
};

