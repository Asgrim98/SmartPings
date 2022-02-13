#pragma once
#include "../plugin_sdk/plugin_sdk.hpp"

namespace koreanPings
{
    void load();
    void unload();
    void on_update();
    void checkNewPingAvailable();
    bool checkCanCastPingDelay();
    void castLimitedPing();
    void on_create_object(game_object_script);
    void on_delete_object(game_object_script);
    void spamPing(_player_ping_type ping_type);
    void search_enemy_move_out_fog();
    void search_objects_for_enemy_ward();
    bool check_is_target_enabled(game_object_script, std::map<std::uint32_t, TreeEntry*>);
};