#pragma once
#include "../plugin_sdk/plugin_sdk.hpp"
#include "KoreanPings.h"
#include "PingPackage.h"
#include <random>
#include <iostream>

namespace koreanPings
{
    std::vector<PingPackage> pingPackagesVector;
    std::vector<game_object_script> objectVector;

    std::set<std::uint32_t>invisibleChampions;
    std::set<std::uint32_t>::iterator iterator;

    uint16_t availablePings;

    TreeTab* mainTab = nullptr;
    TreeTab* pingsSettingsTab = nullptr;

    std::random_device rd{};
    auto mtgen = std::mt19937(rd());
    auto ud = std::uniform_int_distribution<>(-50, 50);

    namespace pings_settings
    {
        TreeEntry* pingCount = nullptr;
        TreeEntry* pingDelay = nullptr;
        TreeEntry* wardDistance = nullptr;
        TreeEntry* pingOnWard = nullptr;
        TreeEntry* spamMissingPingHotkey = nullptr;
        TreeEntry* spamDangerPingHotkey = nullptr;
        TreeEntry* spamHelpPingHotkey = nullptr;

        TreeEntry* pingEnemyDistance = nullptr;
        std::map<std::uint32_t, TreeEntry*> pingMoveOutFog;
        std::map<std::uint32_t, TreeEntry*> pingAllyNearWard;
    }

    float mainTimer, newPingStartTimer, delayStartTimer, currentTimer, spamPingTimer;

    void checkNewPingAvailable() {

        currentTimer = gametime->get_time();

        float elapsed_seconds = currentTimer - newPingStartTimer;

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
        float time_delay_ms = static_cast<float>(pings_settings::pingDelay->get_int()) / static_cast<float>(1000);

        return elapsed_seconds > time_delay_ms;
    }

    void castLimitedPing() {

        if (availablePings > 0) {

            if (checkCanCastPingDelay()) {
                PingPackage& firstPing = pingPackagesVector.front();
                auto position = firstPing.getPosition();
                myhero->cast_ping(vector(position.x + ud(mtgen), position.y + ud(mtgen), position.z), nullptr, firstPing.getPingType());
                delayStartTimer = gametime->get_time();

                if (!pingPackagesVector.empty()) {
                    pingPackagesVector.erase(pingPackagesVector.begin());
                }
            }
        }
    }

    void load()
    {
        availablePings = 6;
        newPingStartTimer = gametime->get_time();
        delayStartTimer = gametime->get_time();
        mainTimer = gametime->get_time();
        spamPingTimer = gametime->get_time();

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

        pings_settings::pingEnemyDistance = pingMoveOutFogTab->add_slider("pingEnemyDistance", "Ping only when enemy if far than x", 1200, 100, 4000);
        pings_settings::pingEnemyDistance->set_tooltip("3000 is distance between top pixelbush and top tribush");
        
        

        //TODO
        //FOR FUTURE FEATURES
        //auto pingAllyNearWardTab = pingsSettingsTab->add_tab("pingAllyNearWard", "Ping ally is near ward");
        //{
        //    for (auto&& enemy : entitylist->get_ally_heroes())
        //    {
        //        pings_settings::pingAllyNearWard[enemy->get_network_id()] = pingAllyNearWardTab->add_checkbox(std::to_string(enemy->get_network_id()), enemy->get_model(), true, true);
        //        pings_settings::pingAllyNearWard[enemy->get_network_id()]->set_texture(enemy->get_square_icon_portrait());
        //    }
        //}

        //pings_settings::pingAllyDistance = pingAllyNearWardTab->add_slider("pingAllyDistance", "Ping ally to ward distance < x", 2000, 100, 4000);
        //pings_settings::pingAllyDistance->set_tooltip("3000 is distance between top pixelbush and top tribush");

        auto wardPingSettings = pingsSettingsTab->add_tab("pingOnWard", "Ping enemy place ward");

        pings_settings::pingOnWard = wardPingSettings->add_checkbox("pingOnWard", "Enabled", true, true);
        pings_settings::wardDistance = wardPingSettings->add_slider("wardDistance", "Ward to ally range (x)", 500, 0, 1000);
        pings_settings::wardDistance->set_tooltip("Ping only when no enemies in ward range . Prevent e.g. Lee ward jumps etc.");

        auto spamPingSettings = pingsSettingsTab->add_tab("spamPingSettings", "Ping spam settings");

        pings_settings::spamMissingPingHotkey = spamPingSettings->add_hotkey("spamMissing", "Spam missing pings", TreeHotkeyMode::Hold, 0x47 /*G key*/, false);
        pings_settings::spamDangerPingHotkey = spamPingSettings->add_hotkey("spamDanger", "Spam danger pings", TreeHotkeyMode::Hold, 0x48 /*H key*/, false);
        pings_settings::spamHelpPingHotkey = spamPingSettings->add_hotkey("spamHelp", "Spam help pings", TreeHotkeyMode::Hold, 0x4E /*0x4E N key*/, false);

        pings_settings::pingCount = spamPingSettings->add_slider("pingCount", "How many pings", 2, 1, 6);

        pings_settings::pingDelay = pingsSettingsTab->add_slider("pingDelay", "Delay betweend pings (ms)", 400, 30, 1000);

        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_create_object>::add_callback(on_create_object);
    }

