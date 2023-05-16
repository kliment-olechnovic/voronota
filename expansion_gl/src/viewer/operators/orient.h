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
		scripting::SummaryOfAtoms atoms_summary;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			scripting::VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
		}
	};

	scripting::CongregationOfDataManagers::ObjectQuery query;
	bool reset;

	Orient() : reset(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		query=scripting::OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
		reset=input.get_flag("reset");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		scripting::OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
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

		Result result;

		for(std::size_t i=0;i<objects.size();i++)
		{
			result.atoms_summary.feed(scripting::SummaryOfAtoms(objects[i]->atoms()));
		}

		apollota::SimplePoint global_center=(result.atoms_summary.bounding_box.p_min+result.atoms_summary.bounding_box.p_max)*0.5;

		glm::mat4 matrix(1.0f);

		if(!reset)
		{
			std::vector< std::vector<double> > data;

			for(std::size_t i=0;i<objects.size();i++)
			{
				const scripting::DataManager& dm=*(objects[i]);
				for(std::size_t j=0;j<dm.atoms().size();j++)
				{
					const scripting::Atom& atom=dm.atoms()[j];
					std::vector<double> v(3, 0.0);
					v[0]=atom.value.x;
					v[1]=atom.value.y;
					v[2]=atom.value.z;
					data.push_back(v);
				}
			}

			const std::vector< std::vector<double> > ds=eigen_pca::compute_pca_3d_directions(data);

			glm::vec3 b1(static_cast<float>(ds[0][0]), static_cast<float>(ds[0][1]), static_cast<float>(ds[0][2]));
			glm::vec3 b2(static_cast<float>(ds[1][0]), static_cast<float>(ds[1][1]), static_cast<float>(ds[1][2]));

			{
				if(b1[2]<0.0f)
				{
					b1=b1*(-1.0f);
				}
				const glm::vec3 axis=glm::cross(b1, glm::vec3(0.0f, 0.0f, 1.0f));
				const double axis_length=glm::length(axis);
				const float angle_in_radians=asin(axis_length);

				matrix=glm::rotate(glm::mat4(1.0f), angle_in_radians, axis)*matrix;
			}

			b2=glm::vec3(matrix*glm::vec4(b2, 1.0f));

			{
				if(b2[1]<0.0f)
				{
					b2=b2*(-1.0f);
				}
				const glm::vec3 axis=glm::cross(b2, glm::vec3(0.0f, 1.0f, 0.0f));
				const double axis_length=glm::length(axis);
				const float angle_in_radians=asin(axis_length);

				matrix=glm::rotate(glm::mat4(1.0f), angle_in_radians, axis)*matrix;
			}
		}

		matrix=matrix*glm::translate(glm::mat4(1.0f), glm::vec3(static_cast<float>(0.0-global_center.x), static_cast<float>(0.0-global_center.y), static_cast<float>(0.0-global_center.z)));

		uv::ViewerApplication::instance().reset_view(matrix);

		uv::ViewerApplication::instance_refresh_frame(false);

		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_ORIENT_H_ */
