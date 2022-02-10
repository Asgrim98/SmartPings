#pragma once
#include "../plugin_sdk/plugin_sdk.hpp"

PLUGIN_NAME( "KoreanPings" );

#include "KoreanPings.h"

PLUGIN_API bool on_sdk_load( plugin_sdk_core* plugin_sdk_good )
{
    DECLARE_GLOBALS( plugin_sdk_good );

    console->print( "Changed" );

    koreanPings::load();

    return true;
}

PLUGIN_API void on_sdk_unload( )
{
    koreanPings::unload();
}