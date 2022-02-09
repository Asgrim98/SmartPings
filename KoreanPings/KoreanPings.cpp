#include "../plugin_sdk/plugin_sdk.hpp"
#include "KoreanPings.h"
#include "PingPackage.h";

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

    float newPingStartTimer, delayStartTimer, currentTimer;

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

                if (firstPing.getPingsCount() == 0) {
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
            for (auto&& enemy : entitylist->get_enemy_heroes())
            {
                pings_settings::pingAllyNearWard[enemy->get_network_id()] = pingAllyNearWardTab->add_checkbox(std::to_string(enemy->get_network_id()), enemy->get_model(), true, true);
                pings_settings::pingAllyNearWard[enemy->get_network_id()]->set_texture(enemy->get_square_icon_portrait());
            }
        }

        pings_settings::pingAllyDistance = pingAllyNearWardTab->add_slider("pingAllyDistance", "Ping ally to ping distance < x", 2000, 100, 20000);

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


    bool can_ping_fog_on(game_object_script target)
    {
        auto it = pings_settings::pingMoveOutFog.find(target->get_network_id());
        if (it == pings_settings::pingMoveOutFog.end())
            return false;

        return it->second->get_bool();
    }

    void on_update()
    {
        if (!pingPackagesVector.empty()) {
            castLimitedPing();
        }

        checkNewPingAvailable();


        if (!objectVector.empty()) {
            auto it = objectVector.begin();
            while (it != objectVector.end()) {
                if ((*it)->is_ward() && !(*it)->is_plant()) {
                    PingPackage pingPackage = PingPackage((*it)->get_position(), 1, _player_ping_type::area_is_warded);
                    pingPackagesVector.push_back(pingPackage);
                }
                it = objectVector.erase(it);
            }
        }

        auto enemies = entitylist->get_enemy_heroes();

        for (auto enemy : enemies) {

            std::uint16_t enemyId = enemy->get_network_id();

            if (can_ping_fog_on(enemy)) {
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