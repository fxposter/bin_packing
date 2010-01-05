#include <vector>
#include <utility>
#include <algorithm>
#include <ctime>
#include <iostream>


typedef std::pair<size_t, double> ItemPair;

class ItemPairWeightComparator
{
public:
    bool operator()(const ItemPair& first, const ItemPair& second);
};

bool ItemPairWeightComparator::operator()(const ItemPair& first, const ItemPair& second)
{
    return first.second > second.second;
}

class BinError : public std::exception {};

class Bin
{
public:
    friend bool operator==(const Bin& b1, const Bin& b2);

    Bin(double capacity);
    Bin(double capacity, const ItemPair& item);

    void insert(const ItemPair& item);
    bool replaceOne(const ItemPair& item, ItemPair& replaced);
    bool replaceTwo(const ItemPair& item, ItemPair& firstReplaced, ItemPair& secondReplaced);
    void remove(const ItemPair& item);

    double fits(const ItemPair& item) const;
    bool canInsert(const ItemPair& item) const;
    bool has(const ItemPair& item) const;
    double size() const;
    const std::vector<ItemPair>& items() const;
    double fitness() const;

private:
    std::vector<ItemPair> items_;
    double size_;
    double capacity_;
};

Bin::Bin(double capacity) : capacity_(capacity), size_(0.0)
{
}

Bin::Bin(double capacity, const ItemPair& item) : capacity_(capacity), size_(0.0)
{
    insert(item);
}

double Bin::fitness() const
{
    return (size() / capacity_) * (size() / capacity_);
}

void Bin::insert(const ItemPair& item)
{
    if (item.second + size_ > capacity_)
	    throw BinError();
    if (has(item))
	    throw BinError();

    std::vector< ItemPair >::iterator i = std::lower_bound(items_.begin(), items_.end(), item, ItemPairWeightComparator());
    items_.insert(i, item);
    size_ += item.second;
}

bool Bin::canInsert(const ItemPair& item) const
{
    if (item.second + size_ > capacity_)
	    return false;
    if (has(item))
	    throw BinError();
    return true;
}

bool Bin::replaceOne(const ItemPair& item, ItemPair& replaced)
{
    if (has(item))
	    throw BinError();
	
    size_t size = items_.size();
    for (size_t i = 0; i < size; ++i) {
	    if (items_[i].second >= item.second)
		    break;
			
	    if (size_ + item.second - items_[i].second <= capacity_) {
            replaced = items_[i];
		    remove(replaced);
            insert(item);
		    return true;
	    }
    }
	
    return false;
}

bool Bin::replaceTwo(const ItemPair& item, ItemPair& firstReplaced, ItemPair& secondReplaced)
{
    if (has(item))
	    throw BinError();
	
    size_t size = items_.size();
    for (size_t i = 0; i < size - 1; ++i)
    {
        for (size_t j = i + 1; j < size; ++j)
        {
            double itemsSize = items_[i].second + items_[j].second;
            if (itemsSize >= item.second)
                break;

            if (size_ + item.second - itemsSize <= capacity_)
            {
                firstReplaced = items_[i];
                secondReplaced = items_[j];
		        remove(firstReplaced);
                remove(secondReplaced);
                insert(item);
		        return true;
	        }
        }
    }
	
    return false;
}

void Bin::remove(const ItemPair& item)
{
    size_t size = items_.size();
    for (size_t i = 0; i < size; ++i)
        if (items_[i].first == item.first)
        {
            size_ -= items_[i].second;
            items_.erase(items_.begin() + i);
            return;
        }

    throw BinError();
}

double Bin::fits(const ItemPair& item) const
{
    if (has(item))
	    throw BinError();
    return capacity_ - (item.second + size_);
}

bool Bin::has(const ItemPair& item) const
{
    size_t size = items_.size();
    for (size_t i = 0; i < size; ++i)
        if (items_[i].first == item.first)
            return true;
    return false;
}

double Bin::size() const
{
    return size_;
}

const std::vector<ItemPair>& Bin::items() const
{
    return items_;
}

bool operator==(const Bin& b1, const Bin& b2)
{
    return b1.items_ == b2.items_;
}

