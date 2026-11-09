#include "game/world/generation/AddOceanTemperatureLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/biome/Biome.hpp"

AddOceanTemperatureLayer::AddOceanTemperatureLayer(long long seed, boost::shared_ptr<Layer> parent) : Layer(seed)
{
    this->parent = parent;
}

arrayWithLength<int> AddOceanTemperatureLayer::getArea(int x, int z, int width, int height)
{
    PIXBeginNamedEvent(0.0f, "AddOceanTemperatureLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            initRandom((long long)(j + x), (long long)(i + z));

            int random = nextRandom(100);
            float temperature = (float)random / 100.0f;
            if (temperature < 0.075f)
            {
                area.data[j + i * width] = Biome::eBiomeIDs_WarmOcean;
            }
            else if (temperature < 0.4f)
            {
                area.data[j + i * width] = Biome::eBiomeIDs_LukewarmOcean;
            }
            else if (temperature < 0.675f)
            {
                area.data[j + i * width] = Biome::eBiomeIDs_Ocean;
            }
            else if (temperature < 0.95f)
            {
                area.data[j + i * width] = Biome::eBiomeIDs_ColdOcean;
            }
            else
            {
                area.data[j + i * width] = Biome::eBiomeIDs_FrozenOcean;
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
