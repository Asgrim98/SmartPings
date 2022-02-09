#include "../plugin_sdk/plugin_sdk.hpp"
#include "KoreanPings.h"
#include "PingPackage.h";
#include "WardsNotifier.h";

#include <iostream>
#include<chrono>
#include <sstream>


//const char* str = std::to_string(elapsed_seconds).c_str();
//console->print(str);
//test123 pass
namespace koreanPings
{
    std::vector<PingPackage> pingPackagesVector;
    std::vector<game_object_script> objectVector;

    std::map<std::uint32_t, std::vector<game_object_script>> wardNotifier;

    std::set<std::uint32_t>invisibleChampions;
    std::set<std::uint32_t>::iterator iterator;

    uint16_t availablePings;

    TreeTab* mainTab = nullptr;
    TreeTab* pingsSettingsTab = nullptr;

    namespace pings_settings
    {
        TreeEntry* pingDelay = nullptr;
        TreeEntry* pingOnWard = nullptr;
        TreeEntry* pingAllyDistance = nullptr;
        std::map<std::uint32_t, TreeEntry*> pingMoveOutFog;
        std::map<std::uint32_t, TreeEntry*> pingAllyNearWard;
    }

    float mainTimer, newPingStartTimer, delayStartTimer, currentTimer;

    void checkNewPingAvailable() {

        currentTimer = gametime->get_time();

        float elapsed_seconds = currentTimer - newPingStartTimer;

        //const char* str = std::to_string(elapsed_seconds).c_str();
        //console->print(str);

        if(elapsed_seconds > 1.0f){
            if (availablePings < 6) {
                availablePings++;
            }

            newPingStartTimer = gametime->get_time();
        }
    }

    bool checkCanCastPingDelay() {

        currentTimer = gametime->get_time();

        float elapsed_seconds = currentTimer - delayStartTimer;
        float time_delay_ms = static_cast<float>(pings_settings::pingDelay->get_int() / 1000);

        return elapsed_seconds > time_delay_ms;
    }

    void castLimitedPing() {

        if (availablePings > 0) {

            if (checkCanCastPingDelay()) {
                PingPackage& firstPing = pingPackagesVector.front();
                myhero->cast_ping(firstPing.getPosition(), nullptr, firstPing.getPingType());
                firstPing.setPingsCount(firstPing.getPingsCount() - 1);
                delayStartTimer = gametime->get_time();

                if (firstPing.getPingsCount() <= 0) {
                    pingPackagesVector.erase(pingPackagesVector.begin());
                }
            }
        }
    }

    //TODO handle range to enemy
    void load()
    {
        availablePings = 6;
        newPingStartTimer = gametime->get_time();
        delayStartTimer = gametime->get_time();
        mainTimer = gametime->get_time();

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

        auto pingAllyNearWardTab = pingsSettingsTab->add_tab("pingAllyNearWard", "Ping ally is near ward");
        {
            for (auto&& enemy : entitylist->get_ally_heroes())
            {
                pings_settings::pingAllyNearWard[enemy->get_network_id()] = pingAllyNearWardTab->add_checkbox(std::to_string(enemy->get_network_id()), enemy->get_model(), true, true);
                pings_settings::pingAllyNearWard[enemy->get_network_id()]->set_texture(enemy->get_square_icon_portrait());
            }
        }

        pings_settings::pingAllyDistance = pingAllyNearWardTab->add_slider("pingAllyDistance", "Ping ally to ping distance < x", 2000, 100, 4000);
        pings_settings::pingAllyDistance->set_tooltip("3000 is distance between top pixelbush and top tribush");

        pings_settings::pingOnWard = pingsSettingsTab->add_checkbox("pingOnWard", "Ping enemy place ward", true, true);
        pings_settings::pingDelay = pingsSettingsTab->add_slider("pingDelay", "Delay betweend pings (ms)", 400, 100, 1000);

        //event_handler<events::on_network_packet>::add_callback(on_network_packet);
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_create_object>::add_callback(on_create_object);
    }

