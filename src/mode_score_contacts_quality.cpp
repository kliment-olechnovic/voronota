#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"

#include "modescommon/contacts_scoring_utilities.h"
#include "modescommon/statistics_utilities.h"

namespace
{

std::map<CRAD, double> average_atom_scores_by_residue(const std::map<CRAD, double>& atom_scores)
{
	std::map<CRAD, std::pair<int, double> > summed_scores;
	for(std::map<CRAD, double>::const_iterator it=atom_scores.begin();it!=atom_scores.end();++it)
	{
		std::pair<int, double>& value=summed_scores[it->first.without_atom()];
		value.first++;
		value.second+=it->second;
	}
	std::map<CRAD, double> average_scores;
	for(std::map<CRAD, std::pair<int, double> >::const_iterator it=summed_scores.begin();it!=summed_scores.end();++it)
	{
		const std::pair<int, double>& value=it->second;
		average_scores[it->first]=((value.first>0) ? (value.second/static_cast<double>(value.first)) : 0.0);
	}
	return average_scores;
}

}

void score_contacts_quality(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of atom energy descriptors");
	pohw.describe_io("stdout", false, true, "weighted average local score");

	const double default_mean=poh.argument<double>(pohw.describe_option("--default-mean", "number", "default mean parameter"), 0.4);
	const double default_sd=poh.argument<double>(pohw.describe_option("--default-sd", "number", "default standard deviation parameter"), 0.3);
	const std::string mean_and_sds_file=poh.argument<std::string>(pohw.describe_option("--means-and-sds-file", "string", "file path to input atomic mean and sd parameters"), "");
	const double mean_shift=poh.argument<double>(pohw.describe_option("--mean-shift", "number", "mean shift in standard deviations"), 0.0);
	const std::string external_weights_file=poh.argument<std::string>(pohw.describe_option("--external-weights-file", "string", "file path to input external weights for global scoring"), "");
	const std::vector<unsigned int> smoothing_windows=poh.argument_vector<unsigned int>(pohw.describe_option("--smoothing-window", "number", "window to smooth residue quality scores along sequence"), ',', std::vector<unsigned int>(1, 0));
	const std::string atom_scores_file=poh.argument<std::string>(pohw.describe_option("--atom-scores-file", "string", "file path to output atom scores"), "");
	const std::string residue_scores_file=poh.argument<std::string>(pohw.describe_option("--residue-scores-file", "string", "file path to output residue scores"), "");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	const std::map<CRAD, EnergyDescriptor> atom_energy_descriptors=auxiliaries::IOUtilities().read_lines_to_map< std::map<CRAD, EnergyDescriptor> >(std::cin);
	if(atom_energy_descriptors.empty())
	{
		throw std::runtime_error("No input.");
	}

	const std::map<CRAD, NormalDistributionParameters> means_and_sds=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRAD, NormalDistributionParameters> >(mean_and_sds_file);

	const std::map<CRAD, double> external_weights=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRAD, double> >(external_weights_file);

	std::map<CRAD, double> atom_quality_scores;
	double sum_of_weighted_scores=0.0;
	double sum_of_weights=0.0;
	for(std::map<CRAD, EnergyDescriptor>::const_iterator it=atom_energy_descriptors.begin();it!=atom_energy_descriptors.end();++it)
	{
		const CRAD& crad=it->first;
		const EnergyDescriptor& ed=it->second;
		if(ed.total_area>0.0)
		{
			const double actuality_score=(1.0-(ed.strange_area/ed.total_area));
			const double normalized_energy=(ed.energy/ed.total_area);
			std::map<CRAD, NormalDistributionParameters>::const_iterator mean_and_sd_it=means_and_sds.find(generalize_crad(crad));
			const double mean=(mean_and_sd_it!=means_and_sds.end() ? mean_and_sd_it->second.mean : default_mean);
			const double sd=(mean_and_sd_it!=means_and_sds.end() ? mean_and_sd_it->second.sd : default_sd);
			const double adjusted_normalized_energy=((normalized_energy-mean)/sd);
			const double energy_score=(1.0-(0.5*(1.0+erf((adjusted_normalized_energy-mean_shift)/sqrt(2.0)))));
			const double unweighted_quality_score=(energy_score*actuality_score);
			atom_quality_scores[crad]=unweighted_quality_score;
			{
				std::map<CRAD, double>::const_iterator external_weights_it=external_weights.find(crad);
				const double external_weight=(external_weights_it!=external_weights.end() ? external_weights_it->second : 1.0);
				sum_of_weighted_scores+=(unweighted_quality_score*external_weight);
				sum_of_weights+=external_weight;
			}
		}
		else
		{
			atom_quality_scores[crad]=0.0;
			sum_of_weights+=1.0;
		}
	}
	auxiliaries::IOUtilities().write_map_to_file(atom_quality_scores, atom_scores_file);

	if(!residue_scores_file.empty())
	{
		const std::map<CRAD, double> raw_residue_quality_scores=average_atom_scores_by_residue(atom_quality_scores);
		if(smoothing_windows.empty() || (smoothing_windows.size()==1 && smoothing_windows.front()==0))
		{
			auxiliaries::IOUtilities().write_map_to_file(raw_residue_quality_scores, residue_scores_file);
		}
		else if(smoothing_windows.size()==1)
		{
			auxiliaries::IOUtilities().write_map_to_file(auxiliaries::ChainResidueAtomDescriptorsSequenceOperations::smooth_residue_scores_along_sequence(raw_residue_quality_scores, smoothing_windows.front()), residue_scores_file);
		}
		else
		{
			std::ofstream foutput(residue_scores_file.c_str(), std::ios::out);
			if(foutput.good())
			{
				std::vector< std::map<CRAD, double> > residue_quality_scores(smoothing_windows.size());
				for(std::size_t i=0;i<smoothing_windows.size();i++)
				{
					residue_quality_scores[i]=auxiliaries::ChainResidueAtomDescriptorsSequenceOperations::smooth_residue_scores_along_sequence(raw_residue_quality_scores, smoothing_windows[i]);
				}
				const std::map<CRAD, double>& axis=residue_quality_scores.front();
				for(std::map<CRAD, double>::const_iterator it=axis.begin();it!=axis.end();++it)
				{
					foutput << (it->first);
					for(std::size_t i=0;i<residue_quality_scores.size();i++)
					{
						foutput << " " << residue_quality_scores[i][it->first];
					}
					foutput << "\n";
				}
			}
		}
	}

	std::cout << (sum_of_weights>0.0 ? sum_of_weighted_scores/sum_of_weights : 0.0) << "\n";
}
