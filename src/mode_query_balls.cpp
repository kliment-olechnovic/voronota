#include <iostream>
#include <stdexcept>
#include <fstream>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/atoms_io.h"

#include "modescommon/ball_value.h"
#include "modescommon/sequence_utilities.h"
#include "modescommon/matching_utilities.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;

inline auxiliaries::AtomsIO::AtomRecord convert_ball_record_to_single_atom_record(const CRAD& crad, const BallValue& value, const std::string& temperature_factor_adjunct_name)
{
	auxiliaries::AtomsIO::AtomRecord atom_record=auxiliaries::AtomsIO::AtomRecord();
	atom_record.record_name=(value.props.tags.count("het")>0 ? std::string("HETATM") : std::string("ATOM"));
	if(crad.serial!=CRAD::null_num())
	{
		atom_record.serial=crad.serial;
		atom_record.serial_valid=true;
	}
	atom_record.name=crad.name;
	atom_record.altLoc=crad.altLoc;
	atom_record.resName=crad.resName;
	atom_record.chainID=crad.chainID;
	if(crad.resSeq!=CRAD::null_num())
	{
		atom_record.resSeq=crad.resSeq;
		atom_record.resSeq_valid=true;
	}
	atom_record.iCode=crad.iCode;
	atom_record.x=value.x;
	atom_record.x_valid=true;
	atom_record.y=value.y;
	atom_record.y_valid=true;
	atom_record.z=value.z;
	atom_record.z_valid=true;
	{
		const std::map<std::string, double>::const_iterator oc_it=value.props.adjuncts.find("oc");
		if(oc_it!=value.props.adjuncts.end())
		{
			atom_record.occupancy=oc_it->second;
			atom_record.occupancy_valid=true;
		}
	}
	{
		const std::map<std::string, double>::const_iterator tf_it=value.props.adjuncts.find(temperature_factor_adjunct_name);
		if(tf_it!=value.props.adjuncts.end())
		{
			atom_record.tempFactor=tf_it->second;
			atom_record.tempFactor_valid=true;
		}
	}
	{
		for(std::set<std::string>::const_iterator tags_it=value.props.tags.begin();tags_it!=value.props.tags.end() && atom_record.element.empty();++tags_it)
		{
			if(tags_it->find("el=")!=std::string::npos)
			{
				atom_record.element=tags_it->substr(3, 1);
			}
		}
	}
	return atom_record;
}

}

