#define _USE_MATH_DEFINES

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

#include "../src/collision_detector.h"
#include <vector>
#include <sstream>

using namespace std::literals;
using namespace collision_detector;

constexpr double EPSILON = 1.0e-10;
 
namespace Catch {
    
template<>
struct StringMaker<collision_detector::GatheringEvent> {
  static std::string convert(collision_detector::GatheringEvent const& value) {
      std::ostringstream tmp;
      tmp << "(" << value.item_id << "," << value.gatherer_id << "," << value.sq_distance << "," << value.time << ")";

      return tmp.str();
  }
};
}  // namespace Catch

template <typename Range>
struct IsEqualEventsListMatcher : Catch::Matchers::MatcherGenericBase {
    IsEqualEventsListMatcher(Range range)
        : range_{std::move(range)} {
        auto comp_sort = [](const GatheringEvent& a, const GatheringEvent& b){
            return a.time <= b.time;
        };
         std::sort(std::begin(range_), std::end(range_), comp_sort);
    }
    IsEqualEventsListMatcher(IsEqualEventsListMatcher&&) = default;

    template <typename OtherRange>
    bool match(OtherRange other) const {
        using std::begin;
        using std::end;

        // std::sort(begin(other), end(other));
        // template <typename T>
        auto comp_equal = [](const GatheringEvent& a, const GatheringEvent& b){
            return a.item_id == b.item_id 
            && a.gatherer_id == b.gatherer_id
            && std::abs(a.sq_distance - b.sq_distance) <= EPSILON
            && std::abs(a.time - b.time) <= EPSILON
            ;
        };

        return std::equal(begin(range_), end(range_), begin(other), end(other), comp_equal);
    }

    std::string describe() const override {
        // Описание свойства, проверяемого матчером:
        return "Is equal of: "s + Catch::rangeToString(range_);
    }

private:
    Range range_;
}; 

template<typename Range>
IsEqualEventsListMatcher<Range> IsEqualEventsList(Range&& range) {
    return IsEqualEventsListMatcher<Range>{std::forward<Range>(range)};
} 

using Items = std::vector<collision_detector::Item>;
using Gatherers = std::vector<collision_detector::Gatherer>;

class ItemGathererContaner : public collision_detector::ItemGathererProvider {
public:
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

// Напишите здесь тесты для функции collision_detector::FindGatherEvents

SCENARIO("FindGatherEvents testing") {
    GIVEN("Items and gatherers") {
        Items items;
        Gatherers gatherers;
        gatherers.emplace_back(geom::Point2D(1.0, 1.0), geom::Point2D(5.0, 1.0), 0.3);
        items.emplace_back(geom::Point2D(3.0, 1.5), 0.2);
        items.emplace_back(geom::Point2D(3.0, 1.5), 0.1);
        gatherers.emplace_back(geom::Point2D(5.0, 1.0), geom::Point2D(10.0, 1.0), 0.3);
        items.emplace_back(geom::Point2D(2.5, 1.5), 0.2);
        // собака стоит около трофея
        gatherers.emplace_back(geom::Point2D(0.9, 5.0), geom::Point2D(0.9, 5.0), 0.3);
        items.emplace_back(geom::Point2D(1.0, 5.0), 0.2);

        ItemGathererContaner contaner(items, gatherers);

        THEN("Find gather event") {
            auto gathering_event = FindGatherEvents(contaner);
            CHECK_THAT(gathering_event, IsEqualEventsList(std::vector<GatheringEvent>{
                GatheringEvent{2, 0, 0.25, 0.375}
              , GatheringEvent{0, 0, 0.25, 0.5}
            }));
        }
    }

}