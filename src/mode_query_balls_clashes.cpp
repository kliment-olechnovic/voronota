#include "apollota/search_for_spherical_collisions.h"

#include "auxiliaries/program_options_handler.h"
#include "auxiliaries/io_utilities.h"
#include "auxiliaries/chain_residue_atom_descriptor.h"

namespace
{

typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
typedef auxiliaries::ChainResidueAtomDescriptorsPair CRADsPair;

}

void query_balls_clashes(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "list of balls (line format: 'annotation x y z r')");
	pohw.describe_io("stdout", false, true, "list of clashes (line format: 'annotation1 annotation2 distance')");

	const double clash_distance=poh.argument<double>(pohw.describe_option("--clash-distance", "number", "clash distance threshold in angstroms, default is 3.0"), 3.0);
	const double init_radius_for_BSH=poh.argument<double>(pohw.describe_option("--init-radius-for-BSH", "number", "initial radius for bounding sphere hierarchy"), 3.5);

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	if(clash_distance<0.001)
	{
		throw std::runtime_error("Clash distance threshold is too small.");
	}

	std::vector< std::pair<CRAD, apollota::SimpleSphere> > list_of_balls;
	auxiliaries::IOUtilities().read_lines_to_map(std::cin, list_of_balls);
	if(list_of_balls.empty())
	{
		throw std::runtime_error("No input.");
	}

	std::vector<apollota::SimpleSphere> spheres(list_of_balls.size());
	for(std::size_t i=0;i<list_of_balls.size();i++)
	{
		spheres[i]=apollota::SimpleSphere(list_of_balls[i].second, clash_distance*0.5);
	}

	const apollota::BoundingSpheresHierarchy bsh(spheres, init_radius_for_BSH, 1);

	std::map<CRADsPair, double> map_of_clashes;

	for(std::size_t i=0;i<spheres.size();i++)
	{
		const std::vector<std::size_t> collisions=apollota::SearchForSphericalCollisions::find_all_collisions(bsh, spheres[i]);
		for(std::size_t j=0;j<collisions.size();j++)
		{
			if(collisions[j]!=i)
			{
				const double distance=apollota::distance_from_point_to_point(spheres[i], spheres[collisions[j]]);
				if(distance<clash_distance)
				{
					const CRAD& crad_a=list_of_balls[i].first;
					const CRAD& crad_b=list_of_balls[collisions[j]].first;
					if(!(crad_a.chainID==crad_b.chainID && crad_a.resSeq==crad_b.resSeq && crad_a.iCode==crad_b.iCode))
					{
						map_of_clashes[CRADsPair(crad_a, crad_b)]=distance;
					}
				}
			}
		}
	}

	auxiliaries::IOUtilities().write_map(map_of_clashes, std::cout);
}
