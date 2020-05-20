#include "pch.h"
#include "Shell.h"
#include <iostream>

using namespace std;

Shell::Shell(FileSystem* i_filesystem)
{
	filesystem = i_filesystem;
}

Shell::Shell()
{
	FileSystem filesys;
	filesystem = &filesys;
}

Shell::~Shell()
{
}

void Shell::printHelp()
{
	cout << "Help:\nCommands:\n";
	cout << "create file - " + create_command + " <filename>\n";
	cout << "destroy file - " + destroy_command + " <filename>\n";
	cout << "open file - " + open_command + " <filename>\nnote: open file command " + 
		"returns opened file key, use it to read, write and lseek through the file\n";
	cout << "close file - " + close_command + " <key>\n";
	cout << "read from file - " + read_command + " <key> <?>\n";
	cout << "write to file - " + write_command + " <key> <?>\n";
	cout << "lseek in file - " + lseek_command + " <key> <?>\n";
	cout << "list all files on disk - " + directory_command + "\n";
	cout << "exit simulator - " + exit_command + "\n\n";
}

int Shell::parseCommand(string i_command_string)
{
	if (!isValidCommandName(i_command_string.substr(0, 3))) {
		cout << "Invalid command name, try again. Print gh to get help and view list of commands.\n";
		return invalid_command_code; 
	}

	if (i_command_string.substr(0, 2) == exit_command) {

		cout << "End.\n";
		return exit_code;
	}

	if (i_command_string.substr(0, 2) == create_command) {

		i_command_string.erase(0, 3);
		printCreateCommandResult(i_command_string);
	}

	if (i_command_string.substr(0, 2) == destroy_command) {


	}

	if (i_command_string.substr(0, 2) == open_command) {


	}

	if (i_command_string.substr(0, 2) == close_command) {


	}

	if (i_command_string.substr(0, 2) == read_command) {


	}

	if (i_command_string.substr(0, 2) == write_command) {


	}

	if (i_command_string.substr(0, 2) == lseek_command) {

		
	}

	if (i_command_string.substr(0, 2) == directory_command) {


	}

	if (i_command_string.substr(0, 2) == help_command) {

		printHelp();
		return success_code;
	}

	return -1;
}

void Shell::printCreateCommandResult(const std::string & i_file_name)
{
	if (filesystem->create(i_file_name))
		cout << "File \"" + i_file_name + "\" created successfully.\n";
	else
		cout << "File creation failure.\n";
}

void Shell::filenameLengthExceededTestCase()
{
	parseCommand("cr super_super_super_looooooooooooong_file_name.txt");
}

void Shell::invalidFilenameTestCase()
{
	parseCommand("cr \\name\\#@>.txt");
	parseCommand("cr \\na+dsfd*");
}

void Shell::createDestroyOpenFileTestCase()
{
	parseCommand("cr test.txt");
	parseCommand("ds test.txt");
	parseCommand("op test.txt");
}

void Shell::openAlreadyOpenedFileTestCase()
{
	parseCommand("cr test.txt");
	parseCommand("op test.txt");
	parseCommand("op test.txt");
}

void Shell::destroyOpenedFile()
{
	parseCommand("cr test.txt");
	parseCommand("op test.txt");
	parseCommand("ds test.txt");
}

bool Shell::isValidCommandName(string i_command_name)
{
	if (i_command_name.length() <= 2) 
		return i_command_name == help_command ||
			   i_command_name == exit_command ||
			   i_command_name == directory_command;

	if (i_command_name[2] != ' ') return false;

	string actual_command = i_command_name.substr(0, 2);

	return
		(actual_command == read_command ||
			actual_command == write_command ||
			actual_command == create_command ||
			actual_command == destroy_command ||
			actual_command == open_command ||
			actual_command == close_command ||
			actual_command == lseek_command ||
			actual_command == directory_command ||
			actual_command == help_command ||
			actual_command == exit_command);
}
