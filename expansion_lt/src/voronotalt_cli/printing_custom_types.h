#ifndef VORONOTALT_PRINTING_CUSTOM_TYPES_H_
#define VORONOTALT_PRINTING_CUSTOM_TYPES_H_

#include <string>

#ifdef VORONOTALT_OPENMP
#include <omp.h>
#endif

#include "../voronotalt/radical_tessellation.h"
#include "../voronotalt/simplified_aw_tessellation.h"

#include "spheres_input.h"
#include "io_utilities.h"

namespace voronotalt
{

class PrintingCustomTypes
{
public:
	static void print_balls(
			const std::vector<SimpleSphere>& spheres,
			const std::vector<SphereLabeling::SphereLabel>& sphere_labels,
			const double probe,
			const bool with_header,
			std::string& output) noexcept
	{
		if(spheres.empty())
		{
			return;
		}
		const SphereLabeling::SphereLabel null_label;
		output.reserve(spheres.size()*60);
		if(with_header)
		{
			print_label_header("", output);
			string_append_cstring(output, "x\ty\tz\tradius\n");
		}
		for(std::size_t i=0;i<spheres.size();i++)
		{
			print_label((i<sphere_labels.size() ? sphere_labels[i] : null_label), false, false, output);
			string_append_char(output, '\t');
			string_append_double(output, spheres[i].p.x);
			string_append_char(output, '\t');
			string_append_double(output, spheres[i].p.y);
			string_append_char(output, '\t');
			string_append_double(output, spheres[i].p.z);
			string_append_char(output, '\t');
			string_append_double(output, (spheres[i].r-probe));
			string_append_char(output, '\n');
		}
	}

	template<class ContactsContainer>
	static void print_contacts(
			const ContactsContainer& contacts, const std::vector<SphereLabeling::SphereLabel>& sphere_labels, const bool labels_enabled, std::string& output) noexcept
	{
		const SphereLabeling::SphereLabel null_label;
		if(!contacts.empty())
		{
			output.reserve(contacts.size()*100);
			if(labels_enabled)
			{
				string_append_cstring(output, "ca_header\t");
				print_label_header("1", output);
				print_label_header("2", output);
				string_append_cstring(output, "ID1_index\tID2_index\tarea\tarc_legth\tdistance\n");
			}
			else
			{
				string_append_cstring(output, "ca_header\tID1_index\tID2_index\tarea\tarc_legth\tdistance\n");
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

						std::vector<std::string> suboutputs(n_threads);

						{
							#pragma omp parallel for schedule(static,1)
							for(int i=0;i<n_threads;i++)
							{
								for(int j=thread_data_starts[i];j<data_size && j<((i+1)<n_threads ? thread_data_starts[i+1] : data_size);j++)
								{
									print_contact(static_cast<std::size_t>(j), contacts, sphere_labels, null_label, labels_enabled, suboutputs[i]);
								}
							}
						}

						for(int i=0;i<n_threads;i++)
						{
							string_append_string(output, suboutputs[i]);
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
					print_contact(i, contacts, sphere_labels, null_label, labels_enabled, output);
				}
			}
		}
	}

	template<class ContactsContainer, class GroupedContactsContainer>
	static void print_contacts_residue_level(
			const ContactsContainer& contacts, const std::vector<SphereLabeling::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_contacts_representative_ids, const GroupedContactsContainer& grouped_contacts, std::string& output) noexcept
	{
		print_contacts_residue_level_or_chain_level(false, contacts, sphere_labels, grouped_contacts_representative_ids, grouped_contacts, output);
	}

	template<class ContactsContainer, class GroupedContactsContainer>
	static void print_contacts_chain_level(
			const ContactsContainer& contacts, const std::vector<SphereLabeling::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_contacts_representative_ids, const GroupedContactsContainer& grouped_contacts, std::string& output) noexcept
	{
		print_contacts_residue_level_or_chain_level(true, contacts, sphere_labels, grouped_contacts_representative_ids, grouped_contacts, output);
	}

