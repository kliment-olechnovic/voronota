#ifndef VORONOTALT_PRINTING_CUSTOM_TYPES_H_
#define VORONOTALT_PRINTING_CUSTOM_TYPES_H_

#include <sstream>

#ifdef _OPENMP
#include <omp.h>
#endif

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
		if(!contacts.empty())
		{
			bool printed_in_parallel=false;
#ifdef _OPENMP
			if(contacts.size()>10000)
			{
				const int data_size=static_cast<int>(contacts.size());
				const int n_threads=omp_get_max_threads();
				if(n_threads>1)
				{
					const int approximate_portion_size=(data_size/n_threads);
					if(approximate_portion_size>1000)
					{
						std::vector<int> thread_data_starts(n_threads, 0);
						for(int i=1;i<n_threads;i++)
						{
							thread_data_starts[i]=thread_data_starts[i-1]+approximate_portion_size;
						}

						std::vector<std::ostringstream> suboutputs(n_threads);

						#pragma omp parallel
						{
							#pragma omp for schedule(static,1)
							for(int i=0;i<n_threads;i++)
							{
								for(int j=thread_data_starts[i];j<data_size && j<((i+1)<n_threads ? thread_data_starts[i+1] : data_size);j++)
								{
									print_contact_to_stream(static_cast<std::size_t>(j), contacts, sphere_labels, null_label, labels_enabled, suboutputs[i]);
								}
							}
						}

						for(int i=0;i<n_threads;i++)
						{
							output << suboutputs[i].str();
						}

						printed_in_parallel=true;
					}
				}
			}
#endif
			if(!printed_in_parallel)
			{
				for(std::size_t i=0;i<contacts.size();i++)
				{
					print_contact_to_stream(i, contacts, sphere_labels, null_label, labels_enabled, output);
				}
			}
		}
	}

	template<class ContactsContainer, class GroupedContactsContainer>
	static void print_contacts_residue_level_to_stream(
			const ContactsContainer& contacts, const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_contacts_representative_ids, const GroupedContactsContainer& grouped_contacts, std::ostream& output)
	{
		const SpheresInput::SphereLabel null_label;
		for(std::size_t i=0;i<grouped_contacts.size();i++)
		{
			print_contact_residue_level_or_chain_level_to_stream(i, false, contacts, sphere_labels, grouped_contacts_representative_ids, grouped_contacts, null_label, output);
		}
	}

	template<class ContactsContainer, class GroupedContactsContainer>
	static void print_contacts_chain_level_to_stream(
			const ContactsContainer& contacts, const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_contacts_representative_ids, const GroupedContactsContainer& grouped_contacts, std::ostream& output)
	{
		const SpheresInput::SphereLabel null_label;
		for(std::size_t i=0;i<grouped_contacts.size();i++)
		{
			print_contact_residue_level_or_chain_level_to_stream(i, true, contacts, sphere_labels, grouped_contacts_representative_ids, grouped_contacts, null_label, output);
		}
	}

	template<class CellsContainer>
	static void print_sas_and_volumes_to_stream(
			const CellsContainer& cells, const std::vector<SpheresInput::SphereLabel>& sphere_labels, const bool labels_enabled, std::ostream& output)
	{
		const SpheresInput::SphereLabel null_label;
		if(!cells.empty())
		{
			bool printed_in_parallel=false;
#ifdef _OPENMP
			if(cells.size()>10000)
			{
				const int data_size=static_cast<int>(cells.size());
				const int n_threads=omp_get_max_threads();
				if(n_threads>1)
				{
					const int approximate_portion_size=(data_size/n_threads);
					if(approximate_portion_size>1000)
					{
						std::vector<int> thread_data_starts(n_threads, 0);
						for(int i=1;i<n_threads;i++)
						{
							thread_data_starts[i]=thread_data_starts[i-1]+approximate_portion_size;
						}

						std::vector<std::ostringstream> suboutputs(n_threads);

						#pragma omp parallel
						{
							#pragma omp for schedule(static,1)
							for(int i=0;i<n_threads;i++)
							{
								for(int j=thread_data_starts[i];j<data_size && j<((i+1)<n_threads ? thread_data_starts[i+1] : data_size);j++)
								{
									print_sas_and_volume_to_stream(static_cast<std::size_t>(j), cells, sphere_labels, labels_enabled, null_label, suboutputs[i]);
								}
							}
						}

						for(int i=0;i<n_threads;i++)
						{
							output << suboutputs[i].str();
						}

						printed_in_parallel=true;
					}
				}
			}
#endif
			if(!printed_in_parallel)
			{
				for(std::size_t i=0;i<cells.size();i++)
				{
					print_sas_and_volume_to_stream(i, cells, sphere_labels, labels_enabled, null_label, output);
				}
			}
		}
	}

	template<class CellsContainer, class GroupedCellsContainer>
	static void print_sas_and_volumes_residue_level_to_stream(
			const CellsContainer& cells, const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_cells_representative_ids, const GroupedCellsContainer& grouped_cells, std::ostream& output)
	{
		const SpheresInput::SphereLabel null_label;
		for(std::size_t i=0;i<grouped_cells.size();i++)
		{
			print_sas_and_volume_residue_level_or_chain_level_to_stream(i, false, cells, sphere_labels, grouped_cells_representative_ids, grouped_cells, null_label, output);
		}
	}

	template<class CellsContainer, class GroupedCellsContainer>
	static void print_sas_and_volumes_chain_level_to_stream(
			const CellsContainer& cells, const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_cells_representative_ids, const GroupedCellsContainer& grouped_cells, std::ostream& output)
	{
		const SpheresInput::SphereLabel null_label;
		for(std::size_t i=0;i<grouped_cells.size();i++)
		{
			print_sas_and_volume_residue_level_or_chain_level_to_stream(i, true, cells, sphere_labels, grouped_cells_representative_ids, grouped_cells, null_label, output);
		}
	}

private:
	template<class ContactsContainer>
	static void print_contact_to_stream(
			const std::size_t i,
			const ContactsContainer& contacts,
			const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const SpheresInput::SphereLabel& null_label,
			const bool labels_enabled,
			std::ostream& output)
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

	template<class ContactsContainer, class GroupedContactsContainer>
	static void print_contact_residue_level_or_chain_level_to_stream(
			const std::size_t i,
			const bool chain_level,
			const ContactsContainer& contacts,
			const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_contacts_representative_ids,
			const GroupedContactsContainer& grouped_contacts,
			const SpheresInput::SphereLabel& null_label,
			std::ostream& output)
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

	template<class CellsContainer>
	static void print_sas_and_volume_to_stream(
			const std::size_t i,
			const CellsContainer& cells,
			const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const bool labels_enabled,
			const SpheresInput::SphereLabel& null_label,
			std::ostream& output)
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

	template<class CellsContainer, class GroupedCellsContainer>
	static void print_sas_and_volume_residue_level_or_chain_level_to_stream(
			const std::size_t i,
			const bool chain_level,
			const CellsContainer& cells,
			const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_cells_representative_ids,
			const GroupedCellsContainer& grouped_cells,
			const SpheresInput::SphereLabel& null_label,
			std::ostream& output)
	{
		const std::size_t j=grouped_cells_representative_ids[i];
		output << (chain_level ? "su " : "sr ");
		print_label((cells[j].id<sphere_labels.size() ? sphere_labels[cells[j].id] : null_label), true, chain_level, output);
		output << " ";
		print(grouped_cells[i], output);
		output << "\n";
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