class Solution
{
public:
    friend bool operator==(const Solution& s1, const Solution& s2);

    Solution(std::vector<Bin> bins);
    double fitness() const;
    std::vector<Bin>& bins();
    const std::vector<Bin>& bins() const;
private:
    std::vector<Bin> bins_;
    double fitness_;
};

Solution::Solution(std::vector<Bin> bins) : bins_(bins), fitness_(0.0)
{
    size_t size = bins_.size();
    if (size > 0)
    {
        for (size_t i = 0; i < size; ++i)
            fitness_ += bins_[i].fitness();
        fitness_ /= bins_.size();
    }
}

double Solution::fitness() const
{
    return fitness_;
}

std::vector<Bin>& Solution::bins()
{
    return bins_;
}

const std::vector<Bin>& Solution::bins() const
{
    return bins_;
}
	
bool operator==(const Solution& s1, const Solution& s2)
{
    return s1.fitness() == s2.fitness();
    // return s1.bins_ == s2.bins_;
}

class Context
{
public:
    Context(double containerCapacity, const std::vector<ItemPair>& items, size_t bestKnownNumberOfContainers)
	    : containerCapacity_(containerCapacity), items_(items), bestKnownNumberOfContainers_(bestKnownNumberOfContainers)
    {
    }

    double containerCapacity() const
    {
	    return containerCapacity_;
    }

    const std::vector<ItemPair>& items() const
    {
	    return items_;
    }

    size_t bestKnownNumberOfContainers() const
    {
	    return bestKnownNumberOfContainers_;
    }
private:
    double containerCapacity_;
    std::vector<ItemPair> items_;
    size_t bestKnownNumberOfContainers_;
};

class FitAlgorithm
{
public:
    virtual void fill(std::vector<Bin>& bins) const = 0;
};

class FirstFitAlgorithm : public FitAlgorithm
{
public:
    FirstFitAlgorithm(const Context& context) : items_(context.items()), capacity_(context.containerCapacity())
    {
    }

    void fill(std::vector<Bin>& bins) const
    {
	    for (std::vector<ItemPair>::const_iterator item = items_.begin(); item != items_.end(); ++item) {
		    bool inserted = false;
		    for (std::vector<Bin>::iterator bin = bins.begin(); bin != bins.end(); ++bin) {
			    try {
				    if (static_cast<double>(std::rand()) / RAND_MAX >= 0.5) {
					    bin->insert(*item);
					    inserted = true;
					    break;
				    }
			    } catch (BinError&) {
			    }
		    }
		    if (!inserted) {
			    bins.push_back(Bin(capacity_, *item));
		    }
	    }
    }

protected:
    std::vector<ItemPair> items_;
    double capacity_;
};

class FirstFitDecreasingAlgorithm : public FirstFitAlgorithm
{
public:
    FirstFitDecreasingAlgorithm(const Context& context) : FirstFitAlgorithm(context)
    {
	    std::sort(items_.begin(), items_.end(), ItemPairWeightComparator());
    }
};

class Random
{
public:
    int nextInt(int max)
    {
        return static_cast<int>(nextDouble() * (max - 1));
    }

    size_t nextUnsignedInt(size_t max)
    {
        return static_cast<size_t>(nextDouble() * (max - 1));
    }

    double nextDouble()
    {
        return static_cast<double>(std::rand()) / RAND_MAX;
    }
};

#include <map>
#include <cmath>

class AntColonyOptimization
{
public:
    static const double P_BETTER;
    static const double P_C;
    static const double BETA;
    static const double EVAPORATION_PARAMETER;
    static const size_t ANTS_COUNT = 10;

    AntColonyOptimization(Context& context) : context_(context) {
        size_t n = context_.items().size();
        double pbest = 1.0; // std::pow(0.05, 1.0 / n);

        minimalPheromone_ = 0;// (1.0 / (1.0 - EVAPORATION_PARAMETER) * (1.0 - pbest)) / ((n / 2.0 - 1.0) * pbest);
    }

