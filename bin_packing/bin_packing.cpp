#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <functional>
#include <ctime>

namespace fx
{
	class Result;

	class Range
	{
	public:
		virtual bool empty() const = 0;
		virtual Result* next() = 0;
	};

	class Result
	{
	public:
		typedef Range NeighbourRange;
		typedef std::auto_ptr<Range> NeighbourRangePointer;
		virtual ~Result() {}
		virtual NeighbourRange* neighbours() const = 0;
		virtual std::string toString() const = 0;
	};

	typedef std::auto_ptr<Result> ResultPointer;

	class Context
	{
	public:
		virtual bool less(const Result&, const Result&) const = 0;
		virtual Result* selectRandomResult() const = 0;
	};

	// сгенерировать начальную последовательность
	// сгенерировать соседей
	// реализовать функцию сравнения

	Result* hillClimbing(Context& context)
	{
		Result* currentResult = context.selectRandomResult();

		while (true) {
			Result::NeighbourRange* range = currentResult->neighbours();
			Result* bestNeighbour = 0;
			while (!range->empty()) {
				Result* neighbour = range->next();

				// std::cout << "N: " << neighbour->toString() << std::endl;
				/*if (bestNeighbour)
					std::cout << "BN: " << bestNeighbour->toString() << std::endl;*/

				if (bestNeighbour == 0 || context.less(*neighbour, *bestNeighbour)) {
					delete bestNeighbour;
					bestNeighbour = neighbour;
				} else
					delete neighbour;
			}
			delete range;

			if (bestNeighbour)
				std::cout << "BN: " << bestNeighbour->toString() << std::endl;

			if (bestNeighbour && context.less(*bestNeighbour, *currentResult)) {
				delete currentResult;
				currentResult = bestNeighbour;
			} else {
				std::cout << "R: " << currentResult->toString() << std::endl;
				return currentResult;
			}
		}
	}

	namespace bin_packing
	{
		class BPContext;

		class BPResult : public Result
		{
		public:
			BPResult(const BPContext* context, bool** matrix, size_t containersCount, double* containersWeights = 0);
			virtual ~BPResult();
			virtual NeighbourRange* neighbours() const;

			size_t containersCount() const;
			double* containersWeights() const;

			virtual std::string toString() const;

		private:
			Result* move(size_t item, size_t toContainer) const;
			bool** copyMatrix() const;
			Result* swap(size_t firstItem, size_t secondItem) const;
			void checkContainerWeight(double weight) const;
			size_t findItem(size_t item) const;

			bool** matrix_;
			size_t containersCount_;
			const BPContext* context_;
			double* containersWeights_;
		};

		class BPContext : public Context
		{
		public:
			BPContext(double containerCapacity, size_t itemsCount, double* items) : containerCapacity_(containerCapacity),
				itemsCount_(itemsCount), items_(items)
			{
			}

			bool less(const Result& firstResult, const Result& secondResult) const
			{
				const BPResult& r1 = dynamic_cast<const BPResult&>(firstResult);
				const BPResult& r2 = dynamic_cast<const BPResult&>(secondResult);

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

			virtual Result* selectRandomResult() const
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
				return new BPResult(this, matrix, itemsCount_);
			}

			inline size_t itemsCount() const
			{
				return itemsCount_;
			}

			inline double item(size_t i) const
			{
				return items_[i];
			}

			inline double containerCapacity() const
			{
				return containerCapacity_;
			}

		private:
			double containerCapacity_;
			size_t itemsCount_;
			double* items_;
		};

		class BPRange : public Range
		{
		public:
			BPRange(const std::vector<Result*>& neighbours) : neighbours_(neighbours), index_(0)
			{
			}

			virtual bool empty() const
			{
				return index_ == neighbours_.size();
			}

			virtual Result* next()
			{
				return neighbours_[index_++];
			}

		private:
			size_t index_;
			std::vector<Result*> neighbours_;
		};

		BPResult::BPResult(const BPContext* context, bool** matrix, size_t containersCount, double* containersWeights) : context_(context), matrix_(matrix), containersWeights_(containersWeights), containersCount_(containersCount)
		{
			if (containersWeights_ == 0) {
				containersWeights_ = new double[containersCount_];
				for (size_t j = 0; j < containersCount_; ++j) {
					containersWeights_[j] = 0.0;
					for (size_t i = 0; i < context_->itemsCount(); ++i) {
						if (matrix_[i][j])
							containersWeights_[j] += context_->item(i);
					}
				}
			}
		}

		BPResult::~BPResult() {
			delete[] containersWeights_;
			for (size_t j = 0; j < containersCount_; ++j)
				delete[] matrix_[j];
			delete[] matrix_;
		}

