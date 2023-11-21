#ifndef VORONOTALT_PRINTING_CUSTOM_TYPES_H_
#define VORONOTALT_PRINTING_CUSTOM_TYPES_H_

#include <iostream>

#include "spheres_input.h"
#include "tessellation_full_construction.h"
#include "simplified_aw_tessellation_full_construction.h"

namespace voronotalt
{

class PrintingCustomTypes
{
public:
	template<class ContactsContainer>
	static void print_contacts_to_stream(
			const ContactsContainer& contacts, const std::vector<SpheresInput::SphereLabel>& sphere_labels, const bool labels_enabled, std::ostream& output)
	{
		const SpheresInput::SphereLabel null_label;
		for(std::size_t i=0;i<contacts.size();i++)
		{
			output << "ca ";
			if(labels_enabled)
			{
				print_label((contacts[i].id_a<sphere_labels.size() ? sphere_labels[contacts[i].id_a] : null_label), false, false, output);
				output << " ";
				print_label((contacts[i].id_b<sphere_labels.size() ? sphere_labels[contacts[i].id_b] : null_label), false, false, output);
				output << " ";
			}
			print(contacts[i], output);
			output << "\n";
		}
	}

	template<class ContactsContainer, class GroupedContactsContainer>
	static void print_contacts_residue_level_to_stream(
			const ContactsContainer& contacts, const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_contacts_representative_ids, const GroupedContactsContainer& grouped_contacts, std::ostream& output)
	{
		print_contacts_residue_level_or_chain_level_to_stream(false, contacts, sphere_labels, grouped_contacts_representative_ids, grouped_contacts, output);
	}

	template<class ContactsContainer, class GroupedContactsContainer>
	static void print_contacts_chain_level_to_stream(
			const ContactsContainer& contacts, const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_contacts_representative_ids, const GroupedContactsContainer& grouped_contacts, std::ostream& output)
	{
		print_contacts_residue_level_or_chain_level_to_stream(true, contacts, sphere_labels, grouped_contacts_representative_ids, grouped_contacts, output);
	}

	template<class CellsContainer>
	static void print_sas_and_volumes_to_stream(
			const CellsContainer& cells, const std::vector<SpheresInput::SphereLabel>& sphere_labels, const bool labels_enabled, std::ostream& output)
	{
		const SpheresInput::SphereLabel null_label;
		for(std::size_t i=0;i<cells.size();i++)
		{
			output << "sa ";
			if(labels_enabled)
			{
				print_label((cells[i].id<sphere_labels.size() ? sphere_labels[cells[i].id] : null_label), false, false, output);
				output << " ";
			}
			print(cells[i], output);
			output << "\n";
		}
	}

	template<class CellsContainer, class GroupedCellsContainer>
	static void print_sas_and_volumes_residue_level_to_stream(
			const CellsContainer& cells, const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_cells_representative_ids, const GroupedCellsContainer& grouped_cells, std::ostream& output)
	{
		print_sas_and_volumes_residue_level_or_chain_level_to_stream(false, cells, sphere_labels, grouped_cells_representative_ids, grouped_cells, output);
	}

