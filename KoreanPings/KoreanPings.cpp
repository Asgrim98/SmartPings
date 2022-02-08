#include "../plugin_sdk/plugin_sdk.hpp"
#include "KoreanPings.h"

#include <iostream>
#include<chrono>
#include <sstream>

namespace koreanPings
{

    int pingSended = 0;

    std::set<std::uint32_t>invisibleChampions;
    std::set<std::uint32_t>::iterator iterator;

    TreeTab* mainTab = nullptr;
    TreeTab* pingsSettingsTab = nullptr;

    std::map<std::uint32_t, TreeEntry*> pingMoveOutFog;

    namespace pings_settings
    {
        TreeEntry* pingOnWard = nullptr;
        std::map<std::uint32_t, TreeEntry*> pingMoveOutFog;
    }

    std::chrono::time_point<std::chrono::system_clock> start, end;


    bool checkCanCastPing() {

        end = std::chrono::system_clock::now();

        std::chrono::duration<double> elapsed_seconds = end - start;
        auto x = std::chrono::duration_cast<std::chrono::seconds>(elapsed_seconds);
        
        std::string result = std::to_string(x.count());
        const char* chr = result.c_str();

        return x.count() > 5;
    }

    void castLimitedPing(vector poistion, int incresePingBy, _player_ping_type pingType) {

        if (checkCanCastPing()) {
            if (pingSended % 10 == 0 && pingSended <= 50) {
                myhero->cast_ping(poistion, nullptr, pingType);
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
        mainTab = menu->create_tab("koreanPings", "KoreanPings");
        pingsSettingsTab = mainTab->add_tab("pingSettings", "Ping settings");

        auto pingMoveOutFogTab = pingsSettingsTab->add_tab("pingMoveOutFog", "Ping enemy move out fog");
        {
            for (auto&& enemy : entitylist->get_enemy_heroes())
            {
                pings_settings::pingMoveOutFog[enemy->get_network_id()] = pingMoveOutFogTab->add_checkbox(std::to_string(enemy->get_network_id()), enemy->get_model(), true, true);
                pings_settings::pingMoveOutFog[enemy->get_network_id()]->set_texture(enemy->get_square_icon_portrait());
            }
        }

        pings_settings::pingOnWard = pingsSettingsTab->add_checkbox("pingOnWard", "Ping enemy place ward", true, true);

        //event_handler<events::on_network_packet>::add_callback(on_network_packet);
        event_handler<events::on_issue_order>::add_callback(on_issue_order);
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_create_object>::add_callback(on_create_object);
    }

    void on_network_packet(game_object_script sender, std::uint32_t network_id, pkttype_e type, void* args)
    {
        myhero->cast_ping(sender->get_position(), nullptr, _player_ping_type::danger);
    }

    void unload()
    {

        event_handler<events::on_update>::remove_handler(on_update);
    }

    void on_issue_order(game_object_script& target, vector& pos, _issue_order_type& type, bool* process)
    {
        
    }

    void on_create_object(game_object_script sender)
    {
        if (sender->is_ward() && sender->is_enemy()) {
            castLimitedPing(sender->get_position(), 10, _player_ping_type::area_is_warded);
        }
    }

    void on_update()
    {
        //if (orbwalker->combo_mode()) {
        //    vector my_position = myhero->get_position();
        //    vector vectorToPing = vector(my_position.x + pingSended, my_position.y + pingSended, my_position.z);
        //    castLimitedPing(vectorToPing, 1);
        //}

        auto enemies = entitylist->get_enemy_heroes();

        for (auto enemy : enemies) {

            std::uint16_t enemyId = enemy->get_network_id();

            auto checkedInSettings = pingMoveOutFog.find(enemyId);
            if (checkedInSettings == pingMoveOutFog.end()) {
                if (!enemy->is_visible()) {
                    invisibleChampions.insert(enemyId);
                }
                else {
                    iterator = invisibleChampions.find(enemyId);
                    if (iterator != invisibleChampions.end()) {
                        castLimitedPing(enemy->get_position(), 10, _player_ping_type::danger);
                        invisibleChampions.erase(iterator);
                    }
                }
            }
        }
        
    }
}