    void createPopulation()
    {
        size_t MAX_BEST_SOLUTION_COUNT = 50;
        size_t MAX_NEW_SOLUTION_COUNT = 5;
        size_t newSolutionCount = 0;
        size_t bestSolutionCount = MAX_BEST_SOLUTION_COUNT;

        initializePheromoneTrail();

        Solution bestSolution = Solution(std::vector<Bin>());
        for (size_t gen = 0; gen < 1000; ++gen)
        {
            std::vector<Solution> solutions;
            for (size_t i = 0; i < ANTS_COUNT; ++i)
            {
                std::vector<Bin> bins;
                std::vector<ItemPair> leftOutItems = context_.items();
                std::sort(leftOutItems.begin(), leftOutItems.end(), ItemPairWeightComparator());
                std::vector<ItemPair> itemsForInclusion;
                while (!leftOutItems.empty())
                {
                    Bin bin(context_.containerCapacity());
                    while (true)
                    {
                        fillItemsForInclusion(bin, leftOutItems, itemsForInclusion);
                        if (itemsForInclusion.empty())
                            break;

                        double* probabilities = probabilitiesOfInclusion(bin, itemsForInclusion);
                        size_t itemForInclusion = selectItem(probabilities, itemsForInclusion.size());
                        bin.insert(itemsForInclusion[itemForInclusion]);
                        leftOutItems.erase(std::find(leftOutItems.begin(), leftOutItems.end(), itemsForInclusion[itemForInclusion]));
                        delete[] probabilities;
                        itemsForInclusion.clear();
                    }
                    bins.push_back(bin);
                }
                Solution* solution = mutation(Solution(bins));
                solutions.push_back(*solution);
                delete solution;
            }
            size_t best = findBestSolution(solutions);

            if (solutions[best].fitness() > bestSolution.fitness()) {
                bestSolution = solutions[best];
            }

            std::cout << gen << " - " << bestSolution.bins().size() << " - " << solutions[best].bins().size() << " - " << bestSolution.fitness() << " - " << solutions[best].fitness() << '\n';
            if (bestSolution.bins().size() == context_.bestKnownNumberOfContainers())
                break;

            updatePheromoneTrail(bestSolution);
        }
    }

    void initializePheromoneTrail()
    {
        const std::vector<ItemPair>& items = context_.items();
        for (size_t j = 0; j < items.size(); ++j)
        {
            for (size_t k = j + 1; k < items.size(); ++k)
            {
                double jh = heuristic(items[j]);
                double kh = heuristic(items[k]);
                pheromoneTrail_[jh][kh] = pheromoneTrail_[kh][jh] = 0; // 1.0 / (1.0 - EVAPORATION_PARAMETER);
            }
        }
    }

    void updatePheromoneTrail(const Solution& solution)
    {
        for (std::map< double, std::map<double, double> >::iterator i = pheromoneTrail_.begin(); i != pheromoneTrail_.end(); ++i)
            for (std::map<double, double>::iterator j = i->second.begin(); j != i->second.end(); ++j)
                j->second *= EVAPORATION_PARAMETER;

        for (size_t i = 0; i < solution.bins().size(); ++i)
        {
            const std::vector<ItemPair>& items = solution.bins()[i].items();
            for (size_t j = 0; j < items.size(); ++j)
            {
                for (size_t k = j + 1; k < items.size(); ++k)
                {
                    double jh = heuristic(items[j]);
                    double kh = heuristic(items[k]);
                    pheromoneTrail_[jh][kh] += solution.fitness();
                    pheromoneTrail_[kh][jh] += solution.fitness();
                }
            }
        }

        for (std::map< double, std::map<double, double> >::iterator i = pheromoneTrail_.begin(); i != pheromoneTrail_.end(); ++i)
            for (std::map<double, double>::iterator j = i->second.begin(); j != i->second.end(); ++j)
                if (j->second < minimalPheromone_)
                    j->second = minimalPheromone_;
    }

    size_t findBestSolution(const std::vector<Solution>& solutions)
    {
        size_t best = 0;
        double bestFitness = solutions[0].fitness();

        size_t size = solutions.size();
        for (size_t i = 1; i < size; ++i)
        {
            double fitness = solutions[i].fitness();
            if (bestFitness < fitness)
            {
                best = i;
                bestFitness = fitness;
            }
        }

        return best;
    }
    
