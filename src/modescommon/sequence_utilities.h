#ifndef SEQUENCE_UTILITIES_H_
#define SEQUENCE_UTILITIES_H_

#include <fstream>
#include <sstream>

#include "../auxiliaries/chain_residue_atom_descriptor.h"
#include "../auxiliaries/residue_letters_coding.h"
#include "../auxiliaries/pairwise_sequence_alignment.h"
#include "../auxiliaries/residue_atoms_reference.h"

namespace
{

class SequenceUtilities
{
public:
	static std::string read_sequence_from_stream(std::istream& input)
	{
		std::string result;
		while(input.good())
		{
			std::string line;
			std::getline(input, line);
			if(!line.empty() && line[0]!='>')
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
							result.append(1, c);
						}
						else if(c>='a' && c<='z')
						{
							result.append(1, c-('a'-'A'));
						}
					}
				}
			}
		}
		return result;
	}

	static std::string read_sequence_from_file(const std::string& filename)
	{
		if(!filename.empty())
		{
			std::ifstream finput(filename.c_str(), std::ios::in);
			return read_sequence_from_stream(finput);
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

	static std::map<auxiliaries::ChainResidueAtomDescriptor, int> construct_sequence_mapping(const std::vector<auxiliaries::ChainResidueAtomDescriptor>& residue_sequence_vector, const std::string& reference_sequence, const std::string& ref_seq_alignment_output_filename)
	{
		std::map<auxiliaries::ChainResidueAtomDescriptor, int> result;
		if(!residue_sequence_vector.empty() && !reference_sequence.empty())
		{
			const std::string seq=convert_residue_sequence_container_to_string(residue_sequence_vector);
			if(residue_sequence_vector.size()==seq.size())
			{
				const std::vector< std::pair<int, int> >& alignment=auxiliaries::PairwiseSequenceAlignment::construct_sequence_alignment(reference_sequence, seq, auxiliaries::PairwiseSequenceAlignment::SimpleScorer(10, -10, -11, -1), false, 0);
				for(std::size_t i=0;i<alignment.size();i++)
				{
					const std::pair<int, int>& p=alignment[i];
					if(p.first>=0 && p.second>=0)
					{
						result[residue_sequence_vector.at(p.second)]=(p.first+1);
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
		const std::size_t max_seq_size=std::max(seq_a.size(), seq_b.size());
		if(max_seq_size>0)
		{
			int matches=0.0;
			const std::vector< std::pair<int, int> >& alignment=auxiliaries::PairwiseSequenceAlignment::construct_sequence_alignment(seq_a, seq_b, auxiliaries::PairwiseSequenceAlignment::SimpleScorer(10, -10, -11, -1), false, 0);
			for(std::size_t i=0;i<alignment.size();i++)
			{
				const std::pair<int, int>& p=alignment[i];
				if(p.first>=0 && p.first<static_cast<int>(seq_a.size()) && p.second>=0 && p.second<static_cast<int>(seq_b.size()) && seq_a[p.first]==seq_b[p.second])
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

#endif /* SEQUENCE_UTILITIES_H_ */
