#include "Context.h"
#include "ResultInterface.h"
#include "Result.h"
#include "Clone.h"

#include "RandomGenerators.h"

#include <algorithm>
#include <functional>

#include <iostream>
#include <cmath>

namespace bin_packing
{
	Context::Context(double containerCapacity, size_t itemsCount, double* items, size_t bestKnownNumberOfContainers) : containerCapacity_(containerCapacity),
		itemsCount_(itemsCount), items_(items), bestKnownNumberOfContainers_(bestKnownNumberOfContainers)
	{
	}

	bool Context::less(const ResultInterface& r1, const ResultInterface& r2) const
	{
        const double* r1w = r1.containersWeights();
		const double* r2w = r2.containersWeights();

        double s1 = 0.0;
        for (size_t i = 0; i < r1.containersCount(); ++i) {
            s1 += std::pow(r1w[i] / containerCapacity(), 2);
        }
        s1 /= r1.containersCount();

        double s2 = 0.0;
        for (size_t i = 0; i < r2.containersCount(); ++i) {
            s2 += std::pow(r2w[i] / containerCapacity(), 2);
        }
        s2 /= r2.containersCount();

        // std::cout << s1 << " - " << s2 << '\n';

        return s1 > s2;

        /*
		if (r1.containersCount() < r2.containersCount())
			return true;
		else if (r1.containersCount() > r2.containersCount())
			return false;

		size_t containers = r1.containersCount();

		double* r1w = ::clone(r1.containersWeights(), containers);
		double* r2w = ::clone(r2.containersWeights(), containers);

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

		return result;*/

	}

	ResultInterface* Context::createRandomResult() const
	{
		bool** matrix = 0;
		size_t containersCount = 0;

		FFRandomGenerator generator(itemsCount_, items_, containerCapacity_);
		generator.generate(matrix, containersCount);

		return new Result(this, matrix, containersCount);
	}

	size_t Context::itemsCount() const
	{
		return itemsCount_;
	}

    size_t Context::bestKnownNumberOfContainers() const
	{
		return bestKnownNumberOfContainers_;
	}

	double Context::itemWeight(size_t i) const
	{
		return items_[i];
	}

	double Context::containerCapacity() const
	{
		return containerCapacity_;
	}
}