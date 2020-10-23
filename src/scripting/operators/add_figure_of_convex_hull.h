#ifndef SCRIPTING_OPERATORS_ADD_FIGURE_OF_CONVEX_HULL_H_
#define SCRIPTING_OPERATORS_ADD_FIGURE_OF_CONVEX_HULL_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class AddFigureOfConvexHull : public OperatorBase<AddFigureOfConvexHull>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		double total_volume;

		Result() : total_volume(0.0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("total_volume")=total_volume;
		}
	};

	SelectionManager::Query parameters_for_selecting_atoms;
	std::vector<std::string> figure_name;

	AddFigureOfConvexHull()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_atoms=OperatorsUtilities::read_generic_selecting_query(input);
		figure_name=input.get_value_vector<std::string>("figure-name");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
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

		if(atom_ids.size()<4)
		{
			throw std::runtime_error(std::string("Less than 4 atoms selected."));
		}

		std::vector<apollota::SimpleSphere> points;
		points.reserve(atom_ids.size());

		for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
		{
			points.push_back(apollota::SimpleSphere(data_manager.atoms()[*it].value, 1.0));
		}

		std::set<apollota::Triple> faces;

		double total_volume=0.0;

		{
			const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(points, 4.0, false, false);
			if(triangulation_result.quadruples_map.empty())
			{
				throw std::runtime_error(std::string("Failed to construct triangulation for selection."));
			}
			std::map<apollota::Triple, int> face_counts;
			for(apollota::Triangulation::QuadruplesMap::const_iterator it=triangulation_result.quadruples_map.begin();it!=triangulation_result.quadruples_map.end();++it)
			{
				const apollota::Quadruple& quadruple=it->first;
				const double tetrahedron_volume=fabs(apollota::signed_volume_of_tetrahedron(points[quadruple.get(0)], points[quadruple.get(1)], points[quadruple.get(2)], points[quadruple.get(3)]));
				total_volume+=tetrahedron_volume;
				for(unsigned int i=0;i<4;i++)
				{
					face_counts[quadruple.exclude(i)]++;
				}
			}
			for(std::map<apollota::Triple, int>::const_iterator it=face_counts.begin();it!=face_counts.end();++it)
			{
				if(it->second==1)
				{
					faces.insert(it->first);
				}
			}
		}

		if(faces.size()<4)
		{
			throw std::runtime_error(std::string("Failed to collect convex hull faces."));
		}

		Figure figure;
		figure.name=LongName(figure_name);

		for(std::set<apollota::Triple>::const_iterator it=faces.begin();it!=faces.end();++it)
		{
			const apollota::Triple& triple=(*it);
			const apollota::SimplePoint normal=apollota::plane_normal_from_three_points<apollota::SimplePoint>(points[triple.get(0)], points[triple.get(1)], points[triple.get(2)]);
			figure.add_triangle(points[triple.get(0)], points[triple.get(1)], points[triple.get(2)], normal);
		}

		data_manager.add_figure(figure);

		Result result;
		result.total_volume=total_volume;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_ADD_FIGURE_OF_CONVEX_HULL_H_ */
