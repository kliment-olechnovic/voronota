#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/io_utilities.h"

#include "../common/construction_of_voromqa_score.h"

namespace
{

typedef voronota::common::ChainResidueAtomDescriptor CRAD;
typedef voronota::common::EnergyDescriptor EnergyDescriptor;

}

void score_contacts_quality(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
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

	const std::map<CRAD, EnergyDescriptor> atom_energy_descriptors=voronota::auxiliaries::IOUtilities().read_lines_to_map< std::map<CRAD, EnergyDescriptor> >(std::cin);
	if(atom_energy_descriptors.empty())
	{
		throw std::runtime_error("No input.");
	}

	const std::map<CRAD, voronota::common::NormalDistributionParameters> means_and_sds=voronota::auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRAD, voronota::common::NormalDistributionParameters> >(mean_and_sds_file);

	const std::map<CRAD, double> external_weights=voronota::auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRAD, double> >(external_weights_file);

	voronota::common::ConstructionOfVoroMQAScore::ParametersToConstructBundleOfVoroMQAQualityInformation parameters;
	parameters.default_mean=default_mean;
	parameters.default_sd=default_sd;
	parameters.mean_shift=mean_shift;

	voronota::common::ConstructionOfVoroMQAScore::BundleOfVoroMQAQualityInformation bundle;

	if(!voronota::common::ConstructionOfVoroMQAScore::construct_bundle_of_voromqa_quality_information(parameters, means_and_sds, atom_energy_descriptors, bundle))
	{
		throw std::runtime_error("Failed to calculate quality scores.");
	}

	voronota::auxiliaries::IOUtilities().write_map_to_file(bundle.atom_quality_scores, atom_scores_file);

	if(!residue_scores_file.empty())
	{
		if(smoothing_windows.size()==1)
		{
			voronota::auxiliaries::IOUtilities().write_map_to_file(bundle.residue_quality_scores(smoothing_windows.front()), residue_scores_file);
		}
		else
		{
			std::ofstream foutput(residue_scores_file.c_str(), std::ios::out);
			if(foutput.good())
			{
				std::vector< std::map<CRAD, double> > residue_quality_scores(smoothing_windows.size());
				for(std::size_t i=0;i<smoothing_windows.size();i++)
				{
					residue_quality_scores[i]=bundle.residue_quality_scores(smoothing_windows[i]);
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

	std::cout << bundle.global_quality_score(external_weights, false) << "\n";
}
