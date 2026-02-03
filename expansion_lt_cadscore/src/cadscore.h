#ifndef CADSCORE_H_
#define CADSCORE_H_

#include <string>
#include <map>
#include <set>
#include <limits>

#include "../../expansion_lt/src/voronotalt/parallelization_configuration.h"
#include "../../expansion_lt/src/voronotalt/voronotalt.h"
#include "../../expansion_lt/src/voronotalt_cli/voronotalt_cli.h"

#include "sequtil.h"

namespace cadscore
{

inline const std::string& version() noexcept
{
	static const std::string version_str="0.8";
	return version_str;
}


struct IDChain
{
	std::string chain_name;

	IDChain() noexcept
	{
	}

	explicit IDChain(const voronotalt::SphereLabeling::SphereLabel& sl) noexcept : chain_name(sl.chain_id)
	{
	}

	bool operator==(const IDChain& v) const noexcept
	{
		return (chain_name==v.chain_name);
	}

	bool operator!=(const IDChain& v) const noexcept
	{
		return (chain_name!=v.chain_name);
	}

	bool operator<(const IDChain& v) const noexcept
	{
		return chain_name<v.chain_name;
	}

	inline IDChain with_renamed_chains(const std::map<std::string, std::string>& renaming_map) const noexcept
	{
		std::map<std::string, std::string>::const_iterator it=renaming_map.find(chain_name);
		if(it!=renaming_map.end())
		{
			IDChain result;
			result.chain_name=it->second;
			return result;
		}
		return (*this);
	}

	inline bool match_chain_name(const std::string& query_chain_name) const noexcept
	{
		return (chain_name==query_chain_name);
	}
};

struct IDResidue
{
	IDChain id_chain;
	int residue_seq_number;
	std::string residue_icode;
	std::string residue_name;

	IDResidue() noexcept : residue_seq_number(0)
	{
	}

	explicit IDResidue(const voronotalt::SphereLabeling::SphereLabel& sl) noexcept : id_chain(sl), residue_seq_number(sl.expanded_residue_id.rnum), residue_icode(sl.expanded_residue_id.icode), residue_name(consider_residue_names() ? sl.expanded_residue_id.rname : std::string())
	{
	}

	bool operator==(const IDResidue& v) const noexcept
	{
		return (residue_seq_number==v.residue_seq_number && id_chain==v.id_chain && residue_icode==v.residue_icode && residue_name==v.residue_name);
	}

	bool operator<(const IDResidue& v) const noexcept
	{
		if(id_chain<v.id_chain)
		{
			return true;
		}
		else if(id_chain==v.id_chain)
		{
			if(residue_seq_number<v.residue_seq_number)
			{
				return true;
			}
			else if(residue_seq_number==v.residue_seq_number)
			{
				if(residue_icode<v.residue_icode)
				{
					return true;
				}
				else if(residue_icode==v.residue_icode)
				{
					return (residue_name<v.residue_name);
				}
			}
		}
		return false;
	}

	inline IDResidue with_renamed_chains(const std::map<std::string, std::string>& renaming_map) const noexcept
	{
		IDResidue v=(*this);
		v.id_chain=id_chain.with_renamed_chains(renaming_map);
		return v;
	}

	inline bool match_chain_name(const std::string& query_chain_name) const noexcept
	{
		return id_chain.match_chain_name(query_chain_name);
	}

	static bool& consider_residue_names() noexcept
	{
		static bool status=false;
		return status;
	}
};

struct IDAtom
{
	IDResidue id_residue;
	std::string atom_name;

	IDAtom() noexcept
	{
	}

	explicit IDAtom(const voronotalt::SphereLabeling::SphereLabel& sl) noexcept : id_residue(sl), atom_name(sl.atom_name)
	{
	}

	bool operator==(const IDAtom& v) const noexcept
	{
		return (atom_name==v.atom_name && id_residue==v.id_residue);
	}

	bool operator<(const IDAtom& v) const noexcept
	{
		if(id_residue<v.id_residue)
		{
			return true;
		}
		else if(id_residue==v.id_residue)
		{
			return (atom_name<v.atom_name);
		}
		return false;
	}

	inline IDAtom with_renamed_chains(const std::map<std::string, std::string>& renaming_map) const noexcept
	{
		IDAtom v=(*this);
		v.id_residue=id_residue.with_renamed_chains(renaming_map);
		return v;
	}

	inline bool match_chain_name(const std::string& query_chain_name) const noexcept
	{
		return id_residue.match_chain_name(query_chain_name);
	}
};

struct IDChainChain
{
	IDChain id_a;
	IDChain id_b;

	IDChainChain() noexcept
	{
	}

	IDChainChain(const IDChain& a, const IDChain& b) noexcept : id_a(a<b ? a : b), id_b(a<b ? b : a)
	{
	}

	bool operator==(const IDChainChain& v) const noexcept
	{
		return (id_a==v.id_a && id_b==v.id_b);
	}

	bool operator<(const IDChainChain& v) const noexcept
	{
		if(id_a<v.id_a)
		{
			return true;
		}
		else if(id_a==v.id_a)
		{
			return (id_b<v.id_b);
		}
		return false;
	}

	inline IDChainChain with_renamed_chains(const std::map<std::string, std::string>& renaming_map) const noexcept
	{
		return IDChainChain(id_a.with_renamed_chains(renaming_map), id_b.with_renamed_chains(renaming_map));
	}

	inline bool match_chain_name(const std::string& query_chain_name) const noexcept
	{
		return (id_a.match_chain_name(query_chain_name) || id_b.match_chain_name(query_chain_name));
	}
};

struct IDResidueResidue
{
	IDResidue id_a;
	IDResidue id_b;

	IDResidueResidue() noexcept
	{
	}

	IDResidueResidue(const IDResidue& a, const IDResidue& b) noexcept : id_a(a<b ? a : b), id_b(a<b ? b : a)
	{
	}

	bool operator==(const IDResidueResidue& v) const noexcept
	{
		return (id_a==v.id_a && id_b==v.id_b);
	}

	bool operator<(const IDResidueResidue& v) const noexcept
	{
		if(id_a<v.id_a)
		{
			return true;
		}
		else if(id_a==v.id_a)
		{
			return (id_b<v.id_b);
		}
		return false;
	}

	inline IDResidueResidue with_renamed_chains(const std::map<std::string, std::string>& renaming_map) const noexcept
	{
		return IDResidueResidue(id_a.with_renamed_chains(renaming_map), id_b.with_renamed_chains(renaming_map));
	}

	inline bool match_chain_name(const std::string& query_chain_name) const noexcept
	{
		return (id_a.match_chain_name(query_chain_name) || id_b.match_chain_name(query_chain_name));
	}
};

struct IDAtomAtom
{
	IDAtom id_a;
	IDAtom id_b;

	IDAtomAtom() noexcept
	{
	}

	IDAtomAtom(const IDAtom& a, const IDAtom& b) noexcept : id_a(a<b ? a : b), id_b(a<b ? b : a)
	{
	}

	bool operator==(const IDAtomAtom& v) const noexcept
	{
		return (id_a==v.id_a && id_b==v.id_b);
	}

	bool operator<(const IDAtomAtom& v) const noexcept
	{
		if(id_a<v.id_a)
		{
			return true;
		}
		else if(id_a==v.id_a)
		{
			return (id_b<v.id_b);
		}
		return false;
	}

	inline IDAtomAtom with_renamed_chains(const std::map<std::string, std::string>& renaming_map) const noexcept
	{
		return IDAtomAtom(id_a.with_renamed_chains(renaming_map), id_b.with_renamed_chains(renaming_map));
	}

	inline bool match_chain_name(const std::string& query_chain_name) const noexcept
	{
		return (id_a.match_chain_name(query_chain_name) || id_b.match_chain_name(query_chain_name));
	}
};

struct AtomBall
{
	IDAtom id_atom;
	std::string residue_name;
	double x;
	double y;
	double z;
	double r;

	AtomBall() noexcept : x(0.0), y(0.0), z(0.0), r(0.0)
	{
	}

	AtomBall with_renamed_chains(const std::map<std::string, std::string>& renaming_map) const noexcept
	{
		AtomBall v=(*this);
		v.id_atom=id_atom.with_renamed_chains(renaming_map);
		return v;
	}

	void write_into_atom_record(voronotalt::MolecularFileReading::AtomRecord& ar) const noexcept
	{
		ar.chainID=id_atom.id_residue.id_chain.chain_name;
		ar.resSeq=id_atom.id_residue.residue_seq_number;
		ar.iCode=id_atom.id_residue.residue_icode;
		ar.resName=residue_name;
		ar.name=id_atom.atom_name;
		ar.x=x;
		ar.y=y;
		ar.z=z;
	}
};

class ChainsSequencesMapping
{
public:
	struct ChainSummary
	{
		std::string old_name;
		std::string current_name;
		int reference_sequence_id;
		double reference_sequence_identity;
		std::string printed_alignment;
	};

	struct Result
	{
		std::map<std::string, ChainSummary> chain_summaries;
		std::map<int, int> frequencies_of_reference_sequence_ids;
		std::string chain_renaming_label;

		bool empty() const noexcept
		{
			return chain_summaries.empty();
		}

		void clear() noexcept
		{
			chain_summaries.clear();
			frequencies_of_reference_sequence_ids.clear();
			chain_renaming_label.clear();
		}

		int get_reference_sequence_id_by_chain_name(const std::string& chain_name) const
		{
			if(!chain_summaries.empty())
			{
				std::map<std::string, ChainSummary>::const_iterator it=chain_summaries.find(chain_name);
				if(it!=chain_summaries.end())
				{
					return it->second.reference_sequence_id;
				}
			}
			return -1;
		}
	};

	static bool remap_chain_names_and_residue_numbers_by_reference_sequences(const std::vector<std::string>& reference_sequences, const std::vector<int>& input_stoichiometry, const bool record_printed_alignment, std::vector<AtomBall>& atom_balls, Result& result) noexcept
	{
		result.clear();

		std::vector<ChainDescriptor> cds=init_chain_descriptors(atom_balls);

		if(!assign_reference_sequences_to_chain_descriptors(cds, reference_sequences, input_stoichiometry, record_printed_alignment))
		{
			return false;
		}

		if(!assign_new_chain_names_to_chain_descriptors(cds))
		{
			return false;
		}

		std::map<IDChain, std::size_t> map_of_original_chain_names;

		for(std::size_t i=0;i<cds.size();i++)
		{
			const ChainDescriptor& cd=cds[i];
			if(cd.closest_reference_sequence_id>=0)
			{
				map_of_original_chain_names[cd.original_chain_id]=i;
			}
		}

		if(map_of_original_chain_names.empty())
		{
			return false;
		}

		std::vector<AtomBall> new_atom_balls;
		new_atom_balls.reserve(atom_balls.size());

		for(const AtomBall& ab : atom_balls)
		{
			std::map<IDChain, std::size_t>::const_iterator it=map_of_original_chain_names.find(ab.id_atom.id_residue.id_chain);
			if(it!=map_of_original_chain_names.end())
			{
				const ChainDescriptor& cd=cds[it->second];
				std::map<IDResidue, int>::const_iterator jt=cd.best_renumbering_by_reference_sequence.find(ab.id_atom.id_residue);
				if(jt!=cd.best_renumbering_by_reference_sequence.end())
				{
					AtomBall new_ab=ab;
					new_ab.id_atom.id_residue.id_chain.chain_name=cd.new_chain_name;
					new_ab.id_atom.id_residue.residue_seq_number=jt->second;
					new_atom_balls.push_back(new_ab);
				}
			}
		}

		if(new_atom_balls.empty())
		{
			return false;
		}

		atom_balls.swap(new_atom_balls);

		for(ChainDescriptor& cd : cds)
		{
			if(cd.closest_reference_sequence_id>=0)
			{
				result.chain_summaries[cd.new_chain_name]=cd.summarize();
				result.frequencies_of_reference_sequence_ids[cd.closest_reference_sequence_id]++;
			}
		}

		if(!result.empty())
		{
			std::vector< std::pair<std::string, std::string> > name_pairs;
			name_pairs.reserve(result.chain_summaries.size());
			for(std::map<std::string, ChainSummary>::const_iterator it=result.chain_summaries.begin();it!=result.chain_summaries.end();++it)
			{
				name_pairs.push_back(std::pair<std::string, std::string>(it->second.old_name, it->second.current_name));
			}
			std::sort(name_pairs.begin(), name_pairs.end());
			result.chain_renaming_label="(";
			for(std::size_t i=0;i<name_pairs.size();i++)
			{
				result.chain_renaming_label+=(i>0 ? "," : "");
				result.chain_renaming_label+=name_pairs[i].first;
			}
			result.chain_renaming_label+=")[";
			for(std::size_t i=0;i<name_pairs.size();i++)
			{
				result.chain_renaming_label+=(i>0 ? "," : "");
				result.chain_renaming_label+=name_pairs[i].second;
			}
			result.chain_renaming_label+="]";
		}

		return !result.empty();
	}

private:
	struct ChainDescriptor
	{
		IDChain original_chain_id;
		std::vector<IDResidue> sequence_vector;
		std::string sequence_str;
		int closest_reference_sequence_id;
		double best_reference_sequence_identity;
		std::map<IDResidue, int> best_renumbering_by_reference_sequence;
		std::string printed_alignment;
		std::string new_chain_name;

