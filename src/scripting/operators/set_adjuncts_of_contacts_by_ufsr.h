#ifndef SCRIPTING_OPERATORS_SET_ADJUNCTS_OF_CONTACTS_BY_UFSR_H_
#define SCRIPTING_OPERATORS_SET_ADJUNCTS_OF_CONTACTS_BY_UFSR_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetAdjunctsOfContactsByUFSR : public OperatorBase<SetAdjunctsOfContactsByUFSR>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfContacts contacts_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::string name_prefix;

	SetAdjunctsOfContactsByUFSR()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		name_prefix=input.get_value<std::string>("name-prefix");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("name-prefix", CDOD::DATATYPE_STRING, "adjuncts name prefix"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		std::set<std::size_t> contact_ids=data_manager.selection_manager().select_contacts(parameters_for_selecting);
		if(contact_ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		std::vector<std::size_t> ids(contact_ids.begin(), contact_ids.end());

		std::vector< std::vector<double> > distances(ids.size(), std::vector<double>(ids.size(), 0.0));

		for(std::size_t i=0;(i+1)<ids.size();i++)
		{
			const Contact& ci=data_manager.contacts()[ids[i]];
			const apollota::SimplePoint pi=(apollota::SimplePoint(data_manager.atoms()[ci.ids[0]].value)+apollota::SimplePoint(data_manager.atoms()[ci.ids[1]].value))*0.5;
			for(std::size_t j=(i+1);j<ids.size();j++)
			{
				const Contact& cj=data_manager.contacts()[ids[j]];
				const apollota::SimplePoint pj=(apollota::SimplePoint(data_manager.atoms()[cj.ids[0]].value)+apollota::SimplePoint(data_manager.atoms()[cj.ids[1]].value))*0.5;
				const double d=apollota::distance_from_point_to_point(pi, pj);
				distances[i][j]=d;
				distances[j][i]=d;
			}
		}

		std::vector<std::size_t> maxes(ids.size());
		{
			for(std::size_t i=0;i<ids.size();i++)
			{
				maxes[i]=i;
			}

			for(std::size_t i=0;(i+1)<ids.size();i++)
			{
				for(std::size_t j=(i+1);j<ids.size();j++)
				{
					const double d=distances[i][j];
					if(d>distances[i][maxes[i]])
					{
						maxes[i]=j;
					}
					if(d>distances[j][maxes[j]])
					{
						maxes[j]=i;
					}
				}
			}
		}

		std::vector< std::vector<double> > descriptors(ids.size(), std::vector<double>(3, 0.0));
		{
			for(std::size_t i=0;i<ids.size();i++)
			{
				const std::vector<double>& v=distances[i];
				double sum=0.0;
				for(std::size_t j=0;j<v.size();j++)
				{
					sum+=v[j];
				}
				const double mean=sum/static_cast<double>(v.size());
				double sum2=0.0;
				double sum3=0.0;
				for(std::size_t j=0;j<v.size();j++)
				{
					sum2+=(v[j]-mean)*(v[j]-mean);
					sum3+=(v[j]-mean)*(v[j]-mean)*(v[j]-mean);
				}
				descriptors[i][0]=mean;
				descriptors[i][1]=sum2/static_cast<double>(v.size());
				descriptors[i][2]=sum3/static_cast<double>(v.size());
			}
		}

		std::vector<std::string> names;
		names.push_back(name_prefix+"_a1");
		names.push_back(name_prefix+"_b1");
		names.push_back(name_prefix+"_c1");
		names.push_back(name_prefix+"_a2");
		names.push_back(name_prefix+"_b2");
		names.push_back(name_prefix+"_c2");
		names.push_back(name_prefix+"_a3");
		names.push_back(name_prefix+"_b3");
		names.push_back(name_prefix+"_c3");

		for(std::size_t i=0;i<ids.size();i++)
		{
			const std::size_t a=i;
			const std::size_t b=maxes[a];
			const std::size_t c=maxes[b];
			const std::size_t contact_id=ids[i];
			std::map<std::string, double>& contact_adjuncts=data_manager.contact_adjuncts_mutable(contact_id);
			contact_adjuncts[names[0]]=descriptors[a][0];
			contact_adjuncts[names[1]]=descriptors[b][0];
			contact_adjuncts[names[2]]=descriptors[c][0];
			contact_adjuncts[names[3]]=descriptors[a][1];
			contact_adjuncts[names[4]]=descriptors[b][1];
			contact_adjuncts[names[5]]=descriptors[c][1];
			contact_adjuncts[names[6]]=descriptors[a][2];
			contact_adjuncts[names[7]]=descriptors[b][2];
			contact_adjuncts[names[8]]=descriptors[c][2];
		}

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), contact_ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_ADJUNCTS_OF_CONTACTS_BY_UFSR_H_ */
