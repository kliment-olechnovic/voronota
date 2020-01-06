#ifndef SCRIPTING_OPERATORS_UTILITIES_H_
#define SCRIPTING_OPERATORS_UTILITIES_H_

#include "data_manager.h"
#include "congregation_of_data_managers.h"
#include "heterogeneous_storage.h"
#include "basic_assertions.h"
#include "variant_serialization.h"
#include "command_input.h"
#include "filtering_of_triangulation.h"
#include "io_selectors.h"

namespace scripting
{

namespace operators
{

class Utilities
{
public:
	static SelectionManager::Query read_generic_selecting_query(const std::string& prefix, const std::string& default_expression, CommandInput& input)
	{
		const std::string type_for_expression=prefix+"use";
		const std::string type_for_full_residues=prefix+"full-residues";
		const std::string type_for_forced_id=prefix+"id";

		SelectionManager::Query query(default_expression);

		if(prefix.empty())
		{
			query.expression_string=input.get_value_or_first_unused_unnamed_value_starting_with_prefix_or_default(type_for_expression, "([", true, default_expression);
		}
		else
		{
			query.expression_string=input.get_value_or_default<std::string>(type_for_expression, default_expression);
		}

		query.full_residues=input.get_flag(type_for_full_residues);

		{
			const std::vector<std::size_t> forced_ids_vector=input.get_value_vector_or_default<std::size_t>(type_for_forced_id, std::vector<std::size_t>());
			if(!forced_ids_vector.empty())
			{
				query.from_ids.insert(forced_ids_vector.begin(), forced_ids_vector.end());
			}
		}

		return query;
	}

	static SelectionManager::Query read_generic_selecting_query(CommandInput& input)
	{
		return read_generic_selecting_query("", "[]", input);
	}

	static auxiliaries::ColorUtilities::ColorInteger read_color(CommandInput& input)
	{
		return auxiliaries::ColorUtilities::color_from_name(input.get_value_or_first_unused_unnamed_value_starting_with_prefix("col", "0x", false));
	}

	static auxiliaries::ColorUtilities::ColorInteger get_next_random_color()
	{
		const unsigned long m=2147483647L;
		const unsigned long a=48271L;
		const unsigned long c=0L;

		static unsigned long x=111L;

		static double value_prev=-1.0;

		static std::vector<auxiliaries::ColorUtilities::ColorInteger> anchors;

		if(anchors.empty())
		{
			anchors.push_back(0xFF4444);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0xFF4444);
			anchors.push_back(0x888888);
			anchors.push_back(0xFF4444);

			anchors.push_back(0xFF6644);

			anchors.push_back(0xFF8844);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0xFF8844);
			anchors.push_back(0x888888);
			anchors.push_back(0xFF8844);

			anchors.push_back(0xFFCC44);

			anchors.push_back(0xFFFF44);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0xFFFF44);
			anchors.push_back(0x888888);
			anchors.push_back(0xFFFF44);

			anchors.push_back(0xCCFF44);

			anchors.push_back(0x88FF44);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0x88FF44);
			anchors.push_back(0x888888);
			anchors.push_back(0x88FF44);

			anchors.push_back(0x66FF44);

