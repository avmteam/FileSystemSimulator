#include "pch.h"
#include "Shell.h"
#include <iostream>
#include <Windows.h>
#include "../FileSystem/ErrorCodes.h"

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
	cout << "import disk from file - " + import_disk_command + " <file.txt>\n";
	cout << "save disk to file - " + save_disk_command + " <file.txt>\n";
	cout << "exit simulator - " + exit_command + "\n\n";
}

void Shell::printTestCases()
{
	cout << "Test help:\n";
	cout << "  Test id\t\tTest case\n";
	cout << "\t1\t" << "open already opened file\n";
	cout << "\t2\t" << "exceed filename length\n";
	cout << "\t3\t" << "destroy opened file\n";
	cout << "\t4\t" << "create destroy open file\n";
	cout << "\t5\t" << "lseek further than end of file\n";
	cout << "\t6\t" << "exceed maximum file size\n";
	cout << "\t7\t" << "write on border of data blocks\n";
	cout << "\t8\t" << "create max number of files\n";
	cout << "\t9\t" << "out of disk memory\n";
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

		cout << "\nYou have chosen test #" + i_command_string.substr(5, 6) << endl;

		int test_id = stoi(i_command_string.substr(5, 6));
		if (i_command_string.length() > 6) {

			cout << "Improper test command.\n";
			return invalid_command_code;
		}

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 14));

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
		case 8:
			clean();
			maxFilesNumber();
			break;
		case 9:
			clean();
			outOfDiskMemory();
			break;

		default:
			cout << "Test with this id does not exist.\n";
			break;
		}

		cout << "\n";
		SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));

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
		return printCreateCommandResult(i_command_string);
	}

	if (i_command_string.substr(0, 2) == destroy_command) {

		i_command_string.erase(0, 3);
		return printDestroyCommandResult(i_command_string);
	}

	if (i_command_string.substr(0, 2) == open_command) {

		i_command_string.erase(0, 3);
		return printOpenCommandResult(i_command_string);
	}

	if (i_command_string.substr(0, 2) == close_command) {

		i_command_string.erase(0, 3);
		int key = getKeyFromCommandString(i_command_string);
		if (key == -1) {
			cout << "Invalid key format.\n";
			return -1;
		}
		return printCloseCommandResult(key);
	}

	if (i_command_string.substr(0, 2) == read_command) {

		i_command_string.erase(0, 3);
		int key = getKeyFromCommandString(i_command_string);
		if (key == -1) {
			cout << "Invalid key format.\n";
			return -1;
		}
		int number_of_chars = stoi(getIWord(i_command_string, 2));
		return printReadCommandResult(key, number_of_chars);
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
		return printWriteCommandResult(key, const_cast<char*>(text.c_str()), text.length());
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
			return printLseekCommandResult(key, pos);
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
		return success_code;
	}

	if (i_command_string.substr(0, 2) == import_disk_command) {

		return success_code;
	}

	if (i_command_string.substr(0, 2) == save_disk_command) {

		return success_code;
	}

	return invalid_command_code;
}

int Shell::printCreateCommandResult(const std::string & i_file_name)
{
	int result = filesystem->create(i_file_name);
	if (result == invalid_filename)
		cout << "Invalid filename: \"" + i_file_name + "\". Should be from 1 to 4 symbols inclusively.\n";
	else if (result == max_file_descriptiors_number_exceeded)
		cout << "Out of memory (max number of file descriptors reached).\n";
	else if (result == directory_max_files_exceeded)
		cout << "Out of memory (max number of files directory can contain reached).\n";
	else
		cout << "File \"" + i_file_name + "\" created.\n";
	return result;
}

int Shell::printDestroyCommandResult(const std::string & i_file_name)
{
	int result = filesystem->destroy(i_file_name);
	if (result == invalid_filename)
		cout << "Invalid filename: \"" + i_file_name + "\". You cannot delete directory.\n";
	else if (result == file_is_opened)
		cout << "Error destroying file: cannot destroy opened file.\n";
	else if (result == file_not_found)
		cout << "Error destroying file: file with this name does not exist.\n";
	else
		cout << "File " << i_file_name << " destroyed.\n";
	return result;
}

