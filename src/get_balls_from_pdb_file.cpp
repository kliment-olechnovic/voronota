#include <iostream>

#include "auxiliaries/pdb_file_parsing.h"
#include "auxiliaries/command_line_options.h"

void get_balls_from_pdb_file(const auxiliaries::CommandLineOptions& clo)
{
	clo.check_allowed_options("--include-heteroatoms");

	const bool include_heteroatoms=clo.isopt("--include-heteroatoms");

	std::vector<auxiliaries::PDBFileParsing::AtomRecord> atoms=auxiliaries::PDBFileParsing::read_atom_records_from_pdb_file_stream(std::cin, include_heteroatoms);
}
