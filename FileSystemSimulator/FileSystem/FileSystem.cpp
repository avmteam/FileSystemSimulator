#include "stdafx.h"
#include "FileSystem.h"
#include "../IOSystem/IOSystem.h"

namespace {
  bool namesAreEqual(FileSystem::DirEntry* i_entry, const std::string& i_name) {
    char file_name[FileSystem::DirEntry::MAX_FILE_NAME_LENGTH + 1];
    std::memcpy(file_name, i_entry->file_name, FileSystem::DirEntry::MAX_FILE_NAME_LENGTH);
    file_name[FileSystem::DirEntry::MAX_FILE_NAME_LENGTH] = '\0';

    return (std::strcmp(file_name, i_name.c_str()) == 0);
  }

  char* fixName(char* i_name) {
    char* file_name = new char[FileSystem::DirEntry::MAX_FILE_NAME_LENGTH + 1];
    std::memcpy(file_name, i_name, FileSystem::DirEntry::MAX_FILE_NAME_LENGTH);
    file_name[FileSystem::DirEntry::MAX_FILE_NAME_LENGTH] = '\0';

    return file_name;
  }
}

FileSystem::FileSystem() :
  iosystem(new IOSystem()),
  oft(new OpenFileTable())
{
  init();
}

FileSystem::FileSystem(IOSystem* i_iosystem) :
	iosystem(i_iosystem),
	oft(new OpenFileTable())
{
  init();
}


FileSystem::~FileSystem()
{
  delete oft;
}

bool FileSystem::create(const std::string & i_file_name)
{
  if (i_file_name == "" || i_file_name.size() > DirEntry::MAX_FILE_NAME_LENGTH)
    return false;

  int fd_index = findFreeFileDescriptor();
  if (fd_index == -1)
    return false;

  bool success = recordFileToDir(i_file_name, fd_index);
  if (!success)
    return false;

  writeFileDescriptorToIO(FileDescriptor(false), fd_index);
  return true;
}

bool FileSystem::destroy(const std::string & i_file_name)
{
	if (i_file_name == "")
		return false;
	
	FileDescriptor dir_fd = getFileDescriptor(0);
	size_t entries_number = dir_fd.file_size / sizeof(DirEntry);

	for (int i = 0; i <= dir_fd.getLastBlockIndex(); i++) {
		char block[Sector::BLOCK_SIZE];
		iosystem->read_block(dir_fd.data_blocks[i], block);

		for (size_t j = 0; j < ENTRIES_IN_BLOCK; j++) {
			DirEntry* entry = (DirEntry*)block + j;
      if (namesAreEqual(entry, i_file_name)){

				if (oft->checkOpenFD(entry->file_descr_index)) return false;
				
				FileDescriptor fd = getFileDescriptor(entry->file_descr_index);
        for (int i = 0; i <= fd.getLastBlockIndex(); i++) {
          if (!setBit(fd.data_blocks[i], true)) return false;
        }
				fd.clear();
				writeFileDescriptorToIO(fd, entry->file_descr_index);

				*entry = DirEntry("", -1);
				iosystem->write_block(dir_fd.data_blocks[i], block);
				return true;
			}

			if (i*ENTRIES_IN_BLOCK + j == entries_number - 1)
				break;
		}
	}

	return false;

}

int FileSystem::open(const std::string & i_file_name)
{
  size_t fd_index = findFileDescriptor(i_file_name);
  if (fd_index == -1)
    return -1;

  size_t oft_index = oft->addNewEntry(fd_index);
  if (oft_index == -1)
    return -1;

  FileDescriptor fd = getFileDescriptor(fd_index);
  if (fd.file_size == 0)
    return oft_index;

  OpenFileTable::OFTEntry* entry = oft->getEntry(oft_index);
  iosystem->read_block(fd.data_blocks[0], entry->buffer);

  return oft_index;
}

bool FileSystem::close(size_t i_index)
{
  OpenFileTable::OFTEntry* entry = oft->getEntry(i_index);
  if (!entry)
    return false;
  FileDescriptor fd = getFileDescriptor(entry->fd_index);

  size_t block_index = entry->cur_pos / Sector::BLOCK_SIZE;
  if (block_index < FileDescriptor::MAX_DATA_BLOCKS) {
    int block_number = fd.data_blocks[block_index];
    iosystem->write_block(block_number, entry->buffer);
  }

  oft->freeEntry(i_index);
  return true;
}


