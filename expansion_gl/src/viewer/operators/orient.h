#ifndef VIEWER_OPERATORS_ORIENT_H_
#define VIEWER_OPERATORS_ORIENT_H_

#include "../operators_common.h"

#include "../../../expansion_js/src/dependencies/utilities/eigen_pca.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class Orient : public scripting::OperatorBase<Orient>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		std::vector<double> offset;
		std::vector< std::vector<double> > axises;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			{
				std::vector<scripting::VariantValue>& array=heterostorage.variant_object.values_array("offset");
				for(std::size_t i=0;i<offset.size();i++)
				{
					array.push_back(scripting::VariantValue(offset[i]));
				}
			}
			for(std::size_t j=0;j<axises.size();j++)
			{
				std::vector<scripting::VariantValue>& array=heterostorage.variant_object.values_array(j==0 ? "axis1" : (j==1 ? "axis2" : "axis3"));
				for(std::size_t i=0;i<axises[j].size();i++)
				{
					array.push_back(scripting::VariantValue(axises[j][i]));
				}
			}
		}
	};

	scripting::CongregationOfDataManagers::ObjectQuery query;
	std::string selection_expresion_for_atoms;
	bool vertical;
	bool reset;

	Orient() : vertical(false), reset(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		query=scripting::OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
		selection_expresion_for_atoms=input.get_value_or_default<std::string>("atoms", "[]");
		vertical=input.get_flag("vertical");
		reset=input.get_flag("reset");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		scripting::OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
		doc.set_option_decription(CDOD("atoms", CDOD::DATATYPE_STRING, "selection expression for atoms", "[]"));
		doc.set_option_decription(CDOD("vertical", CDOD::DATATYPE_BOOL, "flag to orient more vertically"));
		doc.set_option_decription(CDOD("reset", CDOD::DATATYPE_BOOL, "flag to reset orientation to initial"));
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		congregation_of_data_managers.assert_objects_availability();

		const std::vector<scripting::DataManager*> objects=congregation_of_data_managers.get_objects(query);
		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}

		std::vector< std::vector<double> > atom_coordinates;

		for(std::size_t i=0;i<objects.size();i++)
		{
			scripting::DataManager& data_manager=*(objects[i]);
			const std::set<std::size_t> atom_ids=data_manager.selection_manager().select_atoms(scripting::SelectionManager::Query(selection_expresion_for_atoms, false));
			for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
			{
				const scripting::Atom& atom=data_manager.atoms()[*it];
				std::vector<double> v(3, 0.0);
				v[0]=atom.value.x;
				v[1]=atom.value.y;
				v[2]=atom.value.z;
				atom_coordinates.push_back(v);
			}
		}

		if(atom_coordinates.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		if(atom_coordinates.size()<3)
		{
			throw std::runtime_error(std::string("Less than 3 atoms selected."));
		}

		std::vector<double> offset(3, 0.0);
		{
			for(std::size_t i=0;i<atom_coordinates.size();i++)
			{
				for(std::size_t j=0;j<offset.size();j++)
				{
					offset[j]-=atom_coordinates[i][j];
				}
			}
			for(std::size_t j=0;j<offset.size();j++)
			{
				offset[j]/=static_cast<double>(atom_coordinates.size());
			}
		}

		Result result;

		result.offset=offset;

		glm::mat4 matrix(1.0f);

		if(!reset)
		{
			const std::vector< std::vector<double> > ds=eigen_pca::compute_pca_3d_directions(atom_coordinates);

			result.axises=ds;

			glm::vec3 b1(static_cast<float>(ds[0][0]), static_cast<float>(ds[0][1]), static_cast<float>(ds[0][2]));
			glm::vec3 b2(static_cast<float>(ds[1][0]), static_cast<float>(ds[1][1]), static_cast<float>(ds[1][2]));

			{
				if(b1[2]<0.0f)
				{
					b1=b1*(-1.0f);
				}
				const glm::vec3 axis=glm::cross(b1, glm::vec3(0.0f, 0.0f, 1.0f));
				const double axis_length=glm::length(axis);
				if(axis_length>0.0)
				{
					matrix=glm::rotate(glm::mat4(1.0f), static_cast<float>(std::asin(axis_length)), axis)*matrix;
				}
			}

			b2=glm::vec3(matrix*glm::vec4(b2, 1.0f));

			{
				if(vertical ? b2[0]<0.0f : b2[1]<0.0f)
				{
					b2=b2*(-1.0f);
				}
				const glm::vec3 axis=glm::cross(b2, glm::vec3((vertical ? 1.0f : 0.0f), (vertical ? 0.0f : 1.0f), 0.0f));
				const double axis_length=glm::length(axis);
				if(axis_length>0.0)
				{
					matrix=glm::rotate(glm::mat4(1.0f), static_cast<float>(std::asin(axis_length)), axis)*matrix;
				}
			}
		}

		matrix=matrix*glm::translate(glm::mat4(1.0f), glm::vec3(static_cast<float>(offset[0]), static_cast<float>(offset[1]), static_cast<float>(offset[2])));

		uv::ViewerApplication::instance().reset_view(matrix);

		uv::ViewerApplication::instance_refresh_frame(false);

		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_ORIENT_H_ */
