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
	cout << "write to file 1st variant - " + write_command + " <key> <text>\n";
	cout << "write to file 2nd variant - " + write_command + " <key> <character> <count>\n";
	cout << "lseek in file - " + lseek_command + " <key> <position>\n";
	cout << "list all files on disk - " + directory_command + "\n";
	cout << "list all test cases - " + tests_help_command + "\n";
	cout << "clean file system - " + clean_command + "\n";
	cout << "exit simulator - " + exit_command + "\n\n";
}

void Shell::printTestCases()
{
	cout << "Test help:\n";
	cout << "  Test id\t\tTest case\n";
	cout << "\t1\t" << "open already opened file\n";
	cout << "\t2\t" << "exceed filename length\n";
	cout << "\t3\t" << "destroy opened file\n";
	cout << "\t4\t" << "create destroy opened file\n";
	cout << "\t5\t" << "lseek further than end of file\n";
	cout << "\t6\t" << "exceed maximum file size\n";
	cout << "\t7\t" << "write on border of data blocks\n";
	//cout << "\t8\t" << "test case 8\n";
	//cout << "\t9\t" << "test case 9\n";
	cout << "\nUsage: test <id>\n";
}

int Shell::parseCommand(string i_command_string)
{
	if (i_command_string == clean_command) {

		clean();
		cout << "Old file system deleted, new file system created.\n";
		return success_code;
	}

	if (i_command_string == tests_help_command) {

		printTestCases();
		return success_code;
	}

	if (i_command_string.length() >= 6 && i_command_string.substr(0, 4) == "test") {

		cout << "You have chosen test #" + i_command_string.substr(5, 6) << endl;

		int test_id = stoi(i_command_string.substr(5, 6));
		if (i_command_string.length() > 6) {

			cout << "Improper test command.\n";
			return invalid_command_code;
		}

		switch (test_id) {

		case 1:
			clean();
			openAlreadyOpenedFile();
			break;
		case 2:
			clean();
			filenameLengthExceeded();
			break;
		case 3:
			clean();
			destroyOpenedFile();
			break;
		case 4:
			clean();
			createDestroyOpenFile();
			break;
		case 5:
			clean();
			lseekFurtherThanEnd();
			break;
		case 6:
			clean();
			exceedMaxFileSize();
			break;
		case 7:
			clean();
			writeDataOnBlocksBorder();
			break;

		default:
			cout << "Test with this id does not exist.\n";
			break;
		}

		return success_code;
	}

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
		if (key == -1) {
			cout << "Invalid key format.\n";
			return -1;
		}
		printCloseCommandResult(key);
	}

	if (i_command_string.substr(0, 2) == read_command) {

		i_command_string.erase(0, 3);
		int key = getKeyFromCommandString(i_command_string);
		if (key == -1) {
			cout << "Invalid key format.\n";
			return -1;
		}
		int number_of_chars = stoi(getIWord(i_command_string, 2));
		printReadCommandResult(key, number_of_chars);
	}

	if (i_command_string.substr(0, 2) == write_command) {

		i_command_string.erase(0, 3);
		int key = getKeyFromCommandString(i_command_string);
		if (key == -1) {
			cout << "Invalid key format.\n";
			return -1;
		}
		string text = getIWord(i_command_string, 2);
		if (text.length() == 1) {

			string count_string = getIWord(i_command_string, 3);

			if (count_string != "") {

				try {

					int count = stoi(count_string);

					string character = text;
					for (int i = 0; i < count - 1; ++i)
						text += character;
				}
				catch (invalid_argument e1) {

					cout << "Invalid count.\n";
					return -1;
				}
			}
		}
		printWriteCommandResult(key, const_cast<char*>(text.c_str()), text.length());
	}

	if (i_command_string.substr(0, 2) == lseek_command) {

		i_command_string.erase(0, 3); 
		int key = getKeyFromCommandString(i_command_string);
		if (key == -1) {
			cout << "Invalid key format.\n";
			return -1;
		}
		try {
			int pos = stoi(getIWord(i_command_string, 2));
			printLseekCommandResult(key, pos);
		}
		catch (invalid_argument e) {
			cout << "Invalid position.\n";
			return -1;
		}
	}

	if (i_command_string.substr(0, 2) == directory_command) {

		cout << "All files:\n";
		for (FileSystem::FileInfo fi : filesystem->directory()) {

			cout << fi.file_name << ", " << fi.file_length << " bytes." << endl;
		}
	}

	return -1;
}

void Shell::printCreateCommandResult(const std::string & i_file_name)
{
	if (filesystem->create(i_file_name))
		cout << "File \"" + i_file_name + "\" created.\n";
	else
		cout << "File creation failure.\n";
}

void Shell::printDestroyCommandResult(const std::string & i_file_name)
{
	if (!filesystem->destroy(i_file_name))
		cout << "Error occured while trying to destroy file \"" << i_file_name << "\".\n";
	else
		cout << "File " << i_file_name << " destroyed.\n";
}

