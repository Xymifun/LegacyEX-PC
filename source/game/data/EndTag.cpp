#include "game/data/EndTag.hpp"

#include "game/core/DataInput.hpp"
#include "game/core/DataOutput.hpp"
#include "game/data/Tag.hpp"

EndTag::EndTag()
{
}

EndTag::~EndTag()
{
}

void EndTag::write(DataOutput* output)
{
}

void EndTag::load(DataInput* input, int depth)
{
}

std::wstring EndTag::toString()
{
    return L"END";
}

unsigned char EndTag::getId()
{
    return 0;
}

bool EndTag::equals(Tag* other)
{
    return Tag::equals(other);
}

EndTag* EndTag::copy()
{
    return new EndTag();
}
