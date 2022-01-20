#ifndef SCRIPTING_OPERATORS_ADD_FIGURE_OF_NTH_ORDER_CELL_H_
#define SCRIPTING_OPERATORS_ADD_FIGURE_OF_NTH_ORDER_CELL_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class AddFigureOfNthOrderCell : public OperatorBase<AddFigureOfNthOrderCell>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		int total_voxels;
		BoundingBox bounding_box;

		Result() : total_voxels(0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(bounding_box, heterostorage.variant_object.object("bounding_box"));
			heterostorage.variant_object.value("total_voxels")=total_voxels;
		}
	};

	double voxel_diameter;
	double probe;
	SelectionManager::Query parameters_for_selecting_atoms;
	std::vector<std::string> figure_name;

	AddFigureOfNthOrderCell() : voxel_diameter(0.1), probe(1.4)
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_atoms=OperatorsUtilities::read_generic_selecting_query(input);
		voxel_diameter=input.get_value_or_default<double>("voxel-diameter", 0.1);
		probe=input.get_value_or_default<double>("probe", 1.4);
		figure_name=input.get_value_vector<std::string>("figure-name");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("voxel-diameter", CDOD::DATATYPE_FLOAT, "voxel diameter", 0.1));
		doc.set_option_decription(CDOD("probe", CDOD::DATATYPE_FLOAT, "rolling probe radius", 1.4));
		doc.set_option_decription(CDOD("figure-name", CDOD::DATATYPE_STRING_ARRAY, "figure name"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		const std::set<std::size_t> atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);

		if(atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		if(atom_ids.size()>16)
		{
			throw std::runtime_error(std::string("Too many atoms selected, must be no more than 16."));
		}

		if(voxel_diameter<0.05)
		{
			throw std::runtime_error(std::string("Voxel diameter is too small, need to be not less than 0.05"));
		}

		std::vector<apollota::SimpleSphere> spheres(data_manager.atoms().size());
		for(std::size_t i=0;i<data_manager.atoms().size();i++)
		{
			spheres[i]=voronota::apollota::SimpleSphere(data_manager.atoms()[i].value, data_manager.atoms()[i].value.r+probe);
		}

		BoundingBox bounding_box;
		for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
		{
			const apollota::SimpleSphere& s=spheres[*it];
			const apollota::SimplePoint p(s);
			apollota::SimplePoint shift(s.r, s.r, s.r);
			bounding_box.update(p-shift);
			bounding_box.update(p+shift);
		}
		bounding_box.p_min.x=std::floor(bounding_box.p_min.x/voxel_diameter)*voxel_diameter;
		bounding_box.p_min.y=std::floor(bounding_box.p_min.y/voxel_diameter)*voxel_diameter;
		bounding_box.p_min.z=std::floor(bounding_box.p_min.z/voxel_diameter)*voxel_diameter;
		bounding_box.p_max.x=std::ceil(bounding_box.p_max.x/voxel_diameter)*voxel_diameter;
		bounding_box.p_max.y=std::ceil(bounding_box.p_max.y/voxel_diameter)*voxel_diameter;
		bounding_box.p_max.z=std::ceil(bounding_box.p_max.z/voxel_diameter)*voxel_diameter;

		std::set<std::size_t> neighboring_ids;
		{
			const apollota::BoundingSpheresHierarchy bsh(spheres, 3.5, 1);
			for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
			{
				const std::vector<std::size_t> collisions=voronota::apollota::SearchForSphericalCollisions::find_all_collisions(bsh, spheres[*it]);
				for(std::size_t j=0;j<collisions.size();j++)
				{
					if(atom_ids.count(collisions[j])==0)
					{
						neighboring_ids.insert(collisions[j]);
					}
				}
			}
		}

		Figure figure;
		figure.name=LongName(figure_name);

		Result result;
		result.bounding_box=bounding_box;

		{
			apollota::SimplePoint p=bounding_box.p_min;
			for(p.x=bounding_box.p_min.x;p.x<=bounding_box.p_max.x;p.x+=voxel_diameter)
			{
				for(p.y=bounding_box.p_min.y;p.y<=bounding_box.p_max.y;p.y+=voxel_diameter)
				{
					for(p.z=bounding_box.p_min.z;p.z<=bounding_box.p_max.z;p.z+=voxel_diameter)
					{
						double min_dist=std::numeric_limits<double>::max();
						double max_dist=(-std::numeric_limits<double>::max());
						for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
						{
							const double dist=apollota::minimal_distance_from_point_to_sphere(p, spheres[*it]);
							min_dist=std::min(min_dist, dist);
							max_dist=std::max(max_dist, dist);
						}
						if(min_dist<=0.0)
						{
							bool discard=false;
							for(std::set<std::size_t>::const_iterator it=neighboring_ids.begin();!discard && it!=neighboring_ids.end();++it)
							{
								if(apollota::minimal_distance_from_point_to_sphere(p, spheres[*it])<max_dist)
								{
									discard=true;
								}
							}
							if(!discard)
							{
								figure.add_voxel(p, voxel_diameter);
								result.total_voxels++;
							}
						}
					}
				}
			}
		}

		data_manager.add_figure(figure);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_ADD_FIGURE_OF_NTH_ORDER_CELL_H_ */
