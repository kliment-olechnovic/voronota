#ifndef NNPORT_STATISTICS_MAP_H_
#define NNPORT_STATISTICS_MAP_H_

#include <map>

#include "table.h"

namespace voronota
{

namespace nnport
{

class StatisticsMap
{
public:
	struct Statistics
	{
		float mean;
		float sd;

		Statistics() : mean(0.0), sd(1.0)
		{
		}
	};

	StatisticsMap()
	{
	}

	StatisticsMap(std::istream& statistics_file_input)
	{
		const nnport::Table statistics_table(statistics_file_input);
		const std::size_t category_column_id=statistics_table.select_column("category");
		const std::size_t value_name_column_id=statistics_table.select_column("value_name");
		const std::size_t value_mean_column_id=statistics_table.select_column("value_mean");
		const std::size_t value_sd_column_id=statistics_table.select_column("value_sd");
		for(std::size_t i=0;i<statistics_table.rows().size();i++)
		{
			Statistics svs;
			svs.mean=statistics_table.get_value<float>(value_mean_column_id, i);
			svs.sd=statistics_table.get_value<float>(value_sd_column_id, i);
			const std::pair<std::string, std::string> key(statistics_table.get_value<std::string>(category_column_id, i), statistics_table.get_value<std::string>(value_name_column_id, i));
			map_[key]=svs;
		}
	}

	const Statistics get_statistics(const std::string& category, const std::string& value_name) const
	{
		Statistics svs;
		if(map_.empty())
		{
			return svs;
		}
		const std::pair<std::string, std::string> key(category, value_name);
		std::map< std::pair<std::string, std::string>, Statistics >::const_iterator it=map_.find(key);
		if(it!=map_.end())
		{
			svs=it->second;
		}
		return svs;
	}

private:
	std::map< std::pair<std::string, std::string>, Statistics > map_;
};

}

}

#endif /* NNPORT_STATISTICS_MAP_H_ */