		ChainDescriptor() noexcept : closest_reference_sequence_id(-1), best_reference_sequence_identity(0.0)
		{
		}

		ChainSummary summarize() const noexcept
		{
			ChainSummary cs;
			cs.old_name=original_chain_id.chain_name;
			cs.current_name=new_chain_name;
			cs.reference_sequence_id=closest_reference_sequence_id;
			cs.reference_sequence_identity=best_reference_sequence_identity;
			cs.printed_alignment=printed_alignment;
			return cs;
		}
	};

	static std::vector<ChainDescriptor> init_chain_descriptors(const std::vector<AtomBall>& atom_balls) noexcept
	{
		std::vector<ChainDescriptor> cds;
		if(!atom_balls.empty())
		{
			std::map< IDChain, std::map<IDResidue, std::string> > chain_residues;
			for(const AtomBall& ab : atom_balls)
			{
				chain_residues[ab.id_atom.id_residue.id_chain][ab.id_atom.id_residue]=ab.residue_name;
			}
			cds.reserve(chain_residues.size());
			for(std::map< IDChain, std::map<IDResidue, std::string> >::const_iterator it=chain_residues.begin();it!=chain_residues.end();++it)
			{
				cds.push_back(ChainDescriptor());
				ChainDescriptor& cd=cds.back();
				cd.original_chain_id=it->first;
				const std::map<IDResidue, std::string>& map_of_residue_ids=it->second;
				cd.sequence_vector.reserve(map_of_residue_ids.size());
				for(std::map<IDResidue, std::string>::const_iterator jt=map_of_residue_ids.begin();jt!=map_of_residue_ids.end();++jt)
				{
					cd.sequence_vector.push_back(jt->first);
					cd.sequence_str+=sequtil::SequenceInputUtilities::convert_residue_code_big_to_small(jt->second);
				}
			}
		}
		return cds;
	}

	static bool assign_reference_sequences_to_chain_descriptors(std::vector<ChainDescriptor>& cds, const std::vector<std::string>& reference_sequences, const std::vector<int>& input_stoichiometry, const bool record_printed_alignment) noexcept
	{
		if(cds.empty() || reference_sequences.empty())
		{
			return false;
		}

		std::vector<int> stoichiometry_used=input_stoichiometry;
		stoichiometry_used.resize(reference_sequences.size(), static_cast<int>(cds.size()));

		std::vector< std::pair<double, std::pair<int, int> > > all_identities;
		all_identities.reserve(cds.size()*reference_sequences.size());
		for(std::size_t i=0;i<cds.size();i++)
		{
			for(std::size_t j=0;j<reference_sequences.size();j++)
			{
				const double identity=sequtil::PairwiseSequenceMapping::calculate_sequence_identity(reference_sequences[j], cds[i].sequence_str);
				all_identities.push_back(std::pair<double, std::pair<int, int> >(0.0-identity, std::pair<int, int>(i, j)));
			}
		}
		std::sort(all_identities.begin(), all_identities.end());

		for(const std::pair<double, std::pair<int, int> >& identity_info : all_identities)
		{
			const int ref_index=identity_info.second.second;
			if(stoichiometry_used[ref_index]>0)
			{
				ChainDescriptor& cd=cds[identity_info.second.first];
				if(cd.closest_reference_sequence_id<0)
				{
					sequtil::PairwiseSequenceMapping::Result mapping_result;
					if(sequtil::PairwiseSequenceMapping::construct_mapping(reference_sequences[ref_index], cd.sequence_str, record_printed_alignment, mapping_result))
					{
						cd.closest_reference_sequence_id=ref_index;
						cd.best_reference_sequence_identity=mapping_result.identity;
						for(const std::pair<int, int>& p : mapping_result.mapping)
						{
							cd.best_renumbering_by_reference_sequence[cd.sequence_vector[p.first]]=p.second+1;
						}
						if(record_printed_alignment)
						{
							cd.printed_alignment.swap(mapping_result.printed_alignment);
						}
						stoichiometry_used[ref_index]--;
					}
				}
			}
		}

		return true;
	}

	static bool assign_new_chain_names_to_chain_descriptors(std::vector<ChainDescriptor>& cds) noexcept
	{
		if(cds.empty())
		{
			return false;
		}
		std::map< int, std::set<IDChain> > map_of_reference_ids;
		for(ChainDescriptor& cd : cds)
		{
			if(cd.closest_reference_sequence_id>=0)
			{
				map_of_reference_ids[cd.closest_reference_sequence_id].insert(cd.original_chain_id);
			}
		}
		if(map_of_reference_ids.empty())
		{
			return false;
		}
		std::map<IDChain, std::string> assignment_of_new_chain_names;
		{
			char current_chain_name_pefix='A';
			int current_chain_name_extension=0;
			std::size_t unassagned_count=0;
			do
			{
				unassagned_count=0;
				for(std::map< int, std::set<IDChain> >::iterator it=map_of_reference_ids.begin();it!=map_of_reference_ids.end();++it)
				{
					std::set<IDChain>& original_chain_ids=it->second;
					if(!original_chain_ids.empty())
					{
						std::set<IDChain>::iterator cid_it=original_chain_ids.begin();
						std::string& new_chain_name=assignment_of_new_chain_names[*cid_it];
						new_chain_name=std::string(1, current_chain_name_pefix);
						if(current_chain_name_extension>0)
						{
							new_chain_name+=std::to_string(current_chain_name_extension);
						}
						if(current_chain_name_pefix=='Z')
						{
							current_chain_name_pefix='a';
						}
						else if(current_chain_name_pefix=='z')
						{
							current_chain_name_pefix='A';
							current_chain_name_extension++;
						}
						else
						{
							current_chain_name_pefix++;
						}
						original_chain_ids.erase(cid_it);
						unassagned_count+=original_chain_ids.size();
					}
				}
			}
			while(unassagned_count>0);
		}
		for(ChainDescriptor& cd : cds)
		{
			if(cd.closest_reference_sequence_id>=0)
			{
				std::map<IDChain, std::string>::const_iterator it=assignment_of_new_chain_names.find(cd.original_chain_id);
				if(it!=assignment_of_new_chain_names.end())
				{
					cd.new_chain_name=it->second;
				}
			}
		}
		return true;
	}
};


struct AreaValue
{
	double area;

	AreaValue() noexcept : area(0.0)
	{
	}

	explicit AreaValue(const double area) noexcept : area(area)
	{
	}

	inline void add(const AreaValue& av) noexcept
	{
		area+=av.area;
	}
};

class CADDescriptor
{
public:
	double target_area_sum;
	double model_area_sum;
	double raw_differences_sum;
	double constrained_differences_sum;
	double model_target_area_sum;
	double confusion_TP;
	double confusion_FP;
	double confusion_FN;

	CADDescriptor() noexcept :
			target_area_sum(0),
			model_area_sum(0),
			raw_differences_sum(0),
			constrained_differences_sum(0),
			model_target_area_sum(0),
			confusion_TP(0),
			confusion_FP(0),
			confusion_FN(0)
	{
	}

	CADDescriptor(const double target_area, const double model_area) noexcept :
			target_area_sum(0),
			model_area_sum(0),
			raw_differences_sum(0),
			constrained_differences_sum(0),
			model_target_area_sum(0),
			confusion_TP(0),
			confusion_FP(0),
			confusion_FN(0)
	{
		add(target_area, model_area);
	}

	inline void add(const double target_area, const double model_area) noexcept
	{
		target_area_sum+=target_area;
		model_area_sum+=model_area;
		raw_differences_sum+=std::abs(target_area-model_area);
		constrained_differences_sum+=std::min(std::abs(target_area-model_area), target_area);
		model_target_area_sum+=(target_area>0.0 ? model_area : 0.0);
		confusion_TP+=std::min(target_area, model_area);
		confusion_FP+=(model_area>target_area ? (model_area-target_area) : 0.0);
		confusion_FN+=(target_area>model_area ? (target_area-model_area) : 0.0);
	}

	inline void add(const CADDescriptor& cadd) noexcept
	{
		target_area_sum+=cadd.target_area_sum;
		model_area_sum+=cadd.model_area_sum;
		raw_differences_sum+=cadd.raw_differences_sum;
		constrained_differences_sum+=cadd.constrained_differences_sum;
		model_target_area_sum+=cadd.model_target_area_sum;
		confusion_TP+=cadd.confusion_TP;
		confusion_FP+=cadd.confusion_FP;
		confusion_FN+=cadd.confusion_FN;
	}

	inline double score() const noexcept
	{
		return ((target_area_sum>0.0) ? (1.0-(constrained_differences_sum/target_area_sum)) : -1.0);
	}

	inline double score_F1() const noexcept
	{
		return ((confusion_TP>0.0) ? (confusion_TP/(0.5*(confusion_FP+confusion_FN)+confusion_TP)) : 0.0);
	}
};

class ChainNamingUtilities
{
public:
	template<class MapContainer>
	static MapContainer rename_chains_in_map_container_with_additive_values(const MapContainer& input_container, const std::map<std::string, std::string>& renaming_map) noexcept
	{
		MapContainer result;
		typename MapContainer::iterator hint=result.end();
		for(typename MapContainer::const_iterator it=input_container.begin();it!=input_container.end();++it)
		{
			const std::size_t old_size=result.size();
			hint=result.emplace_hint(hint, it->first.with_renamed_chains(renaming_map), it->second);
			if(old_size==result.size())
			{
				hint->second.add(it->second);
			}
		}
		return result;
	}

	template<class MapContainer>
	static MapContainer restrict_map_container_by_chain_name(const MapContainer& input_container, const std::string& query_chain_name) noexcept
	{
		MapContainer result;
		typename MapContainer::const_iterator hint=result.end();
		for(typename MapContainer::const_iterator it=input_container.begin();it!=input_container.end();++it)
		{
			if(it->first.match_chain_name(query_chain_name))
			{
				hint=result.emplace_hint(hint, it->first, it->second);
			}
		}
		return result;
	}

	template<class VectorContainer>
	static VectorContainer rename_chains_in_vector_container(const VectorContainer& input_container, const std::map<std::string, std::string>& renaming_map) noexcept
	{
		VectorContainer result;
		result.reserve(input_container.size());
		for(typename VectorContainer::const_iterator it=input_container.begin();it!=input_container.end();++it)
		{
			result.push_back(it->with_renamed_chains(renaming_map));
		}
		return result;
	}

	static std::set<std::string> rename_chains(const std::set<std::string>& chain_ids, const std::map<std::string, std::string>& renaming_map) noexcept
	{
		std::set<std::string> result;
		for(std::set<std::string>::const_iterator it=chain_ids.begin();it!=chain_ids.end();++it)
		{
			std::map<std::string, std::string>::const_iterator jt=renaming_map.find(*it);
			if(jt!=renaming_map.end())
			{
				result.insert(jt->second);
			}
			else
			{
				result.insert(*it);
			}
		}
		return result;
	}

	static std::map< std::string, std::set<std::string> > rename_chains(const std::map< std::string, std::set<std::string> >& chain_adjacencies, const std::map<std::string, std::string>& renaming_map) noexcept
	{
		std::map< std::string, std::set<std::string> > result;
		for(std::map< std::string, std::set<std::string> >::const_iterator it=chain_adjacencies.begin();it!=chain_adjacencies.end();++it)
		{
			std::map<std::string, std::string>::const_iterator jt=renaming_map.find(it->first);
			if(jt!=renaming_map.end())
			{
				result.emplace(jt->second, rename_chains(it->second, renaming_map));
			}
			else
			{
				result.emplace(it->first, rename_chains(it->second, renaming_map));
			}
		}
		return result;
	}

	static std::map<std::string, std::string> generate_renaming_map_from_two_vectors_with_padding(const std::vector<std::string>& left, const std::vector<std::string>& right) noexcept
	{
		const std::set<std::string> set_of_right_values(right.begin(), right.end());
		std::map<std::string, std::string> result;
		for(std::size_t i=0;i<left.size();i++)
		{
			if(i<right.size())
			{
				result[left[i]]=right[i];
			}
			else
			{
				result[left[i]]=(set_of_right_values.count(left[i])==0 ? left[i] : (std::string("_")+left[i]));
			}
		}
		return result;
	}
};

struct MolecularFileInput
{
	std::string input_file_path;
	bool include_heteroatoms;
	bool read_as_assembly;

	MolecularFileInput() noexcept :
		include_heteroatoms(true),
		read_as_assembly(false)
	{
	}

	explicit MolecularFileInput(const std::string& input_file) noexcept :
		input_file_path(input_file),
		include_heteroatoms(true),
		read_as_assembly(false)
	{
	}

