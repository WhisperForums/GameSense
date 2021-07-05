#include "includes.h"

LagRecord* c_resolver::FindIdealRecord(AimPlayer* data) {
	LagRecord* first_valid, * current;

	if (data->m_records.empty())
		return nullptr;

	first_valid = nullptr;

	// iterate records.
	for (const auto& it : data->m_records) {
		if (it->dormant() || it->immune() || !it->valid())
			continue;

		// get current record.
		current = it.get();

		// first record that was valid, store it for later.
		if (!first_valid)
			first_valid = current;
		// todo: simv0l: change this fckng best records priority
		if (it->m_shot || it->m_mode == Modes::RESOLVE_AIR)
			return current;
	}

	// none found above, return the first valid record if possible.
	return (first_valid) ? first_valid : nullptr;
}

LagRecord* c_resolver::FindLastRecord(AimPlayer* data) {
	LagRecord* current;

	if (data->m_records.empty())
		return nullptr;

	// iterate records in reverse.
	for (auto it = data->m_records.crbegin(); it != data->m_records.crend(); ++it) {
		current = it->get();

		// if this record is valid.
		// we are done since we iterated in reverse.
		if (current->valid() && !current->immune() && !current->dormant())
			return current;
	}

	return nullptr;
}

void c_resolver::MatchShot(AimPlayer* data, LagRecord* record) {
	// do not attempt to do this in nospread mode.

	float shoot_time = -1.f;

	Weapon* weapon = data->m_player->GetActiveWeapon();
	if (weapon) {
		// with logging this time was always one tick behind.
		// so add one tick to the last shoot time.
		shoot_time = weapon->m_fLastShotTime() + c_csgo::get()->m_globals->m_interval;
	}

	// this record has a shot on it.
	if (game::TIME_TO_TICKS(shoot_time) == game::TIME_TO_TICKS(record->m_sim_time)) {
		record->m_shot = true;
	}
}

void c_resolver::SetMode(LagRecord* record) {
	// the resolver has 3 modes to chose from.
	// these modes will vary more under the hood depending on what data we have about the player
	// and what kind of hack vs. hack we are playing (mm/nospread).

	float speed = record->m_anim_velocity.length();

	// if not on ground.
	if (!(record->m_flags & FL_ONGROUND))
		record->m_mode = Modes::RESOLVE_AIR;

	// if on ground and moving.
	else if (speed > 6.f)
		record->m_mode = Modes::RESOLVE_WALK;

	// if on ground and not moving.
	else
		record->m_mode = Modes::RESOLVE_STAND;
}

void c_resolver::ResolveAngles(Player* player, LagRecord* record) {
	AimPlayer* data = &c_aimbot::get()->m_players[player->index() - 1];

	// mark this record if it contains a shot.
	MatchShot(data, record);

	// next up mark this record with a resolver mode that will be used.
	SetMode(record);


	float EyeDelta = player->m_PlayerAnimState()->m_flEyeYaw - player->m_PlayerAnimState()->m_flGoalFeetYaw;
	bool LowDelta = EyeDelta < 30.f;
	bool HalfDelta = EyeDelta <= 30.f;
	float desync_delta;
	if (HalfDelta)
		desync_delta = player->GetMaxBodyRotation() / 2;
	else if (LowDelta)
		desync_delta = player->GetMaxBodyRotation() / 2.85;
	else
		desync_delta = player->GetMaxBodyRotation();

	// if we are in nospread mode, force all players pitches to down.
	// TODO; we should check thei actual pitch and up too, since those are the other 2 possible angles.
	// this should be somehow combined into some iteration that matches with the air angle iteration.
	float side;
	int shots = data->m_missed_shots;

	float max_rotation = player->GetMaxBodyRotation();

	// setup a starting brute angle.
	float resolve_value = 45.f;
	static float brute = 0.f;

	float eye_yaw = player->m_PlayerAnimState()->m_flEyeYaw;

	// clamp our starting brute angle, to not brute an angle, we most likely can't hit.
	if (max_rotation < resolve_value)
		resolve_value = max_rotation;

	// detect if player is using maximum desync.
	bool m_extending = record->m_layers[3].m_cycle == 0.f && record->m_layers[3].m_weight == 0.f;


	if (record->m_layers[3].m_sequence == 979 && player->m_vecVelocity().length_2d() == 0.f)
	{
		float LbyAngle = player->m_flLowerBodyYawTarget();

		side = (LbyAngle > 0.f) ? -1 : 1;
	}
	else if (player->m_vecVelocity().length_2d() == 0.f)
	{
		side = (EyeDelta > 0.f) ? -1 : 1;
	}
	else
		side = 0;


	if (side == 0)
	{
		switch (data->m_missed_shots % 6) {
		case 0:
			brute = eye_yaw + desync_delta;
			break;
		case 1:
			brute = eye_yaw - desync_delta;
			break;
		case 2:
			brute = eye_yaw + 30;
			break;
		case 3:
			brute = eye_yaw - 30;
			break;
		case 4:
			brute = eye_yaw + 15;
			break;
		case 5:
			brute = eye_yaw - 17;
			break;
		}
	}
	else
	{
		switch (data->m_missed_shots % 6) {
		case 0:
			brute = eye_yaw + desync_delta * side;
			break;
		case 1:
			brute = eye_yaw + desync_delta * -side;
			break;
		case 2:
			brute = eye_yaw - 17;
			break;
		case 3:
			brute = eye_yaw + 19;
			break;
		case 4:
			brute = eye_yaw - 30;
			break;
		case 5:
			brute = eye_yaw + 30;
			break;
		}
	}



	player->m_PlayerAnimState()->m_flGoalFeetYaw = math::NormalizedAngle(brute);
}