void Shell::printOpenCommandResult(const std::string & i_file_name)
{
	int key = filesystem->open(i_file_name);
	if (key != -1)
		cout << "File " << i_file_name << " opened. Your key for file " << "\"" << i_file_name << "\" is " << key << ".\n";
	else
		cout << "Error occured while trying to open file \"" << i_file_name << "\".\n";
}

void Shell::printCloseCommandResult(size_t i_index)
{
	if (!filesystem->close(i_index))
		cout << "Error occured while trying to close file " << i_index << ".\n";
	else
		cout << "File " << i_index << " closed.\n";
}

void Shell::printReadCommandResult(size_t i_index, size_t i_count)
{
	char* mem_area = new char[i_count + 1];
	int bytes_read = filesystem->read(i_index, mem_area, i_count);
	if (bytes_read == -1)
		cout << "Error occured while trying to read file " << i_index << ".\n";
	else {
		mem_area[bytes_read] = '\0';
		cout << "Read from file: " << mem_area << endl;
	}
	delete[] mem_area;
}

void Shell::printWriteCommandResult(size_t i_index, char * i_mem_area, size_t i_count)
{
	int bytes = filesystem->write(i_index, i_mem_area, i_count);
	if (bytes == -1)
		cout << "Invalid file key.\n";
	else if (bytes == -2)
		cout << "New block allocation failed.\n";
	else 
		cout << i_count << (i_count == 1 ? " byte" : " bytes") << " written to file " << i_index << ".\n";
}

void Shell::printLseekCommandResult(size_t i_index, size_t i_pos)
{
	int pos = filesystem->lseek(i_index, i_pos);
	if (pos == -1)
		cout << "Error occured while trying to lseek through file " << i_index << ".\n";
	else
		cout << "Current position in " << pos << endl;
}

void Shell::filenameLengthExceeded()
{
	parseCommand("cr super_super_super_looooooooooooong_file_name");
}

void Shell::createDestroyOpenFile()
{
	parseCommand(create_command + " f");
	parseCommand(destroy_command + " f");
	parseCommand(open_command + " f");
}

void Shell::openAlreadyOpenedFile()
{
	parseCommand(create_command + " g");
	parseCommand(open_command + " g");
	parseCommand(open_command + " g");

	parseCommand(close_command + " 0");
	parseCommand(destroy_command + " g");
}

void Shell::destroyOpenedFile()
{
	parseCommand(create_command + " h");
	parseCommand(open_command + " h");
	parseCommand(destroy_command + " h");

	parseCommand(close_command + " 0");
	parseCommand(destroy_command + " h");
}

void Shell::lseekFurtherThanEnd()
{
	cout << "Note! We assume this file owns descriptor with index 0.\n";
	parseCommand(create_command + " h");
	parseCommand(open_command + " h");
	parseCommand(write_command + " 0 b 5");
	parseCommand(lseek_command + " 0 5");

	parseCommand(close_command + " 0");
	parseCommand(destroy_command + " h");
}

void Shell::exceedMaxFileSize()
{
	parseCommand(create_command + " f2");
	parseCommand(open_command + " f2");
	size_t max_size = Sector::BLOCK_SIZE * FileDescriptor::MAX_DATA_BLOCKS;
	parseCommand(write_command + " 0 b " + to_string(max_size + 1));
	parseCommand(lseek_command + " 0 0");
	parseCommand(read_command + " 0 " + to_string(max_size));

	parseCommand(close_command + " 0");
	parseCommand(destroy_command + " f2");
}

void Shell::writeDataOnBlocksBorder()
{
	parseCommand(create_command + " f3");
	parseCommand(open_command + " f3");
	parseCommand(write_command + " 0 b " + to_string(Sector::BLOCK_SIZE));
	parseCommand(write_command + " 0 c 5");

	parseCommand(lseek_command + " 0 0");
	parseCommand(read_command + " 0 " + to_string(Sector::BLOCK_SIZE * FileDescriptor::MAX_DATA_BLOCKS));

	parseCommand(close_command + " 0");
	parseCommand(destroy_command + " f3");
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
	string key_string = getIWord(i_command_string, 1);

	try {

		int key = stoi(key_string);
		return key;
	}
	catch (invalid_argument e) {

		return -1;
	}
}

// returns word number index counting from 1
string Shell::getIWord(string i_command_string, int i_index)
{
	int count = 0;
	size_t i = 0;

	string word = "";

	while (count++ != i_index) {

		word = "";

		// TODO: fix so that we cat write white spaces to file
		while (i_command_string[i] != ' ' && i < i_command_string.length())
			word += i_command_string[i++];

		if (!i || (count != i_index && i_command_string.length() == i))
			//throw invalid_argument("invalid word index");
			return "";

		i++;
	}

	return word;
}

void Shell::clean()
{
	delete filesystem;
	filesystem = new FileSystem();
}