	template<class CellsContainer, class GroupedCellsContainer>
	static void print_sas_and_volumes_chain_level_to_stream(
			const CellsContainer& cells, const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_cells_representative_ids, const GroupedCellsContainer& grouped_cells, std::ostream& output)
	{
		print_sas_and_volumes_residue_level_or_chain_level_to_stream(true, cells, sphere_labels, grouped_cells_representative_ids, grouped_cells, output);
	}

private:
	template<class ContactsContainer, class GroupedContactsContainer>
	static void print_contacts_residue_level_or_chain_level_to_stream(const bool chain_level, const ContactsContainer& contacts, const std::vector<SpheresInput::SphereLabel>& sphere_labels, const std::vector<UnsignedInt>& grouped_contacts_representative_ids, const GroupedContactsContainer& grouped_contacts, std::ostream& output)
	{
		const SpheresInput::SphereLabel null_label;
		for(std::size_t i=0;i<grouped_contacts.size();i++)
		{
			const std::size_t j=grouped_contacts_representative_ids[i];
			output << (chain_level ? "cu " : "cr ");
			const SpheresInput::SphereLabel& sl1=(contacts[j].id_a<sphere_labels.size() ? sphere_labels[contacts[j].id_a] : null_label);
			const SpheresInput::SphereLabel& sl2=(contacts[j].id_b<sphere_labels.size() ? sphere_labels[contacts[j].id_b] : null_label);
			const bool no_reverse=(sl1.chain_id<sl2.chain_id || (sl1.chain_id==sl2.chain_id && sl1.residue_id<sl2.residue_id));
			print_label((no_reverse ? sl1 : sl2), true, chain_level, output);
			output << " ";
			print_label((no_reverse ? sl2 : sl1), true, chain_level, output);
			output << " ";
			print(grouped_contacts[i], output);
			output << "\n";
		}
	}

	template<class CellsContainer, class GroupedCellsContainer>
	static void print_sas_and_volumes_residue_level_or_chain_level_to_stream(const bool chain_level, const CellsContainer& cells, const std::vector<SpheresInput::SphereLabel>& sphere_labels, const std::vector<UnsignedInt>& grouped_cells_representative_ids, const GroupedCellsContainer& grouped_cells, std::ostream& output)
	{
		const SpheresInput::SphereLabel null_label;
		for(std::size_t i=0;i<grouped_cells.size();i++)
		{
			const std::size_t j=grouped_cells_representative_ids[i];
			output << (chain_level ? "su " : "sr ");
			print_label((cells[j].id<sphere_labels.size() ? sphere_labels[cells[j].id] : null_label), true, chain_level, output);
			output << " ";
			print(grouped_cells[i], output);
			output << "\n";
		}
	}

	inline static void print_label(const SpheresInput::SphereLabel& obj, const bool no_atom, const bool no_residue, std::ostream& output)
	{
		if(obj.chain_id.empty())
		{
			output << ".";
		}
		else
		{
			output << obj.chain_id;
		}

		output << " ";

		if(no_residue || obj.residue_id.empty())
		{
			output << ".";
		}
		else
		{
			output << obj.residue_id;
		}

		output << " ";

		if(no_atom || obj.atom_name.empty())
		{
			output << ".";
		}
		else
		{
			output << obj.atom_name;
		}
	}

	inline static void print(const TessellationFullConstruction::ContactDescriptorSummary& obj, std::ostream& output)
	{
		output << obj.id_a << " " <<  obj.id_b << " " << obj.area << " " << obj.arc_length << " " << obj.solid_angle_a << " " << obj.solid_angle_b;
	}

	inline static void print(const TessellationFullConstruction::TotalContactDescriptorsSummary& obj, std::ostream& output)
	{
		output << obj.area << " " << obj.arc_length << " " << obj.count;
	}

	inline static void print(const TessellationFullConstruction::CellContactDescriptorsSummary& obj, std::ostream& output)
	{
		output << obj.id << " " << obj.sas_area << " " << obj.sas_inside_volume;
	}

	inline static void print(const TessellationFullConstruction::TotalCellContactDescriptorsSummary& obj, std::ostream& output)
	{
		output << obj.sas_area << " " << obj.sas_inside_volume << " " << obj.count;
	}

	inline static void print(const SimplifiedAWTessellationFullConstruction::ContactDescriptorSummary& obj, std::ostream& output)
	{
		output << obj.id_a << " " <<  obj.id_b << " " << obj.area;
	}

	inline static void print(const SimplifiedAWTessellationFullConstruction::TotalContactDescriptorsSummary& obj, std::ostream& output)
	{
		output << obj.area << " " << obj.count;
	}
};


}

#endif /* VORONOTALT_PRINTING_CUSTOM_TYPES_H_ */