    void fillItemsForInclusion(const Bin& bin, const std::vector<ItemPair>& leftOutItems, std::vector<ItemPair>& result)
    {
        size_t size = leftOutItems.size();
        for (size_t i = 0; i < size; ++i)
            if (bin.canInsert(leftOutItems[i]))
                result.push_back(leftOutItems[i]);
    }

    double* probabilitiesOfInclusion(const Bin& bin, const std::vector<ItemPair>& leftOutItems)
    {
        size_t size = leftOutItems.size();
        double* probabilities = new double[size];
        double sum = 0.0;
        for (size_t i = 0; i < size; ++i)
            sum += (probabilities[i] = pheromoneForBinAndItem(bin, leftOutItems[i]) * std::pow(heuristic(leftOutItems[i]), BETA));

        if (sum == 0)
            for (size_t i = 0; i < size; ++i)
                sum += (probabilities[i] = std::pow(heuristic(leftOutItems[i]), BETA));

        for (size_t i = 0; i < size; ++i)
            probabilities[i] /= sum;

        return probabilities;
    }

    size_t selectItem(double* probabilities, size_t size)
    {
        double random = random_.nextDouble();
        double sum = 0.0;
        for (size_t i = 0; i < size - 1; ++i)
        {
            sum += probabilities[i];
            if (random <= sum)
                return i;
        }
        return size - 1;
    }

    double pheromoneForBinAndItem(const Bin& bin, const ItemPair& item)
    {
        const std::vector<ItemPair>& items = bin.items();

        size_t size = items.size();
        if (size == 0)
            return 1.0;

        std::map<double, double>& pheromoneItemTrail = pheromoneTrail_[heuristic(item)];
        double result = 0.0;
        for (size_t i = 0; i < size; ++i)
        {
            result += pheromoneItemTrail[heuristic(items[i])];
        }

        return result / size;
    }

    double heuristic(const ItemPair& item)
    {
        return item.second;
    }


    Solution* mutation(Solution& solution)
    {
        std::vector<Bin> bins = solution.bins();
        std::vector<ItemPair> leftOutItems;

        size_t bestBin = 0, worstBin = 0;

        size_t size = 0;
        for (size_t i = 1; i < bins.size(); ++i)
        {
            if (bins[i].size() > bins[bestBin].size())
			    bestBin = i;
            if (bins[i].size() < bins[worstBin].size())
			    worstBin = i;
        }

        /*if (random_.nextDouble() <= 0.2)
        {
            leftOutItems.insert(leftOutItems.end(), bins[bestBin].items().begin(), bins[bestBin].items().end());
            bins.erase(bins.begin() + bestBin);
        }
        leftOutItems.insert(leftOutItems.end(), bins[worstBin].items().begin(), bins[worstBin].items().end());
        bins.erase(bins.begin() + worstBin);*/

        size_t count = static_cast<size_t>(std::sqrt(static_cast<double>(context_.items().size())));
        for (size_t i = 0; i < 5; ++i) {
            size_t index = findWorstBin(bins);
            Bin& bin = bins[index];
            leftOutItems.insert(leftOutItems.end(), bin.items().begin(), bin.items().end());
            bins.erase(bins.begin() + index);
        }

        for (size_t i = 0; i < 5; ++i) {
            size_t index = random_.nextUnsignedInt(bins.size());
            Bin& bin = bins[index];
            leftOutItems.insert(leftOutItems.end(), bin.items().begin(), bin.items().end());
            bins.erase(bins.begin() + index);
        }
        fit(bins, leftOutItems);

        return new Solution(bins);
    }

    size_t findWorstBin(const std::vector<Bin>& bins)
    {
        size_t worstBin = 0;
        for (size_t i = 1; i < bins.size(); ++i)
        {
            if (bins[i].size() < bins[worstBin].size())
			    worstBin = i;
        }
        return worstBin;
    }


private:

