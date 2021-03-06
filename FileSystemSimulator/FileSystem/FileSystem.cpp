#include "stdafx.h"
#include "FileSystem.h"
#include "../IOSystem/IOSystem.h"
#include "ErrorCodes.h"

#include <bitset>

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

int FileSystem::create(const std::string & i_file_name)
{
	if (i_file_name == "" || i_file_name.size() > DirEntry::MAX_FILE_NAME_LENGTH)
		return invalid_filename;

	int fd_index = findFreeFileDescriptor();
	if (fd_index == -1)
		return max_file_descriptiors_number_exceeded;

	int status = recordFileToDir(i_file_name, fd_index);
	if (status != success_code)
		return status;

	writeFileDescriptorToIO(FileDescriptor(false), fd_index);
	return FileSystem::success_code;
}

int FileSystem::destroy(const std::string & i_file_name)
{
	if (i_file_name == "")
		return invalid_filename;

	FileDescriptor dir_fd = getFileDescriptor(0);
	size_t entries_number = dir_fd.file_size / sizeof(DirEntry);

	for (int i = 0; i <= dir_fd.getLastBlockIndex(); i++) {
		char block[Sector::BLOCK_SIZE];
		iosystem->read_block(dir_fd.data_blocks[i], block);

		for (size_t j = 0; j < ENTRIES_IN_BLOCK; j++) {
			DirEntry* entry = (DirEntry*)block + j;
			if (namesAreEqual(entry, i_file_name)) {

				if (oft->checkOpenFD(entry->file_descr_index)) return file_is_opened;

				FileDescriptor fd = getFileDescriptor(entry->file_descr_index);
				for (int i = 0; i <= fd.getLastBlockIndex(); i++) {
					if (!setBit(fd.data_blocks[i], true)) return false;	// TODO: return correct code
				}
				fd.clear();
				writeFileDescriptorToIO(fd, entry->file_descr_index);

				*entry = DirEntry("", -1);
				iosystem->write_block(dir_fd.data_blocks[i], block);
				return FileSystem::success_code;
			}

			if (i*ENTRIES_IN_BLOCK + j == entries_number - 1)
				break;
		}
	}

	return file_not_found;
}

int FileSystem::open(const std::string & i_file_name)
{
	int fd_index = findFileDescriptor(i_file_name);
	if (fd_index == -1)
		return file_not_found;

	int oft_index = oft->addNewEntry(fd_index);
	if (oft_index == max_opened_files_number_exceeded)
		return max_opened_files_number_exceeded;

	if (oft_index == file_is_opened)
		return file_is_opened;

	FileDescriptor fd = getFileDescriptor(fd_index);
	if (fd.file_size == 0)
		return oft_index;

	OpenFileTable::OFTEntry* entry = oft->getEntry(oft_index);
	readDataToBuffer(fd.data_blocks[0], entry);

	return oft_index;
}

int FileSystem::close(size_t i_index)
{
	OpenFileTable::OFTEntry* entry = oft->getEntry(i_index);
	if (!entry)
		return file_not_opened;
	FileDescriptor fd = getFileDescriptor(entry->fd_index);

	size_t block_index = entry->cur_pos / Sector::BLOCK_SIZE;
	if (block_index < FileDescriptor::MAX_DATA_BLOCKS) {
		int block_number = fd.data_blocks[block_index];
		if (block_number != -1)
			writeDataFromBuffer(block_number, entry);
	}

	oft->freeEntry(i_index);
	return FileSystem::success_code;
}

// first value: status code, second value: bytes read
std::pair<int, int> FileSystem::write(size_t i_index, char* i_mem_area, size_t i_count)
{
	if (i_count == 0)
		return std::make_pair(success_code, i_count);

	OpenFileTable::OFTEntry* entry = oft->getEntry(i_index);
	if (!entry)
		return std::make_pair(file_not_opened, 0);

	FileDescriptor fd = getFileDescriptor(entry->fd_index);

	if (entry->cur_pos % Sector::BLOCK_SIZE == 0) {
		int status = allocateDataBlock(entry->fd_index);
		if (status != success_code)
			return std::make_pair(status, 0); 

		  // update fd after allocating new block
		fd = getFileDescriptor(entry->fd_index);
	}

	size_t have_written = 0;
	while (true) {

		size_t buffer_pos = entry->cur_pos % Sector::BLOCK_SIZE;
		size_t buffer_space = Sector::BLOCK_SIZE - buffer_pos;

		size_t block_index = entry->cur_pos / Sector::BLOCK_SIZE;
		size_t block_number = fd.data_blocks[block_index];

		if (i_count <= buffer_space) {
			std::memcpy(entry->buffer + buffer_pos, i_mem_area + have_written, i_count);
			entry->buffer_modified = true;

			if (i_count == buffer_space) {
				writeDataFromBuffer(block_number, entry);
				if (fd.getLastBlockIndex() != block_index)
					readDataToBuffer(fd.data_blocks[block_index + 1], entry);
			}

			entry->cur_pos += i_count;
			have_written += i_count;

			if (entry->cur_pos > fd.file_size) {
				fd.file_size = entry->cur_pos;
				writeFileDescriptorToIO(fd, entry->fd_index);
			}

			return make_pair(success_code, have_written);
		}

		std::memcpy(entry->buffer + buffer_pos, i_mem_area + have_written, buffer_space);
		entry->buffer_modified = true;
		writeDataFromBuffer(block_number, entry);

		entry->cur_pos += buffer_space;
		have_written += buffer_space;
		i_count -= buffer_space;

		if (entry->cur_pos > fd.file_size) {

			fd.file_size = entry->cur_pos;
			writeFileDescriptorToIO(fd, entry->fd_index);

			int status = allocateDataBlock(entry->fd_index);
			if (status != success_code)
				return std::make_pair(status, have_written);	

			fd = getFileDescriptor(entry->fd_index);
		}

		readDataToBuffer(fd.data_blocks[block_index + 1], entry);
	}
}

