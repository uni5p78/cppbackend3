#pragma once

#include "geom.h"

#include <algorithm>
#include <vector>
#include <cmath>

namespace collision_detector {

enum class  ItemType {LOOT, OFFICE};

struct CollectionResult {
    bool IsCollected(double width_obj_1, double width_obj_2) const {
        double sq_distance_other = std::pow((width_obj_1 + width_obj_2)/2.0, 2);
        return proj_ratio >= 0 && proj_ratio <= 1 && sq_distance <= sq_distance_other;
    }

    // квадрат расстояния до точки
    double sq_distance;

    // доля пройденного отрезка
    double proj_ratio;
};

// Движемся из точки a в точку b и пытаемся подобрать точку c.
// Эта функция реализована в уроке.
CollectionResult TryCollectPoint(geom::Point2D a, geom::Point2D b, geom::Point2D c);

struct Item {
    geom::Point2D position;
    double width;
    ItemType type;
    size_t id;
};

struct Gatherer {
    geom::Point2D start_pos;
    geom::Point2D end_pos;
    double width;
};

class ItemGathererProvider {
protected:
    ~ItemGathererProvider() = default;

public:
    virtual size_t ItemsCount() const = 0;
    virtual Item GetItem(size_t idx) const = 0;
    virtual size_t GatherersCount() const = 0;
    virtual Gatherer GetGatherer(size_t idx) const = 0;
};

struct GatheringEvent {
    size_t item_id;
    size_t gatherer_id;
    double sq_distance;
    double time;
};

std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider& provider);

class ItemGathererContaner : public collision_detector::ItemGathererProvider {
public:
    using Items = std::vector<collision_detector::Item>;
    using Gatherers = std::vector<collision_detector::Gatherer>;
    ItemGathererContaner(Items items, Gatherers gatherers)
    : items_(std::move(items))
    , gatherers_(std::move(gatherers)){
    }
    size_t ItemsCount() const override {
        return items_.size();
    }

    collision_detector::Item GetItem(size_t idx) const override {
        return items_.at(idx);
    }

    size_t GatherersCount() const override {
        return gatherers_.size();
    }

    collision_detector::Gatherer GetGatherer(size_t idx) const override {
        return gatherers_.at(idx);
    }

private:
    Items items_;
    Gatherers gatherers_;
};

}  // namespace collision_detector