	MolecularFileInput(const std::string& input_file, const bool include_heteroatoms, const bool read_as_assembly) noexcept :
		input_file_path(input_file),
		include_heteroatoms(include_heteroatoms),
		read_as_assembly(read_as_assembly)
	{
	}
};

class ScorableData
{
public:
	struct ConstructionParameters
	{
		double probe;
		bool record_atom_balls;
		bool record_sequence_alignments;
		bool record_atom_atom_contact_summaries;
		bool record_residue_residue_contact_summaries;
		bool record_chain_chain_contact_summaries;
		bool record_atom_cell_summaries;
		bool record_residue_cell_summaries;
		bool record_chain_cell_summaries;
		bool record_atom_site_summaries;
		bool record_residue_site_summaries;
		bool record_chain_site_summaries;
		voronotalt::FilteringBySphereLabels::ExpressionForSingle filtering_expression_for_restricting_input_balls;
		voronotalt::FilteringBySphereLabels::ExpressionForPair filtering_expression_for_restricting_contact_descriptors;
		voronotalt::FilteringBySphereLabels::ExpressionForSingle filtering_expression_for_restricting_atom_descriptors;
		std::vector<std::string> reference_sequences;
		std::vector<int> reference_stoichiometry;

		ConstructionParameters() noexcept :
			probe(1.4),
			record_atom_balls(false),
			record_sequence_alignments(false),
			record_atom_atom_contact_summaries(false),
			record_residue_residue_contact_summaries(false),
			record_chain_chain_contact_summaries(false),
			record_atom_cell_summaries(false),
			record_residue_cell_summaries(false),
			record_chain_cell_summaries(false),
			record_atom_site_summaries(false),
			record_residue_site_summaries(false),
			record_chain_site_summaries(false)
		{
		}

		bool valid() const noexcept
		{
			return (record_atom_atom_contact_summaries || record_residue_residue_contact_summaries || record_chain_chain_contact_summaries || record_atom_cell_summaries || record_residue_cell_summaries || record_chain_cell_summaries || record_atom_site_summaries || record_residue_site_summaries || record_chain_site_summaries);
		}
	};

	std::vector<AtomBall> atom_balls;
	ChainsSequencesMapping::Result chain_sequences_mapping_result;
	std::map<IDAtomAtom, AreaValue> atom_atom_contact_areas;
	std::map<IDResidueResidue, AreaValue> residue_residue_contact_areas;
	std::map<IDChainChain, AreaValue> chain_chain_contact_areas;
	std::map<IDAtom, AreaValue> atom_sas_areas;
	std::map<IDResidue, AreaValue> residue_sas_areas;
	std::map<IDChain, AreaValue> chain_sas_areas;
	std::map<IDAtom, AreaValue> atom_site_areas;
	std::map<IDResidue, AreaValue> residue_site_areas;
	std::map<IDChain, AreaValue> chain_site_areas;
	std::set<std::string> all_chain_ids;
	std::map< std::string, std::set<std::string> > involved_chain_adjacencies;

	ScorableData() noexcept : valid_(false)
	{
	}

	bool valid() const noexcept
	{
		return valid_;
	}

	void clear() noexcept
	{
		valid_=false;
		atom_balls.clear();
		atom_atom_contact_areas.clear();
		residue_residue_contact_areas.clear();
		chain_chain_contact_areas.clear();
		atom_sas_areas.clear();
		residue_sas_areas.clear();
		chain_sas_areas.clear();
		atom_site_areas.clear();
		residue_site_areas.clear();
		chain_site_areas.clear();
		all_chain_ids.clear();
		involved_chain_adjacencies.clear();
	}

	bool construct(const ConstructionParameters& params, const MolecularFileInput& molecular_file_input, std::ostream& error_log) noexcept
	{
		return construct(params, std::vector<AtomBall>(), molecular_file_input, error_log);
	}

	bool construct(const ConstructionParameters& params, const std::vector<AtomBall>& input_atom_balls, std::ostream& error_log) noexcept
	{
		return construct(params, input_atom_balls, MolecularFileInput(), error_log);
	}

	ScorableData rename_chains(const std::map<std::string, std::string>& renaming_map) const noexcept
	{
		ScorableData modified_data;
		if(valid())
		{
			modified_data.atom_balls=ChainNamingUtilities::rename_chains_in_vector_container(atom_balls, renaming_map);
			modified_data.atom_atom_contact_areas=ChainNamingUtilities::rename_chains_in_map_container_with_additive_values(atom_atom_contact_areas, renaming_map);
			modified_data.residue_residue_contact_areas=ChainNamingUtilities::rename_chains_in_map_container_with_additive_values(residue_residue_contact_areas, renaming_map);
			modified_data.chain_chain_contact_areas=ChainNamingUtilities::rename_chains_in_map_container_with_additive_values(chain_chain_contact_areas, renaming_map);
			modified_data.atom_sas_areas=ChainNamingUtilities::rename_chains_in_map_container_with_additive_values(atom_sas_areas, renaming_map);
			modified_data.residue_sas_areas=ChainNamingUtilities::rename_chains_in_map_container_with_additive_values(residue_sas_areas, renaming_map);
			modified_data.chain_sas_areas=ChainNamingUtilities::rename_chains_in_map_container_with_additive_values(chain_sas_areas, renaming_map);
			modified_data.atom_site_areas=ChainNamingUtilities::rename_chains_in_map_container_with_additive_values(atom_site_areas, renaming_map);
			modified_data.residue_site_areas=ChainNamingUtilities::rename_chains_in_map_container_with_additive_values(residue_site_areas, renaming_map);
			modified_data.chain_site_areas=ChainNamingUtilities::rename_chains_in_map_container_with_additive_values(chain_site_areas, renaming_map);
			modified_data.all_chain_ids=ChainNamingUtilities::rename_chains(all_chain_ids, renaming_map);
			modified_data.involved_chain_adjacencies=ChainNamingUtilities::rename_chains(involved_chain_adjacencies, renaming_map);
			modified_data.valid_=true;
		}
		return modified_data;
	}

private:
	class OnReturnGuard
	{
	public:
		explicit OnReturnGuard(ScorableData& self) noexcept : self_(self)
		{
		}

		~OnReturnGuard() noexcept
		{
			if(!self_.valid())
			{
				self_.clear();
			}
		}

	private:
		ScorableData& self_;
	};

	static bool collect_spheres_input_result_from_input_atom_balls(const std::vector<AtomBall>& input_atom_balls, const double probe, voronotalt::SpheresInput::Result& spheres_input_result) noexcept
	{
		spheres_input_result=voronotalt::SpheresInput::Result();
		voronotalt::MolecularFileReading::Data mol_data;
		mol_data.atom_records.resize(input_atom_balls.size());
		for(std::size_t i=0;i<input_atom_balls.size();i++)
		{
			input_atom_balls[i].write_into_atom_record(mol_data.atom_records[i]);
		}
		if(!voronotalt::SpheresInput::read_labeled_spheres_from_molecular_data_descriptor(mol_data, probe, true, spheres_input_result) || spheres_input_result.spheres.size()!=input_atom_balls.size() || spheres_input_result.sphere_labels.size()!=input_atom_balls.size())
		{
			return false;
		}
		for(std::size_t i=0;i<input_atom_balls.size();i++)
		{
			const AtomBall& ab=input_atom_balls[i];
			if(ab.r>0.0)
			{
				spheres_input_result.spheres[i].r=ab.r+probe;
			}
		}
		return true;
	}

	static void collect_input_atom_balls_from_spheres_input_result(const voronotalt::SpheresInput::Result& spheres_input_result, const double probe, std::vector<AtomBall>& input_atom_balls) noexcept
	{
		input_atom_balls.clear();
		input_atom_balls.resize(spheres_input_result.spheres.size());
		for(std::size_t i=0;i<spheres_input_result.spheres.size();i++)
		{
			const voronotalt::SimpleSphere& s=spheres_input_result.spheres[i];
			const voronotalt::SphereLabeling::SphereLabel& sl=spheres_input_result.sphere_labels[i];

			AtomBall& mab=input_atom_balls[i];
			mab.id_atom=IDAtom(sl);
			mab.residue_name=sl.expanded_residue_id.rname;
			mab.x=s.p.x;
			mab.y=s.p.y;
			mab.z=s.p.z;
			mab.r=s.r-probe;
		}
	}

