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
                std::cout << "S: " << stepsCount << '\n';

				// std::cout << result->toGeneralString() << std::endl;
				return result;
			}
		}
	}

    ResultInterface* tabuSearch(Context& context)
	{
		ResultInterface* currentResult = context.createRandomResult();

        size_t containersCount = currentResult->containersCount();
        char** shortTermMemory = new char*[containersCount];
        for (size_t i = 0; i < containersCount; ++i) {
            shortTermMemory[i] = new char[context.itemsCount()];
            std::fill(shortTermMemory[i], shortTermMemory[i] + context.itemsCount(), 0);
        }

        char tabuTenure = static_cast<char>(std::sqrt(static_cast<double>(context.itemsCount() * containersCount)) * 1.2);
        std::cout << "Tabu tenure: " << static_cast<int>(tabuTenure) << '\n';

        ResultInterface* bestResult = currentResult->clone();

		std::cout << "F: " << currentResult->toString() << '\n';

        size_t stepsCount = 0;
        size_t maxSteps = 200;
		while (maxSteps-- > 0) {
            stepsCount++;

			ResultInterface::NeighbourRange* range = currentResult->neighbours();
			ResultInterface* bestNeighbour = 0;
            ResultInterface* neighbourWithMinTabu = 0;
            int minTabu = 256;

			while (!range->empty()) {
				ResultInterface* neighbour = range->next();

                int overallTabu = 0;
                const std::vector< std::pair<size_t, size_t> >& notAllowedToSet = neighbour->notAllowedToSet();
                for (std::vector< std::pair<size_t, size_t> >::const_iterator i = notAllowedToSet.begin(); i != notAllowedToSet.end(); ++i)
                    if (shortTermMemory[i->first][i->second] > 0)
                        overallTabu += shortTermMemory[i->first][i->second];

                if (overallTabu > 0) {
                    if (overallTabu < minTabu) {
                        delete neighbourWithMinTabu;
                        neighbourWithMinTabu = neighbour;
                        neighbour = 0;
                    } else {
                        delete neighbour;
                        neighbour = 0;
                    }
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

                size_t deletedContainer = 0;
                if (bestNeighbour->deletedContainer(deletedContainer)) {
                    char* temp = shortTermMemory[deletedContainer];
                    for (size_t i = deletedContainer; i < containersCount; ++i)
                        shortTermMemory[i] = shortTermMemory[i + 1];
                    shortTermMemory[containersCount - 1] = temp;
                } else {
                    const std::vector< std::pair<size_t, size_t> >& notAllowedToSet = bestNeighbour->notAllowedToSet();
                    std::cout << "Changed items: ";
                    for (std::vector< std::pair<size_t, size_t> >::const_iterator i = notAllowedToSet.begin(); i != notAllowedToSet.end(); ++i) {
                        shortTermMemory[i->first][i->second] += tabuTenure + 1;
                        std::cout << "(" << i->first << ", " << i->second << "), ";
                    }
                    std::cout << '\n';
                }
			}

			if (bestNeighbour) {
                if (context.less(*currentResult, *bestNeighbour)) {
                    std::cout << "----------------------------------Bad result----------------------------------\n";
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
                if (neighbourWithMinTabu) {
                    ResultInterface* oldResult = currentResult;
				    currentResult = neighbourWithMinTabu->clone();
                    delete neighbourWithMinTabu;
				    delete oldResult;
                } else {
                    break;
                }
			}

            for (size_t i = 0; i < containersCount; ++i)
                for (size_t j = 0; j < context.itemsCount(); ++j)
                    if (shortTermMemory[i][j] > 0)
                        --shortTermMemory[i][j];
		}

        std::cout << "R: " << bestResult->toString() << '\n';
        std::cout << "S: " << stepsCount << '\n';

        delete currentResult;
        delete[] shortTermMemory;
        return bestResult;
	}
}