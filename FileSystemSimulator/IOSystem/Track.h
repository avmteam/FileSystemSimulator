#pragma once
#include "Sector.h"
class Track
{
public:
	static const int NUMBER_OF_SECTORS = 8;
	static const int NUMBER_OF_BLOCKS = NUMBER_OF_SECTORS * Sector::NUMBER_OF_BLOCKS;

	Sector** sectors;
	Track() {
		sectors = new Sector*[NUMBER_OF_SECTORS];
		for (int i = 0; i < NUMBER_OF_SECTORS; i++) {
			sectors[i] = new Sector();
		}
	};
	~Track() {
		for (int i = 0; i < NUMBER_OF_SECTORS; i++)
			delete sectors[i];
		delete[] sectors;
	};
};