	bool construct(const ConstructionParameters& params, const std::vector<AtomBall>& input_atom_balls, const MolecularFileInput& molecular_file_input, std::ostream& error_log) noexcept
	{
		clear();

		OnReturnGuard on_return_guard(*this);

		if(!params.valid())
		{
			error_log << "Invalid scorable data recording parameters.\n";
			return false;
		}

		if(input_atom_balls.empty() && molecular_file_input.input_file_path.empty())
		{
			error_log << "No input source provided.\n";
			return false;
		}

		if(!input_atom_balls.empty() && !molecular_file_input.input_file_path.empty())
		{
			error_log << "Conflicted sources of input provided.\n";
			return false;
		}

		const bool need_to_summarize_cells=(params.record_atom_cell_summaries || params.record_residue_cell_summaries || params.record_chain_cell_summaries);
		const bool need_to_summarize_sites=(params.record_atom_site_summaries || params.record_residue_site_summaries || params.record_chain_site_summaries);

		const bool can_restrict_contacts_on_construction=!need_to_summarize_cells;
		const bool compute_only_inter_chain_contacts=(can_restrict_contacts_on_construction && !params.record_atom_atom_contact_summaries && !params.record_residue_residue_contact_summaries && !params.record_atom_site_summaries && !params.record_residue_site_summaries);
		const bool compute_only_inter_residue_contacts=(can_restrict_contacts_on_construction && !params.record_atom_atom_contact_summaries && !params.record_atom_site_summaries);

		voronotalt::TimeRecorder time_recorder;

		voronotalt::SpheresInput::Result spheres_input_result;

		if(!input_atom_balls.empty())
		{
			if(!collect_spheres_input_result_from_input_atom_balls(input_atom_balls, params.probe, spheres_input_result))
			{
				error_log << "Failed to process input vector of atom balls.\n";
				return false;
			}
		}
		else
		{
			std::string input_data;

			if(!voronotalt::read_whole_file_or_pipe_or_stdin_to_string(molecular_file_input.input_file_path, input_data))
			{
				error_log << "Failed to open file '" << molecular_file_input.input_file_path << "'.\n";
				return false;
			}

			if(input_data.empty())
			{
				error_log << "No data in file '" << molecular_file_input.input_file_path << "'.\n";
				return false;
			}

			const voronotalt::MolecularFileReading::Parameters molecular_file_reading_parameters(molecular_file_input.include_heteroatoms, false, molecular_file_input.read_as_assembly);

			std::ostringstream err_stream;

			if(!voronotalt::SpheresInput::read_labeled_or_unlabeled_spheres_from_string(input_data, molecular_file_reading_parameters, params.probe, spheres_input_result, err_stream, time_recorder))
			{
				error_log << "Failed to parse file '" << molecular_file_input.input_file_path << "': " << err_stream.str() << "\n";
				return false;
			}
		}

		if(spheres_input_result.sphere_labels.size()!=spheres_input_result.spheres.size())
		{
			error_log << "Input is not in a recognized molecular data format.\n";
			return false;
		}
		
		if(params.record_atom_balls || !params.reference_sequences.empty())
		{
			collect_input_atom_balls_from_spheres_input_result(spheres_input_result, params.probe, atom_balls);
		}

		if(!params.reference_sequences.empty())
		{
			if(!ChainsSequencesMapping::remap_chain_names_and_residue_numbers_by_reference_sequences(params.reference_sequences, params.reference_stoichiometry, params.record_sequence_alignments, atom_balls, chain_sequences_mapping_result))
			{
				error_log << "Failed to map chain sequences and renumber residues based on reference sequences.\n";
				return false;
			}
			if(!collect_spheres_input_result_from_input_atom_balls(atom_balls, params.probe, spheres_input_result))
			{
				error_log << "Failed to process input vector of atom balls.\n";
				return false;
			}
		}

		if(!params.filtering_expression_for_restricting_input_balls.allow_all())
		{
			voronotalt::FilteringBySphereLabels::VectorExpressionResult ver=params.filtering_expression_for_restricting_input_balls.filter_vector(spheres_input_result.sphere_labels);
			if(!ver.expression_valid)
			{
				error_log << "Restricting input by applying filtering expression failed.\n";
				return false;
			}
			if(!ver.expression_matched())
			{
				error_log << "No input satisfied restricting filtering expression.\n";
				return false;
			}
			if(!spheres_input_result.restrict_spheres(ver.expression_matched_all, ver.expression_matched_ids))
			{
				error_log << "Failed to restrict input.\n";
				return false;
			}
		}

		voronotalt::RadicalTessellation::Result result;

		{
			const voronotalt::PeriodicBox periodic_box;

			voronotalt::RadicalTessellation::ResultGraphics result_graphics;

			const std::vector<int> null_grouping;
			const std::vector<int>& grouping_for_filtering=(compute_only_inter_chain_contacts ? spheres_input_result.grouping_by_chain : (compute_only_inter_residue_contacts ? spheres_input_result.grouping_by_residue : null_grouping));

			voronotalt::SpheresContainer spheres_container;
			spheres_container.init(spheres_input_result.spheres, periodic_box, time_recorder);

			voronotalt::SpheresContainer::ResultOfPreparationForTessellation preparation_result;
			spheres_container.prepare_for_tessellation(grouping_for_filtering, preparation_result, time_recorder);

			if(can_restrict_contacts_on_construction && !params.filtering_expression_for_restricting_contact_descriptors.allow_all())
			{
				const voronotalt::FilteringBySphereLabels::VectorExpressionResult ver=params.filtering_expression_for_restricting_contact_descriptors.filter_vector(spheres_input_result.sphere_labels, spheres_input_result.spheres, preparation_result.relevant_collision_ids);
				if(!ver.expression_matched() || !preparation_result.restrict_relevant_collision_ids(ver.expression_matched_all, ver.expression_matched_ids))
				{
					error_log << "Failed to restrict contacts for construction.\n";
					return false;
				}
			}

			const bool with_tessellation_net=false;
			const bool with_graphics=false;
			const bool with_sas_graphics_if_possible=false;

			voronotalt::RadicalTessellation::construct_full_tessellation(
					spheres_container,
					preparation_result,
					with_tessellation_net,
					voronotalt::RadicalTessellation::ParametersForGraphics(with_graphics, with_sas_graphics_if_possible),
					voronotalt::RadicalTessellation::ParametersForGeneratingSummaries(need_to_summarize_sites, need_to_summarize_cells),
					result,
					result_graphics,
					time_recorder);

			if(!can_restrict_contacts_on_construction && !params.filtering_expression_for_restricting_contact_descriptors.allow_all())
			{
				const voronotalt::FilteringBySphereLabels::VectorExpressionResult ver=params.filtering_expression_for_restricting_contact_descriptors.filter_vector(spheres_input_result.sphere_labels, spheres_input_result.spheres, voronotalt::FilteringBySphereLabels::ExpressionForPair::adapt_indices_container(result.contacts_summaries));
				if(!ver.expression_matched() || !voronotalt::RadicalTessellation::restrict_result_contacts(ver.expression_matched_all, ver.expression_matched_ids, result, result_graphics))
				{
					error_log << "Failed to restrict contacts for output.\n";
					return false;
				}
			}

			if(!result.cells_summaries.empty() && !params.filtering_expression_for_restricting_atom_descriptors.allow_all())
			{
				const voronotalt::FilteringBySphereLabels::VectorExpressionResult ver=params.filtering_expression_for_restricting_atom_descriptors.filter_vector(spheres_input_result.sphere_labels);
				if(!ver.expression_matched() || !voronotalt::RadicalTessellation::restrict_result_cells(ver.expression_matched_all, ver.expression_matched_ids, result, result_graphics))
				{
					error_log << "Failed to restrict cells for output.\n";
					return false;
				}
			}
		}

		voronotalt::RadicalTessellation::GroupedResult result_grouped_by_residue;

		if(params.record_residue_residue_contact_summaries || params.record_residue_cell_summaries || params.record_residue_site_summaries)
		{
			voronotalt::RadicalTessellation::group_results(result, spheres_input_result.grouping_by_residue, result_grouped_by_residue, time_recorder);
		}

		voronotalt::RadicalTessellation::GroupedResult result_grouped_by_chain;

		if(params.record_chain_chain_contact_summaries || params.record_chain_cell_summaries || params.record_chain_site_summaries)
		{
			voronotalt::RadicalTessellation::group_results(result, spheres_input_result.grouping_by_chain, result_grouped_by_chain, time_recorder);
		}
		
		if(!params.record_atom_balls)
		{
			std::vector<AtomBall>().swap(atom_balls);
		}

		if(params.record_atom_atom_contact_summaries)
		{
			for(std::size_t i=0;i<result.contacts_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::ContactDescriptorSummary& cds=result.contacts_summaries[i];
				const voronotalt::SphereLabeling::SphereLabel& sl1=spheres_input_result.sphere_labels[cds.id_a];
				const voronotalt::SphereLabeling::SphereLabel& sl2=spheres_input_result.sphere_labels[cds.id_b];
				atom_atom_contact_areas.emplace_hint(atom_atom_contact_areas.end(), IDAtomAtom(IDAtom(sl1), IDAtom(sl2)), AreaValue(cds.area));
			}
		}

		if(params.record_residue_residue_contact_summaries)
		{
			for(std::size_t i=0;i<result_grouped_by_residue.grouped_contacts_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::ContactDescriptorSummary& cds=result.contacts_summaries[result_grouped_by_residue.grouped_contacts_representative_ids[i]];
				const voronotalt::RadicalTessellation::TotalContactDescriptorsSummary& tcds=result_grouped_by_residue.grouped_contacts_summaries[i];
				const voronotalt::SphereLabeling::SphereLabel& sl1=spheres_input_result.sphere_labels[cds.id_a];
				const voronotalt::SphereLabeling::SphereLabel& sl2=spheres_input_result.sphere_labels[cds.id_b];
				residue_residue_contact_areas.emplace_hint(residue_residue_contact_areas.end(), IDResidueResidue(IDResidue(sl1), IDResidue(sl2)), AreaValue(tcds.area));
			}
		}

		if(params.record_chain_chain_contact_summaries)
		{
			for(std::size_t i=0;i<result_grouped_by_chain.grouped_contacts_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::ContactDescriptorSummary& cds=result.contacts_summaries[result_grouped_by_chain.grouped_contacts_representative_ids[i]];
				const voronotalt::RadicalTessellation::TotalContactDescriptorsSummary& tcds=result_grouped_by_chain.grouped_contacts_summaries[i];
				const voronotalt::SphereLabeling::SphereLabel& sl1=spheres_input_result.sphere_labels[cds.id_a];
				const voronotalt::SphereLabeling::SphereLabel& sl2=spheres_input_result.sphere_labels[cds.id_b];
				chain_chain_contact_areas.emplace_hint(chain_chain_contact_areas.end(), IDChainChain(IDChain(sl1), IDChain(sl2)), AreaValue(tcds.area));
			}
		}

		if(params.record_atom_cell_summaries)
		{
			for(std::size_t i=0;i<result.cells_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::CellContactDescriptorsSummary& ccds=result.cells_summaries[i];
				if(ccds.sas_area>0.0)
				{
					const voronotalt::SphereLabeling::SphereLabel& sl=spheres_input_result.sphere_labels[ccds.id];
					atom_sas_areas.emplace_hint(atom_sas_areas.end(), IDAtom(sl), AreaValue(ccds.sas_area));
				}
			}
		}

		if(params.record_residue_cell_summaries)
		{
			for(std::size_t i=0;i<result_grouped_by_residue.grouped_cells_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::CellContactDescriptorsSummary& ccds=result.cells_summaries[result_grouped_by_residue.grouped_cells_representative_ids[i]];
				const voronotalt::RadicalTessellation::TotalCellContactDescriptorsSummary& tccds=result_grouped_by_residue.grouped_cells_summaries[i];
				if(tccds.sas_area>0.0)
				{
					const voronotalt::SphereLabeling::SphereLabel& sl=spheres_input_result.sphere_labels[ccds.id];
					residue_sas_areas.emplace_hint(residue_sas_areas.end(), IDResidue(sl), AreaValue(tccds.sas_area));
				}
			}
		}

		if(params.record_chain_cell_summaries)
		{
			for(std::size_t i=0;i<result_grouped_by_chain.grouped_cells_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::CellContactDescriptorsSummary& ccds=result.cells_summaries[result_grouped_by_chain.grouped_cells_representative_ids[i]];
				const voronotalt::RadicalTessellation::TotalCellContactDescriptorsSummary& tccds=result_grouped_by_chain.grouped_cells_summaries[i];
				if(tccds.sas_area>0.0)
				{
					const voronotalt::SphereLabeling::SphereLabel& sl=spheres_input_result.sphere_labels[ccds.id];
					chain_sas_areas.emplace_hint(chain_sas_areas.end(), IDChain(sl), AreaValue(tccds.sas_area));
				}
			}
		}

		if(params.record_atom_site_summaries)
		{
			for(std::size_t i=0;i<result.sites_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::SiteContactDescriptorsSummary& scds=result.sites_summaries[i];
				if(scds.area>0.0)
				{
					const voronotalt::SphereLabeling::SphereLabel& sl=spheres_input_result.sphere_labels[scds.id];
					atom_site_areas.emplace_hint(atom_site_areas.end(), IDAtom(sl), AreaValue(scds.area));
				}
			}
		}

		if(params.record_residue_site_summaries)
		{
			for(std::size_t i=0;i<result_grouped_by_residue.grouped_sites_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::SiteContactDescriptorsSummary& scds=result.sites_summaries[result_grouped_by_residue.grouped_sites_representative_ids[i]];
				const voronotalt::RadicalTessellation::TotalSiteContactDescriptorsSummary& tscds=result_grouped_by_residue.grouped_sites_summaries[i];
				if(tscds.area>0.0)
				{
					const voronotalt::SphereLabeling::SphereLabel& sl=spheres_input_result.sphere_labels[scds.id];
					residue_site_areas.emplace_hint(residue_site_areas.end(), IDResidue(sl), AreaValue(tscds.area));
				}
			}
		}

		if(params.record_chain_site_summaries)
		{
			for(std::size_t i=0;i<result_grouped_by_chain.grouped_sites_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::SiteContactDescriptorsSummary& scds=result.sites_summaries[result_grouped_by_chain.grouped_sites_representative_ids[i]];
				const voronotalt::RadicalTessellation::TotalSiteContactDescriptorsSummary& tscds=result_grouped_by_chain.grouped_sites_summaries[i];
				if(tscds.area>0.0)
				{
					const voronotalt::SphereLabeling::SphereLabel& sl=spheres_input_result.sphere_labels[scds.id];
					chain_site_areas.emplace_hint(chain_site_areas.end(), IDChain(sl), AreaValue(tscds.area));
				}
			}
		}

		for(std::size_t i=0;i<spheres_input_result.sphere_labels.size();i++)
		{
			all_chain_ids.emplace_hint(all_chain_ids.end(), spheres_input_result.sphere_labels[i].chain_id);
		}

		if(!result_grouped_by_chain.grouped_contacts_summaries.empty())
		{
			for(std::size_t i=0;i<result_grouped_by_chain.grouped_contacts_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::ContactDescriptorSummary& cds=result.contacts_summaries[result_grouped_by_chain.grouped_contacts_representative_ids[i]];
				const std::string& ic1=spheres_input_result.sphere_labels[cds.id_a].chain_id;
				const std::string& ic2=spheres_input_result.sphere_labels[cds.id_b].chain_id;
				std::set<std::string>& set1=involved_chain_adjacencies[ic1];
				std::set<std::string>& set2=involved_chain_adjacencies[ic2];
				set1.emplace_hint(set1.end(), ic2);
				set2.emplace_hint(set2.end(), ic1);
			}
		}
		else if(!result_grouped_by_residue.grouped_contacts_summaries.empty())
		{
			for(std::size_t i=0;i<result_grouped_by_residue.grouped_contacts_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::ContactDescriptorSummary& cds=result.contacts_summaries[result_grouped_by_residue.grouped_contacts_representative_ids[i]];
				const std::string& ic1=spheres_input_result.sphere_labels[cds.id_a].chain_id;
				const std::string& ic2=spheres_input_result.sphere_labels[cds.id_b].chain_id;
				if(ic1!=ic2)
				{
					std::set<std::string>& set1=involved_chain_adjacencies[ic1];
					std::set<std::string>& set2=involved_chain_adjacencies[ic2];
					set1.emplace_hint(set1.end(), ic2);
					set2.emplace_hint(set2.end(), ic1);
				}
			}
		}
		else
		{
			for(std::size_t i=0;i<result.contacts_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::ContactDescriptorSummary& cds=result.contacts_summaries[i];
				const std::string& ic1=spheres_input_result.sphere_labels[cds.id_a].chain_id;
				const std::string& ic2=spheres_input_result.sphere_labels[cds.id_b].chain_id;
				if(ic1!=ic2)
				{
					std::set<std::string>& set1=involved_chain_adjacencies[ic1];
					std::set<std::string>& set2=involved_chain_adjacencies[ic2];
					set1.emplace_hint(set1.end(), ic2);
					set2.emplace_hint(set2.end(), ic1);
				}
			}
		}

		valid_=true;

		return true;
	}

	bool valid_;
};

class ScoringResult
{
public:
	struct ConstructionParameters
	{
		bool remap_chains;
		bool record_local_scores_on_atom_level;
		bool record_local_scores_on_residue_level;
		bool record_local_scores_on_chain_level;
		bool record_compatible_model_atom_balls;
		int max_permutations_to_check_exhaustively;
		std::map<std::string, std::string> chain_renaming_map;

		ConstructionParameters() noexcept : remap_chains(false), record_local_scores_on_atom_level(false), record_local_scores_on_residue_level(false), record_local_scores_on_chain_level(false), record_compatible_model_atom_balls(false), max_permutations_to_check_exhaustively(200)
		{
		}
	};

