#ifndef SWAP_RESULT_H
#define SWAP_RESULT_H

#include "ResultInterface.h"

namespace bin_packing
{
	class SwapResult : public ResultInterface
	{
	public:
		SwapResult(const ResultInterface* origin, size_t firstItem, size_t firstContainer, size_t secondItem, size_t secondContainer);
		virtual ~SwapResult();

		virtual NeighbourRange* neighbours() const;
		size_t containersCount() const;
		const double* containersWeights() const;

		virtual std::string toString() const;
		virtual std::string toGeneralString() const;

		const Context* context() const;
		ResultInterface* clone() const;
		const bool * const * matrix() const;

        virtual std::vector<size_t> changedItems() const;

	private:
		bool** createMatrix() const;

		const ResultInterface* origin_;

		size_t firstItem_;
		size_t firstContainer_;
		size_t secondItem_;
		size_t secondContainer_;

		double* containersWeights_;
	};
}

#endif // SWAP_RESULT_H