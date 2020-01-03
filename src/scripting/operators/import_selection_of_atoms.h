#ifndef SCRIPTING_OPERATORS_IMPORT_SELECTION_OF_ATOMS_H_
#define SCRIPTING_OPERATORS_IMPORT_SELECTION_OF_ATOMS_H_

#include "../operators_common.h"

namespace scripting
{

namespace operators
{

class ImportSelectionOfAtoms
{
public:
	struct Result
	{
		SummaryOfAtoms atoms_summary;
		std::string selection_name;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.variant_object.value("selection_name")=selection_name;
			return (*this);
		}
	};

	std::string file;
	std::string name;
	bool no_serial;
	bool no_name;
	bool no_resSeq;
	bool no_resName;

	ImportSelectionOfAtoms() : file(""), name(""), no_serial(false), no_name(false), no_resSeq(false), no_resName(false)
	{
	}

	ImportSelectionOfAtoms& init(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		assert_file_name_input(file, false);
		name=input.get_value<std::string>("name");
		assert_selection_name_input(name, false);
		no_serial=input.get_flag("no-serial");
		no_name=input.get_flag("no-name");
		no_resSeq=input.get_flag("no-resSeq");
		no_resName=input.get_flag("no-resName");
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_file_name_input(file, false);
		assert_selection_name_input(name, false);

		if(file.empty())
		{
			throw std::runtime_error(std::string("Empty input selection file name."));
		}

		assert_selection_name_input(name, false);

		InputSelector finput_selector(file);
		std::istream& finput=finput_selector.stream();

		if(!finput.good())
		{
			throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
		}

		std::set<common::ChainResidueAtomDescriptor> set_of_crads;

		auxiliaries::IOUtilities().read_lines_to_set(finput, set_of_crads);

		if(set_of_crads.empty())
		{
			throw std::runtime_error(std::string("No descriptors in file '")+file+"'.");
		}

		if(no_serial || no_name || no_resSeq || no_resName)
		{
			std::set<common::ChainResidueAtomDescriptor> refined_set_of_crads;
			for(std::set<common::ChainResidueAtomDescriptor>::const_iterator it=set_of_crads.begin();it!=set_of_crads.end();++it)
			{
				refined_set_of_crads.insert(it->without_some_info(no_serial, no_name, no_resSeq, no_resName));
			}
			set_of_crads.swap(refined_set_of_crads);
		}

		std::set<std::size_t> ids=data_manager.selection_manager().select_atoms_by_set_of_crads(set_of_crads);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		if(!name.empty())
		{
			data_manager.selection_manager().set_atoms_selection(name, ids);
		}

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), ids);
		result.selection_name=name;

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_IMPORT_SELECTION_OF_ATOMS_H_ */

