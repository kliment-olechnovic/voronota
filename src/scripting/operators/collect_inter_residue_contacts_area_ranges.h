#ifndef SCRIPTING_OPERATORS_COLLECT_INTER_RESIDUE_CONTACTS_AREA_RANGES_H_
#define SCRIPTING_OPERATORS_COLLECT_INTER_RESIDUE_CONTACTS_AREA_RANGES_H_

#include "../operators_common.h"
#include "../../common/contacts_scoring_utilities.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class CollectInterResidueContactAreaRanges : public OperatorBase<CollectInterResidueContactAreaRanges>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		int considered_inter_residue_contacts;

		Result() :
			considered_inter_residue_contacts(0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("considered_inter_residue_contacts")=considered_inter_residue_contacts;
		}
	};

	SelectionManager::Query parameters_for_selecting_contacts;
	CongregationOfDataManagers::ObjectQuery objects_query;
	std::string adjunct_min_area;
	std::string adjunct_max_area;
	std::string adjunct_min_max_area_sqrt_diff;
	std::string stats_output_file;

	CollectInterResidueContactAreaRanges()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_contacts=OperatorsUtilities::read_generic_selecting_query(input);
		objects_query=OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
		adjunct_min_area=input.get_value_or_default<std::string>("adj-min-area", "");
		adjunct_max_area=input.get_value_or_default<std::string>("adj-max-area", "");
		adjunct_min_max_area_sqrt_diff=input.get_value_or_default<std::string>("adj-min-max-area-sqrt-diff", "");
		stats_output_file=input.get_value_or_default<std::string>("stats-output-file", "");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("adj-min-area", CDOD::DATATYPE_STRING, "adjunct name for min area stat", ""));
		doc.set_option_decription(CDOD("adj-max-area", CDOD::DATATYPE_STRING, "adjunct name for max area stat", ""));
		doc.set_option_decription(CDOD("adj-min-max-area-sqrt-diff", CDOD::DATATYPE_STRING, "adjunct name for min-max area difference stat", ""));
		doc.set_option_decription(CDOD("stats-output-file", CDOD::DATATYPE_STRING, "file path to output stats", ""));
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		congregation_of_data_managers.assert_objects_availability();

		const std::vector<DataManager*> objects=congregation_of_data_managers.get_objects(objects_query);
		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}
		if(objects.size()==1)
		{
			throw std::runtime_error(std::string("Only one object selected."));
		}

		for(std::size_t i=0;i<objects.size();i++)
		{
			objects[i]->assert_contacts_availability();
		}

		std::vector< std::set<ResidueSequenceContext> > all_residue_availabilities(objects.size());

		std::vector< std::map<common::ChainResidueAtomDescriptor, ResidueSequenceContext> > all_maps_of_crad_to_residue_sequence_contexts(objects.size());

		std::map< RRIdentifier, std::map<std::size_t, RRContactValue> > inter_residue_contacts_realizations;

		bool solvent_encountered=false;

		for(std::size_t i=0;i<objects.size();i++)
		{
			DataManager& data_manager=(*(objects[i]));

			const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting_contacts);
			if(!ids.empty())
			{
				std::set<ResidueSequenceContext>& residue_availability=all_residue_availabilities[i];
				std::map<common::ChainResidueAtomDescriptor, ResidueSequenceContext>& map_of_crad_to_residue_sequence_contexts=all_maps_of_crad_to_residue_sequence_contexts[i];

				for(std::size_t j=0;j<data_manager.primary_structure_info().residues.size();j++)
				{
					const common::ConstructionOfPrimaryStructure::Residue& residue=data_manager.primary_structure_info().residues[j];
					if(residue.atom_ids.size()>=get_min_acceptable_number_of_heavy_atoms_in_residue(residue.chain_residue_descriptor.resName))
					{
						ResidueSequenceContext rsc(simplified_crad(residue.chain_residue_descriptor));
						map_of_crad_to_residue_sequence_contexts.insert(std::make_pair(rsc.crad, rsc));
					}
				}

				for(std::map<common::ChainResidueAtomDescriptor, ResidueSequenceContext>::const_iterator it=map_of_crad_to_residue_sequence_contexts.begin();it!=map_of_crad_to_residue_sequence_contexts.end();++it)
				{
					residue_availability.insert(it->second);
				}

				std::map<common::ChainResidueAtomDescriptorsPair, RRContactValue> map_of_contacts;
				std::map<common::ChainResidueAtomDescriptor, RRContactValue> map_of_sasa;

				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const Contact& contact=data_manager.contacts()[*it];
					if(contact.solvent())
					{
						map_of_sasa[simplified_crad(data_manager.atoms()[contact.ids[0]].crad)].add(contact.value);
					}
					else
					{
						map_of_contacts[common::ChainResidueAtomDescriptorsPair(simplified_crad(data_manager.atoms()[contact.ids[0]].crad), simplified_crad(data_manager.atoms()[contact.ids[1]].crad))].add(contact.value);
					}
				}

				for(std::map<common::ChainResidueAtomDescriptorsPair, RRContactValue>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
				{
					const common::ChainResidueAtomDescriptorsPair& crads=it->first;
					std::map<common::ChainResidueAtomDescriptor, ResidueSequenceContext>::const_iterator jt_a=map_of_crad_to_residue_sequence_contexts.find(crads.a);
					if(jt_a!=map_of_crad_to_residue_sequence_contexts.end())
					{
						std::map<common::ChainResidueAtomDescriptor, ResidueSequenceContext>::const_iterator jt_b=map_of_crad_to_residue_sequence_contexts.find(crads.b);
						if(jt_b!=map_of_crad_to_residue_sequence_contexts.end())
						{
							inter_residue_contacts_realizations[RRIdentifier(jt_a->second, jt_b->second)][i]=it->second;
						}
					}
				}

				for(std::map<common::ChainResidueAtomDescriptor, RRContactValue>::const_iterator it=map_of_sasa.begin();it!=map_of_sasa.end();++it)
				{
					const common::ChainResidueAtomDescriptor& crad_a=it->first;
					std::map<common::ChainResidueAtomDescriptor, ResidueSequenceContext>::const_iterator jt_a=map_of_crad_to_residue_sequence_contexts.find(crad_a);
					if(jt_a!=map_of_crad_to_residue_sequence_contexts.end())
					{
						inter_residue_contacts_realizations[RRIdentifier(jt_a->second, ResidueSequenceContext(1))][i]=it->second;
						if(!solvent_encountered)
						{
							residue_availability.insert(ResidueSequenceContext(1));
							solvent_encountered=true;
						}
					}
				}
			}
		}

		std::map<RRIdentifier, RRContactValueStatistics> inter_residue_contacts_statistics;

		for(std::map< RRIdentifier, std::map<std::size_t, RRContactValue> >::const_iterator it=inter_residue_contacts_realizations.begin();it!=inter_residue_contacts_realizations.end();++it)
		{
			const RRIdentifier& rrid=it->first;
			const std::map<std::size_t, RRContactValue>& map_of_realized_values=it->second;

			int count_contact_possible_but_not_realized=0;

			if(map_of_realized_values.size()<objects.size())
			{
				for(std::size_t i=0;i<objects.size();i++)
				{
					if(map_of_realized_values.count(i)==0)
					{
						std::set<ResidueSequenceContext>& residue_availability=all_residue_availabilities[i];
						if(residue_availability.count(rrid.rsc_a)>0 && residue_availability.count(rrid.rsc_b)>0)
						{
							count_contact_possible_but_not_realized++;
						}
					}
				}
			}

			RRContactValueStatistics stats;
			stats.count=count_contact_possible_but_not_realized;

			for(std::map<std::size_t, RRContactValue>::const_iterator jt=map_of_realized_values.begin();jt!=map_of_realized_values.end();++jt)
			{
				stats.add(jt->second);
			}

			if(stats.count>1)
			{
				inter_residue_contacts_statistics[rrid]=stats;
			}
		}

		if(!adjunct_min_area.empty() || !adjunct_max_area.empty() || !adjunct_min_max_area_sqrt_diff.empty())
		{
			for(std::size_t i=0;i<objects.size();i++)
			{
				DataManager& data_manager=(*(objects[i]));
				const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting_contacts);
				if(!ids.empty())
				{
					const std::map<common::ChainResidueAtomDescriptor, ResidueSequenceContext>& map_of_crad_to_residue_sequence_contexts=all_maps_of_crad_to_residue_sequence_contexts[i];
					for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
					{
						std::map<std::string, double>& contact_adjuncts=data_manager.contact_adjuncts_mutable(*it);

						if(!adjunct_min_area.empty())
						{
							contact_adjuncts.erase(adjunct_min_area);
						}

						if(!adjunct_max_area.empty())
						{
							contact_adjuncts.erase(adjunct_max_area);
						}

						if(!adjunct_min_max_area_sqrt_diff.empty())
						{
							contact_adjuncts.erase(adjunct_min_max_area_sqrt_diff);
						}

						const Contact& contact=data_manager.contacts()[*it];
						std::map<RRIdentifier, RRContactValueStatistics>::const_iterator jt=inter_residue_contacts_statistics.end();
						if(contact.solvent())
						{
							std::map<common::ChainResidueAtomDescriptor, ResidueSequenceContext>::const_iterator jt_a=map_of_crad_to_residue_sequence_contexts.find(simplified_crad(data_manager.atoms()[contact.ids[0]].crad));
							if(jt_a!=map_of_crad_to_residue_sequence_contexts.end())
							{
								jt=inter_residue_contacts_statistics.find(RRIdentifier(jt_a->second, ResidueSequenceContext(1)));
							}
						}
						else
						{
							std::map<common::ChainResidueAtomDescriptor, ResidueSequenceContext>::const_iterator jt_a=map_of_crad_to_residue_sequence_contexts.find(simplified_crad(data_manager.atoms()[contact.ids[0]].crad));
							if(jt_a!=map_of_crad_to_residue_sequence_contexts.end())
							{
								std::map<common::ChainResidueAtomDescriptor, ResidueSequenceContext>::const_iterator jt_b=map_of_crad_to_residue_sequence_contexts.find(simplified_crad(data_manager.atoms()[contact.ids[1]].crad));
								if(jt_b!=map_of_crad_to_residue_sequence_contexts.end())
								{
									jt=inter_residue_contacts_statistics.find(RRIdentifier(jt_a->second, jt_b->second));
								}
							}
						}

						if(jt!=inter_residue_contacts_statistics.end())
						{
							const RRContactValueStatistics& stats=jt->second;

							if(!adjunct_min_area.empty())
							{
								contact_adjuncts[adjunct_min_area]=stats.min_area;
							}

							if(!adjunct_max_area.empty())
							{
								contact_adjuncts[adjunct_max_area]=stats.max_area;
							}

							if(!adjunct_min_max_area_sqrt_diff.empty())
							{
								contact_adjuncts[adjunct_min_max_area_sqrt_diff]=std::sqrt(stats.max_area)-std::sqrt(stats.min_area);
							}
						}
					}
				}
			}
		}

		if(!stats_output_file.empty())
		{
			OutputSelector output_selector(stats_output_file);
			std::ostream& output=output_selector.stream();
			assert_io_stream(stats_output_file, output);

			for(std::map<RRIdentifier, RRContactValueStatistics>::const_iterator it=inter_residue_contacts_statistics.begin();it!=inter_residue_contacts_statistics.end();++it)
			{
				const RRIdentifier& rrid=it->first;
				const RRContactValueStatistics& stat=it->second;
				const int seq_sep=std::abs(rrid.rsc_a.crad.resSeq-rrid.rsc_b.crad.resSeq);
				output << rrid.rsc_a.crad.resName << " " << rrid.rsc_b.crad.resName << " ";
				output << seq_sep << " ";
				output << stat.min_area << " " << stat.max_area << " ";
				output << rrid.rsc_a.crad.resSeq << " " << rrid.rsc_b.crad.resSeq << "\n";
			}
		}

		Result result;
		result.considered_inter_residue_contacts=static_cast<int>(inter_residue_contacts_statistics.size());

		return result;
	}

