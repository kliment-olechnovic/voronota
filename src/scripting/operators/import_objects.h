#ifndef SCRIPTING_OPERATORS_IMPORT_OBJECTS_H_
#define SCRIPTING_OPERATORS_IMPORT_OBJECTS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ImportObjects : public OperatorBase<ImportObjects>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::vector<VariantObject> objects;
		SummaryOfAtoms summary_of_atoms;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.objects_array("objects")=objects;
			if(summary_of_atoms.bounding_box.filled)
			{
				heterostorage.summaries_of_atoms["zoomed"]=summary_of_atoms;
				VariantSerialization::write(summary_of_atoms.bounding_box, heterostorage.variant_object.object("bounding_box"));
			}
		}
	};

	std::string file;

	ImportObjects()
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

	Result run(CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(file.empty())
		{
			throw std::runtime_error(std::string("Empty input file name."));
		}

		InputSelector finput_selector(file);
		std::istream& finput=finput_selector.stream();

		if(!finput.good())
		{
			throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
		}

		const std::vector<DataManager*> objects=congregation_of_data_managers.load_from_stream(finput);
		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects loaded."));
		}

		Result result;

		for(std::size_t i=0;i<objects.size();i++)
		{
			const CongregationOfDataManagers::ObjectAttributes attributes=congregation_of_data_managers.get_object_attributes(objects[i]);
			VariantObject info;
			info.value("name")=attributes.name;
			info.value("picked")=attributes.picked;
			info.value("visible")=attributes.visible;
			result.objects.push_back(info);
			result.summary_of_atoms.feed(SummaryOfAtoms(objects[i]->atoms()));
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_IMPORT_OBJECTS_H_ */
