#include "stdafx.h"
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
