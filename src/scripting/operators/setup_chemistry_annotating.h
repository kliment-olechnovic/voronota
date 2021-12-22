#ifndef SCRIPTING_OPERATORS_SETUP_CHEMISTRY_ANNOTATING_H_
#define SCRIPTING_OPERATORS_SETUP_CHEMISTRY_ANNOTATING_H_

#include "../operators_common.h"
#include "../primitive_chemistry_annotation.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class SetupChemistryAnnotating : public OperatorBase<SetupChemistryAnnotating>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string more_atom_types_file;

	SetupChemistryAnnotating()
	{
	}

	void initialize(CommandInput& input)
	{
		more_atom_types_file=input.get_value<std::string>("more-atom-types-file");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("more-atom-types-file", CDOD::DATATYPE_STRING, "path to file with more atom types"));
	}

	Result run(void*) const
	{
		if(!PrimitiveChemistryAnnotation::Configuration::setup_default_configuration(more_atom_types_file))
		{
			throw std::runtime_error(std::string("Failed to setup chemistry annotating."));
		}

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_SETUP_CHEMISTRY_ANNOTATING_H_ */
