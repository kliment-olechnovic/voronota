#ifndef SCRIPTING_OPERATORS_CENTER_ATOMS_H_
#define SCRIPTING_OPERATORS_CENTER_ATOMS_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class CenterAtoms : public OperatorBase<CenterAtoms>
{
public:
    struct Result : public OperatorResultBase<Result>
    {
    	SummaryOfAtoms atoms_summary;

    	void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
		}
    };

	CenterAtoms()
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

		const std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(SelectionManager::Query());
		if(ids.size()!=data_manager.atoms().size())
		{
			throw std::runtime_error(std::string("Not all atoms selected."));
		}

		const SummaryOfAtoms summary_of_atoms(data_manager.atoms(), ids);

		const apollota::SimplePoint translation=apollota::SimplePoint(0.0, 0.0, 0.0)-((summary_of_atoms.bounding_box.p_min+summary_of_atoms.bounding_box.p_max)*0.5);

		std::vector<double> translation_vector(3);
		translation_vector[0]=translation.x;
		translation_vector[1]=translation.y;
		translation_vector[2]=translation.z;

		data_manager.transform_coordinates_of_atoms(ids, translation_vector, std::vector<double>(), std::vector<double>(), std::vector<double>());

        Result result;
        result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

        return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_CENTER_ATOMS_H_ */