// first value: status code, second value: bytes read
std::pair<int, int> FileSystem::read(size_t i_index, char* i_mem_area, size_t i_count)
{
	if (i_count == 0)
		return std::make_pair(success_code, i_count);

	OpenFileTable::OFTEntry* entry = oft->getEntry(i_index);
	if (!entry)
		return std::make_pair(file_not_opened, 0);

	FileDescriptor fd = getFileDescriptor(entry->fd_index);
	if (fd.file_size == 0)
		return std::make_pair(eof_reached_before_satisfying_read_count, 0);

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

			if (i_count == have_written)
				return make_pair(success_code, have_written);
			else
				return make_pair(eof_reached_before_satisfying_read_count, have_written);
		}

		std::memcpy(i_mem_area + have_written, entry->buffer + buffer_pos, buffer_space);
		size_t block_number = fd.data_blocks[entry->cur_pos / Sector::BLOCK_SIZE];

		writeDataFromBuffer(block_number, entry);
		readDataToBuffer(block_number + 1, entry);
		entry->cur_pos += buffer_space;
		have_written += buffer_space;
		local_count -= buffer_space;
	}
}


int FileSystem::lseek(size_t i_index, size_t i_pos)
{
	OpenFileTable::OFTEntry* entry = oft->getEntry(i_index);

	if (!entry)
		return file_not_opened;

	FileDescriptor fd = getFileDescriptor(entry->fd_index);

	if (i_pos > fd.file_size || i_pos < 0)
		return position_outside_file_boundaries;

	size_t cur_pos = entry->cur_pos;

	size_t cur_block = cur_pos / Sector::BLOCK_SIZE;
	size_t new_block = i_pos / Sector::BLOCK_SIZE;

	// check if new position is within the current data block 
	if (cur_block != new_block) {
		if (cur_block != FileDescriptor::MAX_DATA_BLOCKS && fd.data_blocks[cur_block] != -1)
			writeDataFromBuffer(fd.data_blocks[cur_block], entry);

		readDataToBuffer(fd.data_blocks[new_block], entry);
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

int FileSystem::init(const std::string & i_file_name)
{
	return iosystem->init(i_file_name);
}

int FileSystem::save(const std::string & i_file_name)
{
	return iosystem->save(i_file_name);
}

void FileSystem::init()
{
	// init bitmap with 0 (means its free)
	std::bitset<Sector::BLOCK_SIZE * 8> bitmap;
	for (size_t i = 0; i < BITMAP_BLOCKS_NUMBER; i++)
		iosystem->write_block(i, (char*)&bitmap);

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

int FileSystem::recordFileToDir(const std::string & i_file_name, size_t i_fd_index)
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
				return file_already_created;

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
		int status = allocateDataBlock(0);
		if (status != success_code)
			return status;	// directory file size exceeds maximum value
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

	return success_code;
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

int FileSystem::allocateDataBlock(size_t i_index)
{
	if (i_index >= FD_NUMBER)
		return max_file_descriptiors_number_exceeded;

	// db_index is global block index
	int db_index = findFreeDataBlock();
	if (db_index == -1)
		return out_of_disk_memory;

	FileDescriptor fd = getFileDescriptor(i_index);
	bool success = fd.addBlock(db_index);
	if (!success)
		return max_file_size_exceeded;

	writeFileDescriptorToIO(fd, i_index);
	setBit(db_index, false);

	return FileSystem::success_code;
}

int FileSystem::findFreeDataBlock()
{
	for (size_t i = 0; i < BITMAP_BLOCKS_NUMBER; i++) {
		std::bitset<8 * Sector::BLOCK_SIZE> bitmap;
		iosystem->read_block(i, (char*)&bitmap);

		for (size_t j = 0; j < 8 * Sector::BLOCK_SIZE; j++) {
			size_t index = i * 8 * Sector::BLOCK_SIZE + j;
			if (index >= DATA_BLOCKS_NUMBER)
				return -1;

			if (!bitmap[index])
				return index + FIRST_DATA_BLOCK_INDEX;  // return global index
		}
	}

	return -1;
}

bool FileSystem::setBit(size_t i_index, bool i_is_free)
{
	int index = i_index - FIRST_DATA_BLOCK_INDEX;

	if (index < 0 || index >= DATA_BLOCKS_NUMBER)
		return false;

	size_t block_number = index / (Sector::BLOCK_SIZE * 8);
	size_t bitmap_index = index % (Sector::BLOCK_SIZE * 8);

	std::bitset<Sector::BLOCK_SIZE * 8> bitmap;
	iosystem->read_block(block_number, (char*)&bitmap);

	bitmap.set(bitmap_index, !i_is_free);

	iosystem->write_block(block_number, (char*)&bitmap);
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

void FileSystem::writeDataFromBuffer(size_t i_block_index, OpenFileTable::OFTEntry * ip_oft_entry)
{
	if (!ip_oft_entry->buffer_modified)
		return;

	iosystem->write_block(i_block_index, ip_oft_entry->buffer);
	ip_oft_entry->buffer_modified = false;
}

void FileSystem::readDataToBuffer(size_t i_block_index, OpenFileTable::OFTEntry * ip_oft_entry)
{
	iosystem->read_block(i_block_index, ip_oft_entry->buffer);
	ip_oft_entry->buffer_modified = false;
}
