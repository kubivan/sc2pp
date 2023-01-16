#include "GridView.h"

namespace sc2::utils
{

GridView::GridView(const proto::ImageData& img)
    : m_width(img.size().x())
    , m_height(img.size().y())
    , m_data(img.data().begin(), img.data().end())
{
}

auto GridView::operator[](const Point2DI& point) const -> bool
{
    div_t idx = div(point.x + point.y * width(), 8);
    auto d = m_data[idx.quot] >> (7 - idx.rem);
    return (m_data[idx.quot] >> (7 - idx.rem)) & 1;
}

}
