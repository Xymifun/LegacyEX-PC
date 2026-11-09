#ifndef GAME_WORLD_GENERATION_UPDATEPLAYERRULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_UPDATEPLAYERRULEDEFINITION_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/minigame/GameRuleDefinition.hpp"

class AddItemRuleDefinition;
class MutableBlockPos;
class Player;

class UpdatePlayerRuleDefinition : public GameRuleDefinition
{
public:
    UpdatePlayerRuleDefinition();
    UpdatePlayerRuleDefinition(boost::shared_ptr<Player> player);
    virtual ~UpdatePlayerRuleDefinition();

    double SanitisedYPos(double x, double y, double z, boost::shared_ptr<Player> player) const;
    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);
    virtual void writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount);
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);
    virtual void postProcessPlayer(boost::shared_ptr<Player> player);
    int getX() const;
    int getY() const;
    int getZ() const;
    float getXRot() const;
    float getYRot() const;
    virtual void WriteAttributesAsXML(std::string& xml);
    virtual ConsoleGameRules::EGameRuleType getActionType();

private:
    std::vector<AddItemRuleDefinition*> mItems;
    int mHealth;
    int mFoodLevel;
    MutableBlockPos* mSpawnPosition;
    int mDimension;
    float mYRot;
    float mXRot;
};

#endif
