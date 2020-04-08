#include "stdafx.h"
#include "IOSystem.h"


IOSystem::IOSystem(Disk* ldisk)
{
	this->ldisk = ldisk;
}


IOSystem::~IOSystem()
{
}

int * IOSystem::getBlockLocationOnDisk(int blockNumber)
{
	int result[3];
	int cylinderNumber = -1, trackNumber = -1, sectorNumber = -1;

	cylinderNumber = blockNumber / BLOCKS_IN_CYLINDER;
	trackNumber = (blockNumber % BLOCKS_IN_CYLINDER) / Track::NUMBER_OF_SECTORS;
	sectorNumber = (blockNumber % BLOCKS_IN_CYLINDER) % Track::NUMBER_OF_SECTORS;

	result[0] = cylinderNumber;
	result[1] = trackNumber;
	result[2] = sectorNumber;
	return result;
}

void IOSystem::read_block(int blockNumber, char * p)
{
	if (blockNumber < 0 || blockNumber >= Disk::NUMBER_OF_BLOCKS)
		throw std::invalid_argument("invalid blocknumber");
	if (!p) throw std::invalid_argument("invalid pointer");


	int* blockLocation = getBlockLocationOnDisk(blockNumber);
	Sector* block = ldisk->cylinders[blockLocation[0]]->tracks[blockLocation[1]]->sectors[blockLocation[2]];
	memcpy(p, block->bytes, sizeof(block->bytes));
}

void IOSystem::write_block(int blockNumber, char * p)
{
	if (blockNumber < 0 || blockNumber >= Disk::NUMBER_OF_BLOCKS)
		throw std::invalid_argument("invalid blocknumber");
	if (!p) throw std::invalid_argument("invalid pointer");


	int* blockLocation = getBlockLocationOnDisk(blockNumber);
	Sector* block = ldisk->cylinders[blockLocation[0]]->tracks[blockLocation[1]]->sectors[blockLocation[2]];
	memcpy(block->bytes, p, sizeof(block->bytes));
}
