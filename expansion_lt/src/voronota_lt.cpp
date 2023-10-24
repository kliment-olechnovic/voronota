#include <iostream>
#include <vector>

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
			if(i==0)
			{
				grid_offset_=gp;
				grid_size_=gp;
			}
			else
			{
				grid_offset_.x=std::min(grid_offset_.x, gp.x);
				grid_offset_.y=std::min(grid_offset_.y, gp.y);
				grid_offset_.z=std::min(grid_offset_.z, gp.z);
				grid_size_.x=std::max(grid_size_.x, gp.x);
				grid_size_.y=std::max(grid_size_.y, gp.y);
				grid_size_.z=std::max(grid_size_.z, gp.z);
			}
		}

		grid_size_.x=grid_size_.x-grid_offset_.x+1;
		grid_size_.y=grid_size_.y-grid_offset_.y+1;
		grid_size_.z=grid_size_.z-grid_offset_.z+1;

		map_of_boxes_.resize(grid_size_.x*grid_size_.y*grid_size_.z, -1);

		for(std::size_t i=0;i<spheres_.size();i++)
		{
			const GridPoint gp(spheres_[i], box_size_, grid_offset_);
			const int index=gp.index(grid_size_);
			const int box_id=map_of_boxes_[index];
			if(box_id<0)
			{
				map_of_boxes_[index]=static_cast<int>(boxes_.size());
				boxes_.push_back(std::vector<std::size_t>(1, i));
			}
			else
			{
				boxes_[box_id].push_back(i);
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
			colliding_ids.reserve(20);
			const GridPoint gp(central_sphere, box_size_, grid_offset_);
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
						const int index=dgp.index(grid_size_);
						if(index>=0)
						{
							const int box_id=map_of_boxes_[index];
							if(box_id>=0)
							{
								const std::vector<std::size_t>& ids=boxes_[box_id];
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
		}
		return (!colliding_ids.empty());
	}

private:
	struct GridPoint
	{
		int x;
		int y;
		int z;

		GridPoint() : x(0), y(0), z(0)
		{
		}

		GridPoint(const SimpleSphere& s, const double grid_step)
		{
			init(s, grid_step);
		}

		GridPoint(const SimpleSphere& s, const double grid_step, const GridPoint& grid_offset)
		{
			init(s, grid_step, grid_offset);
		}

		void init(const SimpleSphere& s, const double grid_step)
		{
			x=static_cast<int>(s.x/grid_step);
			y=static_cast<int>(s.y/grid_step);
			z=static_cast<int>(s.z/grid_step);
		}

		void init(const SimpleSphere& s, const double grid_step, const GridPoint& grid_offset)
		{
			x=static_cast<int>(s.x/grid_step)-grid_offset.x;
			y=static_cast<int>(s.y/grid_step)-grid_offset.y;
			z=static_cast<int>(s.z/grid_step)-grid_offset.z;
		}

		void init(const int index, const GridPoint& grid_size)
		{
			z=index/(grid_size.x*grid_size.y);
			y=(index-(z*grid_size.x*grid_size.y))/grid_size.x;
			x=(index%grid_size.x);
		}

		int index(const GridPoint& grid_size) const
		{
			return ((x>=0 && y>=0 && z>=0 && x<grid_size.x && y<grid_size.y &&z<grid_size.z) ? (z*grid_size.x*grid_size.y+y*grid_size.x+x) : (-1));
		}

		bool operator<(const GridPoint& gp) const
		{
			return (x<gp.x || (x==gp.x && y<gp.y) || (x==gp.x && y==gp.y && z<gp.z));
		}
	};

	std::vector<SimpleSphere> spheres_;
	GridPoint grid_offset_;
	GridPoint grid_size_;
	std::vector<int> map_of_boxes_;
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

	std::vector< std::vector<std::size_t> > all_colliding_ids(spheres_searcher.all_spheres().size());

	{
#pragma omp parallel for
		for(std::size_t i=0;i<spheres_searcher.all_spheres().size();i++)
		{
			spheres_searcher.find_colliding_ids(i, all_colliding_ids[i]);
		}
	}

	std::size_t total_collisions=0;
	for(std::size_t i=0;i<all_colliding_ids.size();i++)
	{
		total_collisions+=all_colliding_ids[i].size();
	}

	std::cout << "total_collisions: " << total_collisions << "\n";

	std::vector< std::pair<std::size_t, std::size_t> > possible_pairs;
	possible_pairs.reserve(total_collisions/2);
	for(std::size_t i=0;i<all_colliding_ids.size();i++)
	{
	    for(std::size_t j=0;j<all_colliding_ids[i].size();j++)
	    {
	        const std::size_t a_id=i;
	        const std::size_t b_id=all_colliding_ids[i][j];
	        if(a_id<b_id)
	        {
	        	possible_pairs.push_back(std::pair<std::size_t, std::size_t>(a_id, b_id));
	        }
	    }
	}

	std::vector<voronota_lt::ConstrainedContactsConstruction::ContactDescriptorSummary> possible_pair_summaries(possible_pairs.size());

	{
#pragma omp parallel for
		for(std::size_t i=0;i<possible_pairs.size();i++)
		{
			const std::size_t a_id=possible_pairs[i].first;
			const std::size_t b_id=possible_pairs[i].second;
			voronota_lt::ConstrainedContactsConstruction::construct_contact_descriptor_summary(spheres_searcher.all_spheres(), a_id, b_id, all_colliding_ids[a_id], all_colliding_ids[b_id], possible_pair_summaries[i]);
		}
	}

	voronota_lt::ConstrainedContactsConstruction::ContactDescriptorSummary total_summary;
	for(std::size_t i=0;i<possible_pair_summaries.size();i++)
	{
		total_summary.add(possible_pair_summaries[i]);
	}

	std::cout << "total_contacts_count: " << total_summary.count << "\n";
	std::cout << "total_contacts_area: " << total_summary.area << "\n";
	std::cout << "total_contacts_complexity: " << total_summary.complexity << "\n";

	return 1;
}

