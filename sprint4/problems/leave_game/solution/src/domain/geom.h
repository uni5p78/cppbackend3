

namespace geom {
    using coord = double;
    struct Point2D {
        coord x, y;
        bool operator==(const Point2D other) const {
            return x == other.x && y == other.y;
        }

    };

    struct Vec2D {
        coord x, y;
    };

} // namespace geom