int Shell::printOpenCommandResult(const std::string & i_file_name)
{
	int key = filesystem->open(i_file_name);
	if (key == file_not_found)
		cout << "Error opening the file \"" + i_file_name + "\". File not found.\n";
	else if (key == max_opened_files_number_exceeded)
		cout << "Error opening the file \"" + i_file_name + "\". Max opened files number reached.\n";
	else if (key == file_is_opened)
		cout << "Error opening the file \"" + i_file_name + "\". File is already opened.\n";
	else
		cout << "File " << i_file_name << " opened. Your key for file " << "\"" << i_file_name << "\" is " << key << ".\n";
	return key;
}

int Shell::printCloseCommandResult(size_t i_index)
{
	int result = filesystem->close(i_index);
	if (result == file_not_opened)
		cout << "Error closing file " << i_index << ". File not opened\n";
	else
		cout << "File " << i_index << " closed.\n";
	return result;
}

int Shell::printReadCommandResult(size_t i_index, size_t i_count)
{
	char* mem_area = new char[i_count + 1];
	pair<int,int> result = filesystem->read(i_index, mem_area, i_count);
	int status = result.first;
	if (status == file_not_opened)
		cout << "Error occured while trying to read file " << i_index << ". File not opened.\n";
	else if (status == eof_reached_before_satisfying_read_count) {
		mem_area[result.second] = '\0';
		cout << "Reading from file " << i_index << ", " << result.second
			<< " bytes read: " << mem_area << ". Status: failed to read the desired amount of bytes.\n";
	}
	else {
		mem_area[result.second] = '\0';
		cout << "Reading from file " << i_index << ", " << result.second
			<< " bytes read: " << mem_area << ". Status: success.\n";
	}
	delete[] mem_area;
	return result.first;
}

int Shell::printWriteCommandResult(size_t i_index, char * i_mem_area, size_t i_count)
{
	pair<int, int> result = filesystem->write(i_index, i_mem_area, i_count);
	int status = result.first;
	if (status == file_not_opened)
		cout << "Error occured while trying to write to file " << i_index << ". File not opened.\n";
	else if (status == out_of_disk_memory)
		cout << "Write operaion failed: out of disk memory.\n";
	else if (status == max_file_size_exceeded)
		cout << result.second << (result.second == 1 ? " byte" : " bytes") << " written to file, status: failure, max file size exceeded.\n";
	else 
		cout << i_count << (i_count == 1 ? " byte" : " bytes") << " written to file " << i_index << ".\n";
	return result.first;
}

int Shell::printLseekCommandResult(size_t i_index, size_t i_pos)
{
	int pos = filesystem->lseek(i_index, i_pos);
	if (pos == file_not_opened)
		cout << "Error trying to set file " << i_index << " to position " << i_pos << ". File not opened.\n";
	else if (pos == position_outside_file_boundaries)
		cout << "Error trying to set file " << i_index << " to position " << i_pos << 
		". Requested position outside file boundaries.\n";
	else
		cout << "Current position in " << pos << endl;
	return pos;
}

void Shell::filenameLengthExceeded()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));
	cout << endl << create_command + " super_super_super_looooooooooooong_file_name" << endl << endl;
	SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 14));
	parseCommand(create_command + " super_super_super_looooooooooooong_file_name");
	SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));

}

void Shell::createDestroyOpenFile()
{
	vector<string> commands;
	commands.push_back(create_command + " f");
	commands.push_back(destroy_command + " f");
	commands.push_back(open_command + " f");

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	for (string var : commands)
	{
		cout << endl;
		SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));
		cout << var << endl;
		SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 14));
		parseCommand(var);
	}
	SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));

}

