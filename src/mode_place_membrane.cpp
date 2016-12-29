#include "apollota/subdivided_icosahedron.h"

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

#include "modescommon/ball_value.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;

struct PointAndScore
{
	apollota::SimplePoint point;
	double score;

	PointAndScore(const apollota::SimplePoint& point, const double score) : point(point), score(score)
	{
	}
};

struct MembranePlacement
{
	apollota::SimplePoint position;
	apollota::SimplePoint normal;

	MembranePlacement(const apollota::SimplePoint& position, const apollota::SimplePoint& normal) : position(position), normal(normal)
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
		const apollota::SimplePoint& direction,
		const double width,
		std::vector< std::pair<double, std::size_t> >& buffer_for_projections)
{
	for(std::size_t i=0;i<points_and_scores.size();i++)
	{
		buffer_for_projections[i].first=points_and_scores[i].point*direction;
		buffer_for_projections[i].second=i;
	}
	std::sort(buffer_for_projections.begin(), buffer_for_projections.end());
	ScoredShift best_scored_shift;
	ScoredShift current_scored_shift;
	std::size_t i_end=0;
	for(std::size_t i=0;i<buffer_for_projections.size() && i_end<buffer_for_projections.size();i++)
	{
		if(i==0)
		{
			current_scored_shift.shift=buffer_for_projections[i].first+width*0.5;
			current_scored_shift.score=0.0;
			for(std::size_t j=0;j<buffer_for_projections.size();j++)
			{
				if(buffer_for_projections[j].first-buffer_for_projections[i].first<width)
				{
					current_scored_shift.score+=points_and_scores[buffer_for_projections[j].second].score;
					i_end=j;
				}
				else
				{
					current_scored_shift.score+=(1.0-points_and_scores[buffer_for_projections[j].second].score);
				}
			}
			best_scored_shift=current_scored_shift;
		}
		else
		{
			current_scored_shift.shift=buffer_for_projections[i].first+width*0.5;
			const double prev_score=points_and_scores[buffer_for_projections[i-1].second].score;
			current_scored_shift.score-=prev_score;
			current_scored_shift.score+=(1.0-prev_score);
			for(std::size_t j=(i_end+1);j<buffer_for_projections.size() && (buffer_for_projections[j].first-buffer_for_projections[i].first<width);j++)
			{
				const double next_score=points_and_scores[buffer_for_projections[j].second].score;
				current_scored_shift.score+=next_score;
				current_scored_shift.score-=(1.0-next_score);
				i_end=j;
			}
			if(current_scored_shift.score>best_scored_shift.score)
			{
				best_scored_shift=current_scored_shift;
			}
		}
	}
	return best_scored_shift;
}

MembranePlacement estimate_translated_membrane_placement(const std::vector<PointAndScore>& points_and_scores, const double width)
{
	std::vector< std::pair<double, std::size_t> > buffer_for_projections(points_and_scores.size());
	apollota::SubdividedIcosahedron sih(3);
	sih.fit_into_sphere(apollota::SimplePoint(0, 0, 0), 1);
	std::size_t best_id=0;
	ScoredShift best_scored_shift;
	double last_cycle_best_score=0.0;
	int number_of_cycles=0;
	int number_of_checks=0;
	while(number_of_cycles<2 || (best_scored_shift.score-last_cycle_best_score)>0.0001)
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
			const ScoredShift scored_shift=estimate_best_scored_shift(points_and_scores, sih.vertices()[i].unit(), width, buffer_for_projections);
			if(number_of_checks==0 || scored_shift.score>best_scored_shift.score)
			{
				best_id=i;
				best_scored_shift=scored_shift;
			}
			number_of_checks++;
		}
		number_of_cycles++;
	}
	std::cerr << "number_of_cycles " << number_of_cycles << "\n";
	std::cerr << "number_of_checks " << number_of_checks << "\n";
	const apollota::SimplePoint best_direction=sih.vertices()[best_id].unit();
	return MembranePlacement(best_direction*best_scored_shift.shift, best_direction);
}

MembranePlacement estimate_membrane_placement(const std::vector<PointAndScore>& points_and_scores, const double width)
{
	apollota::SimplePoint original_center(0, 0, 0);
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

	const MembranePlacement translated_membrane_placement=estimate_translated_membrane_placement(translated_points_and_scores, width);

	return MembranePlacement(original_center+translated_membrane_placement.position, translated_membrane_placement.normal);
}

}

void place_membrane(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'annotation x y z r tags adjuncts')");
	pohw.describe_io("stdout", false, true, "list of balls (line format: 'annotation x y z r tags adjuncts')");

	const std::string scores_file=poh.argument<std::string>(pohw.describe_option("--scores-file", "string", "file path to input atom scores", true), "");
	const double membrane_width=poh.argument<double>(pohw.describe_option("--membrane-width", "number", "full membrane width", true));

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	if(membrane_width<=1.0)
	{
		throw std::runtime_error("Invalid membrane width.");
	}

	std::vector< std::pair<CRAD, BallValue> > list_of_balls;
	auxiliaries::IOUtilities().read_lines_to_map(std::cin, list_of_balls);
	if(list_of_balls.empty())
	{
		throw std::runtime_error("No input balls.");
	}

	const std::map<CRAD, double> map_of_scores=auxiliaries::IOUtilities().read_file_lines_to_map< std::map<CRAD, double> >(scores_file);
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
			points_and_scores.push_back(PointAndScore(apollota::SimplePoint(list_of_balls[i].second), it->second));
		}
	}

	const MembranePlacement membrane_placement=estimate_membrane_placement(points_and_scores, membrane_width);

	std::cerr << "direction " << membrane_placement.normal.x << " " << membrane_placement.normal.y << " " << membrane_placement.normal.z << "\n";

	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		if(fabs((apollota::SimplePoint(list_of_balls[i].second)-membrane_placement.position)*membrane_placement.normal.unit())<(membrane_width*0.5))
		{
			list_of_balls[i].second.props.update_adjuncts("membrane=1");
		}
		else
		{
			list_of_balls[i].second.props.update_adjuncts("membrane=0");
		}
	}

	auxiliaries::IOUtilities().write_map(list_of_balls, std::cout);
}
