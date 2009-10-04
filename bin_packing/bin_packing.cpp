// #include <vld.h>

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
		virtual ~Range() {}

		virtual bool empty() const = 0;
		virtual Result* next() = 0;
	};

	class Result
	{
	public:
		typedef Range NeighbourRange;

		virtual ~Result() {}
		virtual NeighbourRange* neighbours() const = 0;
		virtual std::string toString() const = 0;
		virtual std::string toGeneralString() const = 0;
		virtual Result* clone() const = 0;
	};

	class Context
	{
	public:
		virtual bool less(const Result&, const Result&) const = 0;
		virtual Result* selectRandomResult() const = 0;
	};

	Result* hillClimbing(Context& context)
	{
		Result* currentResult = context.selectRandomResult();

		while (true) {
			Result::NeighbourRange* range = currentResult->neighbours();
			Result* bestNeighbour = 0;

			clock_t begin = clock();

			while (!range->empty()) {
				Result* neighbour = range->next();

				if (bestNeighbour == 0 || context.less(*neighbour, *bestNeighbour)) {
					delete bestNeighbour;
					bestNeighbour = neighbour;
				} else {
					delete neighbour;
				}
			}
			delete range;

			clock_t end = clock();

			std::cout << "Clock time: " << (end - begin) << std::endl;

			if (bestNeighbour)
				std::cout << "BN: " << bestNeighbour->toString() << std::endl;

			if (bestNeighbour && context.less(*bestNeighbour, *currentResult)) {
				Result* oldResult = currentResult;
				currentResult = bestNeighbour->clone();
				delete bestNeighbour;
				delete oldResult;
			} else {
				delete bestNeighbour;
				std::cout << "R: " << currentResult->toString() << std::endl;
				std::cout << currentResult->toGeneralString() << std::endl;
				return currentResult;
			}
		}
	}

	namespace bin_packing
	{
		class BPContext;

		class BPResultInterface : public Result
		{
		public:
			virtual ~BPResultInterface() {};
			virtual NeighbourRange* neighbours() const = 0;

			virtual size_t containersCount() const = 0;
			virtual const double* containersWeights() const = 0;

			virtual std::string toString() const = 0;
			virtual std::string toGeneralString() const = 0;

			virtual Result* clone() const = 0;

			virtual const BPContext* context() const = 0;
		};

		class BPResult : public BPResultInterface
		{
		public:
			BPResult(const BPContext* context, bool** matrix, size_t containersCount, double* containersWeights = 0);
			virtual ~BPResult();
			virtual NeighbourRange* neighbours() const;

			size_t containersCount() const;
			const double* containersWeights() const;

			virtual std::string toString() const;
			virtual std::string toGeneralString() const;

			const BPContext* context() const;

			virtual Result* clone() const;

			bool** copyMatrix() const;

		private:
			Result* move(size_t item, size_t toContainer) const;
			
			Result* swap(size_t firstItem, size_t secondItem) const;
			void checkContainerWeight(double weight) const;
			size_t findItem(size_t item) const;

			bool** matrix_;
			double* containersWeights_;
			size_t containersCount_;

			const BPContext* context_;
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
				const BPResultInterface& r1 = dynamic_cast<const BPResultInterface&>(firstResult);
				const BPResultInterface& r2 = dynamic_cast<const BPResultInterface&>(secondResult);

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

		class BPMoveResult : public BPResultInterface
		{
		public:
			BPMoveResult(const BPResult* origin, size_t item, size_t fromContainer, size_t toContainer) : origin_(origin), item_(item), fromContainer_(fromContainer), toContainer_(toContainer) {
				containersCount_ = origin->containersCount();
				containersWeights_ = new double[containersCount_];
				std::copy(origin->containersWeights(), origin->containersWeights() + containersCount_, containersWeights_);

				containersWeights_[fromContainer] -= origin->context()->item(item);
				containersWeights_[toContainer] += origin->context()->item(item);

				if (containersWeights_[fromContainer] == 0.0) {
					--containersCount_;

					for (size_t j = fromContainer; j < containersCount_; ++j)
						containersWeights_[j] = containersWeights_[j + 1];
				}
			}

			virtual ~BPMoveResult() {
				delete containersWeights_;
			}

			virtual NeighbourRange* neighbours() const {
				throw 1;
			}

			size_t containersCount() const {
				return containersCount_;
			}
			
			const double* containersWeights() const {
				return containersWeights_;
			}

			virtual std::string toString() const {
				std::stringstream ss;
				ss << "(" << containersCount_ << ") ";
				for (size_t i = 0; i < containersCount_; ++i)
					ss << containersWeights_[i] << ' ';
				return ss.str();
			}

			virtual std::string toGeneralString() const {
				throw 1;
			}

			const BPContext* context() const {
				return origin_->context();
			}

			Result* clone() const
			{
				double* containersWeights = new double[containersCount_];
				std::copy(containersWeights_, containersWeights_ + containersCount_, containersWeights);
				return new BPResult(context(), matrix(), containersCount(), containersWeights);
			}

		private:
			bool** matrix() const {
				bool** matrix = origin_->copyMatrix();
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

			const BPResult* origin_;
			size_t item_;
			size_t fromContainer_;
			size_t toContainer_;
			double* containersWeights_;
			size_t containersCount_;
		};

		BPResult::BPResult(const BPContext* context, bool** matrix, size_t containersCount, double* containersWeights) : context_(context), matrix_(matrix), containersWeights_(containersWeights), containersCount_(containersCount)
		{
			// std::cout << "Created" << std::endl;
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
			// std::cout << "Deleted" << std::endl;
			delete[] containersWeights_;
			for (size_t j = 0; j < context_->itemsCount(); ++j)
				delete[] matrix_[j];
			delete[] matrix_;
		}

		const BPContext* BPResult::context() const
		{
			return context_;
		}

		Result* BPResult::clone() const
		{
			return new BPResult(context_, matrix_, containersCount_, containersWeights_);
		}

		BPResult::NeighbourRange* BPResult::neighbours() const
		{
			bool useSwaps = true;
			std::vector<Result*> neighbours;
			for (size_t i = 0; i < context_->itemsCount(); ++i) {
				for (size_t j = 0; j < containersCount_; ++j) {
					try {
						Result* result = move(i, j);
						if (((BPResult*)result)->containersCount() < containersCount_)
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
			return new BPRange(neighbours);
		}

		size_t BPResult::containersCount() const
		{
			return containersCount_;
		}

		const double* BPResult::containersWeights() const
		{
			return containersWeights_;
		}

		Result* BPResult::move(size_t item, size_t toContainer) const
		{
			checkContainerWeight(containersWeights_[toContainer] + context_->item(item));
			size_t fromContainer = findItem(item);
			if (fromContainer == toContainer)
				throw 1;

			/*bool** matrix = copyMatrix();
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
			}*/

			return new BPMoveResult(this, item, fromContainer, toContainer);;
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

			containersWeights[firstContainer]  -= context_->item(firstItem);
			containersWeights[secondContainer] += context_->item(firstItem);
			containersWeights[firstContainer]  += context_->item(secondItem);
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

		std::string BPResult::toGeneralString() const {
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
	}
}


using namespace fx;
using namespace fx::bin_packing;

int main()
{
	double testData[250] = { 42,
69,
67,
57,
93,
90,
38,
36,
45,
42,
33,
79,
27,
57,
44,
84,
86,
92,
46,
38,
85,
33,
82,
73,
49,
70,
59,
23,
57,
72,
74,
69,
33,
42,
28,
46,
30,
64,
29,
74,
41,
49,
55,
98,
80,
32,
25,
38,
82,
30,
35,
39,
57,
84,
62,
50,
55,
27,
30,
36,
20,
78,
47,
26,
45,
41,
58,
98,
91,
96,
73,
84,
37,
93,
91,
43,
73,
85,
81,
79,
71,
80,
76,
83,
41,
78,
70,
23,
42,
87,
43,
84,
60,
55,
49,
78,
73,
62,
36,
44,
94,
69,
32,
96,
70,
84,
58,
78,
25,
80,
58,
66,
83,
24,
98,
60,
42,
43,
43,
39,
97,
57,
81,
62,
75,
81,
23,
43,
50,
38,
60,
58,
70,
88,
36,
90,
37,
45,
45,
39,
44,
53,
70,
24,
82,
81,
47,
97,
35,
65,
74,
68,
49,
55,
52,
94,
95,
29,
99,
20,
22,
25,
49,
46,
98,
59,
98,
60,
23,
72,
33,
98,
80,
95,
78,
57,
67,
53,
47,
53,
36,
38,
92,
30,
80,
32,
97,
39,
80,
72,
55,
41,
60,
67,
53,
65,
95,
20,
66,
78,
98,
47,
100,
85,
53,
53,
67,
27,
22,
61,
43,
52,
76,
64,
61,
29,
30,
46,
79,
66,
27,
79,
98,
90,
22,
75,
57,
67,
36,
70,
99,
48,
43,
45,
71,
100,
88,
48,
27,
39,
38,
100,
60,
42,
20,
69,
24,
23,
92,
32 };
	BPContext context(150, 250, testData);
	Result* result = hillClimbing(context);
	delete result;
	
	std::cin.get();
	return 0;
}
