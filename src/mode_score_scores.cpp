#include <iostream>
#include <stdexcept>
#include <set>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"

namespace
{

typedef std::set< std::pair<std::string, std::string> > SetOfStringsPairs;
typedef std::map<std::string, double> MapOfNamedValues;
typedef std::map<std::string, std::pair<double, double> > MapOfNamedValuesPairs;

struct ClassificationResults
{
	unsigned long TP;
	unsigned long TN;
	unsigned long FP;
	unsigned long FN;

	ClassificationResults() : TP(0), TN(0), FP(0), FN(0)
	{
	}

	void add(const ClassificationResults& v)
	{
		TP+=v.TP;
		TN+=v.TN;
		FP+=v.FP;
		FN+=v.FN;
	}

	void add(const bool reference_outcome, const bool testable_outcome)
	{
		if(reference_outcome)
		{
			if(testable_outcome)
			{
				TP++;
			}
			else
			{
				FN++;
			}
		}
		else
		{
			if(testable_outcome)
			{
				FP++;
			}
			else
			{
				TN++;
			}
		}
	}

	double TPR() const
	{
		return (TP>0 ? static_cast<double>(TP)/static_cast<double>(TP+FN) : 0.0);
	}

	double FPR() const
	{
		return (FP>0 ? static_cast<double>(FP)/static_cast<double>(FP+TN) : 0.0);
	}

	double precision() const
	{
		return (TP>0 ? static_cast<double>(TP)/static_cast<double>(TP+FP) : 0.0);
	}

	double recall() const
	{
		return TPR();
	}
};

MapOfNamedValuesPairs merge_two_maps(const MapOfNamedValues& a, const MapOfNamedValues& b)
{
	MapOfNamedValuesPairs result;
	typename MapOfNamedValues::const_iterator a_it=a.begin();
	typename MapOfNamedValues::const_iterator b_it=b.begin();
	while(a_it!=a.end() && b_it!=b.end())
	{
		if(a_it->first==b_it->first)
		{
			result.insert(result.end(), std::make_pair(a_it->first, std::make_pair(a_it->second, b_it->second)));
			++a_it;
			++b_it;
		}
		else if(a_it->first<b_it->first)
		{
			++a_it;
		}
		else if(b_it->first<a_it->first)
		{
			++b_it;
		}
	}
	return result;
}

ClassificationResults calc_classification_results(const MapOfNamedValuesPairs& map_of_pairs, const double reference_threshold, const double testable_threshold)
{
	ClassificationResults result;
	for(MapOfNamedValuesPairs::const_iterator it=map_of_pairs.begin();it!=map_of_pairs.end();++it)
	{
		const bool reference_outcome=(it->second.first>reference_threshold);
		const bool testable_outcome=(it->second.second>testable_threshold);
		result.add(reference_outcome, testable_outcome);
	}
	return result;
}

void update_classification_results_map(
		const MapOfNamedValuesPairs& map_of_pairs,
		const double reference_threshold,
		const double testable_step,
		std::map<double, ClassificationResults>& classification_results_map)
{
	for(double testable_threshold=0.0;testable_threshold<=1.0;testable_threshold+=testable_step)
	{
		classification_results_map[testable_threshold].add(calc_classification_results(map_of_pairs, reference_threshold, testable_threshold));
	}
}

std::set< std::pair<double, double> > calc_ROC_curve_coordinates(const std::map<double, ClassificationResults>& classification_results_map)
{
	std::set< std::pair<double, double> > result;
	for(std::map<double, ClassificationResults>::const_iterator it=classification_results_map.begin();it!=classification_results_map.end();++it)
	{
		result.insert(result.end(), std::make_pair(it->second.FPR(), it->second.TPR()));
	}
	return result;
}

std::set< std::pair<double, double> > calc_PR_curve_coordinates(const std::map<double, ClassificationResults>& classification_results_map)
{
	std::set< std::pair<double, double> > result;
	for(std::map<double, ClassificationResults>::const_iterator it=classification_results_map.begin();it!=classification_results_map.end();++it)
	{
		result.insert(result.end(), std::make_pair(it->second.recall(), it->second.precision()));
	}
	return result;
}

double calc_AUC(const std::set< std::pair<double, double> >& curve)
{
	double result=0.0;
	std::set< std::pair<double, double> >::const_iterator it=curve.begin();
	std::set< std::pair<double, double> >::const_iterator next_it=it;
	++next_it;
	while(next_it!=curve.end())
	{
		result+=((next_it->first)-(it->first))*((next_it->second)+(it->second))*0.5;
		++it;
		++next_it;
	}
	return result;
}

}

void score_scores(const auxiliaries::ProgramOptionsHandler& poh)
{
	{
		typedef auxiliaries::ProgramOptionsHandler::OptionDescription OD;
		std::vector<OD> list_of_option_descriptions;
		list_of_option_descriptions.push_back(OD("--reference-threshold", "number", "reference scores classification threshold"));
		list_of_option_descriptions.push_back(OD("--testable-step", "number", "testable scores threshold step"));
		list_of_option_descriptions.push_back(OD("--ROC-curve-file", "string", "file path to output ROC curve"));
		list_of_option_descriptions.push_back(OD("--PR-curve-file", "string", "file path to output PR curve"));
		if(!poh.assert(list_of_option_descriptions, false))
		{
			std::cerr << "stdin   <-  pairs of reference and testable scores files\n";
			std::cerr << "stdout  ->  global results\n";
			return;
		}
	}

	const double reference_threshold=poh.argument<double>("--reference-threshold", 0.5);
	const double testable_step=poh.argument<double>("--testable-step", 0.02);
	const std::string ROC_curve_file=poh.argument<std::string>("--ROC-curve-file", "");
	const std::string PR_curve_file=poh.argument<std::string>("--PR-curve-file", "");

	const SetOfStringsPairs input_file_pairs=auxiliaries::IOUtilities().read_lines_to_map<SetOfStringsPairs>(std::cin);
	if(input_file_pairs.empty())
	{
		throw std::runtime_error("No input.");
	}

	std::map<double, ClassificationResults> classification_results_map;
	for(SetOfStringsPairs::const_iterator it=input_file_pairs.begin();it!=input_file_pairs.end();++it)
	{
		const MapOfNamedValues reference_scores_map=auxiliaries::IOUtilities().read_file_lines_to_map<MapOfNamedValues>(it->first);
		const MapOfNamedValues testable_scores_map=auxiliaries::IOUtilities().read_file_lines_to_map<MapOfNamedValues>(it->second);
		const MapOfNamedValuesPairs merged_scores_map=merge_two_maps(reference_scores_map, testable_scores_map);
		if(!merged_scores_map.empty())
		{
			update_classification_results_map(merged_scores_map, reference_threshold, testable_step, classification_results_map);
		}
	}

	const std::set< std::pair<double, double> > ROC_curve_coordinates=calc_ROC_curve_coordinates(classification_results_map);
	const std::set< std::pair<double, double> > PR_curve_coordinates=calc_PR_curve_coordinates(classification_results_map);

	auxiliaries::IOUtilities().write_map_to_file(ROC_curve_coordinates, ROC_curve_file);
	auxiliaries::IOUtilities().write_map_to_file(PR_curve_coordinates, PR_curve_file);

	std::cout << "ROC_AUC " << calc_AUC(ROC_curve_coordinates) << "\n";
	std::cout << "PR_AUC " << calc_AUC(PR_curve_coordinates) << "\n";
}
