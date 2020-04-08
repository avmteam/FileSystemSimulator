#pragma once
class Sector
{
public:
	static const int NUMBER_OF_BYTES = 64;
	char *bytes;

	Sector() {
		bytes = new char[NUMBER_OF_BYTES];
	};

	~Sector() {
		delete[] bytes;
	};
};

