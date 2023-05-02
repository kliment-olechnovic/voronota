#ifndef DUKTAPER_OPERATORS_RUN_HBPLUS_H_
#define DUKTAPER_OPERATORS_RUN_HBPLUS_H_

#include "../../../../src/scripting/operators/export_atoms.h"
#include "../../../../src/scripting/operators/cat_files.h"

#include "../call_shell_utilities.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class RunHBPlus : public scripting::OperatorBase<RunHBPlus>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		std::string hbplus_stdout;
		std::string hbplus_stderr;
		std::string pairs_file;
		std::string hb2_file;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("hbplus_output")=hbplus_stdout;
			heterostorage.variant_object.value("hbplus_log")=hbplus_stderr;
			heterostorage.variant_object.value("pairs_file")=pairs_file;
			heterostorage.variant_object.value("hb2_file")=hb2_file;
		}
	};

	std::string pairs_file;
	std::string select_contacts;
	std::string hb2_file;

	RunHBPlus()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		pairs_file=input.get_value_or_default<std::string>("pairs-file", "");
		select_contacts=input.get_value_or_default<std::string>("select-contacts", "");
		hb2_file=input.get_value_or_default<std::string>("hb2-file", "");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("pairs-file", CDOD::DATATYPE_STRING, "path to output pairs file", ""));
		doc.set_option_decription(CDOD("select-contacts", CDOD::DATATYPE_STRING, "name of selection to select h-bonding contacts", ""));
		doc.set_option_decription(CDOD("hb2-file", CDOD::DATATYPE_STRING, "path to output hb2 file", ""));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		if(pairs_file.empty() && select_contacts.empty() && hb2_file.empty())
		{
			throw std::runtime_error(std::string("No output outcome specified."));
		}

		scripting::assert_selection_name_input(select_contacts, true);

		data_manager.assert_atoms_availability();

		if(!select_contacts.empty())
		{
			data_manager.assert_contacts_availability();
		}

		if(!CallShellUtilities::test_if_shell_command_available("hbplus"))
		{
			throw std::runtime_error(std::string("hbplus application not available."));
		}

		const CallShellUtilities::TemporaryDirectory tmpdir;

		scripting::operators::ExportAtoms().init(CMDIN()
				.set("as-pdb", true)
				.set("file", (tmpdir.dir_path()+"/struct.pdb"))).run(data_manager);

		Result result;

		{
			std::ostringstream command_output;
			command_output << "cd '" << tmpdir.dir_path() << "'\n";
			command_output << "hbplus ./struct.pdb";
			operators::CallShell::Result hbplus_result=operators::CallShell().init(CMDIN().set("command-string", command_output.str())).run(0);
			if(hbplus_result.exit_status!=0)
			{
				throw std::runtime_error(std::string("hbplus call failed."));
			}
			result.hbplus_stdout=hbplus_result.stdout_str;
			result.hbplus_stderr=hbplus_result.stderr_str;
		}

		const std::string hbplus_file_name=(tmpdir.dir_path()+"/struct.hb2");

		std::set<common::ChainResidueAtomDescriptorsPair> set_of_hbplus_crad_pairs;

		{
			scripting::InputSelector finput_selector(hbplus_file_name);
			std::istream& finput=finput_selector.stream();

			if(!finput.good())
			{
				throw std::runtime_error(std::string("No HBPlus output file produced."));
			}

			set_of_hbplus_crad_pairs=read_set_of_hbplus_crad_pairs(finput);

			if(set_of_hbplus_crad_pairs.empty())
			{
				throw std::runtime_error(std::string("No pairs parsed from HBPlus output file."));
			}
		}

		if(!select_contacts.empty())
		{
			std::set<std::size_t> ids=data_manager.selection_manager().select_contacts_by_set_of_crads_pairs(set_of_hbplus_crad_pairs);
			if(ids.empty())
			{
				throw std::runtime_error(std::string("No contacts selected."));
			}

			data_manager.selection_manager().set_contacts_selection(select_contacts, ids);
		}

		if(!pairs_file.empty())
		{
			scripting::OutputSelector output_selector(pairs_file);
			std::ostream& output=output_selector.stream();
			scripting::assert_io_stream(pairs_file, output);

			auxiliaries::IOUtilities().write_set(set_of_hbplus_crad_pairs, output);
		}

		if(!hb2_file.empty())
		{
			scripting::operators::CatFiles().init(CMDIN()
					.set("files", hbplus_file_name)
					.set("files", hb2_file, true)).run(0);

			result.hb2_file=hb2_file;
		}

		return result;
	}

private:
	static std::set<common::ChainResidueAtomDescriptorsPair> read_set_of_hbplus_crad_pairs(std::istream& input)
	{
		std::set<common::ChainResidueAtomDescriptorsPair> set_of_hbplus_crad_pairs;
		voronota::auxiliaries::AtomsIO::HBPlusReader::Data hbplus_file_data=voronota::auxiliaries::AtomsIO::HBPlusReader::read_data_from_file_stream(input);
		if(!hbplus_file_data.hbplus_records.empty())
		{
			for(std::vector<voronota::auxiliaries::AtomsIO::HBPlusReader::HBPlusRecord>::const_iterator it=hbplus_file_data.hbplus_records.begin();it!=hbplus_file_data.hbplus_records.end();++it)
			{
				const voronota::auxiliaries::AtomsIO::HBPlusReader::ShortAtomDescriptor& a=it->first;
				const voronota::auxiliaries::AtomsIO::HBPlusReader::ShortAtomDescriptor& b=it->second;
				set_of_hbplus_crad_pairs.insert(
						common::ChainResidueAtomDescriptorsPair(
								common::ChainResidueAtomDescriptor(common::ChainResidueAtomDescriptor::null_num(), a.chainID, a.resSeq, a.resName, a.name, "", ""),
								common::ChainResidueAtomDescriptor(common::ChainResidueAtomDescriptor::null_num(), b.chainID, b.resSeq, b.resName, b.name, "", "")));
			}
		}
		return set_of_hbplus_crad_pairs;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_RUN_HBPLUS_H_ */
