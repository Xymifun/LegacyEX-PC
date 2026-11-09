#ifndef GAME_CORE_IGAMERULEATTRIBUTE_HPP
#define GAME_CORE_IGAMERULEATTRIBUTE_HPP

#include "game/data/GameRuleSaveInterface.hpp"

#include <string>

extern const wchar_t* gRuleAttributeNames[];

class IGameRuleAttribute
{
public:
    IGameRuleAttribute();
    virtual ~IGameRuleAttribute() {}

    virtual void setValue(const std::wstring& value) = 0;
    virtual void save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface) = 0;
    virtual void writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml) = 0;

    static int attributeIdFromName(const std::wstring& name);

    void setOptional(bool optional);
    bool isOptional();
    bool hasValue();

protected:
    unsigned int mOptional : 1;
    unsigned int mHasMin : 1;
    unsigned int mHasMax : 1;
    unsigned int mHasValue : 1;
};

#endif
