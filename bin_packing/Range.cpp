#include "Range.h"

namespace bin_packing
{
	Range::Range(const std::vector<ResultInterface*>& neighbours) : neighbours_(neighbours), index_(0)
	{
	}

	bool Range::empty() const
	{
		return index_ == neighbours_.size();
	}

	ResultInterface* Range::next()
	{
		return neighbours_[index_++];
	}
}