	template<class CellsContainer>
	static void print_cells(
			const CellsContainer& cells, const std::vector<SphereLabeling::SphereLabel>& sphere_labels, const bool labels_enabled, std::string& output) noexcept
	{
		const SphereLabeling::SphereLabel null_label;
		if(!cells.empty())
		{
			output.reserve(cells.size()*100);
			if(labels_enabled)
			{
				string_append_cstring(output, "sa_header\t");
				print_label_header("", output);
				string_append_cstring(output, "ID_index\tsas_area\tvolume\n");
			}
			else
			{
				string_append_cstring(output, "sa_header\tID_index\tsas_area\tvolume\n");
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

						std::vector<std::string> suboutputs(n_threads);

						{
							#pragma omp parallel for schedule(static,1)
							for(int i=0;i<n_threads;i++)
							{
								for(int j=thread_data_starts[i];j<data_size && j<((i+1)<n_threads ? thread_data_starts[i+1] : data_size);j++)
								{
									print_sas_and_volume(static_cast<std::size_t>(j), cells, sphere_labels, labels_enabled, null_label, suboutputs[i]);
								}
							}
						}

						for(int i=0;i<n_threads;i++)
						{
							string_append_string(output, suboutputs[i]);
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
					print_sas_and_volume(i, cells, sphere_labels, labels_enabled, null_label, output);
				}
			}
		}
	}

	template<class CellsContainer, class GroupedCellsContainer>
	static void print_cells_residue_level(
			const CellsContainer& cells, const std::vector<SphereLabeling::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_cells_representative_ids, const GroupedCellsContainer& grouped_cells, std::string& output) noexcept
	{
		print_cells_residue_level_or_chain_level(false,  cells, sphere_labels, grouped_cells_representative_ids, grouped_cells, output);
	}

	template<class CellsContainer, class GroupedCellsContainer>
	static void print_cells_chain_level(
			const CellsContainer& cells, const std::vector<SphereLabeling::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_cells_representative_ids, const GroupedCellsContainer& grouped_cells, std::string& output) noexcept
	{
		print_cells_residue_level_or_chain_level(true,  cells, sphere_labels, grouped_cells_representative_ids, grouped_cells, output);
	}

	template<class SequentialContainer>
	inline static void print_sequential_container_simply(const SequentialContainer& v, std::string& output) noexcept
	{
		output.reserve(v.size()*80);
		for(typename SequentialContainer::const_iterator it=v.begin();it!=v.end();++it)
		{
			print((*it), output);
		}
	}

	inline static void print_label_header(const char* id_suffix, std::string& output) noexcept
	{
		string_append_cstring(output, "ID");
		string_append_cstring(output, id_suffix);
		string_append_cstring(output, "_chain\tID");
		string_append_cstring(output, id_suffix);
		string_append_cstring(output, "_rnum\tID");
		string_append_cstring(output, id_suffix);
		string_append_cstring(output, "_ic\tID");
		string_append_cstring(output, id_suffix);
		string_append_cstring(output, "_rname\tID");
		string_append_cstring(output, id_suffix);
		string_append_cstring(output, "_atom\t");
	}

