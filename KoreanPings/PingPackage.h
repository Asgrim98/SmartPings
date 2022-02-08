#pragma once
#include "../plugin_sdk/plugin_sdk.hpp"

class PingPackage
{
private:
	vector position;
	uint8_t pingsCount;
	uint16_t timerDelayMs;
	_player_ping_type pingType;

public:
	PingPackage(vector position, uint8_t pingsCount, uint16_t timerDelayMs, _player_ping_type pingType) {
		this->position = position;
		this->pingsCount = pingsCount;
		this->timerDelayMs = timerDelayMs;
		this->pingType = pingType;
	}

	vector getPosition() {
		return this->position;
	}

	uint8_t getPingsCount() {
		return this->pingsCount;
	}

	uint8_t getTimerDelayMs() {
		return this->timerDelayMs;
	}

	_player_ping_type getPingType() {
		return this->pingType;
	}

	void setPingsCount(uint8_t pingsCount) {
		this->pingsCount = pingsCount;
	}
};

