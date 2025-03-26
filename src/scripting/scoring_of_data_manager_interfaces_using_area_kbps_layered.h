#ifndef SCRIPTING_SCORING_OF_DATA_MANAGER_INTERFACES_USING_AREA_KBPS_LAYERED_H_
#define SCRIPTING_SCORING_OF_DATA_MANAGER_INTERFACES_USING_AREA_KBPS_LAYERED_H_

#include "../common/contacts_scoring_utilities.h"

#include "io_selectors.h"
#include "data_manager.h"
#include "primitive_chemistry_annotation.h"

namespace voronota
{

namespace scripting
{

class ScoringOfDataManagerInterfacesUsingAreaKBPsLayered
{
public:
	class Configuration
	{
	public:
		std::vector<std::string> kbp_names;
		std::map< std::string, std::map< common::ChainResidueAtomDescriptorsPair, std::vector<double> > > layers_kbp_coeffs;
		std::map<std::string, double> summing_weights;
		bool has_primitive_chemistry_types;
		bool has_coarse_chemistry_types;

		Configuration() : has_primitive_chemistry_types(false), has_coarse_chemistry_types(false)
		{
		}

		bool valid() const
		{
			return (!kbp_names.empty() && !layers_kbp_coeffs.empty() && !layers_kbp_coeffs.begin()->second.empty() && layers_kbp_coeffs.begin()->second.begin()->second.size()==kbp_names.size());
		}

		static const Configuration& get_default_configuration()
		{
			return get_default_configuration_mutable();
		}

		static bool setup_default_configuration(const std::string& kbps_file, const std::string& summing_weights_file)
		{
			if(kbps_file.empty())
			{
				return false;
			}

			std::map<std::string, double> raw_summing_weights;

			if(!summing_weights_file.empty())
			{
				InputSelector summing_weights_file_input_selector(summing_weights_file);
				std::istream& finput=summing_weights_file_input_selector.stream();
				if(!finput.good())
				{
					return false;
				}
				if(!read_summing_weights(finput, raw_summing_weights))
				{
					return false;
				}
			}

			std::vector<std::string> raw_kbp_names;
			std::map< std::string, std::map< common::ChainResidueAtomDescriptorsPair, std::vector<double> > > raw_layers_kbp_coeffs;
			bool raw_has_primitive_chemistry_types=false;
			bool raw_has_coarse_chemistry_types=false;

			{
				InputSelector kbps_file_input_selector(kbps_file);
				std::istream& finput=kbps_file_input_selector.stream();
				if(!finput.good())
				{
					return false;
				}
				if(!read_kbps_names_and_coeffs(finput, raw_kbp_names, raw_layers_kbp_coeffs, raw_has_primitive_chemistry_types, raw_has_coarse_chemistry_types))
				{
					return false;
				}
			}

			get_default_configuration_mutable().kbp_names.swap(raw_kbp_names);
			get_default_configuration_mutable().layers_kbp_coeffs.swap(raw_layers_kbp_coeffs);
			get_default_configuration_mutable().summing_weights.swap(raw_summing_weights);
			get_default_configuration_mutable().has_primitive_chemistry_types=raw_has_primitive_chemistry_types;
			get_default_configuration_mutable().has_coarse_chemistry_types=raw_has_coarse_chemistry_types;

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
		bool verbose_adjuncts;

		Parameters() : verbose_adjuncts(false)
		{
		}
	};

	struct Result
	{
		std::vector<std::string> score_names;
		std::vector<double> score_values;
		double known_area;
		double weighted_sum;
		SummaryOfContacts contacts_summary;

		Result() : known_area(0.0), weighted_sum(0.0)
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

		std::vector<double> score_weights(configuration.kbp_names.size(), 0.0);
		if(!configuration.summing_weights.empty())
		{
			for(std::size_t i=0;i<configuration.kbp_names.size();i++)
			{
				std::map<std::string, double>::const_iterator it=configuration.summing_weights.find(configuration.kbp_names[i]);
				if(it!=configuration.summing_weights.end())
				{
					score_weights[i]=(it->second);
				}
			}
		}

		for(std::set<std::size_t>::const_iterator it=all_contact_ids.begin();it!=all_contact_ids.end();++it)
		{
			const Contact& contact=data_manager.contacts()[*it];

			const common::ChainResidueAtomDescriptorsPair crads=common::generalize_crads_pair(common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), contact));

			std::vector<double> score_values(configuration.kbp_names.size(), 0.0);
			double known_area=0.0;