	ConstructionParameters params;

	std::map<IDAtomAtom, CADDescriptor> cadscores_atom_atom;
	std::map<IDResidueResidue, CADDescriptor> cadscores_atom_atom_summarized_per_residue_residue;
	std::map<IDChainChain, CADDescriptor> cadscores_atom_atom_summarized_per_chain_chain;
	std::map<IDAtom, CADDescriptor> cadscores_atom_atom_summarized_per_atom;
	std::map<IDResidue, CADDescriptor> cadscores_atom_atom_summarized_per_residue;
	std::map<IDChain, CADDescriptor> cadscores_atom_atom_summarized_per_chain;
	CADDescriptor cadscores_atom_atom_summarized_globally;

	std::map<IDResidueResidue, CADDescriptor> cadscores_residue_residue;
	std::map<IDChainChain, CADDescriptor> cadscores_residue_residue_summarized_per_chain_chain;
	std::map<IDResidue, CADDescriptor> cadscores_residue_residue_summarized_per_residue;
	std::map<IDChain, CADDescriptor> cadscores_residue_residue_summarized_per_chain;
	CADDescriptor cadscores_residue_residue_summarized_globally;

	std::map<IDChainChain, CADDescriptor> cadscores_chain_chain;
	std::map<IDChain, CADDescriptor> cadscores_chain_chain_contact_summarized_per_chain;
	CADDescriptor cadscores_chain_chain_summarized_globally;

	std::map<IDAtom, CADDescriptor> cadscores_atom_sas;
	std::map<IDResidue, CADDescriptor> cadscores_atom_sas_summarized_per_residue;
	std::map<IDChain, CADDescriptor> cadscores_atom_sas_summarized_per_chain;
	CADDescriptor cadscores_atom_sas_summarized_globally;

	std::map<IDResidue, CADDescriptor> cadscores_residue_sas;
	std::map<IDChain, CADDescriptor> cadscores_residue_sas_summarized_per_chain;
	CADDescriptor cadscores_residue_sas_summarized_globally;

	std::map<IDChain, CADDescriptor> cadscores_chain_sas;
	CADDescriptor cadscores_chain_sas_summarized_globally;

	std::map<IDAtom, CADDescriptor> cadscores_atom_site;
	std::map<IDResidue, CADDescriptor> cadscores_atom_site_summarized_per_residue;
	std::map<IDChain, CADDescriptor> cadscores_atom_site_summarized_per_chain;
	CADDescriptor cadscores_atom_site_summarized_globally;

	std::map<IDResidue, CADDescriptor> cadscores_residue_site;
	std::map<IDChain, CADDescriptor> cadscores_residue_site_summarized_per_chain;
	CADDescriptor cadscores_residue_site_summarized_globally;

	std::map<IDChain, CADDescriptor> cadscores_chain_site;
	CADDescriptor cadscores_chain_site_summarized_globally;

	std::vector<AtomBall> compatible_model_atom_balls;

	ScoringResult() noexcept : valid_(false)
	{
	}

	bool valid() const noexcept
	{
		return valid_;
	}

	void clear() noexcept
	{
		valid_=false;

		params=ConstructionParameters();

		cadscores_atom_atom.clear();
		cadscores_atom_atom_summarized_per_residue_residue.clear();
		cadscores_atom_atom_summarized_per_chain_chain.clear();
		cadscores_atom_atom_summarized_per_atom.clear();
		cadscores_atom_atom_summarized_per_residue.clear();
		cadscores_atom_atom_summarized_per_chain.clear();
		cadscores_atom_atom_summarized_globally=CADDescriptor();

		cadscores_residue_residue.clear();
		cadscores_residue_residue_summarized_per_chain_chain.clear();
		cadscores_residue_residue_summarized_per_residue.clear();
		cadscores_residue_residue_summarized_per_chain.clear();
		cadscores_residue_residue_summarized_globally=CADDescriptor();

		cadscores_chain_chain.clear();
		cadscores_chain_chain_contact_summarized_per_chain.clear();
		cadscores_chain_chain_summarized_globally=CADDescriptor();

		cadscores_atom_sas.clear();
		cadscores_atom_sas_summarized_per_residue.clear();
		cadscores_atom_sas_summarized_per_chain.clear();
		cadscores_atom_sas_summarized_globally=CADDescriptor();

		cadscores_residue_sas.clear();
		cadscores_residue_sas_summarized_per_chain.clear();
		cadscores_residue_sas_summarized_globally=CADDescriptor();

		cadscores_chain_sas.clear();
		cadscores_chain_sas_summarized_globally=CADDescriptor();

		cadscores_atom_site.clear();
		cadscores_atom_site_summarized_per_residue.clear();
		cadscores_atom_site_summarized_per_chain.clear();
		cadscores_atom_site_summarized_globally=CADDescriptor();

		cadscores_residue_site.clear();
		cadscores_residue_site_summarized_per_chain.clear();
		cadscores_residue_site_summarized_globally=CADDescriptor();

		cadscores_chain_site.clear();
		cadscores_chain_site_summarized_globally=CADDescriptor();
	}

	bool construct(const ConstructionParameters& init_params, const ScorableData& target_data, const ScorableData& model_data, std::ostream& error_log) noexcept
	{
		return construct(init_params, false, target_data, model_data, error_log);
	}

private:
	class OnReturnGuard
	{
	public:
		explicit OnReturnGuard(ScoringResult& self) noexcept : self_(self)
		{
		}

		~OnReturnGuard() noexcept
		{
			if(!self_.valid())
			{
				self_.clear();
			}
		}

	private:
		ScoringResult& self_;
	};

	bool construct(const ConstructionParameters& init_params, const bool chain_renaming_done, const ScorableData& target_data, const ScorableData& model_data, std::ostream& error_log) noexcept
	{
		clear();

		OnReturnGuard on_return_guard(*this);

		if(!target_data.valid())
		{
			error_log << "Invalid target data provided.\n";
			return false;
		}

		if(!model_data.valid())
		{
			error_log << "Invalid model data provided.\n";
			return false;
		}

		if(target_data.atom_atom_contact_areas.empty()
				&& target_data.residue_residue_contact_areas.empty()
				&& target_data.chain_chain_contact_areas.empty()
				&& target_data.atom_sas_areas.empty()
				&& target_data.residue_sas_areas.empty()
				&& target_data.chain_sas_areas.empty()
				&& target_data.atom_site_areas.empty()
				&& target_data.residue_site_areas.empty()
				&& target_data.chain_site_areas.empty())
		{
			error_log << "No target data filled.\n";
			return false;
		}

		if(!chain_renaming_done && (init_params.remap_chains || !init_params.chain_renaming_map.empty()))
		{
			if(init_params.remap_chains && !init_params.chain_renaming_map.empty())
			{
				error_log << "Automatic chain remapping was requested, but non-empty chain renaming map was provided.\n";
				return false;
			}

			if(init_params.remap_chains)
			{
				ConstructionParameters adjusted_init_params=init_params;
				if(!remap_chains_optimally(target_data, model_data, adjusted_init_params.max_permutations_to_check_exhaustively, adjusted_init_params.chain_renaming_map, error_log))
				{
					error_log << "Automatic chain remapping failed.\n";
					return false;
				}
				return construct(adjusted_init_params, true, target_data, model_data.rename_chains(adjusted_init_params.chain_renaming_map), error_log);
			}

			if(!init_params.chain_renaming_map.empty())
			{
				return construct(init_params, true, target_data, model_data.rename_chains(init_params.chain_renaming_map), error_log);
			}
		}

		params=init_params;

		if(!target_data.atom_atom_contact_areas.empty())
		{
			if(params.record_local_scores_on_atom_level || params.record_local_scores_on_residue_level || params.record_local_scores_on_chain_level)
			{
				merge_target_and_model_area_values(target_data.atom_atom_contact_areas, model_data.atom_atom_contact_areas, cadscores_atom_atom);
				cadscores_atom_atom_summarized_globally=construct_global_cad_descriptor(cadscores_atom_atom);
				for(std::map<IDAtomAtom, CADDescriptor>::const_iterator it=cadscores_atom_atom.begin();it!=cadscores_atom_atom.end();++it)
				{
					if(params.record_local_scores_on_atom_level)
					{
						cadscores_atom_atom_summarized_per_atom[it->first.id_a].add(it->second);
						cadscores_atom_atom_summarized_per_atom[it->first.id_b].add(it->second);
					}
					if(params.record_local_scores_on_residue_level)
					{
						cadscores_atom_atom_summarized_per_residue[it->first.id_a.id_residue].add(it->second);
						cadscores_atom_atom_summarized_per_residue[it->first.id_b.id_residue].add(it->second);
						cadscores_atom_atom_summarized_per_residue_residue[IDResidueResidue(it->first.id_a.id_residue, it->first.id_b.id_residue)].add(it->second);
					}
					if(params.record_local_scores_on_chain_level)
					{
						cadscores_atom_atom_summarized_per_chain[it->first.id_a.id_residue.id_chain].add(it->second);
						cadscores_atom_atom_summarized_per_chain[it->first.id_b.id_residue.id_chain].add(it->second);
						cadscores_atom_atom_summarized_per_chain_chain[IDChainChain(it->first.id_a.id_residue.id_chain, it->first.id_b.id_residue.id_chain)].add(it->second);
					}
				}
				if(!params.record_local_scores_on_atom_level)
				{
					cadscores_atom_atom.clear();
				}
			}
			else
			{
				cadscores_atom_atom_summarized_globally=construct_global_cad_descriptor(target_data.atom_atom_contact_areas, model_data.atom_atom_contact_areas);
			}
		}

		if(!target_data.residue_residue_contact_areas.empty())
		{
			if(params.record_local_scores_on_residue_level || params.record_local_scores_on_chain_level)
			{
				merge_target_and_model_area_values(target_data.residue_residue_contact_areas, model_data.residue_residue_contact_areas, cadscores_residue_residue);
				cadscores_residue_residue_summarized_globally=construct_global_cad_descriptor(cadscores_residue_residue);
				for(std::map<IDResidueResidue, CADDescriptor>::const_iterator it=cadscores_residue_residue.begin();it!=cadscores_residue_residue.end();++it)
				{
					if(params.record_local_scores_on_residue_level)
					{
						cadscores_residue_residue_summarized_per_residue[it->first.id_a].add(it->second);
						cadscores_residue_residue_summarized_per_residue[it->first.id_b].add(it->second);
					}
					if(params.record_local_scores_on_chain_level)
					{
						cadscores_residue_residue_summarized_per_chain[it->first.id_a.id_chain].add(it->second);
						cadscores_residue_residue_summarized_per_chain[it->first.id_b.id_chain].add(it->second);
						cadscores_residue_residue_summarized_per_chain_chain[IDChainChain(it->first.id_a.id_chain, it->first.id_b.id_chain)].add(it->second);
					}
				}
				if(!params.record_local_scores_on_residue_level)
				{
					cadscores_residue_residue.clear();
				}
			}
			else
			{
				cadscores_residue_residue_summarized_globally=construct_global_cad_descriptor(target_data.residue_residue_contact_areas, model_data.residue_residue_contact_areas);
			}
		}

		if(!target_data.chain_chain_contact_areas.empty())
		{
			if(params.record_local_scores_on_chain_level)
			{
				merge_target_and_model_area_values(target_data.chain_chain_contact_areas, model_data.chain_chain_contact_areas, cadscores_chain_chain);
				cadscores_chain_chain_summarized_globally=construct_global_cad_descriptor(cadscores_chain_chain);
				for(std::map<IDChainChain, CADDescriptor>::const_iterator it=cadscores_chain_chain.begin();it!=cadscores_chain_chain.end();++it)
				{
					cadscores_chain_chain_contact_summarized_per_chain[it->first.id_a].add(it->second);
					cadscores_chain_chain_contact_summarized_per_chain[it->first.id_b].add(it->second);
				}
			}
			else
			{
				cadscores_chain_chain_summarized_globally=construct_global_cad_descriptor(target_data.chain_chain_contact_areas, model_data.chain_chain_contact_areas);
			}
		}

		if(!target_data.atom_sas_areas.empty())
		{
			if(params.record_local_scores_on_atom_level || params.record_local_scores_on_residue_level || params.record_local_scores_on_chain_level)
			{
				merge_target_and_model_area_values(target_data.atom_sas_areas, model_data.atom_sas_areas, cadscores_atom_sas);
				cadscores_atom_sas_summarized_globally=construct_global_cad_descriptor(cadscores_atom_sas);
				if(params.record_local_scores_on_residue_level || params.record_local_scores_on_chain_level)
				{
					for(std::map<IDAtom, CADDescriptor>::const_iterator it=cadscores_atom_sas.begin();it!=cadscores_atom_sas.end();++it)
					{
						if(params.record_local_scores_on_residue_level)
						{
							cadscores_atom_sas_summarized_per_residue[it->first.id_residue].add(it->second);
						}
						if(params.record_local_scores_on_chain_level)
						{
							cadscores_atom_sas_summarized_per_chain[it->first.id_residue.id_chain].add(it->second);
						}
					}
				}
				if(!params.record_local_scores_on_atom_level)
				{
					cadscores_atom_sas.clear();
				}
			}
			else
			{
				cadscores_atom_sas_summarized_globally=construct_global_cad_descriptor(target_data.atom_sas_areas, model_data.atom_sas_areas);
			}
		}

		if(!target_data.residue_sas_areas.empty())
		{
			if(params.record_local_scores_on_residue_level || params.record_local_scores_on_chain_level)
			{
				merge_target_and_model_area_values(target_data.residue_sas_areas, model_data.residue_sas_areas, cadscores_residue_sas);
				cadscores_residue_sas_summarized_globally=construct_global_cad_descriptor(cadscores_residue_sas);
				if(params.record_local_scores_on_chain_level)
				{
					for(std::map<IDResidue, CADDescriptor>::const_iterator it=cadscores_residue_sas.begin();it!=cadscores_residue_sas.end();++it)
					{
						cadscores_residue_sas_summarized_per_chain[it->first.id_chain].add(it->second);
					}
				}
				if(!params.record_local_scores_on_residue_level)
				{
					cadscores_residue_sas.clear();
				}
			}
			else
			{
				cadscores_residue_sas_summarized_globally=construct_global_cad_descriptor(target_data.residue_sas_areas, model_data.residue_sas_areas);
			}
		}

		if(!target_data.chain_sas_areas.empty())
		{
			if(params.record_local_scores_on_chain_level)
			{
				merge_target_and_model_area_values(target_data.chain_sas_areas, model_data.chain_sas_areas, cadscores_chain_sas);
				cadscores_chain_sas_summarized_globally=construct_global_cad_descriptor(cadscores_chain_sas);
			}
			else
			{
				cadscores_chain_sas_summarized_globally=construct_global_cad_descriptor(target_data.chain_sas_areas, model_data.chain_sas_areas);
			}
		}

		if(!target_data.atom_site_areas.empty())
		{
			if(params.record_local_scores_on_atom_level || params.record_local_scores_on_residue_level || params.record_local_scores_on_chain_level)
			{
				merge_target_and_model_area_values(target_data.atom_site_areas, model_data.atom_site_areas, cadscores_atom_site);
				cadscores_atom_site_summarized_globally=construct_global_cad_descriptor(cadscores_atom_site);
				if(params.record_local_scores_on_residue_level || params.record_local_scores_on_chain_level)
				{
					for(std::map<IDAtom, CADDescriptor>::const_iterator it=cadscores_atom_site.begin();it!=cadscores_atom_site.end();++it)
					{
						if(params.record_local_scores_on_residue_level)
						{
							cadscores_atom_site_summarized_per_residue[it->first.id_residue].add(it->second);
						}
						if(params.record_local_scores_on_chain_level)
						{
							cadscores_atom_site_summarized_per_chain[it->first.id_residue.id_chain].add(it->second);
						}
					}
				}
				if(!params.record_local_scores_on_atom_level)
				{
					cadscores_atom_site.clear();
				}
			}
			else
			{
				cadscores_atom_site_summarized_globally=construct_global_cad_descriptor(target_data.atom_site_areas, model_data.atom_site_areas);
			}
		}

		if(!target_data.residue_site_areas.empty())
		{
			if(params.record_local_scores_on_residue_level || params.record_local_scores_on_chain_level)
			{
				merge_target_and_model_area_values(target_data.residue_site_areas, model_data.residue_site_areas, cadscores_residue_site);
				cadscores_residue_site_summarized_globally=construct_global_cad_descriptor(cadscores_residue_site);
				if(params.record_local_scores_on_chain_level)
				{
					for(std::map<IDResidue, CADDescriptor>::const_iterator it=cadscores_residue_site.begin();it!=cadscores_residue_site.end();++it)
					{
						cadscores_residue_site_summarized_per_chain[it->first.id_chain].add(it->second);
					}
				}
				if(!params.record_local_scores_on_residue_level)
				{
					cadscores_residue_site.clear();
				}
			}
			else
			{
				cadscores_residue_site_summarized_globally=construct_global_cad_descriptor(target_data.residue_site_areas, model_data.residue_site_areas);
			}
		}

		if(!target_data.chain_site_areas.empty())
		{
			if(params.record_local_scores_on_chain_level)
			{
				merge_target_and_model_area_values(target_data.chain_site_areas, model_data.chain_site_areas, cadscores_chain_site);
				cadscores_chain_site_summarized_globally=construct_global_cad_descriptor(cadscores_chain_site);
			}
			else
			{
				cadscores_chain_site_summarized_globally=construct_global_cad_descriptor(target_data.chain_site_areas, model_data.chain_site_areas);
			}
		}

		if(params.record_compatible_model_atom_balls)
		{
			compatible_model_atom_balls=model_data.atom_balls;
		}

		valid_=true;

		return true;
	}

private:
	template<class InputContainer, class OutputContainer>
	static void merge_target_and_model_area_values(const InputContainer& target_map, const InputContainer& model_map, OutputContainer& result) noexcept
	{
		result.clear();
		typename InputContainer::const_iterator target_it=target_map.begin();
		typename InputContainer::const_iterator model_it=model_map.begin();
		while(target_it!=target_map.end() || model_it!=model_map.end())
		{
			if(target_it==target_map.end())
			{
				while(model_it!=model_map.end())
				{
					result.emplace_hint(result.end(), model_it->first, CADDescriptor(0.0, model_it->second.area));
					++model_it;
				}
				return;
			}
			else if(model_it==model_map.end())
			{
				while(target_it!=target_map.end())
				{
					result.emplace_hint(result.end(), target_it->first, CADDescriptor(target_it->second.area, 0.0));
					++target_it;
				}
				return;
			}
			else
			{
				if((target_it->first)<(model_it->first))
				{
					result.emplace_hint(result.end(), target_it->first, CADDescriptor(target_it->second.area, 0.0));
					++target_it;
				}
				else if((model_it->first)<(target_it->first))
				{
					result.emplace_hint(result.end(), model_it->first, CADDescriptor(0.0, model_it->second.area));
					++model_it;
				}
				else
				{
					result.emplace_hint(result.end(), target_it->first, CADDescriptor(target_it->second.area, model_it->second.area));
					++target_it;
					++model_it;
				}
			}
		}
	}

