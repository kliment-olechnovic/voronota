#ifndef SCRIPTING_VCBLOCKS_OF_DATA_MANAGER_H_
#define SCRIPTING_VCBLOCKS_OF_DATA_MANAGER_H_

#include "data_manager.h"

namespace voronota
{

namespace scripting
{

class VCBlocksOfDataManager
{
public:
	static std::size_t null_id()
	{
		return std::numeric_limits<std::size_t>::max();
	}

	struct Parameters
	{
		int seq_sep_threshold;
		std::string selection_of_contacts_for_recording_blocks;
		std::vector<std::string> names_of_raw_values_describing_residues;
		std::vector<std::string> names_of_raw_values_describing_rr_contacts_far;
		std::vector<std::string> names_of_raw_values_describing_rr_contacts_near;

		Parameters() : seq_sep_threshold(2), selection_of_contacts_for_recording_blocks("[ --min-seq-sep 6 ]")
		{
			names_of_raw_values_describing_residues.push_back("sas_area");
			names_of_raw_values_describing_rr_contacts_far.push_back("area");
			names_of_raw_values_describing_rr_contacts_near.push_back("area");
		}

		bool names_filled() const
		{
			return (!names_of_raw_values_describing_residues.empty() && !names_of_raw_values_describing_rr_contacts_far.empty() && !names_of_raw_values_describing_rr_contacts_near.empty());
		}
	};

	class Standardizer
	{
	public:
		std::vector<double> values_of_means;
		std::vector<double> values_of_sds;

		static const Standardizer& get_default_standardizer()
		{
			return get_default_standardizer_mutable();
		}

		static void reset_default_standardizer()
		{
			get_default_standardizer_mutable()=Standardizer();
		}

		static bool setup_default_standardizer(const std::string& file_of_means, const std::string& file_of_sds)
		{
			if(file_of_means.empty() || file_of_sds.empty())
			{
				return false;
			}

			Standardizer standardizer;
			standardizer.input_files.push_back(file_of_means);
			standardizer.input_files.push_back(file_of_sds);

			if(!get_default_standardizer().empty() && get_default_standardizer().input_files==standardizer.input_files)
			{
				return true;
			}

			voronota::auxiliaries::IOUtilities().read_file_lines_to_set(file_of_means, standardizer.values_of_means);
			voronota::auxiliaries::IOUtilities().read_file_lines_to_set(file_of_means, standardizer.values_of_sds);

			if(standardizer.values_of_means.empty() || standardizer.values_of_means.size()!=standardizer.values_of_sds.size())
			{
				return false;
			}

			get_default_standardizer_mutable().values_of_means.swap(standardizer.values_of_means);
			get_default_standardizer_mutable().values_of_sds.swap(standardizer.values_of_sds);
			get_default_standardizer_mutable().input_files.swap(standardizer.input_files);

			return true;
		}

		bool empty() const
		{
			return (values_of_means.empty() && values_of_sds.empty());
		}

		bool valid(const std::size_t input_length) const
		{
			return (values_of_means.size()==input_length && values_of_means.size()==values_of_sds.size());
		}

		void apply(const std::vector<double>& input_values, std::vector<double>& output_values) const
		{
			if(!valid(input_values.size()))
			{
				throw std::runtime_error(std::string("Invalid standardizer for vcblocks."));
			}
			output_values.resize(input_values.size(), 0.0);
			for(std::size_t i=0;i<input_values.size();i++)
			{
				const double mean=values_of_means[i];
				const double sd=values_of_sds[i];
				if(sd<=0.0)
				{
					output_values[i]=0.0;
				}
				else
				{
					output_values[i]=(input_values[i]-mean)/sd;
				}
			}
		}

	private:
		std::vector<std::string> input_files;

		static Standardizer& get_default_standardizer_mutable()
		{
			static Standardizer standarizer;
			return standarizer;
		}
	};

	struct ResidueDescriptor
	{
		int atoms_count;
		double volume;
		double contacts_area_for_seq_sep_1;
		double contacts_area_for_seq_sep_2_plus;
		double sas_area;

		ResidueDescriptor() : atoms_count(0), volume(0.0), contacts_area_for_seq_sep_1(0.0), contacts_area_for_seq_sep_2_plus(0.0), sas_area(0.0)
		{
		}
	};

	struct RRContactDescriptor
	{
		std::size_t rr_pair[2];
		std::vector<std::size_t> aa_contact_ids;
		int seq_sep_class;

		RRContactDescriptor() : seq_sep_class(0)
		{
		}
	};

