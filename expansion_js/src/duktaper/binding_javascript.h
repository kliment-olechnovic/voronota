#ifndef DUKTAPER_BINDING_JAVASCRIPT_H_
#define DUKTAPER_BINDING_JAVASCRIPT_H_

#include <sstream>

#include "../../../src/scripting/command_documentation.h"

namespace voronota
{

namespace duktaper
{

class BindingJavascript
{
public:
	static std::string generate_setup_script(const scripting::CollectionOfCommandDocumentations& cds, const bool restrictive)
	{
		std::ostringstream script;

		script << ""
				"raw_voronota_parameters_object_to_array=function(obj)"
				"{"
				"  var result=[];"
				"  var obj_type=Object.prototype.toString.call(obj);"
				"  if(obj_type==='[object Object]')"
				"  {"
				"    for(var key in obj)"
				"    {"
				"      result.push('-'+key.split('_').join('-'));"
				"      var value=obj[key];"
				"      var value_type=Object.prototype.toString.call(value);"
				"      if(value_type==='[object Array]')"
				"      {"
				"        for(var i=0;i<value.length;i++)"
				"        {"
				"          result.push(value[i]);"
				"        }"
				"      }"
				"      else"
				"      {"
				"        result.push(value);"
				"      }"
				"    }"
				"  }"
				"  else if(obj_type==='[object Array]')"
				"  {"
				"    for(var i=0;i<obj.length;i++)"
				"    {"
				"      result.push(obj[i]);"
				"    }"
				"  }"
				"  else"
				"  {"
				"    result.push(obj);"
				"  }"
				"  return result;"
				"}"
				"\n"
				"raw_voronota_named=function(name, args)"
				"{"
				"  var full_str=name;"
				"  for(var i=0;i<args.length;++i)"
				"  {"
				"    var values=raw_voronota_parameters_object_to_array(args[i]);"
				"    for(var j=0;j<values.length;j++)"
				"    {"
				"      value=''+values[j];"
				"      full_str+=' ';"
				"      if(value.indexOf('-')===0)"
				"      {"
				"        full_str+=value;"
				"      }"
				"      else if(value.indexOf(\"'\")===-1)"
				"      {"
				"        full_str+=\"'\";"
				"        full_str+=value;"
				"        full_str+=\"'\";"
				"      }"
				"      else if(value.indexOf('\"')===-1)"
				"      {"
				"        full_str+='\"';"
				"        full_str+=value;"
				"        full_str+='\"';"
				"      }"
				"      else"
				"      {"
				"        full_str+=value;"
				"      }"
				"    }"
				"  }"
				"  return raw_voronota(full_str);"
				"}"
				"\n";

		if(!restrictive)
		{
			script << ""
					"exit=function(){return raw_voronota('exit');}"
					"\n";
		}

		script << ""
				"Voronota={}"
				"\n"
				"voronota_do=function(str){return raw_voronota(str);}"
				"\n"
				"Voronota.do=voronota_do;"
				"\n"
				"voronota_last_output=function(){return raw_voronota_last_output();}"
				"\n"
				"Voronota.last_output=voronota_last_output;"
				"\n"
				"voronota_assert_full_success=function(str)"
				"{"
				"  if(voronota_last_output().results_summary.full_success!==true)"
				"  {"
				"    log(voronota_last_output());"
				"    throw (str);"
				"  }"
				"  return true;"
				"}"
				"\n"
				"Voronota.assert_full_success=voronota_assert_full_success;"
				"\n"
				"voronota_assert_partial_success=function(str)"
				"{"
				"  if(voronota_last_output().results_summary.partial_success!==true)"
				"  {"
				"    log(voronota_last_output());"
				"    throw (str);"
				"  }"
				"  return true;"
				"}"
				"\n"
				"Voronota.assert_partial_success=voronota_assert_partial_success;"
				"\n"
				"voronota_auto_assert_full_success=false;"
				"Voronota.auto_assert_full_success=voronota_auto_assert_full_success;"
				"\n"
				"voronota_auto_assert_partial_success=false;"
				"Voronota.auto_assert_partial_success=voronota_auto_assert_partial_success;"
				"\n";

		const std::vector<std::string> command_names=cds.get_all_names();

		for(std::size_t i=0;i<command_names.size();i++)
		{
			const std::string& command_name=command_names[i];
			std::string function_name=command_name;
			for(std::size_t j=0;j<function_name.size();j++)
			{
				if(function_name[j]=='-')
				{
					function_name[j]='_';
				}
			}
			script << "voronota_" << function_name << "=function(){var result=raw_voronota_named('";
			script << command_name;
			script << "', arguments);"
					"\n"
					"if(voronota_auto_assert_full_success){voronota_assert_full_success('Failed call to voronota_";
			script << function_name;
			script << "');}"
					"\n"
					"if(voronota_auto_assert_partial_success){voronota_assert_partial_success('Failed call to voronota_";
			script << function_name;
			script << "');}"
					"\n"
					"return result;}"
					"\n";
			script << "Voronota." << function_name << "=" << "voronota_" << function_name << ";\n";
		}

		return script.str();
	}
};

}

}

#endif /* DUKTAPER_BINDING_JAVASCRIPT_H_ */