	template<class InputContainer>
	static CADDescriptor construct_global_cad_descriptor(const InputContainer& local_cadds) noexcept
	{
		CADDescriptor cadd;
		for(typename InputContainer::const_iterator it=local_cadds.begin();it!=local_cadds.end();++it)
		{
			cadd.add(it->second);
		}
		return cadd;
	}

	template<class InputContainer>
	static CADDescriptor construct_global_cad_descriptor(const InputContainer& target_map, const InputContainer& model_map) noexcept
	{
		CADDescriptor cadd;
		typename InputContainer::const_iterator target_it=target_map.begin();
		typename InputContainer::const_iterator model_it=model_map.begin();
		while(target_it!=target_map.end() || model_it!=model_map.end())
		{
			if(target_it==target_map.end())
			{
				while(model_it!=model_map.end())
				{
					cadd.add(0.0, model_it->second.area);
					++model_it;
				}
				return cadd;
			}
			else if(model_it==model_map.end())
			{
				while(target_it!=target_map.end())
				{
					cadd.add(target_it->second.area, 0.0);
					++target_it;
				}
				return cadd;
			}
			else
			{
				if((target_it->first)<(model_it->first))
				{
					cadd.add(target_it->second.area, 0.0);
					++target_it;
				}
				else if((model_it->first)<(target_it->first))
				{
					cadd.add(0.0, model_it->second.area);
					++model_it;
				}
				else
				{
					cadd.add(target_it->second.area, model_it->second.area);
					++target_it;
					++model_it;
				}
			}
		}
		return cadd;
	}

	static int calculate_number_of_all_permutations_of_chains(const int number_of_chains) noexcept
	{
		int result=1;
		const int max_limit=std::numeric_limits<int>::max();
		for(int i=2;i<=number_of_chains;i++)
		{
			if(result>(max_limit/i))
			{
				return max_limit;
			}
			result*=i;
		}
		return result;
	}

	static int calculate_number_of_relevant_permutations_of_chains(const int number_of_chains, const std::map<int, int>& frequencies_of_reference_sequence_ids) noexcept
	{
		if(frequencies_of_reference_sequence_ids.empty())
		{
			return calculate_number_of_all_permutations_of_chains(number_of_chains);
		}
		int result=1;
		const int max_limit=std::numeric_limits<int>::max();
		for(std::map<int, int>::const_iterator it=frequencies_of_reference_sequence_ids.begin();it!=frequencies_of_reference_sequence_ids.end();++it)
		{
			const int mult=calculate_number_of_all_permutations_of_chains(it->second);
			if(result>(max_limit/mult))
			{
				return max_limit;
			}
			result*=mult;
		}
		return result;
	}

