#include "game/core/Difficulty.hpp"
#include "game/core/StringIds.hpp"

const Difficulty* Difficulty::PEACEFUL = new Difficulty(0, IDS_DIFFICULTY_TITLE_PEACEFUL);
const Difficulty* Difficulty::EASY = new Difficulty(1, IDS_DIFFICULTY_TITLE_EASY);
const Difficulty* Difficulty::NORMAL = new Difficulty(2, IDS_DIFFICULTY_TITLE_NORMAL);
const Difficulty* Difficulty::HARD = new Difficulty(3, IDS_DIFFICULTY_TITLE_HARD);

const Difficulty* Difficulty::BY_ID[4] = {PEACEFUL, EASY, NORMAL, HARD};

Difficulty::Difficulty(int id, int key) : id(id), key(key)
{
}

const Difficulty* Difficulty::byId(int id)
{
    return BY_ID[id % 4];
}

int Difficulty::getId() const
{
    return id;
}

int Difficulty::getKey() const
{
    return key;
}
