#ifndef NN_UTIL_UTIL_STATICASSERT_H_
#define NN_UTIL_UTIL_STATICASSERT_H_

#include <boost/static_assert.hpp>

#define NN_STATIC_ASSERT(condition) BOOST_STATIC_ASSERT(condition)
#define NN_STATIC_ASSERT_IS_POD(type)

#endif
