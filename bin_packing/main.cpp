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
    return first.second < second.second;
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
    return size() / capacity_;
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

bool Bin::replaceOne(const ItemPair& item, ItemPair& replaced)
{
    if (has(item))
	    throw BinError();
		
    for (std::vector< ItemPair >::iterator i = items_.begin(); i != items_.end(); ++i) {
	    if (i->second >= item.second)
		    break;
			
	    if (size_ + item.second - i->second <= capacity_) {
            replaced = *i;
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
		
    for (std::vector< ItemPair >::iterator i = items_.begin(); i != items_.end(); ++i) {
        for (std::vector< ItemPair >::iterator j = i + 1; j != items_.end(); ++j) {
            if (i->second + j->second >= item.second)
		        break;
    			
	        if (size_ + item.second - i->second - j->second <= capacity_) {
                firstReplaced = *i;
                secondReplaced = *j;
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
    std::vector< ItemPair >::iterator i = std::find(items_.begin(), items_.end(), item);
    if (i == items_.end())
	    throw BinError();
    size_ -= i->second;
    items_.erase(i);
}

double Bin::fits(const ItemPair& item) const
{
    if (has(item))
	    throw BinError();
    return capacity_ - (item.second + size_);
}

bool Bin::has(const ItemPair& item) const
{
    return std::find(items_.begin(), items_.end(), item) != items_.end();
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
    for (std::vector<Bin>::const_iterator i = bins_.begin(); i != bins_.end(); ++i)
        fitness_ += i->fitness();
    fitness_ /= bins_.size();
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
    return s1.bins_ == s2.bins_;
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
        return static_cast<int>(nextDouble() * max);
    }

    size_t nextUnsignedInt(size_t max)
    {
        return static_cast<size_t>(nextDouble() * max);
    }

    double nextDouble()
    {
        return static_cast<double>(std::rand()) / RAND_MAX;
    }
};

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
		
	    while (population.size() < 10) {
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

        for (size_t generation = 0; generation < 10; ) {
            if (random_.nextDouble() <= P_C)
            {
                const Solution* firstParent = 0;
                const Solution* secondParent = 0;
                binaryTournament(population, firstParent, secondParent);
                Solution solution = crossover(*firstParent, *secondParent);
                if (!populationIncludeSolution(population, solution))
                {
                    population.push_back(solution);
                    size_t worst = 0;
                    for (size_t i = 1; i < population.size(); ++i)
                        if (population[i].fitness() < population[worst].fitness())
			                worst = i;
                    population.erase(population.begin() + worst);
                    if (worst < best)
                        --best;
                    if (solution.fitness() > population[best].fitness())
                        best = population.size() - 1;
                    ++generation;
                }
            }
            else
            {
                size_t a = random_.nextUnsignedInt(population.size());
                Solution solution = mutation(population[a]);
                if (!populationIncludeSolution(population, solution))
                {
                    population.push_back(solution);
                    size_t worst = 0;
                    for (size_t i = 1; i < population.size(); ++i)
                        if (population[i].fitness() < population[worst].fitness())
			                worst = i;
                    population.erase(population.begin() + worst);
                    if (worst < best)
                        --best;
                    if (solution.fitness() > population[best].fitness())
                        best = population.size() - 1;
                    ++generation;
                }
            }
	        
        }
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

    Solution crossover(const Solution& first, const Solution& second) {
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
        return Solution(secondBins);
        /*mutation(secondBins);
        int i = 10;*/
    }

    Solution mutation(Solution& solution)
    {
        std::vector<Bin> bins = solution.bins();
        std::vector<ItemPair> leftOutItems;
        for (size_t i = 0; i < 3; ++i) {
            size_t index = random_.nextUnsignedInt(bins.size() - 1);
            Bin& bin = bins[index];
            leftOutItems.insert(leftOutItems.end(), bin.items().begin(), bin.items().end());
            bins.erase(bins.begin() + index);
        }
        fit(bins, leftOutItems);
        return Solution(bins);
    }
private:
    void binaryTournament(const std::vector<Solution>& population, const Solution*& firstParent, const Solution*& secondParent)
    {
        int a = random_.nextUnsignedInt(population.size() - 1);
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
        }
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
                    leftOutItems.erase(leftOutItems.begin() + j);
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

double GeneticAlgorithm::P_BETTER = 0.5;
double GeneticAlgorithm::P_C = 0.5;

int main()
{
    std::vector<ItemPair> items;
    items.push_back(ItemPair(0, 7));
    items.push_back(ItemPair(1, 5));
    items.push_back(ItemPair(2, 3));
    items.push_back(ItemPair(3, 9));
    items.push_back(ItemPair(4, 1));
    items.push_back(ItemPair(5, 6));
    items.push_back(ItemPair(6, 5));
    items.push_back(ItemPair(7, 4));
    Context context(10, items, 4);

    size_t t = static_cast<size_t>(std::time(0));

    std::cout << t << '\n';

    std::srand(1260662779);
    GeneticAlgorithm ga(context);
    ga.createPopulation();
    return 0;
}