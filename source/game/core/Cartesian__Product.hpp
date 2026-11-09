#ifndef GAME_CORE_CARTESIAN__PRODUCT_HPP
#define GAME_CORE_CARTESIAN__PRODUCT_HPP

#include <vector>

#include "game/world/biome/arrayWithLength.hpp"

class Boxed;

namespace Cartesian
{

class Product
{
public:
    class ProductIterator
    {
    public:
        ProductIterator(std::vector<const std::vector<Boxed*>*>* iterables);
        virtual ~ProductIterator()
        {
            delete[] mResults.data;
            delete[] mIterators.data;
        }

        bool hasNext();
        arrayWithLength<Boxed*> next()
        {
            for (; mIndex < (int)mIterators.length; mIndex++)
            {
                mResults[mIndex] = *mIterators[mIndex]++;
            }

            return mResults;
        }

    private:
        int mIndex;
        std::vector<const std::vector<Boxed*>*>* mIterables;
        arrayWithLength<std::vector<Boxed*>::const_iterator> mIterators;
        arrayWithLength<Boxed*> mResults;
    };

    Product(std::vector<const std::vector<Boxed*>*>* iterables) : mIterables(iterables) {}

    virtual ~Product();

    ProductIterator* iterator() { return new ProductIterator(mIterables); }

private:
    std::vector<const std::vector<Boxed*>*>* mIterables;
};

}

#endif
