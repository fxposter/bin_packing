#include "Result.h"
#include "Context.h"
#include "Range.h"
#include "MoveResult.h"
#include "SwapResult.h"
#include "Clone.h"

#include <vector>
#include <sstream>

namespace bin_packing
{
	Result::Result(const Context* context, bool** matrix, size_t containersCount, double* containersWeights) : context_(context), matrix_(matrix), containersWeights_(containersWeights), containersCount_(containersCount)
	{
		// std::cout << "Created" << std::endl;
		if (containersWeights_ == 0) {
			containersWeights_ = new double[containersCount_];
			for (size_t j = 0; j < containersCount_; ++j) {
				containersWeights_[j] = 0.0;
				for (size_t i = 0; i < context_->itemsCount(); ++i) {
					if (matrix_[i][j])
						containersWeights_[j] += context_->itemWeight(i);
				}
			}
		}
	}

	Result::~Result() {
		// std::cout << "Deleted" << std::endl;
		delete[] containersWeights_;
		for (size_t j = 0; j < context_->itemsCount(); ++j)
			delete[] matrix_[j];
		delete[] matrix_;
	}

	const Context* Result::context() const
	{
		return context_;
	}

	ResultInterface* Result::clone() const
	{
		return new Result(context_, ::clone(matrix_, context_->itemsCount(), containersCount_), containersCount_/*, ::clone(containersWeights_, containersCount_)*/);
	}

	Result::NeighbourRange* Result::neighbours() const
	{
		bool useSwaps = true;
		std::vector<ResultInterface*> neighbours;
		for (size_t i = 0; i < context_->itemsCount(); ++i) {
			for (size_t j = 0; j < containersCount_; ++j) {
				try {
					ResultInterface* result = move(i, j);
					if (result->containersCount() < containersCount_)
						useSwaps = false;
					neighbours.push_back(result);
				} catch (int&) {
				}
			}
		}

		if (useSwaps) {
			for (size_t i = 0; i < context_->itemsCount(); ++i) {
				for (size_t j = i + 1; j < context_->itemsCount(); ++j) {
					try {
						neighbours.push_back(swap(i, j));
					} catch (int&) {
					}
				}
			}
		}
		return new NeighbourRange(neighbours);
	}

	size_t Result::containersCount() const
	{
		return containersCount_;
	}

	const double* Result::containersWeights() const
	{
		return containersWeights_;
	}

	ResultInterface* Result::move(size_t item, size_t toContainer) const
	{
		checkContainerWeight(containersWeights_[toContainer] + context_->itemWeight(item));
		size_t fromContainer = findItem(item);
		if (fromContainer == toContainer)
			throw 1;

		return new MoveResult(this, item, fromContainer, toContainer);
	}

	bool** Result::copyMatrix() const {
		size_t itemsCount = context_->itemsCount();
		bool** matrix = new bool*[itemsCount];
		for (size_t i = 0; i < itemsCount; ++i) {
			matrix[i] = new bool[containersCount_];
			for (size_t j = 0; j < containersCount_; ++j)
				matrix[i][j] = matrix_[i][j];
		}
		return matrix;
	}

	ResultInterface* Result::swap(size_t firstItem, size_t secondItem) const {
		size_t firstContainer = findItem(firstItem);
		size_t secondContainer = findItem(secondItem);
		if (firstContainer == secondContainer || firstItem == secondItem)
			throw 1;
		checkContainerWeight(containersWeights_[firstContainer] + context_->itemWeight(secondItem) - context_->itemWeight(firstItem));
		checkContainerWeight(containersWeights_[secondContainer] + context_->itemWeight(firstItem) - context_->itemWeight(secondItem));

		return new SwapResult(this, firstItem, firstContainer, secondItem, secondContainer);
	}

	void Result::checkContainerWeight(double weight) const
	{
		if (weight > context_->containerCapacity())
			throw 1;
	}

	size_t Result::findItem(size_t item) const
	{
		for (size_t i = 0; i < containersCount_; ++i) {
			if (matrix_[item][i])
				return i;
		}
		throw 1;//std::exception();
	}

	std::string Result::toString() const {
		std::stringstream ss;
		ss << "(" << containersCount_ << ") ";
		for (size_t i = 0; i < containersCount_; ++i)
			ss << containersWeights_[i] << ' ';
		return ss.str();
	}

	std::string Result::toGeneralString() const {
		std::stringstream ss;
		ss << "Containers: " << containersCount_ << std::endl;
		for (size_t i = 0; i < containersCount_; ++i) {
			ss.width(5);
			ss << i << ": ";
			for (size_t j = 0; j < context_->itemsCount(); ++j) {
				if (matrix_[j][i]) {
					ss.width(5);
					ss << j << ' ';
				}
			}
			ss << std::endl;
		}
		return ss.str();
	}

	const bool * const * Result::matrix() const
	{
		return matrix_;
	}
}