    void unload()
    {
        event_handler<events::on_update>::remove_handler(on_update);
        event_handler<events::on_create_object>::remove_handler(on_create_object);
    }

    void on_network_packet(game_object_script sender, std::uint32_t network_id, pkttype_e type, void* args)
    {
        PingPackage pingPackage = PingPackage(sender->get_position(), 1, _player_ping_type::assist_me);
        pingPackagesVector.push_back(pingPackage);
    }


    void on_create_object(game_object_script sender)
    {
        if (sender->is_valid()) {
            objectVector.push_back(sender);
        }
    }

    bool check_is_target_enabled(game_object_script target, std::map<std::uint32_t, TreeEntry*> elements)
    {
        auto it = elements.find(target->get_network_id());
        if (it == elements.end())
            return false;

        return it->second->get_bool();
    }

    void ping_ally_close_to_ward() {

        auto allies = entitylist->get_ally_heroes();

        for (auto ally : allies) {
            if (check_is_target_enabled(ally, pings_settings::pingAllyNearWard)) {
                if (wardNotifier.find(ally->get_network_id()) != wardNotifier.end()) {
                    std::vector<game_object_script> wards = wardNotifier[ally->get_network_id()];
                    if (!wards.empty()) {
                        auto it = wards.begin();
                        while (it != wards.end()) {
                            if ((*it)->is_valid()) {
                                float distance = ally->get_distance(*it);
                                const char* str = std::to_string(distance).c_str();
                                console->print(str);
                                if (distance < pings_settings::pingAllyDistance->get_int()) {
                                    PingPackage pingPackage = PingPackage((*it)->get_position(), 1, _player_ping_type::area_is_warded);
                                    pingPackagesVector.push_back(pingPackage);
                                }
                            }
                            it = wards.erase(it);
                        }
                    }
                }
            }
        }
    }

    void search_objects_for_enemy_ward() {
        if (!objectVector.empty()) {
            auto it = objectVector.begin();
            while (it != objectVector.end()) {
                if ((*it)->is_valid() && (*it)->is_ward() && !(*it)->is_plant() && !(*it)->is_dead()) {

                    auto allies = entitylist->get_ally_heroes();

                    for (auto ally : allies) {
                        if (check_is_target_enabled(ally, pings_settings::pingAllyNearWard)) {
                            wardNotifier[ally->get_network_id()].push_back((*it));
                        }
                    }
                    
                    PingPackage pingPackage = PingPackage((*it)->get_position(), 1, _player_ping_type::area_is_warded);
                    pingPackagesVector.push_back(pingPackage);

                }
                it = objectVector.erase(it);
            }
        }
    }

    void search_enemy_move_out_fog() {
        auto enemies = entitylist->get_enemy_heroes();

        if (!enemies.empty()) {
            for (auto enemy : enemies) {

                std::uint16_t enemyId = enemy->get_network_id();

                if (check_is_target_enabled(enemy, pings_settings::pingMoveOutFog)) {
                    if (!enemy->is_visible()) {
                        invisibleChampions.insert(enemyId);
                    }
                    else {
                        iterator = invisibleChampions.find(enemyId);
                        if (iterator != invisibleChampions.end()) {
                            delayStartTimer = gametime->get_time();

                            PingPackage pingPackage = PingPackage(enemy->get_position(), 1, _player_ping_type::danger);
                            pingPackagesVector.push_back(pingPackage);

                            invisibleChampions.erase(iterator);
                        }
                    }
                }
            }
        }
    }

    void on_update()
    {
        float onUpdateTimer = gametime->get_time();

        checkNewPingAvailable();

        if (!pingPackagesVector.empty()) {
            castLimitedPing();
        }
        else if (pingPackagesVector.size() > 6) {
            console->print_success("USUWAM WSZYSTKIE WARDY");
            pingPackagesVector.clear();
        }

        if (onUpdateTimer - mainTimer > 1) {
            
            search_objects_for_enemy_ward();
            search_enemy_move_out_fog();
            ping_ally_close_to_ward();

            mainTimer = gametime->get_time();
        }


    }
}