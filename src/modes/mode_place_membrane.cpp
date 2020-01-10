#include "../apollota/subdivided_icosahedron.h"

#include "../auxiliaries/program_options_handler.h"

#include "../common/chain_residue_atom_descriptor.h"
#include "../common/ball_value.h"

namespace
{

typedef voronota::common::ChainResidueAtomDescriptor CRAD;

struct PointAndScore
{
	voronota::apollota::SimplePoint point;
	double score;

	PointAndScore(const voronota::apollota::SimplePoint& point, const double score) : point(point), score(score)
	{
	}
};

struct MembranePlacement
{
	double width;
	double width_extension;
	double score;
	voronota::apollota::SimplePoint position;
	voronota::apollota::SimplePoint normal;

	MembranePlacement(
			const double width,
			const double width_extension,
			const double score,
			const voronota::apollota::SimplePoint& position,
			const voronota::apollota::SimplePoint& normal) :
				width(width),
				width_extension(width_extension),
				score(score),
				position(position),
				normal(normal)
	{
	}
};

struct ScoredShift
{
	double score;
	double shift;

	ScoredShift() : score(0), shift(0)
	{
	}
};

ScoredShift estimate_best_scored_shift(
		const std::vector<PointAndScore>& points_and_scores,
		const voronota::apollota::SimplePoint& direction,
		const double width,
		const double width_extension,
		std::vector< std::pair<double, double> >& buffer_for_projections)
{
	const double length_step=0.5;
	static std::vector< std::pair<int, double> > bins(1000);

	for(std::size_t i=0;i<points_and_scores.size();i++)
	{
		buffer_for_projections[i].first=points_and_scores[i].point*direction;
		buffer_for_projections[i].second=points_and_scores[i].score;
	}
	std::sort(buffer_for_projections.begin(), buffer_for_projections.end());

	const double full_length=(buffer_for_projections.back().first-buffer_for_projections.front().first);
	bins.resize(static_cast<std::size_t>(ceil(full_length/length_step)));
	const int bins_n=static_cast<int>(bins.size());
	const int width_half_bins_n=static_cast<int>(floor(width*0.5/length_step+0.5));
	const int extended_width_half_bins_n=static_cast<int>(floor((width+width_extension)*0.5/length_step+0.5));

	for(std::size_t i=0;i<bins.size();i++)
	{
		bins[i].first=0;
		bins[i].second=0.0;
	}

	for(std::size_t i=0;i<buffer_for_projections.size();i++)
	{
		int bin_pos=static_cast<int>(floor((buffer_for_projections[i].first-buffer_for_projections.front().first)/length_step));
		bin_pos=std::min(std::max(bin_pos, 0), bins_n-1);
		bins[bin_pos].first++;
		bins[bin_pos].second+=buffer_for_projections[i].second;
	}

	int count_of_all_bin_values=0;
	double sum_of_all_bin_values=0.0;
	for(std::size_t i=0;i<bins.size();i++)
	{
		if(bins[i].first>0)
		{
			count_of_all_bin_values+=bins[i].first;
			sum_of_all_bin_values+=bins[i].second;
		}
	}

	double best_score=0.0;
	int best_score_i=0;
	bool best_initialized=false;
	for(int i=0;i<bins_n;i++)
	{
		int count_in=0;
		double sum_in=0.0;
		for(int j=(0-width_half_bins_n);j<=width_half_bins_n;j++)
		{
			const int p=(i+j);
			if(p>=0 && p<bins_n)
			{
				count_in+=bins[p].first;
				sum_in+=bins[p].second;
			}
		}

		int count_in_extension=0;
		double sum_in_extension=0.0;
		for(int j=(0-extended_width_half_bins_n);j<(0-width_half_bins_n);j++)
		{
			const int p=(i+j);
			if(p>=0 && p<bins_n)
			{
				count_in_extension+=bins[p].first;
				sum_in_extension+=bins[p].second;
			}
		}
		for(int j=(width_half_bins_n+1);j<=extended_width_half_bins_n;j++)
		{
			const int p=(i+j);
			if(p>=0 && p<bins_n)
			{
				count_in_extension+=bins[p].first;
				sum_in_extension+=bins[p].second;
			}
		}

		const double score=
				static_cast<double>(count_of_all_bin_values-count_in-count_in_extension)
				-(sum_of_all_bin_values-sum_in-sum_in_extension)
				+sum_in
				+(static_cast<double>(count_in_extension)*0.5);
		if(!best_initialized || best_score<score)
		{
			best_score=score;
			best_score_i=i;
			best_initialized=true;
		}
	}

	ScoredShift result;
	result.score=best_score;
	result.shift=buffer_for_projections.front().first+(static_cast<double>(best_score_i)*(full_length/static_cast<double>(bins_n)));
	return result;
}

MembranePlacement estimate_translated_membrane_placement(const std::vector<PointAndScore>& points_and_scores, const double width, const double width_extension)
{
	std::vector< std::pair<double, double> > buffer_for_projections(points_and_scores.size());
	voronota::apollota::SubdividedIcosahedron sih(3);
	sih.fit_into_sphere(voronota::apollota::SimplePoint(0, 0, 0), 1);
	std::size_t best_id=0;
	ScoredShift best_scored_shift;
	double last_cycle_best_score=0.0;
	int number_of_cycles=0;
	int number_of_checks=0;
	while(number_of_cycles<2 || (best_scored_shift.score-last_cycle_best_score)>0.0001 || number_of_cycles>10000)
	{
		std::size_t start_id=0;
		if(number_of_cycles>0)
		{
			start_id=sih.vertices().size();
			sih.grow(best_id, true);
			last_cycle_best_score=best_scored_shift.score;
		}
		for(std::size_t i=start_id;i<sih.vertices().size();i++)
		{
			const ScoredShift scored_shift=estimate_best_scored_shift(points_and_scores, sih.vertices()[i].unit(), width, width_extension, buffer_for_projections);
			if(number_of_checks==0 || scored_shift.score>best_scored_shift.score)
			{
				best_id=i;
				best_scored_shift=scored_shift;
			}
			number_of_checks++;
		}
		number_of_cycles++;
	}
	const voronota::apollota::SimplePoint best_direction=sih.vertices()[best_id].unit();
	return MembranePlacement(width, width_extension, best_scored_shift.score/static_cast<double>(points_and_scores.size()), best_direction*best_scored_shift.shift, best_direction);
}

MembranePlacement estimate_membrane_placement(const std::vector<PointAndScore>& points_and_scores, const double width, const double width_extension, const bool optimize_width, const bool optimize_width_extension)
{
	voronota::apollota::SimplePoint original_center(0, 0, 0);
	for(std::size_t i=0;i<points_and_scores.size();i++)
	{
		original_center=(original_center+points_and_scores[i].point);
	}
	original_center=original_center*(1.0/static_cast<double>(points_and_scores.size()));

	std::vector<PointAndScore> translated_points_and_scores=points_and_scores;
	for(std::size_t i=0;i<points_and_scores.size();i++)
	{
		translated_points_and_scores[i].point=(points_and_scores[i].point-original_center);
	}

	std::vector<MembranePlacement> best_result;
	{
		const double width_step=5.0;
		const double width_extension_step=5.0;
		const double width_start=(optimize_width ? 10.0 : width);
		const double width_extension_start=(optimize_width_extension ? 0.0 : width_extension);
		for(double current_width=width_start;current_width<=width;current_width+=width_step)
		{
			for(double current_width_extension=width_extension_start;current_width_extension<=width_extension;current_width_extension+=width_extension_step)
			{
				const MembranePlacement translated_membrane_placement=estimate_translated_membrane_placement(translated_points_and_scores, current_width, current_width_extension);
				if(best_result.empty())
				{
					best_result.push_back(translated_membrane_placement);
				}
				else if(best_result[0].score<translated_membrane_placement.score)
				{
					best_result[0]=translated_membrane_placement;
				}
			}
		}
	}
	const MembranePlacement translated_membrane_placement=(best_result.empty() ? estimate_translated_membrane_placement(translated_points_and_scores, width, width_extension) : best_result[0]);
	return MembranePlacement(translated_membrane_placement.width, translated_membrane_placement.width_extension, translated_membrane_placement.score, original_center+translated_membrane_placement.position, translated_membrane_placement.normal);
}

}