	struct VCBlock
	{
		bool recorded;
		std::size_t rr_contact_descriptor_id_main;
		std::size_t residue_id_main[2];
		std::vector<std::size_t> residue_ids_surrounding;
		std::vector<std::size_t> rr_contact_descriptor_ids_surrounding[2];
		std::vector<std::size_t> rr_contact_descriptor_ids_capping[2];
		std::vector< std::vector<std::size_t> > rr_contact_descriptor_ids_paracapping;
		std::vector<double> angles_of_surrounding_residues;
		std::vector<double> adjacency_lengths_of_surrounding_residues;
		std::vector<double> full_encoding;
		std::vector<double> standardized_encoding;

		VCBlock() : recorded(false), rr_contact_descriptor_id_main(null_id())
		{
		}
	};

	struct Result
	{
		Parameters used_params;
		std::vector<ResidueDescriptor> residue_descriptors;
		std::vector<RRContactDescriptor> rr_contact_descriptors;
		std::vector<std::size_t> map_of_aa_contact_ids_to_rr_contact_descriptors;
		std::vector<VCBlock> vcblocks;
		std::vector<std::size_t> indices_of_recorded_vcblocks;
		std::vector< std::vector<double> > raw_values_for_residues;
		std::vector< std::vector<double> > raw_values_for_rr_contacts;
		std::vector<std::string> header_for_vcblock_encodings;

		Result()
		{
		}
	};

