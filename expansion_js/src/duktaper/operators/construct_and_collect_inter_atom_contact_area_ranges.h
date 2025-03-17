#ifndef DUKTAPER_OPERATORS_CONSTRUCT_AND_COLLECT_INTER_ATOM_CONTACT_AREA_RANGES_H_
#define DUKTAPER_OPERATORS_CONSTRUCT_AND_COLLECT_INTER_ATOM_CONTACT_AREA_RANGES_H_

#include "construct_contacts_radically_fast.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class ConstructAndCollectInterAtomContactAreaRanges : public scripting::OperatorBase<ConstructAndCollectInterAtomContactAreaRanges>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		int considered_inter_atom_contacts;

		Result() :
			considered_inter_atom_contacts(0)
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("considered_inter_atom_contacts")=considered_inter_atom_contacts;
		}
	};

	ConstructContactsRadicallyFast contacts_construction_operator;
	scripting::SelectionManager::Query parameters_for_selecting_contacts;
	scripting::CongregationOfDataManagers::ObjectQuery objects_query;
	std::vector<std::string> area_value_names;
	std::string stats_output_file;
	bool on_residue_level;

	ConstructAndCollectInterAtomContactAreaRanges() : on_residue_level(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		contacts_construction_operator.initialize(input);
		parameters_for_selecting_contacts=scripting::OperatorsUtilities::read_generic_selecting_query(input);
		objects_query=scripting::OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
		area_value_names=input.get_value_vector_or_default<std::string>("area-value-names", std::vector<std::string>(1, "area"));
		stats_output_file=input.get_value<std::string>("stats-output-file");
		on_residue_level=input.get_flag("on-residue-level");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		contacts_construction_operator.document(doc);
		scripting::OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
		scripting::OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("area-value-names", CDOD::DATATYPE_STRING_ARRAY, "vector of contact value names", "area"));
		doc.set_option_decription(CDOD("stats-output-file", CDOD::DATATYPE_STRING, "file path to output stats"));
		doc.set_option_decription(CDOD("on-residue-level", CDOD::DATATYPE_BOOL, "flag to calculate and output results on level"));
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		congregation_of_data_managers.assert_objects_availability();

		const std::vector<scripting::DataManager*> all_objects=congregation_of_data_managers.get_objects(objects_query);
		if(all_objects.empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}
		if(all_objects.size()==1)
		{
			throw std::runtime_error(std::string("Only one object selected."));
		}

		std::vector<scripting::DataManager*> suitable_objects;

		std::vector<AAIdentifier> sorted_list_of_all_encountered_contact_ids;

		{
			ConstructContactsRadicallyFast simplified_contacts_construction_operator=contacts_construction_operator;
			simplified_contacts_construction_operator.adjunct_circle_restrictions.clear();
			simplified_contacts_construction_operator.precutting_shifts.clear();

			std::set<AAIdentifier> set_of_all_encountered_contact_ids;

			for(std::size_t i=0;i<all_objects.size();i++)
			{
				scripting::DataManager& data_manager=(*(all_objects[i]));

				simplified_contacts_construction_operator.run(data_manager);

				bool suitable=false;
				for(std::size_t j=0;j<data_manager.contacts().size() && !suitable;j++)
				{
					const scripting::Contact& contact=data_manager.contacts()[j];
					if(!contact.solvent() && std::abs((data_manager.atoms()[contact.ids[0]].crad.resSeq)-(data_manager.atoms()[contact.ids[1]].crad.resSeq))>1)
					{
						suitable=true;
					}
				}

				if(suitable)
				{
					const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting_contacts);

					if(!ids.empty())
					{
						for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
						{
							const scripting::Contact& contact=data_manager.contacts()[*it];
							if(contact.solvent())
							{
								const AAIdentifier aaid(AtomSequenceContext(simplified_crad(data_manager.atoms()[contact.ids[0]].crad, on_residue_level)), AtomSequenceContext(1));
								if(is_residue_standard(aaid.asc_a.crad.resName))
								{
									set_of_all_encountered_contact_ids.insert(aaid);
								}
							}
							else
							{
								const AAIdentifier aaid(AtomSequenceContext(simplified_crad(data_manager.atoms()[contact.ids[0]].crad, on_residue_level)), AtomSequenceContext(simplified_crad(data_manager.atoms()[contact.ids[1]].crad, on_residue_level)));
								if(is_residue_standard(aaid.asc_a.crad.resName) && is_residue_standard(aaid.asc_b.crad.resName))
								{
									set_of_all_encountered_contact_ids.insert(aaid);
								}
							}
						}
					}

					suitable_objects.push_back(all_objects[i]);
				}

				data_manager.remove_contacts();
			}

			if(suitable_objects.size()<2)
			{
				throw std::runtime_error(std::string("Less than two object suitable."));
			}

			sorted_list_of_all_encountered_contact_ids=std::vector<AAIdentifier>(set_of_all_encountered_contact_ids.begin(), set_of_all_encountered_contact_ids.end());
		}

		std::map<AAIdentifier, AAContactValueStatistics> inter_atom_contacts_statistics;

		for(std::size_t i=0;i<suitable_objects.size();i++)
		{
			scripting::DataManager& data_manager=(*(suitable_objects[i]));

			contacts_construction_operator.run(data_manager);

			const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting_contacts);

			if(!ids.empty())
			{
				std::set<AtomSequenceContext> atom_availabilities;
				atom_availabilities.insert(AtomSequenceContext(1));

				for(std::size_t j=0;j<data_manager.atoms().size();j++)
				{
					const AtomSequenceContext asc(simplified_crad(data_manager.atoms()[j].crad, on_residue_level));
					if(is_residue_standard(asc.crad.resName))
					{
						atom_availabilities.insert(asc);
					}
				}

				std::map< AAIdentifier, AAContactValue > inter_atom_contacts_realizations;

				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const scripting::Contact& contact=data_manager.contacts()[*it];
					if(contact.solvent())
					{
						const AAIdentifier aaid(AtomSequenceContext(simplified_crad(data_manager.atoms()[contact.ids[0]].crad, on_residue_level)), AtomSequenceContext(1));
						if(is_residue_standard(aaid.asc_a.crad.resName))
						{
							inter_atom_contacts_realizations[aaid].update(contact.value, area_value_names);
						}
					}
					else
					{
						const AAIdentifier aaid(AtomSequenceContext(simplified_crad(data_manager.atoms()[contact.ids[0]].crad, on_residue_level)), AtomSequenceContext(simplified_crad(data_manager.atoms()[contact.ids[1]].crad, on_residue_level)));
						if(is_residue_standard(aaid.asc_a.crad.resName) && is_residue_standard(aaid.asc_b.crad.resName))
						{
							inter_atom_contacts_realizations[aaid].update(contact.value, area_value_names);
						}
					}
				}

				{
					std::vector<AAIdentifier> sorted_list_of_realized_contact_ids;
					sorted_list_of_realized_contact_ids.reserve(inter_atom_contacts_realizations.size());
					for(std::map< AAIdentifier, AAContactValue >::const_iterator it=inter_atom_contacts_realizations.begin();it!=inter_atom_contacts_realizations.end();++it)
					{
						sorted_list_of_realized_contact_ids.push_back(it->first);
					}

					std::vector<AAIdentifier>::const_iterator it1=sorted_list_of_all_encountered_contact_ids.begin();
					std::vector<AAIdentifier>::const_iterator it2=sorted_list_of_realized_contact_ids.begin();
					while(it1!=sorted_list_of_all_encountered_contact_ids.end() && it2!=sorted_list_of_realized_contact_ids.end())
					{
						if((*it1)<(*it2))
						{
							if(atom_availabilities.count(it1->asc_a)>0 && atom_availabilities.count(it1->asc_b)>0)
							{
								inter_atom_contacts_statistics[*it1].increase_count_without_adding();
							}
							++it1;
						}
						else if((*it2)<(*it1))
						{
							++it2;
						}
						else
						{
							++it1;
							++it2;
						}
					}
					while(it1!=sorted_list_of_all_encountered_contact_ids.end())
					{
						if(atom_availabilities.count(it1->asc_a)>0 && atom_availabilities.count(it1->asc_b)>0)
						{
							inter_atom_contacts_statistics[*it1].increase_count_without_adding();
						}
						++it1;
					}
				}

				for(std::map< AAIdentifier, AAContactValue >::const_iterator it=inter_atom_contacts_realizations.begin();it!=inter_atom_contacts_realizations.end();++it)
				{
					inter_atom_contacts_statistics[it->first].add(it->second);
				}
			}

			data_manager.remove_contacts();
		}

		if(!stats_output_file.empty())
		{
			scripting::OutputSelector output_selector(stats_output_file);
			std::ostream& output=output_selector.stream();
			scripting::assert_io_stream(stats_output_file, output);

			{
				output << "residue_seqnum1 residue_name1 atom_name1 residue_seqnum2 residue_name2 atom_name2 seq_sep";
				for(std::size_t i=0;i<area_value_names.size();i++)
				{
					const std::string vname=area_value_names[i];
					output << " " << vname << "_min" << " " << vname << "_max" << " " << vname << "_count" << " " << vname << "_mean";
				}
				output << "\n";
			}

			for(std::map<AAIdentifier, AAContactValueStatistics>::const_iterator it=inter_atom_contacts_statistics.begin();it!=inter_atom_contacts_statistics.end();++it)
			{
				const AAIdentifier& aaid=it->first;
				const AAContactValueStatistics& stat=it->second;
				if(stat.count>1)
				{
					output << aaid.asc_a.crad.resSeq << " " << aaid.asc_a.crad.resName << " " << aaid.asc_a.crad.name << " ";
					output << aaid.asc_b.crad.resSeq << " " << aaid.asc_b.crad.resName << " " << aaid.asc_b.crad.name << " ";
					output << std::abs(aaid.asc_a.crad.resSeq-aaid.asc_b.crad.resSeq) << " ";
					for(std::size_t i=0;i<stat.area_value_statistics.size();i++)
					{
						const ValueStatistics& vstat=stat.area_value_statistics[i];
						output << " " << vstat.min_area << " " << vstat.max_area << " " << stat.count << " " << vstat.mean_area;
					}
					output << "\n";
				}
			}
		}

		Result result;
		result.considered_inter_atom_contacts=static_cast<int>(inter_atom_contacts_statistics.size());

		return result;
	}

