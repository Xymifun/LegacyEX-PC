#include "game/util/StringUtils.hpp"

static const int POWER_TABLE_SIZE = 78;
static const int POWER_TABLE_UNIT_INDEX = 39;

static const double POWERS_OF_TEN[POWER_TABLE_SIZE] = {
    1e-39, 1e-38, 1e-37, 1e-36, 1e-35, 1e-34, 1e-33, 1e-32, 1e-31, 1e-30, 1e-29, 1e-28, 1e-27, 1e-26, 1e-25, 1e-24,
    1e-23, 1e-22, 1e-21, 1e-20, 1e-19, 1e-18, 1e-17, 1e-16, 1e-15, 1e-14, 1e-13, 1e-12, 1e-11, 1e-10, 1e-9,  1e-8,
    1e-7,  1e-6,  1e-5,  1e-4,  1e-3,  1e-2,  1e-1,  1e0,   1e1,   1e2,   1e3,   1e4,   1e5,   1e6,   1e7,   1e8,
    1e9,   1e10,  1e11,  1e12,  1e13,  1e14,  1e15,  1e16,  1e17,  1e18,  1e19,  1e20,  1e21,  1e22,  1e23,  1e24,
    1e25,  1e26,  1e27,  1e28,  1e29,  1e30,  1e31,  1e32,  1e33,  1e34,  1e35,  1e36,  1e37,  1e38};

static const double DIGIT_VALUES[10] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};

static const double MAX_PARSED_VALUE = 3.40282e+38f;

int atoi_with_len(const char* text, int base, int* length)
{
    const char* current = text;
    int character;
    const bool negative = *text == '-';
    if (*text == '-' || (character = *current) == '+')
    {
        current = text + 1;
        character = *current;
    }

    int value = 0;
    if (base != 0)
    {
        for (;;)
        {
            int digit;
            if (character >= '0' && character <= '9')
            {
                digit = character - '0';
            }
            else if (character >= 'A' && character <= 'F')
            {
                digit = character - 'A' + 10;
            }
            else if (character >= 'a' && character <= 'f')
            {
                digit = character - 'a' + 10;
            }
            else
            {
                break;
            }
            character = *++current;
            value = value * 16 + digit;
        }
    }
    else
    {
        while (character >= '0' && character <= '9')
        {
            value = value * 10 + character - '0';
            character = *++current;
        }
    }

    if (negative)
    {
        value = -value;
    }
    if (length != 0)
    {
        *length = static_cast<int>(current - text);
    }
    return value;
}

float atof_with_len(const char* text, int* length)
{
    const char* start = text;
    int character;
    const bool negative = *start == '-';
    if (*start == '-' || (character = *start) == '+')
    {
        character = *++text;
    }
    while (character == '0')
    {
        character = *++text;
    }

    const char* integerBegin = text;
    const char* lastIntegerDigit = text;
    const char* integerEnd = text;
    if (character >= '0' && character <= '9')
    {
        do
        {
            if (character != '0')
            {
                lastIntegerDigit = text;
            }
            character = *++text;
        } while (character >= '0' && character <= '9');
        integerEnd = text;
    }

    const char* fractionBegin = text;
    const char* firstFractionDigit = text;
    const char* lastFractionDigit = text;
    const char* fractionEnd = text;
    if (character == '.')
    {
        character = *++text;
        fractionBegin = text;
        while (character == '0')
        {
            character = *++text;
        }
        firstFractionDigit = text;
        lastFractionDigit = text;
        if (character >= '0' && character <= '9')
        {
            do
            {
                if (character != '0')
                {
                    lastFractionDigit = text;
                }
                character = *++text;
            } while (character >= '0' && character <= '9');
        }
        fractionEnd = text;
    }

    int exponent = 0;
    if (character == 'e' || character == 'E')
    {
        character = *++text;
        const bool negativeExponent = character == '-';
        if (character == '-' || character == '+')
        {
            character = *++text;
        }
        while (character >= '0' && character <= '9')
        {
            exponent = exponent * 10 + character - '0';
            character = *++text;
        }
        if (negativeExponent)
        {
            exponent = -exponent;
        }
    }

    double value = 0.0;
    if (integerBegin != integerEnd)
    {
        const double* power = &POWERS_OF_TEN[integerEnd + POWER_TABLE_UNIT_INDEX - integerBegin + exponent - 1];
        if (power >= POWERS_OF_TEN + POWER_TABLE_SIZE)
        {
            value = MAX_PARSED_VALUE;
        }
        else
        {
            power += integerBegin - lastIntegerDigit;
            const char* digit = lastIntegerDigit;
            do
            {
                if (power >= POWERS_OF_TEN)
                {
                    value += *power * DIGIT_VALUES[*digit - '0'];
                }
                ++power;
                --digit;
            } while (digit >= integerBegin);
        }
    }

    if (firstFractionDigit != fractionEnd)
    {
        const double* power =
            &POWERS_OF_TEN[fractionBegin - firstFractionDigit + exponent + POWER_TABLE_UNIT_INDEX - 1];
        if (power >= POWERS_OF_TEN + POWER_TABLE_SIZE)
        {
            value = MAX_PARSED_VALUE;
        }
        else
        {
            double fraction = 0.0;
            power += firstFractionDigit - lastFractionDigit;
            const char* digit = lastFractionDigit;
            do
            {
                if (power >= POWERS_OF_TEN)
                {
                    fraction += *power * DIGIT_VALUES[*digit - '0'];
                }
                ++power;
                --digit;
            } while (digit >= firstFractionDigit);
            value += fraction;
        }
    }

    if (negative)
    {
        value = -value;
    }
    if (length != 0)
    {
        *length = static_cast<int>(text - start);
    }
    return static_cast<float>(value);
}
