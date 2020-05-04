#pragma once
#include "Cylinder.h"
class Disk
{
public:
	static const int NUMBER_OF_CYLINDERS = 4;
	Cylinder** cylinders;
	static const int NUMBER_OF_BLOCKS = NUMBER_OF_CYLINDERS * Cylinder::NUMBER_OF_TRACKS * Track::NUMBER_OF_SECTORS * Sector::NUMBER_OF_BLOCKS;

	Disk() {
		cylinders = new Cylinder*[NUMBER_OF_CYLINDERS];
		for (int i = 0; i < NUMBER_OF_CYLINDERS; i++) {
			cylinders[i] = new Cylinder();
		}
	};

	~Disk() {
		for (int i = 0; i < NUMBER_OF_CYLINDERS; i++)
			delete cylinders[i];
		delete[] cylinders;
	};
	
};

