#include "Algorithms.h"

#include "Context.h"
#include "ResultInterface.h"
#include "Range.h"

#include <iostream>
#include <ctime>

namespace bin_packing
{
	ResultInterface* hillClimbing(Context& context)
	{
		ResultInterface* currentResult = context.createRandomResult();

		std::cout << "F: " << currentResult->toString() << '\n';

		while (true) {
			ResultInterface::NeighbourRange* range = currentResult->neighbours();
			ResultInterface* bestNeighbour = 0;

			clock_t begin = clock();

			while (!range->empty()) {
				ResultInterface* neighbour = range->next();

				if (bestNeighbour == 0 || context.less(*neighbour, *bestNeighbour)) {
					delete bestNeighbour;
					bestNeighbour = neighbour;
				} else {
					delete neighbour;
				}
			}
			delete range;

			clock_t end = clock();

			std::cout << "Clock time: " << (end - begin) << '\n';

			if (bestNeighbour) {
				std::cout << "BN: " << bestNeighbour->toString() << '\n';
			}

			if (bestNeighbour && context.less(*bestNeighbour, *currentResult)) {
				ResultInterface* oldResult = currentResult;
				currentResult = bestNeighbour->clone();
				delete bestNeighbour;
				delete oldResult;
			} else {
				ResultInterface* result = currentResult->clone();
				if (bestNeighbour)
					delete bestNeighbour;

				delete currentResult;
				std::cout << "R: " << result->toString() << '\n';
				// std::cout << result->toGeneralString() << std::endl;
				return result;
			}
		}
	}
}