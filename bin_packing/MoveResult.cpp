#include "MoveResult.h"
#include "Clone.h"
#include "Context.h"
#include "Result.h"

#include <sstream>
#include <cmath>

#include <functional>
#include <algorithm>

namespace bin_packing
{
	MoveResult::MoveResult(const ResultInterface* origin, size_t item, size_t fromContainer, size_t toContainer) : origin_(origin), item_(item), fromContainer_(fromContainer), toContainer_(toContainer) {
		containersCount_ = origin->containersCount();
		containersWeights_ = ::clone(origin->containersWeights(), containersCount_);

		containersWeights_[fromContainer] -= origin->context()->itemWeight(item);
		containersWeights_[toContainer] += origin->context()->itemWeight(item);

        deletedContainer_ = false;
        if (containersWeights_[fromContainer] == 0.0) {
			--containersCount_;
            deletedContainer_ = true;

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

    std::vector< std::pair<size_t, size_t> > MoveResult::notAllowedToSet() const
    {
        std::vector< std::pair<size_t, size_t> > result;
        result.push_back(std::make_pair(fromContainer_, item_));
        return result;
    }

    bool MoveResult::deletedContainer(size_t& containter) const
    {
        if (deletedContainer_) {
            containter = fromContainer_;
            return true;
        }
        return false;
    }
}