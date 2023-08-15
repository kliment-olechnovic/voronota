#ifndef SCRIPTING_OPERATORS_DELETE_FIGURES_OF_LABELS_H_
#define SCRIPTING_OPERATORS_DELETE_FIGURES_OF_LABELS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class DeleteFiguresOfLabels : public OperatorBase<DeleteFiguresOfLabels>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		Result()
		{
		}

		void store(HeterogeneousStorage&) const
		{
		}
	};

	SelectionManager::Query parameters_for_selecting_atoms;
	std::string mode;
	std::vector<std::string> figure_name_start;

	DeleteFiguresOfLabels()
	{
	}

	void initialize(CommandInput& input)
	{
		parameters_for_selecting_atoms=OperatorsUtilities::read_generic_selecting_query(input);
		mode=input.get_value_or_default<std::string>("mode", "residue");
		std::vector<std::string> default_figure_name_start;
		default_figure_name_start.push_back("label");
		default_figure_name_start.push_back(mode);
		figure_name_start=input.get_value_vector_or_default<std::string>("figure-name-start", default_figure_name_start);
	}

	void document(CommandDocumentation& doc) const
	{
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("figure-name", CDOD::DATATYPE_STRING_ARRAY, "figure name", "'label' 'mode value'"));
		doc.set_option_decription(CDOD("mode", CDOD::DATATYPE_STRING, "labeling mode, atom or residue", "atom"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();
		data_manager.assert_figures_availability();

		if(mode!="atom" && mode!="residue")
		{
			throw std::runtime_error(std::string("Invalid mode, must be 'atom' or 'residue'"));
		}

		if(figure_name_start.empty())
		{
			throw std::runtime_error(std::string("No figure name start provided."));
		}

		const std::set<std::size_t> atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting_atoms);
		if(atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		const std::map< std::string, std::vector<std::size_t> > map_of_ids=data_manager.generate_ids_for_of_labels(atom_ids, true, (mode=="atom"));

		std::set<std::size_t> all_figure_ids;

		for(std::map< std::string, std::vector<std::size_t> >::const_iterator it=map_of_ids.begin();it!=map_of_ids.end();++it)
		{
			const std::set<std::size_t> figure_ids=LongName::match(data_manager.figures(), LongName(figure_name_start, it->first));
			all_figure_ids.insert(figure_ids.begin(), figure_ids.end());
		}

		if(all_figure_ids.empty())
		{
			throw std::runtime_error(std::string("No figures selected."));
		}

		data_manager.remove_figures(all_figure_ids);

		Result result;

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_DELETE_FIGURES_OF_LABELS_H_ */
