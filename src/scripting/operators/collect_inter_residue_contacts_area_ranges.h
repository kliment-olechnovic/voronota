#ifndef SCRIPTING_OPERATORS_COLLECT_INTER_RESIDUE_CONTACTS_AREA_RANGES_H_
#define SCRIPTING_OPERATORS_COLLECT_INTER_RESIDUE_CONTACTS_AREA_RANGES_H_

#include "../operators_common.h"

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
	std::string adjunct_min_max_area_diff;

	CollectInterResidueContactAreaRanges()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_contacts=OperatorsUtilities::read_generic_selecting_query(input);
		objects_query=OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
		adjunct_min_area=input.get_value_or_default<std::string>("adj-min-area", "stat_min_area");
		adjunct_max_area=input.get_value_or_default<std::string>("adj-max-area", "stat_max_area");
		adjunct_min_max_area_diff=input.get_value_or_default<std::string>("adj-min-max-area-diff", "stat_min_max_area_diff");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("adj-min-area", CDOD::DATATYPE_STRING, "adjunct name for min area stat", "stat_min_area"));
		doc.set_option_decription(CDOD("adj-max-area", CDOD::DATATYPE_STRING, "adjunct name for max area stat", "stat_max_area"));
		doc.set_option_decription(CDOD("adj-min-max-area-diff", CDOD::DATATYPE_STRING, "adjunct name for min-max area difference stat", "stat_min_max_area_diff"));
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

		std::map< common::ChainResidueAtomDescriptor, std::set<std::size_t> > residue_availability;

		for(std::size_t i=0;i<objects.size();i++)
		{
			const DataManager& data_manager=(*(objects[i]));
			for(std::size_t j=0;j<data_manager.primary_structure_info().residues.size();j++)
			{
				residue_availability[simplified_crad(data_manager.primary_structure_info().residues[j].chain_residue_descriptor)].insert(i);
			}
			residue_availability[common::ChainResidueAtomDescriptor::solvent()].insert(i);
		}

		std::map<common::ChainResidueAtomDescriptorsPair, std::map<std::size_t, RRContactValue> > inter_residue_contacts_realizations;

		for(std::size_t i=0;i<objects.size();i++)
		{
			DataManager& data_manager=(*(objects[i]));
			const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting_contacts);
			if(!ids.empty())
			{
				std::map<common::ChainResidueAtomDescriptorsPair, RRContactValue> map_of_contacts;

				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const Contact& contact=data_manager.contacts()[*it];
					if(contact.solvent())
					{
						map_of_contacts[common::ChainResidueAtomDescriptorsPair(simplified_crad(data_manager.atoms()[contact.ids[0]].crad), common::ChainResidueAtomDescriptor::solvent())].add(contact.value);
					}
					else
					{
						map_of_contacts[common::ChainResidueAtomDescriptorsPair(simplified_crad(data_manager.atoms()[contact.ids[0]].crad), simplified_crad(data_manager.atoms()[contact.ids[1]].crad))].add(contact.value);
					}
				}

				for(std::map<common::ChainResidueAtomDescriptorsPair, RRContactValue>::const_iterator it=map_of_contacts.begin();it!=map_of_contacts.end();++it)
				{
					inter_residue_contacts_realizations[it->first][i]=it->second;
				}
			}
		}

		std::map<common::ChainResidueAtomDescriptorsPair, RRContactValueStatistics> inter_residue_contacts_statistics;

		for(std::map<common::ChainResidueAtomDescriptorsPair, std::map<std::size_t, RRContactValue> >::const_iterator it=inter_residue_contacts_realizations.begin();it!=inter_residue_contacts_realizations.end();++it)
		{
			const common::ChainResidueAtomDescriptorsPair& crads_pair=it->first;
			const std::map<std::size_t, RRContactValue>& map_of_realized_values=it->second;

			int count_contact_possible_but_not_realized_in_some_object=0;

			if(map_of_realized_values.size()<objects.size())
			{
				for(std::size_t i=0;i<objects.size();i++)
				{
					if(map_of_realized_values.count(i)==0)
					{
						std::map< common::ChainResidueAtomDescriptor, std::set<std::size_t> >::const_iterator jt_a=residue_availability.find(crads_pair.a);
						std::map< common::ChainResidueAtomDescriptor, std::set<std::size_t> >::const_iterator jt_b=residue_availability.find(crads_pair.b);
						if(jt_a!=residue_availability.end() && jt_b!=residue_availability.end() && jt_a->second.count(i)>0 && jt_b->second.count(i)>0)
						{
							count_contact_possible_but_not_realized_in_some_object++;
						}
					}
				}
			}

			RRContactValueStatistics stats;
			stats.count=count_contact_possible_but_not_realized_in_some_object;

			for(std::map<std::size_t, RRContactValue>::const_iterator jt=map_of_realized_values.begin();jt!=map_of_realized_values.end();++jt)
			{
				stats.add(jt->second);
			}

			inter_residue_contacts_statistics[crads_pair]=stats;
		}

		for(std::size_t i=0;i<objects.size();i++)
		{
			DataManager& data_manager=(*(objects[i]));
			const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting_contacts);
			if(!ids.empty())
			{
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

					if(!adjunct_min_max_area_diff.empty())
					{
						contact_adjuncts.erase(adjunct_min_max_area_diff);
					}

					const Contact& contact=data_manager.contacts()[*it];
					std::map<common::ChainResidueAtomDescriptorsPair, RRContactValueStatistics>::const_iterator jt;
					if(contact.solvent())
					{
						jt=inter_residue_contacts_statistics.find(common::ChainResidueAtomDescriptorsPair(simplified_crad(data_manager.atoms()[contact.ids[0]].crad), common::ChainResidueAtomDescriptor::solvent()));
					}
					else
					{
						jt=inter_residue_contacts_statistics.find(common::ChainResidueAtomDescriptorsPair(simplified_crad(data_manager.atoms()[contact.ids[0]].crad), simplified_crad(data_manager.atoms()[contact.ids[1]].crad)));
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

						if(!adjunct_min_max_area_diff.empty())
						{
							contact_adjuncts[adjunct_min_max_area_diff]=std::sqrt(stats.max_area)-std::sqrt(stats.min_area);
						}
					}
				}
			}
		}

		Result result;
		result.considered_inter_residue_contacts=static_cast<int>(inter_residue_contacts_statistics.size());

		return result;
	}

private:
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
		common::ChainResidueAtomDescriptor s_crad;
		s_crad.resSeq=crad.resSeq;
		s_crad.resName=crad.resName;
		return s_crad;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_COLLECT_INTER_RESIDUE_CONTACTS_AREA_RANGES_H_ */
