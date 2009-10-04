#include "Context.h"

namespace bin_packing
{
	Context::Context(double containerCapacity, size_t itemsCount, double* items) : containerCapacity_(containerCapacity),
		itemsCount_(itemsCount), items_(items)
	{
	}

	bool Context::less(const ResultInterface& r1, const ResultInterface& r2) const
	{
		if (r1.containersCount() < r2.containersCount())
			return true;
		else if (r1.containersCount() > r2.containersCount())
			return false;

		size_t containers = r1.containersCount();

		double* r1w = new double[containers];
		std::copy(r1.containersWeights(), r1.containersWeights() + containers, r1w);
		double* r2w = new double[containers];
		std::copy(r2.containersWeights(), r2.containersWeights() + containers, r2w);

		for (size_t i = 0; i < containers; ++i) {
			r1w[i] = containerCapacity_ - r1w[i];
			r2w[i] = containerCapacity_ - r2w[i];
		}
		std::sort(r1w, r1w + containers, std::greater<double>());
		std::sort(r2w, r2w + containers, std::greater<double>());

		bool result = false;
		for (size_t i = 0; i < containers; ++i)
			if (r1w[i] > r2w[i]) {
				result = true;
				break;
			} else if (r1w[i] < r2w[i]) {
				result = false;
				break;
			}
		delete[] r1w;
		delete[] r2w;

		return result;

	}

	Result* Context::selectRandomResult() const
	{
		bool** matrix = new bool*[itemsCount_];
		for (size_t i = 0; i < itemsCount_; ++i) {
			matrix[i] = new bool[itemsCount_];
			for (size_t j = 0; j < itemsCount_; ++j) {
				if (i == j)
					matrix[i][j] = true;
				else
					matrix[i][j] = false;
			}
		}

		return new Result(this, matrix, itemsCount_);
	}

	size_t Context::itemsCount() const
	{
		return itemsCount_;
	}

	double Context::item(size_t i) const
	{
		return items_[i];
	}

	double Context::containerCapacity() const
	{
		return containerCapacity_;
	}
}