    void fit(std::vector<Bin>& bins, std::vector<ItemPair>& leftOutItems)
    {
        ItemPair first(0, 0), second(0, 0);
        for (size_t j = leftOutItems.size(); j > 0; --j) {
            ItemPair& item = leftOutItems[j - 1];
            for (std::vector<Bin>::iterator i = bins.begin(); i != bins.end(); ++i) {
                if (i->replaceOne(item, first)) {
                    leftOutItems.insert(leftOutItems.begin() + j - 1, first);
                    leftOutItems.erase(leftOutItems.begin() + j);
                    ++j;
                    break;
                } else if (i->replaceTwo(item, first, second)) {
                    leftOutItems.insert(leftOutItems.begin() + j - 1, first);
                    leftOutItems.insert(leftOutItems.begin() + j - 1, second);
                    leftOutItems.erase(leftOutItems.begin() + j + 1);
                    j += 2;
                    break;
                }
            }
        }

        std::sort(leftOutItems.begin(), leftOutItems.end(), ItemPairWeightComparator());

        for (std::vector<ItemPair>::const_iterator j = leftOutItems.begin(); j != leftOutItems.end(); ++j) {
            // ItemPair& item = leftOutItems[j - 1];
            double min = 0.0;
            std::vector<Bin>::iterator minIterator = bins.end();
            for (std::vector<Bin>::iterator i = bins.begin(); i != bins.end(); ++i) {
                double amount = i->fits(*j);
                if (amount >= 0) {
                    if (minIterator == bins.end() || amount < min) {
                        min = amount;
                        minIterator = i;
                    }
                }
            }
            if (minIterator != bins.end()) {
                minIterator->insert(*j);
            } else {
                bins.push_back(Bin(context_.containerCapacity(), *j));
            }
        }
    }

private:
    Context& context_;
    Random random_;
    std::map< double, std::map<double, double> > pheromoneTrail_;
    double minimalPheromone_;
};

const double AntColonyOptimization::P_BETTER = 0.8;//0.5;
const double AntColonyOptimization::P_C = 0.8;
const double AntColonyOptimization::BETA = 2.0;
const double AntColonyOptimization::EVAPORATION_PARAMETER = 0.75;

class GeneticAlgorithm
{
public:
    static double P_BETTER;
    static double P_C;

    GeneticAlgorithm(Context& context) : context_(context) {
    }

    void createPopulation()
    {
	    std::vector<Solution> population;
	    FirstFitDecreasingAlgorithm algorithm(context_);
		
	    while (population.size() < 100) {
		    std::vector<Bin> bins;
		    algorithm.fill(bins);
		    Solution solution(bins);

            if (!populationIncludeSolution(population, solution))
			    population.push_back(solution);
	    }

        size_t best = 0;
        for (size_t i = 1; i < population.size(); ++i)
            if (population[i].fitness() > population[best].fitness())
			    best = i;

        for (size_t generation = 0; generation < 1000; ) {
            Solution* solution = 0;
            if (random_.nextDouble() <= P_C)
            {
                size_t firstParent = binaryTournament(population);
                size_t secondParent = binaryTournament(population);
                solution = crossover(population[firstParent], population[secondParent]);
            }
            else
            {
                size_t winner = trinaryTournament(population);
                solution = mutation(population[winner]);
            }

            if (!populationIncludeSolution(population, *solution))
            {
                population.push_back(*solution);
                size_t worst = 0;
                for (size_t i = 1; i < population.size(); ++i)
                    if (population[i].fitness() < population[worst].fitness())
		                worst = i;
                size_t worstSolution = population[worst].bins().size();
                double worstSolutionFitness = population[worst].fitness();
                population.erase(population.begin() + worst);
                if (worst < best)
                    --best;
                if (solution->fitness() > population[best].fitness())
                    best = population.size() - 1;
                size_t bestSolution = population[best].bins().size();
                double bestSolutionFitness = population[best].fitness();
                std::cout << generation << ' ' << bestSolution << ' ' << worstSolution << ' ' << bestSolutionFitness << ' ' << worstSolutionFitness << '\n';
                ++generation;
            }
            delete solution;

            if (population[best].bins().size() == context_.bestKnownNumberOfContainers())
                break;
        }

        // std::cout << '\n' << population[best].bins().size() << '\n';
        // std::cin.get();
        int i = 10;
    }

    bool populationIncludeSolution(const std::vector<Solution>& population, const Solution& solution)
    {
        for (std::vector<Solution>::const_iterator s = population.begin(); s != population.end(); ++s) {
		    if (*s == solution) {
			    return true;
		    }
	    }
        return false;
    }

