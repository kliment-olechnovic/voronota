#ifndef CADSCORELT_SEQUENCES_PROCESSING_H_
#define CADSCORELT_SEQUENCES_PROCESSING_H_

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>

namespace cadscorelt
{

class SequenceInputUtilities
{
public:
	static std::vector<std::string> read_sequences_from_stream(std::istream& input, const bool allow_special_symbols) noexcept
	{
		std::vector<std::string> result;
		while(input.good())
		{
			std::string line;
			std::getline(input, line);
			if(!line.empty())
			{
				if(result.empty() || line[0]=='>')
				{
					result.push_back(std::string());
				}
				if(line[0]!='>')
				{
					for(std::size_t i=0;i<line.size();i++)
					{
						const char c=line[i];
						if(c>='A' && c<='Z')
						{
							result.back().append(1, c);
						}
						else if(c>='a' && c<='z')
						{
							result.back().append(1, c-('a'-'A'));
						}
						else if(allow_special_symbols && c=='-')
						{
							result.back().append(1, c);
						}
					}
				}
			}
		}
		return result;
	}

	static std::vector<std::string> read_sequences_from_file(const std::string& filename, const bool allow_special_symbols) noexcept
	{
		if(!filename.empty())
		{
			std::ifstream finput(filename.c_str(), std::ios::in);
			return read_sequences_from_stream(finput, allow_special_symbols);
		}
		return std::vector<std::string>();
	}

	static std::vector<std::string> read_sequences_from_string(const std::string& data, const bool allow_special_symbols) noexcept
	{
		if(!data.empty())
		{
			std::istringstream input(data);
			return read_sequences_from_stream(input, allow_special_symbols);
		}
		return std::vector<std::string>();
	}

	inline static std::string convert_residue_code_big_to_small(const std::string& big_code) noexcept
	{
		static const std::map<std::string, std::string> m=create_map_of_residue_codes_big_to_small();
		const std::map<std::string, std::string>::const_iterator it=m.find(big_code);
		return ((it==m.end() || it->second.size()!=1) ? std::string("X") : it->second);
	}

private:
	static std::map<std::string, std::string> create_map_of_residue_codes_big_to_small() noexcept
	{
		std::map<std::string, std::string> m;
		m["LEU"]="L";
		m["VAL"]="V";
		m["ILE"]="I";
		m["ALA"]="A";
		m["PHE"]="F";
		m["TRP"]="W";
		m["MET"]="M";
		m["PRO"]="P";
		m["ASP"]="D";
		m["GLU"]="E";
		m["LYS"]="K";
		m["ARG"]="R";
		m["HIS"]="H";
		m["CYS"]="C";
		m["SER"]="S";
		m["THR"]="T";
		m["TYR"]="Y";
		m["ASN"]="N";
		m["GLN"]="Q";
		m["GLY"]="G";
		m["ASX"]="B";
		m["GLX"]="Z";
		m["XLE"]="J";
		m["A"]="A";
		m["C"]="C";
		m["G"]="G";
		m["T"]="T";
		m["U"]="U";
		m["DA"]="A";
		m["DC"]="C";
		m["DG"]="G";
		m["DT"]="T";
		m["XAA"]="X";
		return m;
	}
};

class PairwiseSequenceAlignment
{
public:
	struct SimpleScorer
	{
		int match_score;
		int mismatch_score;
		int gap_start_score;
		int gap_extension_score;

		SimpleScorer(int match_score, int mismatch_score, int gap_start_score, int gap_extension_score) noexcept :
			match_score(match_score),
			mismatch_score(mismatch_score),
			gap_start_score(gap_start_score),
			gap_extension_score(gap_extension_score)
		{
		}

		template<typename T>
		int match(const T& v1, const T& v2) const noexcept
		{
			return (v1==v2 ? match_score : mismatch_score);
		}

		int gap_start() const noexcept
		{
			return gap_start_score;
		}

		int gap_extension() const noexcept
		{
			return gap_extension_score;
		}
	};

