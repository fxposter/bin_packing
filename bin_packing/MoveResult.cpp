#include "MoveResult.h"
#include "Clone.h"
#include "Context.h"
#include "Result.h"

#include <sstream>

namespace bin_packing
{
	MoveResult::MoveResult(const ResultInterface* origin, size_t item, size_t fromContainer, size_t toContainer) : origin_(origin), item_(item), fromContainer_(fromContainer), toContainer_(toContainer) {
		containersCount_ = origin->containersCount();
		containersWeights_ = ::clone(origin->containersWeights(), containersCount_);

		containersWeights_[fromContainer] -= origin->context()->itemWeight(item);
		containersWeights_[toContainer] += origin->context()->itemWeight(item);

		if (containersWeights_[fromContainer] == 0.0) {
			--containersCount_;

			for (size_t j = fromContainer; j < containersCount_; ++j)
				containersWeights_[j] = containersWeights_[j + 1];
		}
	}

	MoveResult::~MoveResult() {
		delete containersWeights_;
	}

	MoveResult::NeighbourRange* MoveResult::neighbours() const {
		throw 1;
	}

	size_t MoveResult::containersCount() const {
		return containersCount_;
	}
	
	const double* MoveResult::containersWeights() const {
		return containersWeights_;
	}

	std::string MoveResult::toString() const {
		std::stringstream ss;
		ss << "(" << containersCount_ << ") ";
		for (size_t i = 0; i < containersCount_; ++i)
			ss << containersWeights_[i] << ' ';
		return ss.str();
	}

	std::string MoveResult::toGeneralString() const {
		throw 1;
	}

	const Context* MoveResult::context() const {
		return origin_->context();
	}

	ResultInterface* MoveResult::clone() const
	{
		return new Result(context(), createMatrix(), containersCount_/*, ::clone(containersWeights_, containersCount_)*/);
	}

	const bool * const * MoveResult::matrix() const
	{
		throw 1;
	}

	bool** MoveResult::createMatrix() const {
		bool** matrix = ::clone(origin_->matrix(), context()->itemsCount(), origin_->containersCount());
		matrix[item_][fromContainer_] = false;
		matrix[item_][toContainer_] = true;

		if (containersCount_ != origin_->containersCount()) {
			size_t itemsCount = context()->itemsCount();
			for (size_t i = 0; i < itemsCount; ++i)
				for (size_t j = fromContainer_; j < containersCount_; ++j)
					matrix[i][j] = matrix[i][j + 1];
		}

		return matrix;
	}
}