#pragma once
#include "../plugin_sdk/plugin_sdk.hpp"

class WardsNotifier
{
private:
	game_object_script target;
	game_object_script ward;

public:
	WardsNotifier(game_object_script target, game_object_script ward) : target(target), ward(ward) {}


	game_object_script getTarget() {
		return this->target;
	}

	game_object_script getWard() {
		return this->ward;
	}
};
