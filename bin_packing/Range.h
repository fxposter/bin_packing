#ifndef RANGE_H
#define RANGE_H

#include <vector>

namespace bin_packing
{
	class ResultInterface;

	class Range
	{
	public:
		Range(const std::vector<ResultInterface*>& neighbours);
		bool empty() const;
		ResultInterface* next();

	private:
		size_t index_;
		std::vector<ResultInterface*> neighbours_;
	};
}

#endif // RANGE_H
