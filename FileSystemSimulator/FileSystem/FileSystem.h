#pragma once

#include "../IOSystem/IOSystem.h"
#include "OpenFileTable.h"
#include "FileDescriptor.h"
#include <vector>

// TODO: add errors code
class FileSystem
{
public:


  struct DirEntry {
    static const size_t MAX_FILE_NAME_LENGTH = 16;

    DirEntry(const std::string& i_file_name, size_t i_index) {
      std::memcpy(file_name, i_file_name.c_str(), MAX_FILE_NAME_LENGTH);
      file_descr_index = i_index;
    }

    char file_name[MAX_FILE_NAME_LENGTH];
    size_t file_descr_index;
  };

  struct FileInfo {

	  FileInfo(const std::string& i_file_name, size_t i_file_length) {
		  std::memcpy(file_name, i_file_name.c_str(), DirEntry::MAX_FILE_NAME_LENGTH);
		  file_length = i_file_length;
	  }

	  char file_name[DirEntry::MAX_FILE_NAME_LENGTH];
	  size_t file_length;
  };

public:
  FileSystem(IOSystem* i_iosystem);
  ~FileSystem();

  bool create(const std::string& i_file_name);
  bool destroy(const std::string& i_file_name);
  int open(const std::string& i_file_name);
  bool close(size_t index);
  bool read(size_t index, char* mem_area, size_t count);
  bool write(size_t index, char* mem_area, size_t count);
  bool lseek(size_t index, size_t pos);
  std::vector<FileInfo> directory();

private:
  int findFreeFileDescriptor();
  bool recordFileToDir(const std::string & i_file_name, size_t i_fd_index);
  FileDescriptor getFileDescriptor(size_t i_index);
  bool writeFileDescriptorToIO(const FileDescriptor& i_fd, size_t i_index);
  bool allocateDataBlock(size_t i_index);
  int findFreeDataBlock();
  bool setBit(size_t i_index, bool i_is_free);
  int findFileDescriptor(const std::string& i_file_name);

private:
  IOSystem* iosystem;
  OpenFileTable* oft;

  static const size_t DATA_BLOCKS_NUMBER = Disk::NUMBER_OF_BLOCKS * Sector::BLOCK_SIZE / (Sector::BLOCK_SIZE + sizeof(FileDescriptor) + 1);
  static const size_t BITMAP_BLOCKS_NUMBER = DATA_BLOCKS_NUMBER / Sector::BLOCK_SIZE + 1;
  static const size_t FD_BLOCKS_NUMBER = Disk::NUMBER_OF_BLOCKS - DATA_BLOCKS_NUMBER - BITMAP_BLOCKS_NUMBER;
  static const size_t FDS_IN_BLOCK = Sector::BLOCK_SIZE / sizeof(FileDescriptor);
  static const size_t FD_NUMBER = FD_BLOCKS_NUMBER / FDS_IN_BLOCK;

  static const size_t FIRST_DATA_BLOCK_INDEX = Disk::NUMBER_OF_BLOCKS - DATA_BLOCKS_NUMBER;
  static const size_t ENTRIES_IN_BLOCK = Sector::BLOCK_SIZE / sizeof(DirEntry);
};

