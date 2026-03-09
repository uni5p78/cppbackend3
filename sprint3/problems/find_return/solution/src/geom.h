

namespace geom {
    using coord = double;
    struct Point2D {
        coord x, y;
        // bool operator==(const Point2D& other) const = default;
        bool operator==(const Point2D other) const {
            return x == other.x && y == other.y;
        }

    };

} // namespace geom