#include "game/core/Cartesian__Product.hpp"

namespace Cartesian
{

Product::~Product()
{
}

Product::ProductIterator::ProductIterator(std::vector<const std::vector<Boxed*>*>* iterables)
{
    mIndex = -2;
    mIterables = iterables;
    mIterators = arrayWithLength<std::vector<Boxed*>::const_iterator>(iterables->size(), true);

    for (unsigned int i = 0; i < mIterables->size(); i++)
    {
        mIterators.data[i] = mIterables->at(i)->begin();
    }

    mResults = arrayWithLength<Boxed*>(mIterators.length, true);
}

bool Product::ProductIterator::hasNext()
{
    if (mIndex == -2)
    {
        mIndex = 0;

        for (unsigned int i = 0; i < mIterators.length; i++)
        {
            std::vector<Boxed*>::const_iterator iterator = mIterators.data[i];

            if (iterator == mIterables->at(i)->end())
            {
                mIndex = -1;
                break;
            }
        }

        return true;
    }

    if (mIndex >= (int)mIterators.length)
    {
        for (mIndex = (int)mIterators.length - 1; mIndex >= 0; mIndex--)
        {
            std::vector<Boxed*>::const_iterator iterator = mIterators.data[mIndex];

            if (iterator != mIterables->at(mIndex)->end())
            {
                break;
            }

            if (mIndex == 0)
            {
                mIndex = -1;
                break;
            }

            iterator = mIterables->at(mIndex)->begin();
            mIterators.data[mIndex] = iterator;

            if (iterator == mIterables->at(mIndex)->end())
            {
                mIndex = -1;
                break;
            }
        }
    }

    return mIndex >= 0;
}

}