	inline static void print_label(const SphereLabeling::SphereLabel& obj, const bool no_atom, const bool no_residue, std::string& output) noexcept
	{
		if(obj.chain_id.empty())
		{
			string_append_char(output, '.');
		}
		else
		{
			string_append_string(output, obj.chain_id);
		}

		string_append_char(output, '\t');

		if(no_residue || obj.residue_id.empty() || obj.residue_id==".")
		{
			string_append_cstring(output, ".\t.\t.");
		}
		else
		{
			if(obj.expanded_residue_id.parsed && obj.expanded_residue_id.valid)
			{
				string_append_int(output, obj.expanded_residue_id.rnum);
				string_append_char(output, '\t');
				if(obj.expanded_residue_id.icode.empty())
				{
					string_append_char(output, '.');
				}
				else
				{
					string_append_string(output, obj.expanded_residue_id.icode);
				}
				string_append_char(output, '\t');
				if(obj.expanded_residue_id.rname.empty())
				{
					string_append_char(output, '.');
				}
				else
				{
					string_append_string(output, obj.expanded_residue_id.rname);
				}
			}
			else
			{
				string_append_cstring(output, ".\t.\t");
				string_append_string(output, obj.residue_id);
			}
		}

		string_append_char(output, '\t');

		if(no_atom || obj.atom_name.empty())
		{
			string_append_char(output, '.');
		}
		else
		{
			string_append_string(output, obj.atom_name);
		}
	}

private:
	template<class ContactsContainer, class GroupedContactsContainer>
	static void print_contacts_residue_level_or_chain_level(
			const bool chain_level,
			const ContactsContainer& contacts,
			const std::vector<SphereLabeling::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_contacts_representative_ids,
			const GroupedContactsContainer& grouped_contacts,
			std::string& output) noexcept
	{
		const SphereLabeling::SphereLabel null_label;
		if(!grouped_contacts.empty())
		{
			string_append_cstring(output, (chain_level ? "cu" : "cr"));
			string_append_cstring(output, "_header\t");
			print_label_header("1", output);
			print_label_header("2", output);
			string_append_cstring(output, "area\tarc_legth\tdistance\tcount\n");
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

						std::vector<std::string> suboutputs(n_threads);

						{
							#pragma omp parallel for schedule(static,1)
							for(int i=0;i<n_threads;i++)
							{
								for(int j=thread_data_starts[i];j<data_size && j<((i+1)<n_threads ? thread_data_starts[i+1] : data_size);j++)
								{
									print_contact_residue_level_or_chain_level(static_cast<std::size_t>(j), chain_level, contacts, sphere_labels, grouped_contacts_representative_ids, grouped_contacts, null_label, suboutputs[i]);
								}
							}
						}

						for(int i=0;i<n_threads;i++)
						{
							string_append_string(output, suboutputs[i]);
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
					print_contact_residue_level_or_chain_level(i, chain_level, contacts, sphere_labels, grouped_contacts_representative_ids, grouped_contacts, null_label, output);
				}
			}
		}
	}

