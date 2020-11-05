#ifndef SCRIPTING_OPERATORS_CALCULATE_BURIAL_DEPTH_H_
#define SCRIPTING_OPERATORS_CALCULATE_BURIAL_DEPTH_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class CalculateBurialDepth : public OperatorBase<CalculateBurialDepth>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string name;
	int min_seq_sep;
	double min_sas_area;
	unsigned int smoothing_iterations;

	CalculateBurialDepth() : min_seq_sep(1), min_sas_area(0.0), smoothing_iterations(0)
	{
	}

	void initialize(CommandInput& input)
	{
		name=input.get_value<std::string>("name");
		min_seq_sep=input.get_value_or_default<int>("min-seq-sep", 1);
		min_sas_area=input.get_value_or_default<double>("min-sas-area", 0.0);
		smoothing_iterations=input.get_value_or_default<unsigned int>("smoothing-iterations", 0);
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("name", CDOD::DATATYPE_STRING, "adjunct name to write value in atoms"));
		doc.set_option_decription(CDOD("min-seq-sep", CDOD::DATATYPE_INT, "minimal sequence separation restriction for contacts", 1));
		doc.set_option_decription(CDOD("min-sas-area", CDOD::DATATYPE_FLOAT, "minimal SAS area to define surface atoms", 0.0));
		doc.set_option_decription(CDOD("smoothing-iterations", CDOD::DATATYPE_INT, "number of smoothing iterations", 0));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();

		assert_adjunct_name_input(name, false);

		std::set<common::ChainResidueAtomDescriptorsPair> set_of_contacts;
		for(std::size_t i=0;i<data_manager.contacts().size();i++)
		{
			const Contact& contact=data_manager.contacts()[i];
			if(!(contact.solvent() && contact.value.area<min_sas_area))
			{
				const common::ChainResidueAtomDescriptorsPair crads=common::ConversionOfDescriptors::get_contact_descriptor(data_manager.atoms(), contact);
				if(common::ChainResidueAtomDescriptor::match_with_sequence_separation_interval(crads.a, crads.b, min_seq_sep, common::ChainResidueAtomDescriptor::null_num(), true))
				{
					set_of_contacts.insert(crads);
				}
			}
		}

		const std::map<common::ChainResidueAtomDescriptor, int> raw_map_crad_to_depth=common::ChainResidueAtomDescriptorsGraphOperations::calculate_burial_depth_values(set_of_contacts);

		std::map<common::ChainResidueAtomDescriptor, double> map_crad_to_depth;
		for(std::map<common::ChainResidueAtomDescriptor, int>::const_iterator it=raw_map_crad_to_depth.begin();it!=raw_map_crad_to_depth.end();++it)
		{
			map_crad_to_depth[it->first]=static_cast<double>(it->second);
		}

		if(smoothing_iterations>0)
		{
			std::map< common::ChainResidueAtomDescriptor, std::set<common::ChainResidueAtomDescriptor> > graph;
			for(std::set<common::ChainResidueAtomDescriptorsPair>::const_iterator contacts_it=set_of_contacts.begin();contacts_it!=set_of_contacts.end();++contacts_it)
			{
				const common::ChainResidueAtomDescriptorsPair& crads=(*contacts_it);
				if(crads.a!=crads.b && map_crad_to_depth.count(crads.a)>0 && map_crad_to_depth.count(crads.b)>0)
				{
					graph[crads.a].insert(crads.b);
					graph[crads.b].insert(crads.a);
				}
			}

			for(unsigned int iteration=0;iteration<smoothing_iterations;iteration++)
			{
				std::map<common::ChainResidueAtomDescriptor, double> updated_map_crad_to_depth;
				for(std::map< common::ChainResidueAtomDescriptor, std::set<common::ChainResidueAtomDescriptor> >::const_iterator graph_it=graph.begin();graph_it!=graph.end();++graph_it)
				{
					const std::set<common::ChainResidueAtomDescriptor>& neighbors=graph_it->second;
					double sum=map_crad_to_depth[graph_it->first];
					for(std::set<common::ChainResidueAtomDescriptor>::const_iterator neighbors_it=neighbors.begin();neighbors_it!=neighbors.end();++neighbors_it)
					{
						sum+=map_crad_to_depth[*neighbors_it];
					}
					updated_map_crad_to_depth[graph_it->first]=(sum/static_cast<double>(neighbors.size()+1));
				}
				map_crad_to_depth=updated_map_crad_to_depth;
			}
		}


		for(std::size_t i=0;i<data_manager.atoms().size();i++)
		{
			const Atom& atom=data_manager.atoms()[i];
			std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(i);
			atom_adjuncts.erase(name);
			std::map<common::ChainResidueAtomDescriptor, double>::const_iterator it=map_crad_to_depth.find(atom.crad);
			if(it!=map_crad_to_depth.end())
			{
				atom_adjuncts[name]=it->second;
			}
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_CALCULATE_BURIAL_DEPTH_H_ */
