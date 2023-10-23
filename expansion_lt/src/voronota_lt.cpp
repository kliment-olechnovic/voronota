#include <iostream>
#include <vector>
#include <map>

#include "voronota_lt.h"

namespace voronota_lt
{

class SpheresSearcher
{
public:
	SpheresSearcher(const std::vector<SimpleSphere>& spheres) : spheres_(spheres), box_size_(1.0)
	{
		for(std::size_t i=0;i<spheres_.size();i++)
		{
			const SimpleSphere& s=spheres_[i];
			box_size_=std::max(box_size_, s.r*2.0+0.25);
		}

		for(std::size_t i=0;i<spheres_.size();i++)
		{
			const GridPoint gp(spheres_[i], box_size_);
			std::map<GridPoint, std::size_t>::iterator it=map_of_boxes_.find(gp);
			if(it==map_of_boxes_.end())
			{
				map_of_boxes_.insert(std::pair<GridPoint, std::size_t>(gp, boxes_.size()));
				boxes_.push_back(std::vector<std::size_t>(1, i));
			}
			else
			{
				boxes_[it->second].push_back(i);
			}
		}
	}

	void init()
	{

	}

	const std::vector<SimpleSphere>& all_spheres() const
	{
		return spheres_;
	}

	bool find_colliding_ids(const std::size_t& central_id, std::vector<std::size_t>& colliding_ids) const
	{
		colliding_ids.clear();
		if(central_id<spheres_.size())
		{
			const SimpleSphere& central_sphere=spheres_[central_id];
			colliding_ids.reserve(10);
			const GridPoint gp(central_sphere, box_size_);
			GridPoint dgp=gp;
			for(int dx=-1;dx<=1;dx++)
			{
				dgp.x=gp.x+dx;
				for(int dy=-1;dy<=1;dy++)
				{
					dgp.y=gp.y+dy;
					for(int dz=-1;dz<=1;dz++)
					{
						dgp.z=gp.z+dz;
						std::map<GridPoint, std::size_t>::const_iterator it=map_of_boxes_.find(dgp);
						if(it!=map_of_boxes_.end())
						{
							const std::vector<std::size_t>& ids=boxes_[it->second];
							for(std::size_t i=0;i<ids.size();i++)
							{
								const std::size_t id=ids[i];
								if(id!=central_id && sphere_intersects_sphere(central_sphere, spheres_[id]))
								{
									colliding_ids.push_back(id);
								}
							}
						}
					}
				}
			}
		}
		return (!colliding_ids.empty());
	}

	std::size_t find_all_colliding_ids(std::vector< std::vector<std::size_t> >& all_colliding_ids) const
	{
		std::size_t number_of_collisions=0;
		all_colliding_ids.clear();
		all_colliding_ids.resize(spheres_.size());
		for(std::size_t i=0;i<spheres_.size();i++)
		{
			if(find_colliding_ids(i, all_colliding_ids[i]))
			{
				number_of_collisions+=all_colliding_ids[i].size();
			}
		}
		return number_of_collisions;
	}

private:
	struct GridPoint
	{
		int x;
		int y;
		int z;

		GridPoint(const SimpleSphere& s, const double grid_step) : x(static_cast<int>(s.x/grid_step)), y(static_cast<int>(s.y/grid_step)), z(static_cast<int>(s.z/grid_step))
		{
		}

		bool operator<(const GridPoint& gp) const
		{
			return (x<gp.x || (x==gp.x && y<gp.y) || (x==gp.x && y==gp.y && z<gp.z));
		}
	};

	std::vector<SimpleSphere> spheres_;
	std::map<GridPoint, std::size_t> map_of_boxes_;
	std::vector< std::vector<std::size_t> > boxes_;
	double box_size_;
};

}

int main(const int /*argc*/, const char** /*argv*/)
{
	std::vector<voronota_lt::SimpleSphere> spheres;
	{
		voronota_lt::SimpleSphere sphere;
		std::cin >> std::ws;
		while(std::cin.good())
		{
			std::cin >> sphere.x;
			if(std::cin.fail())
			{
				std::cerr << "Error reading sphere.x at line " << spheres.size() << "\n";
				return 1;
			}
			std::cin >> sphere.y;
			if(std::cin.fail())
			{
				std::cerr << "Error reading sphere.y at line " << spheres.size() << "\n";
				return 1;
			}
			std::cin >> sphere.z;
			if(std::cin.fail())
			{
				std::cerr << "Error reading sphere.z at line " << spheres.size() << "\n";
				return 1;
			}
			std::cin >> sphere.r;
			if(std::cin.fail())
			{
				std::cerr << "Error reading sphere.r at line " << spheres.size() << "\n";
				return 1;
			}
			std::cin >> std::ws;
			sphere.r+=1.4;
			spheres.push_back(sphere);
		}
	}

	std::cout << "total_balls: " << spheres.size() << "\n";

	voronota_lt::SpheresSearcher spheres_searcher(spheres);

	std::vector< std::vector<std::size_t> > all_colliding_ids;

	const int total_collisions=spheres_searcher.find_all_colliding_ids(all_colliding_ids);

	std::cout << "total_collisions: " << total_collisions << "\n";

	int total_count=0;
	double total_area=0.0;
	int total_complexity=0;

	for(std::size_t i=0;i<all_colliding_ids.size();i++)
	{
	    for(std::size_t j=0;j<all_colliding_ids[i].size();j++)
	    {
	        const std::size_t a_id=i;
	        const std::size_t b_id=all_colliding_ids[i][j];
	        if(a_id<b_id)
	        {
	            voronota_lt::ConstrainedContactsConstruction::ContactDescriptor contact_descriptor;
	            if(voronota_lt::ConstrainedContactsConstruction::construct_contact_descriptor_for_expanded_spheres_without_tessellation(spheres_searcher.all_spheres(), a_id, b_id, all_colliding_ids[a_id], all_colliding_ids[b_id], contact_descriptor))
	            {
	                total_count++;
	                total_area+=contact_descriptor.area;
	                total_complexity+=static_cast<int>(contact_descriptor.contour.size());
	            }
	        }
	    }
	}

	std::cout << "total_contacts_count: " << total_count << "\n";
	std::cout << "total_contacts_area: " << total_area << "\n";
	std::cout << "total_contacts_complexity: " << total_complexity << "\n";

	return 1;
}

