#include "Algorithms.h"

#include "Context.h"
#include "ResultInterface.h"
#include "Range.h"

#include <iostream>
#include <ctime>
#include <cmath>

namespace bin_packing
{
	ResultInterface* hillClimbing(Context& context)
	{
		ResultInterface* currentResult = context.createRandomResult();

		std::cout << "F: " << currentResult->toString() << '\n';

        size_t stepsCount = 0;
		while (true) {
            stepsCount++;

			ResultInterface::NeighbourRange* range = currentResult->neighbours();
			ResultInterface* bestNeighbour = 0;

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

			if (bestNeighbour) {
				// std::cout << "BN: " << bestNeighbour->toString() << '\n';
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
                std::cout << "S: " << stepsCount << '\n';

				// std::cout << result->toGeneralString() << std::endl;
				return result;
			}
		}
	}

    ResultInterface* tabuSearch(Context& context)
	{
        char* shortTermMemory = new char[context.itemsCount()];
        char tabuTenure = 3; // static_cast<char>(std::sqrt(static_cast<double>(context.itemsCount())) * 1.2);

        std::fill(shortTermMemory, shortTermMemory + context.itemsCount(), 0);

		ResultInterface* currentResult = context.createRandomResult();
        ResultInterface* bestResult = currentResult->clone();

		std::cout << "F: " << currentResult->toString() << '\n';

        size_t stepsCount = 0;
        size_t maxSteps = 300;
		while (maxSteps-- > 0) {
            stepsCount++;

			ResultInterface::NeighbourRange* range = currentResult->neighbours();
			ResultInterface* bestNeighbour = 0;

			while (!range->empty()) {
				ResultInterface* neighbour = range->next();

                const std::vector<size_t>& changedItems = neighbour->changedItems();
                for (std::vector<size_t>::const_iterator i = changedItems.begin(); i != changedItems.end(); ++i)
                    if (shortTermMemory[*i] > 0) {
                        delete neighbour;
                        neighbour = 0;
                        break;
                    }

                if (neighbour) {
				    if (bestNeighbour == 0 || context.less(*neighbour, *bestNeighbour)) {
					    delete bestNeighbour;
					    bestNeighbour = neighbour;
				    } else {
					    delete neighbour;
				    }
                }
			}
			delete range;

			if (bestNeighbour) {
				std::cout << "BN: " << bestNeighbour->toString() << '\n';

                const std::vector<size_t>& changedItems = bestNeighbour->changedItems();
                // std::cout << "Changed items: ";
                for (std::vector<size_t>::const_iterator i = changedItems.begin(); i != changedItems.end(); ++i) {
                    shortTermMemory[*i] += tabuTenure;
                    // std::cout << *i << ", ";
                }
                // std::cout << '\n';
			}

			if (bestNeighbour) {
                if (context.less(*currentResult, *bestNeighbour)) {
                    std::cout << "Bad result\n";
                }

				ResultInterface* oldResult = currentResult;
				currentResult = bestNeighbour->clone();

                if (context.less(*bestNeighbour, *bestResult)) {
                    delete bestResult;
                    bestResult = bestNeighbour->clone();
                }

				delete bestNeighbour;
				delete oldResult;
			} else {
                break; // TODO
				ResultInterface* result = currentResult->clone();
				if (bestNeighbour)
					delete bestNeighbour;

				delete currentResult;
				std::cout << "R: " << result->toString() << '\n';
                std::cout << "S: " << stepsCount << '\n';
				// std::cout << result->toGeneralString() << std::endl;

                delete[] shortTermMemory;
				return result;
			}

            for (size_t i = 0; i < context.itemsCount(); ++i)
                if (shortTermMemory[i] > 0)
                    --shortTermMemory[i];
		}

        std::cout << "R: " << bestResult->toString() << '\n';
        std::cout << "S: " << stepsCount << '\n';

        delete currentResult;
        delete[] shortTermMemory;
        return bestResult;
	}
}