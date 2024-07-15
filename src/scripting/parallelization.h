#ifndef SCRIPTING_PARALLELIZATION_H_
#define SCRIPTING_PARALLELIZATION_H_

#ifdef _OPENMP
#include <omp.h>
#endif

namespace voronota
{

namespace scripting
{

class Parallelization
{
public:
	class Configuration
	{
	public:
		int processors;
		bool dynamic_adjustment;
		bool in_script;

		Configuration() : processors(1), dynamic_adjustment(false), in_script(false)
		{
		}

		static const Configuration& get_default_configuration()
		{
			return get_default_configuration_mutable();
		}

		static void setup_default_configuration(const Configuration& configuration)
		{
			if(possible())
			{
				Configuration checked_configuration=configuration;
				if(checked_configuration.processors<2)
				{
					checked_configuration=Configuration();
				}
				get_default_configuration_mutable()=checked_configuration;
			}
			else
			{
				get_default_configuration_mutable()=Configuration();
			}
		}

	private:
		static Configuration& get_default_configuration_mutable()
		{
			static Configuration configuration;
			return configuration;
		}
	};


#ifdef _OPENMP
	static bool possible()
	{
		return true;
	}

	static void apply_configuration(const Configuration& configuration)
	{
		omp_set_num_threads(configuration.processors);
		omp_set_dynamic(configuration.dynamic_adjustment ? 1 : 0);
		omp_set_nested(0);
	}
#else
	static bool possible()
	{
		return false;
	}

	static void apply_configuration(const Configuration& /*configuration*/)
	{
	}
#endif

	static void setup_and_apply_default_configuration(const Configuration& new_configuration)
	{
		Configuration::setup_default_configuration(new_configuration);
		apply_configuration(Configuration::get_default_configuration());
	}
};

}

}


#endif /* SCRIPTING_PARALLELIZATION_H_ */
