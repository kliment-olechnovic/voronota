#ifndef SCRIPTING_OPERATORS_EXPORT_FIGURES_AS_PYMOL_CGO_H_
#define SCRIPTING_OPERATORS_EXPORT_FIGURES_AS_PYMOL_CGO_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class ExportFiguresAsPymolCGO
{
public:
	struct Result
	{
		std::string file;
		std::string dump;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
			if(!dump.empty())
			{
				heterostorage.variant_object.value("dump")=dump;
			}
			return (*this);
		}
	};

	std::vector<std::string> figure_name;
	std::vector<std::string> representation_names;
	std::string cgo_name;
	std::string file;

	ExportFiguresAsPymolCGO()
	{
	}

	ExportFiguresAsPymolCGO& init(CommandInput& input)
	{
		figure_name=input.get_value_vector_or_default<std::string>("figure-name", std::vector<std::string>());
		representation_names=input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
		cgo_name=input.get_value_or_default<std::string>("cgo-name", "figures");
		file=input.get_value<std::string>("file");
		assert_file_name_input(file, false);
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_figures_availability();
		data_manager.assert_figures_representations_availability();

		assert_file_name_input(file, false);

		if(cgo_name.empty())
		{
			throw std::runtime_error(std::string("Missing object name."));
		}

		std::set<std::size_t> representation_ids=data_manager.figures_representation_descriptor().ids_by_names(representation_names);

		if(representation_ids.empty())
		{
			representation_ids.insert(0);
		}

		if(representation_ids.size()>1)
		{
			throw std::runtime_error(std::string("More than one representation requested."));
		}

		const std::set<std::size_t> ids=data_manager.filter_figures_drawable_implemented_ids(
				representation_ids,
				LongName::match(data_manager.figures(), LongName(figure_name)),
				false);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable figures selected."));
		}

		auxiliaries::OpenGLPrinter opengl_printer;
		{
			unsigned int prev_color=0;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::size_t id=(*it);
				for(std::set<std::size_t>::const_iterator jt=representation_ids.begin();jt!=representation_ids.end();++jt)
				{
					const std::size_t visual_id=(*jt);
					if(visual_id<data_manager.figures_display_states()[id].visuals.size())
					{
						const DataManager::DisplayState::Visual& dsv=data_manager.figures_display_states()[id].visuals[visual_id];
						if(prev_color==0 || dsv.color!=prev_color)
						{
							opengl_printer.add_color(dsv.color);
						}
						prev_color=dsv.color;
						const Figure& figure=data_manager.figures()[id];
						for(std::size_t i=0;i<figure.indices.size();i+=3)
						{
							std::vector<apollota::SimplePoint> vertices(3);
							std::vector<apollota::SimplePoint> normals(3);
							for(std::size_t j=0;j<3;j++)
							{
								const std::size_t v_id=(i+j);
								vertices[j].x=figure.vertices[v_id*3+0];
								vertices[j].y=figure.vertices[v_id*3+1];
								vertices[j].z=figure.vertices[v_id*3+2];
								normals[j].x=figure.normals[v_id*3+0];
								normals[j].y=figure.normals[v_id*3+1];
								normals[j].z=figure.normals[v_id*3+2];
							}
							opengl_printer.add_triangle_strip(vertices, normals);
						}
					}
				}
			}
		}

		OutputSelector output_selector(file);

		{
			std::ostream& output=output_selector.stream();
			assert_io_stream(file, output);
			opengl_printer.print_pymol_script(cgo_name, true, output);
		}

		Result result;
		result.file=file;
		if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
		{
			result.dump=output_selector.str();
		}

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_FIGURES_AS_PYMOL_CGO_H_ */
