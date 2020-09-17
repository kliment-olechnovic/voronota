#ifndef NNPORT_TABLE_H_
#define NNPORT_TABLE_H_

#include <iostream>
#include <set>
#include <limits>

#include "read_vector.h"

namespace voronota
{

namespace nnport
{

class Table
{
public:
	Table(std::istream& input, const std::size_t min_number_of_rows=1, const std::size_t max_number_of_rows=std::numeric_limits<std::size_t>::max())
	{
		read(input, min_number_of_rows, max_number_of_rows);
	}

	bool add_row(std::istream& input)
	{
		std::vector<std::string> row;
		while(input.good() && row.empty())
		{
			row=read_vector<std::string>(input, header_.size());
		}
		if(!row.empty())
		{
			rows_.push_back(row);
			return true;
		}
		return false;
	}

	void clear_rows()
	{
		rows_.clear();
	}

	const std::vector<std::string>& header() const
	{
		return header_;
	}

	const std::vector< std::vector<std::string> >& rows() const
	{
		return rows_;
	}

	std::size_t select_column(const std::string& column_name) const
	{
		for(std::size_t i=0;i<header_.size();i++)
		{
			if(header_[i]==column_name)
			{
				return i;
			}
		}
		throw std::runtime_error(std::string("No column named '")+column_name+"'");
		return header_.size();
	}

	std::vector<std::size_t> select_columns(const std::vector<std::string>& column_names) const
	{
		std::vector<std::size_t> column_ids;
		column_ids.reserve(column_names.size());
		for(std::size_t i=0;i<column_names.size();i++)
		{
			column_ids.push_back(select_column(column_names[i]));
		}
		return column_ids;
	}

	template<typename T>
	T get_value(const std::size_t column_id, const std::size_t row_id) const
	{
		assert_column_id(column_id);
		assert_row_id(row_id);
		std::istringstream input(rows_[row_id][column_id]);
		T value;
		input >> value;
		if(input.fail())
		{
			throw std::runtime_error(std::string("Failed to convert table value '")+rows_[row_id][column_id]+"'");
		}
		return value;
	}

	template<typename T>
	std::vector<T> get_row_values(const std::vector<std::size_t>& column_ids, const std::size_t row_id) const
	{
		std::vector<T> row_values;
		row_values.reserve(column_ids.size());
		for(std::size_t i=0;i<column_ids.size();i++)
		{
			row_values.push_back(get_value<T>(column_ids[i], row_id));
		}
		return row_values;
	}

private:
	void read(std::istream& input, const std::size_t min_number_of_rows, const std::size_t max_number_of_rows)
	{
		header_=nnport::read_vector<std::string>(input);
		assert_header();
		while(input.good() && rows_.size()<max_number_of_rows)
		{
			const std::vector<std::string> row=nnport::read_vector<std::string>(input, header_.size());
			if(!row.empty())
			{
				rows_.push_back(row);
			}
		}
		if(rows_.size()<min_number_of_rows)
		{
			throw std::runtime_error(std::string("Number of rows is fewer than the requested minimum"));
		}
	}

	void assert_header() const
	{
		if(header_.empty())
		{
			throw std::runtime_error(std::string("No table header"));
		}
		{
			std::set<std::string> names;
			for(std::size_t i=0;i<header_.size();i++)
			{
				if(names.count(header_[i])>0)
				{
					throw std::runtime_error(std::string("Table header has repeating names"));
				}
				names.insert(header_[i]);
			}
		}
	}

	void assert_column_id(const std::size_t column_id) const
	{
		if(column_id>=header_.size())
		{
			throw std::runtime_error(std::string("Invalid table column index"));
		}
	}

	void assert_row_id(const std::size_t row_id) const
	{
		if(row_id>=rows_.size())
		{
			throw std::runtime_error(std::string("Invalid table row index"));
		}
	}

	std::vector<std::string> header_;
	std::vector< std::vector<std::string> > rows_;
};

}

}

#endif /* NNPORT_TABLE_H_ */
