namespace bin_packing
{
	class ResultInterface;
	class Context;

	ResultInterface* hillClimbing(Context& context);
    ResultInterface* tabuSearch(Context& context);
}