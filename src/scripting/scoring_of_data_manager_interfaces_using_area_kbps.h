#ifndef SCRIPTING_SCORING_OF_DATA_MANAGER_INTERFACES_USING_AREA_KBPS_H_
#define SCRIPTING_SCORING_OF_DATA_MANAGER_INTERFACES_USING_AREA_KBPS_H_

#include "../common/contacts_scoring_utilities.h"

#include "virtual_file_storage.h"
#include "data_manager.h"

namespace voronota
{

namespace scripting
{

class ScoringOfDataManagerInterfacesUsingAreaKBPs
{
public:
	class Configuration
	{
	public:
		std::vector<std::string> kbp_names;
		std::map< common::ChainResidueAtomDescriptorsPair, std::vector<double> > kbp_coeffs;

		bool valid() const
		{
			return (!kbp_names.empty() && !kbp_coeffs.empty() && kbp_coeffs.begin()->second.size()==kbp_names.size());
		}

		static const Configuration& get_default_configuration()
		{
			return get_default_configuration_mutable();
		}

		static bool setup_default_configuration(const std::string& kbps_file)
		{
			if(kbps_file.empty())
			{
				return false;
			}

			std::vector<std::string> raw_kbp_names;
			std::map< common::ChainResidueAtomDescriptorsPair, std::vector<double> > raw_kbp_coeffs;

			{
				InputSelector kbps_file_input_selector(kbps_file);
				std::istream& finput=kbps_file_input_selector.stream();
				if(!finput.good())
				{
					return false;
				}
				if(!read_kbps_names_and_coeffs(finput, raw_kbp_names, raw_kbp_coeffs))
				{
					return false;
				}
			}

			get_default_configuration_mutable().kbp_names.swap(raw_kbp_names);
			get_default_configuration_mutable().kbp_coeffs.swap(raw_kbp_coeffs);

			return get_default_configuration().valid();
		}

	private:
		static Configuration& get_default_configuration_mutable()
		{
			static Configuration configuration;
			return configuration;
		}
	};

	struct Parameters
	{
		SelectionManager::Query parameters_for_selecting;
		std::string adjunct_prefix;
		std::string global_adjunct_prefix;

		Parameters()
		{
		}
	};

	struct Result
	{
		std::vector<std::string> score_names;
		std::vector<double> score_values;
		std::vector<double> score_values_solvated;
		double known_area;
		double unknown_area;
		SummaryOfContacts contacts_summary;

		Result() : known_area(0.0), unknown_area(0.0)
		{
		}
	};

	static void construct_result(const Configuration& configuration, const Parameters& params, DataManager& data_manager, Result& result)
	{
		result=Result();

		if(!configuration.valid())
		{
			throw std::runtime_error(std::string("Invalid configuration."));
		}

		data_manager.assert_contacts_availability();

		const std::set<std::size_t> all_contact_ids=data_manager.selection_manager().select_contacts(params.parameters_for_selecting);

		if(all_contact_ids.empty())
		{
			throw std::runtime_error(std::string("No suitable contacts."));
		}

		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), all_contact_ids);

		if(!params.adjunct_prefix.empty())
		{
			for(std::size_t j=0;j<configuration.kbp_names.size();j++)
			{
				const std::string adjunct_name=params.adjunct_prefix+"_"+configuration.kbp_names[j];
				const std::string adjunct_coeff_name=adjunct_name+"_coeff";
				for(std::size_t i=0;i<data_manager.contacts().size();i++)
				{
					data_manager.contact_adjuncts_mutable(i).erase(adjunct_name);
					data_manager.contact_adjuncts_mutable(i).erase(adjunct_coeff_name);
				}
			}
		}

		result.score_names=configuration.kbp_names;
		result.score_values.resize(configuration.kbp_names.size());
		result.score_values_solvated.resize(configuration.kbp_names.size());