			for(std::map< std::string, std::map< common::ChainResidueAtomDescriptorsPair, std::vector<double> > >::const_iterator layer_it=configuration.layers_kbp_coeffs.begin();layer_it!=configuration.layers_kbp_coeffs.end();++layer_it)
			{
				const std::string& layer=layer_it->first;
				const std::map< common::ChainResidueAtomDescriptorsPair, std::vector<double> >& coeffs=layer_it->second;
				std::map< common::ChainResidueAtomDescriptorsPair, std::vector<double> >::const_iterator kbp_it=coeffs.find(crads);
				if(configuration.has_primitive_chemistry_types && kbp_it==coeffs.end())
				{
					common::ChainResidueAtomDescriptor alt_crad_a;
					alt_crad_a.name=PrimitiveChemistryAnnotation::get_knodle_atom_type_string(crads.a);
					if(!alt_crad_a.name.empty())
					{
						alt_crad_a.resName="ANY";
						const common::ChainResidueAtomDescriptorsPair alt_crads_ao(alt_crad_a, crads.b);
						kbp_it=coeffs.find(alt_crads_ao);
						if(kbp_it==coeffs.end())
						{
							common::ChainResidueAtomDescriptor alt_crad_b;
							alt_crad_b.name=PrimitiveChemistryAnnotation::get_knodle_atom_type_string(crads.b);
							if(!alt_crad_b.name.empty())
							{
								alt_crad_b.resName="ANY";
								const common::ChainResidueAtomDescriptorsPair alt_crads_ob(crads.a, alt_crad_b);
								kbp_it=coeffs.find(alt_crads_ob);
								if(kbp_it==coeffs.end())
								{
									const common::ChainResidueAtomDescriptorsPair alt_crads_ab(alt_crad_a, alt_crad_b);
									kbp_it=coeffs.find(alt_crads_ab);
								}
							}
						}
					}
				}
				if(configuration.has_coarse_chemistry_types && kbp_it==coeffs.end())
				{
					common::ChainResidueAtomDescriptor alt_crad_a;
					alt_crad_a.resName=crads.a.resName;
					alt_crad_a.name="ANY";
					const common::ChainResidueAtomDescriptorsPair alt_crads_ao(alt_crad_a, crads.b);
					kbp_it=coeffs.find(alt_crads_ao);
					if(kbp_it==coeffs.end())
					{
						common::ChainResidueAtomDescriptor alt_crad_b;
						alt_crad_b.resName=crads.b.resName;
						alt_crad_b.name="ANY";
						const common::ChainResidueAtomDescriptorsPair alt_crads_ob(crads.a, alt_crad_b);
						kbp_it=coeffs.find(alt_crads_ob);
						if(kbp_it==coeffs.end())
						{
							const common::ChainResidueAtomDescriptorsPair alt_crads_ab(alt_crad_a, alt_crad_b);
							kbp_it=coeffs.find(alt_crads_ab);
						}
					}
				}
				if(kbp_it!=coeffs.end() && kbp_it->second.size()==configuration.kbp_names.size())
				{
					std::map<std::string, double>::const_iterator adjuncts_it=contact.value.props.adjuncts.find(layer);
					if(adjuncts_it!=contact.value.props.adjuncts.end())
					{
						const double subarea=adjuncts_it->second;
						known_area+=subarea;
						for(std::size_t i=0;i<configuration.kbp_names.size();i++)
						{
							score_values[i]+=kbp_it->second[i]*subarea;
						}
					}
				}
			}

			if(known_area>0.0)
			{
				known_area=std::min(known_area, contact.value.area);

				double weighted_sum=0.0;

				if(!configuration.summing_weights.empty())
				{
					for(std::size_t i=0;i<score_weights.size();i++)
					{
						weighted_sum+=score_values[i]*score_weights[i];
					}
				}

				result.known_area+=known_area;
				result.weighted_sum+=weighted_sum;

				for(std::size_t i=0;i<configuration.kbp_names.size();i++)
				{
					result.score_values[i]+=score_values[i];
				}

				if(!params.adjunct_prefix.empty())
				{
					std::map<std::string, double>& map_of_contact_adjuncts=data_manager.contact_adjuncts_mutable(*it);
					map_of_contact_adjuncts[params.adjunct_prefix+"_known_area"]=known_area;
					map_of_contact_adjuncts[params.adjunct_prefix+"_weighted_sum"]=weighted_sum;
					if(params.verbose_adjuncts)
					{
						map_of_contact_adjuncts[params.adjunct_prefix+"_weighted_mean"]=weighted_sum/known_area;
					}
					for(std::size_t i=0;i<configuration.kbp_names.size();i++)
					{
						map_of_contact_adjuncts[params.adjunct_prefix+"_"+configuration.kbp_names[i]]=score_values[i];
						if(params.verbose_adjuncts)
						{
							map_of_contact_adjuncts[params.adjunct_prefix+"_"+configuration.kbp_names[i]+"_mean"]=score_values[i]/known_area;
							map_of_contact_adjuncts[params.adjunct_prefix+"_"+configuration.kbp_names[i]+"_weighted"]=(score_values[i]*score_weights[i]);
							map_of_contact_adjuncts[params.adjunct_prefix+"_"+configuration.kbp_names[i]+"_weighted_mean"]=(score_values[i]*score_weights[i])/known_area;
						}
					}
				}
			}
		}

