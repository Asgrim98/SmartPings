#pragma once
#include "../plugin_sdk/plugin_sdk.hpp"

namespace koreanPings
{
    void load();
    void unload();
    void on_update();
    bool checkCanCastPing();
    void castLimitedPing(vector, int);
    void on_create_object(game_object_script);
    void on_issue_order(game_object_script&, vector&, _issue_order_type&, bool*);
};