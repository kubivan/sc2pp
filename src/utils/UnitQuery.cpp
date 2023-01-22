#include <functional>

#include "UnitQuery.h"

namespace sc2
{

//TODO: move
Point2D operator-(const Point2D& lhs, const Point2D& rhs) {
    return Point2D(lhs.x - rhs.x, lhs.y - rhs.y);
}


float dot(const Point2D& a, const Point2D& b)
{
    return a.x * b.x + a.y * b.y;
}

float dot(const Point3D& a, const Point3D& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float dist_squared(const Point2D& a, const Point2D& b)
{
    Point2D diff = a - b;
    return dot(diff, diff);
}

const Unit* closest(const Point2D& pos, const std::vector<const Unit*>& objects)
{
    assert(!objects.empty());
    return *std::min_element(objects.cbegin()
        , objects.cend()
        , [&pos](const auto a, const auto b) {
            return dist_squared(pos, a->pos)
                < dist_squared(pos, b->pos);
        });
}
const Unit* closest(const Unit* unit, const std::vector<const Unit*>& objects)
{
    return closest(unit->pos, objects);
}

}
