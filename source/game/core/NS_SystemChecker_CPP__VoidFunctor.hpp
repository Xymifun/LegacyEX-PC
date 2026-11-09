#ifndef GAME_CORE_NS_SYSTEMCHECKER_CPP__VOIDFUNCTOR_HPP
#define GAME_CORE_NS_SYSTEMCHECKER_CPP__VOIDFUNCTOR_HPP

namespace NS_SystemChecker_CPP
{
class VoidFunctor
{
public:
    virtual ~VoidFunctor() {}
    virtual void operator()() const = 0;
};
}

#endif