	template<typename T, typename Scorer>
	static std::vector< std::pair<int, int> > construct_sequence_alignment(const T& seq1, const T& seq2, const Scorer& scorer, const bool local, int* result_score) noexcept
	{
		std::vector< std::pair<int, int> > alignment;
		if(!seq1.empty() && !seq2.empty())
		{
			std::vector< std::vector<int> > scores_matrix(seq1.size()+1, std::vector<int>(seq2.size()+1, 0));
			std::vector< std::vector<int> > directions_matrix(seq1.size()+1, std::vector<int>(seq2.size()+1, 0));
			for(std::size_t i=1;i<=seq1.size();i++)
			{
				scores_matrix[i][0]=((i==1) ? scorer.gap_start() : (scores_matrix[i-1][0]+scorer.gap_extension()));
				directions_matrix[i][0]=1;
			}
			for(std::size_t j=1;j<=seq2.size();j++)
			{
				scores_matrix[0][j]=((j==1) ? scorer.gap_start() : (scores_matrix[0][j-1]+scorer.gap_extension()));
				directions_matrix[0][j]=2;
			}
			std::pair<std::size_t, std::size_t> result_score_pos(0, 0);
			for(std::size_t i=1;i<=seq1.size();i++)
			{
				for(std::size_t j=1;j<=seq2.size();j++)
				{
					const typename T::value_type& v1=seq1[i-1];
					const typename T::value_type& v2=seq2[j-1];
					const int match_score=scores_matrix[i-1][j-1]+scorer.match(v1, v2)+(((i==1 && j==1) || (i==seq1.size() && j==seq2.size())) ? -1 : 0);
					const int deletion_score=scores_matrix[i-1][j]+(directions_matrix[i-1][j]!=1 ? scorer.gap_start() : scorer.gap_extension());
					const int insertion_score=scores_matrix[i][j-1]+(directions_matrix[i][j-1]!=2 ? scorer.gap_start() : scorer.gap_extension());
					const int max_score=std::max(match_score, std::max(deletion_score, insertion_score));
					directions_matrix[i][j]=(max_score==insertion_score ? 2 : (max_score==deletion_score ? 1 : 0));
					scores_matrix[i][j]=(local ? std::max(0, max_score) : max_score);
					if(local && scores_matrix[i][j]>scores_matrix[result_score_pos.first][result_score_pos.second])
					{
						result_score_pos=std::make_pair(i, j);
					}
				}
			}
			if(!local)
			{
				result_score_pos.first=seq1.size();
				result_score_pos.second=seq2.size();
			}
			if(result_score!=0)
			{
				*result_score=scores_matrix[result_score_pos.first][result_score_pos.second];
			}

			int i=static_cast<int>(seq1.size());
			int j=static_cast<int>(seq2.size());
			while(i>static_cast<int>(result_score_pos.first))
			{
				i--;
				alignment.push_back(std::make_pair(i, -1));
			}
			while(j>static_cast<int>(result_score_pos.second))
			{
				j--;
				alignment.push_back(std::make_pair(-1, j));
			}
			while(i>0 && j>0 && (!local || scores_matrix[i][j]>0))
			{
				const int dir=directions_matrix[i][j];
				if(dir==0)
				{
					i--;
					j--;
					alignment.push_back(std::make_pair(i, j));
				}
				else if(dir==1)
				{
					i--;
					alignment.push_back(std::make_pair(i, -1));
				}
				else
				{
					j--;
					alignment.push_back(std::make_pair(-1, j));
				}
			}
			while(i>0)
			{
				i--;
				alignment.push_back(std::make_pair(i, -1));
			}
			while(j>0)
			{
				j--;
				alignment.push_back(std::make_pair(-1, j));
			}
			std::reverse(alignment.begin(), alignment.end());
		}
		return alignment;
	}

