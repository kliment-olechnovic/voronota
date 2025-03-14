#ifndef VORONOTALT_PRINTING_CUSTOM_TYPES_H_
#define VORONOTALT_PRINTING_CUSTOM_TYPES_H_

#include <sstream>

#ifdef VORONOTALT_OPENMP
#include <omp.h>
#endif

#include "../voronotalt/radical_tessellation.h"
#include "../voronotalt/simplified_aw_tessellation.h"

#include "spheres_input.h"

namespace voronotalt
{

class PrintingCustomTypes
{
public:
	static void print_balls_to_stream(
			const std::vector<SimpleSphere>& spheres,
			const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const double probe,
			std::ostream& output) noexcept
	{
		const SpheresInput::SphereLabel null_label;
		for(std::size_t i=0;i<spheres.size();i++)
		{
			print_label((i<sphere_labels.size() ? sphere_labels[i] : null_label), false, false, output);
			output << "\t";
			output << spheres[i].p.x << "\t" << spheres[i].p.y << "\t" << spheres[i].p.z << "\t" << (spheres[i].r-probe);
			output << "\n";
		}
	}

	template<class ContactsContainer>
	static void print_contacts_to_stream(
			const ContactsContainer& contacts, const std::vector<SpheresInput::SphereLabel>& sphere_labels, const bool labels_enabled, std::ostream& output) noexcept
	{
		const SpheresInput::SphereLabel null_label;
		if(!contacts.empty())
		{
			if(labels_enabled)
			{
				output << "ca_header\tID1_chain\tID1_residue\tID1_atom\tID2_chain\tID2_residue\tID2_atom\tID1_index\tID2_index\tarea\tarc_legth\tdistance\n";
			}
			else
			{
				output << "ca_header\tID1_index\tID2_index\tarea\tarc_legth\tdistance\n";
			}
			bool printed_in_parallel=false;
#ifdef VORONOTALT_OPENMP
			if(contacts.size()>1000)
			{
				const int data_size=static_cast<int>(contacts.size());
				const int n_threads=omp_get_max_threads();
				if(n_threads>1)
				{
					const int approximate_portion_size=(data_size/n_threads);
					if(approximate_portion_size>100)
					{
						std::vector<int> thread_data_starts(n_threads, 0);
						for(int i=1;i<n_threads;i++)
						{
							thread_data_starts[i]=thread_data_starts[i-1]+approximate_portion_size;
						}

						std::vector<std::ostringstream> suboutputs(n_threads);

						{
							#pragma omp parallel for schedule(static,1)
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
			const std::vector<UnsignedInt>& grouped_contacts_representative_ids, const GroupedContactsContainer& grouped_contacts, std::ostream& output) noexcept
	{
		print_contacts_residue_level_or_chain_level_to_stream(false, contacts, sphere_labels, grouped_contacts_representative_ids, grouped_contacts, output);
	}

	template<class ContactsContainer, class GroupedContactsContainer>
	static void print_contacts_chain_level_to_stream(
			const ContactsContainer& contacts, const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_contacts_representative_ids, const GroupedContactsContainer& grouped_contacts, std::ostream& output) noexcept
	{
		print_contacts_residue_level_or_chain_level_to_stream(true, contacts, sphere_labels, grouped_contacts_representative_ids, grouped_contacts, output);
	}

	template<class CellsContainer>
	static void print_cells_to_stream(
			const CellsContainer& cells, const std::vector<SpheresInput::SphereLabel>& sphere_labels, const bool labels_enabled, std::ostream& output) noexcept
	{
		const SpheresInput::SphereLabel null_label;
		if(!cells.empty())
		{
			if(labels_enabled)
			{
				output << "sa_header\tID_chain\tID_residue\tID_atom\tID_index\tsas_area\tvolume\n";
			}
			else
			{
				output << "sa_header\tID_index\tsas_area\tvolume\n";
			}
			bool printed_in_parallel=false;
#ifdef VORONOTALT_OPENMP
			if(cells.size()>1000)
			{
				const int data_size=static_cast<int>(cells.size());
				const int n_threads=omp_get_max_threads();
				if(n_threads>1)
				{
					const int approximate_portion_size=(data_size/n_threads);
					if(approximate_portion_size>100)
					{
						std::vector<int> thread_data_starts(n_threads, 0);
						for(int i=1;i<n_threads;i++)
						{
							thread_data_starts[i]=thread_data_starts[i-1]+approximate_portion_size;
						}

						std::vector<std::ostringstream> suboutputs(n_threads);

						{
							#pragma omp parallel for schedule(static,1)
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
	static void print_cells_residue_level_to_stream(
			const CellsContainer& cells, const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_cells_representative_ids, const GroupedCellsContainer& grouped_cells, std::ostream& output) noexcept
	{
		print_cells_residue_level_or_chain_level_to_stream(false,  cells, sphere_labels, grouped_cells_representative_ids, grouped_cells, output);
	}

	template<class CellsContainer, class GroupedCellsContainer>
	static void print_cells_chain_level_to_stream(
			const CellsContainer& cells, const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_cells_representative_ids, const GroupedCellsContainer& grouped_cells, std::ostream& output) noexcept
	{
		print_cells_residue_level_or_chain_level_to_stream(true,  cells, sphere_labels, grouped_cells_representative_ids, grouped_cells, output);
	}

	template<class Result, class GroupedResult>
	static void print_tessellation_full_construction_result_log_basic(const Result& result, const GroupedResult& result_grouped_by_residue, const GroupedResult& result_grouped_by_chain, std::ostream& output) noexcept
	{
		output << "log_total_input_balls\t" << result.total_spheres << "\n";
		output << "log_total_collisions\t" << result.total_collisions << "\n";
		output << "log_total_relevant_collisions\t" << result.total_relevant_collisions << "\n";
		output << "log_total_contacts_count\t" << result.total_contacts_summary.count << "\n";
		output << "log_total_contacts_area\t" << result.total_contacts_summary.area << "\n";
		output << "log_total_residue_level_contacts_count\t" << result_grouped_by_residue.grouped_contacts_summaries.size() << "\n";
		output << "log_total_chain_level_contacts_count\t" << result_grouped_by_chain.grouped_contacts_summaries.size() << "\n";
	}

	template<class Result, class GroupedResult>
	static void print_tessellation_full_construction_result_log_about_cells(const Result& result, const GroupedResult& result_grouped_by_residue, const GroupedResult& result_grouped_by_chain, std::ostream& output) noexcept
	{
		output << "log_total_cells_count\t" << result.total_cells_summary.count << "\n";
		output << "log_total_cells_sas_area\t" << result.total_cells_summary.sas_area << "\n";
		output << "log_total_cells_sas_inside_volume\t" << result.total_cells_summary.sas_inside_volume << "\n";
		output << "log_total_residue_level_cells_count\t" << result_grouped_by_residue.grouped_cells_summaries.size() << "\n";
		output << "log_total_chain_level_cells_count\t" << result_grouped_by_chain.grouped_cells_summaries.size() << "\n";
	}

	inline static void print_label(const SpheresInput::SphereLabel& obj, const bool no_atom, const bool no_residue, std::ostream& output) noexcept
	{
		if(obj.chain_id.empty())
		{
			output << ".";
		}
		else
		{
			output << obj.chain_id;
		}

		output << "\t";

		if(no_residue || obj.residue_id.empty())
		{
			output << ".";
		}
		else
		{
			output << obj.residue_id;
		}

		output << "\t";

		if(no_atom || obj.atom_name.empty())
		{
			output << ".";
		}
		else
		{
			output << obj.atom_name;
		}
	}

	template<class SequentialContainer>
	inline static void print_sequential_container_simply(const SequentialContainer& v, std::ostream& output) noexcept
	{
		for(typename SequentialContainer::const_iterator it=v.begin();it!=v.end();++it)
		{
			print((*it), output);
		}
	}

private:
	template<class ContactsContainer, class GroupedContactsContainer>
	static void print_contacts_residue_level_or_chain_level_to_stream(
			const bool chain_level,
			const ContactsContainer& contacts,
			const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_contacts_representative_ids,
			const GroupedContactsContainer& grouped_contacts,
			std::ostream& output) noexcept
	{
		const SpheresInput::SphereLabel null_label;
		if(!grouped_contacts.empty())
		{
			output << (chain_level ? "cu" : "cr") << "_header\tID1_chain\tID1_residue\tID1_atom\tID2_chain\tID2_residue\tID2_atom\tarea\tarc_legth\tdistance\tcount\n";
			bool printed_in_parallel=false;
#ifdef VORONOTALT_OPENMP
			if(grouped_contacts.size()>1000)
			{
				const int data_size=static_cast<int>(grouped_contacts.size());
				const int n_threads=omp_get_max_threads();
				if(n_threads>1)
				{
					const int approximate_portion_size=(data_size/n_threads);
					if(approximate_portion_size>100)
					{
						std::vector<int> thread_data_starts(n_threads, 0);
						for(int i=1;i<n_threads;i++)
						{
							thread_data_starts[i]=thread_data_starts[i-1]+approximate_portion_size;
						}

						std::vector<std::ostringstream> suboutputs(n_threads);

						{
							#pragma omp parallel for schedule(static,1)
							for(int i=0;i<n_threads;i++)
							{
								for(int j=thread_data_starts[i];j<data_size && j<((i+1)<n_threads ? thread_data_starts[i+1] : data_size);j++)
								{
									print_contact_residue_level_or_chain_level_to_stream(static_cast<std::size_t>(j), chain_level, contacts, sphere_labels, grouped_contacts_representative_ids, grouped_contacts, null_label, suboutputs[i]);
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
				for(std::size_t i=0;i<grouped_contacts.size();i++)
				{
					print_contact_residue_level_or_chain_level_to_stream(i, chain_level, contacts, sphere_labels, grouped_contacts_representative_ids, grouped_contacts, null_label, output);
				}
			}
		}
	}

	template<class CellsContainer, class GroupedCellsContainer>
	static void print_cells_residue_level_or_chain_level_to_stream(
			const bool chain_level,
			const CellsContainer& cells,
			const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_cells_representative_ids,
			const GroupedCellsContainer& grouped_cells,
			std::ostream& output) noexcept
	{
		const SpheresInput::SphereLabel null_label;
		if(!grouped_cells.empty())
		{
			output << (chain_level ? "su" : "sr") << "_header\tID_chain\tID_residue\tID_atom\tsas_area\tvolume\tcount\n";
			bool printed_in_parallel=false;
#ifdef VORONOTALT_OPENMP
			if(grouped_cells.size()>1000)
			{
				const int data_size=static_cast<int>(grouped_cells.size());
				const int n_threads=omp_get_max_threads();
				if(n_threads>1)
				{
					const int approximate_portion_size=(data_size/n_threads);
					if(approximate_portion_size>100)
					{
						std::vector<int> thread_data_starts(n_threads, 0);
						for(int i=1;i<n_threads;i++)
						{
							thread_data_starts[i]=thread_data_starts[i-1]+approximate_portion_size;
						}

						std::vector<std::ostringstream> suboutputs(n_threads);

						{
							#pragma omp parallel for schedule(static,1)
							for(int i=0;i<n_threads;i++)
							{
								for(int j=thread_data_starts[i];j<data_size && j<((i+1)<n_threads ? thread_data_starts[i+1] : data_size);j++)
								{
									print_sas_and_volume_residue_level_or_chain_level_to_stream(static_cast<std::size_t>(j), chain_level, cells, sphere_labels, grouped_cells_representative_ids, grouped_cells, null_label, suboutputs[i]);
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
				for(std::size_t i=0;i<grouped_cells.size();i++)
				{
					print_sas_and_volume_residue_level_or_chain_level_to_stream(i, chain_level, cells, sphere_labels, grouped_cells_representative_ids, grouped_cells, null_label, output);
				}
			}
		}
	}

	template<class ContactsContainer>
	static void print_contact_to_stream(
			const std::size_t i,
			const ContactsContainer& contacts,
			const std::vector<SpheresInput::SphereLabel>& sphere_labels,
			const SpheresInput::SphereLabel& null_label,
			const bool labels_enabled,
			std::ostream& output) noexcept
	{
		output << "ca\t";
		if(labels_enabled)
		{
			print_label((contacts[i].id_a<sphere_labels.size() ? sphere_labels[contacts[i].id_a] : null_label), false, false, output);
			output << "\t";
			print_label((contacts[i].id_b<sphere_labels.size() ? sphere_labels[contacts[i].id_b] : null_label), false, false, output);
			output << "\t";
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
			std::ostream& output) noexcept
	{
		const std::size_t j=grouped_contacts_representative_ids[i];
		output << (chain_level ? "cu\t" : "cr\t");
		const SpheresInput::SphereLabel& sl1=(contacts[j].id_a<sphere_labels.size() ? sphere_labels[contacts[j].id_a] : null_label);
		const SpheresInput::SphereLabel& sl2=(contacts[j].id_b<sphere_labels.size() ? sphere_labels[contacts[j].id_b] : null_label);
		const bool no_reverse=(sl1.chain_id<sl2.chain_id || (sl1.chain_id==sl2.chain_id && sl1.residue_id<sl2.residue_id));
		print_label((no_reverse ? sl1 : sl2), true, chain_level, output);
		output << "\t";
		print_label((no_reverse ? sl2 : sl1), true, chain_level, output);
		output << "\t";
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
			std::ostream& output) noexcept
	{
		if(cells[i].stage>0)
		{
			output << "sa\t";
			if(labels_enabled)
			{
				print_label((cells[i].id<sphere_labels.size() ? sphere_labels[cells[i].id] : null_label), false, false, output);
				output << "\t";
			}
			print(cells[i], output);
			output << "\n";
		}
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
			std::ostream& output) noexcept
	{
		const std::size_t j=grouped_cells_representative_ids[i];
		output << (chain_level ? "su\t" : "sr\t");
		print_label((cells[j].id<sphere_labels.size() ? sphere_labels[cells[j].id] : null_label), true, chain_level, output);
		output << "\t";
		print(grouped_cells[i], output);
		output << "\n";
	}

	inline static void print(const RadicalTessellation::ContactDescriptorSummary& obj, std::ostream& output) noexcept
	{
		output << obj.id_a << "\t" <<  obj.id_b << "\t" << obj.area << "\t" << obj.arc_length << "\t" << obj.distance;
	}

	inline static void print(const RadicalTessellation::TotalContactDescriptorsSummary& obj, std::ostream& output) noexcept
	{
		output << obj.area << "\t" << obj.arc_length << "\t" << obj.distance << "\t" << obj.count;
	}

	inline static void print(const RadicalTessellation::CellContactDescriptorsSummary& obj, std::ostream& output) noexcept
	{
		output << obj.id << "\t" << obj.sas_area << "\t" << obj.sas_inside_volume;
	}

	inline static void print(const RadicalTessellation::TotalCellContactDescriptorsSummary& obj, std::ostream& output) noexcept
	{
		output << obj.sas_area << "\t" << obj.sas_inside_volume << "\t" << obj.count;
	}

	inline static void print(const SimplifiedAWTessellation::ContactDescriptorSummary& obj, std::ostream& output) noexcept
	{
		output << obj.id_a << "\t" <<  obj.id_b << "\t" << obj.area << "\t" << obj.arc_length << "\t" << obj.distance;
	}

	inline static void print(const SimplifiedAWTessellation::TotalContactDescriptorsSummary& obj, std::ostream& output) noexcept
	{
		output << obj.area << "\t" << obj.arc_length << "\t" << obj.distance << "\t" << obj.count;
	}

	inline static void print(const RadicalTessellationContactConstruction::TessellationEdge& te, std::ostream& output) noexcept
	{
		output << te.ids_of_spheres[0] << "\t" << te.ids_of_spheres[1] << "\t";
		if(te.ids_of_spheres[2]==null_id())
		{
			output << "-1";
		}
		else
		{
			output << te.ids_of_spheres[2];
		}
		output << "\t" << te.length << "\n";
	}

	inline static void print(const RadicalTessellationContactConstruction::TessellationVertex& tv, std::ostream& output) noexcept
	{
		output << tv.ids_of_spheres[0] << "\t" << tv.ids_of_spheres[1] << "\t";
		if(tv.ids_of_spheres[2]==null_id())
		{
			output << "-1";
		}
		else
		{
			output << tv.ids_of_spheres[2];
		}
		output << "\t";
		if(tv.ids_of_spheres[3]==null_id())
		{
			output << "-1";
		}
		else
		{
			output << tv.ids_of_spheres[3];
		}
		output << "\t" << tv.position.x << "\t" << tv.position.y << "\t" << tv.position.z << "\t" << tv.dist_min << "\t" << tv.dist_max << "\n";
	}
};


}

#endif /* VORONOTALT_PRINTING_CUSTOM_TYPES_H_ */