private:
	struct ResidueSequenceContext
	{
		int special_id;
		common::ChainResidueAtomDescriptor crad;

		explicit ResidueSequenceContext(const common::ChainResidueAtomDescriptor& crad) : special_id(0), crad(crad)
		{
		}

		explicit ResidueSequenceContext(const int special_id) : special_id(special_id)
		{
			if(special_id==1)
			{
				crad.resName="ZSR";
				crad.name="ZSA";
				crad.resSeq=1000000;
			}
		}

		bool operator==(const ResidueSequenceContext& v) const
		{
			return (special_id==v.special_id && crad==v.crad);
		}

		bool operator<(const ResidueSequenceContext& v) const
		{
			return (special_id<v.special_id || (special_id==v.special_id && crad<v.crad));
		}
	};

	struct RRIdentifier
	{
		ResidueSequenceContext rsc_a;
		ResidueSequenceContext rsc_b;

		RRIdentifier(const ResidueSequenceContext& a, const ResidueSequenceContext& b) :  rsc_a(a<b ? a : b), rsc_b(a<b ? b : a)
		{
		}

		bool operator<(const RRIdentifier& v) const
		{
			return ((rsc_a<v.rsc_a) || (rsc_a==v.rsc_a && rsc_b<v.rsc_b));
		}
	};

	struct RRContactValue
	{
		double area;
		double dist;
		bool accumulated;

		RRContactValue() : area(0.0), dist(0.0), accumulated(false)
		{
		}

		void add(const common::ContactValue& v)
		{
			area+=v.area;
			dist=(!accumulated ? v.dist : std::min(dist, v.dist));
			accumulated=true;
		}
	};

	struct RRContactValueStatistics
	{
		double min_area;
		double max_area;
		int count;

		RRContactValueStatistics() : min_area(0.0), max_area(0.0), count(0)
		{
		}

		void add(const RRContactValue& v)
		{
			min_area=(count>0 ? std::min(min_area, v.area) : v.area);
			max_area=(count>0 ? std::max(max_area, v.area) : v.area);
			count++;
		}
	};

	static common::ChainResidueAtomDescriptor simplified_crad(const common::ChainResidueAtomDescriptor& crad)
	{
		const common::ChainResidueAtomDescriptor gen_crad=common::generalize_crad(crad);
		common::ChainResidueAtomDescriptor s_crad;
		s_crad.resSeq=crad.resSeq;
		s_crad.resName=gen_crad.resName;
		return s_crad;
	}

	static unsigned int get_min_acceptable_number_of_heavy_atoms_in_residue(const std::string& residue_name)
	{
		if(residue_name=="ILE"){return 7;}
		else if(residue_name=="VAL"){return 6;}
		else if(residue_name=="LEU"){return 7;}
		else if(residue_name=="PHE"){return 10;}
		else if(residue_name=="CYS"){return 5;}
		else if(residue_name=="MET"){return 7;}
		else if(residue_name=="ALA"){return 5;}
		else if(residue_name=="GLY"){return 4;}
		else if(residue_name=="THR"){return 6;}
		else if(residue_name=="SER"){return 5;}
		else if(residue_name=="TRP"){return 13;}
		else if(residue_name=="TYR"){return 11;}
		else if(residue_name=="PRO"){return 6;}
		else if(residue_name=="HIS"){return 9;}
		else if(residue_name=="GLU"){return 8;}
		else if(residue_name=="GLN"){return 8;}
		else if(residue_name=="ASP"){return 7;}
		else if(residue_name=="ASN"){return 7;}
		else if(residue_name=="LYS"){return 7;}
		else if(residue_name=="ARG"){return 10;}
		return 999999999;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_COLLECT_INTER_RESIDUE_CONTACTS_AREA_RANGES_H_ */
