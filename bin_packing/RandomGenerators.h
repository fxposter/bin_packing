#include <vector>
#include <cstdlib>

namespace bin_packing
{
	class RandomGenerator {
	public:
		RandomGenerator(size_t itemsCount, double* items, double containerCapacity) : itemsCount_(itemsCount), items_(items), containerCapacity_(containerCapacity)
		{
		}

		virtual void generate(bool**& matrix, size_t& containersCount) const = 0;

	protected:
		bool** generateEmptyMatrix() const 
		{
			bool** matrix = new bool*[itemsCount_];
			for (size_t i = 0; i < itemsCount_; ++i) {
				matrix[i] = new bool[itemsCount_];
				for (size_t j = 0; j < itemsCount_; ++j) {
						matrix[i][j] = false;
				}
			}
			return matrix;
		}

		size_t itemsCount() const 
		{
			return itemsCount_;
		}

		double itemWeight(size_t i) const 
		{
			return items_[i];
		}

		double containerCapacity() const 
		{
			return containerCapacity_;
		}

	private:
		size_t itemsCount_;
		double* items_;
		double containerCapacity_;
	};

	class WorstRandomGenerator : public RandomGenerator{
	public:
		WorstRandomGenerator(size_t itemsCount, double* items, double containerCapacity) : RandomGenerator(itemsCount, items, containerCapacity)
		{
		}

		virtual void generate(bool**& matrix, size_t& containersCount) const
		{
			matrix = generateEmptyMatrix();
			for (size_t i = 0; i < itemsCount(); ++i)
				matrix[i][i] = true;
			containersCount = itemsCount();
		}
	};

	class FFRandomGenerator : public RandomGenerator {
	public:
		FFRandomGenerator(size_t itemsCount, double* items, double containerCapacity, bool useRandom = true) : RandomGenerator(itemsCount, items, containerCapacity), useRandom_(useRandom)
		{
		}

		virtual void generate(bool**& matrix, size_t& containersCount) const
		{
			matrix = generateEmptyMatrix();

			std::vector<double> containers;
			for (size_t i = 0; i < itemsCount(); ++i) {
				bool found = false;
				for (size_t j = 0; j < containers.size(); ++j) {
					bool itemFits = (containers[j] - itemWeight(i) >= 0);
					bool randomize = !useRandom_ || static_cast<double>(std::rand()) / RAND_MAX > 0.5;
					if (itemFits && randomize) {
						containers[j] -= itemWeight(i);
						matrix[i][j] = true;
						found = true;
						break;
					}
				}

				if (!found) {
					containers.push_back(containerCapacity());
					containers[containers.size() - 1] -= itemWeight(i);
					matrix[i][containers.size() - 1] = true;
				}
			}

			containersCount = containers.size();
		}

	private:
		bool useRandom_;
	};
}