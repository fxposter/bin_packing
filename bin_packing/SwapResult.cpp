#include "SwapResult.h"
#include "Clone.h"
#include "Context.h"
#include "Result.h"

#include <sstream>

#include <functional>
#include <algorithm>

namespace bin_packing
{
	SwapResult::SwapResult(const ResultInterface* origin, size_t firstItem, size_t firstContainer, size_t secondItem, size_t secondContainer) : origin_(origin), firstItem_(firstItem), firstContainer_(firstContainer), secondItem_(secondItem), secondContainer_(secondContainer) {
		containersWeights_ = ::clone(origin->containersWeights(), origin_->containersCount());

		containersWeights_[firstContainer]  -= context()->itemWeight(firstItem);
		containersWeights_[secondContainer] += context()->itemWeight(firstItem);
		containersWeights_[firstContainer]  += context()->itemWeight(secondItem);
		containersWeights_[secondContainer] -= context()->itemWeight(secondItem);
	}

	SwapResult::~SwapResult() {
		delete containersWeights_;
	}

	SwapResult::NeighbourRange* SwapResult::neighbours() const {
		throw 1;
	}

	size_t SwapResult::containersCount() const {
		return origin_->containersCount();
	}
	
	const double* SwapResult::containersWeights() const {
		return containersWeights_;
	}

	std::string SwapResult::toString() const {
		double* rw = ::clone(containersWeights_, containersCount());

		for (size_t i = 0; i < containersCount(); ++i)
			rw[i] = context()->containerCapacity() - rw[i];
		std::sort(rw, rw + containersCount(), std::greater<double>());

		std::stringstream ss;
		ss << "(" << containersCount() << ") ";
		for (size_t i = 0; i < containersCount(); ++i)
			ss << rw[i] << ' ';
        delete[] rw;

		return ss.str();
	}

	std::string SwapResult::toGeneralString() const {
		throw 1;
	}

	const Context* SwapResult::context() const {
		return origin_->context();
	}

	ResultInterface* SwapResult::clone() const
	{
		return new Result(context(), createMatrix(), containersCount()/*, ::clone(containersWeights_, containersCount())*/);
	}

	const bool * const * SwapResult::matrix() const
	{
		throw 1;
	}

	bool** SwapResult::createMatrix() const {
		bool** matrix = ::clone(origin_->matrix(), context()->itemsCount(), origin_->containersCount());
		matrix[firstItem_][firstContainer_] = false;
		matrix[firstItem_][secondContainer_] = true;
		matrix[secondItem_][firstContainer_] = true;
		matrix[secondItem_][secondContainer_] = false;

		return matrix;
	}

    std::vector<size_t> SwapResult::changedItems() const
    {
        std::vector<size_t> result;
        result.push_back(firstItem_);
        result.push_back(secondItem_);
        return result;
    }
}