int FileSystem::write(size_t i_index, char* i_mem_area, size_t i_count)
{
  if (i_count == 0)
    return i_count;

  OpenFileTable::OFTEntry* entry = oft->getEntry(i_index);
  if (!entry)
    return -1;

  FileDescriptor fd = getFileDescriptor(entry->fd_index);
  if (fd.file_size == 0) {
    bool success = allocateDataBlock(entry->fd_index);
    if (!success)
      return -2; // failed to allocate block code

    // update fd after allocating new block
    fd = getFileDescriptor(entry->fd_index);
  }

  size_t have_written = 0;
  while (true) {

    size_t buffer_pos = entry->cur_pos % Sector::BLOCK_SIZE;
    size_t buffer_space = Sector::BLOCK_SIZE - buffer_pos;

    if (i_count <= buffer_space) {
      std::memcpy(entry->buffer + buffer_pos, i_mem_area + have_written, i_count);
      entry->cur_pos += i_count;
      have_written += i_count;

	    if (entry->cur_pos > fd.file_size) {
		    fd.file_size = entry->cur_pos;
		    writeFileDescriptorToIO(fd, entry->fd_index);
	    }

      return have_written;
    }

    std::memcpy(entry->buffer + buffer_pos, i_mem_area + have_written, buffer_space);

    size_t block_index = entry->cur_pos / Sector::BLOCK_SIZE;
	  size_t block_number = fd.data_blocks[block_index];

	  iosystem->write_block(block_number, entry->buffer);
    entry->cur_pos += buffer_space;
    have_written += buffer_space;
    i_count -= buffer_space;

    if (entry->cur_pos > fd.file_size) {

      fd.file_size = entry->cur_pos;
      writeFileDescriptorToIO(fd, entry->fd_index);

      bool success = allocateDataBlock(entry->fd_index);
      if (!success)
        return -2;

      fd = getFileDescriptor(entry->fd_index);
    }

    iosystem->read_block(fd.data_blocks[block_index + 1], entry->buffer);
  }
}

int FileSystem::read(size_t i_index, char* i_mem_area, size_t i_count)
{
	if (i_count == 0)
		return i_count;

	OpenFileTable::OFTEntry* entry = oft->getEntry(i_index);
	if (!entry)
		return -1;

	FileDescriptor fd = getFileDescriptor(entry->fd_index);
	if (fd.file_size == 0) 
		return -1;

	int local_count;
	if (i_count > fd.file_size - entry->cur_pos) local_count = fd.file_size - entry->cur_pos;
	else local_count = i_count;

	size_t have_written = 0;

	while (true) {

		size_t buffer_pos = entry->cur_pos % Sector::BLOCK_SIZE;
		size_t buffer_space = Sector::BLOCK_SIZE - buffer_pos;

		if (local_count <= buffer_space) {
			std::memcpy(i_mem_area + have_written, entry->buffer + buffer_pos, local_count);
			entry->cur_pos += local_count;
			have_written += local_count;

			return have_written;
		}

		std::memcpy(i_mem_area + have_written, entry->buffer + buffer_pos, buffer_space);
		size_t block_number = fd.data_blocks[entry->cur_pos / Sector::BLOCK_SIZE];

		iosystem->write_block(block_number, entry->buffer);
		iosystem->read_block(block_number + 1, entry->buffer);
		entry->cur_pos += buffer_space;
		have_written += buffer_space;
		local_count -= buffer_space;
	}
}


int FileSystem::lseek(size_t i_index, size_t i_pos)
{
	OpenFileTable::OFTEntry* entry = oft->getEntry(i_index);

	if (!entry)
		return -1;

	FileDescriptor fd = getFileDescriptor(entry->fd_index);

	if (i_pos >= fd.file_size)
		return -1;

	size_t cur_pos = entry->cur_pos;

	size_t cur_block = cur_pos / Sector::BLOCK_SIZE;
	size_t new_block = i_pos / Sector::BLOCK_SIZE;

	// check if new position is within the current data block 
	if (cur_block != new_block)
	{
    if (cur_block != FileDescriptor::MAX_DATA_BLOCKS)
		  iosystem->write_block(fd.data_blocks[cur_block], entry->buffer);
		iosystem->read_block(fd.data_blocks[new_block], entry->buffer);
	}

	entry->cur_pos = i_pos;

	return entry->cur_pos;
}

std::vector<FileSystem::FileInfo> FileSystem::directory()
{
	FileDescriptor dir_fd = getFileDescriptor(0);
	size_t entries_number = dir_fd.file_size / sizeof(DirEntry);

	vector<FileInfo> files;

	for (int i = 0; i <= dir_fd.getLastBlockIndex(); i++) {

		char block[Sector::BLOCK_SIZE];
		iosystem->read_block(dir_fd.data_blocks[i], block);

		for (size_t j = 0; j < ENTRIES_IN_BLOCK; j++) {

      if (i * ENTRIES_IN_BLOCK + j == entries_number)
        break;

			DirEntry* entry = (DirEntry*)block + j;		
      if (namesAreEqual(entry, ""))
			    continue;

      char* name = fixName(entry->file_name);

      // findFileDescriptor returns index of found file descriptor
			FileDescriptor fd = getFileDescriptor(findFileDescriptor(name));

			FileInfo fi(name, fd.file_size);
			files.push_back(fi);

      delete[] name;
		}
	}

	return files;
}

