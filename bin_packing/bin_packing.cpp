#include <vld.h>

#include <iostream>
#include <ctime>
#include <fstream>

#include "Context.h"
#include "Result.h"
#include "Range.h"

#include "Algorithms.h"

using namespace bin_packing;

class DataLoader
{
public:
	DataLoader(const std::string& filename) : filename_(filename)
	{
	}

	Context* load(size_t dataSetNumber)
	{
		std::ifstream file(filename_.c_str(), std::ios::in);
		if (!file.is_open())
			throw 1;

		size_t dataSetCount;
		file >> dataSetCount;

		if (dataSetCount < dataSetNumber)
			throw 1;

		std::string s;
		std::getline(file, s, '\n');
		for (size_t i = 0; i <= dataSetNumber; ++i) {
			std::getline(file, s, '\n');
			double containerCapacity;
			size_t itemsCount, bestKnownNumberOfContainers;
			file >> containerCapacity >> itemsCount >> bestKnownNumberOfContainers;
			if (i != dataSetNumber) {
				for (size_t j = 0; j <= itemsCount; ++j)
					std::getline(file, s, '\n');
			} else {
				double* items = new double[itemsCount];
				for (size_t j = 0; j < itemsCount; ++j)
					file >> items[j];
				file.close();
				return new Context(containerCapacity, itemsCount, items);
			}
		}

		file.close();
		throw 1;
	}

private:
	std::string filename_;
};

int main()
{
	DataLoader loader("data/binpack7.txt");
	//for (size_t i = 0; i < 20; ++i) {
		Context* context = loader.load(0);
		ResultInterface* result = hillClimbing(*context);
		delete result;
		delete context;
	//}
	
	std::cin.get();
	return 0;
}
