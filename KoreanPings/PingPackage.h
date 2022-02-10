#pragma once
#include "../plugin_sdk/plugin_sdk.hpp"

class PingPackage
{
private:
	vector position;
	uint8_t pingsCount;
	_player_ping_type pingType;

public:
	PingPackage(vector position, _player_ping_type pingType) {
		this->position = position;
		this->pingType = pingType;
	}

	vector getPosition() {
		return this->position;
	}

	uint8_t getPingsCount() {
		return this->pingsCount;
	}

	_player_ping_type getPingType() {
		return this->pingType;
	}

	void setPingsCount(uint8_t pingsCount) {
		this->pingsCount = pingsCount;
	}
};