	static void construct_result(const Parameters& params, DataManager& data_manager, Result& result)
	{
		result=Result();
		result.used_params=params;

		data_manager.assert_primary_structure_info_valid();
		data_manager.assert_contacts_availability();
		data_manager.assert_contacts_adjacencies_availability();

		result.residue_descriptors.resize(data_manager.primary_structure_info().residues.size());

		for(std::size_t i=0;i<result.residue_descriptors.size();i++)
		{
			const common::ConstructionOfPrimaryStructure::Residue& res=data_manager.primary_structure_info().residues[i];
			ResidueDescriptor& rd=result.residue_descriptors[i];
			for(std::size_t j=0;j<res.atom_ids.size();j++)
			{
				const Atom& atom=data_manager.atoms()[res.atom_ids[j]];
				rd.atoms_count++;
				std::map<std::string, double>::const_iterator it=atom.value.props.adjuncts.find("volume");
				if(it!=atom.value.props.adjuncts.end())
				{
					rd.volume+=it->second;
				}
			}
		}

		for(std::size_t i=0;i<data_manager.contacts().size();i++)
		{
			const Contact& contact=data_manager.contacts()[i];
			if(contact.solvent())
			{
				std::size_t residue_id=data_manager.primary_structure_info().map_of_atoms_to_residues[contact.ids[0]];
				result.residue_descriptors[residue_id].sas_area+=contact.value.area;
			}
			else
			{
				std::size_t residue_pair_ids[2];
				residue_pair_ids[0]=data_manager.primary_structure_info().map_of_atoms_to_residues[contact.ids[0]];
				residue_pair_ids[1]=data_manager.primary_structure_info().map_of_atoms_to_residues[contact.ids[1]];
				if(residue_pair_ids[0]!=residue_pair_ids[1])
				{
					const common::ConstructionOfPrimaryStructure::Residue& res_a=data_manager.primary_structure_info().residues[residue_pair_ids[0]];
					const common::ConstructionOfPrimaryStructure::Residue& res_b=data_manager.primary_structure_info().residues[residue_pair_ids[1]];
					if(res_a.segment_id==res_b.segment_id && std::abs(res_a.position_in_segment-res_b.position_in_segment)<2)
					{
						result.residue_descriptors[residue_pair_ids[0]].contacts_area_for_seq_sep_1+=contact.value.area;
						result.residue_descriptors[residue_pair_ids[1]].contacts_area_for_seq_sep_1+=contact.value.area;
					}
					else
					{
						result.residue_descriptors[residue_pair_ids[0]].contacts_area_for_seq_sep_2_plus+=contact.value.area;
						result.residue_descriptors[residue_pair_ids[1]].contacts_area_for_seq_sep_2_plus+=contact.value.area;
					}
				}
			}
		}

		const std::set<std::size_t> selected_aa_contact_ids_for_recording_blocks=data_manager.selection_manager().select_contacts(SelectionManager::Query(params.selection_of_contacts_for_recording_blocks, false));

		if(selected_aa_contact_ids_for_recording_blocks.empty())
		{
			throw std::runtime_error(std::string("No atom-atom contacts selected to define recording."));
		}

		std::map< std::vector<std::size_t>,  std::vector<std::size_t> > map_of_rr_pairs_to_aa_contact_ids;

		for(std::size_t i=0;i<data_manager.contacts().size();i++)
		{
			const Contact& contact=data_manager.contacts()[i];
			if(!contact.solvent())
			{
				std::vector<std::size_t> residue_pair_ids(2);
				residue_pair_ids[0]=data_manager.primary_structure_info().map_of_atoms_to_residues[contact.ids[0]];
				residue_pair_ids[1]=data_manager.primary_structure_info().map_of_atoms_to_residues[contact.ids[1]];
				if(residue_pair_ids[0]!=residue_pair_ids[1])
				{
					std::sort(residue_pair_ids.begin(), residue_pair_ids.end());
					map_of_rr_pairs_to_aa_contact_ids[residue_pair_ids].push_back(i);
				}
			}
		}

		result.rr_contact_descriptors.resize(map_of_rr_pairs_to_aa_contact_ids.size());

		std::map< std::vector<std::size_t>, std::size_t > map_of_rr_pairs_to_rr_contact_descriptors;

		{
			std::size_t i=0;
			for(std::map< std::vector<std::size_t>,  std::vector<std::size_t> >::const_iterator it=map_of_rr_pairs_to_aa_contact_ids.begin();it!=map_of_rr_pairs_to_aa_contact_ids.end();++it)
			{
				RRContactDescriptor& rrcd=result.rr_contact_descriptors[i];
				rrcd.rr_pair[0]=it->first[0];
				rrcd.rr_pair[1]=it->first[1];
				rrcd.aa_contact_ids=it->second;
				rrcd.seq_sep_class=0;
				{
					const common::ConstructionOfPrimaryStructure::Residue& res_a=data_manager.primary_structure_info().residues[rrcd.rr_pair[0]];
					const common::ConstructionOfPrimaryStructure::Residue& res_b=data_manager.primary_structure_info().residues[rrcd.rr_pair[1]];
					if(res_a.segment_id==res_b.segment_id && std::abs(res_a.position_in_segment-res_b.position_in_segment)<params.seq_sep_threshold)
					{
						rrcd.seq_sep_class=1;
					}
				}
				map_of_rr_pairs_to_rr_contact_descriptors[it->first]=i;
				i++;
			}
		}

		result.map_of_aa_contact_ids_to_rr_contact_descriptors.resize(data_manager.contacts().size(), null_id());

		for(std::size_t i=0;i<result.rr_contact_descriptors.size();i++)
		{
			const RRContactDescriptor& rrcd=result.rr_contact_descriptors[i];
			for(std::size_t j=0;j<rrcd.aa_contact_ids.size();j++)
			{
				result.map_of_aa_contact_ids_to_rr_contact_descriptors[rrcd.aa_contact_ids[j]]=i;
			}
		}

		std::vector< std::set<std::size_t> > graph_of_residues(data_manager.primary_structure_info().residues.size());

		std::vector< std::set<std::size_t> > map_of_residues_to_rr_contact_descriptors(data_manager.primary_structure_info().residues.size());

		for(std::size_t i=0;i<result.rr_contact_descriptors.size();i++)
		{
			const RRContactDescriptor& rrcd=result.rr_contact_descriptors[i];
			graph_of_residues[rrcd.rr_pair[0]].insert(rrcd.rr_pair[1]);
			graph_of_residues[rrcd.rr_pair[1]].insert(rrcd.rr_pair[0]);
			map_of_residues_to_rr_contact_descriptors[rrcd.rr_pair[0]].insert(i);
			map_of_residues_to_rr_contact_descriptors[rrcd.rr_pair[1]].insert(i);
		}

		result.vcblocks.resize(result.rr_contact_descriptors.size());
		result.indices_of_recorded_vcblocks.reserve(result.vcblocks.size());

		for(std::size_t i=0;i<result.rr_contact_descriptors.size();i++)
		{
			const RRContactDescriptor& rrcd=result.rr_contact_descriptors[i];
			VCBlock& vcblock=result.vcblocks[i];

			for(std::size_t j=0;j<rrcd.aa_contact_ids.size() && !vcblock.recorded;j++)
			{
				if(selected_aa_contact_ids_for_recording_blocks.count(rrcd.aa_contact_ids[j])>0)
				{
					vcblock.recorded=true;
				}
			}

			if(vcblock.recorded)
			{
				result.indices_of_recorded_vcblocks.push_back(i);

				vcblock.residue_id_main[0]=rrcd.rr_pair[0];
				vcblock.residue_id_main[1]=rrcd.rr_pair[1];
				vcblock.rr_contact_descriptor_id_main=i;

				std::set<std::size_t> shared_neighbors;
				std::set<std::size_t> unique_neighbors[2];
				{
					std::map<std::size_t, int> neighboring_residue_counts;
					for(int j=0;j<2;j++)
					{
						std::set<std::size_t>& neighbors=graph_of_residues[vcblock.residue_id_main[j]];
						for(std::set<std::size_t>::const_iterator it=neighbors.begin();it!=neighbors.end();++it)
						{
							neighboring_residue_counts[*it]++;
						}
					}
					for(std::map<std::size_t, int>::const_iterator it=neighboring_residue_counts.begin();it!=neighboring_residue_counts.end();++it)
					{
						if(it->second==2)
						{
							shared_neighbors.insert(it->first);
						}
					}
					for(int j=0;j<2;j++)
					{
						std::set<std::size_t>& neighbors=graph_of_residues[vcblock.residue_id_main[j]];
						for(std::set<std::size_t>::const_iterator it=neighbors.begin();it!=neighbors.end();++it)
						{
							if((*it)!=vcblock.residue_id_main[j==0 ? 1 : 0] && shared_neighbors.count(*it)==0)
							{
								unique_neighbors[j].insert(*it);
							}
						}
					}
				}

				vcblock.residue_ids_surrounding.insert(vcblock.residue_ids_surrounding.end(), shared_neighbors.begin(), shared_neighbors.end());

				for(int j=0;j<2;j++)
				{
					vcblock.rr_contact_descriptor_ids_surrounding[j].reserve(vcblock.residue_ids_surrounding.size());
					for(std::vector<std::size_t>::const_iterator it=vcblock.residue_ids_surrounding.begin();it!=vcblock.residue_ids_surrounding.end();++it)
					{
						std::vector<std::size_t> residue_pair_ids(2);
						residue_pair_ids[0]=vcblock.residue_id_main[j];
						residue_pair_ids[1]=(*it);
						std::sort(residue_pair_ids.begin(), residue_pair_ids.end());
						vcblock.rr_contact_descriptor_ids_surrounding[j].push_back(map_of_rr_pairs_to_rr_contact_descriptors[residue_pair_ids]);
					}
					vcblock.rr_contact_descriptor_ids_capping[j].reserve(unique_neighbors[j].size());
					for(std::set<std::size_t>::const_iterator it=unique_neighbors[j].begin();it!=unique_neighbors[j].end();++it)
					{
						std::vector<std::size_t> residue_pair_ids(2);
						residue_pair_ids[0]=vcblock.residue_id_main[j];
						residue_pair_ids[1]=(*it);
						std::sort(residue_pair_ids.begin(), residue_pair_ids.end());
						vcblock.rr_contact_descriptor_ids_capping[j].push_back(map_of_rr_pairs_to_rr_contact_descriptors[residue_pair_ids]);
					}
				}

				vcblock.rr_contact_descriptor_ids_paracapping.resize(vcblock.residue_ids_surrounding.size());

				for(std::size_t j=0;j<vcblock.residue_ids_surrounding.size();j++)
				{
					const std::set<std::size_t>& relevant_rr_contact_descriptors=map_of_residues_to_rr_contact_descriptors[vcblock.residue_ids_surrounding[j]];
					vcblock.rr_contact_descriptor_ids_paracapping[j].reserve(relevant_rr_contact_descriptors.size());
					for(std::set<std::size_t>::const_iterator it=relevant_rr_contact_descriptors.begin();it!=relevant_rr_contact_descriptors.end();++it)
					{
						const RRContactDescriptor& rrcd=result.rr_contact_descriptors[*it];
						if(rrcd.rr_pair[0]!=vcblock.residue_id_main[0] && rrcd.rr_pair[0]!=vcblock.residue_id_main[1] && rrcd.rr_pair[1]!=vcblock.residue_id_main[0] && rrcd.rr_pair[1]!=vcblock.residue_id_main[1])
						{
							vcblock.rr_contact_descriptor_ids_paracapping[j].push_back(*it);
						}
					}
				}

				{
					vcblock.angles_of_surrounding_residues.resize(vcblock.residue_ids_surrounding.size(), 0.0);

					if(vcblock.residue_ids_surrounding.size()>1)
					{
						apollota::SimplePoint main_residue_points[2]={apollota::SimplePoint(), apollota::SimplePoint()};
						{
							std::map< std::size_t, std::vector<std::size_t> > map_of_residues_to_atoms_involved;
							for(std::vector<std::size_t>::const_iterator it=rrcd.aa_contact_ids.begin();it!=rrcd.aa_contact_ids.end();++it)
							{
								const Contact& contact=data_manager.contacts()[*it];
								map_of_residues_to_atoms_involved[data_manager.primary_structure_info().map_of_atoms_to_residues[contact.ids[0]]].push_back(contact.ids[0]);
								map_of_residues_to_atoms_involved[data_manager.primary_structure_info().map_of_atoms_to_residues[contact.ids[1]]].push_back(contact.ids[1]);
							}
							for(int j=0;j<2;j++)
							{
								const std::vector<std::size_t>& ids_of_atoms_involved=map_of_residues_to_atoms_involved[rrcd.rr_pair[j]];
								for(std::size_t e=0;e<ids_of_atoms_involved.size();e++)
								{
									main_residue_points[j]=main_residue_points[j]+apollota::SimplePoint(data_manager.atoms()[ids_of_atoms_involved[e]].value);
								}
								main_residue_points[j]=main_residue_points[j]*(1.0/static_cast<double>(ids_of_atoms_involved.size()));
							}
						}

						std::vector<apollota::SimplePoint> surroinding_residue_points(vcblock.residue_ids_surrounding.size());
						{
							std::map< std::size_t, std::vector<std::size_t> > map_of_residues_to_atoms_involved;
							for(int j=0;j<2;j++)
							{
								for(std::size_t e=0;e<vcblock.rr_contact_descriptor_ids_surrounding[j].size();e++)
								{
									const RRContactDescriptor& srrcd=result.rr_contact_descriptors[vcblock.rr_contact_descriptor_ids_surrounding[j][e]];
									for(std::vector<std::size_t>::const_iterator it=srrcd.aa_contact_ids.begin();it!=srrcd.aa_contact_ids.end();++it)
									{
										const Contact& contact=data_manager.contacts()[*it];
										map_of_residues_to_atoms_involved[data_manager.primary_structure_info().map_of_atoms_to_residues[contact.ids[0]]].push_back(contact.ids[0]);
										map_of_residues_to_atoms_involved[data_manager.primary_structure_info().map_of_atoms_to_residues[contact.ids[1]]].push_back(contact.ids[1]);
									}
								}
							}
							for(std::size_t j=0;j<vcblock.residue_ids_surrounding.size();j++)
							{
								const std::vector<std::size_t>& ids_of_atoms_involved=map_of_residues_to_atoms_involved[vcblock.residue_ids_surrounding[j]];
								for(std::size_t e=0;e<ids_of_atoms_involved.size();e++)
								{
									surroinding_residue_points[j]=surroinding_residue_points[j]+apollota::SimplePoint(data_manager.atoms()[ids_of_atoms_involved[e]].value);
								}
								surroinding_residue_points[j]=surroinding_residue_points[j]*(1.0/static_cast<double>(ids_of_atoms_involved.size()));
							}
						}

						{
							const apollota::SimplePoint o=(main_residue_points[0]+main_residue_points[1])*0.5;
							const apollota::SimplePoint& c=main_residue_points[1];
							const apollota::SimplePoint& a=surroinding_residue_points[0];
							for(std::size_t j=1;j<vcblock.residue_ids_surrounding.size();j++)
							{
								const apollota::SimplePoint& b=surroinding_residue_points[j];
								vcblock.angles_of_surrounding_residues[j]=apollota::directed_angle(o, a, b, c);
							}
						}
					}
				}

				{
					vcblock.adjacency_lengths_of_surrounding_residues.resize(vcblock.residue_ids_surrounding.size(), 0.0);
					std::map<std::size_t, std::size_t> map_of_residue_ids_to_surrounding_indices;
					for(std::size_t j=0;j<vcblock.residue_ids_surrounding.size();j++)
					{
						map_of_residue_ids_to_surrounding_indices[vcblock.residue_ids_surrounding[j]]=j;
					}
					for(std::vector<std::size_t>::const_iterator it=rrcd.aa_contact_ids.begin();it!=rrcd.aa_contact_ids.end();++it)
					{
						const std::map<std::size_t, double>& map_of_adjacencies=data_manager.contacts_adjacencies()[*it];
						for(std::map<std::size_t, double>::const_iterator jt=map_of_adjacencies.begin();jt!=map_of_adjacencies.end();++jt)
						{
							const Contact& contact=data_manager.contacts()[jt->first];
							std::size_t residue_pair_ids[2];
							residue_pair_ids[0]=data_manager.primary_structure_info().map_of_atoms_to_residues[contact.ids[0]];
							residue_pair_ids[1]=data_manager.primary_structure_info().map_of_atoms_to_residues[contact.ids[1]];
							if(!((rrcd.rr_pair[0]==residue_pair_ids[0] && rrcd.rr_pair[1]==residue_pair_ids[1]) || (rrcd.rr_pair[0]==residue_pair_ids[1] && rrcd.rr_pair[1]==residue_pair_ids[0])))
							{
								for(int j=0;j<2;j++)
								{
									std::map<std::size_t, std::size_t>::const_iterator s_it=map_of_residue_ids_to_surrounding_indices.find(residue_pair_ids[j]);
									if(s_it!=map_of_residue_ids_to_surrounding_indices.end())
									{
										vcblock.adjacency_lengths_of_surrounding_residues[s_it->second]+=jt->second*0.5;
									}
								}
							}
						}
					}
				}
			}
		}

		if(params.names_filled())
		{
			std::vector<int> involvement_of_residue_descriptors(result.residue_descriptors.size(), 0);
			std::vector<int> involvement_of_rr_contact_descriptors(result.rr_contact_descriptors.size(), 0);

			for(std::size_t i=0;i<result.indices_of_recorded_vcblocks.size();i++)
			{
				VCBlock& vcblock=result.vcblocks[result.indices_of_recorded_vcblocks[i]];
				involvement_of_rr_contact_descriptors[vcblock.rr_contact_descriptor_id_main]=1;
				involvement_of_residue_descriptors[vcblock.residue_id_main[0]]=1;
				involvement_of_residue_descriptors[vcblock.residue_id_main[1]]=1;
				for(int e=0;e<2;e++)
				{
					for(std::vector<std::size_t>::const_iterator it=vcblock.rr_contact_descriptor_ids_capping[e].begin();it!=vcblock.rr_contact_descriptor_ids_capping[e].end();++it)
					{
						involvement_of_rr_contact_descriptors[*it]=1;
					}
				}
				for(std::size_t j=0;j<vcblock.residue_ids_surrounding.size();j++)
				{
					involvement_of_residue_descriptors[vcblock.residue_ids_surrounding[j]]=1;
					involvement_of_rr_contact_descriptors[vcblock.rr_contact_descriptor_ids_surrounding[0][j]];
					involvement_of_rr_contact_descriptors[vcblock.rr_contact_descriptor_ids_surrounding[1][j]];
					for(std::vector<std::size_t>::const_iterator it=vcblock.rr_contact_descriptor_ids_paracapping[j].begin();it!=vcblock.rr_contact_descriptor_ids_paracapping[j].end();++it)
					{
						involvement_of_rr_contact_descriptors[*it]=1;
					}
				}
			}

			result.raw_values_for_residues.resize(result.residue_descriptors.size(), std::vector<double>(params.names_of_raw_values_describing_residues.size(), 0.0));

			for(std::size_t i=0;i<result.residue_descriptors.size();i++)
			{
				if(involvement_of_residue_descriptors[i]>0)
				{
					const ResidueDescriptor& rd=result.residue_descriptors[i];
					for(std::size_t j=0;j<params.names_of_raw_values_describing_residues.size();j++)
					{
						const std::string& name=params.names_of_raw_values_describing_residues[j];
						double& value=result.raw_values_for_residues[i][j];
						if(name=="atoms_count")
						{
							value=rd.atoms_count;
						}
						else if(name=="volume")
						{
							value=rd.volume;
						}
						else if(name=="area_near")
						{
							value=rd.contacts_area_for_seq_sep_1;
						}
						else if(name=="area_far")
						{
							value=rd.contacts_area_for_seq_sep_2_plus;
						}
						else if(name=="sas_area")
						{
							value=rd.sas_area;
						}
					}
				}
			}

			std::vector<std::string> combined_names_of_raw_values_describing_rr_contacts;
			combined_names_of_raw_values_describing_rr_contacts.reserve(params.names_of_raw_values_describing_rr_contacts_far.size()+params.names_of_raw_values_describing_rr_contacts_near.size());

			for(std::size_t i=0;i<params.names_of_raw_values_describing_rr_contacts_far.size();i++)
			{
				combined_names_of_raw_values_describing_rr_contacts.push_back(params.names_of_raw_values_describing_rr_contacts_far[i]+std::string("_far"));
			}

			for(std::size_t i=0;i<params.names_of_raw_values_describing_rr_contacts_near.size();i++)
			{
				combined_names_of_raw_values_describing_rr_contacts.push_back(params.names_of_raw_values_describing_rr_contacts_near[i]+std::string("_near"));
			}

			result.raw_values_for_rr_contacts.resize(result.rr_contact_descriptors.size(), std::vector<double>(combined_names_of_raw_values_describing_rr_contacts.size(), 0.0));

			for(std::size_t i=0;i<result.rr_contact_descriptors.size();i++)
			{
				if(involvement_of_rr_contact_descriptors[i]>0)
				{
					const RRContactDescriptor& rrcd=result.rr_contact_descriptors[i];
					for(std::size_t j=0;j<combined_names_of_raw_values_describing_rr_contacts.size();j++)
					{
						if((rrcd.seq_sep_class==0 && j<params.names_of_raw_values_describing_rr_contacts_far.size()) || (rrcd.seq_sep_class!=0 && j>=params.names_of_raw_values_describing_rr_contacts_far.size()))
						{
							const std::string& name=(rrcd.seq_sep_class==0 ? params.names_of_raw_values_describing_rr_contacts_far[j] : params.names_of_raw_values_describing_rr_contacts_near[j-params.names_of_raw_values_describing_rr_contacts_far.size()]);
							double& value=result.raw_values_for_rr_contacts[i][j];
							for(std::size_t e=0;e<rrcd.aa_contact_ids.size();e++)
							{
								const Contact& contact=data_manager.contacts()[rrcd.aa_contact_ids[e]];
								if(name=="area")
								{
									value+=contact.value.area;
								}
								else
								{
									std::map<std::string, double>::const_iterator it=contact.value.props.adjuncts.find(name);
									if(it!=contact.value.props.adjuncts.end())
									{
										value+=it->second;
									}
								}
							}
						}
					}
				}
			}

			{
				for(std::size_t i=0;i<params.names_of_raw_values_describing_rr_contacts_far.size();i++)
				{
					result.header_for_vcblock_encodings.push_back(std::string("main_rr_contact__")+params.names_of_raw_values_describing_rr_contacts_far[i]);
				}
				for(std::size_t i=0;i<params.names_of_raw_values_describing_residues.size();i++)
				{
					result.header_for_vcblock_encodings.push_back(std::string("main_r_pair__")+params.names_of_raw_values_describing_residues[i]+std::string("__v1"));
					result.header_for_vcblock_encodings.push_back(std::string("main_r_pair__")+params.names_of_raw_values_describing_residues[i]+std::string("__v2"));
				}
				for(std::size_t i=0;i<combined_names_of_raw_values_describing_rr_contacts.size();i++)
				{
					result.header_for_vcblock_encodings.push_back(std::string("capping_contacts_sum__")+combined_names_of_raw_values_describing_rr_contacts[i]+std::string("__v1"));
					result.header_for_vcblock_encodings.push_back(std::string("capping_contacts_sum__")+combined_names_of_raw_values_describing_rr_contacts[i]+std::string("__v2"));
				}

				{
					std::vector<std::string> header_for_surroundings;

					for(std::size_t i=0;i<params.names_of_raw_values_describing_residues.size();i++)
					{
						header_for_surroundings.push_back(std::string("sur_r__")+params.names_of_raw_values_describing_residues[i]);
					}

					header_for_surroundings.push_back(std::string("sur_adjacency"));

					for(std::size_t i=0;i<combined_names_of_raw_values_describing_rr_contacts.size();i++)
					{
						header_for_surroundings.push_back(std::string("sur_contact_pair__")+combined_names_of_raw_values_describing_rr_contacts[i]+std::string("__v1"));
						header_for_surroundings.push_back(std::string("sur_contact_pair__")+combined_names_of_raw_values_describing_rr_contacts[i]+std::string("__v2"));
					}

					for(std::size_t i=0;i<combined_names_of_raw_values_describing_rr_contacts.size();i++)
					{
						header_for_surroundings.push_back(std::string("paracapping_contacts_sum__")+combined_names_of_raw_values_describing_rr_contacts[i]);
					}

					for(std::size_t i=0;i<header_for_surroundings.size();i++)
					{
						result.header_for_vcblock_encodings.push_back(header_for_surroundings[i]+std::string("__mean_v"));
						result.header_for_vcblock_encodings.push_back(header_for_surroundings[i]+std::string("__mean_x"));
						result.header_for_vcblock_encodings.push_back(header_for_surroundings[i]+std::string("__mean_y"));
						result.header_for_vcblock_encodings.push_back(header_for_surroundings[i]+std::string("__var_v"));
						result.header_for_vcblock_encodings.push_back(header_for_surroundings[i]+std::string("__var_x"));
						result.header_for_vcblock_encodings.push_back(header_for_surroundings[i]+std::string("__var_y"));
						result.header_for_vcblock_encodings.push_back(header_for_surroundings[i]+std::string("__cov_xy"));
					}
				}
			}

			for(std::size_t i=0;i<result.indices_of_recorded_vcblocks.size();i++)
			{
				VCBlock& vcblock=result.vcblocks[result.indices_of_recorded_vcblocks[i]];

				vcblock.full_encoding.reserve(result.header_for_vcblock_encodings.size());

				encode_values_cut_simply_and_append_to_output(result.raw_values_for_rr_contacts[vcblock.rr_contact_descriptor_id_main], params.names_of_raw_values_describing_rr_contacts_far.size(), vcblock.full_encoding);

				encode_values_for_pair_and_append_to_output(result.raw_values_for_residues[vcblock.residue_id_main[0]], result.raw_values_for_residues[vcblock.residue_id_main[1]], vcblock.full_encoding);

				{
					std::vector<double> summed_contact_values[2];
					for(int e=0;e<2;e++)
					{
						summed_contact_values[e].resize(combined_names_of_raw_values_describing_rr_contacts.size());
						for(std::vector<std::size_t>::const_iterator it=vcblock.rr_contact_descriptor_ids_capping[e].begin();it!=vcblock.rr_contact_descriptor_ids_capping[e].end();++it)
						{
							for(std::size_t l=0;l<combined_names_of_raw_values_describing_rr_contacts.size();l++)
							{
								summed_contact_values[e][l]+=result.raw_values_for_rr_contacts[*it][l];
							}
						}
					}
					encode_values_for_pair_and_append_to_output(summed_contact_values[0], summed_contact_values[1], vcblock.full_encoding);
				}

				if(!vcblock.residue_ids_surrounding.empty())
				{
					std::vector< std::vector<double> > all_sector_encodings(vcblock.residue_ids_surrounding.size());

					for(std::size_t j=0;j<vcblock.residue_ids_surrounding.size();j++)
					{
						std::vector<double>& sector_encoding=all_sector_encodings[j];

						encode_values_simply_and_append_to_output(result.raw_values_for_residues[vcblock.residue_ids_surrounding[j]], sector_encoding);

						sector_encoding.push_back(vcblock.adjacency_lengths_of_surrounding_residues[j]);

						encode_values_for_pair_and_append_to_output(result.raw_values_for_rr_contacts[vcblock.rr_contact_descriptor_ids_surrounding[0][j]], result.raw_values_for_rr_contacts[vcblock.rr_contact_descriptor_ids_surrounding[1][j]], sector_encoding);

						{
							std::vector<double> summed_contact_values;
							summed_contact_values.resize(combined_names_of_raw_values_describing_rr_contacts.size());
							for(std::vector<std::size_t>::const_iterator it=vcblock.rr_contact_descriptor_ids_paracapping[j].begin();it!=vcblock.rr_contact_descriptor_ids_paracapping[j].end();++it)
							{
								for(std::size_t l=0;l<combined_names_of_raw_values_describing_rr_contacts.size();l++)
								{
									summed_contact_values[l]+=result.raw_values_for_rr_contacts[*it][l];
								}
							}
							encode_values_simply_and_append_to_output(summed_contact_values, sector_encoding);
						}
					}

					const std::size_t num_of_of_sector_encoding_values=all_sector_encodings[0].size();

					std::vector<double> stats_of_sector_encoding_values;
					stats_of_sector_encoding_values.reserve(num_of_of_sector_encoding_values*7);

					for(std::size_t e=0;e<num_of_of_sector_encoding_values;e++)
					{
						double N=static_cast<double>(all_sector_encodings.size());
						double mean_v=0.0;
						double mean_x=0.0;
						double mean_y=0.0;
						for(std::size_t j=0;j<all_sector_encodings.size();j++)
						{
							const double angle=vcblock.angles_of_surrounding_residues[j];
							const double v=all_sector_encodings[j][e];
							mean_v+=v;
							mean_x+=(v*std::cos(angle));
							mean_y+=(v*std::sin(angle));
						}
						mean_v/=N;
						mean_x/=N;
						mean_y/=N;
						double var_v=0.0;
						double var_x=0.0;
						double var_y=0.0;
						double cov_xy=0.0;
						for(std::size_t j=0;j<all_sector_encodings.size();j++)
						{
							const double angle=vcblock.angles_of_surrounding_residues[j];
							const double v=all_sector_encodings[j][e];
							var_v+=square(v-mean_v);
							var_x+=square((v*std::cos(angle))-mean_x);
							var_y+=square((v*std::sin(angle))-mean_y);
							cov_xy+=((v*std::cos(angle))-mean_x)*((v*std::sin(angle))-mean_y);
						}
						var_v/=N;
						var_x/=N;
						var_y/=N;
						cov_xy/=N;
						stats_of_sector_encoding_values.push_back(mean_v);
						stats_of_sector_encoding_values.push_back(mean_x);
						stats_of_sector_encoding_values.push_back(mean_y);
						stats_of_sector_encoding_values.push_back(var_v);
						stats_of_sector_encoding_values.push_back(var_x);
						stats_of_sector_encoding_values.push_back(var_y);
						stats_of_sector_encoding_values.push_back(cov_xy);
					}

					encode_values_simply_and_append_to_output(stats_of_sector_encoding_values, vcblock.full_encoding);
				}

				if(vcblock.full_encoding.size()!=result.header_for_vcblock_encodings.size())
				{
					vcblock.full_encoding.resize(result.header_for_vcblock_encodings.size(), 0.0);
				}

				const Standardizer& standardizer=Standardizer::get_default_standardizer();

				if(!standardizer.empty())
				{
					standardizer.apply(vcblock.full_encoding, vcblock.standardized_encoding);
				}
			}
		}
	}

private:
	static void encode_values_simply_and_append_to_output(const std::vector<double>& raw_values, std::vector<double>& output)
	{
		output.insert(output.end(), raw_values.begin(), raw_values.end());
	}

	static void encode_values_cut_simply_and_append_to_output(const std::vector<double>& raw_values, const std::size_t cut_size, std::vector<double>& output)
	{
		for(std::size_t i=0;i<cut_size && i<raw_values.size();i++)
		{
			output.push_back(raw_values[i]);
		}
	}

	static void encode_values_for_pair_and_append_to_output(const std::vector<double>& raw_values1, const std::vector<double>& raw_values2, std::vector<double>& output)
	{
		for(std::size_t i=0;i<raw_values1.size() && i<raw_values2.size();i++)
		{
			const double x=raw_values1[i];
			const double y=raw_values2[i];
			output.push_back((x+y)*0.5);
			output.push_back(std::abs(x-y));
		}
	}

	static double square(const double v)
	{
		return (v*v);
	}
};

}

}

#endif /* SCRIPTING_VCBLOCKS_OF_DATA_MANAGER_H_ */
