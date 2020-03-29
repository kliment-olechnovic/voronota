#ifndef NNPORT_PREDICT_FOR_VECTOR_H_
#define NNPORT_PREDICT_FOR_VECTOR_H_

#include <sstream>

#include <fdeep/fdeep.hpp>

#include "read_vector.h"

namespace voronota
{

namespace nnport
{

inline std::vector<float> predict_for_vector(const fdeep::model& model, const std::vector<float>& input_vector)
{
	const fdeep::shared_float_vec shared_input_vector(fplus::make_shared_ref<fdeep::float_vec>(input_vector));
	fdeep::tensor input_tensor(fdeep::tensor_shape(input_vector.size()), shared_input_vector);
	const fdeep::tensors result=model.predict({input_tensor});
	return *(result.at(0).as_vector());
}

inline std::vector<float> predict_for_vector(const std::vector<fdeep::model>& models, const std::vector<float>& input_vector)
{
	std::vector<float> result;
	for(std::size_t i=0;i<models.size();i++)
	{
		const std::vector<float> subresult=predict_for_vector(models[i], input_vector);
		result.insert(result.end(), subresult.begin(), subresult.end());
	}
	return result;
}

inline void silent_logger(const std::string&)
{
}

}

}

#endif /* NNPORT_PREDICT_FOR_VECTOR_H_ */