private:
	struct AtomSequenceContext
	{
		int special_id;
		common::ChainResidueAtomDescriptor crad;

		explicit AtomSequenceContext(const common::ChainResidueAtomDescriptor& crad) : special_id(0), crad(crad)
		{
		}

		explicit AtomSequenceContext(const int special_id) : special_id(special_id)
		{
			if(special_id==1)
			{
				crad.resName="ZSR";
				crad.name="ZSA";
				crad.resSeq=1000000;
			}
		}

		bool operator==(const AtomSequenceContext& v) const
		{
			return (special_id==v.special_id && crad==v.crad);
		}

		bool operator<(const AtomSequenceContext& v) const
		{
			return (special_id<v.special_id || (special_id==v.special_id && crad<v.crad));
		}
	};

	struct AAIdentifier
	{
		AtomSequenceContext asc_a;
		AtomSequenceContext asc_b;

		AAIdentifier(const AtomSequenceContext& a, const AtomSequenceContext& b) :  asc_a(a<b ? a : b), asc_b(a<b ? b : a)
		{
		}

		bool operator<(const AAIdentifier& v) const
		{
			return ((asc_a<v.asc_a) || (asc_a==v.asc_a && asc_b<v.asc_b));
		}
	};

	struct AAContactValue
	{
		std::vector<double> area_values;
		double dist;

		AAContactValue() : dist(std::numeric_limits<double>::max())
		{
		}

		void update(const common::ContactValue& v, const std::vector<std::string>& area_value_names)
		{
			if(area_values.size()!=area_value_names.size())
			{
				area_values.resize(area_value_names.size(), 0.0);
			}
			for(std::size_t i=0;i<area_value_names.size();i++)
			{
				if(area_value_names[i]=="area")
				{
					area_values[i]+=v.area;
				}
				else
				{
					std::map<std::string, double>::const_iterator adjunct_it=v.props.adjuncts.find(area_value_names[i]);
					if(adjunct_it!=v.props.adjuncts.end())
					{
						area_values[i]=+adjunct_it->second;
					}
				}
			}
			dist=std::min(dist, v.dist);
		}
	};

	struct ValueStatistics
	{
		double min_area;
		double max_area;
		double mean_area;

		ValueStatistics() : min_area(0.0), max_area(0.0), mean_area(0.0)
		{
		}
	};

	struct AAContactValueStatistics
	{
		std::vector<ValueStatistics> area_value_statistics;
		int count;

		AAContactValueStatistics() : count(0)
		{
		}

		void add(const AAContactValue& v)
		{
			if(area_value_statistics.size()!=v.area_values.size())
			{
				area_value_statistics.resize(v.area_values.size(), ValueStatistics());
			}
			for(std::size_t i=0;i<v.area_values.size();i++)
			{
				area_value_statistics[i].min_area=(count>0 ? std::min(area_value_statistics[i].min_area, v.area_values[i]) : v.area_values[i]);
				area_value_statistics[i].max_area=(count>0 ? std::max(area_value_statistics[i].max_area, v.area_values[i]) : v.area_values[i]);
			}

			count++;

			if(count==1)
			{
				for(std::size_t i=0;i<v.area_values.size();i++)
				{
					area_value_statistics[i].mean_area=v.area_values[i];
				}
			}
			else
			{
				for(std::size_t i=0;i<v.area_values.size();i++)
				{
					area_value_statistics[i].mean_area+=(v.area_values[i]-area_value_statistics[i].mean_area)/static_cast<double>(count);
				}
			}
		}

		void increase_count_without_adding()
		{
			for(std::size_t i=0;i<area_value_statistics.size();i++)
			{
				area_value_statistics[i].min_area=0.0;
			}

			count++;

			if(count>1)
			{
				for(std::size_t i=0;i<area_value_statistics.size();i++)
				{
					area_value_statistics[i].mean_area+=(0.0-area_value_statistics[i].mean_area)/static_cast<double>(count);
				}
			}
		}
	};

	static common::ChainResidueAtomDescriptor simplified_crad(const common::ChainResidueAtomDescriptor& crad, const bool residue_level)
	{
		const common::ChainResidueAtomDescriptor gen_crad=common::generalize_crad(crad);
		common::ChainResidueAtomDescriptor s_crad;
		s_crad.resSeq=crad.resSeq;
		s_crad.resName=gen_crad.resName;
		if(residue_level)
		{
			s_crad.name="ANY";
		}
		else
		{
			s_crad.name=gen_crad.name;
		}
		return s_crad;
	}

	static bool is_residue_standard(const std::string& residue_name)
	{
		if(residue_name=="ILE"){return true;}
		else if(residue_name=="VAL"){return true;}
		else if(residue_name=="LEU"){return true;}
		else if(residue_name=="PHE"){return true;}
		else if(residue_name=="CYS"){return true;}
		else if(residue_name=="MET"){return true;}
		else if(residue_name=="ALA"){return true;}
		else if(residue_name=="GLY"){return true;}
		else if(residue_name=="THR"){return true;}
		else if(residue_name=="SER"){return true;}
		else if(residue_name=="TRP"){return true;}
		else if(residue_name=="TYR"){return true;}
		else if(residue_name=="PRO"){return true;}
		else if(residue_name=="HIS"){return true;}
		else if(residue_name=="GLU"){return true;}
		else if(residue_name=="GLN"){return true;}
		else if(residue_name=="ASP"){return true;}
		else if(residue_name=="ASN"){return true;}
		else if(residue_name=="LYS"){return true;}
		else if(residue_name=="ARG"){return true;}
		return false;
	}
};

}

}

}


#endif /* DUKTAPER_OPERATORS_CONSTRUCT_AND_COLLECT_INTER_ATOM_CONTACT_AREA_RANGES_H_ */
