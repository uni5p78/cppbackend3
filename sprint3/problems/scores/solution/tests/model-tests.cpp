#include <cmath>
#include <sstream>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>

#include "../src/json_loader.h"

using namespace std::literals;



SCENARIO("Loot generation on map") {
    GIVEN("a loots") {
        model::Loots loots;
        THEN("The container loots must be empty."){
            CHECK(loots.GetCount() == 0);
        }
        WHEN("add 1 loot") {
            loots.AddLoot({1, {1.0, 1.0}});
            THEN("There should be one thing in the container"){
                CHECK(loots.GetCount() == 1);
                CHECK(loots.GetLootsList().size() == 1);
            }
        }
        WHEN("add 10 loot") {
            for (size_t i = 0; i<10; i++) {
                loots.AddLoot({1, {1.0, 1.0}});
            }
            THEN("There should be 10 things in the container"){
                CHECK(loots.GetCount() == 10);
                CHECK(loots.GetLootsList().size() == 10);
            }
        }
    }

    GIVEN("a map") {
        using IdMap = util::Tagged<std::string, model::Map>;
        model::Map map(IdMap("id"s), "name");
        WHEN("Loot Types Count == 10") {
            map.SetLootTypesCount(10);
            THEN("The loop type number must be less than ten and greater than or equal to zero."){
                for (size_t i = 0; i<20; i++) {
                    int rendom_loot_type = map.GetRandomLootType();
                    CHECK(rendom_loot_type < 10);
                    CHECK(rendom_loot_type >= 0);
                }
            }
            GIVEN("a GameSession") {
                model::GameSession game_session(&map, true);
                model::Road road(model::Road::HORIZONTAL, {0, 0}, 40);
                map.AddRoad(std::move(road));
                WHEN("Add in session 5 loops"){
                    game_session.AddLoots(5);
                    THEN("There should be five trophies in the session."){
                        CHECK(game_session.GetLootsCount() == 5);
                        auto loots = game_session.GetLoots();
                        CHECK(loots.GetCount() == 5);
                        CHECK(loots.GetLootsList().size() == 5);
                    }
                    WHEN("Add in session 5 loops, again"){
                        game_session.AddLoots(3);
                        THEN("There should be 8 trophies in total in the session."){
                            CHECK(game_session.GetLootsCount() == 8);
                            auto loots = game_session.GetLoots();
                            CHECK(loots.GetCount() == 8);
                            CHECK(loots.GetLootsList().size() == 8);
                        }
                    }
                    THEN("All loots must be on the road."){
                        auto loots = game_session.GetLoots().GetLootsList();
                        double old_x = -1.0;
                        for (const auto& loot : loots) {
                            CHECK(loot.pos_.x >= 0.0);
                            CHECK(loot.pos_.x <= 40.0);
                            CHECK(loot.pos_.y == 0.0);
                            CHECK(old_x != loot.pos_.x);
                            old_x = loot.pos_.x;
                        }
                    }
                    THEN("Type number must be less than ten and greater than or equal to zero."){
                        auto loots = game_session.GetLoots().GetLootsList();
                        int old_rendom_loot_type = -1;
                        for (const auto& loot : loots) {
                            CHECK(loot.type_ < 10);
                            CHECK(loot.type_ >= 0);
                            CHECK(old_rendom_loot_type != loot.type_);
                            old_rendom_loot_type = loot.type_;
                        }
                    }
                }
                WHEN("There are two roads on the map and many loots") {
                    model::Road road(model::Road::VERTICAL, {10, 0}, 20);
                    map.AddRoad(std::move(road));
                    const int LOOTS_COUNT = 20;
                    game_session.AddLoots(LOOTS_COUNT);

                    THEN("Then some of the loots will be on one road, and some on the other.") {
                        auto loots = game_session.GetLoots().GetLootsList();
                        CHECK(loots.size() == LOOTS_COUNT);
                        int horizont_count = 0;
                        std::stringstream ss;
                        ss << "Points of loots:" << std::endl;
                        for (const auto& loot : loots) {
                            if (loot.pos_.y == 0.0) {
                                horizont_count++;
                            }
                            ss << loot.pos_.x << " " << loot.pos_.y << std::endl;
                        }
                        INFO(ss.str());
                        CHECK(horizont_count > 0);
                        CHECK(horizont_count < LOOTS_COUNT);
                    }
                }


            }
        }

    }
}
