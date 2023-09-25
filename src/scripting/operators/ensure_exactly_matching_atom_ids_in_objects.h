#ifndef SCRIPTING_OPERATORS_ENSURE_EXACTLY_MATCHING_ATOM_IDS_IN_OBJECTS_H_
#define SCRIPTING_OPERATORS_ENSURE_EXACTLY_MATCHING_ATOM_IDS_IN_OBJECTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class EnsureExactlyMatchingAtomIDsInObjects : public OperatorBase<EnsureExactlyMatchingAtomIDsInObjects>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::vector<VariantObject> objects;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.objects_array("objects")=objects;
		}
	};

	CongregationOfDataManagers::ObjectQuery query;

	EnsureExactlyMatchingAtomIDsInObjects()
	{
	}

	void initialize(CommandInput& input)
	{
		query=OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		congregation_of_data_managers.assert_objects_availability();

		const std::vector<DataManager*> objects=congregation_of_data_managers.get_objects(query);
		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}

		typedef std::map< common::ChainResidueAtomDescriptor, std::map<std::size_t, std::size_t> > MapOfIDs;
		MapOfIDs map_of_atom_ids;

		for(std::size_t i=0;i<objects.size();i++)
		{
			const DataManager& data_manager=(*(objects[i]));
			for(std::size_t j=0;j<data_manager.atoms().size();j++)
			{
				const Atom& atom=data_manager.atoms()[j];
				common::ChainResidueAtomDescriptor crad;
				crad.chainID=atom.crad.chainID;
				crad.resSeq=atom.crad.resSeq;
				crad.iCode=atom.crad.iCode;
				crad.name=atom.crad.name;
				map_of_atom_ids[crad][i]=j;
			}
		}

		std::size_t count_of_complete=0;

		for(MapOfIDs::const_iterator it=map_of_atom_ids.begin();it!=map_of_atom_ids.end();++it)
		{
			if(it->second.size()==objects.size())
			{
				count_of_complete++;
			}
		}

		if(count_of_complete<1)
		{
			throw std::runtime_error(std::string("Objects do not have matching atom IDs."));
		}

		std::vector< std::vector<std::size_t> > objects_atom_ids(objects.size());

		for(MapOfIDs::const_iterator it=map_of_atom_ids.begin();it!=map_of_atom_ids.end();++it)
		{
			const std::map<std::size_t, std::size_t>& mapped_atom_ids=it->second;
			if(mapped_atom_ids.size()==objects.size())
			{
				for(std::map<std::size_t, std::size_t>::const_iterator jt=mapped_atom_ids.begin();jt!=mapped_atom_ids.end();++jt)
				{
					std::vector<std::size_t>& object_atom_ids=objects_atom_ids[jt->first];
					if(object_atom_ids.empty())
					{
						object_atom_ids.reserve(count_of_complete);
					}
					object_atom_ids.push_back(jt->second);
				}
			}
		}

		Result result;

		for(std::size_t i=0;i<objects.size();i++)
		{
			int atoms_before=0;
			int atoms_after=0;

			{
				DataManager& data_manager=(*(objects[i]));
				const std::vector<std::size_t>& object_atom_ids=objects_atom_ids[i];
				std::vector<Atom> restricted_atoms;
				restricted_atoms.reserve(object_atom_ids.size());
				for(std::size_t j=0;j<object_atom_ids.size();j++)
				{
					restricted_atoms.push_back(data_manager.atoms()[object_atom_ids[j]]);
				}
				atoms_before=data_manager.atoms().size();
				data_manager.reset_atoms_by_swapping(restricted_atoms);
				atoms_after=data_manager.atoms().size();
			}

			{
				const CongregationOfDataManagers::ObjectAttributes attributes=congregation_of_data_managers.get_object_attributes(objects[i]);
				VariantObject info;
				info.value("name")=attributes.name;
				info.value("atoms_before")=atoms_before;
				info.value("atoms_after")=atoms_after;
				result.objects.push_back(info);
			}
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_ENSURE_EXACTLY_MATCHING_ATOM_IDS_IN_OBJECTS_H_ */

