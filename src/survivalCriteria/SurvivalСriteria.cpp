#include <cassert>
#include <utility>

#include "SurvivalCriteria.h"

namespace BS
{
    /**
     * Overridable function for creating shapes of survival criteria
     */
    void SurvivalCriteria::initShapes(int liveDisplayScale)
    {
    }

    /**
     * Overridable function that called at the end of each step.
     * Some criterias like ChallengeRadioactiveWalls has special logic
     * performed after each step. Most of criterias do nothing.
     */
    void SurvivalCriteria::endOfStep(unsigned simStep, const Params &p, Grid &grid, Peeps &peeps)
    {
    }

    /**
     * Clears all shape descriptors of survival criteria
     */
    void SurvivalCriteria::clearShapes()
    {
        this->shapes.clear();
    }

    void SurvivalCriteria::createCircle(float radius, float x, float y)
    {
        shapes.push_back(ShapeCircle{radius, x, y, defaultColor, false});
    }

    void SurvivalCriteria::createBorder(float size, int liveDisplayScale)
    {
        constexpr ShapeColor borderColor = {0, 0, 0, 255};

        // top
        shapes.push_back(ShapeRect{0 - size, 0 - size,
            p.sizeX * (float)liveDisplayScale + size * 2, size, borderColor});
        // bottom
        shapes.push_back(ShapeRect{0 - size, p.sizeY * (float)liveDisplayScale,
            p.sizeX * (float)liveDisplayScale + size * 2, size, borderColor});
        // left
        shapes.push_back(ShapeRect{0 - size, 0 - size,
            size, p.sizeY * (float)liveDisplayScale + size * 2, borderColor});
        // right
        shapes.push_back(ShapeRect{p.sizeX * (float)liveDisplayScale, 0 - size,
            size, p.sizeY * (float)liveDisplayScale + size * 2, borderColor});
    }

    void SurvivalCriteria::createLine(float x1, float y1, float x2, float y2)
    {
        shapes.push_back(ShapeLine{x1, y1, x2, y2, defaultColor});
    }
}
