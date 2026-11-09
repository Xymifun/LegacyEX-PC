#ifndef GAME_CORE_DIFFICULTY_HPP
#define GAME_CORE_DIFFICULTY_HPP

class Difficulty
{
public:
    static const Difficulty* PEACEFUL;
    static const Difficulty* EASY;
    static const Difficulty* NORMAL;
    static const Difficulty* HARD;

    Difficulty(int id, int key);

    static const Difficulty* byId(int id);

    int getId() const;
    int getKey() const;

private:
    static const Difficulty* BY_ID[4];

    int id;
    int key;
};

#endif
