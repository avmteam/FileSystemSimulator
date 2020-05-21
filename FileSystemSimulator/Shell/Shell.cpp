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
	filesystem = new FileSystem();
}

Shell::~Shell()
{
	delete filesystem;
}

void Shell::printHelp()
{
	cout << "Help:\nCommands:\n";
	cout << "create file - " + create_command + " <filename>\n";
	cout << "destroy file - " + destroy_command + " <filename>\n";
	cout << "open file - " + open_command + " <filename>\nnote: open file command " + 
		"returns opened file key, use it to read, write and lseek through the file\n";
	cout << "close file - " + close_command + " <key>\n";
	cout << "read from file - " + read_command + " <key> <number_of_characters_to_read>\n";
	cout << "write to file - " + write_command + " <key> <text>\n";
	cout << "lseek in file - " + lseek_command + " <key> <position>\n";
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

	if (i_command_string.substr(0, 2) == help_command) {

		printHelp();
		return success_code;
	}

	if (i_command_string.substr(0, 2) == create_command) {

		i_command_string.erase(0, 3);
		printCreateCommandResult(i_command_string);
	}

	if (i_command_string.substr(0, 2) == destroy_command) {

		i_command_string.erase(0, 3);
		printDestroyCommandResult(i_command_string);
	}

	if (i_command_string.substr(0, 2) == open_command) {

		i_command_string.erase(0, 3);
		printOpenCommandResult(i_command_string);
	}

	if (i_command_string.substr(0, 2) == close_command) {

		i_command_string.erase(0, 3);
		int key = getKeyFromCommandString(i_command_string);
		printCloseCommandResult(key);
	}

	if (i_command_string.substr(0, 2) == read_command) {

		i_command_string.erase(0, 3);
		int key = stoi(getIWord(i_command_string, 1));
		int number_of_chars = stoi(getIWord(i_command_string, 2));
		printReadCommandResult(key, number_of_chars);
	}

	if (i_command_string.substr(0, 2) == write_command) {

		i_command_string.erase(0, 3);
		int key = getKeyFromCommandString(i_command_string);
		string text = getIWord(i_command_string, 2);
		printWriteCommandResult(key, const_cast<char*>(text.c_str()), text.length());
	}

	if (i_command_string.substr(0, 2) == lseek_command) {

		i_command_string.erase(0, 3);
		int key = getKeyFromCommandString(i_command_string);
		int pos = stoi(getIWord(i_command_string, 2));
		printLseekCommandResult(key, pos);
	}

	if (i_command_string.substr(0, 2) == directory_command) {

		cout << "All files: ";
		for (FileSystem::FileInfo fi : filesystem->directory()) {

			cout << fi.file_name << ", " << fi.file_length << "b" << endl;
		}
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

void Shell::printDestroyCommandResult(const std::string & i_file_name)
{
	if (!filesystem->destroy(i_file_name))
		cout << "Error occured while trying to destroy requested file.\n";
}

void Shell::printOpenCommandResult(const std::string & i_file_name)
{
	int key = filesystem->open(i_file_name);
	if (key != -1)
		cout << "Your key for file " << "\"" << i_file_name << "\"" << key << endl;
	else
		cout << "Error occured while trying to open requested file.\n";
}

void Shell::printCloseCommandResult(size_t index)
{
	if (!filesystem->close(index)) 
		cout << "Error occured while trying to close requested file";
}

void Shell::printReadCommandResult(size_t index, size_t count)
{
	char* mem_area = new char[0];
	if (!filesystem->read(index, mem_area, count))
		cout << "Error occured while trying to read requested file.\n";
	else
		cout << "Read from file: " << mem_area << endl;
}

void Shell::printWriteCommandResult(size_t index, char * mem_area, size_t count)
{
	if (!filesystem->write(index, mem_area, count))
		cout << "Error occured while trying to write to requested file.\n";
}

void Shell::printLseekCommandResult(size_t index, size_t pos)
{
	if (!filesystem->lseek(index, pos))
		cout << "Error occured while trying to lseek through requested file.\n";
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

int Shell::getKeyFromCommandString(string i_command_string)
{
	return(stoi(getIWord(i_command_string, 1)));
}

// returns word number index counting from 1
string Shell::getIWord(string i_command_string, int index)
{
	int count = 0;
	size_t i = 0;

	string word = "";

	while (count++ != index) {

		word = "";

		// TODO: fix so that we cat write white spaces to file
		while (i_command_string[i] != ' ' && i < i_command_string.length())
			word += i_command_string[i++];

		if (!i || i == i_command_string.length())
			throw std::invalid_argument("invalid word index");

		i++;
	}

	return word;
}
