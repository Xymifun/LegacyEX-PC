#include "game/core/FJStringBuilder.hpp"

FJStringBuilder::FJStringBuilder()
{
}

FJStringBuilder::FJStringBuilder(StringBuilderCore* core)
{
    mCore = boost::shared_ptr<StringBuilderCore>(core);
}

std::wstring FJStringBuilder::operator()()
{
    return mCore.get()->getString();
}
