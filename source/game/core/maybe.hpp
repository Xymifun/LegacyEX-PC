#ifndef GAME_CORE_MAYBE_HPP
#define GAME_CORE_MAYBE_HPP

template <class T>
class maybe
{
public:
    maybe() : hasValue(false), value() {}

    maybe(const T& value) : hasValue(true), value(value) {}

    bool isSet() const { return hasValue; }

    const T& get() const { return value; }

    T otherwise(T fallback) const { return hasValue ? value : fallback; }

    maybe<T> otherwise(maybe<T> fallback) const { return hasValue ? maybe<T>(value) : fallback; }

    template <class U>
    maybe<U> fmap(maybe<U> (*func)(T)) const
    {
        if (hasValue)
        {
            return func(value);
        }

        return maybe<U>();
    }

private:
    bool hasValue;
    T value;
};

#endif
