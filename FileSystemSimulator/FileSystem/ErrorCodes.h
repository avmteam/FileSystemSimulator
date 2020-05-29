#pragma once

enum ErrorCodes
{
	file_already_created = -100,				// cr
	invalid_filename,							// cr, de
	out_of_disk_memory,							// cr, wr
	file_is_opened,								// de, op
	max_opened_files_number_exceeded,			// op
	max_file_descriptiors_number_exceeded,		// op
	file_not_opened,							// cl, wr, rd, sk
	eof_reached_before_satisfying_read_count,	// rd, 
	position_outside_file_boundaries,			// sk
	directory_max_files_exceeded,				// cr
	max_file_size_exceeded,						// 
	wrong_file_size = -91,						// in
	file_not_found = -92,						// de, op, in
};