		if(!params.global_adjunct_prefix.empty())
		{
			data_manager.global_numeric_adjuncts_mutable()[params.global_adjunct_prefix+"_known_area"]=result.known_area;
			data_manager.global_numeric_adjuncts_mutable()[params.global_adjunct_prefix+"_weighted_sum"]=result.weighted_sum;
			if(params.verbose_adjuncts)
			{
				data_manager.global_numeric_adjuncts_mutable()[params.global_adjunct_prefix+"_weighted_mean"]=result.weighted_sum/result.known_area;
			}
			for(std::size_t i=0;i<result.score_names.size();i++)
			{
				data_manager.global_numeric_adjuncts_mutable()[params.global_adjunct_prefix+"_"+result.score_names[i]]=result.score_values[i];
				if(params.verbose_adjuncts)
				{
					data_manager.global_numeric_adjuncts_mutable()[params.global_adjunct_prefix+"_"+result.score_names[i]+"_mean"]=result.score_values[i]/result.known_area;
					data_manager.global_numeric_adjuncts_mutable()[params.global_adjunct_prefix+"_"+result.score_names[i]+"_weighted"]=(result.score_values[i]*score_weights[i]);
					data_manager.global_numeric_adjuncts_mutable()[params.global_adjunct_prefix+"_"+result.score_names[i]+"_weighted_mean"]=(result.score_values[i]*score_weights[i])/result.known_area;
				}
			}
		}
	}

	static void construct_result(const Parameters& params, DataManager& data_manager, Result& result)
	{
		construct_result(Configuration::get_default_configuration(), params, data_manager, result);
	}

	static bool read_kbps_names_and_coeffs(std::istream& input_stream, std::vector<std::string>& kbp_names, std::map< std::string, std::map< common::ChainResidueAtomDescriptorsPair, std::vector<double> > >& layers_kbp_coeffs, bool& has_primitive_chemistry_types, bool& has_coarse_chemistry_types)
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

		if(header.size()<6 || header[0]!="res1" || header[1]!="atom1" || header[2]!="res2" || header[3]!="atom2" || header[4]!="cat")
		{
			return false;
		}

		kbp_names=std::vector<std::string>(header.begin()+5, header.end());

		{
			std::string layer;
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
				input_stream >> layer;
				if(input_stream.fail())
				{
					return false;
				}
				for(std::size_t i=0;i<iname_coeffs.second.size();i++)
				{
					input_stream >> iname_coeffs.second[i];
					if(input_stream.fail())
					{
						return false;
					}
				}
				std::map< common::ChainResidueAtomDescriptorsPair, std::vector<double> >& layer_map=layers_kbp_coeffs[layer];
				layer_map.insert(layer_map.end(), iname_coeffs);
				has_primitive_chemistry_types=(has_primitive_chemistry_types || iname_coeffs.first.a.resName=="ANY" || iname_coeffs.first.b.resName=="ANY");
				has_coarse_chemistry_types=(has_coarse_chemistry_types || iname_coeffs.first.a.name=="ANY" || iname_coeffs.first.b.name=="ANY");
				input_stream >> std::ws;
			}
		}

		return !layers_kbp_coeffs.empty();
	}

	static bool read_summing_weights(std::istream& input_stream, std::map<std::string, double>& summing_weights)
	{
		summing_weights.clear();

		input_stream >> std::ws;
		while(input_stream.good())
		{
			std::string line;
			std::getline(input_stream, line);
			if(!line.empty())
			{
				std::istringstream linput(line);
				std::string name;
				linput >> name;
				if(linput.fail() || name.empty())
				{
					return false;
				}
				double value=0.0;
				linput >> value;
				if(linput.fail())
				{
					return false;
				}
				summing_weights[name]=value;
			}
			input_stream >> std::ws;
		}

		return !summing_weights.empty();
	}
};

}

}

#endif /* SCRIPTING_SCORING_OF_DATA_MANAGER_INTERFACES_USING_AREA_KBPS_LAYERED_H_ */