	static bool remap_chains_optimally(const ScorableData& target_data, const ScorableData& model_data, const int max_permutations_to_check_exhaustively, std::map<std::string, std::string>& final_chain_renaming_map, std::ostream& error_log) noexcept
	{
		final_chain_renaming_map.clear();

		if(target_data.residue_residue_contact_areas.empty() || model_data.residue_residue_contact_areas.empty())
		{
			error_log << "Chain remapping was requested, but residue-residue contact areas were not provided.\n";
			return false;
		}

		if(target_data.all_chain_ids.empty())
		{
			error_log << "Chain remapping was requested, but there are no chain names in target.\n";
			return false;
		}

		if(model_data.all_chain_ids.empty())
		{
			error_log << "Chain remapping was requested, but there are no chain names in model.\n";
			return false;
		}

		const std::vector<std::string> chain_names_in_target(target_data.all_chain_ids.begin(), target_data.all_chain_ids.end());
		const std::vector<std::string> chain_names_in_model(model_data.all_chain_ids.begin(), model_data.all_chain_ids.end());

		if(chain_names_in_model.size()==1)
		{
			if(chain_names_in_target.size()==1)
			{
				if(chain_names_in_model[0]==chain_names_in_target[0])
				{
					return true;
				}
				else
				{
					final_chain_renaming_map[chain_names_in_model[0]]=chain_names_in_target[0];
					return true;
				}
			}
			else if(chain_names_in_target.size()>1)
			{
				double best_score=-1.0;
				std::string best_score_chain_name;
				std::map<std::string, std::string> renaming_map;
				std::string& mapped_value=renaming_map[chain_names_in_model[0]];
				for(std::vector<std::string>::const_iterator it=chain_names_in_target.begin();it!=chain_names_in_target.end();++it)
				{
					mapped_value=(*it);
					const bool consistent_with_reference_sequence_ids=target_data.chain_sequences_mapping_result.empty() || model_data.chain_sequences_mapping_result.empty() || (target_data.chain_sequences_mapping_result.get_reference_sequence_id_by_chain_name(mapped_value)==model_data.chain_sequences_mapping_result.get_reference_sequence_id_by_chain_name(chain_names_in_model[0]));
					if(consistent_with_reference_sequence_ids)
					{
						const double score=construct_global_cad_descriptor(target_data.residue_residue_contact_areas, ChainNamingUtilities::rename_chains_in_map_container_with_additive_values(model_data.residue_residue_contact_areas, renaming_map)).score();
						if(score>best_score)
						{
							best_score=score;
							best_score_chain_name=(*it);
						}
					}
				}
				if(!best_score_chain_name.empty())
				{
					final_chain_renaming_map[chain_names_in_model[0]]=best_score_chain_name;
					return true;
				}
				else
				{
					error_log << "Failed to map single chain.\n";
					return false;
				}
			}
		}
		else
		{
			const bool available_chain_sequences_mapping_result=(!target_data.chain_sequences_mapping_result.empty() && !model_data.chain_sequences_mapping_result.empty());
			int num_of_permutations_to_check_exhaustively=0;
			if(!available_chain_sequences_mapping_result)
			{
				num_of_permutations_to_check_exhaustively=std::max(calculate_number_of_all_permutations_of_chains(chain_names_in_model.size()), calculate_number_of_all_permutations_of_chains(chain_names_in_target.size()));
			}
			else
			{
				num_of_permutations_to_check_exhaustively=std::max(calculate_number_of_relevant_permutations_of_chains(chain_names_in_model.size(), model_data.chain_sequences_mapping_result.frequencies_of_reference_sequence_ids), calculate_number_of_relevant_permutations_of_chains(chain_names_in_target.size(), target_data.chain_sequences_mapping_result.frequencies_of_reference_sequence_ids));
			}

			if(num_of_permutations_to_check_exhaustively<=max_permutations_to_check_exhaustively)
			{
				double best_score=-1.0;
				std::map<std::string, std::string> best_score_renaming_map;
				{
					const bool model_not_shorter=(chain_names_in_model.size()>=chain_names_in_target.size());
					std::vector<std::string> permutated_chain_names=(model_not_shorter ? chain_names_in_model : chain_names_in_target);
					const std::vector<std::string>& actual_target_chain_names=(model_not_shorter ? chain_names_in_target : permutated_chain_names);
					const std::vector<std::string>& actual_model_chain_names=(model_not_shorter ? permutated_chain_names : chain_names_in_model);
					do
					{
						bool consistent_with_reference_sequence_ids=true;
						if(available_chain_sequences_mapping_result)
						{
							for(std::size_t i=0;i<actual_target_chain_names.size() && i<actual_model_chain_names.size() && consistent_with_reference_sequence_ids;i++)
							{
								consistent_with_reference_sequence_ids=consistent_with_reference_sequence_ids && target_data.chain_sequences_mapping_result.get_reference_sequence_id_by_chain_name(actual_target_chain_names[i])==model_data.chain_sequences_mapping_result.get_reference_sequence_id_by_chain_name(actual_model_chain_names[i]);
							}
						}
						if(consistent_with_reference_sequence_ids)
						{
							std::map<std::string, std::string> renaming_map=ChainNamingUtilities::generate_renaming_map_from_two_vectors_with_padding(actual_model_chain_names, actual_target_chain_names);
							const double score=construct_global_cad_descriptor(target_data.residue_residue_contact_areas, ChainNamingUtilities::rename_chains_in_map_container_with_additive_values(model_data.residue_residue_contact_areas, renaming_map)).score();
							if(score>best_score)
							{
								best_score=score;
								best_score_renaming_map=renaming_map;
							}
						}
					}
					while(std::next_permutation(permutated_chain_names.begin(), permutated_chain_names.end()));
				}
				if(best_score_renaming_map.empty())
				{
					error_log << "Failed to perform chain remapping exhaustively.\n";
					return false;
				}
				else
				{
					final_chain_renaming_map.swap(best_score_renaming_map);
					return true;
				}
			}
			else
			{
				std::map<std::string, std::string> map_of_renamings_in_model;
				for(std::size_t i=0;i<chain_names_in_model.size();i++)
				{
					map_of_renamings_in_model[chain_names_in_model[i]]=std::string();
				}
				std::map<std::string, std::string> map_of_renamings_in_target;
				for(std::size_t i=0;i<chain_names_in_target.size();i++)
				{
					map_of_renamings_in_target[chain_names_in_target[i]]=std::string();
				}
				std::map<std::string, std::string> reverse_map_of_renamings;
				std::set<std::string> set_of_free_chains_left=model_data.all_chain_ids;
				std::set<std::string> set_of_free_chains_right=target_data.all_chain_ids;
				std::set< std::pair<std::string, std::string> > set_of_hopeless_pairs;
				std::map< std::string, std::set<std::string> > map_of_chain_neighbors=model_data.involved_chain_adjacencies;
				std::map< std::string, std::set<std::string> > map_of_chain_neighbors_in_target=target_data.involved_chain_adjacencies;
				while(!set_of_free_chains_left.empty() && !set_of_free_chains_right.empty())
				{
					std::pair<std::string, std::string> best_pair(*set_of_free_chains_left.begin(), *set_of_free_chains_right.begin());
					double best_score=0.0;
					for(int adjacency_preference_mode=0;adjacency_preference_mode<2 && best_score<=0.0;adjacency_preference_mode++)
					{
						bool prefer_adjacent_chains=(adjacency_preference_mode==0);
						for(std::set<std::string>::const_iterator it_right=set_of_free_chains_right.begin();it_right!=set_of_free_chains_right.end();++it_right)
						{
							bool allowed_right=!prefer_adjacent_chains;
							if(prefer_adjacent_chains)
							{
								const std::set<std::string>& neighbors_of_right=map_of_chain_neighbors_in_target[*it_right];
								for(std::set<std::string>::const_iterator it_neighbors_of_right=neighbors_of_right.begin();it_neighbors_of_right!=neighbors_of_right.end() && !allowed_right;++it_neighbors_of_right)
								{
									allowed_right=set_of_free_chains_right.count(*it_neighbors_of_right)==0;
								}
							}
							if(allowed_right)
							{
								std::set<std::string> neighbors_of_right_as_left;
								if(prefer_adjacent_chains)
								{
									const std::set<std::string>& neighbors_of_right=map_of_chain_neighbors_in_target[*it_right];
									for(std::set<std::string>::const_iterator it_neighbors_of_right=neighbors_of_right.begin();it_neighbors_of_right!=neighbors_of_right.end();++it_neighbors_of_right)
									{
										std::map<std::string, std::string>::const_iterator it_reverse_map_of_renamings=reverse_map_of_renamings.find(*it_neighbors_of_right);
										if(it_reverse_map_of_renamings!=reverse_map_of_renamings.end())
										{
											neighbors_of_right_as_left.insert(it_reverse_map_of_renamings->second);
										}
									}
								}
								std::map<std::string, std::string> new_map_of_renamings_in_target=map_of_renamings_in_target;
								new_map_of_renamings_in_target[*it_right]=(*it_right);
								std::map<IDResidueResidue, AreaValue> new_submap_of_target_contacts=ChainNamingUtilities::rename_chains_in_map_container_with_additive_values(ChainNamingUtilities::restrict_map_container_by_chain_name(target_data.residue_residue_contact_areas, *it_right), new_map_of_renamings_in_target);
								const int right_reference_sequence_id=(available_chain_sequences_mapping_result ? target_data.chain_sequences_mapping_result.get_reference_sequence_id_by_chain_name(*it_right) : -1);
								for(std::set<std::string>::const_iterator it_left=set_of_free_chains_left.begin();it_left!=set_of_free_chains_left.end();++it_left)
								{
									const int left_reference_sequence_id=(available_chain_sequences_mapping_result ? model_data.chain_sequences_mapping_result.get_reference_sequence_id_by_chain_name(*it_left): -1);
									const bool consistent_with_reference_sequence_ids=(!available_chain_sequences_mapping_result || right_reference_sequence_id==left_reference_sequence_id);
									if(consistent_with_reference_sequence_ids && set_of_hopeless_pairs.count(std::make_pair(*it_left, *it_right))==0)
									{
										bool allowed_left=!prefer_adjacent_chains;
										if(prefer_adjacent_chains)
										{
											for(std::set<std::string>::const_iterator it_neighbors_of_right_as_left=neighbors_of_right_as_left.begin();it_neighbors_of_right_as_left!=neighbors_of_right_as_left.end() && !allowed_left;++it_neighbors_of_right_as_left)
											{
												allowed_left=map_of_chain_neighbors[*it_neighbors_of_right_as_left].count(*it_left)>0;
											}
										}
										if(allowed_left)
										{
											std::map<std::string, std::string> new_map_of_renamings=map_of_renamings_in_model;
											new_map_of_renamings[*it_left]=(*it_right);
											const CADDescriptor cad_descriptor=construct_global_cad_descriptor(new_submap_of_target_contacts, ChainNamingUtilities::rename_chains_in_map_container_with_additive_values(ChainNamingUtilities::restrict_map_container_by_chain_name(model_data.residue_residue_contact_areas, *it_left), new_map_of_renamings));
											const double score=cad_descriptor.score()*cad_descriptor.target_area_sum;
											if(score>best_score)
											{
												best_pair=std::make_pair(*it_left, *it_right);
												best_score=score;
											}
											else if(score==0.0)
											{
												set_of_hopeless_pairs.insert(std::make_pair(*it_left, *it_right));
											}
										}
									}
								}
							}
						}
					}
					map_of_renamings_in_model[best_pair.first]=best_pair.second;
					map_of_renamings_in_target[best_pair.second]=best_pair.second;
					reverse_map_of_renamings[best_pair.second]=best_pair.first;
					set_of_free_chains_left.erase(best_pair.first);
					set_of_free_chains_right.erase(best_pair.second);
				}
				final_chain_renaming_map.swap(map_of_renamings_in_model);
				{
					const double final_score=construct_global_cad_descriptor(target_data.residue_residue_contact_areas, ChainNamingUtilities::rename_chains_in_map_container_with_additive_values(model_data.residue_residue_contact_areas, final_chain_renaming_map)).score();
					std::map<std::string, std::string> default_chain_renaming_map=ChainNamingUtilities::generate_renaming_map_from_two_vectors_with_padding(chain_names_in_model, chain_names_in_target);
					const double default_score=construct_global_cad_descriptor(target_data.residue_residue_contact_areas, ChainNamingUtilities::rename_chains_in_map_container_with_additive_values(model_data.residue_residue_contact_areas, default_chain_renaming_map)).score();
					if(default_score>final_score)
					{
						final_chain_renaming_map.swap(default_chain_renaming_map);
					}
				}
				return true;
			}
		}

		error_log << "Failed to perform chain remapping.\n";
		return false;
	}

	bool valid_;
};

class PrintingUtilites
{
public:
	static void print(const IDChain& id, const bool header, const std::string& header_prefix, std::string& output) noexcept
	{
		if(header)
		{
			output+=header_prefix;
			output+="chain";
		}
		else
		{
			output+=id.chain_name;
		}
	}

	static void print(const IDResidue& id, const bool header, const std::string& header_prefix, std::string& output) noexcept
	{
		print(id.id_chain, header, header_prefix, output);
		output+="\t";
		if(header)
		{
			output+=header_prefix;
			output+="rnum\t";
			output+=header_prefix;
			output+="icode\t";
			output+=header_prefix;
			output+="rname";
		}
		else
		{
			output+=std::to_string(id.residue_seq_number);
			output+="\t";
			output+=id.residue_icode.empty() ? std::string(".") : id.residue_icode;
			output+="\t";
			output+=id.residue_name.empty() ? std::string(".") : id.residue_name;
		}
	}

	static void print(const IDAtom& id, const bool header, const std::string& header_prefix, std::string& output) noexcept
	{
		print(id.id_residue, header, header_prefix, output);
		output+="\t";
		if(header)
		{
			output+=header_prefix;
			output+="aname";
		}
		else
		{
			output+=id.atom_name;
		}
	}

	static void print(const IDChainChain& id, const bool header, const std::string& header_prefix, std::string& output) noexcept
	{
		print(id.id_a, header, (header ? header_prefix+"id1_" : header_prefix), output);
		output+="\t";
		print(id.id_b, header, (header ? header_prefix+"id2_" : header_prefix), output);
	}

	static void print(const IDResidueResidue& id, const bool header, const std::string& header_prefix, std::string& output) noexcept
	{
		print(id.id_a, header, (header ? header_prefix+"id1_" : header_prefix), output);
		output+="\t";
		print(id.id_b, header, (header ? header_prefix+"id2_" : header_prefix), output);
	}

	static void print(const IDAtomAtom& id, const bool header, const std::string& header_prefix, std::string& output) noexcept
	{
		print(id.id_a, header, (header ? header_prefix+"id1_" : header_prefix), output);
		output+="\t";
		print(id.id_b, header, (header ? header_prefix+"id2_" : header_prefix), output);
	}

	static void print(const int level_of_details, const CADDescriptor& cadd, const bool header, const std::string& header_prefix, std::string& output) noexcept
	{
		if(header)
		{
			output+=header_prefix;
			output+="cadscore";
			if(level_of_details>=1)
			{
				output+="\t";
				output+=header_prefix;
				output+="F1_of_areas";
				if(level_of_details>=2)
				{
					output+="\t";
					output+=header_prefix;
					output+="target_area\t";
					output+=header_prefix;
					output+="model_area\t";
					output+=header_prefix;
					output+="TP_area\t";
					output+=header_prefix;
					output+="FP_area\t";
					output+=header_prefix;
					output+="FN_area";
				}
			}
		}
		else
		{
			output+=std::to_string(cadd.score());
			if(level_of_details>=1)
			{
				output+="\t";
				output+=std::to_string(cadd.score_F1());
				if(level_of_details>=2)
				{
					output+="\t";
					output+=std::to_string(cadd.target_area_sum);
					output+="\t";
					output+=std::to_string(cadd.model_target_area_sum);
					output+="\t";
					output+=std::to_string(cadd.confusion_TP);
					output+="\t";
					output+=std::to_string(cadd.confusion_FP);
					output+="\t";
					output+=std::to_string(cadd.confusion_FN);
				}
			}
		}
	}

	template<class MapContainer>
	static void print(const int level_of_details, const MapContainer& container, std::string& output) noexcept
	{
		typedef typename MapContainer::key_type ID;
		typedef typename MapContainer::mapped_type Value;
		print(ID(), true, std::string(), output);
		output+="\t";
		print(level_of_details, Value(), true, std::string(), output);
		output+="\n";
		for(typename MapContainer::const_iterator it=container.begin();it!=container.end();++it)
		{
			print(it->first, false, std::string(), output);
			output+="\t";
			print(level_of_details, it->second, false, std::string(), output);
			output+="\n";
		}
	}

