#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cmath>

#include "modescommon/assert_options.h"
#include "modescommon/handle_annotations.h"
#include "modescommon/handle_mappings.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;

struct CADDescriptor
{
	double target_area_sum;
	double model_area_sum;
	double raw_differences_sum;
	double constrained_differences_sum;

	CADDescriptor() : target_area_sum(0), model_area_sum(0), raw_differences_sum(), constrained_differences_sum(0)
	{
	}

	void add(const double target_area, const double model_area)
	{
		target_area_sum+=target_area;
		model_area_sum+=model_area;
		raw_differences_sum+=fabs(target_area-model_area);
		constrained_differences_sum+=std::min(fabs(target_area-model_area), target_area);
	}

	void add(const CADDescriptor& cadd)
	{
		target_area_sum+=cadd.target_area_sum;
		model_area_sum+=cadd.target_area_sum;
		raw_differences_sum+=cadd.raw_differences_sum;
		constrained_differences_sum+=cadd.constrained_differences_sum;
	}

	double score() const
	{
		return (1.0-(constrained_differences_sum/target_area_sum));
	}
};

inline void print_score(const std::string& name, const CADDescriptor& cadd, std::ostream& output)
{
	output << name << " ";
	output << cadd.score() << " " << cadd.target_area_sum << " " << cadd.model_area_sum << " " << cadd.raw_differences_sum << " " << cadd.constrained_differences_sum << "\n";
}

void print_pair_scores_to_file(const std::map< std::pair<CRAD, CRAD>, CADDescriptor >& map_of_pair_cad_descriptors, const std::string& filename)
{
	if(!filename.empty())
	{
		std::ofstream foutput(filename.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map< std::pair<CRAD, CRAD>, CADDescriptor >::const_iterator it=map_of_pair_cad_descriptors.begin();it!=map_of_pair_cad_descriptors.end();++it)
			{
				print_score(it->first.first.str()+" "+it->first.second.str(), it->second, foutput);
			}
		}
	}
}

void print_single_scores_to_file(const std::map<CRAD, CADDescriptor>& map_of_single_cad_descriptors, const std::string& filename)
{
	if(!filename.empty())
	{
		std::ofstream foutput(filename.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map<CRAD, CADDescriptor>::const_iterator it=map_of_single_cad_descriptors.begin();it!=map_of_single_cad_descriptors.end();++it)
			{
				print_score(it->first.str(), it->second, foutput);
			}
		}
	}
}

}

void compare_contacts(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--target-contacts-file", "string", "file path to input target contacts", true));
		list_of_option_descriptions.push_back(OD("--inter-atom-scores-file", "string", "file path to output inter-atom scores"));
		list_of_option_descriptions.push_back(OD("--inter-residue-scores-file", "string", "file path to output inter-residue scores"));
		list_of_option_descriptions.push_back(OD("--atom-scores-file", "string", "file path to output atom scores"));
		list_of_option_descriptions.push_back(OD("--residue-scores-file", "string", "file path to output residue scores"));
		list_of_option_descriptions.push_back(OD("--depth", "number", "neighborhood normalization depth"));
		if(!modescommon::assert_options(list_of_option_descriptions, poh, false))
		{
			std::cerr << "stdin   <-  list of contacts (line format: 'annotation1 annotation2 area')\n";
			std::cerr << "stdout  ->  two lines of global scores (atom-level and residue-level)\n";
			return;
		}
	}

	const std::string target_contacts_file=poh.argument<std::string>("--target-contacts-file");
	const std::string inter_atom_scores_file=poh.argument<std::string>("--inter-atom-scores-file", "");
	const std::string inter_residue_scores_file=poh.argument<std::string>("--inter-residue-scores-file", "");
	const std::string atom_scores_file=poh.argument<std::string>("--atom-scores-file", "");
	const std::string residue_scores_file=poh.argument<std::string>("--residue-scores-file", "");
//	const int depth=poh.argument<int>("--depth", 0);
}
