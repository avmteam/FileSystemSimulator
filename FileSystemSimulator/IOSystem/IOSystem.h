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

	void get_block_location_on_disk(int, int*);

	

};