    void spamPing(_player_ping_type ping_type)
    {
        for (size_t i = 0; i < pings_settings::pingCount->get_int(); i++) {
            PingPackage pingPackage = PingPackage(hud->get_hud_input_logic()->get_game_cursor_position(), ping_type);
            pingPackagesVector.push_back(pingPackage);
        }
    }

    void unload()
    {
        event_handler<events::on_update>::remove_handler(on_update);
        event_handler<events::on_create_object>::remove_handler(on_create_object);
    }

    void on_create_object(game_object_script sender)
    {
        if (pings_settings::pingOnWard->get_bool()) {
            if (sender->is_valid()) {
                objectVector.push_back(sender);
            }
        }
    }

    bool check_is_target_enabled(game_object_script target, std::map<std::uint32_t, TreeEntry*> elements)
    {
        auto it = elements.find(target->get_network_id());
        if (it == elements.end())
            return false;

        return it->second->get_bool();
    }

    void search_objects_for_enemy_ward() {
        if (!objectVector.empty()) {
            auto it = objectVector.begin();
            while (it != objectVector.end()) {
                if ((*it)->is_valid() && 
                    (*it)->is_enemy() &&
                    (*it)->is_ward() && 
                    !(*it)->is_plant() && 
                    !(*it)->is_dead() && 
                    !(*it)->is_general_particle_emitter() && 
                    (*it)->is_targetable_to_team(myhero->get_team()) && 
                    (*it)->count_allies_in_range(static_cast<float>(pings_settings::wardDistance->get_int())) == 0) {
                    PingPackage pingPackage = PingPackage((*it)->get_position(), _player_ping_type::area_is_warded);
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

                            float distance = myhero->get_distance(enemy);
                            if (distance > pings_settings::pingEnemyDistance->get_int()) {
                                delayStartTimer = gametime->get_time();

                                PingPackage pingPackage = PingPackage(enemy->get_position(), _player_ping_type::danger);
                                pingPackagesVector.push_back(pingPackage);

                                invisibleChampions.erase(iterator);
                            }
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
        
        if (pingPackagesVector.size() > 6) {
            pingPackagesVector.clear();
        }

        if (!pingPackagesVector.empty()) {
            castLimitedPing();
        }

        //USING TIMER TO PREVENT ACTION PER SECOND
        if (pings_settings::spamMissingPingHotkey->get_bool()) {
            if (onUpdateTimer - spamPingTimer > 1) {
                spamPing(_player_ping_type::missing_enemy);
                spamPingTimer = gametime->get_time();
            }
        }

        if (pings_settings::spamDangerPingHotkey->get_bool()) {
            if (onUpdateTimer - spamPingTimer > 1) {
                spamPing(_player_ping_type::danger);
                spamPingTimer = gametime->get_time();
            }
        }

        if (pings_settings::spamHelpPingHotkey->get_bool()) {
            if (onUpdateTimer - spamPingTimer > 1) {
                spamPing(_player_ping_type::assist_me);
                spamPingTimer = gametime->get_time();
            }
        }

        if (onUpdateTimer - mainTimer > 1) {
            if (pings_settings::pingOnWard->get_bool()) {
                search_objects_for_enemy_ward();
            }
            search_enemy_move_out_fog();
            mainTimer = gametime->get_time();
        }
    }
}