	template<class MapContainer>
	static std::string print(const int level_of_details, const MapContainer& container) noexcept
	{
		std::string output;
		print(level_of_details, container, output);
		return output;
	}
};

class MolecularFileWritingUtilities
{
public:
	class PDB
	{
	public:
		static bool check_compatability_with_pdb_format(const std::vector<AtomBall>& atom_balls) noexcept
		{
			if(atom_balls.size()>99999)
			{
				return false;
			}
			for(std::size_t i=0;i<atom_balls.size();i++)
			{
				const AtomBall& ab=atom_balls[i];
				if(ab.id_atom.id_residue.id_chain.chain_name.size()>1)
				{
					return false;
				}
			}
			return true;
		}

		template<class MapContainer>
		static void print(const std::vector<AtomBall>& atom_balls, const MapContainer& map_of_cadds, std::string& output) noexcept
		{
			if(check_compatability_with_pdb_format(atom_balls))
			{
				for(std::size_t i=0;i<atom_balls.size();i++)
				{
					output+=print_atom_line(atom_balls[i], static_cast<int>(i+1), map_of_cadds);
					output+="\n";
				}
			}
		}

		template<class MapContainer>
		static std::string print(const std::vector<AtomBall>& atom_balls, const MapContainer& map_of_cadds) noexcept
		{
			std::string output;
			print(atom_balls, map_of_cadds, output);
			return output;
		}

	private:
		static bool insert_string_to_columned_line(const std::string& str, const std::size_t start, const std::size_t end, const bool shift_right, std::string& line) noexcept
		{
			if(str.empty())
			{
				return true;
			}
			if(start>=1 && start<=end && end<=line.size())
			{
				const std::size_t interval_length=(end-start)+1;
				if(str.size()<=interval_length)
				{
					const std::string addition(interval_length-str.size(), ' ');
					line.replace(start-1, interval_length, (shift_right ? addition+str : str+addition));
					return true;
				}
			}
			return false;
		}

		static std::string print_atom_line(const AtomBall& atom_ball, const int serial, const CADDescriptor& cadd) noexcept
		{
			std::string line(80, ' ');
			insert_string_to_columned_line("ATOM", 1, 6, false, line);
			insert_string_to_columned_line((serial>0 ? std::to_string(serial) : std::string()), 7, 11, true, line);
			insert_string_to_columned_line(atom_ball.id_atom.atom_name, (atom_ball.id_atom.atom_name.size()>3 ? 13 : 14), 16, false, line);
			insert_string_to_columned_line(atom_ball.residue_name, 18, 20, true, line);
			insert_string_to_columned_line(atom_ball.id_atom.id_residue.id_chain.chain_name.substr(0, 1), 22, 22, false, line);
			insert_string_to_columned_line(std::to_string(atom_ball.id_atom.id_residue.residue_seq_number), 23, 26, true, line);
			insert_string_to_columned_line(atom_ball.id_atom.id_residue.residue_icode, 27, 27, false, line);
			insert_string_to_columned_line(convert_double_to_string(atom_ball.x, 3), 31, 38, true, line);
			insert_string_to_columned_line(convert_double_to_string(atom_ball.y, 3), 39, 46, true, line);
			insert_string_to_columned_line(convert_double_to_string(atom_ball.z, 3), 47, 54, true, line);
			insert_string_to_columned_line(convert_double_to_string((cadd.target_area_sum>0.0 ? 1.0 : 0.0), 2), 55, 60, true, line);
			insert_string_to_columned_line(convert_double_to_string((cadd.target_area_sum>0.0 ? cadd.score() : 0.0), 2), 61, 66, true, line);
			return line;
		}

		static std::string print_atom_line(const AtomBall atom_ball, const int serial, const std::map<IDAtom, CADDescriptor>& map_of_cadds) noexcept
		{
			std::map<IDAtom, CADDescriptor>::const_iterator it=map_of_cadds.find(atom_ball.id_atom);
			return print_atom_line(atom_ball, serial, (it==map_of_cadds.end() ? CADDescriptor() : it->second));
		}

		static std::string print_atom_line(const AtomBall atom_ball, const int serial, const std::map<IDResidue, CADDescriptor>& map_of_cadds) noexcept
		{
			std::map<IDResidue, CADDescriptor>::const_iterator it=map_of_cadds.find(atom_ball.id_atom.id_residue);
			return print_atom_line(atom_ball, serial, (it==map_of_cadds.end() ? CADDescriptor() : it->second));
		}

		static std::string print_atom_line(const AtomBall atom_ball, const int serial, const std::map<IDChain, CADDescriptor>& map_of_cadds) noexcept
		{
			std::map<IDChain, CADDescriptor>::const_iterator it=map_of_cadds.find(atom_ball.id_atom.id_residue.id_chain);
			return print_atom_line(atom_ball, serial, (it==map_of_cadds.end() ? CADDescriptor() : it->second));
		}
	};

	class MMCIF
	{
	public:
		template<class MapContainer>
		static void print(const std::vector<AtomBall>& atom_balls, const MapContainer& map_of_cadds, std::string& output) noexcept
		{
			if(!atom_balls.empty())
			{
				output="data_minimal";
				output+=R"(
loop_
_atom_site.group_PDB
_atom_site.id
_atom_site.type_symbol
_atom_site.label_atom_id
_atom_site.label_alt_id
_atom_site.label_comp_id
_atom_site.label_asym_id
_atom_site.label_entity_id
_atom_site.label_seq_id
_atom_site.pdbx_PDB_ins_code
_atom_site.Cartn_x
_atom_site.Cartn_y
_atom_site.Cartn_z
_atom_site.occupancy
_atom_site.B_iso_or_equiv
_atom_site.pdbx_formal_charge
_atom_site.auth_seq_id
_atom_site.auth_comp_id
_atom_site.auth_asym_id
_atom_site.auth_atom_id
_atom_site.pdbx_PDB_model_num
)";
				for(std::size_t i=0;i<atom_balls.size();i++)
				{
					output+=print_atom_line(atom_balls[i], static_cast<int>(i+1), map_of_cadds);
					output+="\n";
				}

				output+="#\n";
			}
		}

		template<class MapContainer>
		static std::string print(const std::vector<AtomBall>& atom_balls, const MapContainer& map_of_cadds) noexcept
		{
			std::string output;
			print(atom_balls, map_of_cadds, output);
			return output;
		}

	private:
		static std::string print_atom_line(const AtomBall& atom_ball, const int serial, const CADDescriptor& cadd) noexcept
		{
			std::string line="ATOM ";
			line+=std::to_string(serial);
			line+=" ? ";
			line+=atom_ball.id_atom.atom_name;
			line+=" . ";
			line+=atom_ball.residue_name;
			line+=" ";
			line+=atom_ball.id_atom.id_residue.id_chain.chain_name;
			line+=" 1 ";
			line+=std::to_string(atom_ball.id_atom.id_residue.residue_seq_number);
			line+=" ";
			line+=(atom_ball.id_atom.id_residue.residue_icode.empty() ? std::string("?") : atom_ball.id_atom.id_residue.residue_icode);
			line+=" ";
			line+=convert_double_to_string(atom_ball.x, 3);
			line+=" ";
			line+=convert_double_to_string(atom_ball.y, 3);
			line+=" ";
			line+=convert_double_to_string(atom_ball.z, 3);
			line+=" ";
			line+=convert_double_to_string((cadd.target_area_sum>0.0 ? 1.0 : 0.0), 2);
			line+=" ";
			line+=convert_double_to_string((cadd.target_area_sum>0.0 ? cadd.score() : 0.0), 2);
			line+=" ? ";
			line+=std::to_string(atom_ball.id_atom.id_residue.residue_seq_number);
			line+=" ";
			line+=atom_ball.residue_name;
			line+=" ";
			line+=atom_ball.id_atom.id_residue.id_chain.chain_name;
			line+=" ";
			line+=atom_ball.id_atom.atom_name;
			line+=" 1";
			return line;
		}

		static std::string print_atom_line(const AtomBall atom_ball, const int serial, const std::map<IDAtom, CADDescriptor>& map_of_cadds) noexcept
		{
			std::map<IDAtom, CADDescriptor>::const_iterator it=map_of_cadds.find(atom_ball.id_atom);
			return print_atom_line(atom_ball, serial, (it==map_of_cadds.end() ? CADDescriptor() : it->second));
		}

		static std::string print_atom_line(const AtomBall atom_ball, const int serial, const std::map<IDResidue, CADDescriptor>& map_of_cadds) noexcept
		{
			std::map<IDResidue, CADDescriptor>::const_iterator it=map_of_cadds.find(atom_ball.id_atom.id_residue);
			return print_atom_line(atom_ball, serial, (it==map_of_cadds.end() ? CADDescriptor() : it->second));
		}

		static std::string print_atom_line(const AtomBall atom_ball, const int serial, const std::map<IDChain, CADDescriptor>& map_of_cadds) noexcept
		{
			std::map<IDChain, CADDescriptor>::const_iterator it=map_of_cadds.find(atom_ball.id_atom.id_residue.id_chain);
			return print_atom_line(atom_ball, serial, (it==map_of_cadds.end() ? CADDescriptor() : it->second));
		}
	};

private:
	static std::string convert_double_to_string(const double value, const int precision)
	{
		std::ostringstream output;
		output << std::fixed << std::setprecision(precision) << value;
		return output.str();
	}
};

class ContactMapPlottingUtilities
{
public:
	static std::string print(const std::map<IDAtomAtom, CADDescriptor>& map_of_cadds) noexcept
	{
		if(map_of_cadds.empty())
		{
			return std::string();
		}
		voronotalt::ContactPlotter plotter(voronotalt::ContactPlotter::LevelMode::inter_atom);
		for(std::map<IDAtomAtom, CADDescriptor>::const_iterator it=map_of_cadds.begin();it!=map_of_cadds.end();++it)
		{
			const IDAtomAtom& id=it->first;
			const CADDescriptor& cadd=it->second;
			if(cadd.target_area_sum>0.0 || cadd.model_area_sum>0.0)
			{
				const double max_value=std::max(cadd.target_area_sum, cadd.model_area_sum);
				plotter.add_contact(id.id_a.id_residue.id_chain.chain_name, id.id_a.id_residue.residue_seq_number, id.id_a.id_residue.residue_icode, id.id_a.atom_name, id.id_b.id_residue.id_chain.chain_name, id.id_b.id_residue.residue_seq_number, id.id_b.id_residue.residue_icode, id.id_b.atom_name, max_value, generate_color(max_value, cadd));
			}
		}
		return plotter.write_to_string(generate_config_flags());
	}

	static std::string print(const std::map<IDResidueResidue, CADDescriptor>& map_of_cadds) noexcept
	{
		if(map_of_cadds.empty())
		{
			return std::string();
		}
		voronotalt::ContactPlotter plotter(voronotalt::ContactPlotter::LevelMode::inter_atom);
		for(std::map<IDResidueResidue, CADDescriptor>::const_iterator it=map_of_cadds.begin();it!=map_of_cadds.end();++it)
		{
			const IDResidueResidue& id=it->first;
			const CADDescriptor& cadd=it->second;
			if(cadd.target_area_sum>0.0 || cadd.model_area_sum>0.0)
			{
				const double max_value=std::max(cadd.target_area_sum, cadd.model_area_sum);
				plotter.add_contact(id.id_a.id_chain.chain_name, id.id_a.residue_seq_number, id.id_a.residue_icode, id.id_b.id_chain.chain_name, id.id_b.residue_seq_number, id.id_b.residue_icode, max_value, generate_color(max_value, cadd));
			}
		}
		return plotter.write_to_string(generate_config_flags());
	}

	static std::string print(const std::map<IDChainChain, CADDescriptor>& map_of_cadds) noexcept
	{
		if(map_of_cadds.empty())
		{
			return std::string();
		}
		voronotalt::ContactPlotter plotter(voronotalt::ContactPlotter::LevelMode::inter_atom);
		for(std::map<IDChainChain, CADDescriptor>::const_iterator it=map_of_cadds.begin();it!=map_of_cadds.end();++it)
		{
			const IDChainChain& id=it->first;
			const CADDescriptor& cadd=it->second;
			if(cadd.target_area_sum>0.0 || cadd.model_area_sum>0.0)
			{
				const double max_value=std::max(cadd.target_area_sum, cadd.model_area_sum);
				plotter.add_contact(id.id_a.chain_name, id.id_b.chain_name, max_value, generate_color(max_value, cadd));
			}
		}
		return plotter.write_to_string(generate_config_flags());
	}

private:
	static voronotalt::ContactPlotter::ConfigFlags generate_config_flags() noexcept
	{
		voronotalt::ContactPlotter::ConfigFlags cf;
		cf.colored=true;
		cf.xlabeled=true;
		cf.ylabeled=true;
		cf.compact=false;
		cf.dark=true;
		cf.valid=true;
		return cf;
	}

	static unsigned int generate_color(const double max_value, const CADDescriptor& cadd) noexcept
	{
		const unsigned int red_value=static_cast<unsigned int>(std::min(cadd.target_area_sum/max_value, 1.0)*255.0);
		const unsigned int green_value=static_cast<unsigned int>(std::min(cadd.model_area_sum/max_value, 1.0)*255.0);
		const unsigned int color=(red_value << 16)+(green_value << 8);
		return color;
	}
};

}

#endif /* CADSCORE_H_ */