		for(std::set<std::size_t>::const_iterator it=all_contact_ids.begin();it!=all_contact_ids.end();++it)
		{
			const Contact& contact=data_manager.contacts()[*it];
			const common::ChainResidueAtomDescriptorsPair crads=common::generalize_crads_pair(common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), contact));
			const common::ChainResidueAtomDescriptorsPair crads_solv1=common::generalize_crads_pair(common::ChainResidueAtomDescriptorsPair(data_manager.atoms()[contact.ids[0]].crad, common::ChainResidueAtomDescriptor::solvent()));
			const common::ChainResidueAtomDescriptorsPair crads_solv2=common::generalize_crads_pair(common::ChainResidueAtomDescriptorsPair(data_manager.atoms()[contact.ids[1]].crad, common::ChainResidueAtomDescriptor::solvent()));
			std::map< common::ChainResidueAtomDescriptorsPair, std::vector<double> >::const_iterator kbp_it=configuration.kbp_coeffs.find(crads);
			std::map< common::ChainResidueAtomDescriptorsPair, std::vector<double> >::const_iterator kbp_solv1_it=configuration.kbp_coeffs.find(crads_solv1);
			std::map< common::ChainResidueAtomDescriptorsPair, std::vector<double> >::const_iterator kbp_solv2_it=configuration.kbp_coeffs.find(crads_solv2);
			if(kbp_it==configuration.kbp_coeffs.end() || kbp_solv1_it==configuration.kbp_coeffs.end() || kbp_solv2_it==configuration.kbp_coeffs.end())
			{
				result.unknown_area+=contact.value.area;
			}
			else
			{
				result.known_area+=contact.value.area;
				for(std::size_t i=0;i<configuration.kbp_names.size();i++)
				{
					result.score_values[i]+=kbp_it->second[i]*contact.value.area;
					result.score_values_solvated[i]+=(kbp_solv1_it->second[i]+kbp_solv2_it->second[i])*contact.value.area;
				}
				if(!params.adjunct_prefix.empty())
				{
					std::map<std::string, double>& map_of_contact_adjuncts=data_manager.contact_adjuncts_mutable(*it);
					for(std::size_t i=0;i<configuration.kbp_names.size();i++)
					{
						map_of_contact_adjuncts[params.adjunct_prefix+"_"+configuration.kbp_names[i]]=kbp_it->second[i]*contact.value.area;
						map_of_contact_adjuncts[params.adjunct_prefix+"_"+configuration.kbp_names[i]+"_coeff"]=kbp_it->second[i];
						map_of_contact_adjuncts[params.adjunct_prefix+"_"+configuration.kbp_names[i]+"_solvated"]=(kbp_solv1_it->second[i]+kbp_solv2_it->second[i])*contact.value.area;
						map_of_contact_adjuncts[params.adjunct_prefix+"_"+configuration.kbp_names[i]+"_solvated_coeff"]=(kbp_solv1_it->second[i]+kbp_solv2_it->second[i]);
					}
				}
			}
		}

		if(!params.global_adjunct_prefix.empty())
		{
			for(std::size_t i=0;i<result.score_names.size();i++)
			{
				data_manager.global_numeric_adjuncts_mutable()[params.global_adjunct_prefix+"_"+result.score_names[i]]=result.score_values[i];
				data_manager.global_numeric_adjuncts_mutable()[params.global_adjunct_prefix+"_"+result.score_names[i]+"_mean"]=result.score_values[i]/result.known_area;
				data_manager.global_numeric_adjuncts_mutable()[params.global_adjunct_prefix+"_"+result.score_names[i]+"_solvated"]=result.score_values_solvated[i];
				data_manager.global_numeric_adjuncts_mutable()[params.global_adjunct_prefix+"_"+result.score_names[i]+"_solvated_mean"]=result.score_values_solvated[i]/result.known_area;
			}
		}
	}

	static void construct_result(const Parameters& params, DataManager& data_manager, Result& result)
	{
		construct_result(Configuration::get_default_configuration(), params, data_manager, result);
	}

	static bool read_kbps_names_and_coeffs(std::istream& input_stream, std::vector<std::string>& kbp_names, std::map< common::ChainResidueAtomDescriptorsPair, std::vector<double> >& kbp_coeffs)
	{
		std::vector<std::string> header;

		{
			input_stream >> std::ws;
			std::string line;
			std::getline(input_stream, line);
			if(!line.empty())
			{
				std::istringstream linput(line);
				while(linput.good())
				{
					std::string token;
					linput >> token;
					if(!token.empty())
					{
						header.push_back(token);
					}
				}
			}
		}

		if(header.size()<5 || header[0]!="res1" || header[1]!="atom1" || header[2]!="res2" || header[3]!="atom2")
		{
			return false;
		}

		kbp_names=std::vector<std::string>(header.begin()+4, header.end());

		{
			std::pair< common::ChainResidueAtomDescriptorsPair, std::vector<double> > iname_coeffs;
			iname_coeffs.second.resize(kbp_names.size(), 0.0);
			input_stream >> std::ws;
			while(input_stream.good())
			{
				input_stream >> iname_coeffs.first.a.resName;
				if(input_stream.fail())
				{
					return false;
				}
				input_stream >> iname_coeffs.first.a.name;
				if(input_stream.fail())
				{
					return false;
				}
				input_stream >> iname_coeffs.first.b.resName;
				if(input_stream.fail())
				{
					return false;
				}
				input_stream >> iname_coeffs.first.b.name;
				if(input_stream.fail())
				{
					return false;
				}
				if(iname_coeffs.first.b.resName=="ZSR")
				{
					iname_coeffs.first.b=common::ChainResidueAtomDescriptor::solvent();
				}
				else
				{
					iname_coeffs.first.b.chainID.clear();
				}
				for(std::size_t i=0;i<iname_coeffs.second.size();i++)
				{
					input_stream >> iname_coeffs.second[i];
					if(input_stream.fail())
					{
						return false;
					}
				}
				kbp_coeffs.insert(kbp_coeffs.end(), iname_coeffs);
				input_stream >> std::ws;
			}
		}

		return !kbp_coeffs.empty();
	}
};

}

}

#endif /* SCRIPTING_SCORING_OF_DATA_MANAGER_INTERFACES_USING_AREA_KBPS_H_ */
