#pragma once
#include "Track.h"
class Cylinder
{
public:
	static const int NUMBER_OF_TRACKS = 2;
	Track** tracks;

	Cylinder() {
		tracks = new Track*[NUMBER_OF_TRACKS];
		for (int i = 0; i < NUMBER_OF_TRACKS; i++) {
			tracks[i] = new Track();
		}
	};

	~Cylinder() {
                for (int i = 0; i < NUMBER_OF_TRACKS; i++)
                    delete tracks[i];
		delete[] tracks;
	};
};
