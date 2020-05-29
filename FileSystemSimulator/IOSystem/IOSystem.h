#pragma once
#include <stdexcept>
#include <cstring>
#include "Disk.h"

using namespace std;
class IOSystem
{
public:
	IOSystem(Disk*);
	IOSystem();
	~IOSystem();

	void read_block(int, char*);
	void write_block(int, char*);

	int init(const std::string & i_file_name);
	int save(const std::string & i_file_name);

private:
	Disk* ldisk;
	const int file_not_found = -92;
	const int wrong_file_size = -91;
	const size_t success_code = 0;

	void get_block_location_on_disk(int, int*);

	

};

