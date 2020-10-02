#ifndef SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_PROJECTION_H_
#define SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_PROJECTION_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetAdjunctOfAtomsByProjection : public OperatorBase<SetAdjunctOfAtomsByProjection>
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

	SelectionManager::Query parameters_for_selecting;
	std::vector<double> axis;
	std::vector<double> origin;
	std::string adjunct;

	SetAdjunctOfAtomsByProjection()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		axis=input.get_value_vector<double>("axis");
		origin=input.get_value_vector_or_default<double>("origin", std::vector<double>(3, 0.0));
		adjunct=input.get_value<std::string>("adjunct");
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("axis", CDOD::DATATYPE_FLOAT_ARRAY, "axis to project on"));
		doc.set_option_decription(CDOD("origin", CDOD::DATATYPE_FLOAT_ARRAY, "origin for the axis"));
		doc.set_option_decription(CDOD("adjunct", CDOD::DATATYPE_STRING, "output adjunct name"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_adjunct_name_input(adjunct, false);

		if(axis.size()!=3)
		{
			throw std::runtime_error(std::string("Axis is not properly defined, three coordinates needed."));
		}

		if(origin.size()!=3)
		{
			throw std::runtime_error(std::string("Origin is not properly defined, three coordinates needed."));
		}

		std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		const apollota::SimplePoint unit_axis=apollota::SimplePoint(axis[0], axis[1], axis[2]).unit();
		const apollota::SimplePoint origin_point=apollota::SimplePoint(origin[0], origin[1], origin[2]);

		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			std::map<std::string, double>& atom_adjuncts=data_manager.atom_adjuncts_mutable(*it);
			const apollota::SimplePoint atom_point(data_manager.atoms()[*it].value);
			atom_adjuncts[adjunct]=(atom_point-origin_point)*unit_axis;
		}

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_ADJUNCT_OF_ATOMS_BY_PROJECTION_H_ */
