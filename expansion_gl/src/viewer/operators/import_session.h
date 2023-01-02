#ifndef VIEWER_OPERATORS_IMPORT_SESSION_H_
#define VIEWER_OPERATORS_IMPORT_SESSION_H_

#include "../operators_common.h"

#include "../../../expansion_js/src/dependencies/lodepng/lodepng.h"

namespace voronota
{

namespace viewer
{

namespace operators
{

class ImportSession : public scripting::OperatorBase<ImportSession>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		std::vector<scripting::VariantObject> objects;
		bool view_loaded;

		Result() : view_loaded(false)
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.objects_array("objects")=objects;
			heterostorage.variant_object.value("view_loaded")=view_loaded;
		}
	};

	std::string file;

	ImportSession()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(file.empty())
		{
			throw std::runtime_error(std::string("Empty input file name."));
		}

		std::string compressed_data_str;
		{
			scripting::InputSelector finput_selector(file);
			std::istream& finput=finput_selector.stream();

			if(!finput.good())
			{
				throw std::runtime_error(std::string("Failed to open file '")+file+"'.");
			}

			std::istreambuf_iterator<char> eos;
			compressed_data_str=std::string(std::istreambuf_iterator<char>(finput), eos);
		}

		if(compressed_data_str.empty())
		{
			throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
		}

		std::vector<unsigned char> uncompressed_data;
		int decompression_status=lodepng::decompress(uncompressed_data, reinterpret_cast<const unsigned char*>(compressed_data_str.c_str()), compressed_data_str.size());
		if(decompression_status!=0 || uncompressed_data.empty())
		{
			throw std::runtime_error(std::string("Failed to decompress data."));
		}

		std::istringstream str_input(std::string(reinterpret_cast<const char*>(&uncompressed_data[0]), uncompressed_data.size()));

		const std::vector<scripting::DataManager*> objects=congregation_of_data_managers.load_from_stream(str_input);
		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects loaded."));
		}

		std::vector<int> recommended_effective_rendering_size;

		const bool view_loaded=uv::ViewerApplication::instance().load_view_from_stream(str_input, recommended_effective_rendering_size);

		Result result;

		for(std::size_t i=0;i<objects.size();i++)
		{
			const scripting::CongregationOfDataManagers::ObjectAttributes attributes=congregation_of_data_managers.get_object_attributes(objects[i]);
			scripting::VariantObject info;
			info.value("name")=attributes.name;
			info.value("picked")=attributes.picked;
			info.value("visible")=attributes.visible;
			result.objects.push_back(info);
		}

		result.view_loaded=view_loaded;

		return result;
	}
};

}

}

}


#endif /* VIEWER_OPERATORS_IMPORT_SESSION_H_ */
