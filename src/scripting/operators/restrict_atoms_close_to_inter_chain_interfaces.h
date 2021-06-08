#ifndef SCRIPTING_OPERATORS_RESTRICT_ATOMS_CLOSE_TO_INTER_CHAIN_INTERFACES_H_
#define SCRIPTING_OPERATORS_RESTRICT_ATOMS_CLOSE_TO_INTER_CHAIN_INTERFACES_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class RestrictAtomsCloseToInterchainInterfaces : public OperatorBase<RestrictAtomsCloseToInterchainInterfaces>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary_old;
		SummaryOfAtoms atoms_summary_new;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary_old, heterostorage.variant_object.object("atoms_summary_old"));
			VariantSerialization::write(atoms_summary_new, heterostorage.variant_object.object("atoms_summary_new"));
		}
	};

	RestrictAtomsCloseToInterchainInterfaces()
	{
	}

	void initialize(CommandInput&)
	{
	}

	void document(CommandDocumentation&) const
	{
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		std::set<std::size_t> ids;

		std::vector<apollota::SimpleSphere> spheres(data_manager.atoms().size());
		for(std::size_t i=0;i<data_manager.atoms().size();i++)
		{
			spheres[i]=voronota::apollota::SimpleSphere(data_manager.atoms()[i].value, data_manager.atoms()[i].value.r+1.4);
		}

		const apollota::BoundingSpheresHierarchy bsh(spheres, 3.5, 1);

		for(std::size_t i=0;i<spheres.size();i++)
		{
			const std::vector<std::size_t> collisions=voronota::apollota::SearchForSphericalCollisions::find_all_collisions(bsh, spheres[i]);
			for(std::size_t j=0;j<collisions.size();j++)
			{
				if(data_manager.atoms()[collisions[j]].crad.chainID!=data_manager.atoms()[i].crad.chainID)
				{
					ids.insert(i);
					ids.insert(collisions[j]);
				}
			}
		}

		if(ids.size()<2)
		{
			throw std::runtime_error(std::string("Less than 2 atoms selected."));
		}

		Result result;

		result.atoms_summary_old=SummaryOfAtoms(data_manager.atoms());

		if(ids.size()<data_manager.atoms().size())
		{
			data_manager.restrict_atoms(ids);
			result.atoms_summary_new=SummaryOfAtoms(data_manager.atoms());
		}
		else
		{
			result.atoms_summary_new=result.atoms_summary_old;
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_RESTRICT_ATOMS_CLOSE_TO_INTER_CHAIN_INTERFACES_H_ */
