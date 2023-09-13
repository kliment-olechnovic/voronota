#ifndef SCRIPTING_OPERATORS_IMPORT_MANY_H_
#define SCRIPTING_OPERATORS_IMPORT_MANY_H_

#include "import.h"
#include "split_pdb_file.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ImportMany : public OperatorBase<ImportMany>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary;
		SummaryOfContacts contacts_summary;
		std::vector<std::string> object_names;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
			heterostorage.summaries_of_atoms["loaded"]=atoms_summary;
			if(contacts_summary.number_total>0)
			{
				VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
				heterostorage.summaries_of_contacts["loaded"]=contacts_summary;
			}
			std::vector<VariantValue>& object_names_array=heterostorage.variant_object.values_array("object_names");
			for(std::size_t i=0;i<object_names.size();i++)
			{
				object_names_array.push_back(VariantValue(object_names[i]));
			}
		}

		void add(const Import::Result& import_result)
		{
			atoms_summary.feed(import_result.atoms_summary);
			contacts_summary.feed(import_result.contacts_summary);
			object_names.insert(object_names.end(), import_result.object_names.begin(), import_result.object_names.end());
		}
	};

	bool split_pdb_files;
	std::vector<std::string> files;
	Import import_operator;
	std::string title;

	ImportMany() : split_pdb_files(false)
	{
	}

	void initialize(CommandInput& input, const bool managed)
	{
		if(!managed)
		{
			files=input.get_value_vector_or_all_unused_unnamed_values("files");
		}
		split_pdb_files=input.get_flag("split-pdb-files");
		import_operator.initialize(input, true);
		title=import_operator.title;
	}

	void initialize(CommandInput& input)
	{
		initialize(input, false);
	}

	void document(CommandDocumentation& doc, const bool managed) const
	{
		if(!managed)
		{
			doc.set_option_decription(CDOD("files", CDOD::DATATYPE_STRING_ARRAY, "paths to files"));
		}
		doc.set_option_decription(CDOD("split-pdb-files", CDOD::DATATYPE_BOOL, "flag to split PDB files by models"));
		import_operator.document(doc, true);
	}

	void document(CommandDocumentation& doc) const
	{
		document(doc, false);
	}

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(files.empty())
		{
			throw std::runtime_error(std::string("Missing file paths."));
		}

		Result result;

		for(std::size_t i=0;i<files.size();i++)
		{
			const std::string& main_file=files[i];
			const std::string main_file_basename=(title.empty() ? OperatorsUtilities::get_basename_from_path(main_file) : title);
			bool unsplit_import=true;
			if(split_pdb_files && (import_operator.loading_parameters.format=="pdb" || LoadingOfData::get_format_from_atoms_file_name(main_file)=="pdb"))
			{
				scripting::VirtualFileStorage::TemporaryFile tmpfile;
				const std::string prefix=tmpfile.filename()+"/"+main_file_basename+"_state_";

				std::vector<std::string> subfiles=SplitPDBFile().init(CMDIN()
						.set("input-file", main_file)
						.set("prefix", prefix)).run(0).result_filenames;

				if(!subfiles.empty())
				{
					const scripting::VirtualFileStorage::AutodeleterOfFiles adf(subfiles);

					for(std::size_t j=0;j<subfiles.size();j++)
					{
						Import import_operator_to_use=import_operator;
						import_operator_to_use.loading_parameters.file=subfiles[j];
						import_operator_to_use.title=OperatorsUtilities::get_basename_from_path(subfiles[j]);
						result.add(import_operator_to_use.run(congregation_of_data_managers));
					}

					unsplit_import=false;
				}
			}
			if(unsplit_import)
			{
				Import import_operator_to_use=import_operator;
				import_operator_to_use.loading_parameters.file=main_file;
				import_operator_to_use.title=(title.empty() ? OperatorsUtilities::get_basename_from_path(main_file) : title);
				result.add(import_operator_to_use.run(congregation_of_data_managers));
			}
		}

		congregation_of_data_managers.set_all_objects_picked(false);
		for(std::size_t i=0;i<result.object_names.size();i++)
		{
			congregation_of_data_managers.set_object_picked(result.object_names[i], true);
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_IMPORT_MANY_H_ */
