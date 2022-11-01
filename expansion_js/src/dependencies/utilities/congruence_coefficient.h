#ifndef DEPENDENCIES_UTILITIES_CONGRUENCE_COEFFICIENT_H_
#define DEPENDENCIES_UTILITIES_CONGRUENCE_COEFFICIENT_H_

#include <map>

namespace congruence_coefficient
{

typedef std::pair<std::size_t, std::size_t> Adjacency;
typedef std::map<Adjacency, double> AdjacencyMap;

double calculate_congruence_coefficient_of_two_square_symmetric_matrices(const AdjacencyMap& adjacency_map1, const AdjacencyMap& adjacency_map2);

}

#endif /* DEPENDENCIES_UTILITIES_CONGRUENCE_COEFFICIENT_H_ */