		BPResult::NeighbourRange* BPResult::neighbours() const
		{
			std::vector<Result*> neighbours;
			for (size_t i = 0; i < context_->itemsCount(); ++i) {
				for (size_t j = 0; j < containersCount_; ++j) {
					try {
						neighbours.push_back(move(i, j));
					} catch (int&) {
					}
				}
			}

			for (size_t i = 0; i < context_->itemsCount(); ++i) {
				for (size_t j = i + 1; j < context_->itemsCount(); ++j) {
					try {
						neighbours.push_back(swap(i, j));
					} catch (int&) {
					}
				}
			}
			return new BPRange(neighbours);
		}

		size_t BPResult::containersCount() const
		{
			return containersCount_;
		}

		double* BPResult::containersWeights() const
		{
			return containersWeights_;
		}

		Result* BPResult::move(size_t item, size_t toContainer) const
		{
			checkContainerWeight(containersWeights_[toContainer] + context_->item(item));
			size_t fromContainer = findItem(item);
			if (fromContainer == toContainer)
				throw 1;

			bool** matrix = copyMatrix();
			matrix[item][fromContainer] = false;
			matrix[item][toContainer] = true;

			double* containersWeights = new double[containersCount_];
			for (size_t i = 0; i < containersCount_; ++i)
				containersWeights[i] = containersWeights_[i];
			containersWeights[fromContainer] -= context_->item(item);
			containersWeights[toContainer] += context_->item(item);

			size_t containersCount = containersCount_;
			if (containersWeights_[fromContainer] - context_->item(item) == 0.0) {
				--containersCount;

				size_t itemsCount = context_->itemsCount();
				for (size_t i = 0; i < itemsCount; ++i)
					for (size_t j = fromContainer; j < containersCount_ - 1; ++j)
						matrix[i][j] = matrix[i][j + 1];

				for (size_t j = fromContainer; j < containersCount_ - 1; ++j)
						containersWeights[j] = containersWeights[j + 1];
			}

			return new BPResult(context_, matrix, containersCount, containersWeights);
		}

		bool** BPResult::copyMatrix() const {
			size_t itemsCount = context_->itemsCount();
			bool** matrix = new bool*[itemsCount];
			for (size_t i = 0; i < itemsCount; ++i) {
				matrix[i] = new bool[containersCount_];
				for (size_t j = 0; j < containersCount_; ++j)
					matrix[i][j] = matrix_[i][j];
			}
			return matrix;
		}

		Result* BPResult::swap(size_t firstItem, size_t secondItem) const {
			size_t firstContainer = findItem(firstItem);
			size_t secondContainer = findItem(secondItem);
			if (firstContainer == secondContainer || firstItem == secondItem)
				throw 1;
			checkContainerWeight(containersWeights_[firstContainer] + context_->item(secondItem) - context_->item(firstItem));
			checkContainerWeight(containersWeights_[secondContainer] + context_->item(firstItem) - context_->item(secondItem));

			bool** matrix = copyMatrix();
			matrix[firstItem][firstContainer] = false;
			matrix[firstItem][secondContainer] = true;
			matrix[secondItem][firstContainer] = true;
			matrix[secondItem][secondContainer] = false;

			double* containersWeights = new double[containersCount_];
			for (size_t i = 0; i < containersCount_; ++i)
				containersWeights[i] = containersWeights_[i];
			containersWeights[firstContainer] -= context_->item(firstItem);
			containersWeights[secondContainer] += context_->item(firstItem);
			containersWeights[firstContainer] += context_->item(secondItem);
			containersWeights[secondContainer] -= context_->item(secondItem);

			return new BPResult(context_, matrix, containersCount_, containersWeights);
		}

		void BPResult::checkContainerWeight(double weight) const
		{
			if (weight > context_->containerCapacity())
				throw 1;
		}

		size_t BPResult::findItem(size_t item) const
		{
			for (size_t i = 0; i < containersCount_; ++i) {
				if (matrix_[item][i])
					return i;
			}
			throw 1;//std::exception();
		}

		std::string BPResult::toString() const {
			std::stringstream ss;
			ss << "(" << containersCount_ << ") ";
			for (size_t i = 0; i < containersCount_; ++i)
				ss << containersWeights_[i] << ' ';
			return ss.str();
		}
	}
}

using namespace fx;
using namespace fx::bin_packing;

int main()
{
	double testData[60] = { 36.6,
26.8,
36.6,
43.0,
26.3,
30.7,
41.4,
28.7,
29.9,
49.5,
25.1,
25.4,
47.4,
25.2,
27.4,
37.0,
26.9,
36.1,
47.3,
25.2,
27.5,
47.2,
25.9,
26.9,
44.4,
25.8,
29.8,
43.9,
27.3,
28.8,
44.5,
27.2,
28.3,
41.9,
26.1,
32.0,
36.3,
27.1,
36.6,
35.5,
27.3,
37.2,
46.6,
26.2,
27.2,
35.7,
29.2,
35.1,
39.5,
25.5,
35.0,
35.0,
30.3,
34.7,
45.0,
25.2,
29.8,
41.0,
27.5,
31.5 };
	BPContext context(100, 60, testData);
	hillClimbing(context);

	std::cin.get();
}
