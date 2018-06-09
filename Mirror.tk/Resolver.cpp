#include "Resolver.h"
#include "Ragebot.h"
#include "Hooks.h"
#include "RenderManager.h"


#ifdef NDEBUG
#define XorStr( s ) ( XorCompileTime::XorString< sizeof( s ) - 1, __COUNTER__ >( s, std::make_index_sequence< sizeof( s ) - 1>() ).decrypt() )
#else
#define XorStr( s ) ( s )
#endif
float get_average_lby_standing_update_delta(IClientEntity* player) {
	static float last_update_time[64];
	static float second_laste_update_time[64];
	static float oldlowerbody[64];
	float lby = static_cast<int>(fabs(player->GetEyeAnglesPointer()->y - player->GetLowerBodyYaw()));

	if (lby != oldlowerbody[player->GetIndex()]) {
		second_laste_update_time[player->GetIndex()] = last_update_time[player->GetIndex()];
		last_update_time[player->GetIndex()] = Interfaces::Globals->curtime;
		oldlowerbody[player->GetIndex()] = lby;
	}

	return last_update_time[player->GetIndex()] - second_laste_update_time[player->GetIndex()];
}

bool lby_keeps_updating() {
	return get_average_lby_standing_update_delta;
}


void ResolverSetup::preso(IClientEntity * pEntity)
{
	if (Options::Menu.RageBotTab.preso.GetIndex() == 1)
	{
		pEntity->GetEyeAnglesXY()->x = 89;
	}
	else if (Options::Menu.RageBotTab.preso.GetIndex() == 2)
	{
		pEntity->GetEyeAnglesXY()->x = -89;
	}
	else if (Options::Menu.RageBotTab.preso.GetIndex() == 3)
	{
		pEntity->GetEyeAnglesXY()->x = 0;
	}
	else if (Options::Menu.RageBotTab.preso.GetIndex() == 4)
	{
		float last_simtime[64] = { 0.f };
		float stored_pitch[64] = { 89.f };
		float last_lby[64] = { 0.f };
		float last_lby_delta[64] = { 0.f };
		float large_lby_delta[64] = { 0.f };
		float moving_lby[64] = { 0.f };
		bool  was_moving[64] = { false };

		const auto local = hackManager.pLocal();
		if (!local) return;

		for (auto i = 0; i < Interfaces::Engine->GetMaxClients(); ++i) {
			const auto player = const_cast <IClientEntity*>(Interfaces::EntList->GetClientEntity(i));


			if (!player || local == player || player->GetTeamNum() == local->GetTeamNum() || player->IsImmune() || player->IsDormant()) {
				last_simtime[i] = 0.f;
				stored_pitch[i] = 89.f;
				continue;
			}


			const auto eye = player->GetEyeAnglesXY();
			const auto sim = player->GetSimulationTime();



			auto missed = Globals::fired - Globals::hit;
			while (missed > 5) missed -= 5;
			while (missed < 0) missed += 5;

			auto is_legit = false;
			auto update = false;


			if (sim - last_simtime[i] >= 1)
			{
				stored_pitch[i] = eye->x;
				last_simtime[i] = sim;
			}

			player->GetEyeAnglesXY()->x = stored_pitch[i];
		}


	}
}


player_info_t GetInfo2(int Index) {
	player_info_t Info;
	Interfaces::Engine->GetPlayerInfo(Index, &Info);
	return Info;
}


float Bolbilize(float Yaw)
{
	if (Yaw > 180)
	{
		Yaw -= (round(Yaw / 360) * 360.f);
	}
	else if (Yaw < -180)
	{
		Yaw += (round(Yaw / 360) * -360.f);
	}
	return Yaw;
}

float GetCurTime(CUserCmd* ucmd) {
	IClientEntity* local_player = hackManager.pLocal();
	static int g_tick = 0;
	static CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
		g_tick = (float)local_player->GetTickBase();
	}
	else {
		// Required because prediction only runs on frames, not ticks
		// So if your framerate goes below tickrate, m_nTickBase won't update every tick
		++g_tick;
	}
	g_pLastCmd = ucmd;
	float curtime = g_tick * Interfaces::Globals->interval_per_tick;
	return curtime;
}

Vector CalcAngle69(Vector dst, Vector src)
{
	Vector angles;

	double delta[3] = { (src.x - dst.x), (src.y - dst.y), (src.z - dst.z) };
	double hyp = sqrt(delta[0] * delta[0] + delta[1] * delta[1]);
	angles.x = (float)(atan(delta[2] / hyp) * 180.0 / 3.14159265);
	angles.y = (float)(atanf(delta[1] / delta[0]) * 57.295779513082f);
	angles.z = 0.0f;

	if (delta[0] >= 0.0)
	{
		angles.y += 180.0f;
	}

	return angles;
}

bool HasFakeHead(IClientEntity* pEntity) {

	return abs(pEntity->GetEyeAnglesXY()->y - pEntity->GetLowerBodyYaw()) > 35;
}
bool Lbywithin35(IClientEntity* pEntity) {

	if ((pEntity->GetLowerBodyYaw() - 36 < pEntity->GetEyeAnglesXY()->y && pEntity->GetLowerBodyYaw() + 15 > pEntity->GetEyeAnglesXY()->y) || (pEntity->GetLowerBodyYaw() + 36 > pEntity->GetEyeAnglesXY()->y && pEntity->GetLowerBodyYaw() - 15 < pEntity->GetEyeAnglesXY()->y))
		return true;
	else
		return false;

}
bool IsMovingOnGround(IClientEntity* pEntity) {

	return pEntity->GetVelocity().Length2D() > 25.f && pEntity->GetFlags() & FL_ONGROUND;
}