void Shell::openAlreadyOpenedFile()
{
	vector<string> commands;
	commands.push_back(create_command + " g");
	commands.push_back(open_command + " g");
	commands.push_back(open_command + " g");
	commands.push_back(close_command + " 0");
	commands.push_back(destroy_command + " g");

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	for (string var : commands)
	{
		cout << endl;
		SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));
		cout << var << endl;
		SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 14));
		parseCommand(var);
	}
	SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));

}

void Shell::destroyOpenedFile()
{
	vector<string> commands;
	commands.push_back(create_command + " g");
	commands.push_back(open_command + " g");
	commands.push_back(destroy_command + " g");
	commands.push_back(close_command + " 0");
	commands.push_back(destroy_command + " g");

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	for (string var : commands)
	{
		cout << endl;
		SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));
		cout << var << endl;
		SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 14));
		parseCommand(var);
	}
	SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));

}

void Shell::lseekFurtherThanEnd()
{
	vector<string> commands;
	commands.push_back(create_command + " h");
	commands.push_back(open_command + " h");
	commands.push_back(write_command + " 0 b 5");
	commands.push_back(lseek_command + " 0 15");
	commands.push_back(close_command + " 0");
	commands.push_back(destroy_command + " h");

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	for (string var : commands)
	{
		cout << endl;
		SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));
		cout << var << endl;
		SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 14));
		parseCommand(var);
	}
	SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));

}

void Shell::exceedMaxFileSize()
{
	size_t max_size = Sector::BLOCK_SIZE * FileDescriptor::MAX_DATA_BLOCKS;

	vector<string> commands;
	commands.push_back(create_command + " f2");
	commands.push_back(open_command + " f2");
	commands.push_back(write_command + " 0 b " + to_string(max_size + 1));
	commands.push_back(lseek_command + " 0 0");
	commands.push_back(read_command + " 0 " + to_string(max_size));
	commands.push_back(close_command + " 0");
	commands.push_back(destroy_command + " f2");

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	for (string var : commands)
	{
		cout << endl;
		SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));
		cout << var << endl;
		SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 14));
		parseCommand(var);
	}
	SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));

}

void Shell::writeDataOnBlocksBorder()
{
	size_t max_size = Sector::BLOCK_SIZE * FileDescriptor::MAX_DATA_BLOCKS;

	vector<string> commands;
	commands.push_back(create_command + " f3");
	commands.push_back(open_command + " f3");
	commands.push_back(write_command + " 0 b " + to_string(Sector::BLOCK_SIZE - 2));
	commands.push_back(write_command + " 0 c 5");

	commands.push_back(lseek_command + " 0 0");
	commands.push_back(read_command + " 0 " + to_string(max_size));

	commands.push_back(close_command + " 0");
	commands.push_back(destroy_command + " f3");

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	for (string var : commands)
	{
		cout << endl;
		SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));
		cout << var << endl;
		SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 14));
		parseCommand(var);
	}
	SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));

}

void Shell::maxFilesNumber()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));
	cout << "Creating " << FileSystem::FD_NUMBER << " files...\n\n";

	SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 14));
	for (int i = 1; i <= FileSystem::FD_NUMBER; ++i) {

		parseCommand(create_command + ' ' + to_string(i));
	}
	SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));

}

void Shell::outOfDiskMemory()
{
	size_t i = 0;

	while (createAndFillFile(to_string(i), 0) == success_code) ++i;
}

int Shell::createAndFillFile(string i_filename, size_t i_index)
{
	size_t max_size = Sector::BLOCK_SIZE * FileDescriptor::MAX_DATA_BLOCKS;

	int status;

	parseCommand(create_command + " " + i_filename);
	parseCommand(open_command + " " + i_filename);
	status = parseCommand(write_command + " " + to_string(i_index) + " a " + to_string(max_size));
	parseCommand(close_command + " " + to_string(i_index));

	return status;
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
			actual_command == exit_command ||
			actual_command == import_disk_command ||
			actual_command == save_disk_command);
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
