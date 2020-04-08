#pragma once
#include "Sector.h"
class Track
{
public:
	static const int NUMBER_OF_SECTORS = 8;
	Sector** sectors;
	Track() {
		sectors = new Sector*[NUMBER_OF_SECTORS];
		for (int i = 0; i < NUMBER_OF_SECTORS; i++) {
			sectors[i] = new Sector();
		}
	};
	~Track() {
		delete[] sectors;
	};
};

