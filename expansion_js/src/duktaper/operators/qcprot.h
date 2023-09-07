#ifndef DUKTAPER_OPERATORS_QCPROT_H_
#define DUKTAPER_OPERATORS_QCPROT_H_

#include "../../dependencies/qcprot/qcprot_wrapper.h"

#include "../../../../src/scripting/operators/move_atoms.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class QCProt : public scripting::OperatorBase<QCProt>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		std::string target_name;
		std::string model_name;
		double rmsd;
		std::size_t number_of_atoms;

		Result() : rmsd(0.0), number_of_atoms(0)
		{
		}

		void store(scripting::VariantObject& variant_object) const
		{
			variant_object.value("target_name")=target_name;
			variant_object.value("model_name")=model_name;
			variant_object.value("rmsd")=rmsd;
			variant_object.value("number_of_atoms")=number_of_atoms;
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			store(heterostorage.variant_object);
		}

		bool operator<(const Result& r) const
		{
			return ((rmsd<r.rmsd) || (rmsd==r.rmsd && number_of_atoms>r.number_of_atoms)
					|| (rmsd==r.rmsd && number_of_atoms==r.number_of_atoms && target_name<r.target_name)
					|| (rmsd==r.rmsd && number_of_atoms==r.number_of_atoms && target_name==r.target_name && model_name<r.model_name));
		}
	};

	std::string target_name;
	std::string model_name;
	std::string target_selection;
	std::string model_selection;
	bool no_move;
	bool verify_atom_ids;

	QCProt() : no_move(false), verify_atom_ids(false)
	{
	}

	void initialize(scripting::CommandInput& input, const bool managed)
	{
		if(managed)
		{
			target_name=input.get_value_or_first_unused_unnamed_value_or_default("target", "");
		}
		else
		{
			target_name=input.get_value_or_first_unused_unnamed_value("target");
			model_name=input.get_value_or_first_unused_unnamed_value("model");
		}
		target_selection=input.get_value_or_default<std::string>("target-sel", "");
		model_selection=input.get_value_or_default<std::string>("model-sel", "");
		no_move=input.get_flag("no-move");
		verify_atom_ids=input.get_flag("verify-atom-ids");
	}

	void initialize(scripting::CommandInput& input)
	{
		initialize(input, false);
	}

	void document(scripting::CommandDocumentation& doc, const bool managed) const
	{
		if(managed)
		{
			doc.set_option_decription(CDOD("target", CDOD::DATATYPE_STRING, "name of target object", ""));
		}
		else
		{
			doc.set_option_decription(CDOD("target", CDOD::DATATYPE_STRING, "name of target object"));
			doc.set_option_decription(CDOD("model", CDOD::DATATYPE_STRING, "name of model object"));
		}
		doc.set_option_decription(CDOD("target-sel", CDOD::DATATYPE_STRING, "selection of atoms for target object", ""));
		doc.set_option_decription(CDOD("model-sel", CDOD::DATATYPE_STRING, "selection of atoms for model object", ""));
		doc.set_option_decription(CDOD("no-move", CDOD::DATATYPE_BOOL, "flag to not transform any coordinates"));
		doc.set_option_decription(CDOD("verify-atom-ids", CDOD::DATATYPE_BOOL, "flag to verify matching of atom IDs using residue numbers and atom names"));
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		document(doc, false);
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(target_name.empty())
		{
			throw std::runtime_error(std::string("No target object name provided."));
		}

		if(model_name.empty())
		{
			throw std::runtime_error(std::string("No model object name provided."));
		}

		if(target_name==model_name)
		{
			throw std::runtime_error(std::string("Equal object names provided."));
		}

		congregation_of_data_managers.assert_object_availability(target_name);
		congregation_of_data_managers.assert_object_availability(model_name);

		scripting::DataManager* target_object=congregation_of_data_managers.get_object(target_name);
		scripting::DataManager* model_object=congregation_of_data_managers.get_object(model_name);

		scripting::DataManager& target_dm=(*target_object);
		scripting::DataManager& model_dm=(*model_object);

		const std::string target_use=target_selection.empty() ? std::string("([])") : (std::string("(")+target_selection+")");
		const std::string model_use=model_selection.empty() ? std::string("([])") : (std::string("(")+model_selection+")");

		const std::set<std::size_t> target_atom_ids_set=target_dm.selection_manager().select_atoms(scripting::SelectionManager::Query(target_use, false));
		if(target_atom_ids_set.empty())
		{
			throw std::runtime_error(std::string("No target atoms selected."));
		}

		const std::set<std::size_t> model_atom_ids_set=model_dm.selection_manager().select_atoms(scripting::SelectionManager::Query(model_use, false));
		if(model_atom_ids_set.empty())
		{
			throw std::runtime_error(std::string("No model atoms selected."));
		}

		std::vector<std::size_t> target_atom_ids;
		std::vector<std::size_t> model_atom_ids;

		if(!verify_atom_ids)
		{
			target_atom_ids.insert(target_atom_ids.end(), target_atom_ids_set.begin(), target_atom_ids_set.end());
			model_atom_ids.insert(model_atom_ids.end(), model_atom_ids_set.begin(), model_atom_ids_set.end());
		}
		else
		{
			std::map< common::ChainResidueAtomDescriptor, std::vector<std::size_t> > crads_map;
			for(std::set<std::size_t>::const_iterator it=target_atom_ids_set.begin();it!=target_atom_ids_set.end();++it)
			{
				const scripting::Atom& atom=target_dm.atoms()[*it];
				common::ChainResidueAtomDescriptor crad;
				crad.resSeq=atom.crad.resSeq;
				crad.name=atom.crad.name;
				crads_map[crad].push_back(*it);
			}
			for(std::set<std::size_t>::const_iterator it=model_atom_ids_set.begin();it!=model_atom_ids_set.end();++it)
			{
				const scripting::Atom& atom=model_dm.atoms()[*it];
				common::ChainResidueAtomDescriptor crad;
				crad.resSeq=atom.crad.resSeq;
				crad.name=atom.crad.name;
				crads_map[crad].push_back(*it);
			}
			target_atom_ids.reserve(crads_map.size());
			model_atom_ids.reserve(crads_map.size());
			for(std::map< common::ChainResidueAtomDescriptor, std::vector<std::size_t> >::const_iterator it=crads_map.begin();it!=crads_map.end();++it)
			{
				if(it->second.size()==2)
				{
					target_atom_ids.push_back(it->second[0]);
					model_atom_ids.push_back(it->second[1]);
				}
			}
		}

		if(target_atom_ids.size()!=model_atom_ids.size())
		{
			throw std::runtime_error(std::string("Mismatched numbers of selected target and model atoms."));
		}

		const std::size_t N=target_atom_ids.size();

		std::vector< std::vector<double> > xyz_coords_target(3, std::vector<double>(N, 0.0));
		std::vector< std::vector<double> > xyz_coords_model(3, std::vector<double>(N, 0.0));

		for(std::size_t i=0;i<N;i++)
		{
			const scripting::Atom& atom_target=target_dm.atoms()[target_atom_ids[i]];
			xyz_coords_target[0][i]=atom_target.value.x;
			xyz_coords_target[1][i]=atom_target.value.y;
			xyz_coords_target[2][i]=atom_target.value.z;

			const scripting::Atom& atom_model=model_dm.atoms()[model_atom_ids[i]];
			xyz_coords_model[0][i]=atom_model.value.x;
			xyz_coords_model[1][i]=atom_model.value.y;
			xyz_coords_model[2][i]=atom_model.value.z;
		}

		QCProtWrapper::QCProtResult qcprot_result;

		if(!QCProtWrapper::run_qcprot(xyz_coords_target, xyz_coords_model, qcprot_result))
		{
			throw std::runtime_error(std::string("Failed to run QCProt."));
		}

		if(!no_move)
		{
			scripting::DataManager::TransformationOfCoordinates transformation;
			transformation.pre_translation_vector=std::vector<double>(qcprot_result.translation_vector_b, qcprot_result.translation_vector_b+3);
			transformation.rotation_matrix=std::vector<double>(qcprot_result.rotation_matrix, qcprot_result.rotation_matrix+9);
			transformation.post_translation_vector=std::vector<double>(qcprot_result.translation_vector_a, qcprot_result.translation_vector_a+3);

			model_dm.transform_coordinates_of_atoms(model_dm.selection_manager().select_atoms(scripting::SelectionManager::Query()), transformation);
		}

		Result result;
		result.target_name=target_name;
		result.model_name=model_name;
		result.rmsd=qcprot_result.rmsd;
		result.number_of_atoms=N;

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_QCPROT_H_ */
