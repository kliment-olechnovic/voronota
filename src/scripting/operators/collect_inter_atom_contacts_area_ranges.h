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
	std::string stats_output_file;

	CollectInterAtomContactAreaRanges()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_contacts=OperatorsUtilities::read_generic_selecting_query(input);
		objects_query=OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
		stats_output_file=input.get_value_or_default<std::string>("stats-output-file", "");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
		OperatorsUtilities::document_read_generic_selecting_query(doc);
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
					if(!contact.solvent())
					{
						const AAIdentifier aaid(simplified_crad(data_manager.atoms()[contact.ids[0]].crad), simplified_crad(data_manager.atoms()[contact.ids[1]].crad));
						if(is_residue_standard(aaid.asc_a.crad.resName) && is_residue_standard(aaid.asc_b.crad.resName))
						{
							inter_atom_contacts_realizations[aaid][i].set(contact.value);
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

			for(std::map<AAIdentifier, AAContactValueStatistics>::const_iterator it=inter_atom_contacts_statistics.begin();it!=inter_atom_contacts_statistics.end();++it)
			{
				const AAIdentifier& aaid=it->first;
				const AAContactValueStatistics& stat=it->second;
				const int seq_sep=std::abs(aaid.asc_a.crad.resSeq-aaid.asc_b.crad.resSeq);
				output << aaid.asc_a.crad.resName << " " << aaid.asc_a.crad.name << " ";
				output << aaid.asc_b.crad.resName << " " << aaid.asc_b.crad.name << " ";
				output << seq_sep << " ";
				output << stat.min_area << " " << stat.max_area << "\n";
			}
		}

		Result result;
		result.considered_inter_atom_contacts=static_cast<int>(inter_atom_contacts_statistics.size());

		return result;
	}

private:
	struct AtomSequenceContext
	{
		common::ChainResidueAtomDescriptor crad;

		AtomSequenceContext(const common::ChainResidueAtomDescriptor& crad) : crad(crad)
		{
		}

		bool operator==(const AtomSequenceContext& v) const
		{
			return (crad==v.crad);
		}

		bool operator<(const AtomSequenceContext& v) const
		{
			return (crad<v.crad);
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
		double area;
		double dist;

		AAContactValue() : area(0.0), dist(0.0)
		{
		}

		void set(const common::ContactValue& v)
		{
			area=v.area;
			dist=v.dist;
		}
	};

	struct AAContactValueStatistics
	{
		double min_area;
		double max_area;
		int count;

		AAContactValueStatistics() : min_area(0.0), max_area(0.0), count(0)
		{
		}

		void add(const AAContactValue& v)
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
