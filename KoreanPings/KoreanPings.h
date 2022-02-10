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
};