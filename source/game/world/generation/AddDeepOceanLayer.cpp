#include "game/world/generation/AddDeepOceanLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/biome/Biome.hpp"

AddDeepOceanLayer::AddDeepOceanLayer(long long seed, boost::shared_ptr<Layer> parentLayer) : Layer(seed)
{
    parent = parentLayer;
}

AddDeepOceanLayer::~AddDeepOceanLayer()
{
}

arrayWithLength<int> AddDeepOceanLayer::getArea(int x, int z, int width, int height)
{
    int parentX = x - 1;
    int parentZ = z - 1;
    int parentWidth = width + 2;
    int parentHeight = height + 2;

    arrayWithLength<int> parentArea = parent->getArea(parentX, parentZ, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "AddDeepOceanLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int north = parentArea[j + 1 + (i + 1 - 1) * (width + 2)];
            int east = parentArea[j + 1 + 1 + (i + 1) * (width + 2)];
            int west = parentArea[j + 1 - 1 + (i + 1) * (width + 2)];
            int south = parentArea[j + 1 + (i + 1 + 1) * (width + 2)];
            int centre = parentArea[j + 1 + (i + 1) * parentWidth];

            int shallowNeighbours = 0;
            if (isShallowOcean(north))
            {
                shallowNeighbours++;
            }
            if (isShallowOcean(east))
            {
                shallowNeighbours++;
            }
            if (isShallowOcean(west))
            {
                shallowNeighbours++;
            }
            if (isShallowOcean(south))
            {
                shallowNeighbours++;
            }

            if (isShallowOcean(centre) && shallowNeighbours > 3)
            {
                switch (centre)
                {
                case Biome::eBiomeIDs_WarmOcean:
                    area[j + i * width] = Biome::eBiomeIDs_DeepWarmOcean;
                    break;

                case Biome::eBiomeIDs_LukewarmOcean:
                    area[j + i * width] = Biome::eBiomeIDs_DeepLukewarmOcean;
                    break;

                case Biome::eBiomeIDs_Ocean:
                    area[j + i * width] = Biome::eBiomeIDs_DeepOcean;
                    break;

                case Biome::eBiomeIDs_FrozenOcean:
                    area[j + i * width] = Biome::eBiomeIDs_DeepFrozenOcean;
                    break;

                case Biome::eBiomeIDs_ColdOcean:
                    area[j + i * width] = Biome::eBiomeIDs_DeepColdOcean;
                    break;
                }
            }
            else
            {
                area[j + i * width] = centre;
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