bool IsMovingOnInAir(IClientEntity* pEntity) {

	return !(pEntity->GetFlags() & FL_ONGROUND);
}
bool OnGround(IClientEntity* pEntity) {

	return pEntity->GetFlags() & FL_ONGROUND;
}
bool IsFakeWalking(IClientEntity* pEntity) {

	return IsMovingOnGround(pEntity) && (pEntity->GetVelocity().Length2D() <= 25.0f && pEntity->GetVelocity().Length2D() > 1.1f);
}

inline float RandomFloat(float min, float max)
{
	static auto fn = (decltype(&RandomFloat))(GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomFloat"));
	return fn(min, max);
}
void BETA_LBYBreakerCorrections(IClientEntity* pEntity)
{
	float movinglby[64];
	float lbytomovinglbydelta[64];
	bool onground = pEntity->GetFlags() & FL_ONGROUND;

	if (Options::Menu.RageBotTab.LBYCorrection.GetState())
	{
		lbytomovinglbydelta[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw() - lbytomovinglbydelta[pEntity->GetIndex()];

		if (pEntity->GetVelocity().Length2D() > 6 && pEntity->GetVelocity().Length2D() < 42)
		{
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 120;
		}
		else if (pEntity->GetVelocity().Length2D() < 6 || pEntity->GetVelocity().Length2D() > 42) // they are moving
		{
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
			movinglby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
		}
		else if (lbytomovinglbydelta[pEntity->GetIndex()] > 50 && lbytomovinglbydelta[pEntity->GetIndex()] < -50 &&
			lbytomovinglbydelta[pEntity->GetIndex()] < 112 && lbytomovinglbydelta[pEntity->GetIndex()] < -112)
		{
			pEntity->GetEyeAnglesXY()->y = movinglby[pEntity->GetIndex()];
		}
		else pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
	}
}




static int GetSequenceActivity(IClientEntity* pEntity, int sequence)
{
	const model_t* pModel = pEntity->GetModel();
	if (!pModel)
		return 0;

	auto hdr = Interfaces::ModelInfo->GetStudiomodel(pEntity->GetModel());

	if (!hdr)
		return -1;

	static auto get_sequence_activity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(Utilities::Memory::FindPatternV2("client.dll", "55 8B EC 83 7D 08 FF 56 8B F1 74 3D"));

	return get_sequence_activity(pEntity, hdr, sequence);
}

static bool stored[128] = { false };
float neweye[128];
float lastLBY[65];
static float storedLBYFromUpdate[65] = { 0.f };
static float staticLBYFromUpdate[65] = { 0.f };
static double oldStoredCurtime[65] = { 0.f };
static bool firstBreak[65] = { false };
static float lastSimtime[65] = { false };

static float pelvisVelocity[65][65] = { 0.0f };
float pelvisMaxVelocityLog[65][6] = { 0.0f };
float pelvisAverageVelocity[65] = { 0.0f };
int pelvisCalculationCounter[125] = { 0 };
static bool didHit = false;
float deltaDiffName[65] = { 0 };
bool lastDeltaOver120[65] = { false };
float saveYaw = 0.f;
static Vector lastPelvis[65] = { Vector(0, 0, 0) };
static Vector yawangle[65] = { Vector(0, 0, 0) };

float NormalizeFloatToAngle(float input)
{
	for (auto i = 0; i < 3; i++) {
		while (input < -180.0f) input += 360.0f;
		while (input > 180.0f) input -= 360.0f;
	}
	return input;
}


bool ResolverSetup::lbyDeltaOver120(int plID)
{

	if (lastDeltaOver120[plID] && pelvisAverageVelocity[plID] > 3.f)
		deltaDiffName[plID] = Interfaces::Globals->curtime;
	else if (pelvisAverageVelocity[plID] > 6.f &&  Interfaces::Globals->curtime - deltaDiffName[plID] > .1f)
	{
		deltaDiffName[plID] = Interfaces::Globals->curtime;
		lastDeltaOver120[plID] = true;
	}
	else if (!lastDeltaOver120[plID] && pelvisAverageVelocity[plID] < 3.f)
		deltaDiffName[plID] = Interfaces::Globals->curtime;
	else if (pelvisAverageVelocity[plID] < 3.f &&  Interfaces::Globals->curtime - deltaDiffName[plID] > .1f)
	{
		deltaDiffName[plID] = Interfaces::Globals->curtime;
		lastDeltaOver120[plID] = false;
	}
	return lastDeltaOver120[plID];
}



float override_yaw(IClientEntity* player, IClientEntity* local) {
	Vector eye_pos, pos_enemy;
	CalcAngle(player->GetEyePosition(), local->GetEyePosition(), eye_pos);
	if (Render::TransformScreen(player->GetOrigin(), pos_enemy)) {
		if (GUI.GetMouse().x < pos_enemy.x)
			return (eye_pos.y - 90);
		else if (GUI.GetMouse().x > pos_enemy.x)
			return (eye_pos.y + 90);
	}
	return eye_pos.y + 180;
}



bool ResolverSetup::didLBYUpdate(IClientEntity* player)
{
	int curTarget = player->GetIndex();

	double cTime = player->GetSimulationTime() + curTarget;




	for (int s = 0; s < 14; s++)
	{
		auto anim_layer = player->GetAnimOverlay(s);
		if (!anim_layer.m_pOwner)
			continue;
		auto AnimeLayer = &player->GetAnimOverlays()[1];
		auto activity = GetSequenceActivity(player, AnimeLayer->m_nSequence);

		if (activity == -1)
			continue;
		if (activity == 979 && (anim_layer.m_flCycle > 0.862494) && player->GetVelocity().Length2D() < 38.f)
		{

			storedLBYFromUpdate[curTarget] = player->GetLowerBodyYaw();
			ResolverSetup::lbyUpdate[player->GetIndex()] = true;
			oldStoredCurtime[curTarget] = cTime + 1.1;
			return true;
		}
	}

	return false;
}

bool lowerBodyYawUpdated(IClientEntity* pEntity) {
	for (int w = 0; w < 14; w++)
	{
		AnimationLayer currentLayer = pEntity->GetAnimOverlay(w);
		const int activity = GetSequenceActivity(pEntity, currentLayer.m_nSequence);
		float flcycle = currentLayer.m_flCycle, flprevcycle = currentLayer.m_flPrevCycle, flweight = currentLayer.m_flWeight, flweightdatarate = currentLayer.m_flWeightDeltaRate;
		uint32_t norder = currentLayer.m_nOrder;
		if (activity == 973 && flweight == 1.f || activity == 979 && flweight == 1.0f && currentLayer.m_flPrevCycle != currentLayer.m_flCycle)// 961 looks like they shot //flweight seems like right as the animation or right after 972 could be moving
			return true;
	}
	return false;
}
bool didLBYUpdateOver120(IClientEntity* player)
{
	int curTarget = player->GetIndex();

	double cTime = player->GetSimulationTime() + curTarget;




	for (int s = 0; s < 14; s++)
	{
		auto anim_layer = player->GetAnimOverlay(s);
		if (!anim_layer.m_pOwner)
			continue;
		auto AnimeLayer = &player->GetAnimOverlays()[1];
		auto activity = GetSequenceActivity(player, AnimeLayer->m_nSequence);

		if (activity == -1) //If idle animation, skip it
			continue;
		if (activity == 979 && player->GetVelocity().Length2D() < 38.f)
		{
			return true;
		}
	}

	return false;
}


bool hurt[65] = { false };
int hurtType[65] = { 0 };


float YawDelta[64];
float reset[64];
float Delta[64];
float Resolved_angles[64];
float OldLowerBodyYaw[64];





void ResolverSetup::Resolve(IClientEntity* pEntity, int CurrentTarget)
{






#define RandomInt(nMin, nMax) (rand() % (nMax - nMin + 1) + nMin);
	std::string aa_info[64];


	//------bool------//

	std::vector<int> HitBoxesToScan;




	bool MeetsLBYReq;
	bool maybefakehead = 0;

	//------bool------//

	//------float------//



	//------float------//


	//------Statics------//
	static Vector Skip[65];
	static float StaticHeadAngle = 0;

	static bool GetEyeAngles[65]; //Resolve: Frame EyeAngle
	static bool GetLowerBodyYawTarget[65]; //Resolve: LowerBody
	static bool isLBYPredictited[65];
	static bool switch2;

	static float OldLowerBodyYaws[65];
	static float OldYawDeltas[65];
	static float oldTimer[65];








	ResolverSetup::NewANgles[pEntity->GetIndex()] = *pEntity->GetEyeAnglesXY();
	ResolverSetup::newlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResolverSetup::newsimtime = pEntity->GetSimulationTime();
	ResolverSetup::newdelta[pEntity->GetIndex()] = pEntity->GetEyeAnglesXY()->y;
	ResolverSetup::newlbydelta[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResolverSetup::finaldelta[pEntity->GetIndex()] = ResolverSetup::newdelta[pEntity->GetIndex()] - ResolverSetup::storeddelta[pEntity->GetIndex()];
	ResolverSetup::finallbydelta[pEntity->GetIndex()] = ResolverSetup::newlbydelta[pEntity->GetIndex()] - ResolverSetup::storedlbydelta[pEntity->GetIndex()];
	if (newlby == storedlby)
		ResolverSetup::lbyupdated = false;
	else
		ResolverSetup::lbyupdated = true;
	StoreThings(pEntity);

	static float LatestLowerBodyYawUpdateTime[55];

	IClientEntity* pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();


	if (pEntity->GetFlags() & FL_ONGROUND)
		MeetsLBYReq = true;
	else
		MeetsLBYReq = false;

	StoreFGE(pEntity);






	static float time_at_update[65];
	float kevin[64];
	static bool bLowerBodyIsUpdated = false;

	if (pEntity->GetLowerBodyYaw() != kevin[pEntity->GetIndex()])
		bLowerBodyIsUpdated = true;
	else
		bLowerBodyIsUpdated = false;

	if (pEntity->GetVelocity().Length2D() < 1)
	{
		kevin[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();//storing their moving lby for later
		LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] = pEntity->GetSimulationTime() + 1.1;

	}




	bool ismoving; if (pEntity->GetVelocity().Length2D() > 1.1) ismoving = true; else ismoving = false;


	float LBY = pEntity->GetLowerBodyYaw();
	float LBYDelta = pEntity->GetEyeAnglesXY()->y - LBY;

	bool fakespin;
	bool fakespin2;
	bool fakespin4;

	bool lbybreak;
	bool fakespin3;


	if (newlby == storedlby)
		ResolverSetup::lbyupdated = false;
	else
		ResolverSetup::lbyupdated = true;

	if (LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] = pEntity->GetSimulationTime() + 0.4 && pEntity->GetVelocity().Length2D() < 1.0f && FL_ONGROUND)
	{

		if (ResolverSetup::newdelta[pEntity->GetIndex()] < (pEntity->GetEyeAnglesXY()->y + 60) || ResolverSetup::newdelta[pEntity->GetIndex()] > (pEntity->GetEyeAnglesXY()->y - 60))
		{
			if (LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] = pEntity->GetSimulationTime() + 0.2 && pEntity->GetVelocity().Length2D() < 1.0f)
			{
				if (ResolverSetup::newdelta[pEntity->GetIndex()] < ((pEntity->GetEyeAnglesXY()->y + 60) * 2) || ResolverSetup::newdelta[pEntity->GetIndex()] > ((pEntity->GetEyeAnglesXY()->y - 60) * 2))
				{
					fakespin = true;
				}
				else
					fakespin3 = true;

				if ((ResolverSetup::newdelta[pEntity->GetIndex()] + 30) < LBY && (ResolverSetup::newdelta[pEntity->GetIndex()] - 30) > LBY)
				{
					lbybreak = true;
				}
				else
					lbybreak = false;
			}
		}
		if ((((ResolverSetup::newdelta[pEntity->GetIndex()]) < (pEntity->GetEyeAnglesXY()->y + 25) && (ResolverSetup::newdelta[pEntity->GetIndex()]) > (pEntity->GetEyeAnglesXY()->y + 25)) || (ResolverSetup::newdelta[pEntity->GetIndex()] > (pEntity->GetEyeAnglesXY()->y - 35) && (ResolverSetup::newdelta[pEntity->GetIndex()]) < (pEntity->GetEyeAnglesXY()->y - 35))) && ResolverSetup::newdelta[pEntity->GetIndex()] > (pEntity->GetEyeAnglesXY()->y + 60) || ResolverSetup::newdelta[pEntity->GetIndex()] < (pEntity->GetEyeAnglesXY()->y - 60))
		{
			fakespin2 = true;
		}
		if ((((ResolverSetup::newdelta[pEntity->GetIndex()]) < (pEntity->GetEyeAnglesXY()->y + 85) && (ResolverSetup::newdelta[pEntity->GetIndex()]) > (pEntity->GetEyeAnglesXY()->y + 85)) || (ResolverSetup::newdelta[pEntity->GetIndex()] > (pEntity->GetEyeAnglesXY()->y - 85) && (ResolverSetup::newdelta[pEntity->GetIndex()]) < (pEntity->GetEyeAnglesXY()->y - 85))))
		{
			fakespin4 = true;
		}
		if ((ResolverSetup::newdelta[pEntity->GetIndex()]) < (pEntity->GetEyeAnglesXY()->y + 20) || ResolverSetup::newdelta[pEntity->GetIndex()] > (pEntity->GetEyeAnglesXY()->y - 20))
		{
			if (LBY != pEntity->GetEyeAnglesXY()->y &&  pEntity->GetEyeAnglesXY()->y <= (LBYDelta + 30) && pEntity->GetEyeAnglesXY()->y >= (LBYDelta - 30))
			{

				lbybreak = true;
				fakespin2 = true;
			}
			else
			{

				fakespin2 = true;
				lbybreak = false;
			}
		}
		if ((LBY != pEntity->GetEyeAnglesXY()->y &&  pEntity->GetEyeAnglesXY()->y <= (LBYDelta + 40) && ((pEntity->GetEyeAnglesXY()->y >= (LBYDelta - 40) && ((ResolverSetup::newdelta[pEntity->GetIndex()]) > (pEntity->GetEyeAnglesXY()->y + 30) || (ResolverSetup::newdelta[pEntity->GetIndex()]) < (pEntity->GetEyeAnglesXY()->y - 30))) || !((ResolverSetup::newdelta[pEntity->GetIndex()]) < (pEntity->GetEyeAnglesXY()->y + 20) || ResolverSetup::newdelta[pEntity->GetIndex()] > (pEntity->GetEyeAnglesXY()->y - 20)))))
		{
			lbybreak = true;
			fakespin = false;
			fakespin2 = false;
		}
		else
		{
			lbybreak = false;
			fakespin = false;
			fakespin2 = false;
		}
	}


	float moving_lby[64];
	float last_moving_lby[64];

	float moving_lby2[64];
	float last_moving_lby2[64];

	float moving_pitch[64];
	float last_pitch[64];

	float delta = pEntity->GetEyeAnglesXY()->y - pEntity->GetLowerBodyYaw();

	float loglby[64];
	float loggedlby[64];

	if (Options::Menu.RageBotTab.resolver.GetIndex() == 1)
	{



		for (int i = 0; i < Interfaces::Engine->GetMaxClients(); ++i)
		{

			IClientEntity *player = (IClientEntity*)Interfaces::EntList->GetClientEntity(i);

			if (!player || player->IsDormant() || player->GetHealth() < 1 || (DWORD)player == (DWORD)pLocal)
				continue;

			if (!player)
				continue;

			if (pLocal)
				continue;

		}

		if (LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] < pEntity->GetSimulationTime() || bLowerBodyIsUpdated)
		{
			LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] = pEntity->GetSimulationTime() + 1.1;

			switch (Globals::missedshots % 4)
			{
			case 0: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw(); break;
			case 1: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()]; break;
			case 2: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 35; break;
			case 3: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 35; break;
			}

		}
		else
		{
			if (pEntity->GetVelocity().Length2D() > 45 & pEntity->GetFlags() & FL_ONGROUND)
			{
				moving_lby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
				last_moving_lby[pEntity->GetIndex()] = moving_lby[pEntity->GetIndex()];

				pEntity->GetEyeAnglesXY()->y = moving_lby[pEntity->GetIndex()];

			}
			else if (IsMovingOnGround(pEntity))
			{
				switch (Globals::missedshots % 4)
				{
				case 0: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw(); break;
				case 1: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 45; break;
				case 2: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw(); break;
				case 3: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 45; break;
				}
			}
			else if (IsFakeWalking(pEntity) && !(pEntity->GetMoveType() & FL_DUCKING))
			{
				switch (Globals::missedshots % 4)
				{
				case 0: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()]; break;
				case 1: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] + 60; break;
				case 2: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] - 60; break;
				case 3: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] + 180; break;
				}
			}
			else if (IsMovingOnInAir(pEntity))
			{
				switch (Globals::missedshots % 4) // 
				{
				case 0: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()]; break;
				case 1: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] - 75; break;
				case 2: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] + 75; break;
				case 3: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] + 180; break;
				}
			}
			else if (!ismoving && MeetsLBYReq)
			{
				if (Lbywithin35(pEntity))
				{
					if (HasFakeHead(pEntity))
					{
						switch (Globals::missedshots % 5)
						{
						case 0: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()]; break;
						case 1: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] + 25; break;
						case 2: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] - 25; break;
						case 3: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] + 15; break;
						case 4: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 145; break;
						}
					}
					else
					{
						switch (Globals::missedshots % 5)
						{
						case 0: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()]; break;
						case 1: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] + 25; break;
						case 2: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] - 25; break;
						case 3: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] + 15; break;
						case 4: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 145; break;
						}
					}
				}
				else
				{
					switch (Globals::missedshots % 5)
					{
					case 0: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()]; break;
					case 1: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] + 25; break;
					case 2: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] - 35; break;
					case 3: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] + 45; break;
					case 4: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 145; break;
					}
				}


			}
			else
			{
				switch (Globals::missedshots % 5)
				{
				case 0: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw(); break;
				case 1: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] + 25; break;
				case 2: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] - 35; break;
				case 3: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 45; break;
				case 4: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 145; break;
				}
			}
		}

	}
	else if (Options::Menu.RageBotTab.resolver.GetIndex() == 2)
	{


		float last_simtime[64] = { 0.f };
		float stored_pitch[64] = { 89.f };
		float last_lby[64] = { 0.f };
		float last_lby_delta[64] = { 0.f };
		float large_lby_delta[64] = { 0.f };
		float moving_lby[64] = { 0.f };
		bool  was_moving[64] = { false };

		auto legit = false;
		auto update = false;


		const auto local = hackManager.pLocal();

		if (!local) return;

		for (auto i = 0; i < Interfaces::Engine->GetMaxClients(); ++i) {
			const auto player = const_cast <IClientEntity*>(Interfaces::EntList->GetClientEntity(i));



			if (!player || local == player || player->GetTeamNum() == local->GetTeamNum() || player->IsImmune() || player->IsDormant()) {
				last_simtime[i] = 0.f;
				stored_pitch[i] = 89.f;
				last_lby[i] = 0.f;
				last_lby_delta[i] = 0.f;
				large_lby_delta[i] = 0.f;
				was_moving[i] = false;
				continue;
			}

			const auto lby = player->GetLowerBodyYaw();
			const auto eye = player->GetEyeAnglesXY();
			const auto sim = player->GetSimulationTime();
			const auto vel = player->GetVelocity().Length2D();

			auto missed = Globals::fired - Globals::hit;
			while (missed > 8) missed -= 8;
			while (missed < 0) missed += 8;

			auto angle = 0.f;
			auto angle2 = 0.f;

			if (lby != last_lby[i]) {
				update = true;
				auto delta = fabsf(lby - last_lby[i]);
				last_lby_delta[i] = delta;
				if (delta > 90)
				{
					large_lby_delta[i] = delta;
				}
				last_lby[i] = lby;
			}


			if (pEntity->GetVelocity().Length2D() > 1)
			{
				if (pEntity->GetFlags() & FL_ONGROUND)
				{
					if (pEntity->GetVelocity().Length2D() > 35)
					{
						moving_lby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
						last_moving_lby[pEntity->GetIndex()] = moving_lby[pEntity->GetIndex()];
						was_moving[i] = true;

						if (GetAsyncKeyState(Options::Menu.RageBotTab.OverrideKey.GetKey()) && vel < 100)
						{
							angle = override_yaw(player, local);
						}
						else
						{
							switch (Globals::missedshots % 5)
							{
							case 0: angle = pEntity->GetLowerBodyYaw() - 10; break;
							case 1: angle = moving_lby[pEntity->GetIndex()]; break;
							case 2: angle = moving_lby[pEntity->GetIndex()] + 25; break;
							case 3: angle = last_moving_lby[pEntity->GetIndex()]; break;
							case 4: angle = moving_lby[pEntity->GetIndex()]; break;
							}
						}
					}
					else
					{
						if (GetAsyncKeyState(Options::Menu.RageBotTab.OverrideKey.GetKey()) && vel < 60)
						{
							angle = override_yaw(player, local);
						}
						else
						{
							if (pEntity->GetMoveType() & FL_DUCKING)
							{
								switch (Globals::missedshots % 5)
								{
								case 0: angle = lby - 25; break;
								case 1: angle = lby; break;
								case 2: angle = lby + 25; break;
								case 3: angle = last_moving_lby[pEntity->GetIndex()] + 30; break;
								case 4: angle = last_moving_lby[pEntity->GetIndex()] - 30; break;
								}
							}
							else
							{
								switch (Globals::missedshots % 5)
								{
								case 0: angle = last_moving_lby[pEntity->GetIndex()] + 15; break;
								case 1: angle = lby - 15; break;
								case 2: angle = lby + 15; break;
								case 3: angle = lby + 45; break;
								case 4: angle = last_moving_lby[pEntity->GetIndex()] - 40; break;
								}
							}
						}
					}
				}
				else
				{
					switch (missed)
					{
					case 0: angle = lby + 25; break;
					case 1: angle = lby - 45; break;
					case 2: angle = lby - 180; break;
					case 3: angle = last_moving_lby[pEntity->GetIndex()] - 45; break;
					case 4: angle = last_moving_lby[pEntity->GetIndex()] + 45; break;
					}
				}
			}
			else
			{
				if (GetAsyncKeyState(Options::Menu.RageBotTab.OverrideKey.GetKey()) && vel < 60)
				{
					angle = override_yaw(player, local);
				}
				if (sim - last_simtime[i] >= 1)
				{
					if (sim - last_simtime[i] == 1)
					{
						legit = true;
						switch (missed)
						{
						case 0: angle = last_moving_lby[pEntity->GetIndex()] - 20; break;
						case 1: angle = last_moving_lby[pEntity->GetIndex()] + 20; break;
						case 2: angle = pEntity->GetLowerBodyYaw(); break;
						case 3: angle = pEntity->GetLowerBodyYaw() ; break;
						}
					}
				}
				else
				{
					

					if (was_moving[i])
					{
						switch (Globals::missedshots % 5) {
						case 0: angle = moving_lby[i]; break;
						case 1: angle = lby + large_lby_delta[i]; break;
						case 2: angle = lby + last_lby_delta[i]; break;
						case 3: angle = moving_lby[i]; break;
						case 4: angle = moving_lby[i] + 180; break;
						default: angle = last_moving_lby[pEntity->GetIndex()];
						}

					}
					else
					{

						switch (Globals::missedshots % 9)
						{
						case 0: angle = last_moving_lby[pEntity->GetIndex()] - 25; break;
						case 1: angle = last_moving_lby[pEntity->GetIndex()] + 45; break;
						case 2: angle = lby + last_lby_delta[i]; break;
						case 3: angle = lby + large_lby_delta[i]; break;
						case 4: angle = last_moving_lby[pEntity->GetIndex()]; break;
						case 5: angle = angle - 120; break;
						case 6: angle = angle + 120; break;
						case 7: angle = angle - last_lby_delta[i]; break;
						case 8: angle = angle - 180; break;
						}

					}
					
				}
			}
			player->GetEyeAnglesXY()->y = angle;
		}
	}
	else if (Options::Menu.RageBotTab.resolver.GetIndex() == 3)
	{

		float last_simtime[64] = { 0.f };
		float last_lby[64] = { 0.f };
		float last_lby_delta[64] = { 0.f };
		float large_lby_delta[64] = { 0.f };
		float moving_lby[64] = { 0.f };
		bool  was_moving[64] = { false };

		const auto local = hackManager.pLocal();
		if (!local) return;

		for (auto i = 0; i < Interfaces::Engine->GetMaxClients(); ++i) {
			const auto player = const_cast < IClientEntity* >(Interfaces::EntList->GetClientEntity(i));


			if (!player || local == player || player->GetTeamNum() == local->GetTeamNum() || player->IsImmune() || player->IsDormant()) {
				last_simtime[i] = 0.f;

				last_lby[i] = 0.f;
				last_lby_delta[i] = 0.f;
				large_lby_delta[i] = 0.f;
				was_moving[i] = false;
				continue;
			}

			//grab values from player
			const auto lby = player->GetLowerBodyYaw();
			const auto eye = player->GetEyeAnglesXY();
			const auto sim = player->GetSimulationTime();
			const auto vel = player->GetVelocity().Length2D();

			//auto missed = Globals::missedshots;
			auto missed = Globals::fired - Globals::hit;
			while (missed > 5) missed -= 5;
			while (missed < 0) missed += 5;

			auto is_legit = false;
			auto update = false;


			if (sim - last_simtime[i] >= 1) {
				if (sim - last_simtime[i] == 1)
				{

					is_legit = true;
				}


				last_simtime[i] = sim;
			}


			if (lby != last_lby[i]) {
				update = true;
				auto delta = fabsf(lby - last_lby[i]);
				last_lby_delta[i] = delta;
				if (delta > 90)
				{
					large_lby_delta[i] = delta;
				}
				last_lby[i] = lby;
			}


			auto angle = 0.f;
			if (GetAsyncKeyState(Options::Menu.RageBotTab.OverrideKey.GetKey()) && vel < 60) {
				angle = override_yaw(player, local);
			}
			if (is_legit)
			{
				if (GetAsyncKeyState(Options::Menu.RageBotTab.OverrideKey.GetKey()))
				{
					angle = override_yaw(pEntity, pLocal);
				}
				else
					angle = eye->y;
			}
			else if (update)
			{

				angle = lby;
			}

			else if (vel > 35) {
				angle = lby;
				moving_lby[i] = lby;
				was_moving[i] = true;
			}
			else if (was_moving[i])
			{
				if (GetAsyncKeyState(Options::Menu.RageBotTab.OverrideKey.GetKey()))
				{
					angle = override_yaw(pEntity, pLocal);
				}
				else
				{
					switch (missed) {
					case 0: angle = moving_lby[i]; break;
					case 1: angle = lby + large_lby_delta[i]; break;
					case 2: angle = lby + last_lby_delta[i]; break;
					case 3: angle = moving_lby[i]; break;
					case 4: angle = moving_lby[i] + 180; break;
					default: angle = lby - 120;
					}
				}
			}
			else {
				if (GetAsyncKeyState(Options::Menu.RageBotTab.OverrideKey.GetKey()))
				{
					angle = override_yaw(pEntity, pLocal);
				}
				else
				{
					switch (missed) {
					case 0:angle = lby - 120; break;
					case 1: angle = lby + large_lby_delta[i]; break;
					case 2: angle = lby + last_lby_delta[i]; break;
					case 3: angle = moving_lby[i] - 35; break;
					case 4: angle = moving_lby[i] + 145; break;
					default: angle = lby + (90 * (missed + 1));
					}
				}

			}


			player->GetEyeAnglesXY()->y = angle;
		}
	}

	else if (Options::Menu.RageBotTab.resolver.GetIndex() == 4)
	{


		CUserCmd* pCmd;
		bool  was_moving[64] = { false };

		for (auto i = 0; i < Interfaces::Engine->GetMaxClients(); ++i) {


			auto angle = 0.f;
			if (pEntity->GetVelocity().Length2D() > 1)
			{
				if (pEntity->GetFlags() & FL_ONGROUND)
				{
					if (pEntity->GetVelocity().Length2D() > 45)
					{
						moving_lby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
						last_moving_lby[pEntity->GetIndex()] = moving_lby[pEntity->GetIndex()];

						was_moving[i] = true;

						angle = moving_lby[pEntity->GetIndex()];
					}
					else
					{
						switch (Globals::missedshots % 4)
						{
						case 0: angle = pEntity->GetLowerBodyYaw(); break;
						case 1: angle = last_moving_lby[pEntity->GetIndex()]; break;
						case 2: angle = pEntity->GetLowerBodyYaw() - 25; break;
						case 3: angle = moving_lby[pEntity->GetIndex()] + 25; break;
						}
					}
				}
				else
				{
					switch (Globals::missedshots % 4)
					{
					case 0: angle = last_moving_lby[pEntity->GetIndex()]; break;
					case 1: angle = pEntity->GetLowerBodyYaw() - 45; break;
					case 2: angle = pEntity->GetLowerBodyYaw() - 180; break;
					case 3: angle = pEntity->GetLowerBodyYaw() + 135; break;
					}
				}

			}
			else
			{
				if (Lbywithin35(pEntity))
				{

					if (HasFakeHead(pEntity))
					{
						if (GetAsyncKeyState(Options::Menu.RageBotTab.OverrideKey.GetKey()))
						{
							angle = override_yaw(pEntity, pLocal);
						}
						else
						{
							switch (Globals::missedshots % 7)
							{
							case 0: angle = last_moving_lby[pEntity->GetIndex()] + 15; break;
							case 1: angle = last_moving_lby[pEntity->GetIndex()] - 45; break;
							case 2: angle = last_moving_lby[pEntity->GetIndex()] + 65; break;
							case 3: angle = angle - pEntity->GetLowerBodyYaw(); break;
							case 4: angle = (angle + pEntity->GetLowerBodyYaw()) - 120; break;
							case 5: angle = (angle + pEntity->GetLowerBodyYaw()) + 120; break;
							case 6: angle = (angle - pEntity->GetLowerBodyYaw()) - (150 - rand() % 30); break;
							}
						}
					}
					else
					{
						if (GetAsyncKeyState(Options::Menu.RageBotTab.OverrideKey.GetKey()))
						{
							angle = override_yaw(pEntity, pLocal);
						}
						else
						{
							switch (Globals::missedshots % 7)
							{
							case 0: angle = last_moving_lby[pEntity->GetIndex()] - 15; break;
							case 1: angle = last_moving_lby[pEntity->GetIndex()] + 15; break;
							case 2: angle = pEntity->GetLowerBodyYaw() - 119; break;
							case 3: angle = angle - 90; break;
							case 4: angle = angle - 60; break;
							case 5: angle = (angle + pEntity->GetLowerBodyYaw()) + 120; break;
							case 6: angle = (angle - pEntity->GetLowerBodyYaw()) - (150 - rand() % 30); break;
							}
						}
					}

				}
				else
				{
					if (GetAsyncKeyState(Options::Menu.RageBotTab.OverrideKey.GetKey()))
					{
						angle = override_yaw(pEntity, pLocal);
					}
					else
					{
						if (was_moving[i])
						{
							switch (Globals::missedshots % 7)
							{
							case 0: angle = last_moving_lby[pEntity->GetIndex()] - 10; break;
							case 1: angle = last_moving_lby[pEntity->GetIndex()] + 35; break;
							case 2: angle = last_moving_lby[pEntity->GetIndex()] + 90; break;
							case 3: angle = last_moving_lby[pEntity->GetIndex()] - 90; break;
							case 4: angle = last_moving_lby[pEntity->GetIndex()]; break;
							case 5: angle = last_moving_lby[pEntity->GetIndex()] - 180; break;
							case 6: angle = angle - last_moving_lby[pEntity->GetIndex()]; break;
							}
						}
						else
						{
							switch (Globals::missedshots % 7)
							{
							case 0: angle = angle - pEntity->GetLowerBodyYaw(); break;
							case 1: angle = angle - 90; break;
							case 2: angle = angle + 90; break;
							case 3: angle = pEntity->GetLowerBodyYaw(); break;
							case 4: angle = angle - pEntity->GetLowerBodyYaw(); break;
							case 5: angle = angle - 90; break;
							case 6: angle = angle + 180; break;
							}

						}
					}
				}

			}
			pEntity->GetEyeAnglesXY()->y = angle;
		}
	}

	else if (Options::Menu.LegitBotTab.legitresolver.GetState())
	{



		if (pEntity->GetVelocity().Length2D() >= 10)
		{
			if (pEntity->GetFlags() & FL_ONGROUND)
			{
				moving_lby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
				last_moving_lby[pEntity->GetIndex()] = moving_lby[pEntity->GetIndex()];

				switch (Globals::missedshots % 5)
				{
				case 0: pEntity->GetEyeAnglesXY()->y = moving_lby[pEntity->GetIndex()];
				case 1: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] + 25;
				case 2: pEntity->GetEyeAnglesXY()->y = moving_lby[pEntity->GetIndex()];
				case 3: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] - 25;
				case 4: pEntity->GetEyeAnglesXY()->y = moving_lby[pEntity->GetIndex()];
				}
			}
			else
			{
				switch (Globals::missedshots % 5)
				{
				case 0: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
				case 1: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 40;
				case 2: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 180;
				case 3: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 40;
				case 4: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()];
				}
			}
		}
		else
		{
			if (pEntity->GetEyeAnglesXY()->x > 80)
			{
				switch (Globals::missedshots % 6)
				{
				case 0: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
				case 1: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()];
				case 2: pEntity->GetEyeAnglesXY()->y = last_moving_lby[pEntity->GetIndex()] - 45;
				case 3: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 90;
				case 4: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 90;
				case 5: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 180;
				}
			}
			else
			{
				pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
			}
		}


	}
	else if (Options::Menu.VisualsTab.fakemedia.GetState())
	{
		pEntity->GetEyeAnglesXY()->x = 88;
		pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 180;

	}


}





