#include "pch.h"
#include <iostream>
#include <string>
#include "Shell.h"
#include <Windows.h>


using namespace std;

int main()
{
	Shell shell;

	shell.printHelp();

	string command;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	do
	{
		SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 15));
		cout << "> ";
		getline(cin, command);
		SetConsoleTextAttribute(hConsole, (WORD)((0 << 4) | 14));

	} while (shell.parseCommand(command) != shell.exit_code);
}