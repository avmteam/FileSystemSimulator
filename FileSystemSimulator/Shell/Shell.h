#pragma once
#include <string>
#include "../FileSystem/FileSystem.h"

class Shell
{
public:
	
	const std::string read_command = "rd";
	const std::string write_command = "wr";
	const std::string create_command = "cr";
	const std::string destroy_command = "ds"; 
	const std::string open_command = "op";
	const std::string close_command = "cl";
	const std::string lseek_command = "ls";
	const std::string directory_command = "dr";
	const std::string help_command = "gh";	// get help
	const std::string exit_command = "ex";

	const int exit_code = 1;
	const int invalid_command_code = 2;
	const int success_code = 0;

public:
	Shell(FileSystem* i_filesystem);
	Shell();
	~Shell();

	void printHelp();
	int parseCommand(string i_command_string);
	void printCreateCommandResult(const std::string& i_file_name);
	void printDestroyCommandResult(const std::string& i_file_name);
	void printOpenCommandResult(const std::string& i_file_name);
	void printCloseCommandResult(size_t index);
	void printReadCommandResult(size_t index, size_t count);
	void printWriteCommandResult(size_t index, char* mem_area, size_t coun);
	void printLseekCommandResult(size_t index, size_t pos);

	void filenameLengthExceededTestCase();
	void invalidFilenameTestCase();
	void createDestroyOpenFileTestCase();
	void openAlreadyOpenedFileTestCase();
	void destroyOpenedFile();

private:

	bool isValidCommandName(string i_command_name);
	int getKeyFromCommandString(string i_command_string);
	static string getIWord(string i_command_string, int index);

private:

	FileSystem* filesystem;
};
