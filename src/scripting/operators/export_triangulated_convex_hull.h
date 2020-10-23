#ifndef SCRIPTING_OPERATORS_EXPORT_TRIANGULATED_CONVEX_HULL_H_
#define SCRIPTING_OPERATORS_EXPORT_TRIANGULATED_CONVEX_HULL_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ExportTriangulatedConvexHull : public OperatorBase<ExportTriangulatedConvexHull>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string file;
		std::string dump;
		std::size_t total_number_of_tetrahedrons;
		double total_relevant_tetrahedron_volume;

		Result() : total_number_of_tetrahedrons(0), total_relevant_tetrahedron_volume(0.0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
			if(!dump.empty())
			{
				heterostorage.variant_object.value("dump")=dump;
			}
			heterostorage.variant_object.value("total_number_of_tetrahedrons")=total_number_of_tetrahedrons;
			heterostorage.variant_object.value("total_relevant_tetrahedron_volume")=total_relevant_tetrahedron_volume;
		}
	};

	SelectionManager::Query parameters_for_selecting_atoms;
	std::string file;
	std::vector<std::string> adjuncts;

	ExportTriangulatedConvexHull()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_atoms=OperatorsUtilities::read_generic_selecting_query(input);
		file=input.get_value_or_first_unused_unnamed_value("file");
		assert_file_name_input(file, false);
		adjuncts=input.get_value_vector_or_default<std::string>("adjuncts", std::vector<std::string>());
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		doc.set_option_decription(CDOD("adjuncts", CDOD::DATATYPE_STRING_ARRAY, "names of atom adjuncts to output", ""));
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

		std::vector<apollota::SimpleSphere> points1;
		points1.reserve(atom_ids.size());

		std::vector<std::size_t> points1_ids_to_atoms_ids;
		points1_ids_to_atoms_ids.reserve(atom_ids.size());

		for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
		{
			points1.push_back(apollota::SimpleSphere(data_manager.atoms()[*it].value, 1.0));
			points1_ids_to_atoms_ids.push_back(*it);
		}

		std::set<apollota::Triple> faces1;

		{
			const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(points1, 4.0, false, false);
			if(triangulation_result.quadruples_map.empty())
			{
				throw std::runtime_error(std::string("Failed to construct triangulation for selection."));
			}
			std::map<apollota::Triple, int> face_counts;
			for(apollota::Triangulation::QuadruplesMap::const_iterator it=triangulation_result.quadruples_map.begin();it!=triangulation_result.quadruples_map.end();++it)
			{
				const apollota::Quadruple& q=it->first;
				for(unsigned int i=0;i<4;i++)
				{
					face_counts[q.exclude(i)]++;
				}
			}
			for(std::map<apollota::Triple, int>::const_iterator it=face_counts.begin();it!=face_counts.end();++it)
			{
				if(it->second==1)
				{
					faces1.insert(it->first);
				}
			}
		}

		if(faces1.size()<4)
		{
			throw std::runtime_error(std::string("Failed to collect convex hull faces."));
		}

		std::vector<apollota::SimpleSphere> points2;
		points2.reserve(atom_ids.size());

		std::vector<std::size_t> points2_ids_to_atoms_ids;
		points2_ids_to_atoms_ids.reserve(atom_ids.size());

		{
			std::set<std::size_t> selected_point1_ids;
			for(std::set<apollota::Triple>::const_iterator it=faces1.begin();it!=faces1.end();++it)
			{
				const apollota::Triple& triple=(*it);
				selected_point1_ids.insert(triple.get(0));
				selected_point1_ids.insert(triple.get(1));
				selected_point1_ids.insert(triple.get(2));
			}
			for(std::set<std::size_t>::const_iterator it=selected_point1_ids.begin();it!=selected_point1_ids.end();++it)
			{
				points2.push_back(points1[*it]);
				points2_ids_to_atoms_ids.push_back(points1_ids_to_atoms_ids[*it]);
			}
		}

		OutputSelector output_selector(file);

		std::size_t total_number_of_tetrahedrons=0;
		double total_relevant_tetrahedron_volume=0.0;

		{
			std::ostream& output=output_selector.stream();
			assert_io_stream(file, output);

			const apollota::Triangulation::Result convex_hull_triangulation_result=apollota::Triangulation::construct_result(points2, 4.0, false, false);
			if(convex_hull_triangulation_result.quadruples_map.empty())
			{
				throw std::runtime_error(std::string("Failed to triangulate convex hull."));
			}

			for(apollota::Triangulation::QuadruplesMap::const_iterator it=convex_hull_triangulation_result.quadruples_map.begin();it!=convex_hull_triangulation_result.quadruples_map.end();++it)
			{
				const apollota::Quadruple& quadruple=it->first;
				const apollota::SimpleSphere& sphere=it->second.at(0);
				const double tetrahedron_volume=fabs(apollota::signed_volume_of_tetrahedron(points2[quadruple.get(0)], points2[quadruple.get(1)], points2[quadruple.get(2)], points2[quadruple.get(3)]));
				total_number_of_tetrahedrons++;
				total_relevant_tetrahedron_volume+=tetrahedron_volume;
				const apollota::Quadruple mapped_quadruple(points2_ids_to_atoms_ids[quadruple.get(0)], points2_ids_to_atoms_ids[quadruple.get(1)], points2_ids_to_atoms_ids[quadruple.get(2)], points2_ids_to_atoms_ids[quadruple.get(3)]);
				for(int j=0;j<4;j++)
				{
					output << mapped_quadruple.get(j) << " ";
				}
				output << sphere.x << " " << sphere.y << " " << sphere.z << " " << sphere.r << " " << tetrahedron_volume;
				for(std::size_t a=0;a<adjuncts.size();a++)
				{
					const std::string& adjunct_name=adjuncts[a];
					for(int j=0;j<4;j++)
					{
						const std::map<std::string, double>& adjuncts_map=data_manager.atoms().at(mapped_quadruple.get(j)).value.props.adjuncts;
						std::map<std::string, double>::const_iterator adjunct_it=adjuncts_map.find(adjunct_name);
						if(adjunct_it!=adjuncts_map.end())
						{
							output << " " << (adjunct_it->second);
						}
						else
						{
							output << " 0";
						}
					}
				}
				output << "\n";
			}
		}

		Result result;
		result.file=file;
		if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
		{
			result.dump=output_selector.str();
		}
		result.total_number_of_tetrahedrons=total_number_of_tetrahedrons;
		result.total_relevant_tetrahedron_volume=total_relevant_tetrahedron_volume;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_TRIANGULATED_CONVEX_HULL_H_ */
