namespace bin_packing
{
	class Context;

	class ResultInterface
	{
	public:
		virtual ~ResultInterface() {};
		virtual NeighbourRange* neighbours() const = 0;

		virtual size_t containersCount() const = 0;
		virtual const double* containersWeights() const = 0;

		virtual std::string toString() const = 0;
		virtual std::string toGeneralString() const = 0;

		virtual ResultInterface* clone() const = 0;

		virtual const Context* context() const = 0;
	};
}
