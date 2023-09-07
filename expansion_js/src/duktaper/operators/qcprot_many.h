#ifndef DUKTAPER_OPERATORS_QCPROT_MANY_H_
#define DUKTAPER_OPERATORS_QCPROT_MANY_H_

#include "qcprot.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class QCProtMany : public scripting::OperatorBase<QCProtMany>
{
public:
	struct Result : public scripting::OperatorResultBase<Result>
	{
		std::vector<QCProt::Result> qcprot_results;

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			if(!qcprot_results.empty())
			{
				double global_sum_of_rmsd=0.0;
				std::map<std::string, std::pair<double, double> > per_target_sums_of_rmsd;
				for(std::size_t i=0;i<qcprot_results.size();i++)
				{
					std::vector<scripting::VariantObject>& voa=heterostorage.variant_object.objects_array("qcprot_results");
					voa.push_back(scripting::VariantObject());
					qcprot_results[i].store(voa.back());
					global_sum_of_rmsd+=qcprot_results[i].rmsd;
					per_target_sums_of_rmsd[qcprot_results[i].target_name].first+=qcprot_results[i].rmsd;
					per_target_sums_of_rmsd[qcprot_results[i].target_name].second+=1.0;
				}
				std::map<std::string, std::pair<double, double> >::iterator it_of_min_mean_rmsd=per_target_sums_of_rmsd.begin();
				for(std::map<std::string, std::pair<double, double> >::iterator it=per_target_sums_of_rmsd.begin();it!=per_target_sums_of_rmsd.end();++it)
				{
					it_of_min_mean_rmsd=((it->second.first/it->second.second)<(it_of_min_mean_rmsd->second.first/it_of_min_mean_rmsd->second.second) ? it : it_of_min_mean_rmsd);
				}
				scripting::VariantObject& vo_summary=heterostorage.variant_object.object("qcprot_results_summary");
				vo_summary.value("global_mean_rmsd_value")=global_sum_of_rmsd/static_cast<double>(qcprot_results.size());
				vo_summary.value("min_mean_rmsd_value")=(it_of_min_mean_rmsd->second.first/it_of_min_mean_rmsd->second.second);
				vo_summary.value("min_mean_rmsd_target")=it_of_min_mean_rmsd->first;
			}
		}
	};

	scripting::CongregationOfDataManagers::ObjectQuery query;
	QCProt qcprot_operator;
	bool all_to_all;
	std::string output_rmsd_file;

	QCProtMany() : all_to_all(false)
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		query=scripting::OperatorsUtilities::read_congregation_of_data_managers_object_query(input);
		qcprot_operator.initialize(input, true);
		all_to_all=input.get_flag("all-to-all");
		output_rmsd_file=input.get_value_or_default<std::string>("output-rmsd-file", "");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		scripting::OperatorsUtilities::document_read_congregation_of_data_managers_object_query(doc);
		qcprot_operator.document(doc, true);
		doc.set_option_decription(CDOD("all-to-all", CDOD::DATATYPE_BOOL, "flag to analyze all pairs and to not transform any coordinates"));
		doc.set_option_decription(CDOD("output-rmsd-file", CDOD::DATATYPE_STRING, "path to output RMSD values", ""));
	}

	Result run(scripting::CongregationOfDataManagers& congregation_of_data_managers) const
	{
		if(all_to_all && !qcprot_operator.target_name.empty())
		{
			throw std::runtime_error(std::string("Target name and all-to-all enabled together."));
		}

		const std::vector<scripting::DataManager*> all_objects=congregation_of_data_managers.get_objects();

		if(all_objects.size()<2)
		{
			throw std::runtime_error(std::string("Less than two objects overall available."));
		}

		const std::vector<scripting::DataManager*> objects=congregation_of_data_managers.get_objects(query);

		if(objects.empty())
		{
			throw std::runtime_error(std::string("No objects selected."));
		}

		QCProt qcprot_operator_to_use=qcprot_operator;
		qcprot_operator_to_use.no_move=(qcprot_operator_to_use.no_move || all_to_all);

		Result result;

		if(all_to_all)
		{
			for(std::size_t a=0;a<objects.size();a++)
			{
				qcprot_operator_to_use.target_name=congregation_of_data_managers.get_object_attributes(objects[a]).name;
				for(std::size_t b=0;b<objects.size();b++)
				{
					if(a!=b)
					{
						qcprot_operator_to_use.model_name=congregation_of_data_managers.get_object_attributes(objects[b]).name;
						result.qcprot_results.push_back(qcprot_operator_to_use.run(congregation_of_data_managers));
					}
				}
			}
		}
		else
		{
			qcprot_operator_to_use.target_name=(qcprot_operator.target_name.empty() ? congregation_of_data_managers.get_object_attributes(all_objects[0]).name : qcprot_operator.target_name);
			congregation_of_data_managers.assert_object_availability(qcprot_operator_to_use.target_name);
			for(std::size_t i=0;i<objects.size();i++)
			{
				const scripting::CongregationOfDataManagers::ObjectAttributes attributes=congregation_of_data_managers.get_object_attributes(objects[i]);
				if(attributes.name!=qcprot_operator_to_use.target_name)
				{
					qcprot_operator_to_use.model_name=attributes.name;
					result.qcprot_results.push_back(qcprot_operator_to_use.run(congregation_of_data_managers));
				}
			}
		}

		if(result.qcprot_results.empty())
		{
			throw std::runtime_error(std::string("No valid pairs of objects."));
		}

		std::sort(result.qcprot_results.begin(), result.qcprot_results.end());
		std::reverse(result.qcprot_results.begin(), result.qcprot_results.end());

		if(!output_rmsd_file.empty())
		{
			scripting::OutputSelector output_selector(output_rmsd_file);
			std::ostream& output=output_selector.stream();
			scripting::assert_io_stream(output_rmsd_file, output);

			for(std::size_t i=0;i<result.qcprot_results.size();i++)
			{
				output << result.qcprot_results[i].target_name << " " << result.qcprot_results[i].model_name
						<< " " << result.qcprot_results[i].rmsd << " " << result.qcprot_results[i].number_of_atoms << "\n";
			}
		}

		return result;
	}
};

}

}

}

#endif /* DUKTAPER_OPERATORS_QCPROT_MANY_H_ */

