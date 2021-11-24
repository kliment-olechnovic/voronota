#ifndef SCRIPTING_OPERATORS_SET_ATOM_SERIALS_H_
#define SCRIPTING_OPERATORS_SET_ATOM_SERIALS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetAtomSerials : public OperatorBase<SetAtomSerials>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	bool to_null;

	SetAtomSerials() : to_null(false)
	{
	}

	void initialize(CommandInput& input)
	{
		to_null=input.get_flag("to-null");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("to-null", CDOD::DATATYPE_BOOL, "flag to drop serial numbers"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		std::vector<Atom> atoms=data_manager.atoms();

		bool updated=false;

		for(std::size_t i=0;i<atoms.size();i++)
		{
			int new_serial=common::ChainResidueAtomDescriptor::null_num();
			if(!to_null)
			{
				new_serial=static_cast<int>(i);
			}
			if(atoms[i].crad.serial!=new_serial)
			{
				atoms[i].crad.serial=new_serial;
				updated=true;
			}
		}

		if(updated)
		{
			data_manager.reset_atoms_by_swapping(atoms);
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_ATOM_SERIALS_H_ */
