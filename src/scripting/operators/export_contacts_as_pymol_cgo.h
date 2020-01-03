#ifndef SCRIPTING_OPERATORS_EXPORT_CONTACTS_AS_PYMOL_CGO_H_
#define SCRIPTING_OPERATORS_EXPORT_CONTACTS_AS_PYMOL_CGO_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class ExportContactsAsPymolCGO
{
public:
	struct Result
	{
		std::string file;
		std::string dump;
		SummaryOfContacts contacts_summary;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
			if(!dump.empty())
			{
				heterostorage.variant_object.value("dump")=dump;
			}
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			return (*this);
		}
	};

	std::string name;
	bool wireframe;
	SelectionManager::Query parameters_for_selecting;
	std::vector<std::string> representation_names;
	std::string file;

	ExportContactsAsPymolCGO() : wireframe(false)
	{
	}

	ExportContactsAsPymolCGO& init(CommandInput& input)
	{
		name=input.get_value_or_default<std::string>("name", "contacts");
		wireframe=input.get_flag("wireframe");
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		representation_names=input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
		file=input.get_value<std::string>("file");
		assert_file_name_input(file, false);
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();
		data_manager.assert_contacts_representations_availability();

		assert_file_name_input(file, false);

		if(name.empty())
		{
			throw std::runtime_error(std::string("Missing object name."));
		}

		std::set<std::size_t> representation_ids=data_manager.contacts_representation_descriptor().ids_by_names(representation_names);

		if(representation_ids.empty())
		{
			representation_ids.insert(0);
		}

		if(representation_ids.size()>1)
		{
			throw std::runtime_error(std::string("More than one representation requested."));
		}

		const std::set<std::size_t> ids=data_manager.filter_contacts_drawable_implemented_ids(
				representation_ids,
				data_manager.selection_manager().select_contacts(parameters_for_selecting),
				false);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable visible contacts selected."));
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
					if(visual_id<data_manager.contacts_display_states()[id].visuals.size())
					{
						const DataManager::DisplayState::Visual& dsv=data_manager.contacts_display_states()[id].visuals[visual_id];
						if(prev_color==0 || dsv.color!=prev_color)
						{
							opengl_printer.add_color(dsv.color);
						}
						prev_color=dsv.color;
						if(wireframe)
						{
							opengl_printer.add_as_wireframe(data_manager.contacts()[id].value.graphics);
						}
						else
						{
							opengl_printer.add(data_manager.contacts()[id].value.graphics);
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
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_CONTACTS_AS_PYMOL_CGO_H_ */
