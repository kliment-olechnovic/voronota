#ifndef SCRIPTING_OPERATORS_BASE_H_
#define SCRIPTING_OPERATORS_BASE_H_

#include "command_input.h"
#include "heterogeneous_storage.h"
#include "json_writer.h"
#include "command_documentation.h"

namespace voronota
{

namespace scripting
{

template<class Operator>
class OperatorBase
{
public:
	OperatorBase()
	{
	}

	virtual ~OperatorBase()
	{
	}

	Operator& init(CommandInput& input)
	{
		initialize(input);
		input.assert_nothing_unusable();
		return underlying();
	}

	Operator& init()
	{
		CommandInput input;
		return init(input);
	}

	CommandDocumentation documentation() const
	{
		CommandDocumentation doc;
		document(doc);
		return doc;
	}

	virtual void initialize(CommandInput&)
	{
	}

	virtual void document(CommandDocumentation&) const
	{
	}

private:
	Operator& underlying()
	{
		return static_cast<Operator&>(*this);
	}
};

template<class OperatorResult>
class OperatorResultBase
{
public:
	OperatorResultBase()
	{
	}

	virtual ~OperatorResultBase()
	{
	}

	const OperatorResult& write(HeterogeneousStorage& heterostorage) const
	{
		store(heterostorage);
		return underlying();
	}

	const OperatorResult& write(std::ostream& output) const
	{
		HeterogeneousStorage heterostorage;
		store(heterostorage);
		if(heterostorage.variant_object.empty())
		{
			return std::string();
		}
		JSONWriter::write(JSONWriter::Configuration(0), heterostorage.variant_object, output);
		return underlying();
	}

	std::string str() const
	{
		std::ostringstream output;
		write(output);
		return output.str();
	}

	virtual void store(HeterogeneousStorage&) const
	{
	}

private:
	const OperatorResult& underlying() const
	{
		return static_cast<const OperatorResult&>(*this);
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_BASE_H_ */
