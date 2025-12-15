#ifndef VORONOTALT_H_
#define VORONOTALT_H_

#include <vector>
#include <stdexcept>

#ifndef SWIG
#include "voronotalt/voronotalt.h"
#include "voronotalt_cli/voronotalt_cli.h"
#endif

struct Ball
{
	Ball() : x(0.0), y(0.0), z(0.0), r(0.0)
	{
	}

	Ball(double x, double y, double z, double r) : x(x), y(y), z(z), r(r)
	{
	}

	double x;
	double y;
	double z;
	double r;
};

struct Contact
{
	Contact() : index_a(0), index_b(0), area(0.0), arc_length(0.0)
	{
	}

	int index_a;
	int index_b;
	double area;
	double arc_length;
};

struct Cell
{
	Cell() : sas_area(0.0), volume(0.0), included(false)
	{
	}

	double sas_area;
	double volume;
	bool included;
};

class RadicalTessellation
{
public:
	std::vector<Ball> balls;
	std::vector<Contact> contacts;
	std::vector<Cell> cells;

	RadicalTessellation(const std::vector<Ball>& balls, double probe) : balls(balls)
	{
		recompute(probe);
	}

	int recompute(const double probe)
	{
		contacts.clear();
		cells.clear();

		if(balls.empty())
		{
			throw std::runtime_error("No balls to compute the tessellation for.");
		}

		voronotalt::RadicalTessellation::Result result;
		voronotalt::RadicalTessellation::construct_full_tessellation(voronotalt::get_spheres_from_balls(balls, probe), result);

		if(result.contacts_summaries.empty())
		{
			throw std::runtime_error("No contacts constructed for the provided balls and probe.");
		}

		if(result.cells_summaries.empty())
		{
			throw std::runtime_error("No cells constructed for the provided balls and probe.");
		}

		contacts.resize(result.contacts_summaries.size());
		for(std::size_t i=0;i<result.contacts_summaries.size();i++)
		{
			contacts[i].index_a=result.contacts_summaries[i].id_a;
			contacts[i].index_b=result.contacts_summaries[i].id_b;
			contacts[i].area=result.contacts_summaries[i].area;
			contacts[i].arc_length=result.contacts_summaries[i].arc_length;
		}

		cells.resize(balls.size());
		for(std::size_t i=0;i<result.cells_summaries.size();i++)
		{
			if(result.cells_summaries[i].stage>0)
			{
				const std::size_t index=static_cast<std::size_t>(result.cells_summaries[i].id);
				cells[index].sas_area=result.cells_summaries[i].sas_area;
				cells[index].volume=result.cells_summaries[i].sas_inside_volume;
				cells[index].included=true;
			}
		}

		return static_cast<int>(contacts.size());
	}
};

struct MolecularAtomBall
{
	std::string ID_chain;
	int ID_residue_seq_number;
	std::string ID_residue_icode;
	std::string ID_residue_name;
	std::string ID_atom_name;
	double x;
	double y;
	double z;
	double r;

	MolecularAtomBall() : ID_residue_seq_number(0), x(0.0), y(0.0), z(0.0), r(0.0)
	{
	}

	MolecularAtomBall(
			const std::string& ID_chain,
			const int ID_residue_seq_number,
			const std::string& ID_residue_icode,
			const std::string& ID_residue_name,
			const std::string& ID_atom_name,
			const double x,
			const double y,
			const double z) :
		ID_chain(ID_chain),
		ID_residue_seq_number(ID_residue_seq_number),
		ID_residue_icode(ID_residue_icode),
		ID_residue_name(ID_residue_name),
		ID_atom_name(ID_atom_name),
		x(x),
		y(y),
		z(z),
		r(0.0)
	{
	}
};

struct MolecularInterAtomContactSummary
{
	std::string ID1_chain;
	int ID1_residue_seq_number;
	std::string ID1_residue_icode;
	std::string ID1_residue_name;
	std::string ID1_atom_name;
	std::string ID2_chain;
	int ID2_residue_seq_number;
	std::string ID2_residue_icode;
	std::string ID2_residue_name;
	std::string ID2_atom_name;
	double area;
	double arc_length;