void calculate_angle(Vector src, Vector dst, Vector &angles)
{
	Vector delta = src - dst;
	vec_t hyp = delta.Length2D();
	angles.y = (atan(delta.y / delta.x) * 57.295779513082f);
	angles.x = (atan(delta.z / hyp) * 57.295779513082f);
	angles[2] = 0.0f;
	if (delta.x >= 0.0) angles.y += 180.0f;
}


void ResolverSetup::OverrideResolver(IClientEntity* pEntity)
{

	/* IClientEntity* pLocal = hackManager.pLocal();
	Vector pos_enemy;
	Vector local_target_angle;
	Vector* m_angles = pEntity->GetEyeAnglesXY();
	if (GetAsyncKeyState(Options::Menu.RageBotTab.OverrideKey.GetKey()))
	{

	if (!pEntity->IsMoving())
	{
	if (Interfaces::Engine->WorldToScreenMatrix())
	{
	calculate_angle(pLocal->GetOrigin(), pEntity->GetOrigin(), local_target_angle);

	POINT mouse = GUI.GetMouse();
	float delta = mouse.x - pos_enemy.x;

	if (delta < 1) m_angles->y = local_target_angle.y + 90;
	else  m_angles->y = local_target_angle.y - 90;

	}
	}
	} */

}


