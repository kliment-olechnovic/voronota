#ifndef SCRIPTING_OPERATORS_SET_RESIDUE_NUMBERS_SEQUENTIALLY_H_
#define SCRIPTING_OPERATORS_SET_RESIDUE_NUMBERS_SEQUENTIALLY_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetResidueNumbersSequentially : public OperatorBase<SetResidueNumbersSequentially>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	SetResidueNumbersSequentially()
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

		std::vector<Atom> atoms=data_manager.atoms();

		int current_old_residue_number=0;
		int current_new_residue_number=0;

		for(std::size_t i=0;i<atoms.size();i++)
		{
			int& resSeq=atoms[i].crad.resSeq;
			if(i==0 || resSeq!=current_old_residue_number)
			{
				current_old_residue_number=resSeq;
				current_new_residue_number++;
			}
			resSeq=current_new_residue_number;
		}

		data_manager.reset_atoms_by_swapping(atoms);

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SET_RESIDUE_NUMBERS_SEQUENTIALLY_H_ */