	MolecularInterAtomContactSummary() : ID1_residue_seq_number(0), ID2_residue_seq_number(0), area(0.0), arc_length(0.0)
	{
	}
};

struct MolecularInterResidueContactSummary
{
	std::string ID1_chain;
	int ID1_residue_seq_number;
	std::string ID1_residue_icode;
	std::string ID1_residue_name;
	std::string ID2_chain;
	int ID2_residue_seq_number;
	std::string ID2_residue_icode;
	std::string ID2_residue_name;
	double area;
	double arc_length;

	MolecularInterResidueContactSummary() : ID1_residue_seq_number(0), ID2_residue_seq_number(0), area(0.0), arc_length(0.0)
	{
	}
};

struct MolecularInterChainContactSummary
{
	std::string ID1_chain;
	std::string ID2_chain;
	double area;
	double arc_length;

	MolecularInterChainContactSummary() : area(0.0), arc_length(0.0)
	{
	}
};

struct MolecularAtomCellSummary
{
	std::string ID_chain;
	int ID_residue_seq_number;
	std::string ID_residue_icode;
	std::string ID_residue_name;
	std::string ID_atom_name;
	double sas_area;
	double volume;

	MolecularAtomCellSummary() : ID_residue_seq_number(0), sas_area(0.0), volume(0.0)
	{
	}
};

struct MolecularResidueCellSummary
{
	std::string ID_chain;
	int ID_residue_seq_number;
	std::string ID_residue_icode;
	std::string ID_residue_name;
	double sas_area;
	double volume;

	MolecularResidueCellSummary() : ID_residue_seq_number(0), sas_area(0.0), volume(0.0)
	{
	}
};

struct MolecularChainCellSummary
{
	std::string ID_chain;
	double sas_area;
	double volume;

	MolecularChainCellSummary() : sas_area(0.0), volume(0.0)
	{
	}
};

struct MolecularFileInput
{
	std::string input_file_path;
	bool include_heteroatoms;
	bool read_as_assembly;

	MolecularFileInput() :
		include_heteroatoms(true),
		read_as_assembly(false)
	{
	}

	explicit MolecularFileInput(const std::string& input_file) :
		input_file_path(input_file),
		include_heteroatoms(true),
		read_as_assembly(false)
	{
	}
};

struct MolecularRadicalTessellationParameters
{
	std::string restrict_input_atoms;
	std::string restrict_contacts_to_construct;
	std::string restrict_contacts_for_output;
	std::string restrict_cells_for_output;
	double probe;
	bool compute_only_inter_chain_contacts;
	bool compute_only_inter_residue_contacts;
	bool record_atom_balls;
	bool record_inter_atom_contact_summaries;
	bool record_inter_residue_contact_summaries;
	bool record_inter_chain_contact_summaries;
	bool record_atom_cell_summaries;
	bool record_residue_cell_summaries;
	bool record_chain_cell_summaries;
	bool record_everything_possible;

	MolecularRadicalTessellationParameters() :
		probe(1.4),
		compute_only_inter_chain_contacts(false),
		compute_only_inter_residue_contacts(false),
		record_atom_balls(false),
		record_inter_atom_contact_summaries(false),
		record_inter_residue_contact_summaries(false),
		record_inter_chain_contact_summaries(false),
		record_atom_cell_summaries(false),
		record_residue_cell_summaries(false),
		record_chain_cell_summaries(false),
		record_everything_possible(true)
	{
	}
};

class MolecularRadicalTessellation
{
public:
	MolecularRadicalTessellationParameters params;
	std::vector<MolecularAtomBall> atom_balls;
	std::vector<MolecularInterAtomContactSummary> inter_atom_contact_summaries;
	std::vector<MolecularInterResidueContactSummary> inter_residue_contact_summaries;
	std::vector<MolecularInterChainContactSummary> inter_chain_contact_summaries;
	std::vector<MolecularAtomCellSummary> atom_cell_summaries;
	std::vector<MolecularResidueCellSummary> residue_cell_summaries;
	std::vector<MolecularChainCellSummary> chain_cell_summaries;

