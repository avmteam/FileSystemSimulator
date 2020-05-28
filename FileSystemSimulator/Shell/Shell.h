#pragma once
#include <string>
#include "../FileSystem/FileSystem.h"

class Shell
{
public:
	
	const std::string read_command = "rd";
	const std::string write_command = "wr";
	const std::string create_command = "cr";
	const std::string destroy_command = "de"; 
	const std::string open_command = "op";
	const std::string close_command = "cl";
	const std::string lseek_command = "sk";
	const std::string directory_command = "dr";
	const std::string help_command = "gh";	// get help
	const std::string import_disk_command = "in";
	const std::string save_disk_command = "sv";
	const std::string exit_command = "ex";
	const std::string tests_help_command = "thelp";
	const std::string clean_command = "clean";
	

	const int exit_code = 1;
	const int invalid_command_code = 2;
	const int success_code = 0;

public:
	Shell(FileSystem* i_filesystem);
	Shell();
	~Shell();

	void printHelp();
	void printTestCases();
	int parseCommand(string i_command_string);
	int printCreateCommandResult(const std::string& i_file_name);
	int printDestroyCommandResult(const std::string& i_file_name);
	int printOpenCommandResult(const std::string& i_file_name);
	int printCloseCommandResult(size_t index);
	int printReadCommandResult(size_t i_index, size_t i_count);
	int printWriteCommandResult(size_t i_index, char* i_mem_area, size_t i_count);
	int printLseekCommandResult(size_t i_index, size_t i_pos);

	void filenameLengthExceeded();
	void createDestroyOpenFile();
	void openAlreadyOpenedFile();
	void destroyOpenedFile(); 
	void lseekFurtherThanEnd();
	void exceedMaxFileSize();
	void writeDataOnBlocksBorder();
	void maxFilesNumber();
	void outOfDiskMemory();

private:

	bool isValidCommandName(string i_command_name);
	int getKeyFromCommandString(string i_command_string);
	static string getIWord(string i_command_string, int i_index);
	int createAndFillFile(string i_filename, size_t i_index);
	void clean();

private:

	FileSystem* filesystem;
};

