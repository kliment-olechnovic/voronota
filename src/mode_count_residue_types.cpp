#include <iostream>
#include <stdexcept>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"

#include "modescommon/sequence_utilities.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;

template<typename T>
inline bool read_and_accumulate_to_map_of_counts(std::istream& input, std::map<T, long>& map_of_counts)
{
	T crad;
	long count;
	input >> crad >> count;
	if(!input.fail() && count>0)
	{
		map_of_counts[crad]+=count;
		return true;
	}
	return false;
}

template<typename T>
long sum_counts(const std::map<T, long>& map_of_counts)
{
	long sum=0;
	for(typename std::map<T, long>::const_iterator it=map_of_counts.begin();it!=map_of_counts.end();++it)
	{
		sum+=it->second;
	}
	return sum;
}

}

void count_residue_types(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		ods.push_back(OD("--input-pre-counted", "", "flag to input types with counts"));
		ods.push_back(OD("--input-sequence", "", "flag to input sequences"));
		if(!poh.assert(ods, false))
		{
			poh.print_io_description("stdin", true, false,
					"list of ids, or list of types with counts, or sequence\n(default mode line format: 'id')\n(pre-counted mode line format: 'type count')\n(sequence mode format: plain or FASTA)");
			poh.print_io_description("stdout", false, true, "list of types with counts (line format: 'type count')");
			return;
		}
	}

	const bool input_pre_counted=poh.contains_option("--input-pre-counted");
	const bool input_sequence=poh.contains_option("--input-sequence");

	std::map<CRAD, long> result;

	if(input_pre_counted)
	{
		auxiliaries::IOUtilities().read_lines_to_container(std::cin, read_and_accumulate_to_map_of_counts<CRAD>, result);
	}
	else if(input_sequence)
	{
		const std::string sequence=SequenceUtilities::read_sequence_from_stream(std::cin);
		for(std::size_t i=0;i<sequence.size();i++)
		{
			CRAD crad;
			crad.resName=auxiliaries::ResidueLettersCoding::convert_residue_code_small_to_big(sequence.substr(i, 1));
			result[crad]++;
		}
	}
	else
	{
		const std::vector<CRAD> crads=auxiliaries::IOUtilities().read_lines_to_set< std::vector<CRAD> >(std::cin);
		std::set<CRAD> residue_crads;
		for(std::size_t i=0;i<crads.size();i++)
		{
			residue_crads.insert(crads[i].without_atom());
		}
		for(std::set<CRAD>::const_iterator it=residue_crads.begin();it!=residue_crads.end();++it)
		{
			CRAD crad;
			crad.resName=it->resName;
			result[crad]++;
		}
	}

	auxiliaries::IOUtilities().write_map(result, std::cout);
}

void count_residue_types_coefficients(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> ods;
		ods.push_back(OD("--first-counts-file", "string", "file to input first set of counts of residue types", true));
		ods.push_back(OD("--second-counts-file", "string", "file to input second set of counts of residue types", true));
		if(!poh.assert(ods, false))
		{
			poh.print_io_description("stdout", false, true, "list of types with coefficients (line format: 'type coefficient')");
			return;
		}
	}

	const std::string first_counts_file=poh.argument<std::string>("--first-counts-file");
	const std::string second_counts_file=poh.argument<std::string>("--second-counts-file");

	std::map<CRAD, long> first_counts;
	std::map<CRAD, long> second_counts;

	auxiliaries::IOUtilities().read_file_lines_to_container(first_counts_file, read_and_accumulate_to_map_of_counts<CRAD>, first_counts);
	auxiliaries::IOUtilities().read_file_lines_to_container(second_counts_file, read_and_accumulate_to_map_of_counts<CRAD>, second_counts);

	const long first_sum=sum_counts(first_counts);
	const long second_sum=sum_counts(second_counts);

	if(first_sum<=0 || second_sum<=0)
	{
		throw std::runtime_error("Invalid input.");
	}

	std::map<CRAD, double> result;
	for(std::map<CRAD, long>::const_iterator it=first_counts.begin();it!=first_counts.end();++it)
	{
		double first_r=static_cast<double>(it->second)/static_cast<double>(first_sum);
		double second_r=static_cast<double>(second_counts[it->first])/static_cast<double>(second_sum);
		result[it->first]=(second_r/first_r);
	}

	auxiliaries::IOUtilities().write_map(result, std::cout);
}
