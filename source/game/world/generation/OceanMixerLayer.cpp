#include "game/world/generation/OceanMixerLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

OceanMixerLayer::OceanMixerLayer(long long seed, boost::shared_ptr<Layer> landLayer,
                                 boost::shared_ptr<Layer> oceanLayer)
    : Layer(seed),
      landLayer(landLayer),
      oceanLayer(oceanLayer)
{
}

void OceanMixerLayer::init(long long seed)
{
    landLayer->init(seed);
    oceanLayer->init(seed);

    Layer::init(seed);
}

arrayWithLength<int> OceanMixerLayer::getArea(int x, int z, int width, int height)
{
    arrayWithLength<int> landArea = landLayer->getArea(x, z, width, height);
    arrayWithLength<int> oceanArea = oceanLayer->getArea(x, z, width, height);

    PIXBeginNamedEvent(0.0f, "OceanMixerLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < width * height; i++)
    {
        if (!isOcean(landArea.data[i]) || !isOcean(oceanArea.data[i]))
        {
            area.data[i] = landArea.data[i];
        }
        else
        {
            if (landArea.data[i] == 24)
            {
                switch (oceanArea.data[i])
                {
                case 42:
                    area.data[i] = 43;
                    break;

                case 0:
                    area.data[i] = 24;
                    break;

                case 44:
                    area.data[i] = 45;
                    break;

                case 46:
                    area.data[i] = 47;
                    break;

                default:
                    area.data[i] = oceanArea.data[i];
                    break;
                }
            }
            else
            {
                area.data[i] = oceanArea.data[i];
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
