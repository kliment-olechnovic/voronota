#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/io_utilities.h"

#include "../common/chain_residue_atom_descriptor.h"

namespace
{

double rescale_global_score(const double score, const double completeness, const double completeness_threshold)
{
	return ((completeness<completeness_threshold) ? score*completeness : score);
}

double rescale_local_score(const double score)
{
	const double a_mean=0.310;
	const double a_sd=0.097;
	const double b_mean=0.487;
	const double b_sd=0.203;
	const double d0=3.0;

	double s=(((score-a_mean)/a_sd)*b_sd+b_mean);
	s=std::max(s, 0.01);
	s=std::min(s, 1.0);
	return (d0*sqrt((1-s)/s));
}

}

void write_qa_scores_in_casp_format(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	typedef voronota::common::ChainResidueAtomDescriptor CRAD;

	const std::string name=poh.argument<std::string>("--name");
	const double global_score=poh.argument<double>("--global-score");
	const int sequence_length=poh.argument<int>("--sequence-length");
	const std::string local_scores=poh.argument<std::string>("--local-scores");
	const int wrap_size=poh.argument<int>("--wrap-size", 20);
	const double completeness_threshold=poh.argument<double>("--completeness-threshold", 0.85);
	const std::string converted_local_scores=poh.argument<std::string>("--converted-local-scores", "");

	if(name.empty() || global_score<0.0 || global_score>1.0 || sequence_length<3 || wrap_size<1 || completeness_threshold>0.99)
	{
		throw std::runtime_error("Invalid parameters.");
	}

	const std::map<CRAD, double> map_of_crad_scores=voronota::auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRAD, double> >(local_scores);

	if(map_of_crad_scores.empty())
	{
		throw std::runtime_error("No local scores.");
	}

	std::map<int, double> map_of_position_scores;
	for(std::map<CRAD, double>::const_iterator it=map_of_crad_scores.begin();it!=map_of_crad_scores.end();++it)
	{
		const int pos=it->first.resSeq;
		if(pos>=1 && pos<=sequence_length && map_of_position_scores.count(pos)==0)
		{
			map_of_position_scores[pos]=(it->second);
		}
	}

	if(map_of_position_scores.empty())
	{
		throw std::runtime_error("No local scores in required range.");
	}

	const double local_scores_completeness=static_cast<double>(map_of_position_scores.size())/static_cast<double>(sequence_length);

	std::cout << name << " ";
	std::cout.precision(3);
	std::cout << std::fixed << rescale_global_score(global_score, local_scores_completeness, completeness_threshold);

	for(int i=1;i<=sequence_length;i++)
	{
		std::cout << ((i>1 && ((i-1)%wrap_size==0)) ? "\t" : " ");
		std::map<int, double>::const_iterator it=map_of_position_scores.find(i);
		if(it!=map_of_position_scores.end())
		{
			std::cout.precision(2);
			std::cout << std::fixed << rescale_local_score(it->second);
		}
		else
		{
			std::cout << "X";
		}
	}
	std::cout << "\n";

	if(!converted_local_scores.empty())
	{
		std::ofstream foutput(converted_local_scores.c_str(), std::ios::out);
		if(foutput.good())
		{
			for(std::map<CRAD, double>::const_iterator it=map_of_crad_scores.begin();it!=map_of_crad_scores.end();++it)
			{
				foutput << (it->first) << " " << rescale_local_score(it->second) << "\n";
			}
		}
	}
}
