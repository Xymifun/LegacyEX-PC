#ifndef GAME_WORLD_BLOCK_ITERATOR_HPP
#define GAME_WORLD_BLOCK_ITERATOR_HPP

template <class T>
class Iterator
{
public:
    virtual ~Iterator() {}

    virtual bool hasNext() = 0;
    virtual T next() = 0;
};

#endif
