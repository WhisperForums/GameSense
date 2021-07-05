#include "includes.h"

bool Hooks::InPrediction() {
	Stack stack;
	ang_t *angles;

	// note - dex; first 2 'test al, al' instructions in C_BasePlayer::CalcPlayerView.
	static Address CalcPlayerView_ret1{ pattern::find(c_csgo::get()->m_client_dll, XOR("84 C0 75 0B 8B 0D ? ? ? ? 8B 01 FF 50 4C")) };
	static Address CalcPlayerView_ret2{ pattern::find(c_csgo::get()->m_client_dll, XOR("84 C0 75 08 57 8B CE E8 ? ? ? ? 8B 06")) };

	/*static const Address MaintainSequenceTransitionsAddr{ pattern::find(c_csgo::get()->m_client_dll, XOR("84 C0 74 17 8B 87")) };

	if (stack.ReturnAddress() == MaintainSequenceTransitionsAddr)
		return false;*/

	if (c_client::get()->m_local) {
		// note - dex; apparently this calls 'view->DriftPitch()'.
		//             i don't know if this function is crucial for normal gameplay, if it causes issues then comment it out.
		if (stack.ReturnAddress() == CalcPlayerView_ret1)
			return true;

		if (stack.ReturnAddress() == CalcPlayerView_ret2) {
			// at this point, angles are copied into the CalcPlayerView's eyeAngles argument.
			// (ebp) InPrediction -> (ebp) CalcPlayerView + 0xC = eyeAngles.
			angles = stack.next().arg(0xC).to< ang_t* >();

			if (angles) {
				*angles -= c_client::get()->m_local->m_viewPunchAngle()
					+ (c_client::get()->m_local->m_aimPunchAngle() * c_csgo::get()->weapon_recoil_scale->GetFloat())
					* c_csgo::get()->view_recoil_tracking->GetFloat();
			}

			return true;
		}
	}

	return g_hooks.m_prediction.GetOldMethod< InPrediction_t >(CPrediction::INPREDICTION)(this);
}

void Hooks::RunCommand( Entity* ent, CUserCmd* cmd, IMoveHelper* movehelper ) {
	// airstuck jitter / overpred fix.
	if( cmd->m_tick >= std::numeric_limits< int >::max( ) )
		return;

	g_hooks.m_prediction.GetOldMethod< RunCommand_t >( CPrediction::RUNCOMMAND )( this, ent, cmd, movehelper );

	// store non compressed netvars.
	g_netdata.store( );
}
