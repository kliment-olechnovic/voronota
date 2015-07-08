#include <iostream>
#include <stdexcept>
#include <set>
#include <cmath>

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"

#include "modescommon/generic_utilities.h"

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
		return ((TP+FN)>0 ? static_cast<double>(TP)/static_cast<double>(TP+FN) : 0.0);
	}

	double FPR() const
	{
		return ((FP+TN)>0 ? static_cast<double>(FP)/static_cast<double>(FP+TN) : 0.0);
	}

	double precision() const
	{
		return ((TP+FP)>0 ? static_cast<double>(TP)/static_cast<double>(TP+FP) : 1.0);
	}

	double recall() const
	{
		return TPR();
	}

	double accuracy() const
	{
		unsigned long d=(TP+TN+FP+FN);
		return (d>0 ? static_cast<double>(TP+TN)/static_cast<double>(d) : 0.0);
	}

	double MCC() const
	{
		unsigned long c=((TP*TN)-(FP*FN));
		unsigned long d1=(TP+FP);
		unsigned long d2=(TP+FN);
		unsigned long d3=(TN+FP);
		unsigned long d4=(TN+FN);
		if(d1>0 && d2>0 && d3>0 && d4>0)
		{
			const double ratio=(c*(1.0/sqrt(d1))*(1.0/sqrt(d2))*(1.0/sqrt(d3))*(1.0/sqrt(d4)));
			return (fabs(ratio)<=1.0 ? ratio : 0.0);
		}
		else
		{
			return 0.0;
		}
	}
};

inline std::ostream& operator<<(std::ostream& output, const ClassificationResults& v)
{
	output << v.TP << " " << v.TN << " " << v.FP << " " << v.FN;
	return output;
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
	std::pair<double, double> min_max_testable_value(0.0, 0.0);
	for(MapOfNamedValuesPairs::const_iterator it=map_of_pairs.begin();it!=map_of_pairs.end();++it)
	{
		if(it==map_of_pairs.begin() || it->second.second<min_max_testable_value.first)
		{
			min_max_testable_value.first=it->second.second;
		}
		if(it==map_of_pairs.begin() || it->second.second>min_max_testable_value.second)
		{
			min_max_testable_value.second=it->second.second;
		}
	}
	std::pair<long, long> min_max_integers(static_cast<long>(floor(min_max_testable_value.first/testable_step)), static_cast<long>(ceil(min_max_testable_value.second/testable_step)));
	for(long i=min_max_integers.first;i<=min_max_integers.second;i++)
	{
		const double testable_threshold=static_cast<double>(i)*testable_step;
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

std::pair<double, double> calc_best_accuracy(const std::map<double, ClassificationResults>& classification_results_map)
{
	std::pair<double, double> result(0.0, 0.0);
	for(std::map<double, ClassificationResults>::const_iterator it=classification_results_map.begin();it!=classification_results_map.end();++it)
	{
		if(it==classification_results_map.begin() || it->second.accuracy()>result.second)
		{
			result=std::make_pair(it->first, it->second.accuracy());
		}
	}
	return result;
}

std::pair<double, double> calc_best_MCC(const std::map<double, ClassificationResults>& classification_results_map)
{
	std::pair<double, double> result(0.0, 0.0);
	for(std::map<double, ClassificationResults>::const_iterator it=classification_results_map.begin();it!=classification_results_map.end();++it)
	{
		if(it==classification_results_map.begin() || it->second.MCC()>result.second)
		{
			result=std::make_pair(it->first, it->second.MCC());
		}
	}
	return result;
}

}

void score_scores(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "pairs of reference and testable scores files");
	pohw.describe_io("stdout", false, true, "global results");

	const double reference_threshold=poh.argument<double>(pohw.describe_option("--reference-threshold", "number", "reference scores classification threshold"), 0.5);
	const double testable_step=poh.argument<double>(pohw.describe_option("--testable-step", "number", "testable scores threshold step"), 0.02);
	const std::string outcomes_file=poh.argument<std::string>(pohw.describe_option("--outcomes-file", "string", "file path to output lines of 'threshold TP TN FP FN'"), "");
	const std::string ROC_curve_file=poh.argument<std::string>(pohw.describe_option("--ROC-curve-file", "string", "file path to output ROC curve"), "");
	const std::string PR_curve_file=poh.argument<std::string>(pohw.describe_option("--PR-curve-file", "string", "file path to output PR curve"), "");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

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
		const MapOfNamedValuesPairs merged_scores_map=GenericUtilities::merge_two_maps(reference_scores_map, testable_scores_map);
		if(!merged_scores_map.empty())
		{
			update_classification_results_map(merged_scores_map, reference_threshold, testable_step, classification_results_map);
		}
	}

	const std::set< std::pair<double, double> > ROC_curve_coordinates=calc_ROC_curve_coordinates(classification_results_map);
	const std::set< std::pair<double, double> > PR_curve_coordinates=calc_PR_curve_coordinates(classification_results_map);

	const std::pair<double, double> best_accuracy=calc_best_accuracy(classification_results_map);
	const std::pair<double, double> best_MCC=calc_best_MCC(classification_results_map);

	auxiliaries::IOUtilities().write_map_to_file(classification_results_map, outcomes_file);
	auxiliaries::IOUtilities().write_map_to_file(ROC_curve_coordinates, ROC_curve_file);
	auxiliaries::IOUtilities().write_map_to_file(PR_curve_coordinates, PR_curve_file);

	std::cout << "ROC_AUC " << calc_AUC(ROC_curve_coordinates) << "\n";
	std::cout << "PR_AUC " << calc_AUC(PR_curve_coordinates) << "\n";
	std::cout << "best_accuracy_threshold " << best_accuracy.first << "\n";
	std::cout << "best_accuracy " << best_accuracy.second << "\n";
	std::cout << "best_MCC_threshold " << best_MCC.first << "\n";
	std::cout << "best_MCC " << best_MCC.second << "\n";
	std::cout << "input_file_pairs_count " << input_file_pairs.size() << "\n";
}