void query_balls(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		ods.push_back(OD("--match", "string", "selection"));
		ods.push_back(OD("--match-not", "string", "negative selection"));
		ods.push_back(OD("--match-tags", "string", "tags to match"));
		ods.push_back(OD("--match-tags-not", "string", "tags to not match"));
		ods.push_back(OD("--match-adjuncts", "string", "adjuncts intervals to match"));
		ods.push_back(OD("--match-adjuncts-not", "string", "adjuncts intervals to not match"));
		ods.push_back(OD("--match-external-annotations", "string", "file path to input matchable annotations"));
		ods.push_back(OD("--invert", "", "flag to invert selection"));
		ods.push_back(OD("--whole-residues", "", "flag to select whole residues"));
		ods.push_back(OD("--drop-atom-serials", "", "flag to drop atom serial numbers from input"));
		ods.push_back(OD("--drop-altloc-indicators", "", "flag to drop alternate location indicators from input"));
		ods.push_back(OD("--drop-tags", "", "flag to drop all tags from input"));
		ods.push_back(OD("--drop-adjuncts", "", "flag to drop all adjuncts from input"));
		ods.push_back(OD("--set-tags", "string", "set tags instead of filtering"));
		ods.push_back(OD("--set-dssp-tags", "string", "file path to input DSSP file"));
		ods.push_back(OD("--set-adjuncts", "string", "set adjuncts instead of filtering"));
		ods.push_back(OD("--set-external-adjuncts", "string", "file path to input external adjuncts"));
		ods.push_back(OD("--set-external-adjuncts-name", "string", "name for external adjuncts"));
		ods.push_back(OD("--renumber-from-adjunct", "string", "adjunct name to use for input residue renumbering"));
		ods.push_back(OD("--set-ref-seq-num-adjunct", "string", "file path to input reference sequence"));
		ods.push_back(OD("--ref-seq-alignment", "string", "file path to output alignment with reference"));
		ods.push_back(OD("--seq-output", "string", "file path to output query result sequence string"));
		ods.push_back(OD("--pdb-output", "string", "file path to output query result in PDB format"));
		ods.push_back(OD("--pdb-output-b-factor", "string", "name of adjunct to output as B-factor in PDB format"));
		ods.push_back(OD("--pdb-output-template", "string", "file path to input template for B-factor insertions"));
		if(!poh.assert(ods, false))
		{
			poh.print_io_description("stdin", true, false, "list of balls (line format: 'annotation x y z r tags adjuncts')");
			poh.print_io_description("stdout", false, true, "list of balls (line format: 'annotation x y z r tags adjuncts')");
			return;
		}
	}

	const std::string match=poh.argument<std::string>("--match", "");
	const std::string match_not=poh.argument<std::string>("--match-not", "");
	const std::string match_tags=poh.argument<std::string>("--match-tags", "");
	const std::string match_tags_not=poh.argument<std::string>("--match-tags-not", "");
	const std::string match_adjuncts=poh.argument<std::string>("--match-adjuncts", "");
	const std::string match_adjuncts_not=poh.argument<std::string>("--match-adjuncts-not", "");
	const std::string match_external_annotations=poh.argument<std::string>("--match-external-annotations", "");
	const bool invert=poh.contains_option("--invert");
	const bool whole_residues=poh.contains_option("--whole-residues");
	const bool drop_atom_serial=poh.contains_option("--drop-atom-serials");
	const bool drop_altloc_indicators=poh.contains_option("--drop-altloc-indicators");
	const bool drop_tags=poh.contains_option("--drop-tags");
	const bool drop_adjuncts=poh.contains_option("--drop-adjuncts");
	const std::string set_tags=poh.argument<std::string>("--set-tags", "");
	const std::string set_dssp_tags=poh.argument<std::string>("--set-dssp-tags", "");
	const std::string set_adjuncts=poh.argument<std::string>("--set-adjuncts", "");
	const std::string set_external_adjuncts=poh.argument<std::string>("--set-external-adjuncts", "");
	const std::string set_external_adjuncts_name=poh.argument<std::string>("--set-external-adjuncts-name", "ex");
	const std::string renumber_from_adjunct=poh.argument<std::string>("--renumber-from-adjunct", "");
	const std::string set_ref_seq_num_adjunct=poh.argument<std::string>("--set-ref-seq-num-adjunct", "");
	const std::string ref_seq_alignment=poh.argument<std::string>("--ref-seq-alignment", "");
	const std::string seq_output=poh.argument<std::string>("--seq-output", "");
	const std::string pdb_output=poh.argument<std::string>("--pdb-output", "");
	const std::string pdb_output_b_factor=poh.argument<std::string>("--pdb-output-b-factor", "tf");
	const std::string pdb_output_template=poh.argument<std::string>("--pdb-output-template", "");

	std::vector< std::pair<CRAD, BallValue> > list_of_balls;
	auxiliaries::IOUtilities().read_lines_to_map(std::cin, list_of_balls);
	if(list_of_balls.empty())
	{
		throw std::runtime_error("No input.");
	}

	if(!renumber_from_adjunct.empty())
	{
		std::vector< std::pair<CRAD, BallValue> > refined_list_of_balls;
		refined_list_of_balls.reserve(list_of_balls.size());
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			const BallValue& value=list_of_balls[i].second;
			if(value.props.adjuncts.count(renumber_from_adjunct)>0)
			{
				refined_list_of_balls.push_back(list_of_balls[i]);
				refined_list_of_balls.back().first.resSeq=static_cast<int>(value.props.adjuncts.find(renumber_from_adjunct)->second);
				refined_list_of_balls.back().second.props.adjuncts.erase(renumber_from_adjunct);
			}
		}
		list_of_balls=refined_list_of_balls;
	}

	if(drop_atom_serial || drop_altloc_indicators || drop_tags || drop_adjuncts)
	{
		for(std::vector< std::pair<CRAD, BallValue> >::iterator it=list_of_balls.begin();it!=list_of_balls.end();++it)
		{
			if(drop_atom_serial)
			{
				it->first.serial=CRAD::null_num();
			}
			if(drop_altloc_indicators)
			{
				it->first.altLoc.clear();
			}
			if(drop_tags)
			{
				it->second.props.tags.clear();
			}
			if(drop_adjuncts)
			{
				it->second.props.adjuncts.clear();
			}
		}
	}

	const std::set<CRAD> matchable_external_set_of_crads=auxiliaries::IOUtilities().read_file_lines_to_set< std::set<CRAD> >(match_external_annotations);

	auxiliaries::AtomsIO::DSSPReader::Data dssp_file_data;
	if(!set_dssp_tags.empty())
	{
		std::ifstream input_file(set_dssp_tags.c_str(), std::ios::in);
		dssp_file_data=auxiliaries::AtomsIO::DSSPReader::read_data_from_file_stream(input_file);
	}

	const std::map<CRAD, double> map_of_external_adjunct_values=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRAD, double> >(set_external_adjuncts);

	const std::string reference_sequence=SequenceUtilities::read_sequence_from_file(set_ref_seq_num_adjunct);

	std::set<std::size_t> output_set_of_ball_ids;

	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		const CRAD& crad=list_of_balls[i].first;
		const BallValue& value=list_of_balls[i].second;
		const bool passed=(MatchingUtilities::match_crad(crad, match, match_not) &&
				MatchingUtilities::match_set_of_tags(value.props.tags, match_tags, match_tags_not) &&
				MatchingUtilities::match_map_of_adjuncts(value.props.adjuncts, match_adjuncts, match_adjuncts_not) &&
				(match_external_annotations.empty() || MatchingUtilities::match_crad_with_set_of_crads(crad, matchable_external_set_of_crads)));
		if((passed && !invert) || (!passed && invert))
		{
			output_set_of_ball_ids.insert(i);
		}
	}

	if(whole_residues)
	{
		std::set<CRAD> residues_crads;
		for(std::set<std::size_t>::const_iterator it=output_set_of_ball_ids.begin();it!=output_set_of_ball_ids.end();++it)
		{
			residues_crads.insert(list_of_balls[*it].first.without_atom());
		}
		for(std::size_t i=0;i<list_of_balls.size();i++)
		{
			if(MatchingUtilities::match_crad_with_set_of_crads(list_of_balls[i].first, residues_crads))
			{
				output_set_of_ball_ids.insert(i);
			}
		}
	}

	std::vector<CRAD> residue_sequence_vector;
	{
		std::set<CRAD> residues_crads;
		for(std::set<std::size_t>::const_iterator it=output_set_of_ball_ids.begin();it!=output_set_of_ball_ids.end();++it)
		{
			const CRAD residue_crad=list_of_balls[*it].first.without_atom();
			if(residues_crads.count(residue_crad)==0)
			{
				residue_sequence_vector.push_back(residue_crad);
				residues_crads.insert(residue_crad);
			}
		}
	}

	if(!set_tags.empty() || !set_adjuncts.empty() || !map_of_external_adjunct_values.empty())
	{
		for(std::set<std::size_t>::const_iterator it=output_set_of_ball_ids.begin();it!=output_set_of_ball_ids.end();++it)
		{
			const CRAD& crad=list_of_balls[*it].first;
			BallValue& value=list_of_balls[*it].second;
			value.props.update_tags(set_tags);
			value.props.update_adjuncts(set_adjuncts);
			if(!map_of_external_adjunct_values.empty())
			{
				std::map<CRAD, double>::const_iterator adjunct_value_it=map_of_external_adjunct_values.find(crad);
				if(adjunct_value_it==map_of_external_adjunct_values.end())
				{
					adjunct_value_it=map_of_external_adjunct_values.find(crad.without_atom());
				}
				if(adjunct_value_it!=map_of_external_adjunct_values.end())
				{
					value.props.adjuncts[set_external_adjuncts_name]=adjunct_value_it->second;
				}
			}
		}
	}

	if(!reference_sequence.empty())
	{
		const std::map<CRAD, double> sequence_mapping=SequenceUtilities::construct_sequence_mapping(residue_sequence_vector, reference_sequence, ref_seq_alignment);
		for(std::set<std::size_t>::const_iterator it=output_set_of_ball_ids.begin();it!=output_set_of_ball_ids.end();++it)
		{
			const CRAD& crad=list_of_balls[*it].first;
			const std::map<CRAD, double>::const_iterator sm_it=sequence_mapping.find(crad.without_atom());
			if(sm_it!=sequence_mapping.end())
			{
				BallValue& value=list_of_balls[*it].second;
				value.props.adjuncts["refseq"]=sm_it->second;
			}
		}
	}

	if(!dssp_file_data.dssp_records.empty())
	{
		std::map<CRAD, std::size_t> map_of_dssp_records;
		for(std::size_t i=0;i<dssp_file_data.dssp_records.size();i++)
		{
			const auxiliaries::AtomsIO::DSSPReader::DSSPRecord& record=dssp_file_data.dssp_records[i];
			CRAD crad;
			crad.chainID=record.chainID;
			crad.resSeq=record.resSeq;
			crad.iCode=record.iCode;
			map_of_dssp_records.insert(std::make_pair(crad, i));
		}
		for(std::vector< std::pair<CRAD, BallValue> >::iterator it=list_of_balls.begin();it!=list_of_balls.end();++it)
		{
			const CRAD& full_crad=it->first;
			CRAD crad;
			crad.chainID=full_crad.chainID;
			crad.resSeq=full_crad.resSeq;
			crad.iCode=full_crad.iCode;
			const std::map<CRAD, std::size_t>::const_iterator drm_it=map_of_dssp_records.find(crad);
			if(drm_it!=map_of_dssp_records.end())
			{
				const auxiliaries::AtomsIO::DSSPReader::DSSPRecord& dssp_record=dssp_file_data.dssp_records[drm_it->second];
				if(!dssp_record.resSSE.empty())
				{
					BallValue& value=it->second;
					value.props.tags.insert(std::string("dssp=")+dssp_record.resSSE);
				}
			}
		}
	}

	if(!set_tags.empty() || !set_adjuncts.empty() || !map_of_external_adjunct_values.empty() || !reference_sequence.empty() || !dssp_file_data.dssp_records.empty())
	{
		auxiliaries::IOUtilities().write_map(list_of_balls, std::cout);
	}
	else
	{
		for(std::set<std::size_t>::const_iterator it=output_set_of_ball_ids.begin();it!=output_set_of_ball_ids.end();++it)
		{
			std::cout << list_of_balls[*it].first << " " << list_of_balls[*it].second << "\n";
		}
	}

	if(!output_set_of_ball_ids.empty() && !seq_output.empty())
	{
		std::ofstream foutput(seq_output.c_str(), std::ios::out);
		if(foutput.good())
		{
			foutput << SequenceUtilities::convert_residue_sequence_container_to_string(residue_sequence_vector) << "\n";
		}
	}

	if(!output_set_of_ball_ids.empty() && !pdb_output.empty())
	{
		if(pdb_output_template.empty())
		{
			std::ofstream foutput(pdb_output.c_str(), std::ios::out);
			if(foutput.good())
			{
				for(std::set<std::size_t>::const_iterator it=output_set_of_ball_ids.begin();it!=output_set_of_ball_ids.end();++it)
				{
					foutput << auxiliaries::AtomsIO::PDBWriter::write_atom_record_in_line(convert_ball_record_to_single_atom_record(list_of_balls[*it].first, list_of_balls[*it].second, pdb_output_b_factor)) << "\n";
				}
			}
		}
		else
		{
			auxiliaries::AtomsIO::PDBReader::Data pdb_file_data;
			{
				std::ifstream finput(pdb_output_template.c_str(), std::ios::in);
				pdb_file_data=auxiliaries::AtomsIO::PDBReader::read_data_from_file_stream(finput, true, true, true);
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
					for(std::set<std::size_t>::const_iterator it=output_set_of_ball_ids.begin();it!=output_set_of_ball_ids.end();++it)
					{
						output_map_of_ball_ids[list_of_balls[*it].first]=(*it);
					}
					int icount=0;
					for(std::size_t i=0;i<pdb_file_data.atom_records.size();i++)
					{
						const auxiliaries::AtomsIO::AtomRecord& atom_record=pdb_file_data.atom_records[i];
						const std::map<CRAD, std::size_t>::const_iterator ball_id_it=output_map_of_ball_ids.find(CRAD(atom_record.serial, atom_record.chainID, atom_record.resSeq, atom_record.resName, atom_record.name, atom_record.altLoc, atom_record.iCode));
						if(ball_id_it!=output_map_of_ball_ids.end())
						{
							const std::map<std::string, double>& ball_adjuncts=list_of_balls[ball_id_it->second].second.props.adjuncts;
							const std::map<std::string, double>::const_iterator temperature_factor_it=ball_adjuncts.find(pdb_output_b_factor);
							std::string& ball_line=pdb_file_data.all_lines[pdb_file_data.map_of_atom_records_to_all_lines.at(i)];
							ball_line=(temperature_factor_it!=ball_adjuncts.end()) ?
									auxiliaries::AtomsIO::PDBWriter::write_temperature_factor_to_line(ball_line, true, temperature_factor_it->second) :
									auxiliaries::AtomsIO::PDBWriter::write_temperature_factor_to_line(ball_line, false, 0);
							icount++;
						}
					}
					for(std::size_t i=0;i<pdb_file_data.all_lines.size();i++)
					{
						foutput << pdb_file_data.all_lines[i] << "\n";
					}
				}
			}
		}
	}
}