void ResolverSetup::StoreFGE(IClientEntity* pEntity)
{
	ResolverSetup::storedanglesFGE = pEntity->GetEyeAnglesXY()->y;
	ResolverSetup::storedlbyFGE = pEntity->GetLowerBodyYaw();
	ResolverSetup::storedsimtimeFGE = pEntity->GetSimulationTime();
	ResolverSetup::badangle[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResolverSetup::movinglbyFGE = pEntity->GetLowerBodyYaw();
}

void ResolverSetup::StoreThings(IClientEntity* pEntity)
{
	ResolverSetup::storedhealth[pEntity->GetIndex()] = pEntity->GetHealth();
	ResolverSetup::StoredAngles[pEntity->GetIndex()] = *pEntity->GetEyeAnglesXY();
	ResolverSetup::storedlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResolverSetup::storedsimtime = pEntity->GetSimulationTime();
	ResolverSetup::storeddelta[pEntity->GetIndex()] = pEntity->GetEyeAnglesXY()->y;
	ResolverSetup::storedlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResolverSetup::storedhp[pEntity->GetIndex()] = pEntity->GetHealth();
}

void ResolverSetup::CM(IClientEntity* pEntity)
{
	for (int x = 1; x < Interfaces::Engine->GetMaxClients(); x++)
	{

		pEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(x);

		if (!pEntity
			|| pEntity == hackManager.pLocal()
			|| pEntity->IsDormant()
			|| !pEntity->IsAlive())
			continue;

		ResolverSetup::StoreThings(pEntity);
	}
}

void ResolverSetup::FSN(IClientEntity* pEntity, ClientFrameStage_t stage)
{
	if (stage == ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		for (int i = 1; i < Interfaces::Engine->GetMaxClients(); i++)
		{

			pEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(i);

			if (!pEntity || pEntity == hackManager.pLocal() || pEntity->IsDormant() || !pEntity->IsAlive())
				continue;

			ResolverSetup::Resolve(pEntity, stage);
			ResolverSetup::preso(pEntity);
			ResolverSetup::OverrideResolver(pEntity);
		}
	}
}