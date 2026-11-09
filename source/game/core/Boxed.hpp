#ifndef GAME_CORE_BOXED_HPP
#define GAME_CORE_BOXED_HPP

#include <typeinfo>
#include <string>

class Boxed
{
public:
    Boxed();
    explicit Boxed(const std::type_info* type);
    virtual ~Boxed();
    virtual bool isA(const std::type_info* type) const;
    virtual bool isA(const std::type_info& type) const;
    virtual unsigned int hashOf() const = 0;
    virtual bool operator==(const Boxed* value) const = 0;
    virtual bool operator==(Boxed* value) const;
    virtual bool operator!=(const Boxed* value) const;
    virtual bool operator!=(Boxed* value) const;
    virtual void setValue(const Boxed* value) = 0;
    virtual std::wstring toString() const = 0;

protected:
    const std::type_info* mValueClass;
};

#endif
