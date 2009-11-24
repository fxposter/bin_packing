#ifndef CONTEXT_H
#define CONTEXT_H

namespace bin_packing
{
	class ResultInterface;
	class RandomGenerator;

	class Context
	{
	public:
		Context(double containerCapacity, size_t itemsCount, double* items, size_t bestKnownNumberOfContainers);

		bool less(const ResultInterface& firstResult, const ResultInterface& secondResult) const;
		virtual ResultInterface* createRandomResult() const;

		size_t itemsCount() const;
		double itemWeight(size_t i) const;
		double containerCapacity() const;
        size_t bestKnownNumberOfContainers() const;

	private:
		double containerCapacity_;
		size_t itemsCount_;
		double* items_;
        size_t bestKnownNumberOfContainers_;

		RandomGenerator* generator_;
	};
}

#endif // CONTEXT_H
