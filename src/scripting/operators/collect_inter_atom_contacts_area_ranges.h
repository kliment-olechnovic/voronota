#ifndef SCRIPTING_OPERATORS_COLLECT_INTER_ATOM_CONTACTS_AREA_RANGES_H_
#define SCRIPTING_OPERATORS_COLLECT_INTER_ATOM_CONTACTS_AREA_RANGES_H_

#include "../operators_common.h"
#include "../../common/contacts_scoring_utilities.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class CollectInterAtomContactAreaRanges : public OperatorBase<CollectInterAtomContactAreaRanges>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		int considered_inter_atom_contacts;

		Result() :
			considered_inter_atom_contacts(0)
		{
		}

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("considered_inter_atom_contacts")=considered_inter_atom_contacts;
		}
	};

	SelectionManager::Query parameters_for_selecting_contacts;
	CongregationOfDataManagers::ObjectQuery objects_query;
	std::vector<std::string> area_value_names;
	std::string stats_output_file;

	CollectInterAtomContactAreaRanges()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_contacts=OperatorsUtilities::read_generic_selecting_query(input);
		objects_query=OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
		area_value_names=input.get_value_vector_or_default<std::string>("area-value-names", std::vector<std::string>(1, "area"));
		stats_output_file=input.get_value_or_default<std::string>("stats-output-file", "");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("area-value-names", CDOD::DATATYPE_STRING_ARRAY, "vector of contact value names", "area"));
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

		std::vector< std::set<AtomSequenceContext> > all_atom_availabilities(objects.size());

		std::map< AAIdentifier, std::map<std::size_t, AAContactValue> > inter_atom_contacts_realizations;

		bool solvent_encountered=false;

		for(std::size_t i=0;i<objects.size();i++)
		{
			DataManager& data_manager=(*(objects[i]));

			const std::set<std::size_t> ids=data_manager.selection_manager().select_contacts(parameters_for_selecting_contacts);

			if(!ids.empty())
			{
				for(std::size_t j=0;j<data_manager.atoms().size();j++)
				{
					const AtomSequenceContext asc(simplified_crad(data_manager.atoms()[j].crad));
					if(is_residue_standard(asc.crad.resName))
					{
						all_atom_availabilities[i].insert(asc);
					}
				}

				for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
				{
					const Contact& contact=data_manager.contacts()[*it];
					if(contact.solvent())
					{
						const AAIdentifier aaid(AtomSequenceContext(simplified_crad(data_manager.atoms()[contact.ids[0]].crad)), AtomSequenceContext(1));
						if(is_residue_standard(aaid.asc_a.crad.resName))
						{
							inter_atom_contacts_realizations[aaid][i].add(contact.value, area_value_names);
							if(!solvent_encountered)
							{
								all_atom_availabilities[i].insert(aaid.asc_b);
								solvent_encountered=true;
							}
						}
					}
					else
					{
						const AAIdentifier aaid(AtomSequenceContext(simplified_crad(data_manager.atoms()[contact.ids[0]].crad)), AtomSequenceContext(simplified_crad(data_manager.atoms()[contact.ids[1]].crad)));
						if(is_residue_standard(aaid.asc_a.crad.resName) && is_residue_standard(aaid.asc_b.crad.resName))
						{
							inter_atom_contacts_realizations[aaid][i].add(contact.value, area_value_names);
						}
					}
				}
			}
		}

		std::map<AAIdentifier, AAContactValueStatistics> inter_atom_contacts_statistics;

		for(std::map< AAIdentifier, std::map<std::size_t, AAContactValue> >::const_iterator it=inter_atom_contacts_realizations.begin();it!=inter_atom_contacts_realizations.end();++it)
		{
			const AAIdentifier& aaid=it->first;
			const std::map<std::size_t, AAContactValue>& map_of_realized_values=it->second;

			int count_contact_possible_but_not_realized=0;

			if(map_of_realized_values.size()<objects.size())
			{
				for(std::size_t i=0;i<objects.size();i++)
				{
					if(map_of_realized_values.count(i)==0)
					{
						std::set<AtomSequenceContext>& atom_availability=all_atom_availabilities[i];
						if(atom_availability.count(aaid.asc_a)>0 && atom_availability.count(aaid.asc_b)>0)
						{
							count_contact_possible_but_not_realized++;
						}
					}
				}
			}

			AAContactValueStatistics stats;
			stats.count=count_contact_possible_but_not_realized;

			for(std::map<std::size_t, AAContactValue>::const_iterator jt=map_of_realized_values.begin();jt!=map_of_realized_values.end();++jt)
			{
				stats.add(jt->second);
			}

			if(stats.count>1)
			{
				inter_atom_contacts_statistics[aaid]=stats;
			}
		}

		if(!stats_output_file.empty())
		{
			OutputSelector output_selector(stats_output_file);
			std::ostream& output=output_selector.stream();
			assert_io_stream(stats_output_file, output);

			{
				output << "residue_name1 atom_name1 residue_name2 atom_name2 seq_sep";
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
					const int seq_sep=std::abs(aaid.asc_a.crad.resSeq-aaid.asc_b.crad.resSeq);
					output << aaid.asc_a.crad.resName << " " << aaid.asc_a.crad.name << " ";
					output << aaid.asc_b.crad.resName << " " << aaid.asc_b.crad.name << " ";
					output << seq_sep << " ";
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
		bool accumulated;

		AAContactValue() : dist(0.0), accumulated(false)
		{
		}

		void add(const common::ContactValue& v, const std::vector<std::string>& area_value_names)
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
						area_values[i]+=adjunct_it->second;
					}
				}
			}


			dist=(!accumulated ? v.dist : std::min(dist, v.dist));
			accumulated=true;
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
	};

	static common::ChainResidueAtomDescriptor simplified_crad(const common::ChainResidueAtomDescriptor& crad)
	{
		const common::ChainResidueAtomDescriptor gen_crad=common::generalize_crad(crad);
		common::ChainResidueAtomDescriptor s_crad;
		s_crad.resSeq=crad.resSeq;
		s_crad.resName=gen_crad.resName;
		s_crad.name=gen_crad.name;
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

#endif /* SCRIPTING_OPERATORS_COLLECT_INTER_ATOM_CONTACTS_AREA_RANGES_H_ */
