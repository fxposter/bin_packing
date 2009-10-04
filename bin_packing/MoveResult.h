#ifndef MOVE_RESULT_H
#define MOVE_RESULT_H

#include "ResultInterface.h"

namespace bin_packing
{
	class MoveResult : public ResultInterface
	{
	public:
		MoveResult(const ResultInterface* origin, size_t item, size_t fromContainer, size_t toContainer);
		virtual ~MoveResult();

		virtual NeighbourRange* neighbours() const;
		size_t containersCount() const;
		const double* containersWeights() const;

		virtual std::string toString() const;
		virtual std::string toGeneralString() const;

		const Context* context() const;
		ResultInterface* clone() const;
		const bool * const * matrix() const;

	private:
		bool** createMatrix() const;

		const ResultInterface* origin_;
		size_t item_;
		size_t fromContainer_;
		size_t toContainer_;

		double* containersWeights_;
		size_t containersCount_;
	};
}

#endif // MOVE_RESULT_H