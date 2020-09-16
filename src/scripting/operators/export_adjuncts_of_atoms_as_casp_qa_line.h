#ifndef SCRIPTING_OPERATORS_EXPORT_ADJUNCTS_OF_ATOMS_AS_CASP_QA_LINE_H_
#define SCRIPTING_OPERATORS_EXPORT_ADJUNCTS_OF_ATOMS_AS_CASP_QA_LINE_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ExportAdjunctsOfAtomsAsCASPQALine : public OperatorBase<ExportAdjunctsOfAtomsAsCASPQALine>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		SummaryOfAtoms atoms_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
		}
	};

	std::string file;
	SelectionManager::Query parameters_for_selecting;
	std::string adjunct;
	std::string title;
	double global_score;
	int sequence_length;
	double scale_by_completeness;
	int wrap;

	ExportAdjunctsOfAtomsAsCASPQALine() : global_score(0.0), sequence_length(0), scale_by_completeness(1.0), wrap(20)
	{
	}

	void initialize(CommandInput& input)
	{
		file=input.get_value<std::string>("file");
		assert_file_name_input(file, false);
		parameters_for_selecting=OperatorsUtilities::read_generic_selecting_query(input);
		adjunct=input.get_value<std::string>("adjunct");
		title=input.get_value<std::string>("title");
		global_score=input.get_value<double>("global-score");
		sequence_length=input.get_value<int>("sequence-length");
		scale_by_completeness=input.get_value<double>("scale-by-completeness");
		wrap=input.get_value<int>("wrap");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		OperatorsUtilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("adjunct", CDOD::DATATYPE_STRING, "adjunct name"));
		doc.set_option_decription(CDOD("title", CDOD::DATATYPE_STRING, "line start title"));
		doc.set_option_decription(CDOD("global-score", CDOD::DATATYPE_FLOAT, "global score"));
		doc.set_option_decription(CDOD("sequence-length", CDOD::DATATYPE_INT, "sequence length"));
		doc.set_option_decription(CDOD("scale-by-completeness", CDOD::DATATYPE_FLOAT, "max completeness for scaling"));
		doc.set_option_decription(CDOD("wrap", CDOD::DATATYPE_INT, "word wrap threshold"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_file_name_input(file, false);

		if(adjunct.empty())
		{
			throw std::runtime_error(std::string("Adjunct name not specified."));
		}

		if(title.empty())
		{
			throw std::runtime_error(std::string("Title not specified."));
		}

		if(global_score<0.0 || global_score>1.0)
		{
			throw std::runtime_error(std::string("Invalid global score."));
		}

		if(sequence_length<1)
		{
			throw std::runtime_error(std::string("Invalid sequence length."));
		}

		if(scale_by_completeness<0.0 || scale_by_completeness>1.0)
		{
			throw std::runtime_error(std::string("Invalid completeness threshold."));
		}

		std::set<std::size_t> atom_ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(atom_ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		std::map<int, double> map_of_scores;
		for(std::set<std::size_t>::const_iterator it=atom_ids.begin();it!=atom_ids.end();++it)
		{
			const Atom& atom=data_manager.atoms()[*it];
			if(atom.crad.resSeq<=sequence_length && atom.crad.resSeq>0)
			{
				std::map<std::string, double>::const_iterator jt=atom.value.props.adjuncts.find(adjunct);
				if(jt!=atom.value.props.adjuncts.end())
				{
					map_of_scores[atom.crad.resSeq]=jt->second;
				}
			}
		}

		const double completeness=static_cast<double>(map_of_scores.size())/static_cast<double>(sequence_length);
		const double rescaled_global_score=((completeness<scale_by_completeness) ? global_score*completeness : global_score);

		OutputSelector output_selector(file);
		std::ostream& output=output_selector.stream();
		assert_io_stream(file, output);

		output << title << " ";
		output.precision(3);
		output << std::fixed << rescaled_global_score;
		for(int i=1;i<=sequence_length;i++)
		{
			output << ((wrap>1 && i>1 && ((i-1)%wrap==0)) ? "\t" : " ");
			std::map<int, double>::const_iterator it=map_of_scores.find(i);
			if(it!=map_of_scores.end())
			{
				output.precision(2);
				output << std::fixed << (it->second);
			}
			else
			{
				output << "X";
			}
		}
		output << "\n";

		Result result;
		result.atoms_summary=SummaryOfAtoms(data_manager.atoms(), atom_ids);

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_ADJUNCTS_OF_ATOMS_AS_CASP_QA_LINE_H_ */
