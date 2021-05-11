#ifndef SCRIPTING_OPERATORS_EXPORT_CARTOON_AS_PYMOL_CGO_H_
#define SCRIPTING_OPERATORS_EXPORT_CARTOON_AS_PYMOL_CGO_H_

#include "../operators_common.h"
#include "../../common/construction_of_structural_cartoon.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ExportCartoonAsPymolCGO : public OperatorBase<ExportCartoonAsPymolCGO>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string file;
		std::string dump;
		SummaryOfAtoms atoms_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
			if(!dump.empty())
			{
				heterostorage.variant_object.value("dump")=dump;
			}
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
		}
	};

	std::string name;
	bool wireframe;
	SelectionManager::Query parameters_for_selecting;
	std::vector<std::string> representation_names;
	std::string file;

	ExportCartoonAsPymolCGO() : wireframe(false)
	{
	}

	void initialize(CommandInput& input)
	{
		name=input.get_value_or_default<std::string>("name", "atoms");
		wireframe=input.get_flag("wireframe");
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		representation_names=input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
		file=input.get_value<std::string>("file");
		assert_file_name_input(file, false);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "name of CGO object", "atoms"));
		doc.set_option_decription(CDOD("wireframe", CDOD::DATATYPE_BOOL, "flag use wireframe representation"));
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("rep", CDOD::DATATYPE_STRING_ARRAY, "representation names", ""));
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();
		data_manager.assert_atoms_representations_availability();

		assert_file_name_input(file, false);

		if(name.empty())
		{
			throw std::runtime_error(std::string("Missing object name."));
		}

		std::set<std::size_t> representation_ids=data_manager.atoms_representation_descriptor().ids_by_names(representation_names);

		if(representation_ids.empty())
		{
			representation_ids.insert(0);
		}

		if(representation_ids.size()>1)
		{
			throw std::runtime_error(std::string("More than one representation requested."));
		}

		const std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable visible atoms selected."));
		}

		if(!data_manager.bonding_links_info().valid(data_manager.atoms(), data_manager.primary_structure_info()))
		{
			data_manager.reset_bonding_links_info_by_creating(common::ConstructionOfBondingLinks::ParametersToConstructBundleOfBondingLinks());
		}

		common::ConstructionOfStructuralCartoon::Parameters parameters_for_cartoon;
		common::ConstructionOfStructuralCartoon::BundleOfMeshInformation bundle_of_cartoon_mesh;
		if(!common::ConstructionOfStructuralCartoon::construct_bundle_of_mesh_information(
				parameters_for_cartoon,
				data_manager.atoms(),
				data_manager.primary_structure_info(),
				data_manager.secondary_structure_info(),
				data_manager.bonding_links_info(),
				bundle_of_cartoon_mesh))
		{
			throw std::runtime_error(std::string("Failed to construct cartoon mesh."));
		}

		auxiliaries::OpenGLPrinter opengl_printer;
		{
			std::vector<apollota::SimplePoint> vertices(3);
			std::vector<apollota::SimplePoint> normals(3);
			unsigned int prev_color=0;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::size_t id=(*it);
				const std::vector<unsigned int>& indices=bundle_of_cartoon_mesh.mapped_indices[id];
				if(!indices.empty() && indices.size()%3==0)
				{
					for(std::set<std::size_t>::const_iterator jt=representation_ids.begin();jt!=representation_ids.end();++jt)
					{
						const std::size_t visual_id=(*jt);
						if(visual_id<data_manager.atoms_display_states()[id].visuals.size())
						{
							const DataManager::DisplayState::Visual& dsv=data_manager.atoms_display_states()[id].visuals[visual_id];
							if(prev_color==0 || dsv.color!=prev_color)
							{
								opengl_printer.add_color(dsv.color);
							}
							prev_color=dsv.color;
							for(std::size_t i=0;i<indices.size();i+=3)
							{
								for(std::size_t j=0;j<3;j++)
								{
									vertices[j].x=bundle_of_cartoon_mesh.global_buffer_of_vertices[indices[i+j]*3+0];
									vertices[j].y=bundle_of_cartoon_mesh.global_buffer_of_vertices[indices[i+j]*3+1];
									vertices[j].z=bundle_of_cartoon_mesh.global_buffer_of_vertices[indices[i+j]*3+2];
									normals[j].x=bundle_of_cartoon_mesh.global_buffer_of_normals[indices[i+j]*3+0];
									normals[j].y=bundle_of_cartoon_mesh.global_buffer_of_normals[indices[i+j]*3+1];
									normals[j].z=bundle_of_cartoon_mesh.global_buffer_of_normals[indices[i+j]*3+2];
								}
								if(wireframe)
								{
									opengl_printer.add_line_loop(vertices);
								}
								else
								{
									opengl_printer.add_triangle_strip(vertices, normals);
								}
							}
						}
					}
				}
			}
		}

		OutputSelector output_selector(file);

		{
			std::ostream& output=output_selector.stream();
			assert_io_stream(file, output);
			opengl_printer.print_pymol_script(name, true, output);
		}

		Result result;
		result.file=file;
		if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
		{
			result.dump=output_selector.str();
		}
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_CARTOON_AS_PYMOL_CGO_H_ */
