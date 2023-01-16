#pragma once

#include <sc2pp/Common.h>

namespace sc2::utils
{

class GridView
{
public:
    GridView(const proto::ImageData& data);

    auto operator[](const Point2DI& point) const -> bool;

    auto data() const { return m_data.data(); };

    int width() const { return m_width; };
    int height() const { return m_height; };

private:
    std::vector<char> m_data;
    int m_width;
    int m_height;
};

}