#pragma once
namespace koreanPings
{
    void load();
    void unload();
    void on_update();
    bool checkCanCastPing();
    void castLimitedPing();
};