	MolecularRadicalTessellation(const std::string& input_file)
	{
		reconstruct(std::vector<MolecularAtomBall>(), MolecularFileInput(input_file), MolecularRadicalTessellationParameters());
	}

	MolecularRadicalTessellation(const std::vector<MolecularAtomBall>& input_atom_balls)
	{
		reconstruct(input_atom_balls, MolecularFileInput(), MolecularRadicalTessellationParameters());
	}

	MolecularRadicalTessellation(const MolecularFileInput& molecular_file_input, const MolecularRadicalTessellationParameters& init_params)
	{
		reconstruct(std::vector<MolecularAtomBall>(), molecular_file_input, init_params);
	}

	MolecularRadicalTessellation(const std::vector<MolecularAtomBall>& input_atom_balls, const MolecularRadicalTessellationParameters& init_params)
	{
		reconstruct(input_atom_balls, MolecularFileInput(), init_params);
	}

private:
	void reconstruct(const std::vector<MolecularAtomBall>& input_atom_balls, const MolecularFileInput& molecular_file_input, const MolecularRadicalTessellationParameters& init_params)
	{
		params=init_params;
		atom_balls.clear();
		inter_atom_contact_summaries.clear();
		inter_residue_contact_summaries.clear();
		inter_chain_contact_summaries.clear();
		atom_cell_summaries.clear();
		residue_cell_summaries.clear();
		chain_cell_summaries.clear();

		if(input_atom_balls.empty() && molecular_file_input.input_file_path.empty())
		{
			throw std::runtime_error("No input source provided.");
		}

		if(!input_atom_balls.empty() && !molecular_file_input.input_file_path.empty())
		{
			throw std::runtime_error("Conflicted sources of input provided.");
		}

		const bool restricted_construction_of_contacts=(
				params.compute_only_inter_chain_contacts
				|| params.compute_only_inter_residue_contacts
				|| !(params.restrict_contacts_to_construct.empty() || params.restrict_contacts_to_construct=="[]"));

		if(params.record_everything_possible)
		{
			params.record_atom_balls=true;
			params.record_inter_atom_contact_summaries=true;
			params.record_inter_residue_contact_summaries=true;
			params.record_inter_chain_contact_summaries=true;
			params.record_atom_cell_summaries=!restricted_construction_of_contacts;
			params.record_residue_cell_summaries=!restricted_construction_of_contacts;
			params.record_chain_cell_summaries=!restricted_construction_of_contacts;
		}

		if(restricted_construction_of_contacts && (params.record_atom_cell_summaries || params.record_residue_cell_summaries || params.record_chain_cell_summaries))
		{
			throw std::runtime_error("Requested recording of cell summaries is not allowed when the construction of contacts is restricted.");
		}

		voronotalt::FilteringBySphereLabels::ExpressionForSingle filtering_expression_for_restricting_input_atoms;
		voronotalt::FilteringBySphereLabels::ExpressionForPair filtering_expression_for_restricting_contacts_to_construct;
		voronotalt::FilteringBySphereLabels::ExpressionForPair filtering_expression_for_restricting_contacts_for_output;
		voronotalt::FilteringBySphereLabels::ExpressionForSingle filtering_expression_for_restricting_cells_for_output;

		if(!params.restrict_input_atoms.empty())
		{
			filtering_expression_for_restricting_input_atoms=voronotalt::FilteringBySphereLabels::ExpressionForSingle(params.restrict_input_atoms);
			if(!filtering_expression_for_restricting_input_atoms.valid())
			{
				throw std::runtime_error("Invalid input atoms filtering expression");
			}
		}

		if(!params.restrict_contacts_to_construct.empty())
		{
			filtering_expression_for_restricting_contacts_to_construct=voronotalt::FilteringBySphereLabels::ExpressionForPair(params.restrict_contacts_to_construct);
			if(!filtering_expression_for_restricting_contacts_to_construct.valid())
			{
				throw std::runtime_error("Invalid filtering expression for restricting contacts to construct");
			}
		}

		if(!params.restrict_contacts_for_output.empty())
		{
			filtering_expression_for_restricting_contacts_for_output=voronotalt::FilteringBySphereLabels::ExpressionForPair(params.restrict_contacts_for_output);
			if(!filtering_expression_for_restricting_contacts_for_output.valid())
			{
				throw std::runtime_error("Invalid filtering expression for restricting contacts for output");
			}
		}

		if(!params.restrict_cells_for_output.empty())
		{
			filtering_expression_for_restricting_cells_for_output=voronotalt::FilteringBySphereLabels::ExpressionForSingle(params.restrict_cells_for_output);
			if(!filtering_expression_for_restricting_cells_for_output.valid())
			{
				throw std::runtime_error("Invalid filtering expression for restricting cells for output");
			}
		}

		voronotalt::TimeRecorder time_recorder;

		voronotalt::SpheresInput::Result spheres_input_result;

		if(!input_atom_balls.empty())
		{
			voronotalt::MolecularFileReading::Data mol_data;
			mol_data.atom_records.resize(input_atom_balls.size());
			for(std::size_t i=0;i<input_atom_balls.size();i++)
			{
				const MolecularAtomBall& ab=input_atom_balls[i];
				voronotalt::MolecularFileReading::AtomRecord& ar=mol_data.atom_records[i];
				ar.chainID=ab.ID_chain;
				ar.resSeq=ab.ID_residue_seq_number;
				ar.iCode=ab.ID_residue_icode;
				ar.resName=ab.ID_residue_name;
				ar.name=ab.ID_atom_name;
				ar.x=ab.x;
				ar.y=ab.y;
				ar.z=ab.z;
			}
			if(!voronotalt::SpheresInput::read_labeled_spheres_from_molecular_data_descriptor(mol_data, params.probe, true, spheres_input_result)
					|| spheres_input_result.spheres.size()!=input_atom_balls.size() || spheres_input_result.sphere_labels.size()!=input_atom_balls.size())
			{
				throw std::runtime_error("Failed to process input vector of atom balls.");
			}
			for(std::size_t i=0;i<input_atom_balls.size();i++)
			{
				const MolecularAtomBall& ab=input_atom_balls[i];
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
				throw std::runtime_error(std::string("Failed to open file '")+molecular_file_input.input_file_path+"'");
			}

			if(input_data.empty())
			{
				throw std::runtime_error(std::string("No data in file '")+molecular_file_input.input_file_path+"'");
			}

			const voronotalt::MolecularFileReading::Parameters molecular_file_reading_parameters(molecular_file_input.include_heteroatoms, false, molecular_file_input.read_as_assembly);

			std::ostringstream err_stream;

			if(!voronotalt::SpheresInput::read_labeled_or_unlabeled_spheres_from_string(input_data, molecular_file_reading_parameters, params.probe, spheres_input_result, err_stream, time_recorder))
			{
				throw std::runtime_error(std::string("Failed to parse file '")+molecular_file_input.input_file_path+"': "+err_stream.str());
			}
		}

		if(spheres_input_result.sphere_labels.size()!=spheres_input_result.spheres.size())
		{
			throw std::runtime_error(std::string("Input is not in a recognized molecular data format."));
		}

		if(!filtering_expression_for_restricting_input_atoms.allow_all())
		{
			voronotalt::FilteringBySphereLabels::VectorExpressionResult ver=filtering_expression_for_restricting_input_atoms.filter_vector(spheres_input_result.sphere_labels);
			if(!ver.expression_valid)
			{
				throw std::runtime_error(std::string("Restricting input by applying filtering expression failed"));
			}
			if(!ver.expression_matched())
			{
				throw std::runtime_error(std::string("No input satisfied restricting filtering expression"));
			}
			if(!spheres_input_result.restrict_spheres(ver.expression_matched_all, ver.expression_matched_ids))
			{
				throw std::runtime_error(std::string("Failed to restrict input"));
			}
		}

		voronotalt::RadicalTessellation::Result result;

		{
			const voronotalt::PeriodicBox periodic_box;

			voronotalt::RadicalTessellation::ResultGraphics result_graphics;

			const std::vector<int> null_grouping;
			const std::vector<int>& grouping_for_filtering=(params.compute_only_inter_chain_contacts ? spheres_input_result.grouping_by_chain : (params.compute_only_inter_residue_contacts ? spheres_input_result.grouping_by_residue : null_grouping));

			voronotalt::SpheresContainer spheres_container;
			spheres_container.init(spheres_input_result.spheres, periodic_box, time_recorder);

			voronotalt::SpheresContainer::ResultOfPreparationForTessellation preparation_result;
			spheres_container.prepare_for_tessellation(grouping_for_filtering, preparation_result, time_recorder);

			if(!filtering_expression_for_restricting_contacts_to_construct.allow_all())
			{
				const voronotalt::FilteringBySphereLabels::VectorExpressionResult ver=filtering_expression_for_restricting_contacts_to_construct.filter_vector(spheres_input_result.sphere_labels, spheres_input_result.spheres, preparation_result.relevant_collision_ids);
				if(!ver.expression_matched() || !preparation_result.restrict_relevant_collision_ids(ver.expression_matched_all, ver.expression_matched_ids))
				{
					throw std::runtime_error(std::string("Failed to restrict contacts for construction"));
				}
			}

			const bool summarize_cells=!preparation_result.collision_ids_constrained;

			const bool with_tessellation_net=false;
			const bool with_graphics=false;
			const bool with_sas_graphics_if_possible=false;

			voronotalt::RadicalTessellation::construct_full_tessellation(
					spheres_container,
					preparation_result,
					with_tessellation_net,
					voronotalt::RadicalTessellation::ParametersForGraphics(with_graphics, with_sas_graphics_if_possible),
					voronotalt::RadicalTessellation::ParametersForGeneratingSummaries(false, summarize_cells),
					result,
					result_graphics,
					time_recorder);

			if(!filtering_expression_for_restricting_contacts_for_output.allow_all())
			{
				const voronotalt::FilteringBySphereLabels::VectorExpressionResult ver=filtering_expression_for_restricting_contacts_for_output.filter_vector(spheres_input_result.sphere_labels, spheres_input_result.spheres, voronotalt::FilteringBySphereLabels::ExpressionForPair::adapt_indices_container(result.contacts_summaries));
				if(!ver.expression_matched() || !voronotalt::RadicalTessellation::restrict_result_contacts(ver.expression_matched_all, ver.expression_matched_ids, result, result_graphics))
				{
					throw std::runtime_error(std::string("Failed to restrict contacts for output"));
				}
			}

			if(!result.cells_summaries.empty() && !filtering_expression_for_restricting_cells_for_output.allow_all())
			{
				const voronotalt::FilteringBySphereLabels::VectorExpressionResult ver=filtering_expression_for_restricting_cells_for_output.filter_vector(spheres_input_result.sphere_labels);
				if(!ver.expression_matched() || !voronotalt::RadicalTessellation::restrict_result_cells(ver.expression_matched_all, ver.expression_matched_ids, result, result_graphics))
				{
					throw std::runtime_error(std::string("Failed to restrict cells for output"));
				}
			}
		}

		voronotalt::RadicalTessellation::GroupedResult result_grouped_by_residue;

		if(params.record_inter_residue_contact_summaries || params.record_residue_cell_summaries)
		{
			voronotalt::RadicalTessellation::group_results(result, spheres_input_result.grouping_by_residue, result_grouped_by_residue, time_recorder);
		}

		voronotalt::RadicalTessellation::GroupedResult result_grouped_by_chain;

		if(params.record_inter_chain_contact_summaries || params.record_chain_cell_summaries)
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

				MolecularAtomBall& mab=atom_balls[i];
				mab.ID_chain=sl.chain_id;
				mab.ID_residue_seq_number=sl.expanded_residue_id.rnum;
				mab.ID_residue_icode=sl.expanded_residue_id.icode;
				mab.ID_residue_name=sl.expanded_residue_id.rname;
				mab.ID_atom_name=sl.atom_name;
				mab.x=s.p.x;
				mab.y=s.p.y;
				mab.z=s.p.z;
				mab.r=s.r-params.probe;
			}
		}

		if(params.record_inter_atom_contact_summaries)
		{
			inter_atom_contact_summaries.resize(result.contacts_summaries.size());
			for(std::size_t i=0;i<result.contacts_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::ContactDescriptorSummary& cds=result.contacts_summaries[i];
				const voronotalt::SphereLabeling::SphereLabel& sl1=spheres_input_result.sphere_labels[cds.id_a];
				const voronotalt::SphereLabeling::SphereLabel& sl2=spheres_input_result.sphere_labels[cds.id_b];
				MolecularInterAtomContactSummary& miacs=inter_atom_contact_summaries[i];
				miacs.ID1_chain=sl1.chain_id;
				miacs.ID1_residue_seq_number=sl1.expanded_residue_id.rnum;
				miacs.ID1_residue_icode=sl1.expanded_residue_id.icode;
				miacs.ID1_residue_name=sl1.expanded_residue_id.rname;
				miacs.ID1_atom_name=sl1.atom_name;
				miacs.ID2_chain=sl2.chain_id;
				miacs.ID2_residue_seq_number=sl2.expanded_residue_id.rnum;
				miacs.ID2_residue_icode=sl2.expanded_residue_id.icode;
				miacs.ID2_residue_name=sl2.expanded_residue_id.rname;
				miacs.ID2_atom_name=sl2.atom_name;
				miacs.area=cds.area;
				miacs.arc_length=cds.arc_length;
			}
		}

		if(params.record_atom_cell_summaries)
		{
			atom_cell_summaries.resize(result.cells_summaries.size());
			for(std::size_t i=0;i<result.cells_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::CellContactDescriptorsSummary& ccds=result.cells_summaries[i];
				const voronotalt::SphereLabeling::SphereLabel& sl=spheres_input_result.sphere_labels[ccds.id];
				MolecularAtomCellSummary& macs=atom_cell_summaries[i];
				macs.ID_chain=sl.chain_id;
				macs.ID_residue_seq_number=sl.expanded_residue_id.rnum;
				macs.ID_residue_icode=sl.expanded_residue_id.icode;
				macs.ID_residue_name=sl.expanded_residue_id.rname;
				macs.ID_atom_name=sl.atom_name;
				macs.sas_area=ccds.sas_area;
				macs.volume=ccds.sas_inside_volume;
			}
		}

		if(params.record_inter_residue_contact_summaries)
		{
			inter_residue_contact_summaries.resize(result_grouped_by_residue.grouped_contacts_summaries.size());
			for(std::size_t i=0;i<result_grouped_by_residue.grouped_contacts_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::ContactDescriptorSummary& cds=result.contacts_summaries[result_grouped_by_residue.grouped_contacts_representative_ids[i]];
				const voronotalt::RadicalTessellation::TotalContactDescriptorsSummary& tcds=result_grouped_by_residue.grouped_contacts_summaries[i];
				const voronotalt::SphereLabeling::SphereLabel& sl1=spheres_input_result.sphere_labels[cds.id_a];
				const voronotalt::SphereLabeling::SphereLabel& sl2=spheres_input_result.sphere_labels[cds.id_b];
				MolecularInterResidueContactSummary& mircs=inter_residue_contact_summaries[i];
				mircs.ID1_chain=sl1.chain_id;
				mircs.ID1_residue_seq_number=sl1.expanded_residue_id.rnum;
				mircs.ID1_residue_icode=sl1.expanded_residue_id.icode;
				mircs.ID1_residue_name=sl1.expanded_residue_id.rname;
				mircs.ID2_chain=sl2.chain_id;
				mircs.ID2_residue_seq_number=sl2.expanded_residue_id.rnum;
				mircs.ID2_residue_icode=sl2.expanded_residue_id.icode;
				mircs.ID2_residue_name=sl2.expanded_residue_id.rname;
				mircs.area=tcds.area;
				mircs.arc_length=tcds.arc_length;
			}
		}

		if(params.record_residue_cell_summaries)
		{
			residue_cell_summaries.resize(result_grouped_by_residue.grouped_cells_summaries.size());
			for(std::size_t i=0;i<result_grouped_by_residue.grouped_cells_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::CellContactDescriptorsSummary& ccds=result.cells_summaries[result_grouped_by_residue.grouped_cells_representative_ids[i]];
				const voronotalt::RadicalTessellation::TotalCellContactDescriptorsSummary& tccds=result_grouped_by_residue.grouped_cells_summaries[i];
				const voronotalt::SphereLabeling::SphereLabel& sl=spheres_input_result.sphere_labels[ccds.id];
				MolecularResidueCellSummary& mrcs=residue_cell_summaries[i];
				mrcs.ID_chain=sl.chain_id;
				mrcs.ID_residue_seq_number=sl.expanded_residue_id.rnum;
				mrcs.ID_residue_icode=sl.expanded_residue_id.icode;
				mrcs.ID_residue_name=sl.expanded_residue_id.rname;
				mrcs.sas_area=tccds.sas_area;
				mrcs.volume=tccds.sas_inside_volume;
			}
		}

		if(params.record_inter_chain_contact_summaries)
		{
			inter_chain_contact_summaries.resize(result_grouped_by_chain.grouped_contacts_summaries.size());
			for(std::size_t i=0;i<result_grouped_by_chain.grouped_contacts_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::ContactDescriptorSummary& cds=result.contacts_summaries[result_grouped_by_chain.grouped_contacts_representative_ids[i]];
				const voronotalt::RadicalTessellation::TotalContactDescriptorsSummary& tcds=result_grouped_by_chain.grouped_contacts_summaries[i];
				const voronotalt::SphereLabeling::SphereLabel& sl1=spheres_input_result.sphere_labels[cds.id_a];
				const voronotalt::SphereLabeling::SphereLabel& sl2=spheres_input_result.sphere_labels[cds.id_b];
				MolecularInterChainContactSummary& miccs=inter_chain_contact_summaries[i];
				miccs.ID1_chain=sl1.chain_id;
				miccs.ID2_chain=sl2.chain_id;
				miccs.area=tcds.area;
				miccs.arc_length=tcds.arc_length;
			}
		}

		if(params.record_chain_cell_summaries)
		{
			chain_cell_summaries.resize(result_grouped_by_chain.grouped_cells_summaries.size());
			for(std::size_t i=0;i<result_grouped_by_chain.grouped_cells_summaries.size();i++)
			{
				const voronotalt::RadicalTessellation::CellContactDescriptorsSummary& ccds=result.cells_summaries[result_grouped_by_chain.grouped_cells_representative_ids[i]];
				const voronotalt::RadicalTessellation::TotalCellContactDescriptorsSummary& tccds=result_grouped_by_chain.grouped_cells_summaries[i];
				const voronotalt::SphereLabeling::SphereLabel& sl=spheres_input_result.sphere_labels[ccds.id];
				MolecularChainCellSummary& mccs=chain_cell_summaries[i];
				mccs.ID_chain=sl.chain_id;
				mccs.sas_area=tccds.sas_area;
				mccs.volume=tccds.sas_inside_volume;
			}
		}
	}
};

void reset_molecular_radii_assignment_rules()
{
	voronotalt::MolecularRadiiAssignment::clear_radius_value_rules();
}

void configure_molecular_radii_assignment_rules(const std::string& configuration_file_path)
{
	std::string input_data;

	if(!voronotalt::read_whole_file_or_pipe_or_stdin_to_string(configuration_file_path, input_data))
	{
		throw std::runtime_error(std::string("Failed to open file '")+configuration_file_path+"'");
	}

	if(input_data.empty())
	{
		throw std::runtime_error(std::string("No data in file '")+configuration_file_path+"'");
	}

	if(!voronotalt::MolecularRadiiAssignment::set_radius_value_rules(input_data))
	{
		throw std::runtime_error(std::string("Invalid atom radii configuration file '")+configuration_file_path+"'");
	}
}

#endif /* VORONOTALT_H_ */