			anchors.push_back(0x44FF44);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0x44FF44);
			anchors.push_back(0x888888);
			anchors.push_back(0x44FF44);

			anchors.push_back(0x44FF66);

			anchors.push_back(0x44FF88);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0x44FF88);
			anchors.push_back(0x888888);
			anchors.push_back(0x44FF88);

			anchors.push_back(0x44FFCC);

			anchors.push_back(0x44FFFF);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0x44FFFF);
			anchors.push_back(0x888888);
			anchors.push_back(0x44FFFF);

			anchors.push_back(0x44CCFF);

			anchors.push_back(0x4488FF);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0x4488FF);
			anchors.push_back(0x888888);
			anchors.push_back(0x4488FF);

			anchors.push_back(0x4466FF);

			anchors.push_back(0x4444FF);
			anchors.push_back(0xEEEEEE);
			anchors.push_back(0x4444FF);
			anchors.push_back(0x888888);
			anchors.push_back(0x4444FF);
		}

		x=((a*x+c)%m);

		double value=static_cast<double>(x%100000L)/static_cast<double>(99999L);

		if(value_prev>=0.0 && std::abs(value-value_prev)<0.1)
		{
			value+=((value<value_prev) ? -0.1 : 0.1);
			if(value<0.0)
			{
				value+=1.0;
			}
			if(value>1.0)
			{
				value-=1.0;
			}
		}

		value_prev=value;

		return auxiliaries::ColorUtilities::color_from_gradient(anchors, value);
	}

	static double calculate_zscore_reverse(const double zscore, const double mean_of_values, const double sd_of_values)
	{
		return ((zscore*sd_of_values)+mean_of_values);
	}

	static void calculate_spectrum_info(
			const bool as_z_scores,
			const bool min_val_present,
			const double min_val,
			const bool max_val_present,
			const double max_val,
			double& min_val_actual,
			double& max_val_actual,
			int& num_of_vals,
			double& mean_of_values,
			double& sd_of_values,
			std::map<std::size_t, double>& map_of_ids_values)
	{
		{
			double sum_of_vals=0.0;
			double sum_of_squared_vals=0.0;

			for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
			{
				const double val=it->second;
				if(it==map_of_ids_values.begin() || min_val_actual>val)
				{
					min_val_actual=val;
				}
				if(it==map_of_ids_values.begin() || max_val_actual<val)
				{
					max_val_actual=val;
				}
				num_of_vals++;
				sum_of_vals+=val;
				sum_of_squared_vals+=val*val;
			}

			mean_of_values=(sum_of_vals/static_cast<double>(num_of_vals));
			sd_of_values=sqrt((sum_of_squared_vals/static_cast<double>(num_of_vals))-(mean_of_values*mean_of_values));
		}

		const double min_val_to_use=(min_val_present ?
				(as_z_scores ? calculate_zscore_reverse(min_val, mean_of_values, sd_of_values) : min_val) :
				min_val_actual);
		const double max_val_to_use=(max_val_present ?
				(as_z_scores ? calculate_zscore_reverse(max_val, mean_of_values, sd_of_values) : max_val) :
				max_val_actual);

		if(max_val_to_use<min_val_to_use)
		{
			throw std::runtime_error(std::string("Minimum and maximum values do not define range."));
		}

		for(std::map<std::size_t, double>::iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
		{
			double& val=it->second;
			if(val<=min_val_to_use)
			{
				val=0.0;
			}
			else if(val>=max_val_to_use)
			{
				val=1.0;
			}
			else
			{
				val=(val-min_val_to_use)/(max_val_to_use-min_val_to_use);
			}
		}
	}

	static FilteringOfTriangulation::Query read_filtering_of_triangulation_query(CommandInput& input)
	{
		FilteringOfTriangulation::Query query;
		query.strict=input.get_flag("strict");
		query.max_edge=input.get_value_or_default<double>("max-edge", query.max_edge);
		query.min_radius=input.get_value_or_default<double>("min-radius", query.min_radius);
		query.max_radius=input.get_value_or_default<double>("max-radius", query.max_radius);
		query.expansion=input.get_value_or_default<double>("expansion", query.expansion);
		return query;
	}

	static void assert_new_object_name_input(const std::string& name)
	{
		if(name.empty())
		{
			throw std::runtime_error(std::string("New name is empty."));
		}
		if(name.find_first_of("{}()[]<>\\/*/'\"@#$%^&`~?|")!=std::string::npos)
		{
			throw std::runtime_error(std::string("New name '")+name+"' contains invalid symbols.");
		}
		else if(name.find_first_of("-+,;.: ", 0)==0)
		{
			throw std::runtime_error(std::string("New name '")+name+"' starts with invalid symbol.");
		}
	}

	static std::string get_basename_from_path(const std::string& path)
	{
		const std::size_t pos=path.find_last_of("/\\");
		if(pos==std::string::npos)
		{
			return path;
		}
		else if((pos+1)<path.size())
		{
			return path.substr(pos+1);
		}
		else
		{
			return std::string();
		}
	}

	static CongregationOfDataManagers::ObjectQuery read_congregation_of_data_managers_object_query(CommandInput& input)
	{
		CongregationOfDataManagers::ObjectQuery query;

		query.picked=input.get_flag("picked");
		query.not_picked=input.get_flag("not-picked");
		query.visible=input.get_flag("visible");
		query.not_visible=input.get_flag("not-visible");

		const std::vector<std::string> names=input.get_value_vector_or_all_unnamed_values("names");
		query.names.insert(names.begin(), names.end());

		return query;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_UTILITIES_H_ */
