#ifndef PLATFORM_WIIU_MIIICON_MIIICONASSERT_HPP
#define PLATFORM_WIIU_MIIICON_MIIICONASSERT_HPP

#include <coreinit/debug.h>

#define MII_ICON_ASSERT_POINTER(pointer)                                                                     \
    do                                                                                                       \
    {                                                                                                        \
        if ((pointer) == NULL)                                                                               \
            OSPanic(__FILE__, __LINE__, "Invalid Pointer: %x", (pointer));                                   \
    } while (0)

#define MII_ICON_ASSERT(expression)                                                                          \
    do                                                                                                       \
    {                                                                                                        \
        if (!(expression))                                                                                   \
            OSPanic(__FILE__, __LINE__, "Assertion Failed: %s", #expression);                                \
    } while (0)

#define MII_ICON_ASSERT_MESSAGE(expression, ...)                                                             \
    do                                                                                                       \
    {                                                                                                        \
        if (!(expression))                                                                                   \
            OSPanic(__FILE__, __LINE__, __VA_ARGS__);                                                        \
    } while (0)

#define MII_ICON_INVALID_CONDITION(value) OSPanic(__FILE__, __LINE__, "Invalid Condition: %d", (value))

#endif
