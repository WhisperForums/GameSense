#pragma once
#include "singleton.h"

class ShotRecord;

class c_resolver : public singleton<c_resolver> {
public:
	enum Modes : size_t {
		RESOLVE_NONE = 0,
		RESOLVE_STAND,
		RESOLVE_WALK,
		RESOLVE_AIR,
	};

public:
	LagRecord* FindIdealRecord( AimPlayer* data );
	LagRecord* FindLastRecord( AimPlayer* data );

	void MatchShot( AimPlayer* data, LagRecord* record );
	void SetMode( LagRecord* record );

	void ResolveAngles( Player* player, LagRecord* record );

public:
	std::array< vec3_t, 64 > m_impacts;
};
