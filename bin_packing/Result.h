#ifndef RESULT_H
#define RESULT_H

#include "ResultInterface.h"

namespace bin_packing
{
	class Context;

	class Result : public ResultInterface
	{
	public:
		Result(const Context* context, bool** matrix, size_t containersCount, double* containersWeights = 0);
		
		virtual ~Result();
		virtual NeighbourRange* neighbours() const;

		virtual size_t containersCount() const;
		virtual const double* containersWeights() const;

		virtual std::string toString() const;
		virtual std::string toGeneralString() const;

		virtual const Context* context() const;
		virtual ResultInterface* clone() const;

		virtual const bool * const * matrix() const;

	private:
		bool** copyMatrix() const;

		ResultInterface* move(size_t item, size_t toContainer) const;
		ResultInterface* swap(size_t firstItem, size_t secondItem) const;

		void checkContainerWeight(double weight) const;
		size_t findItem(size_t item) const;

		bool** matrix_;
		double* containersWeights_;
		size_t containersCount_;

		const Context* context_;
	};
}

#endif // RESULT_H