void FileSystem::init()
{
  // init bitmap with true (is_free)
  char bitmap[Sector::BLOCK_SIZE];
  std::memset(bitmap, true, Sector::BLOCK_SIZE);
  for (size_t i = 0; i < BITMAP_BLOCKS_NUMBER; i++)
    iosystem->write_block(i, bitmap);

  // init file descriptors with is_free = true
  FileDescriptor block[FDS_IN_BLOCK];
  for (size_t i = BITMAP_BLOCKS_NUMBER; i < FIRST_DATA_BLOCK_INDEX; i++)
    iosystem->write_block(i, (char*)block);

  // dir file descriptor (index 0) is not free
  writeFileDescriptorToIO(FileDescriptor(false), 0);
}

int FileSystem::findFreeFileDescriptor()
{
  size_t first_fd_index = BITMAP_BLOCKS_NUMBER;

  for (size_t i = first_fd_index; i < FIRST_DATA_BLOCK_INDEX; i++) {
    char block[Sector::BLOCK_SIZE];
    iosystem->read_block(i, block);
    for (size_t j = 0; j < FDS_IN_BLOCK; j++) {
      FileDescriptor* fd = (FileDescriptor*)block + j;
      if (fd->isFree())
        return (i - first_fd_index)*FDS_IN_BLOCK + j;
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
  for (int i = 0; i <= dir_fd.getLastBlockIndex(); i++) {
    char block[Sector::BLOCK_SIZE];
    iosystem->read_block(dir_fd.data_blocks[i], block);

    for (size_t j = 0; j < ENTRIES_IN_BLOCK; j++) {
      DirEntry* entry = (DirEntry*)block + j;
      if (namesAreEqual(entry, i_file_name))
        return false;

      // check if this entry is free
      if (namesAreEqual(entry, "") && first_free_index == -1)
        first_free_index = i * ENTRIES_IN_BLOCK + j;

      if (i*ENTRIES_IN_BLOCK + j == entries_number - 1)
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

FileDescriptor FileSystem::getFileDescriptor(size_t i_index)
{
  char block[Sector::BLOCK_SIZE];

  size_t first_fd_index = BITMAP_BLOCKS_NUMBER;

  size_t block_number = i_index / FDS_IN_BLOCK;
  size_t fd_number = i_index % FDS_IN_BLOCK;

  iosystem->read_block(first_fd_index + block_number, block);
  FileDescriptor* fd = (FileDescriptor*)block + fd_number;

  return *fd;
}

bool FileSystem::writeFileDescriptorToIO(const FileDescriptor& i_fd, size_t i_index)
{
  if (i_index >= FD_NUMBER)
    return false;

  char block[Sector::BLOCK_SIZE];
  size_t first_fd_index = BITMAP_BLOCKS_NUMBER;

  size_t block_number = i_index / FDS_IN_BLOCK;
  size_t fd_number = i_index % FDS_IN_BLOCK;

  iosystem->read_block(first_fd_index + block_number, block);
  FileDescriptor* fd = (FileDescriptor*)block + fd_number;

  *fd = i_fd;
  iosystem->write_block(first_fd_index + block_number, block);

  return true;
}

bool FileSystem::allocateDataBlock(size_t i_index)
{
  if (i_index >= FD_NUMBER)
    return false;

  // db_index is global block index
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
      size_t index = i * Sector::BLOCK_SIZE + j + FIRST_DATA_BLOCK_INDEX;

      if (*bit)
        return (index >= Disk::NUMBER_OF_BLOCKS ? -1 : index);
    }
  }

  return -1;
}

bool FileSystem::setBit(size_t i_index, bool i_is_free)
{
  int index = i_index - FIRST_DATA_BLOCK_INDEX;

  if (index < 0 || index >= DATA_BLOCKS_NUMBER)
    return false;

  size_t block_number = index / Sector::BLOCK_SIZE;
  size_t block_index = index % Sector::BLOCK_SIZE;

  char block[Sector::BLOCK_SIZE];
  iosystem->read_block(block_number, block);

  bool* bit = (bool*)block + block_index;
  *bit = i_is_free;

  iosystem->write_block(block_number, block);
  return true;
}

int FileSystem::findFileDescriptor(const std::string & i_file_name)
{
  FileDescriptor dir_fd = getFileDescriptor(0);
  size_t entries_number = dir_fd.file_size / sizeof(DirEntry);

  for (int i = 0; i <= dir_fd.getLastBlockIndex(); i++) {
    char block[Sector::BLOCK_SIZE];
    iosystem->read_block(dir_fd.data_blocks[i], block);

    for (size_t j = 0; j < ENTRIES_IN_BLOCK; j++) {
      DirEntry* entry = (DirEntry*)block + j;

      if (namesAreEqual(entry, i_file_name))
        return entry->file_descr_index;

      if (i*ENTRIES_IN_BLOCK + j == entries_number - 1)
        break;
    }
  }

  return -1;
}
