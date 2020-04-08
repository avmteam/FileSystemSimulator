#pragma once
#include <stdexcept>
#include <cstring>

#include "Disk.h"
using namespace std;
class IOSystem
{
public:
	IOSystem(Disk*);
	~IOSystem();

	void read_block(int, char*);
	void write_block(int, char*);

private:
	Disk* ldisk;
	static const int BLOCKS_IN_CYLINDER = Cylinder::NUMBER_OF_TRACKS * Track::NUMBER_OF_SECTORS;

	int* getBlockLocationOnDisk(int);

	

};

