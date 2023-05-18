#ifndef MODESCOMMON_SEQUENCE_UTILITIES_H_
#define MODESCOMMON_SEQUENCE_UTILITIES_H_

#include <fstream>
#include <sstream>

#include "../auxiliaries/residue_letters_coding.h"
#include "../auxiliaries/pairwise_sequence_alignment.h"
#include "../auxiliaries/residue_atoms_reference.h"

#include "chain_residue_atom_descriptor.h"

namespace voronota
{

namespace common
{

class SequenceUtilities
{
public:
	static std::vector<std::string> read_sequences_from_stream(std::istream& input, const bool allow_special_symbols)
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
					std::istringstream sinput(line);
					while(sinput.good())
					{
						std::string token;
						sinput >> token;
						for(std::size_t i=0;i<token.size();i++)
						{
							const char c=token[i];
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
		}
		return result;
	}

	static std::vector<std::string> read_sequences_from_file(const std::string& filename)
	{
		if(!filename.empty())
		{
			std::ifstream finput(filename.c_str(), std::ios::in);
			return read_sequences_from_stream(finput, false);
		}
		else
		{
			return std::vector<std::string>();
		}
	}

	static std::string read_sequence_from_stream(std::istream& input, const bool allow_special_symbols)
	{
		const std::vector<std::string> sequences=read_sequences_from_stream(input, allow_special_symbols);
		std::string result;
		for(std::size_t i=0;i<sequences.size();i++)
		{
			result+=sequences[i];
		}
		return result;
	}

	static std::string read_sequence_from_file(const std::string& filename)
	{
		if(!filename.empty())
		{
			std::ifstream finput(filename.c_str(), std::ios::in);
			return read_sequence_from_stream(finput, false);
		}
		else
		{
			return std::string();
		}
	}

	template<typename T>
	static std::string convert_residue_sequence_container_to_string(const T& residue_sequence_container)
	{
		std::string seq;
		for(typename T::const_iterator it=residue_sequence_container.begin();it!=residue_sequence_container.end();++it)
		{
			seq+=auxiliaries::ResidueLettersCoding::convert_residue_code_big_to_small(it->resName);
		}
		return seq;
	}

	static bool char_counts_as_residue(const char c)
	{
		return ((c>='A' && c<='Z') || (c>='a' && c<='z'));
	}

	static int count_residues_in_sequence(const std::string& seq)
	{
		int n=0;
		for(std::size_t i=0;i<seq.size();i++)
		{
			if(char_counts_as_residue(seq[i]))
			{
				n++;
			}
		}
		return n;
	}

	static std::map<common::ChainResidueAtomDescriptor, int> construct_sequence_mapping(const std::vector<common::ChainResidueAtomDescriptor>& residue_sequence_vector, const std::string& reference_sequence, const bool only_equal_pairs, const bool treat_X_permissively, double* identity, const std::string& ref_seq_alignment_output_filename)
	{
		std::map<common::ChainResidueAtomDescriptor, int> result;
		if(!residue_sequence_vector.empty() && !reference_sequence.empty())
		{
			const std::string seq=convert_residue_sequence_container_to_string(residue_sequence_vector);
			if(residue_sequence_vector.size()==seq.size())
			{
				const std::vector< std::pair<int, int> >& alignment=auxiliaries::PairwiseSequenceAlignment::construct_sequence_alignment(reference_sequence, seq, auxiliaries::PairwiseSequenceAlignment::SimpleScorer(10, -10, -11, -1), false, 0);
				for(std::size_t i=0;i<alignment.size();i++)
				{
					const std::pair<int, int>& p=alignment[i];
					if(p.first>=0 && p.second>=0 && (!only_equal_pairs || (p.first<static_cast<int>(reference_sequence.size()) && p.second<static_cast<int>(seq.size()) && (reference_sequence[p.first]==seq[p.second] || (treat_X_permissively && seq[p.second]=='X')))))
					{
						result[residue_sequence_vector.at(p.second)]=(p.first+1);
					}
				}
				if(identity!=0)
				{
					int max_seq_size=std::max(count_residues_in_sequence(reference_sequence), count_residues_in_sequence(seq));
					if(max_seq_size>0)
					{
						int matches=0;
						for(std::size_t i=0;i<alignment.size();i++)
						{
							const std::pair<int, int>& p=alignment[i];
							if(p.first>=0 && p.first<static_cast<int>(reference_sequence.size()) && p.second>=0 && p.second<static_cast<int>(seq.size()) && reference_sequence[p.first]==seq[p.second] && char_counts_as_residue(reference_sequence[p.first]))
							{
								matches++;
							}
						}
						(*identity)=(static_cast<double>(matches)/static_cast<double>(max_seq_size));
					}
					else
					{
						(*identity)=0.0;
					}
				}
				if(!ref_seq_alignment_output_filename.empty())
				{
					std::ofstream foutput(ref_seq_alignment_output_filename.c_str(), std::ios::out);
					if(foutput.good())
					{
						auxiliaries::PairwiseSequenceAlignment::print_sequence_alignment(reference_sequence, seq, alignment, foutput);
					}
				}
			}
		}
		return result;
	}

	static double calculate_sequence_identity(const std::string& seq_a, const std::string& seq_b)
	{
		const std::size_t max_seq_size=std::max(count_residues_in_sequence(seq_a), count_residues_in_sequence(seq_b));
		if(max_seq_size>0)
		{
			int matches=0;
			const std::vector< std::pair<int, int> >& alignment=auxiliaries::PairwiseSequenceAlignment::construct_sequence_alignment(seq_a, seq_b, auxiliaries::PairwiseSequenceAlignment::SimpleScorer(10, -10, -11, -1), false, 0);
			for(std::size_t i=0;i<alignment.size();i++)
			{
				const std::pair<int, int>& p=alignment[i];
				if(p.first>=0 && p.first<static_cast<int>(seq_a.size()) && p.second>=0 && p.second<static_cast<int>(seq_b.size()) && seq_a[p.first]==seq_b[p.second] && char_counts_as_residue(seq_a[p.first]))
				{
					matches++;
				}
			}
			return (static_cast<double>(matches)/static_cast<double>(max_seq_size));
		}
		return 0.0;
	}
};

}

}

#endif /* MODESCOMMON_SEQUENCE_UTILITIES_H_ */
