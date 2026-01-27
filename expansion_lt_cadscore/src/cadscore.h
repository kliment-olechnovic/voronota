#ifndef CADSCORE_H_
#define CADSCORE_H_

#include <string>
#include <map>

#include "../../expansion_lt/src/voronotalt/parallelization_configuration.h"
#include "../../expansion_lt/src/voronotalt/voronotalt.h"
#include "../../expansion_lt/src/voronotalt_cli/voronotalt_cli.h"

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

		ConstructionParameters() noexcept :
			probe(1.4),
			record_atom_balls(false),
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

	ConstructionParameters params;
	std::vector<AtomBall> atom_balls;
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
		params=ConstructionParameters();
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

	bool construct(const ConstructionParameters& init_params, const MolecularFileInput& molecular_file_input, std::ostream& error_log) noexcept
	{
		return construct(init_params, std::vector<AtomBall>(), molecular_file_input, error_log);
	}

	bool construct(const ConstructionParameters& init_params, const std::vector<AtomBall>& input_atom_balls, std::ostream& error_log) noexcept
	{
		return construct(init_params, input_atom_balls, MolecularFileInput(), error_log);
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

	bool construct(const ConstructionParameters& init_params, const std::vector<AtomBall>& input_atom_balls, const MolecularFileInput& molecular_file_input, std::ostream& error_log) noexcept
	{
		clear();

		OnReturnGuard on_return_guard(*this);

		if(!init_params.valid())
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

		params=init_params;

		const bool need_to_summarize_cells=(params.record_atom_cell_summaries || params.record_residue_cell_summaries || params.record_chain_cell_summaries);
		const bool need_to_summarize_sites=(params.record_atom_site_summaries || params.record_residue_site_summaries || params.record_chain_site_summaries);

		const bool can_restrict_contacts_on_construction=!need_to_summarize_cells;
		const bool compute_only_inter_chain_contacts=(can_restrict_contacts_on_construction && !params.record_atom_atom_contact_summaries && !params.record_residue_residue_contact_summaries && !params.record_atom_site_summaries && !params.record_residue_site_summaries);
		const bool compute_only_inter_residue_contacts=(can_restrict_contacts_on_construction && !params.record_atom_atom_contact_summaries && !params.record_atom_site_summaries);

		voronotalt::TimeRecorder time_recorder;

		voronotalt::SpheresInput::Result spheres_input_result;

		if(!input_atom_balls.empty())
		{
			voronotalt::MolecularFileReading::Data mol_data;
			mol_data.atom_records.resize(input_atom_balls.size());
			for(std::size_t i=0;i<input_atom_balls.size();i++)
			{
				input_atom_balls[i].write_into_atom_record(mol_data.atom_records[i]);
			}
			if(!voronotalt::SpheresInput::read_labeled_spheres_from_molecular_data_descriptor(mol_data, params.probe, true, spheres_input_result) || spheres_input_result.spheres.size()!=input_atom_balls.size() || spheres_input_result.sphere_labels.size()!=input_atom_balls.size())
			{
				error_log << "Failed to process input vector of atom balls.\n";
				return false;
			}
			for(std::size_t i=0;i<input_atom_balls.size();i++)
			{
				const AtomBall& ab=input_atom_balls[i];
				if(ab.r>0.0)
				{
					spheres_input_result.spheres[i].r=ab.r+params.probe;
				}
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

		if(params.record_atom_balls)
		{
			atom_balls.resize(spheres_input_result.spheres.size());
			for(std::size_t i=0;i<spheres_input_result.spheres.size();i++)
			{
				const voronotalt::SimpleSphere& s=spheres_input_result.spheres[i];
				const voronotalt::SphereLabeling::SphereLabel& sl=spheres_input_result.sphere_labels[i];

				AtomBall& mab=atom_balls[i];
				mab.id_atom=IDAtom(sl);
				mab.residue_name=sl.expanded_residue_id.rname;
				mab.x=s.p.x;
				mab.y=s.p.y;
				mab.z=s.p.z;
				mab.r=s.r-params.probe;
			}
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
		bool record_local_scores;
		int max_chains_to_fully_permute;
		std::map<std::string, std::string> chain_renaming_map;

		ConstructionParameters() noexcept : remap_chains(false), record_local_scores(false), max_chains_to_fully_permute(5)
		{
		}
	};

	ConstructionParameters params;

	std::map<IDAtomAtom, CADDescriptor> atom_atom_contact_cad_descriptors;
	std::map<IDAtom, CADDescriptor> atom_atom_contact_cad_descriptors_per_atom;
	CADDescriptor atom_atom_contact_cad_descriptor_global;

	std::map<IDResidueResidue, CADDescriptor> residue_residue_contact_cad_descriptors;
	std::map<IDResidue, CADDescriptor> residue_residue_contact_cad_descriptors_per_residue;
	CADDescriptor residue_residue_contact_cad_descriptor_global;

	std::map<IDChainChain, CADDescriptor> chain_chain_contact_cad_descriptors;
	std::map<IDChain, CADDescriptor> chain_chain_contact_cad_descriptors_per_chain;
	CADDescriptor chain_chain_contact_cad_descriptor_global;

	std::map<IDAtom, CADDescriptor> atom_sas_cad_descriptors;
	CADDescriptor atom_sas_cad_descriptor_global;

	std::map<IDResidue, CADDescriptor> residue_sas_cad_descriptors;
	CADDescriptor residue_sas_cad_descriptor_global;

	std::map<IDChain, CADDescriptor> chain_sas_cad_descriptors;
	CADDescriptor chain_sas_cad_descriptor_global;

	std::map<IDAtom, CADDescriptor> atom_site_cad_descriptors;
	CADDescriptor atom_site_cad_descriptor_global;

	std::map<IDResidue, CADDescriptor> residue_site_cad_descriptors;
	CADDescriptor residue_site_cad_descriptor_global;

	std::map<IDChain, CADDescriptor> chain_site_cad_descriptors;
	CADDescriptor chain_site_cad_descriptor_global;

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

		atom_atom_contact_cad_descriptors.clear();
		atom_atom_contact_cad_descriptors_per_atom.clear();
		atom_atom_contact_cad_descriptor_global=CADDescriptor();

		residue_residue_contact_cad_descriptors.clear();
		residue_residue_contact_cad_descriptors_per_residue.clear();
		residue_residue_contact_cad_descriptor_global=CADDescriptor();

		chain_chain_contact_cad_descriptors.clear();
		chain_chain_contact_cad_descriptors_per_chain.clear();
		chain_chain_contact_cad_descriptor_global=CADDescriptor();

		atom_sas_cad_descriptors.clear();
		atom_sas_cad_descriptor_global=CADDescriptor();

		residue_sas_cad_descriptors.clear();
		residue_sas_cad_descriptor_global=CADDescriptor();

		chain_sas_cad_descriptors.clear();
		chain_sas_cad_descriptor_global=CADDescriptor();

		atom_site_cad_descriptors.clear();
		atom_site_cad_descriptor_global=CADDescriptor();

		residue_site_cad_descriptors.clear();
		residue_site_cad_descriptor_global=CADDescriptor();

		chain_site_cad_descriptors.clear();
		chain_site_cad_descriptor_global=CADDescriptor();
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
				if(!remap_chains_optimally(target_data, model_data, adjusted_init_params.max_chains_to_fully_permute, adjusted_init_params.chain_renaming_map, error_log))
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
			if(params.record_local_scores)
			{
				merge_target_and_model_area_values(target_data.atom_atom_contact_areas, model_data.atom_atom_contact_areas, atom_atom_contact_cad_descriptors);
				atom_atom_contact_cad_descriptor_global=construct_global_cad_descriptor(atom_atom_contact_cad_descriptors);
				for(std::map<IDAtomAtom, CADDescriptor>::const_iterator it=atom_atom_contact_cad_descriptors.begin();it!=atom_atom_contact_cad_descriptors.end();++it)
				{
					atom_atom_contact_cad_descriptors_per_atom[it->first.id_a].add(it->second);
					atom_atom_contact_cad_descriptors_per_atom[it->first.id_b].add(it->second);
				}
			}
			else
			{
				atom_atom_contact_cad_descriptor_global=construct_global_cad_descriptor(target_data.atom_atom_contact_areas, model_data.atom_atom_contact_areas);
			}
		}

		if(!target_data.residue_residue_contact_areas.empty())
		{
			if(params.record_local_scores)
			{
				merge_target_and_model_area_values(target_data.residue_residue_contact_areas, model_data.residue_residue_contact_areas, residue_residue_contact_cad_descriptors);
				residue_residue_contact_cad_descriptor_global=construct_global_cad_descriptor(residue_residue_contact_cad_descriptors);
				for(std::map<IDResidueResidue, CADDescriptor>::const_iterator it=residue_residue_contact_cad_descriptors.begin();it!=residue_residue_contact_cad_descriptors.end();++it)
				{
					residue_residue_contact_cad_descriptors_per_residue[it->first.id_a].add(it->second);
					residue_residue_contact_cad_descriptors_per_residue[it->first.id_b].add(it->second);
				}
			}
			else
			{
				residue_residue_contact_cad_descriptor_global=construct_global_cad_descriptor(target_data.residue_residue_contact_areas, model_data.residue_residue_contact_areas);
			}
		}

		if(!target_data.chain_chain_contact_areas.empty())
		{
			if(params.record_local_scores)
			{
				merge_target_and_model_area_values(target_data.chain_chain_contact_areas, model_data.chain_chain_contact_areas, chain_chain_contact_cad_descriptors);
				chain_chain_contact_cad_descriptor_global=construct_global_cad_descriptor(chain_chain_contact_cad_descriptors);
				for(std::map<IDChainChain, CADDescriptor>::const_iterator it=chain_chain_contact_cad_descriptors.begin();it!=chain_chain_contact_cad_descriptors.end();++it)
				{
					chain_chain_contact_cad_descriptors_per_chain[it->first.id_a].add(it->second);
					chain_chain_contact_cad_descriptors_per_chain[it->first.id_b].add(it->second);
				}
			}
			else
			{
				chain_chain_contact_cad_descriptor_global=construct_global_cad_descriptor(target_data.chain_chain_contact_areas, model_data.chain_chain_contact_areas);
			}
		}

		if(!target_data.atom_sas_areas.empty())
		{
			if(params.record_local_scores)
			{
				merge_target_and_model_area_values(target_data.atom_sas_areas, model_data.atom_sas_areas, atom_sas_cad_descriptors);
				atom_sas_cad_descriptor_global=construct_global_cad_descriptor(atom_sas_cad_descriptors);
			}
			else
			{
				atom_sas_cad_descriptor_global=construct_global_cad_descriptor(target_data.atom_sas_areas, model_data.atom_sas_areas);
			}
		}

		if(!target_data.residue_sas_areas.empty())
		{
			if(params.record_local_scores)
			{
				merge_target_and_model_area_values(target_data.residue_sas_areas, model_data.residue_sas_areas, residue_sas_cad_descriptors);
				residue_sas_cad_descriptor_global=construct_global_cad_descriptor(residue_sas_cad_descriptors);
			}
			else
			{
				residue_sas_cad_descriptor_global=construct_global_cad_descriptor(target_data.residue_sas_areas, model_data.residue_sas_areas);
			}
		}

		if(!target_data.chain_sas_areas.empty())
		{
			if(params.record_local_scores)
			{
				merge_target_and_model_area_values(target_data.chain_sas_areas, model_data.chain_sas_areas, chain_sas_cad_descriptors);
				chain_sas_cad_descriptor_global=construct_global_cad_descriptor(chain_sas_cad_descriptors);
			}
			else
			{
				chain_sas_cad_descriptor_global=construct_global_cad_descriptor(target_data.chain_sas_areas, model_data.chain_sas_areas);
			}
		}

		if(!target_data.atom_site_areas.empty())
		{
			if(params.record_local_scores)
			{
				merge_target_and_model_area_values(target_data.atom_site_areas, model_data.atom_site_areas, atom_site_cad_descriptors);
				atom_site_cad_descriptor_global=construct_global_cad_descriptor(atom_site_cad_descriptors);
			}
			else
			{
				atom_site_cad_descriptor_global=construct_global_cad_descriptor(target_data.atom_site_areas, model_data.atom_site_areas);
			}
		}

		if(!target_data.residue_site_areas.empty())
		{
			if(params.record_local_scores)
			{
				merge_target_and_model_area_values(target_data.residue_site_areas, model_data.residue_site_areas, residue_site_cad_descriptors);
				residue_site_cad_descriptor_global=construct_global_cad_descriptor(residue_site_cad_descriptors);
			}
			else
			{
				residue_site_cad_descriptor_global=construct_global_cad_descriptor(target_data.residue_site_areas, model_data.residue_site_areas);
			}
		}

		if(!target_data.chain_site_areas.empty())
		{
			if(params.record_local_scores)
			{
				merge_target_and_model_area_values(target_data.chain_site_areas, model_data.chain_site_areas, chain_site_cad_descriptors);
				chain_site_cad_descriptor_global=construct_global_cad_descriptor(chain_site_cad_descriptors);
			}
			else
			{
				chain_site_cad_descriptor_global=construct_global_cad_descriptor(target_data.chain_site_areas, model_data.chain_site_areas);
			}
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

	static bool remap_chains_optimally(const ScorableData& target_data, const ScorableData& model_data, const int max_chains_to_fully_permute, std::map<std::string, std::string>& final_chain_renaming_map, std::ostream& error_log) noexcept
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
					const double score=construct_global_cad_descriptor(target_data.residue_residue_contact_areas, ChainNamingUtilities::rename_chains_in_map_container_with_additive_values(model_data.residue_residue_contact_areas, renaming_map)).score();
					if(score>best_score)
					{
						best_score=score;
						best_score_chain_name=(*it);
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
		else if(max_chains_to_fully_permute>1 && max_chains_to_fully_permute<7 && chain_names_in_model.size()<=static_cast<std::size_t>(max_chains_to_fully_permute) && chain_names_in_target.size()<=static_cast<std::size_t>(max_chains_to_fully_permute))
		{
			double best_score=-1.0;
			std::map<std::string, std::string> best_score_renaming_map=ChainNamingUtilities::generate_renaming_map_from_two_vectors_with_padding(chain_names_in_model, chain_names_in_target);
			{
				const bool model_not_shorter=(chain_names_in_model.size()>=chain_names_in_target.size());
				std::vector<std::string> permutated_chain_names=(model_not_shorter ? chain_names_in_model : chain_names_in_target);
				do
				{
					std::map<std::string, std::string> renaming_map=(model_not_shorter ?
							ChainNamingUtilities::generate_renaming_map_from_two_vectors_with_padding(permutated_chain_names, chain_names_in_target) :
							ChainNamingUtilities::generate_renaming_map_from_two_vectors_with_padding(chain_names_in_model, permutated_chain_names));
					const double score=construct_global_cad_descriptor(target_data.residue_residue_contact_areas, ChainNamingUtilities::rename_chains_in_map_container_with_additive_values(model_data.residue_residue_contact_areas, renaming_map)).score();
					if(score>best_score)
					{
						best_score=score;
						best_score_renaming_map=renaming_map;
					}
				}
				while(std::next_permutation(permutated_chain_names.begin(), permutated_chain_names.end()));
			}
			final_chain_renaming_map.swap(best_score_renaming_map);
			return true;
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
							for(std::set<std::string>::const_iterator it_left=set_of_free_chains_left.begin();it_left!=set_of_free_chains_left.end();++it_left)
							{
								if(set_of_hopeless_pairs.count(std::make_pair(*it_left, *it_right))==0)
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
};

}

#endif /* CADSCORE_H_ */
