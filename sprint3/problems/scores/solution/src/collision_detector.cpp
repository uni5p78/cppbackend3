#include "collision_detector.h"
#include <cassert>

namespace collision_detector {

CollectionResult TryCollectPoint(geom::Point2D a, geom::Point2D b, geom::Point2D c) {
    // Проверим, что перемещение ненулевое.
    // Тут приходится использовать строгое равенство, а не приближённое,
    // пскольку при сборе заказов придётся учитывать перемещение даже на небольшое
    // расстояние.
    assert(b.x != a.x || b.y != a.y);
    const double u_x = c.x - a.x;
    const double u_y = c.y - a.y;
    const double v_x = b.x - a.x;
    const double v_y = b.y - a.y;
    const double u_dot_v = u_x * v_x + u_y * v_y;
    const double u_len2 = u_x * u_x + u_y * u_y;
    const double v_len2 = v_x * v_x + v_y * v_y;
    const double proj_ratio = u_dot_v / v_len2;
    const double sq_distance = u_len2 - (u_dot_v * u_dot_v) / v_len2;

    return CollectionResult(sq_distance, proj_ratio);
}

// В задании на разработку тестов реализовывать следующую функцию не нужно -
// она будет линковаться извне.

std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider& provider) {
    auto gatherers_count = provider.GatherersCount();
    auto items_count = provider.ItemsCount();
    std::vector<GatheringEvent> res;
    for(size_t g_ind = 0; g_ind < gatherers_count; g_ind++) {
        auto gatherer = provider.GetGatherer(g_ind);
        // Стоячая собака не может собирать трофеи
        if (gatherer.start_pos == gatherer.end_pos) {
            continue;
        }
        for(size_t i_ind = 0; i_ind < items_count; i_ind++) {
            auto item = provider.GetItem(i_ind);
            auto col_res = TryCollectPoint(gatherer.start_pos, gatherer.end_pos, item.position);
            if (col_res.IsCollected(gatherer.width, item.width)) {
                res.emplace_back(i_ind, g_ind, col_res.sq_distance, col_res.proj_ratio);
            }
        }
    }
    // Сортируем события по возрастанию времени
    auto comp_sort = [](const GatheringEvent& a, const GatheringEvent& b){
        return a.time <= b.time;
    };
    std::sort(std::begin(res), std::end(res), comp_sort);

    return res;
}


}  // namespace collision_detector