	static std::string print_sequence_alignment(const std::string& seq1, const std::string& seq2, const std::vector< std::pair<int, int> >& alignment) noexcept
	{
		std::string output;

		for(std::size_t i=0;i<alignment.size();i++)
		{
			if(alignment[i].first>=static_cast<int>(seq1.size()) || alignment[i].second>=static_cast<int>(seq2.size()))
			{
				return std::string();
			}
		}

		for(std::size_t i=0;i<alignment.size();i++)
		{
			const int j=alignment[i].first;
			output.append(1, (j>=0 ? seq1[j] : '-'));
		}
		output+="\n";

		for(std::size_t i=0;i<alignment.size();i++)
		{
			const int j=alignment[i].second;
			output.append(1, (j>=0 ? seq2[j] : '-'));
		}
		output+="\n";

		return output;
	}
};

class PairwiseSequenceMapping
{
public:
	struct Result
	{
		double identity;
		std::vector< std::pair<int, int> > mapping;
		std::string printed_alignment;

		Result() noexcept : identity(0.0)
		{
		}

		bool valid() const noexcept
		{
			return (!mapping.empty());
		}

		void clear() noexcept
		{
			identity=0.0;
			mapping.clear();
			printed_alignment.clear();
		}

		void swap(Result& v) noexcept
		{
			std::swap(identity, v.identity);
			mapping.swap(v.mapping);
			printed_alignment.swap(v.printed_alignment);
		}
	};

	static double calculate_sequence_identity(const std::string& seq_a, const std::string& seq_b)
	{
		const std::size_t max_seq_size=std::max(count_residues_in_sequence(seq_a), count_residues_in_sequence(seq_b));
		if(max_seq_size>0)
		{
			int matches=0;
			const std::vector< std::pair<int, int> >& alignment=PairwiseSequenceAlignment::construct_sequence_alignment(seq_a, seq_b, PairwiseSequenceAlignment::SimpleScorer(10, -10, -11, -1), false, 0);
			for(std::size_t i=0;i<alignment.size();i++)
			{
				const std::pair<int, int>& p=alignment[i];
				if(p.first>=0 && p.first<static_cast<int>(seq_a.size()) && p.second>=0 && p.second<static_cast<int>(seq_b.size()) && seq_a[p.first]==seq_b[p.second] && letter_is_residue(seq_a[p.first]))
				{
					matches++;
				}
			}
			return (static_cast<double>(matches)/static_cast<double>(max_seq_size));
		}
		return 0.0;
	}

	static bool construct_mapping(const std::string& seq_a, const std::string& seq_b, const bool record_printed_alignment, Result& result) noexcept
	{
		result.clear();
		if(seq_a.empty() || seq_b.empty())
		{
			return false;
		}
		const int max_seq_size=std::max(count_residues_in_sequence(seq_a), count_residues_in_sequence(seq_b));
		if(max_seq_size<1)
		{
			return false;
		}
		const std::vector< std::pair<int, int> > alignment=PairwiseSequenceAlignment::construct_sequence_alignment(seq_a, seq_b, PairwiseSequenceAlignment::SimpleScorer(10, -10, -11, -1), false, 0);
		if(alignment.empty())
		{
			return false;
		}
		int matches=0;
		result.mapping.reserve(alignment.size());
		for(std::size_t i=0;i<alignment.size();i++)
		{
			const std::pair<int, int>& p=alignment[i];
			if(p.first>=0 && p.first<static_cast<int>(seq_a.size()) && p.second>=0 && p.second<static_cast<int>(seq_b.size()))
			{
				result.mapping.push_back(std::pair<int, int>(p.second, p.first));
				if(seq_a[p.first]==seq_b[p.second] && letter_is_residue(seq_a[p.first]))
				{
					matches++;
				}
			}
		}
		result.identity=(static_cast<double>(matches)/static_cast<double>(max_seq_size));
		if(record_printed_alignment)
		{
			result.printed_alignment=PairwiseSequenceAlignment::print_sequence_alignment(seq_a, seq_b, alignment);
		}
		return result.valid();
	}

private:
	static bool letter_is_residue(const char c) noexcept
	{
		return ((c>='A' && c<='Z') || (c>='a' && c<='z'));
	}

	static int count_residues_in_sequence(const std::string& seq) noexcept
	{
		int n=0;
		for(std::size_t i=0;i<seq.size();i++)
		{
			if(letter_is_residue(seq[i]))
			{
				n++;
			}
		}
		return n;
	}
};

}

#endif /* CADSCORELT_SEQUENCES_PROCESSING_H_ */
