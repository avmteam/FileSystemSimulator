#include "pch.h"
#include <iostream>
#include <string>
#include "Shell.h"

using namespace std;

int main()
{
	Shell shell;

	shell.printHelp();

	string command;

	do
	{
		cout << "> ";
		getline(cin, command);

	} while (shell.parseCommand(command) != shell.exit_code);
}