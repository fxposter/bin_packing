#ifndef RESULT_INTERFACE_H
#define RESULT_INTERFACE_H

#include <string>
#include <vector>

namespace bin_packing
{
	class Context;
	class Range;

	class ResultInterface
	{
	public:
		typedef Range NeighbourRange;

		virtual ~ResultInterface() {};
		virtual NeighbourRange* neighbours() const = 0;

		virtual size_t containersCount() const = 0;
		virtual const double* containersWeights() const = 0;

		virtual std::string toString() const = 0;
		virtual std::string toGeneralString() const = 0;

		virtual ResultInterface* clone() const = 0;

		virtual const Context* context() const = 0;

		virtual const bool * const * matrix() const = 0;

        virtual std::vector< std::pair<size_t, size_t> > notAllowedToSet() const = 0;
        virtual bool deletedContainer(size_t& containter) const = 0;
	};
}

#endif RESULT_INTERFACE_H
