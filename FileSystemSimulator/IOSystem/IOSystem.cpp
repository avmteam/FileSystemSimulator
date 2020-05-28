#include "stdafx.h"
#include <fstream>
#include "IOSystem.h"


IOSystem::IOSystem(Disk* ldisk)
{
	this->ldisk = ldisk;
}

IOSystem::IOSystem()
{
	ldisk = new Disk();
}


IOSystem::~IOSystem()
{
}

void IOSystem::get_block_location_on_disk(int blockNumber, int* result)
{

	result[0] = blockNumber / Cylinder::NUMBER_OF_BLOCKS; //cylinder number
	result[1] = (blockNumber % Cylinder::NUMBER_OF_BLOCKS) / Track::NUMBER_OF_BLOCKS; //track number
	result[2] = ((blockNumber % Cylinder::NUMBER_OF_BLOCKS) % Track::NUMBER_OF_BLOCKS) / Sector::NUMBER_OF_BLOCKS; //sector number
	result[3] = ((blockNumber % Cylinder::NUMBER_OF_BLOCKS) % Track::NUMBER_OF_BLOCKS) % Sector::NUMBER_OF_BLOCKS; //block number

}

void IOSystem::read_block(int blockNumber, char * p)
{
	if (blockNumber < 0 || blockNumber >= Disk::NUMBER_OF_BLOCKS)
		throw std::invalid_argument("invalid blocknumber");
	//if (!p) throw std::invalid_argument("invalid pointer");


	int* blockLocation = new int[4];
	get_block_location_on_disk(blockNumber, blockLocation);
	char* block = ldisk->cylinders[blockLocation[0]]->tracks[blockLocation[1]]->sectors[blockLocation[2]]->blocks[blockLocation[3]];
	delete[] blockLocation;
	memcpy(p, block, Sector::BLOCK_SIZE);
}

void IOSystem::write_block(int blockNumber, char * p)
{
	if (blockNumber < 0 || blockNumber >= Disk::NUMBER_OF_BLOCKS)
		throw std::invalid_argument("invalid blocknumber");
	if (!p) throw std::invalid_argument("invalid pointer");


	int* blockLocation = new int[4];
	get_block_location_on_disk(blockNumber, blockLocation);
	char* block = ldisk->cylinders[blockLocation[0]]->tracks[blockLocation[1]]->sectors[blockLocation[2]]->blocks[blockLocation[3]];
	delete[] blockLocation;
	memcpy(block, p, Sector::BLOCK_SIZE);
}

bool IOSystem::init(const std::string & i_file_name)
{
	ifstream file;
	file.open(i_file_name.c_str(), std::ifstream::binary);

	if (!file) return false;

	file.seekg(0, ios_base::end);
	int lSize = file.tellg();
	if (lSize < Disk::NUMBER_OF_BLOCKS * Sector::BLOCK_SIZE) return false;
	file.seekg(0, ios_base::beg);

	for (int i = 0; i < Disk::NUMBER_OF_CYLINDERS; i++)
		for (int j = 0; j < Cylinder::NUMBER_OF_TRACKS; j++)
			for (int k = 0; k < Track::NUMBER_OF_SECTORS; k++)
				for (int b = 0; b < Sector::NUMBER_OF_BLOCKS; b++)
					file.read(ldisk->cylinders[i]->tracks[j]->sectors[k]->blocks[b], Sector::BLOCK_SIZE);
	file.close();
	return true;
}

bool IOSystem::save(const std::string & i_file_name)
{
	ofstream fileo;
	fileo.open(i_file_name.c_str(), ofstream::out | ofstream::binary);

	for (int i = 0; i < Disk::NUMBER_OF_CYLINDERS; i++)
		for (int j = 0; j < Cylinder::NUMBER_OF_TRACKS; j++)
			for (int k = 0; k < Track::NUMBER_OF_SECTORS; k++)
				for (int b = 0; b < Sector::NUMBER_OF_BLOCKS; b++)
					fileo.write(ldisk->cylinders[i]->tracks[j]->sectors[k]->blocks[b], Sector::BLOCK_SIZE);
	fileo.close();
	return true;
}