    Solution* crossover(const Solution& first, const Solution& second) {
	    const std::vector<Bin>& firstBins = first.bins();
	    std::vector<Bin> secondBins = second.bins();

        size_t firstBinStop1 = random_.nextUnsignedInt(firstBins.size() - 1);
        size_t firstBinStop2 = random_.nextUnsignedInt(firstBins.size());
        while (firstBinStop2 <= firstBinStop1)
            firstBinStop2 = random_.nextUnsignedInt(firstBins.size());

        size_t secondBinStop1 = random_.nextUnsignedInt(secondBins.size() - 1);
        size_t secondBinStop2 = random_.nextUnsignedInt(secondBins.size());
        while (secondBinStop2 <= secondBinStop1)
            secondBinStop2 = random_.nextUnsignedInt(secondBins.size());

        std::vector<ItemPair> insertedItems, leftOutItems;
        for (std::vector<Bin>::const_iterator i = firstBins.begin() + firstBinStop1; i != firstBins.begin() + firstBinStop2; ++i)
            insertedItems.insert(insertedItems.end(), i->items().begin(), i->items().end());
        
        secondBins.insert(secondBins.begin() + secondBinStop1, firstBins.begin() + firstBinStop1, firstBins.begin() + firstBinStop2);
        std::vector<size_t> binsToDelete;
        for (size_t i = 0; i < secondBins.size(); ++i) {
            if (i >= secondBinStop1 && i < secondBinStop1 + (firstBinStop2 - firstBinStop1))
                continue;

            Bin& bin = secondBins[i];
            bool toRemove = false;
            for (std::vector<ItemPair>::const_iterator j = insertedItems.begin(); j != insertedItems.end(); ++j) {
                try {
                    bin.remove(*j);
                    toRemove = true;
                } catch (BinError&) {
                }
            }
            if (toRemove) {
                leftOutItems.insert(leftOutItems.end(), bin.items().begin(), bin.items().end());
                // secondBins.erase(secondBins.begin() + i--);
                binsToDelete.push_back(i--);
            }
        }

        for (size_t i = 0; i < binsToDelete.size(); ++i)
            secondBins.erase(secondBins.begin() + (binsToDelete[i] - i));

        fit(secondBins, leftOutItems);
        return new Solution(secondBins);
    }

    Solution* mutation(Solution& solution)
    {
        std::vector<Bin> bins = solution.bins();
        std::vector<ItemPair> leftOutItems;

        size_t bestBin = 0, worstBin = 0;

        size_t size = 0;
        for (size_t i = 1; i < bins.size(); ++i)
        {
            if (bins[i].size() > bins[bestBin].size())
			    bestBin = i;
            if (bins[i].size() < bins[worstBin].size())
			    worstBin = i;
        }

        if (random_.nextDouble() <= 0.2)
        {
            leftOutItems.insert(leftOutItems.end(), bins[bestBin].items().begin(), bins[bestBin].items().end());
            bins.erase(bins.begin() + bestBin);
            if (bestBin < worstBin)
                --worstBin;
        }
        leftOutItems.insert(leftOutItems.end(), bins[worstBin].items().begin(), bins[worstBin].items().end());
        bins.erase(bins.begin() + worstBin);

        for (size_t i = 0; i < 5; ++i) {
            size_t index = random_.nextUnsignedInt(bins.size());
            Bin& bin = bins[index];
            leftOutItems.insert(leftOutItems.end(), bin.items().begin(), bin.items().end());
            bins.erase(bins.begin() + index);
        }
        fit(bins, leftOutItems);
        return new Solution(bins);
    }
private:
    size_t binaryTournament(const std::vector<Solution>& population)// , const Solution*& firstParent, const Solution*& secondParent)
    {
        return tournament(population, 4);
        /*int a = random_.nextUnsignedInt(population.size() - 1);
        int b = a + random_.nextUnsignedInt(population.size() - a);
        int c = random_.nextUnsignedInt(population.size() - 1);
        int d = c + random_.nextUnsignedInt(population.size() - c);
        if (population[a].fitness() > population[b].fitness() && random_.nextDouble() <= P_BETTER)
        {
            firstParent = &population[a];
        }
        else
        {
            firstParent = &population[b];
        }

        if (population[c].fitness() > population[d].fitness() && random_.nextDouble() <= P_BETTER)
        {
            secondParent = &population[c];
        }
        else
        {
            secondParent = &population[d];
        }*/
    }

