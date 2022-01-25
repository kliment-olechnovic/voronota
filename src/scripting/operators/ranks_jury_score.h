#ifndef SCRIPTING_OPERATORS_RANKS_JURY_SCORE_H_
#define SCRIPTING_OPERATORS_RANKS_JURY_SCORE_H_

#include <algorithm>

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class RanksJuryScore : public OperatorBase<RanksJuryScore>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		void store(HeterogeneousStorage&) const
		{
		}
	};

	std::string input_similarities_file;
	std::string input_ranks_file;
	std::string output_file;
	std::vector<std::size_t> top_slices;
	double similarity_threshold;
	bool generate_slices;
	bool use_max_value;
	std::size_t several_max_values;
	bool use_dominations;
	bool output_uniqueness;

	RanksJuryScore() : similarity_threshold(1.0), generate_slices(false), use_max_value(false), several_max_values(1), use_dominations(false), output_uniqueness(false)
	{
	}

	void initialize(CommandInput& input)
	{
		input_similarities_file=input.get_value<std::string>("input-similarities-file");
		input_ranks_file=input.get_value<std::string>("input-ranks-file");
		output_file=input.get_value<std::string>("output-file");
		top_slices=input.get_value_vector<std::size_t>("top-slices");
		similarity_threshold=input.get_value_or_default<double>("similarity-threshold", 1.0);
		generate_slices=input.get_flag("generate-slices");
		use_max_value=input.get_flag("use-max-value");
		several_max_values=input.get_value_or_default<std::size_t>("several-max-values", 1);
		use_dominations=input.get_flag("use-dominations");
		output_uniqueness=input.get_flag("output-uniqueness");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("input-similarities-file", CDOD::DATATYPE_STRING, "path to input similarities file"));
		doc.set_option_decription(CDOD("input-ranks-file", CDOD::DATATYPE_STRING, "path to ranks similarities file"));
		doc.set_option_decription(CDOD("output-file", CDOD::DATATYPE_STRING, "path to output file"));
		doc.set_option_decription(CDOD("top-slices", CDOD::DATATYPE_INT_ARRAY, "list of slice sizes"));
		doc.set_option_decription(CDOD("similarity-threshold", CDOD::DATATYPE_FLOAT, "similarity threshold for clustering", 1.0));
		doc.set_option_decription(CDOD("generate-slices", CDOD::DATATYPE_BOOL, "flag to generate top slices from interval"));
		doc.set_option_decription(CDOD("use-max-value", CDOD::DATATYPE_BOOL, "flag to use the best value from all the slices"));
		doc.set_option_decription(CDOD("several-max-values", CDOD::DATATYPE_INT, "number of top max values to average", 1));
		doc.set_option_decription(CDOD("use-dominations", CDOD::DATATYPE_BOOL, "flag to use domination counts from all the slices"));
		doc.set_option_decription(CDOD("output-uniqueness", CDOD::DATATYPE_BOOL, "flag to output similarities to higher-ranked IDs"));
	}

	Result run(void*) const
	{
		assert_file_name_input(input_similarities_file, false);
		assert_file_name_input(input_ranks_file, false);
		assert_file_name_input(output_file, false);

		if(top_slices.empty())
		{
			throw std::runtime_error(std::string("No top slices specified."));
		}

		for(std::size_t i=0;i<top_slices.size();i++)
		{
			if(top_slices[i]<1)
			{
				throw std::runtime_error(std::string("Invalid top slices specified."));
			}
		}

		std::vector<std::size_t> top_slices_to_use;

		if(generate_slices)
		{
			if(top_slices.size()<2 || top_slices[0]>=top_slices[1])
			{
				throw std::runtime_error(std::string("Specified first two top slices do not define an interval usable for generating numbers."));
			}
			const std::size_t n_slices=(top_slices[1]-top_slices[0])+1;
			if(n_slices>500)
			{
				throw std::runtime_error(std::string("Specified first two top slices define an interval longer than 500."));
			}
			top_slices_to_use.reserve(n_slices);
			for(std::size_t i=top_slices[0];i<=top_slices[1];i++)
			{
				top_slices_to_use.push_back(i);
			}
			for(std::size_t i=2;i<top_slices.size();i++)
			{
				top_slices_to_use.push_back(top_slices[i]);
			}
		}
		else
		{
			top_slices_to_use=top_slices;
		}

		typedef std::map<std::string, std::map<std::string, double> > MapOfSimilarities;
		MapOfSimilarities map_of_similarities;

		typedef std::map<std::string, std::vector<int> > MapOfRanks;
		MapOfRanks map_of_ranks;

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
				InputSelector finput_selector(input_ranks_file);
				std::istream& finput=finput_selector.stream();
				assert_io_stream(input_ranks_file, finput);

				std::size_t num_of_rankings=0;

				while(finput.good())
				{
					std::string line;
					std::getline(finput, line);
					if(!line.empty())
					{
						if(num_of_rankings==0)
						{
							std::size_t num_of_tokens=0;
							std::istringstream line_input(line);
							while(line_input.good())
							{
								std::string token;
								line_input >> token;
								if(!token.empty())
								{
									num_of_tokens++;
								}
							}
							if(num_of_tokens<2)
							{
								throw std::runtime_error(std::string("Invalid table of ranks, must contain at least one ranking column."));
							}
							num_of_rankings=num_of_tokens-1;
						}

						std::string id;
						std::vector<int> ranks(num_of_rankings, 0);
						std::istringstream line_input(line);
						line_input >> id;
						if(id.empty())
						{
							throw std::runtime_error(std::string("Invalid ranks file line '")+line+"'.");
						}
						for(std::size_t i=0;i<ranks.size() && line_input.good();i++)
						{
							line_input >> ranks[i];
							if(ranks[i]<1)
							{
								throw std::runtime_error(std::string("Invalid ranks file line '")+line+"'.");
							}
						}
						for(std::size_t i=0;i<ranks.size() && line_input.good();i++)
						{
							if(ranks[i]<1)
							{
								throw std::runtime_error(std::string("Incomplete ranks file line '")+line+"'.");
							}
						}

						if(set_of_all_ids.count(id)>0)
						{
							map_of_ranks[id]=ranks;
						}
					}
				}

				for(std::set<std::string>::const_iterator it=set_of_all_ids.begin();it!=set_of_all_ids.end();++it)
				{
					if(map_of_ranks.count(*it)==0)
					{
						map_of_ranks[*it]=std::vector<int>(num_of_rankings, std::numeric_limits<int>::max());
					}
				}
			}

			if(similarity_threshold<1.0)
			{
				typedef std::map< std::pair<int, std::string>, std::set<std::string> > MapOfCandidateCenters;
				MapOfCandidateCenters map_of_candidate_centers;
				for(MapOfSimilarities::const_iterator it=map_of_similarities.begin();it!=map_of_similarities.end();++it)
				{
					const std::map<std::string, double>& map_of_values=it->second;
					std::set<std::string> neighbors;
					for(std::map<std::string, double>::const_iterator jt=map_of_values.begin();jt!=map_of_values.end();++jt)
					{
						if(jt->second>similarity_threshold && (jt->first!=it->first))
						{
							neighbors.insert(jt->first);
						}
					}
					if(!neighbors.empty())
					{
						map_of_candidate_centers[std::pair<int, std::string>(0-static_cast<int>(neighbors.size()), it->first)]=neighbors;
					}
				}
				if(!map_of_candidate_centers.empty())
				{
					std::set<std::string> set_of_ids_to_exclude;
					for(MapOfCandidateCenters::const_iterator candidate_centers_it=map_of_candidate_centers.begin();candidate_centers_it!=map_of_candidate_centers.end();++candidate_centers_it)
					{
						const std::string& id=candidate_centers_it->first.second;
						if(set_of_ids_to_exclude.count(id)==0)
						{
							const std::set<std::string>& neighbors=candidate_centers_it->second;
							set_of_ids_to_exclude.insert(neighbors.begin(), neighbors.end());
						}
					}
					if(!set_of_ids_to_exclude.empty())
					{
						for(std::set<std::string>::const_iterator jt=set_of_ids_to_exclude.begin();jt!=set_of_ids_to_exclude.end();++jt)
						{
							map_of_similarities.erase(*jt);
							map_of_ranks.erase(*jt);
						}
						for(MapOfSimilarities::iterator it=map_of_similarities.begin();it!=map_of_similarities.end();++it)
						{
							for(std::set<std::string>::const_iterator jt=set_of_ids_to_exclude.begin();jt!=set_of_ids_to_exclude.end();++jt)
							{
								it->second.erase(*jt);
							}
						}
					}
				}
			}
		}

		const std::size_t N=map_of_similarities.size();
		const std::size_t M=map_of_ranks.begin()->second.size();
		const std::size_t L=top_slices_to_use.size();

		std::vector<std::string> indices_to_ids(N);
		std::vector< std::vector<double> > matrix_of_similarities(N, std::vector<double>(N, 0.0));
		{
			std::size_t i=0;
			for(MapOfSimilarities::const_iterator it=map_of_similarities.begin();it!=map_of_similarities.end();++it)
			{
				indices_to_ids[i]=it->first;
				const std::map<std::string, double>& map_of_values=it->second;
				std::size_t j=0;
				for(std::map<std::string, double>::const_iterator jt=map_of_values.begin();jt!=map_of_values.end();++jt)
				{
					matrix_of_similarities[i][j]=jt->second;
					j++;
				}
				i++;
			}
		}

		std::vector< std::vector<std::size_t> > orderings(M, std::vector<std::size_t>(N, 0));
		{
			for(std::size_t m=0;m<M;m++)
			{
				std::vector< std::pair<int, std::size_t> > rank_indices(N);
				std::size_t j=0;
				for(MapOfRanks::const_iterator it=map_of_ranks.begin();it!=map_of_ranks.end();++it)
				{
					rank_indices[j].first=it->second[m];
					rank_indices[j].second=j;
					j++;
				}
				std::sort(rank_indices.begin(), rank_indices.end());
				for(std::size_t j=0;j<N;j++)
				{
					orderings[m][j]=rank_indices[j].second;
				}
			}
		}

		std::vector< std::pair<std::vector<double>, std::size_t> > jury_scores(N, std::make_pair(std::vector<double>(L, 0.0), 0));
		for(std::size_t i=0;i<N;i++)
		{
			jury_scores[i].second=i;
		}

		for(std::size_t l=0;l<L;l++)
		{
			const std::size_t top_slice=std::min(top_slices_to_use[l], N);
			std::vector<std::size_t> indices;
			indices.reserve(M*top_slice);
			for(std::size_t m=0;m<M;m++)
			{
				for(std::size_t i=0;i<top_slice;i++)
				{
					indices.push_back(orderings[m][i]);
				}
			}
			std::map<std::size_t, double> indices_jury_scores;
			for(std::size_t i=0;i<indices.size();i++)
			{
				const std::size_t index1=indices[i];
				if(indices_jury_scores.count(index1)==0)
				{
					double sum=0.0;
					int count=0;
					for(std::size_t j=0;j<indices.size();j++)
					{
						if(j!=i)
						{
							const std::size_t index2=indices[j];
							sum+=matrix_of_similarities[index1][index2];
							count++;
						}
					}
					indices_jury_scores[index1]=(count>0 ? (sum/static_cast<double>(count)) : 1.0);
				}
			}
			for(std::map<std::size_t, double>::const_iterator it=indices_jury_scores.begin();it!=indices_jury_scores.end();++it)
			{
				jury_scores[it->first].first[l]=(0.0-(it->second));
			}
		}

		std::vector<int> dominations(N, 0);
		if(use_dominations)
		{
			for(std::size_t l=0;l<L;l++)
			{
				std::vector< std::pair<double, std::size_t> > slice_ordering(N);
				for(std::size_t i=0;i<N;i++)
				{
					slice_ordering[i].first=jury_scores[i].first[l];
					slice_ordering[i].second=i;
				}
				std::sort(slice_ordering.begin(), slice_ordering.end());
				dominations[slice_ordering.front().second]++;
			}
		}

		if(use_max_value)
		{
			for(std::size_t i=0;i<N;i++)
			{
				std::vector<double>& values=jury_scores[i].first;
				if(several_max_values<2)
				{
					const double negative_max_value=*std::min_element(values.begin(), values.end());
					values.insert(values.begin(), negative_max_value);
				}
				else
				{
					std::vector<double> sorted_values=values;
					std::sort(sorted_values.begin(), sorted_values.end());
					double sum=0.0;
					double sum_size=0.0;
					for(std::size_t j=0;j<std::min(several_max_values, sorted_values.size());j++)
					{
						if(sorted_values[j]<0.0)
						{
							sum+=sorted_values[j];
							sum_size+=1.0;
						}
					}
					values.insert(values.begin(), (sum_size>0 ? (sum/sum_size) : 0.0));
				}
			}
		}

		if(use_dominations)
		{
			for(std::size_t i=0;i<N;i++)
			{
				std::vector<double>& values=jury_scores[i].first;
				values.insert(values.begin(), (0.0-static_cast<double>(dominations[jury_scores[i].second])));
			}
		}

		std::sort(jury_scores.begin(), jury_scores.end());

		std::vector< std::pair<double, std::size_t> > uniqueness(N, std::pair<double, std::size_t>(0.0, 0));
		if(output_uniqueness)
		{
			for(std::size_t i=0;i<N;i++)
			{
				const std::size_t index_a=jury_scores[i].second;
				double max_similarity=0.0;
				std::size_t max_similarity_index=index_a;
				for(std::size_t j=0;j<i;j++)
				{
					const std::size_t index_b=jury_scores[j].second;
					const double similarity_value=matrix_of_similarities[index_a][index_b];
					if(similarity_value>max_similarity)
					{
						max_similarity=similarity_value;
						max_similarity_index=index_b;
					}
				}
				uniqueness[index_a].first=max_similarity;
				uniqueness[index_a].second=max_similarity_index;
			}
		}

		{
			OutputSelector foutput_selector(output_file);
			std::ostream& foutput=foutput_selector.stream();
			assert_io_stream(output_file, foutput);

			for(std::size_t i=0;i<N;i++)
			{
				const std::size_t index=jury_scores[i].second;
				foutput << indices_to_ids[index];
				for(std::size_t l=0;l<jury_scores[i].first.size();l++)
				{
					foutput << " " << (0.0-jury_scores[i].first[l]);
				}
				if(output_uniqueness)
				{
					foutput << " " << uniqueness[index].first;
					foutput << " " << indices_to_ids[uniqueness[index].second];
				}
				foutput << "\n";
			}
		}

		Result result;
		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_RANKS_JURY_SCORE_H_ */
