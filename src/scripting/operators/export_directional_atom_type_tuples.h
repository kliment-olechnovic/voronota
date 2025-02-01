#ifndef SCRIPTING_OPERATORS_EXPORT_DIRECTIONAL_ATOM_TYPE_TUPLES_H_
#define SCRIPTING_OPERATORS_EXPORT_DIRECTIONAL_ATOM_TYPE_TUPLES_H_

#include "../operators_common.h"

#include "../primitive_atom_directions_assignment.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ExportDirectionalAtomTypeTuples : public OperatorBase<ExportDirectionalAtomTypeTuples>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string file;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
		}
	};

    std::string file;

	ExportDirectionalAtomTypeTuples()
	{
	}

	void initialize(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
	}

	Result run(DataManager& data_manager) const
	{
		assert_file_name_input(file, false);

		PrimitiveAtomDirectionsAssignment::Result atom_directions_assignment_result;

		PrimitiveAtomDirectionsAssignment::construct_result(data_manager, atom_directions_assignment_result);

		OutputSelector output_selector(file);
		std::ostream& output=output_selector.stream();
		assert_io_stream(file, output);

		for(std::size_t i=0;i<atom_directions_assignment_result.directional_neighbors.size();i++)
		{
			const Atom& central_atom=data_manager.atoms()[i];
			output << central_atom.crad.resName << " " << central_atom.crad.name;
			for(std::size_t l=0;l<3;l++)
			{
				if(l<atom_directions_assignment_result.directional_neighbors[i].size())
				{
					output << " " << data_manager.atoms()[atom_directions_assignment_result.directional_neighbors[i][l]].crad.name;
				}
				else
				{
					output << " XXX";
				}
			}
			output << "\n";
		}

		Result result;
		result.file=file;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_DIRECTIONAL_ATOM_TYPE_TUPLES_H_ */