    size_t tournament(const std::vector<Solution>& population, size_t tournamentSize)
    {
        size_t randomSolutions[8]; // TODO potential bug :)
        for (size_t i = 0; i < tournamentSize; ++i)
            randomSolutions[i] = random_.nextUnsignedInt(population.size());
        
        size_t solutionsCount = tournamentSize;
        while (solutionsCount != 1)
        {
            for (size_t i = 0; i < solutionsCount; i += 2)
            {
                size_t winner = 0;
                if (population[randomSolutions[i]].fitness() > population[randomSolutions[i + 1]].fitness() && random_.nextDouble() <= P_BETTER)
                    winner = i;
                else
                    winner = i + 1;
                randomSolutions[i / 2] = randomSolutions[winner];
            }
            solutionsCount /= 2;
        }
        return randomSolutions[0];
    }

    size_t trinaryTournament(const std::vector<Solution>& population)
    {
        return tournament(population, 8);
    }

    void fit(std::vector<Bin>& bins, std::vector<ItemPair>& leftOutItems)
    {
        ItemPair first(0, 0), second(0, 0);
        for (size_t j = leftOutItems.size(); j > 0; --j) {
            ItemPair& item = leftOutItems[j - 1];
            for (std::vector<Bin>::iterator i = bins.begin(); i != bins.end(); ++i) {
                if (i->replaceOne(item, first)) {
                    leftOutItems.insert(leftOutItems.begin() + j - 1, first);
                    leftOutItems.erase(leftOutItems.begin() + j);
                    ++j;
                    break;
                } else if (i->replaceTwo(item, first, second)) {
                    leftOutItems.insert(leftOutItems.begin() + j - 1, first);
                    leftOutItems.insert(leftOutItems.begin() + j - 1, second);
                    leftOutItems.erase(leftOutItems.begin() + j + 1);
                    j += 2;
                    break;
                }
            }
        }

        for (std::vector<ItemPair>::const_iterator j = leftOutItems.begin(); j != leftOutItems.end(); ++j) {
            // ItemPair& item = leftOutItems[j - 1];
            double min = 0.0;
            std::vector<Bin>::iterator minIterator = bins.end();
            for (std::vector<Bin>::iterator i = bins.begin(); i != bins.end(); ++i) {
                double amount = i->fits(*j);
                if (amount >= 0) {
                    if (minIterator == bins.end() || amount < min) {
                        min = amount;
                        minIterator = i;
                    }
                }
            }
            if (minIterator != bins.end()) {
                minIterator->insert(*j);
            } else {
                bins.push_back(Bin(context_.containerCapacity(), *j));
            }
        }
    }

private:
    Context& context_;
    Random random_;
};

double GeneticAlgorithm::P_BETTER = 0.8;//0.5;
double GeneticAlgorithm::P_C = 0.8;

#include <fstream>
#include <string>

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
				std::vector<ItemPair> items;
                for (size_t j = 0; j < itemsCount; ++j)
                {
                    double value;
                    file >> value;
                    items.push_back(ItemPair(j, value));
                }
				file.close();
				return new Context(containerCapacity, items, bestKnownNumberOfContainers);
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
    DataLoader loader("data/binpack3.txt");
    Context* context = loader.load(1);
    /*std::vector<ItemPair> items;
    items.push_back(ItemPair(0, 7));
    items.push_back(ItemPair(1, 5));
    items.push_back(ItemPair(2, 3));
    items.push_back(ItemPair(3, 9));
    items.push_back(ItemPair(4, 1));
    items.push_back(ItemPair(5, 6));
    items.push_back(ItemPair(6, 5));
    items.push_back(ItemPair(7, 4));
    Context context(10, items, 4);*/

    size_t t = static_cast<size_t>(std::time(0));

    // std::cout << t << '\n';

    // 1260662779
    std::srand(1260662779);
    AntColonyOptimization antColony(*context);
    antColony.createPopulation();
    return 0;
}