#include "../plugin_sdk/plugin_sdk.hpp"
#include "KoreanPings.h"

#include <iostream>
#include<chrono>
#include <sstream>

namespace koreanPings
{

    int pingSended = 0;

    std::set<champion_id>invisibleChampions;
    std::set<champion_id>::iterator iterator;

    TreeTab* main_tab = nullptr;

    std::chrono::time_point<std::chrono::system_clock> start, end;


    bool checkCanCastPing() {

        end = std::chrono::system_clock::now();

        std::chrono::duration<double> elapsed_seconds = end - start;
        auto x = std::chrono::duration_cast<std::chrono::seconds>(elapsed_seconds);
        
        std::string result = std::to_string(x.count());
        const char* chr = result.c_str();
        console->print(chr);


        return x.count() > 5;
    }

    void castLimitedPing(vector poistion, int incresePingBy) {

        if (checkCanCastPing()) {
            if (pingSended % 10 == 0 && pingSended <= 50) {
                myhero->cast_ping(poistion, nullptr, _player_ping_type::danger);
            }

            if (pingSended == 50) {
                pingSended = 0;
                start = std::chrono::system_clock::now();
            }

            pingSended += incresePingBy;
        }
    }

    void load()
    {
        
        start = std::chrono::system_clock::now();
        main_tab = menu->create_tab("koreanPings", "KoreanPings");

        //event_handler<events::on_network_packet>::add_callback(on_network_packet);
        event_handler<events::on_update>::add_callback(on_update);

    }

    void on_network_packet(game_object_script sender, std::uint32_t network_id, pkttype_e type, void* args)
    {
        myhero->cast_ping(sender->get_position(), nullptr, _player_ping_type::danger);
    }

    void unload()
    {

        event_handler<events::on_update>::remove_handler(on_update);
    }

    void on_update()
    {
        if (orbwalker->combo_mode()) {
            vector my_position = myhero->get_position();
            vector vectorToPing = vector(my_position.x + pingSended, my_position.y + pingSended, my_position.z);
            castLimitedPing(vectorToPing, 1);
        }


        auto enemies = entitylist->get_enemy_heroes();

        for (auto enemy : enemies) {
            if (!enemy->is_visible()) {
                invisibleChampions.insert(enemy->get_champion());
            }
            else {
                iterator = invisibleChampions.find(enemy->get_champion());
                if (iterator != invisibleChampions.end()) {
                    castLimitedPing(enemy->get_position(), 10);
                    invisibleChampions.erase(iterator);
                }
            }
        }
    }

}