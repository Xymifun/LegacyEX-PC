#ifndef GAME_WORLD_BLOCK_PREDICATE_HPP
#define GAME_WORLD_BLOCK_PREDICATE_HPP

#include <cstddef>

template <class T>
class Predicate
{
public:
    virtual bool apply(T input) const = 0;
    virtual Predicate<T>* copy() const;
    virtual ~Predicate() {}
};

template <class T>
Predicate<T>* Predicate<T>::copy() const
{
    return NULL;
}

#endif
