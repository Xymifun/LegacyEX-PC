#ifndef GAME_WORLD_LAYERGENERATIONRULEDEFINITION_HPP
#define GAME_WORLD_LAYERGENERATIONRULEDEFINITION_HPP

#include <vector>

class LayerMetadata;
class Random;

template <class T>
T* randomUsableElement(Random& random, std::vector<T*>& definitions, const LayerMetadata& metadata);

#endif
