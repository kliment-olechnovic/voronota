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
	static std::string read_sequence_from_file(const std::string& filename)
	{
		std::string result;
		if(!filename.empty())
		{
			std::ifstream finput(filename.c_str(), std::ios::in);
			while(finput.good())
			{
				std::string line;
				std::getline(finput, line);
				if(!line.empty() && line[0]!='>')
				{
					std::istringstream sinput(line);
					while(sinput.good())
					{
						std::string token;
						sinput >> token;
						if(!token.empty())
						{
							result+=token;
						}
					}
				}
			}
		}
		return result;
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

	static std::map<auxiliaries::ChainResidueAtomDescriptor, double> construct_sequence_mapping(const std::vector<auxiliaries::ChainResidueAtomDescriptor>& residue_sequence_vector, const std::string& reference_sequence, const std::string& ref_seq_alignment_output_filename)
	{
		typedef auxiliaries::ChainResidueAtomDescriptor CRAD;
		std::map<CRAD, double> result;
		if(!residue_sequence_vector.empty() && !reference_sequence.empty())
		{
			const std::string seq=convert_residue_sequence_container_to_string(residue_sequence_vector);
			if(residue_sequence_vector.size()==seq.size())
			{
				const std::vector< std::pair<int, int> >& alignment=auxiliaries::PairwiseSequenceAlignment::construct_sequence_alignment(reference_sequence, seq, auxiliaries::PairwiseSequenceAlignment::SimpleScorer(10, -10, -11, -1));
				for(std::size_t i=0;i<alignment.size();i++)
				{
					const std::pair<int, int>& p=alignment[i];
					if(p.first>=0 && p.second>=0)
					{
						result[residue_sequence_vector.at(p.second)]=static_cast<double>(p.first+1);
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

	static std::size_t count_atoms_from_sequence(const std::string& sequence)
	{
		std::size_t n=0;
		for(std::size_t i=0;i<sequence.size();i++)
		{
			n+=auxiliaries::ResidueAtomsReference::get_residue_atoms_count(auxiliaries::ResidueLettersCoding::convert_residue_code_small_to_big(std::string(1, sequence[i])));
		}
		return n;
	}
};

}

#endif /* SEQUENCE_UTILITIES_H_ */
