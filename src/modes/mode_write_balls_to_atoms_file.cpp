#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/atoms_io.h"

#include "../common/writing_atomic_balls_in_pdb_format.h"

void write_balls_to_atoms_file(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'annotation x y z r tags adjuncts')");
	pohw.describe_io("stdout", false, true, "list of balls (line format: 'annotation x y z r tags adjuncts')");

	const std::string pdb_output=poh.argument<std::string>(pohw.describe_option("--pdb-output", "string", "file path to output query result in PDB format"), "");
	const std::string pdb_output_b_factor=poh.argument<std::string>(pohw.describe_option("--pdb-output-b-factor", "string", "name of adjunct to output as B-factor in PDB format"), "tf");
	const std::string pdb_output_template=poh.argument<std::string>(pohw.describe_option("--pdb-output-template", "string", "file path to input template for B-factor insertions"), "");
	const bool add_chain_terminators=poh.contains_option(pohw.describe_option("--add-chain-terminators", "", "flag to add TER lines after chains"));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	typedef voronota::common::ChainResidueAtomDescriptor CRAD;

	std::vector<voronota::common::ConstructionOfAtomicBalls::AtomicBall> list_of_balls;
	voronota::auxiliaries::IOUtilities().read_lines_to_set(std::cin, list_of_balls);
	if(list_of_balls.empty())
	{
		throw std::runtime_error("No input.");
	}

	if(!pdb_output.empty())
	{
		if(pdb_output_template.empty())
		{
			std::ofstream foutput(pdb_output.c_str(), std::ios::out);
			if(foutput.good())
			{
				voronota::common::WritingAtomicBallsInPDBFormat::write_atomic_balls(list_of_balls, pdb_output_b_factor, add_chain_terminators, foutput);
			}
		}
		else
		{
			voronota::auxiliaries::AtomsIO::PDBReader::Data pdb_file_data;
			{
				std::ifstream finput(pdb_output_template.c_str(), std::ios::in);
				pdb_file_data=voronota::auxiliaries::AtomsIO::PDBReader::read_data_from_file_stream(finput, true, true, true, true);
			}
			if(!pdb_file_data.valid())
			{
				throw std::runtime_error("Invalid PDB file output template.");
			}
			else
			{
				std::ofstream foutput(pdb_output.c_str(), std::ios::out);
				if(foutput.good())
				{
					std::map<CRAD, std::size_t> output_map_of_ball_ids;
					for(std::size_t i=0;i<list_of_balls.size();i++)
					{
						output_map_of_ball_ids[list_of_balls[i].crad]=i;
					}
					for(std::size_t i=0;i<pdb_file_data.atom_records.size();i++)
					{
						const voronota::auxiliaries::AtomsIO::AtomRecord& atom_record=pdb_file_data.atom_records[i];
						const std::map<CRAD, std::size_t>::const_iterator ball_id_it=output_map_of_ball_ids.find(CRAD(atom_record.serial, atom_record.chainID, atom_record.resSeq, atom_record.resName, atom_record.name, atom_record.altLoc, atom_record.iCode));
						if(ball_id_it!=output_map_of_ball_ids.end())
						{
							const std::map<std::string, double>& ball_adjuncts=list_of_balls[ball_id_it->second].value.props.adjuncts;
							const std::map<std::string, double>::const_iterator temperature_factor_it=ball_adjuncts.find(pdb_output_b_factor);
							std::string& ball_line=pdb_file_data.all_lines[pdb_file_data.map_of_atom_records_to_all_lines.at(i)];
							ball_line=(temperature_factor_it!=ball_adjuncts.end()) ?
									voronota::auxiliaries::AtomsIO::PDBWriter::write_temperature_factor_to_line(ball_line, true, temperature_factor_it->second) :
									voronota::auxiliaries::AtomsIO::PDBWriter::write_temperature_factor_to_line(ball_line, false, 0);
						}
					}
					for(std::size_t i=0;i<pdb_file_data.all_lines.size();i++)
					{
						if((i+1<pdb_file_data.all_lines.size()) || !pdb_file_data.all_lines[i].empty())
						{
							foutput << pdb_file_data.all_lines[i] << "\n";
						}
					}
				}
			}
		}
	}

	voronota::auxiliaries::IOUtilities().write_set(list_of_balls, std::cout);
}
