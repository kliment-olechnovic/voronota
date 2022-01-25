#ifndef SCRIPTING_OPERATORS_REDUNDANCY_SCORE_H_
#define SCRIPTING_OPERATORS_REDUNDANCY_SCORE_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class RedundancyScore : public OperatorBase<RedundancyScore>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string input_similarities_file;
	std::string input_ids_file;
	std::string output_file;

	RedundancyScore()
	{
	}

	void initialize(CommandInput& input)
	{
		input_similarities_file=input.get_value<std::string>("input-similarities-file");
		input_ids_file=input.get_value<std::string>("input-ids-file");
		output_file=input.get_value<std::string>("output-file");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("input-similarities-file", CDOD::DATATYPE_STRING, "path to input similarities file"));
		doc.set_option_decription(CDOD("input-ids-file", CDOD::DATATYPE_STRING, "path to IDs file"));
		doc.set_option_decription(CDOD("output-file", CDOD::DATATYPE_STRING, "path to output file"));
	}

	Result run(void*) const
	{
		assert_file_name_input(input_similarities_file, false);
		assert_file_name_input(input_ids_file, false);
		assert_file_name_input(output_file, false);

		typedef std::map<std::string, std::map<std::string, double> > MapOfSimilarities;
		MapOfSimilarities map_of_similarities;

		std::vector<std::string> ordered_ids;

		{
			std::set<std::string> set_of_all_ids;

			{
				InputSelector finput_selector(input_similarities_file);
				std::istream& finput=finput_selector.stream();
				assert_io_stream(input_similarities_file, finput);

				while(finput.good())
				{
					std::string line;
					std::getline(finput, line);
					if(!line.empty())
					{
						std::string id1;
						std::string id2;
						double value=-1.0;
						std::istringstream line_input(line);
						line_input >> id1 >> id2 >> value;
						if(id1.empty() || id2.empty() || value<0.0 || value>1.0)
						{
							throw std::runtime_error(std::string("Invalid similarities file line '")+line+"'.");
						}
						if(id1!=id2)
						{
							map_of_similarities[id1][id2]=value;
							set_of_all_ids.insert(id1);
							set_of_all_ids.insert(id2);
						}
					}
				}
			}

			if(map_of_similarities.empty())
			{
				throw std::runtime_error(std::string("No similarities read."));
			}

			if(set_of_all_ids.size()!=map_of_similarities.size())
			{
				throw std::runtime_error(std::string("Inconsistent set of IDs in similarities file."));
			}

			for(MapOfSimilarities::iterator it=map_of_similarities.begin();it!=map_of_similarities.end();++it)
			{
				it->second[it->first]=1.0;
				if(it->second.size()!=map_of_similarities.size())
				{
					throw std::runtime_error(std::string("Incomplete table of similarities."));
				}
			}

			{
				InputSelector finput_selector(input_ids_file);
				std::istream& finput=finput_selector.stream();
				assert_io_stream(input_ids_file, finput);

				while(finput.good())
				{
					std::string line;
					std::getline(finput, line);
					if(!line.empty())
					{
						std::string id;
						std::istringstream line_input(line);
						line_input >> id;
						if(id.empty())
						{
							throw std::runtime_error(std::string("Invalid ids file line '")+line+"'.");
						}
						ordered_ids.push_back(id);
					}
				}
			}

			for(std::size_t i=0;i<ordered_ids.size();i++)
			{
				if(map_of_similarities.count(ordered_ids[i])==0)
				{
					throw std::runtime_error(std::string("No similarity values for ID '")+ordered_ids[i]+"'.");
				}
			}
		}

		std::set<std::string> set_of_ids;
		for(std::size_t i=0;i<ordered_ids.size();i++)
		{
			set_of_ids.insert(ordered_ids[i]);
		}

		std::vector< std::pair<double, std::string> > ordered_redundancy(ordered_ids.size());
		std::vector< std::pair<double, std::string> > global_redundancy(ordered_ids.size());

		for(std::size_t i=0;i<ordered_ids.size();i++)
		{
			const std::string& id_a=ordered_ids[i];
			double max_similarity=0.0;
			std::string max_similarity_id=id_a;
			for(std::size_t j=0;j<ordered_ids.size();j++)
			{
				if(j==i)
				{
					ordered_redundancy[i].first=max_similarity;
					ordered_redundancy[i].second=max_similarity_id;
				}
				else
				{
					const std::string& id_b=ordered_ids[j];
					const double similarity_value=map_of_similarities[id_a][id_b];
					if(similarity_value>max_similarity)
					{
						max_similarity=similarity_value;
						max_similarity_id=id_b;
					}
				}
			}
			global_redundancy[i].first=max_similarity;
			global_redundancy[i].second=max_similarity_id;
		}

		{
			OutputSelector foutput_selector(output_file);
			std::ostream& foutput=foutput_selector.stream();
			assert_io_stream(output_file, foutput);

			for(std::size_t i=0;i<ordered_ids.size();i++)
			{
				foutput << ordered_ids[i] << " "
						<< ordered_redundancy[i].first << " "
						<< ordered_redundancy[i].second << " "
						<< global_redundancy[i].first << " "
						<< global_redundancy[i].second << "\n";
			}
		}

		Result result;
		return result;
	}
};

}

}

}




#endif /* SCRIPTING_OPERATORS_REDUNDANCY_SCORE_H_ */