	template<class CellsContainer, class GroupedCellsContainer>
	static void print_cells_residue_level_or_chain_level(
			const bool chain_level,
			const CellsContainer& cells,
			const std::vector<SphereLabeling::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_cells_representative_ids,
			const GroupedCellsContainer& grouped_cells,
			std::string& output) noexcept
	{
		const SphereLabeling::SphereLabel null_label;
		if(!grouped_cells.empty())
		{
			string_append_cstring(output, (chain_level ? "su" : "sr"));
			string_append_cstring(output, "_header\t");
			print_label_header("", output);
			string_append_cstring(output, "sas_area\tvolume\tcount\n");
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

						std::vector<std::string> suboutputs(n_threads);

						{
							#pragma omp parallel for schedule(static,1)
							for(int i=0;i<n_threads;i++)
							{
								for(int j=thread_data_starts[i];j<data_size && j<((i+1)<n_threads ? thread_data_starts[i+1] : data_size);j++)
								{
									print_sas_and_volume_residue_level_or_chain_level(static_cast<std::size_t>(j), chain_level, cells, sphere_labels, grouped_cells_representative_ids, grouped_cells, null_label, suboutputs[i]);
								}
							}
						}

						for(int i=0;i<n_threads;i++)
						{
							string_append_string(output, suboutputs[i]);
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
					print_sas_and_volume_residue_level_or_chain_level(i, chain_level, cells, sphere_labels, grouped_cells_representative_ids, grouped_cells, null_label, output);
				}
			}
		}
	}

	template<class ContactsContainer>
	static void print_contact(
			const std::size_t i,
			const ContactsContainer& contacts,
			const std::vector<SphereLabeling::SphereLabel>& sphere_labels,
			const SphereLabeling::SphereLabel& null_label,
			const bool labels_enabled,
			std::string& output) noexcept
	{
		string_append_cstring(output, "ca\t");
		if(labels_enabled)
		{
			print_label((contacts[i].id_a<sphere_labels.size() ? sphere_labels[contacts[i].id_a] : null_label), false, false, output);
			string_append_char(output, '\t');
			print_label((contacts[i].id_b<sphere_labels.size() ? sphere_labels[contacts[i].id_b] : null_label), false, false, output);
			string_append_char(output, '\t');
		}
		print(contacts[i], output);
		string_append_char(output, '\n');
	}

	template<class ContactsContainer, class GroupedContactsContainer>
	static void print_contact_residue_level_or_chain_level(
			const std::size_t i,
			const bool chain_level,
			const ContactsContainer& contacts,
			const std::vector<SphereLabeling::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_contacts_representative_ids,
			const GroupedContactsContainer& grouped_contacts,
			const SphereLabeling::SphereLabel& null_label,
			std::string& output) noexcept
	{
		const std::size_t j=grouped_contacts_representative_ids[i];
		string_append_cstring(output, (chain_level ? "cu\t" : "cr\t"));
		const SphereLabeling::SphereLabel& sl1=(contacts[j].id_a<sphere_labels.size() ? sphere_labels[contacts[j].id_a] : null_label);
		const SphereLabeling::SphereLabel& sl2=(contacts[j].id_b<sphere_labels.size() ? sphere_labels[contacts[j].id_b] : null_label);
		const bool no_reverse=(sl1.chain_id<sl2.chain_id || (sl1.chain_id==sl2.chain_id && sl1.residue_id<sl2.residue_id));
		print_label((no_reverse ? sl1 : sl2), true, chain_level, output);
		string_append_char(output, '\t');
		print_label((no_reverse ? sl2 : sl1), true, chain_level, output);
		string_append_char(output, '\t');
		print(grouped_contacts[i], output);
		string_append_char(output, '\n');
	}

	template<class CellsContainer>
	static void print_sas_and_volume(
			const std::size_t i,
			const CellsContainer& cells,
			const std::vector<SphereLabeling::SphereLabel>& sphere_labels,
			const bool labels_enabled,
			const SphereLabeling::SphereLabel& null_label,
			std::string& output) noexcept
	{
		if(cells[i].stage>0)
		{
			string_append_cstring(output, "sa\t");
			if(labels_enabled)
			{
				print_label((cells[i].id<sphere_labels.size() ? sphere_labels[cells[i].id] : null_label), false, false, output);
				string_append_char(output, '\t');
			}
			print(cells[i], output);
			string_append_char(output, '\n');
		}
	}

	template<class CellsContainer, class GroupedCellsContainer>
	static void print_sas_and_volume_residue_level_or_chain_level(
			const std::size_t i,
			const bool chain_level,
			const CellsContainer& cells,
			const std::vector<SphereLabeling::SphereLabel>& sphere_labels,
			const std::vector<UnsignedInt>& grouped_cells_representative_ids,
			const GroupedCellsContainer& grouped_cells,
			const SphereLabeling::SphereLabel& null_label,
			std::string& output) noexcept
	{
		const std::size_t j=grouped_cells_representative_ids[i];
		string_append_cstring(output, (chain_level ? "su\t" : "sr\t"));
		print_label((cells[j].id<sphere_labels.size() ? sphere_labels[cells[j].id] : null_label), true, chain_level, output);
		string_append_char(output, '\t');
		print(grouped_cells[i], output);
		string_append_char(output, '\n');
	}

	inline static void print(const RadicalTessellation::ContactDescriptorSummary& obj, std::string& output) noexcept
	{
		string_append_int(output, obj.id_a);
		string_append_char(output, '\t');
		string_append_int(output, obj.id_b);
		string_append_char(output, '\t');
		string_append_double(output, obj.area);
		string_append_char(output, '\t');
		string_append_double(output, obj.arc_length);
		string_append_char(output, '\t');
		string_append_double(output, obj.distance);
	}

	inline static void print(const RadicalTessellation::TotalContactDescriptorsSummary& obj, std::string& output) noexcept
	{
		string_append_double(output, obj.area);
		string_append_char(output, '\t');
		string_append_double(output, obj.arc_length);
		string_append_char(output, '\t');
		string_append_double(output, obj.distance);
		string_append_char(output, '\t');
		string_append_int(output, obj.count);
	}

	inline static void print(const RadicalTessellation::CellContactDescriptorsSummary& obj, std::string& output) noexcept
	{
		string_append_int(output, obj.id);
		string_append_char(output, '\t');
		string_append_double(output, obj.sas_area);
		string_append_char(output, '\t');
		string_append_double(output, obj.sas_inside_volume);
	}

	inline static void print(const RadicalTessellation::TotalCellContactDescriptorsSummary& obj, std::string& output) noexcept
	{
		string_append_double(output, obj.sas_area);
		string_append_char(output, '\t');
		string_append_double(output, obj.sas_inside_volume);
		string_append_char(output, '\t');
		string_append_int(output, obj.count);
	}

	inline static void print(const SimplifiedAWTessellation::ContactDescriptorSummary& obj, std::string& output) noexcept
	{
		string_append_int(output, obj.id_a);
		string_append_char(output, '\t');
		string_append_int(output, obj.id_b);
		string_append_char(output, '\t');
		string_append_double(output, obj.area);
		string_append_char(output, '\t');
		string_append_double(output, obj.arc_length);
		string_append_char(output, '\t');
		string_append_double(output, obj.distance);
	}

	inline static void print(const SimplifiedAWTessellation::TotalContactDescriptorsSummary& obj, std::string& output) noexcept
	{
		string_append_double(output, obj.area);
		string_append_char(output, '\t');
		string_append_double(output, obj.arc_length);
		string_append_char(output, '\t');
		string_append_double(output, obj.distance);
		string_append_char(output, '\t');
		string_append_int(output, obj.count);
	}

	inline static void print(const RadicalTessellationContactConstruction::TessellationEdge& te, std::string& output) noexcept
	{
		string_append_int(output, te.ids_of_spheres[0]);
		string_append_char(output, '\t');
		string_append_int(output, te.ids_of_spheres[1]);
		string_append_char(output, '\t');
		if(te.ids_of_spheres[2]==null_id())
		{
			string_append_cstring(output, "-1");
		}
		else
		{
			string_append_int(output, te.ids_of_spheres[2]);
		}
		string_append_char(output, '\t');
		string_append_double(output, te.length);
		string_append_char(output, '\n');
	}

	inline static void print(const RadicalTessellationContactConstruction::TessellationVertex& tv, std::string& output) noexcept
	{
		string_append_int(output, tv.ids_of_spheres[0]);
		string_append_char(output, '\t');
		string_append_int(output, tv.ids_of_spheres[1]);
		string_append_char(output, '\t');
		if(tv.ids_of_spheres[2]==null_id())
		{
			string_append_cstring(output, "-1");
		}
		else
		{
			string_append_int(output, tv.ids_of_spheres[2]);
		}
		string_append_char(output, '\t');
		if(tv.ids_of_spheres[3]==null_id())
		{
			string_append_cstring(output, "-1");
		}
		else
		{
			string_append_int(output, tv.ids_of_spheres[3]);
		}
		string_append_char(output, '\t');
		string_append_double(output, tv.position.x);
		string_append_char(output, '\t');
		string_append_double(output, tv.position.y);
		string_append_char(output, '\t');
		string_append_double(output, tv.position.z);
		string_append_char(output, '\t');
		string_append_double(output, tv.dist_min);
		string_append_char(output, '\t');
		string_append_double(output, tv.dist_max);
		string_append_char(output, '\n');
	}
};


}

#endif /* VORONOTALT_PRINTING_CUSTOM_TYPES_H_ */
