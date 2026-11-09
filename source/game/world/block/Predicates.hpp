#ifndef GAME_WORLD_BLOCK_PREDICATES_HPP
#define GAME_WORLD_BLOCK_PREDICATES_HPP

#include "game/world/block/Predicate.hpp"

#include <vector>

template <class T>
class Predicates
{
public:
    class ConstantPredicate : public Predicate<T>
    {
    public:
        ConstantPredicate(bool value) : mValue(value) {}

        virtual bool apply(T input) const;
        virtual Predicate<T>* copy() const;
        virtual ~ConstantPredicate();

    private:
        bool mValue;
    };

    class FunctionPredicate : public Predicate<T>
    {
    public:
        FunctionPredicate(bool (*function)(T)) : mFunction(function) {}

        virtual bool apply(T input) const;
        virtual ~FunctionPredicate();

    private:
        bool (*mFunction)(T);
    };

    class EqualToValuePredicate : public Predicate<T>
    {
    public:
        EqualToValuePredicate(T value) : mValue(value) {}

        virtual bool apply(T input) const;
        virtual ~EqualToValuePredicate();

    private:
        T mValue;
    };

    class ConnectivePredicate : public Predicate<T>
    {
    public:
        enum eOperator
        {
            eOperator_And,
            eOperator_Xor
        };

        ConnectivePredicate(eOperator op, const Predicate<T>** predicates, unsigned int count);

        virtual bool apply(T input) const;
        virtual ~ConnectivePredicate();

    private:
        eOperator mOperator;
        std::vector<const Predicate<T>*> mPredicates;
    };

    static Predicate<T>* alwaysTrue;
};

template <class T>
Predicate<T>* Predicates<T>::alwaysTrue = new typename Predicates<T>::ConstantPredicate(true);

template <class T>
bool Predicates<T>::ConstantPredicate::apply(T input) const
{
    return mValue;
}

template <class T>
Predicate<T>* Predicates<T>::ConstantPredicate::copy() const
{
    return new ConstantPredicate(mValue);
}

template <class T>
Predicates<T>::ConstantPredicate::~ConstantPredicate()
{
}

template <class T>
bool Predicates<T>::FunctionPredicate::apply(T input) const
{
    return mFunction(input);
}

template <class T>
Predicates<T>::FunctionPredicate::~FunctionPredicate()
{
}

template <class T>
bool Predicates<T>::EqualToValuePredicate::apply(T input) const
{
    return mValue == input;
}

template <class T>
Predicates<T>::EqualToValuePredicate::~EqualToValuePredicate()
{
}

template <class T>
Predicates<T>::ConnectivePredicate::ConnectivePredicate(typename Predicates<T>::ConnectivePredicate::eOperator op,
                                                        const Predicate<T>** predicates, unsigned int count)
    : mOperator(op),
      mPredicates()
{
    for (unsigned int i = 0; i < count; ++i)
    {
        mPredicates.push_back(predicates[i]);
    }
}

template <class T>
bool Predicates<T>::ConnectivePredicate::apply(T input) const
{
    bool result = true;

    if (mOperator == eOperator_Xor)
    {
        result = false;

        for (typename std::vector<const Predicate<T>*>::const_iterator it = mPredicates.begin();
             it != mPredicates.end(); ++it)
        {
            result = result ^ (*it)->apply(input);
        }
    }
    else
    {
        for (typename std::vector<const Predicate<T>*>::const_iterator it = mPredicates.begin();
             it != mPredicates.end(); ++it)
        {
            result = result && (*it)->apply(input);
        }
    }

    return result;
}

template <class T>
Predicates<T>::ConnectivePredicate::~ConnectivePredicate()
{
}

#endif