void place_membrane(const voronota::auxiliaries::ProgramOptionsHandler& poh)
{
	voronota::auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'annotation x y z r tags adjuncts')");
	pohw.describe_io("stdout", false, true, "list of balls (line format: 'annotation x y z r tags adjuncts')");

	const std::string scores_file=poh.argument<std::string>(pohw.describe_option("--scores-file", "string", "file path to input atom scores", true), "");
	const double membrane_width=poh.argument<double>(pohw.describe_option("--membrane-width", "number", "membrane width", true));
	const double membrane_width_extension=poh.argument<double>(pohw.describe_option("--membrane-width-extension", "number", "membrane width extension"), 0.0);
	const bool optimize_width=poh.contains_option(pohw.describe_option("--optimize-width", "", "flag to optimize width"));
	const bool optimize_width_extension=poh.contains_option(pohw.describe_option("--optimize-width-extension", "", "flag to optimize width extension"));
	const std::string placement_output=poh.argument<std::string>(pohw.describe_option("--placement-output", "string", "file path to output placement results"), "");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	if(membrane_width<=1.0)
	{
		throw std::runtime_error("Invalid membrane width.");
	}

	std::vector< std::pair<CRAD, voronota::common::BallValue> > list_of_balls;
	voronota::auxiliaries::IOUtilities().read_lines_to_map(std::cin, list_of_balls);
	if(list_of_balls.empty())
	{
		throw std::runtime_error("No input balls.");
	}

	const std::map<CRAD, double> map_of_scores=voronota::auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRAD, double> >(scores_file);
	if(map_of_scores.empty())
	{
		throw std::runtime_error("No input scores.");
	}

	std::vector<PointAndScore> points_and_scores;
	points_and_scores.reserve(map_of_scores.size());
	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		std::map<CRAD, double>::const_iterator it=map_of_scores.find(list_of_balls[i].first);
		if(it!=map_of_scores.end())
		{
			points_and_scores.push_back(PointAndScore(voronota::apollota::SimplePoint(list_of_balls[i].second), it->second));
		}
	}

	std::ostringstream logstream;

	const MembranePlacement membrane_placement=estimate_membrane_placement(points_and_scores, membrane_width, membrane_width_extension, optimize_width, optimize_width_extension);

	logstream << "width " << membrane_placement.width << "\n";
	logstream << "width_extension " << membrane_placement.width_extension << "\n";
	logstream << "score " << membrane_placement.score << "\n";
	logstream << "position " << membrane_placement.position.x << " " << membrane_placement.position.y << " " << membrane_placement.position.z << "\n";
	logstream << "direction " << membrane_placement.normal.x << " " << membrane_placement.normal.y << " " << membrane_placement.normal.z << "\n";

	if(!placement_output.empty())
	{
		std::ofstream foutput(placement_output.c_str(), std::ios::out);
		if(foutput.good())
		{
			foutput << logstream.str();
		}
	}

	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		if(fabs((voronota::apollota::SimplePoint(list_of_balls[i].second)-membrane_placement.position)*membrane_placement.normal.unit())<(membrane_placement.width*0.5))
		{
			list_of_balls[i].second.props.update_adjuncts("membrane=1");
		}
		else
		{
			list_of_balls[i].second.props.update_adjuncts("membrane=0");
		}
	}

	voronota::auxiliaries::IOUtilities().write_map(list_of_balls, std::cout);
}
