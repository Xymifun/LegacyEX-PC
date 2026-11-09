#ifndef GAME_ITEM_NOT_NULL_PTR_HPP
#define GAME_ITEM_NOT_NULL_PTR_HPP

#include <boost/shared_ptr.hpp>

template <class T>
class not_null_ptr : public boost::shared_ptr<T>
{
public:
    not_null_ptr() {}
    not_null_ptr(T* pObject) : boost::shared_ptr<T>(pObject) {}
    not_null_ptr(const boost::shared_ptr<T>& pObject) : boost::shared_ptr<T>(pObject) {}
};

#endif
