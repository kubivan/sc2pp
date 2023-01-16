#pragma once

#include <sc2pp/Common.h>
#include <utils/GridView.h>

#include <functional>
#include <vector>
#include <deque>
#include <type_traits>

namespace sc2::utils
{

template<class T>
class Grid
{
public:

    Grid(int width, int height, T value = {})
        : m_width(width)
        , m_height(height)
        , m_grid(width * height, value)
    {
    }

    Grid(const proto::ImageData& img)
        : Grid(img.size().x(), img.size().y())
    {
        const auto view = GridView{img};
        for (auto y = 0; y < view.height(); ++y)
        {
            for (auto x = 0; x < view.width(); ++x)
            {
                m_grid[y * view.width() + x] = view[{x,y}];
            }
        }
    }

    const T&
    operator[](const Point2DI& point) const
    {
        return m_grid[size_t(point.y) * m_width + size_t(point.x)];
    }

    T&
    operator[](const Point2DI& point)
    {
        return const_cast<T&>(static_cast<const Grid<T>&>(*this).operator [](point));
    }

    int width() const { return m_width; };
    int height() const { return m_height; };

private:
    int m_width;
    int m_height;
    std::deque<T> m_grid;
};

}
