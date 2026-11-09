#ifndef GAME_CORE_SUBCLASS_HPP
#define GAME_CORE_SUBCLASS_HPP

#include <string>

class SubClass
{
public:
    SubClass(const std::string& name, unsigned long long id);
    void addParent(unsigned long long id);
    void justFlag();
    static void checkDerivations();
};

#endif
