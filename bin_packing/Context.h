namespace bin_packing
{
	class ResultInterface;

	class Context
	{
	public:
		Context(double containerCapacity, size_t itemsCount, double* items);

		bool less(const ResultInterface& firstResult, const ResultInterface& secondResult) const;
		virtual ResultInterface* createRandomResult() const;

		inline size_t itemsCount() const;
		inline double itemWeight(size_t i) const;
		inline double containerCapacity() const;

	private:
		double containerCapacity_;
		size_t itemsCount_;
		double* items_;
	};
}