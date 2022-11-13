#ifndef DUKTAPER_OPERATORS_PLOT_CONTACTS_MAP_H_
#define DUKTAPER_OPERATORS_PLOT_CONTACTS_MAP_H_

#include "../operators_common.h"
#include "../../../src/auxiliaries/color_utilities.h"
#include "../dependencies/lodepng/lodepng.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class PlotContactsMap : public scripting::OperatorBase<PlotContactsMap>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		std::string file;
		scripting::SummaryOfContacts contacts_summary;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
			scripting::VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
		}
	};

	scripting::SelectionManager::Query parameters_for_selecting;
	std::vector<std::string> representation_names;
	std::string file;
	int fixed_plot_size;
	bool on_white;

	PlotContactsMap() : fixed_plot_size(0), on_white(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		parameters_for_selecting=scripting::OperatorsUtilities::read_generic_selecting_query(input);
		representation_names=input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
		fixed_plot_size=input.get_value_or_default<int>("fixed-plot-size", 0);
		on_white=input.get_flag("on-white");
		file=input.get_value<std::string>("file");
		scripting::assert_file_name_input(file, false);
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		scripting::OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("rep", CDOD::DATATYPE_STRING_ARRAY, "representation names", ""));
		doc.set_option_decription(CDOD("fixed-plot-size", CDOD::DATATYPE_INT, "fixed side length of output plot", ""));
		doc.set_option_decription(CDOD("on-white", CDOD::DATATYPE_BOOL, "flag to use white background", false));
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to output PNG file"));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();
		data_manager.assert_contacts_representations_availability();

		if(fixed_plot_size!=0 && fixed_plot_size<3)
		{
			throw std::runtime_error(std::string("Invalid fixed side length of output plot, must be at least 3."));
		}

		scripting::assert_file_name_input(file, false);
		scripting::assert_extension_in_file_name_input(file, ".png");

		std::set<std::size_t> representation_ids=data_manager.contacts_representation_descriptor().ids_by_names(representation_names);

		if(representation_ids.empty())
		{
			representation_ids.insert(0);
		}

		if(representation_ids.size()>1)
		{
			throw std::runtime_error(std::string("More than one representation requested."));
		}

		const std::set<std::size_t> contact_ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);

		if(contact_ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		std::map< common::ChainResidueAtomDescriptorsPair, std::map<unsigned int, double> > map_of_inter_residue_contacts_color_weights;
		for(std::set<std::size_t>::const_iterator it=contact_ids.begin();it!=contact_ids.end();++it)
		{
			const std::size_t contact_id=(*it);
			const scripting::Contact& contact=data_manager.contacts()[contact_id];
			const common::ChainResidueAtomDescriptorsPair crads=common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), contact);
			if(crads.valid())
			{
				std::map<unsigned int, double>& color_weights=map_of_inter_residue_contacts_color_weights[crads.without_some_info(true, true, false, true)];
				for(std::set<std::size_t>::const_iterator jt=representation_ids.begin();jt!=representation_ids.end();++jt)
				{
					const std::size_t visual_id=(*jt);
					if(visual_id<data_manager.contacts_display_states()[contact_id].visuals.size())
					{
						const scripting::DataManager::DisplayState::Visual& dsv=data_manager.contacts_display_states()[contact_id].visuals[visual_id];
						color_weights[dsv.color]+=contact.value.area;
					}
				}
			}
		}

		std::map<common::ChainResidueAtomDescriptorsPair, unsigned int> map_of_inter_residue_contacts_colors;
		for(std::map< common::ChainResidueAtomDescriptorsPair, std::map<unsigned int, double> >::const_iterator it=map_of_inter_residue_contacts_color_weights.begin();it!=map_of_inter_residue_contacts_color_weights.end();++it)
		{
			const std::map<unsigned int, double>& color_weights=it->second;
			if(!color_weights.empty())
			{
				std::map<unsigned int, double>::const_iterator jt=color_weights.begin();
				unsigned int max_area_color=jt->first;
				double max_area=jt->second;
				++jt;
				for(;jt!=color_weights.end();++jt)
				{
					if(max_area<(jt->second))
					{
						max_area_color=jt->first;
						max_area=jt->second;
					}
				}
				map_of_inter_residue_contacts_colors[it->first]=max_area_color;
			}
		}

		std::map<common::ChainResidueAtomDescriptor, int> map_of_crads_to_positions;
		int max_position=0;
		{
			std::map<std::string, int> map_of_chains_to_max_residue_numbers;
			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				const scripting::Atom& atom=data_manager.atoms()[i];
				if(atom.crad.resSeq>0)
				{
					map_of_chains_to_max_residue_numbers[atom.crad.chainID]=std::max(map_of_chains_to_max_residue_numbers[atom.crad.chainID], atom.crad.resSeq);
				}
			}
			std::map<std::string, int> map_of_chains_to_axis_position_offsets;
			{
				int offset=0;
				for(std::map<std::string, int>::const_iterator it=map_of_chains_to_max_residue_numbers.begin();it!=map_of_chains_to_max_residue_numbers.end();++it)
				{
					map_of_chains_to_axis_position_offsets[it->first]=offset;
					offset+=it->second;
				}
				max_position=offset;
			}
			for(std::map<common::ChainResidueAtomDescriptorsPair, unsigned int>::const_iterator it=map_of_inter_residue_contacts_colors.begin();it!=map_of_inter_residue_contacts_colors.end();++it)
			{
				for(int j=0;j<2;j++)
				{
					const common::ChainResidueAtomDescriptor& crad=(j==0 ? it->first.a : it->first.b);
					if(crad.resSeq>0)
					{
						const int position=crad.resSeq+map_of_chains_to_axis_position_offsets[crad.chainID];
						max_position=std::max(max_position, position);
						map_of_crads_to_positions[crad]=position;
					}
				}
			}
		}

		const int plot_size=(fixed_plot_size>0 ? fixed_plot_size : max_position);

		if(plot_size<3)
		{
			throw std::runtime_error(std::string("Invalid side length of output plot, must be at least 3."));
		}

		{
			std::vector<unsigned char> png_image_data(plot_size*plot_size*4, (on_white ? 255 : 0));
			for(std::size_t i=3;i<png_image_data.size();i+=4)
			{
				png_image_data[i]=255;
			}

			for(std::map<common::ChainResidueAtomDescriptorsPair, unsigned int>::const_iterator it=map_of_inter_residue_contacts_colors.begin();it!=map_of_inter_residue_contacts_colors.end();++it)
			{
				int x=map_of_crads_to_positions[it->first.a]-1;
				if(x>=0 && x<plot_size)
				{
					int y=map_of_crads_to_positions[it->first.b]-1;
					if(y>=0 && y<plot_size)
					{
						const unsigned int color=it->second;
						auxiliaries::ColorUtilities::color_to_components<unsigned char>(color, &(png_image_data[4*(y*plot_size+x)]), false);
						auxiliaries::ColorUtilities::color_to_components<unsigned char>(color, &(png_image_data[4*(x*plot_size+y)]), false);
					}
				}
			}

			unsigned int error=lodepng::encode(file, png_image_data, static_cast<unsigned int>(plot_size), static_cast<unsigned int>(plot_size));
			if(error>0)
			{
				throw std::runtime_error(std::string("PNG encoding failed with error '")+lodepng_error_text(error)+"'.");
			}
		}


		Result result;
		result.file=file;
		result.contacts_summary=scripting::SummaryOfContacts(data_manager.contacts(), contact_ids);

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_PLOT_CONTACTS_MAP_H_ */
