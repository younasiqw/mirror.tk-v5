#include "RageBot.h"
#include "RenderManager.h"
#include "Resolver.h"
#include "Autowall.h"
#include <iostream>
#include "UTIL Functions.h"
#include "xostr.h"
#include "Hooks.h"
static bool dir = false;
static bool back = false;
static bool up = false;
static bool jitter = false;




CRageBot ragebot;

#define TICK_INTERVAL			(Interfaces::Globals->interval_per_tick)
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )

static bool panic = false;

void CRageBot::Init()
{
	IsAimStepping = false;
	IsLocked = false;
	TargetID = -1;
}

void CRageBot::Draw()
{

}



float curtime_fixed(CUserCmd* ucmd) {
	auto local_player = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	static int g_tick = 0;
	static CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
		g_tick = local_player->GetTickBase();
	}
	else {


		++g_tick;
	}
	g_pLastCmd = ucmd;
	float curtime = g_tick * Interfaces::Globals->interval_per_tick;
	return curtime;
}

bool next_lby_update_func(CUserCmd* m_pcmd, const float yaw_to_break) {
	auto m_local = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (m_local) {
		static float last_attempted_yaw;
		static float old_lby;
		static float next_lby_update_time;
		const float current_time = curtime_fixed(m_pcmd);
		if (old_lby != m_local->GetLowerBodyYaw() && last_attempted_yaw != m_local->GetLowerBodyYaw()) {
			old_lby = m_local->GetLowerBodyYaw();
			if (m_local->GetVelocity().Length2D() < 1) {
				auto latency = (Interfaces::Engine->GetNetChannelInfo()->GetAvgLatency(FLOW_INCOMING) + Interfaces::Engine->GetNetChannelInfo()->GetAvgLatency(FLOW_OUTGOING));
				next_lby_update_time = current_time + 1.1f;
			}
		}

		if (m_local->GetVelocity().Length2D() < 1) {
			if ((next_lby_update_time < current_time) && m_local->GetFlags() & FL_ONGROUND) {
				last_attempted_yaw = yaw_to_break;
				next_lby_update_time = current_time + 1.1f;
				return true;
			}
		}
	}

	return false;
}



void CRageBot::Fakelag(CUserCmd *pCmd, bool &bSendPacket)
{

	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	static auto choked = 0;
	const auto max_fake_lag = Options::Menu.MiscTab.FakeLagChoke.GetValue();
	const auto max_fake_lag2 = Options::Menu.MiscTab.FakeLagChoke2.GetValue();



	if (choked < max_fake_lag && pLocal->GetVelocity().Length2D() >= 3.5 && (pLocal->GetFlags() & FL_ONGROUND))
	{
		choked++;
		bSendPacket = false;
	}
	else if (choked < max_fake_lag2 && pLocal->GetVelocity().Length2D() >= 3.5 && !(pLocal->GetFlags() & FL_ONGROUND))
	{
		choked++;
		bSendPacket = false;
	}
	else if (choked < 1 && pLocal->GetVelocity().Length2D() < 3.5 && (pLocal->GetFlags() & FL_ONGROUND))
	{
		choked++;
		bSendPacket = false;
	}
	else 
	{
		choked = 0;
		bSendPacket = true;
	}



	if (!bSendPacket &&Options::Menu.MiscTab.BreakLBYDelta.GetValue() != 0) {
		if (next_lby_update_func(pCmd, pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())) {
			pCmd->viewangles.y += Options::Menu.MiscTab.BreakLBYDelta.GetValue();
		}
	}
}



bool IsAbleToShoot(IClientEntity* pLocal)
{
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	if (!pLocal)return false;
	if (!pWeapon)return false;
	float flServerTime = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
	return (!(pWeapon->GetNextPrimaryAttack() > flServerTime));
}

float hitchance(IClientEntity* pLocal, C_BaseCombatWeapon* pWeapon)
{

	float hitchance = 101;
	if (!pWeapon) return 0;
	
	if (Options::Menu.RageBotTab.AccuracyHitchance.GetValue() > 0)
	{
		float inaccuracy = pWeapon->GetInaccuracy();
		if (inaccuracy == 0) inaccuracy = 0.0000001;
		inaccuracy = 1 / inaccuracy;
		hitchance = inaccuracy;
	}
	
	return hitchance;
}

bool CanOpenFire() 
{
	IClientEntity* pLocalEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!pLocalEntity)
		return false;

	C_BaseCombatWeapon* entwep = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocalEntity->GetActiveWeaponHandle());

	float flServerTime = (float)pLocalEntity->GetTickBase() * Interfaces::Globals->interval_per_tick;
	float flNextPrimaryAttack = entwep->GetNextPrimaryAttack();

	std::cout << flServerTime << " " << flNextPrimaryAttack << std::endl;

	return !(flNextPrimaryAttack > flServerTime);
}




template<class T, class U>
inline T clamp(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}

float GetLerpTime()
{
	int ud_rate = Interfaces::CVar->FindVar("cl_updaterate")->GetFloat();
	ConVar *min_ud_rate = Interfaces::CVar->FindVar("sv_minupdaterate");
	ConVar *max_ud_rate = Interfaces::CVar->FindVar("sv_maxupdaterate");

	if (min_ud_rate && max_ud_rate)
		ud_rate = max_ud_rate->GetFloat();

	float ratio = Interfaces::CVar->FindVar("cl_interp_ratio")->GetFloat();

	if (ratio == 0)
		ratio = 1.0f;

	float lerp = Interfaces::CVar->FindVar("cl_interp")->GetFloat();
	ConVar *c_min_ratio = Interfaces::CVar->FindVar("sv_client_min_interp_ratio");
	ConVar *c_max_ratio = Interfaces::CVar->FindVar("sv_client_max_interp_ratio");

	if (c_min_ratio && c_max_ratio && c_min_ratio->GetFloat() != 1)
		ratio = clamp(ratio, c_min_ratio->GetFloat(), c_max_ratio->GetFloat());

	return max(lerp, (ratio / ud_rate));
}

float InterpFix()
{
	static ConVar* cvar_cl_interp = Interfaces::CVar->FindVar("cl_interp");
	static ConVar* cvar_cl_updaterate = Interfaces::CVar->FindVar("cl_updaterate");
	static ConVar* cvar_sv_maxupdaterate = Interfaces::CVar->FindVar("sv_maxupdaterate");
	static ConVar* cvar_sv_minupdaterate = Interfaces::CVar->FindVar("sv_minupdaterate");
	static ConVar* cvar_cl_interp_ratio = Interfaces::CVar->FindVar("cl_interp_ratio");

	IClientEntity* pLocal = hackManager.pLocal();
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	float cl_interp = cvar_cl_interp->GetFloat();
	int cl_updaterate = cvar_cl_updaterate->GetInt();
	int sv_maxupdaterate = cvar_sv_maxupdaterate->GetInt();
	int sv_minupdaterate = cvar_sv_minupdaterate->GetInt();
	int cl_interp_ratio = cvar_cl_interp_ratio->GetInt();

	if (sv_maxupdaterate <= cl_updaterate)
		cl_updaterate = sv_maxupdaterate;

	if (sv_minupdaterate > cl_updaterate)
		cl_updaterate = sv_minupdaterate;

	float new_interp = (float)cl_interp_ratio / (float)cl_updaterate;

	if (new_interp > cl_interp)
		cl_interp = new_interp;

	return max(cl_interp, cl_interp_ratio / cl_updaterate);
}

void CRageBot::Move(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocalEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (!pLocalEntity || !Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;

	Fakelag(pCmd, bSendPacket);

	if (Options::Menu.MiscTab.AntiAimEnable.GetState())
	{
		static int ChokedPackets = 1;

		C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
		if (!pWeapon)
			return;

		if (ChokedPackets < 1 && pLocalEntity->GetLifeState() == LIFE_ALIVE && pCmd->buttons & IN_ATTACK && CanOpenFire() && GameUtils::IsBallisticWeapon(pWeapon))
		{
			bSendPacket = false;
		}
		else
		{
			if (pLocalEntity->GetLifeState() == LIFE_ALIVE)
			{
				DoAntiAim(pCmd, bSendPacket);

			}
			ChokedPackets = 1;
		}

	}

	float simtime = 0;
	float current_aim_simulationtime = simtime;




	if (Options::Menu.RageBotTab.AimbotEnable.GetState() && Options::Menu.RageBotTab.lag_pred.GetIndex() == 0)
	{

		DoAimbot(pCmd, bSendPacket);
	    DoNoRecoil(pCmd); 
		pCmd->tick_count = TIME_TO_TICKS(current_aim_simulationtime) + TIME_TO_TICKS(GetLerpTime());
    }



	if (Options::Menu.RageBotTab.AimbotAimStep.GetState())
	{
		Vector AddAngs = pCmd->viewangles - LastAngle;
		if (AddAngs.Length2D() > 25.f)
		{
			Normalize(AddAngs, AddAngs);
			AddAngs *= 25;
			pCmd->viewangles = LastAngle + AddAngs;
			GameUtils::NormaliseViewAngle(pCmd->viewangles);
		}
	}
	LastAngle = pCmd->viewangles;
}


Vector BestPoint(IClientEntity *targetPlayer, Vector &final)
{
	IClientEntity* pLocal = hackManager.pLocal();

	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	filter.pSkip = targetPlayer;
	ray.Init(final + Vector(0, 0, 10), final);
	Interfaces::Trace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	final = tr.endpos;
	return final;
}


void CRageBot::DoAimbot(CUserCmd *pCmd, bool &bSendPacket)
{
	/*IClientEntity* pTarget = nullptr;
	IClientEntity* pLocal = hackManager.pLocal();
	Vector Start = pLocal->GetViewOffset() + pLocal->GetOrigin();
	bool FindNewTarget = true;
	CSWeaponInfo* weapInfo = ((C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle()))->GetCSWpnData();
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	if (Options::Menu.RageBotTab.AimbotEnable.GetState())

	
		if (GameUtils::IsRevolver(pWeapon))
		{
			static int delay = 0;
			delay++;
			if (delay <= 15)pCmd->buttons |= IN_ATTACK;
			else delay = 0;
		}
	if (pWeapon)
	{
		if (pWeapon->GetAmmoInClip() == 0 || !GameUtils::IsBallisticWeapon(pWeapon)) return;
	}
	else return;
	if (IsLocked && TargetID >= 0 && HitBox >= 0)
	{
		pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		if (pTarget  && TargetMeetsRequirements(pTarget))
		{
			HitBox = HitScan(pTarget);
			if (HitBox >= 0)
			{
				Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset(), View;
				Interfaces::Engine->GetViewAngles(View);
				float FoV = FovToPlayer(ViewOffset, View, pTarget, HitBox);
				if (FoV < Options::Menu.RageBotTab.AimbotFov.GetValue())	FindNewTarget = false;
			}
		}
	}
	if (FindNewTarget)
	{
		TargetID = 0;
		pTarget = nullptr;
		HitBox = -1;
		switch (Options::Menu.RageBotTab.TargetSelection.GetIndex())
		{
		case 0:TargetID = GetTargetCrosshair(); break;
		case 1:TargetID = GetTargetDistance(); break;
		case 2:TargetID = GetTargetHealth(); break;
		case 3:TargetID = GetTargetThreat(pCmd); break;
		case 4:TargetID = GetTargetNextShot(); break;
		}
		if (TargetID >= 0) pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		else
		{
			pTarget = nullptr;
			HitBox = -1;
		}
	} 




	Globals::Target = pTarget;
	Globals::TargetID = TargetID;
	if (TargetID >= 0 && pTarget)
	{
		HitBox = HitScan(pTarget);

		if (!CanOpenFire()) return;

		if (Options::Menu.MiscTab.FakeLagChoke.GetValue() >= 15)return;

		if (Options::Menu.RageBotTab.AimbotKeyPress.GetState())
		{
			int Key = Options::Menu.RageBotTab.AimbotKeyBind.GetKey();
			if (Key >= 0 && !GUI.GetKeyState(Key))
			{
				TargetID = -1;
				pTarget = nullptr;
				HitBox = -1;
				return;
			}
		}
		float pointscale = Options::Menu.RageBotTab.TargetPointscale.GetValue() - 5.f; 
		Vector Point;
		Vector AimPoint = GetHitboxPosition(pTarget, HitBox) + Vector(0, 0, pointscale);
		if (Options::Menu.RageBotTab.TargetMultipoint.GetState()) Point = BestPoint(pTarget, AimPoint);
		else Point = AimPoint;

		if (GameUtils::IsScopedWeapon(pWeapon) && !pWeapon->IsScoped() && Options::Menu.RageBotTab.AccuracyAutoScope.GetState()) pCmd->buttons |= IN_ATTACK2;
		else if ((Options::Menu.RageBotTab.AccuracyHitchance.GetValue() * 1.5 <= hitchance(pLocal, pWeapon)) || Options::Menu.RageBotTab.AccuracyHitchance.GetValue() == 0 || *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() == 64)
			{
				if (AimAtPoint(pLocal, Point, pCmd, bSendPacket))
					if (Options::Menu.RageBotTab.AimbotAutoFire.GetState() && !(pCmd->buttons & IN_ATTACK))pCmd->buttons |= IN_ATTACK;
					else if (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)return;
			}
		if (IsAbleToShoot(pLocal) && pCmd->buttons & IN_ATTACK) Globals::Shots += 1;
	}
	hackManager.pLocal()
	*/

	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	bool FindNewTarget = true;
	//IsLocked = false;

	// Don't aimbot with the knife..
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	if (pWeapon != nullptr)
	{

		if (pWeapon->GetAmmoInClip() == 0 || !GameUtils::IsBallisticWeapon(pWeapon) || GameUtils::IsGrenade(pWeapon)) return;

	}
	else
		return;
	if (GameUtils::IsRevolver(pWeapon))
	{
		static int delay = 0;
		delay++;
		if (delay <= 15)pCmd->buttons |= IN_ATTACK;
		else delay = 0;
	}
	// Make sure we have a good target
	if (IsLocked && TargetID >= 0 && HitBox >= 0)
	{
		pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		if (pTarget  && TargetMeetsRequirements(pTarget))
		{
			HitBox = HitScan(pTarget);
			if (HitBox >= 0)
			{
				Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
				Vector View; Interfaces::Engine->GetViewAngles(View);
				float FoV = FovToPlayer(ViewOffset, View, pTarget, HitBox);
				if (FoV < Options::Menu.RageBotTab.AimbotFov.GetValue())
					FindNewTarget = false;
			}
		}
	}



	if (FindNewTarget)
	{
		Globals::Shots = 0;
		TargetID = 0;
		pTarget = nullptr;
		HitBox = -1;


		TargetID = GetTargetCrosshair();


	
		if (TargetID >= 0)
		{
			pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		}
	}



	if (TargetID >= 0 && pTarget)
	{
		HitBox = HitScan(pTarget);

		if (!CanOpenFire()) return;


	

	

	
	
		Vector AimPoint = GetHitboxPosition(pTarget, HitBox);
	

		if (GameUtils::IsScopedWeapon(pWeapon) && !pWeapon->IsScoped() && Options::Menu.RageBotTab.AccuracyAutoScope.GetState()) pCmd->buttons |= IN_ATTACK2;
		else if ((Options::Menu.RageBotTab.AccuracyHitchance.GetValue() * 1.5 <= hitchance(pLocal, pWeapon)) || Options::Menu.RageBotTab.AccuracyHitchance.GetValue() == 0 || *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() == 64)
		{
			if (AimAtPoint(pLocal, AimPoint, pCmd, bSendPacket))
			{
				if ((pTarget->GetFlags() & FL_ONGROUND) && pTarget->GetChokedTicks() >= 2)
				{
					if (Options::Menu.RageBotTab.AimbotAutoFire.GetState() && !(pCmd->buttons & IN_ATTACK))
					{
						AimPoint -= pTarget->GetAbsOrigin();
						AimPoint += pTarget->GetOrigin();
						pCmd->buttons |= IN_ATTACK;

					}
					else if (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)return;
				}
				else if (!(pTarget->GetFlags() & FL_ONGROUND) && pTarget->GetChokedTicks() > 2)
				{
					if (Options::Menu.RageBotTab.AimbotAutoFire.GetState() && !(pCmd->buttons & IN_ATTACK))
					{
						AimPoint -= pTarget->GetAbsOrigin();
						AimPoint += pTarget->GetOrigin();
						pCmd->buttons |= IN_ATTACK;

					}
					else if (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)return;
				}
				else
				{
					if (Options::Menu.RageBotTab.AimbotAutoFire.GetState() && !(pCmd->buttons & IN_ATTACK))
					{
						pCmd->buttons |= IN_ATTACK;


					}
					else if (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)return;
				}
			}
		}
		if (IsAbleToShoot(pLocal) && pCmd->buttons & IN_ATTACK) Globals::Shots += 1;


		static bool WasFiring = false;
		if (pWeapon != nullptr)
		{
			if (GameUtils::IsPistol(pWeapon) && *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() != 64)
			{
				if (pCmd->buttons & IN_ATTACK && GameUtils::IsBallisticWeapon(pWeapon) && !GameUtils::IsGrenade(pWeapon))
				{
					if (WasFiring)
					{
						pCmd->buttons &= ~IN_ATTACK;
					}
				}

				WasFiring = pCmd->buttons & IN_ATTACK ? true : false;
			}
		}

	}

}







bool CRageBot::TargetMeetsRequirements(IClientEntity* pEntity)
{
	auto local = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	// Is a valid player
	if (pEntity && pEntity->IsDormant() == false && pEntity->IsAlive() && pEntity->GetIndex() != local->GetIndex())
	{
		// Entity Type checks
		ClientClass *pClientClass = pEntity->GetClientClass();
		player_info_t pinfo;
		if (pClientClass->m_ClassID == (int)CSGOClassID::CCSPlayer && Interfaces::Engine->GetPlayerInfo(pEntity->GetIndex(), &pinfo))
		{
			// Team Check
			if (pEntity->GetTeamNum() != local->GetTeamNum() )
			{
				// Spawn Check
				if (!pEntity->HasGunGameImmunity())
				{
					return true;
				}
			}
		}
	}

	// They must have failed a requirement
	return false;
}

float CRageBot::FovToPlayer(Vector ViewOffSet, Vector View, IClientEntity* pEntity, int aHitBox)
{
	// Anything past 180 degrees is just going to wrap around
	CONST FLOAT MaxDegrees = 180.0f;

	// Get local angles
	Vector Angles = View;

	// Get local view / eye position
	Vector Origin = ViewOffSet;

	// Create and intiialize vectors for calculations below
	Vector Delta(0, 0, 0);
	//Vector Origin(0, 0, 0);
	Vector Forward(0, 0, 0);

	// Convert angles to normalized directional forward vector
	AngleVectors(Angles, &Forward);
	Vector AimPos = GetHitboxPosition(pEntity, aHitBox); //pvs fix disabled
														 // Get delta vector between our local eye position and passed vector
	VectorSubtract(AimPos, Origin, Delta);
	//Delta = AimPos - Origin;

	// Normalize our delta vector
	Normalize(Delta, Delta);

	// Get dot product between delta position and directional forward vectors
	FLOAT DotProduct = Forward.Dot(Delta);

	// Time to calculate the field of view
	return (acos(DotProduct) * (MaxDegrees / PI));
}

float FovToPoint(Vector ViewOffSet, Vector View, Vector Point)
{
	// Get local view / eye position
	Vector Origin = ViewOffSet;

	// Create and intiialize vectors for calculations below
	Vector Delta(0, 0, 0);
	Vector Forward(0, 0, 0);

	// Convert angles to normalized directional forward vector
	AngleVectors(View, &Forward);
	Vector AimPos = Point;

	// Get delta vector between our local eye position and passed vector
	Delta = AimPos - Origin;
	//Delta = AimPos - Origin;

	// Normalize our delta vector
	Normalize(Delta, Delta);

	// Get dot product between delta position and directional forward vectors
	FLOAT DotProduct = Forward.Dot(Delta);

	// Time to calculate the field of view
	return (acos(DotProduct) * (180.f / PI));
}
int CRageBot::GetTargetCrosshair()
{
	int target = -1;
	float minFoV = Options::Menu.RageBotTab.AimbotFov.GetValue();

	IClientEntity * pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);

		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (fov < minFoV)
				{
					minFoV = fov;
					target = i;
				}
			}
		}
	}

	return target;
}

int CRageBot::GetTargetDistance()
{
	int target = -1;
	int minDist = 99999;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				Vector Difference = pLocal->GetOrigin() - pEntity->GetOrigin();
				int Distance = Difference.Length();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Distance < minDist && fov < Options::Menu.RageBotTab.AimbotFov.GetValue())
				{
					minDist = Distance;
					target = i;
				}
			}
		}
	}

	return target;
}

int CRageBot::GetTargetNextShot()
{
	int target = -1;
	int minfov = 361;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				int Health = pEntity->GetHealth();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (fov < minfov && fov < Options::Menu.RageBotTab.AimbotFov.GetValue())
				{
					minfov = fov;
					target = i;
				}
				else
					minfov = 361;
			}
		}
	}

	return target;
}

float GetFov(const QAngle& viewAngle, const QAngle& aimAngle)
{
	Vector ang, aim;

	AngleVectors(viewAngle, &aim);
	AngleVectors(aimAngle, &ang);

	return RAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
}

double inline __declspec (naked) __fastcall FASTSQRT(double n)
{
	_asm fld qword ptr[esp + 4]
		_asm fsqrt
	_asm ret 8
}

float VectorDistance(Vector v1, Vector v2)
{
	return FASTSQRT(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
}

int CRageBot::GetTargetThreat(CUserCmd* pCmd)
{
	auto iBestTarget = -1;
	float flDistance = 8192.f;

	IClientEntity* pLocal = hackManager.pLocal();

	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			auto vecHitbox = pEntity->GetBonePos(NewHitBox);
			if (NewHitBox >= 0)
			{
				Vector Difference = pLocal->GetOrigin() - pEntity->GetOrigin();
				QAngle TempTargetAbs;
				CalcAngle(pLocal->GetEyePosition(), vecHitbox, TempTargetAbs);
				float flTempFOVs = GetFov(pCmd->viewangles, TempTargetAbs);
				float flTempDistance = VectorDistance(pLocal->GetOrigin(), pEntity->GetOrigin());
				if (flTempDistance < flDistance && flTempFOVs < Options::Menu.RageBotTab.AimbotFov.GetValue())
				{
					flDistance = flTempDistance;
					iBestTarget = i;
				}
			}
		}
	}
	return iBestTarget;
}

int CRageBot::GetTargetHealth()
{
	int target = -1;
	int minHealth = 101;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				int Health = pEntity->GetHealth();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Health < minHealth && fov < Options::Menu.RageBotTab.AimbotFov.GetValue())
				{
					minHealth = Health;
					target = i;
				}
			}
		}
	}

	return target;
}
int CRageBot::HitScan(IClientEntity* pEntity)
{
	IClientEntity* pLocal = hackManager.pLocal();
	std::vector<int> HitBoxesToScan;

#pragma region GetHitboxesToScan
	int HitScanMode = Options::Menu.RageBotTab.TargetHitscan.GetIndex();
	int huso = (pEntity->GetHealth());
	int health = Options::Menu.RageBotTab.BaimIfUnderXHealth.GetValue();
	bool AWall = Options::Menu.RageBotTab.AccuracyAutoWall.GetState();
	bool Multipoint = Options::Menu.RageBotTab.TargetMultipoint.GetState();
	int TargetHitbox = Options::Menu.RageBotTab.TargetHitbox.GetIndex();
	static bool enemyHP = false;
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (huso < health)
	{
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_UPPER_ARM);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_UPPER_ARM);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_HAND);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_HAND);

		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOREARM);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOREARM);
	}
	else if (Options::Menu.RageBotTab.AWPAtBody.GetState() && GameUtils::AWP(pWeapon))
	{
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
	}
	else
	{
		if (HitScanMode == 0)
		{
			// No Hitscan, just a single hitbox
			switch (Options::Menu.RageBotTab.TargetHitbox.GetIndex())
			{
			case 0:
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
				break;
			case 1:
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
				break;
			case 2:
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
				break;
			case 3:
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
				break;
			case 4:
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_CALF);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_CALF);
				break;
			}
		}
		else
		{
			if (GameUtils::IsZeus(pWeapon))
			{

				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
			}
			else if (pEntity->GetHealth() <= Options::Menu.RageBotTab.BaimIfUnderXHealth.GetValue() && pEntity->GetHealth() != 0)
			{
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOREARM);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOREARM);
			}
			else if (GetAsyncKeyState(Options::Menu.RageBotTab.bigbaim.GetKey()))
			{
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
			}
			else if (Globals::missedshots >= Options::Menu.RageBotTab.baim.GetValue())
			{
				if (Globals::missedshots <= Options::Menu.RageBotTab.bruteX.GetValue() || Globals::missedshots >= Options::Menu.RageBotTab.bruteX.GetValue() + 4)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
				
				}
				else if (GetAsyncKeyState(Options::Menu.RageBotTab.bigbaim.GetKey()))
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
				}
				else
				{
					switch (HitScanMode)
					{
					case 1:

						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_CALF);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_CALF);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOOT);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOOT);
						break;
					case 2:
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_CALF);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_CALF);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOOT);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOOT);
						break;
					case 3:
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOOT);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOOT);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_HAND);
						HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_HAND);
						break;
					}
				}
			}
			else
			{
				switch (HitScanMode)
				{
				case 1:

					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_CALF);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_CALF);
					break;
				case 2:
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOOT);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_CALF);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_CALF);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOOT);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOOT);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);

					break;
				case 3:
				{
					if (pEntity->IsMoving())
					{
						if (pEntity->GetMoveType() & FL_DUCKING)
						{ 
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
						}
						else
						{
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOOT);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOOT);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOREARM);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOREARM);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_CALF);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_CALF);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
						}
					}
					else
					{
						if (pEntity->GetMoveType() & FL_DUCKING)
						{
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOOT);
							HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOOT);
						}
						else
						{
							if (Globals::missedshots <= Options::Menu.RageBotTab.bruteX.GetValue() || Globals::missedshots >= Options::Menu.RageBotTab.bruteX.GetValue() + 4)
							{
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOOT);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOOT);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOREARM);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOREARM);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_CALF);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_CALF);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
							}
							else
							{
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOREARM);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOREARM);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
								HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
							}
						}
					}
				
		        }

					break;
				}
			}
		}
	}
#pragma endregion Get the list of shit to scan
	for (auto HitBoxID : HitBoxesToScan)
	{
		if (AWall) 
		{
			Vector Point = GetHitboxPosition(pEntity, HitBoxID);
			float dmg = 0.f;



			if (CanHit(Point, &dmg)) 
			{
				
				if (dmg >= Options::Menu.RageBotTab.AccuracyMinimumDamage.GetValue()) {
					return HitBoxID;
				}
				
			
			}
		}
		else {
			if (GameUtils::IsVisible(hackManager.pLocal(), pEntity, HitBoxID))
				return HitBoxID;
		}
	}

	return -1;
}


void CRageBot::DoNoRecoil(CUserCmd *pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();



	if (pLocal)
	{
		Vector AimPunch = pLocal->localPlayerExclusive()->GetAimPunchAngle();
		if (AimPunch.Length2D() > 0 && AimPunch.Length2D() < 150)
		{
			if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 0 )
			{
				pCmd->viewangles -= AimPunch *2.05;
				GameUtils::NormaliseViewAngle(pCmd->viewangles);
			}
			else if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 1)
			{
				pCmd->viewangles -= AimPunch * 1.95;
				GameUtils::NormaliseViewAngle(pCmd->viewangles);
			}
			else if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 2)
			{

				if (Globals::Shots <= 2)
				{
					pCmd->viewangles -= AimPunch * (1.473);
					GameUtils::NormaliseViewAngle(pCmd->viewangles);
				}
				else if (Globals::Shots > 2 && Globals::Shots < 5)
				{
					pCmd->viewangles -= AimPunch * (1.659);
					GameUtils::NormaliseViewAngle(pCmd->viewangles);
				}
				else if (Globals::Shots = 5)
				{
					pCmd->viewangles -= AimPunch * (1.81);
					GameUtils::NormaliseViewAngle(pCmd->viewangles);
				}
				else
				{
					pCmd->viewangles -= AimPunch * 1.71;
					GameUtils::NormaliseViewAngle(pCmd->viewangles);
				}
			}
		}
	}
}


void CRageBot::aimAtPlayer(CUserCmd *pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (!pLocal || !pWeapon)
		return;

	Vector eye_position = pLocal->GetEyePosition();

	float best_dist = pWeapon->GetCSWpnData()->m_flRange;

	IClientEntity* target = nullptr;

	for (int i = 0; i < Interfaces::Engine->GetMaxClients(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			if (Globals::TargetID != -1)
				target = Interfaces::EntList->GetClientEntity(Globals::TargetID);
			else
				target = pEntity;

			Vector target_position = target->GetEyePosition();

			float temp_dist = eye_position.DistTo(target_position);

			if (best_dist > temp_dist)
			{
				best_dist = temp_dist;
				CalcAngle(eye_position, target_position, pCmd->viewangles);
			}
		}
	}
}

bool CRageBot::AimAtPoint(IClientEntity* pLocal, Vector point, CUserCmd *pCmd, bool &bSendPacket)
{
	bool ReturnValue = false;

	if (point.Length() == 0) return ReturnValue;

	Vector angles;
	Vector src = pLocal->GetOrigin() + pLocal->GetViewOffset();

	CalcAngle(src, point, angles);
	GameUtils::NormaliseViewAngle(angles);

	if (angles[0] != angles[0] || angles[1] != angles[1])
	{
		return ReturnValue;
	}

	IsLocked = true;

	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	if (!IsAimStepping)
		LastAimstepAngle = LastAngle; 

	float fovLeft = FovToPlayer(ViewOffset, LastAimstepAngle, Interfaces::EntList->GetClientEntity(TargetID), 0);
	Vector AddAngs = angles - LastAimstepAngle;
	if (fovLeft > 37.0f && Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 1)
	{
		

		if (!pLocal->IsMoving())
		{
			Normalize(AddAngs, AddAngs);
			AddAngs *= 37;
			LastAimstepAngle += AddAngs;
			GameUtils::NormaliseViewAngle(LastAimstepAngle);
			angles = LastAimstepAngle;
		}
		else
		{
			Normalize(AddAngs, AddAngs);
			AddAngs *= 39;
			LastAimstepAngle += AddAngs;
			GameUtils::NormaliseViewAngle(LastAimstepAngle);
			angles = LastAimstepAngle;
		}
	}
	else if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 2)
	{
		if (fovLeft > 3.5)
		{
		
				if (Globals::Shots > 3)
				{
					Normalize(AddAngs, AddAngs);
					AddAngs *= 2.6;
					LastAimstepAngle += AddAngs;
					GameUtils::NormaliseViewAngle(LastAimstepAngle);
					angles = LastAimstepAngle;
				}
				else
				{
					Normalize(AddAngs, AddAngs);
					AddAngs *= 2.14;
					LastAimstepAngle += AddAngs;
					GameUtils::NormaliseViewAngle(LastAimstepAngle);
					angles = LastAimstepAngle;
				}
			
		}
		else
		{
			if (pLocal->GetVelocity().Length2D() > 210)
			{
			
					Normalize(AddAngs, AddAngs);
					AddAngs *= (3.12 - (pLocal->GetHealth() / 95));
					LastAimstepAngle += AddAngs;
					GameUtils::NormaliseViewAngle(LastAimstepAngle);
					angles = LastAimstepAngle;
				
			}
			else
			{
				Normalize(AddAngs, AddAngs);
				AddAngs *= (2.9 - (pLocal->GetHealth() / 90));
				LastAimstepAngle += AddAngs;
				GameUtils::NormaliseViewAngle(LastAimstepAngle);
				angles = LastAimstepAngle;
			}
		}
	}
	else
	{
		ReturnValue = true;
	}

	if (Options::Menu.RageBotTab.AimbotSilentAim.GetState())
	{
		pCmd->viewangles = angles;
	}

	if (!Options::Menu.RageBotTab.AimbotSilentAim.GetState())
	{
		Interfaces::Engine->SetViewAngles(angles);
	}

	return ReturnValue;
}


void CorrectMovement(Vector old_angles, CUserCmd* cmd, float old_forwardmove, float old_sidemove)
{
	float delta_view, first_function, second_function;

	if (old_angles.y < 0.f) first_function = 360.0f + old_angles.y;
	else first_function = old_angles.y;
	if (cmd->viewangles.y < 0.0f) second_function = 360.0f + cmd->viewangles.y;
	else second_function = cmd->viewangles.y;

	if (second_function < first_function) delta_view = abs(second_function - first_function);
	else delta_view = 360.0f - abs(first_function - second_function);

	delta_view = 360.0f - delta_view;

	cmd->forwardmove = cos(DEG2RAD(delta_view)) * old_forwardmove + cos(DEG2RAD(delta_view + 90.f)) * old_sidemove;
	cmd->sidemove = sin(DEG2RAD(delta_view)) * old_forwardmove + sin(DEG2RAD(delta_view + 90.f)) * old_sidemove;
}

float GetLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{
		float Latency = nci->GetAvgLatency(FLOW_OUTGOING) + nci->GetAvgLatency(FLOW_INCOMING);
		return Latency;
	}
	else
	{
		return 0.0f;
	}
}
float GetOutgoingLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{
		float OutgoingLatency = nci->GetAvgLatency(FLOW_OUTGOING);
		return OutgoingLatency;
	}
	else
	{
		return 0.0f;
	}
}
float GetIncomingLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{
		float IncomingLatency = nci->GetAvgLatency(FLOW_INCOMING);
		return IncomingLatency;
	}
	else
	{
		return 0.0f;
	}
}

float OldLBY;
float LBYBreakerTimer;
float LastLBYUpdateTime;
bool bSwitch;

float CurrentVelocity(IClientEntity* LocalPlayer)
{
	int vel = LocalPlayer->GetVelocity().Length2D();
	return vel;
}
bool NextLBYUpdate()
{
	IClientEntity* LocalPlayer = hackManager.pLocal();

	float flServerTime = (float)(LocalPlayer->GetTickBase()  * Interfaces::Globals->interval_per_tick);


	if (OldLBY != LocalPlayer->GetLowerBodyYaw())
	{
		LBYBreakerTimer++;
		OldLBY = LocalPlayer->GetLowerBodyYaw();
		bSwitch = !bSwitch;
		LastLBYUpdateTime = flServerTime;
	}

	if (CurrentVelocity(LocalPlayer) > 0.5)
	{
		LastLBYUpdateTime = flServerTime;
		return false;
	}

	if ((LastLBYUpdateTime + 1 - (GetLatency() * 2) < flServerTime) && (LocalPlayer->GetFlags() & FL_ONGROUND))
	{
		if (LastLBYUpdateTime + 1.1 - (GetLatency() * 2) < flServerTime)
		{
			LastLBYUpdateTime += 1.1;
		}
		return true;
	}
	return false;
}
static bool fuckassnig = false;
bool LBYBREAKTEST()
{
	bool lby_flip = false;

	IClientEntity* LocalPlayer = hackManager.pLocal();
	if (!LocalPlayer)//null check
		return false;

	float curtime = (float)(LocalPlayer->GetTickBase()  * Interfaces::Globals->interval_per_tick);
	static float NextLBYUpdate1 = 0;

	if (NextLBYUpdate1 > curtime + 1.1)
	{
		NextLBYUpdate1 = 0;
	}
	auto key1 = Options::Menu.MiscTab.FakeWalk.GetKey();
	if (LocalPlayer->GetVelocity().Length2D() > 0.1f && !GetAsyncKeyState(key1))
	{
		NextLBYUpdate1 = curtime + 0.22 + Interfaces::Globals->interval_per_tick;
		lby_flip = false;
		return false;
	}

	if ((NextLBYUpdate1 < curtime) && (LocalPlayer->GetFlags() & FL_ONGROUND) && LocalPlayer->GetVelocity().Length2D() < 1.f)
	{
		NextLBYUpdate1 = curtime + 1.1 + Interfaces::Globals->interval_per_tick;
		lby_flip = true;
		fuckassnig = true;
		return true;
	}
	lby_flip = false;
	fuckassnig = false;
	return false;
}

float RandomFloat2(float min, float max)
{
	typedef float(*RandomFloat_t)(float, float);
	return ((RandomFloat_t)GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomFloat")) (min, max);
}
inline float RandomFloat(float min, float max)
{
	static auto fn = (decltype(&RandomFloat))(GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomFloat"));
	return fn(min, max);
}


void AngleVectors3(const QAngle &angles, Vector& forward)
{
	float	sp, sy, cp, cy;

	SinCos(DEG2RAD(angles[0]), &sp, &cp);
	SinCos(DEG2RAD(angles[1]), &sy, &cy);

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}
//--------------------------------------------------------------------------------
void AngleVectors3(const QAngle &angles, Vector& forward, Vector& right, Vector& up)
{
	float sr, sp, sy, cr, cp, cy;

    SinCos(DEG2RAD(angles[0]), &sp, &cp);
	SinCos(DEG2RAD(angles[1]), &sy, &cy);
	SinCos(DEG2RAD(angles[2]), &sr, &cr);

	forward.x = (cp * cy);
	forward.y = (cp * sy);
	forward.z = (-sp);
	right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
	right.y = (-1 * sr * sp * sy + -1 * cr *  cy);
	right.z = (-1 * sr * cp);
	up.x = (cr * sp * cy + -sr * -sy);
	up.y = (cr * sp * sy + -sr * cy);
	up.z = (cr * cp);
}

float GetBestHeadAngle(IClientEntity* player, float yaw)
{
	float Back, Right, Left;

	Vector src3D, dst3D, forward, right, up, src, dst;
	trace_t tr;
	Ray_t backray, rightray, leftray;
	CTraceFilter filter;

	QAngle angles;
	Interfaces::Engine->GetViewAngles(angles);
	angles.x = 0.f;

	AngleVectors3(angles, forward, right, up);

	filter.pSkip = player;
	src3D = player->GetEyePosition();
	dst3D = src3D + (forward * 384.f);

	backray.Init(src3D, dst3D);
	Interfaces::Trace->TraceRay(backray, MASK_SHOT, &filter, &tr);
	Back = (tr.endpos - tr.startpos).Length();

	rightray.Init(src3D + right * 35.f, dst3D + right * 35.f);
	Interfaces::Trace->TraceRay(rightray, MASK_SHOT, &filter, &tr);
	Right = (tr.endpos - tr.startpos).Length();

	leftray.Init(src3D - right * 35.f, dst3D - right * 35.f);
	Interfaces::Trace->TraceRay(leftray, MASK_SHOT, &filter, &tr);
	Left = (tr.endpos - tr.startpos).Length();

	if (Left > Right)
		return (yaw - 90.f);
	else if (Right > Left)
		return (yaw + 90.f);
	else if (Back > Right || Back > Left)
		return (yaw - 180.f);

	return 0;
}


bool is_viable_target(IClientEntity* pEntity) {
	IClientEntity* m_local = hackManager.pLocal();
	if (!pEntity) return false;
	if (pEntity->GetClientClass()->m_ClassID != (int)CSGOClassID::CCSPlayer) return false;
	if (pEntity == m_local) return false;
	if (pEntity->GetTeamNum() == m_local->GetTeamNum()) return false;
	if (pEntity->HasGunGameImmunity()) return false;
	if (!pEntity->IsAlive() || pEntity->IsDormant()) return false;
	return true;
}



static bool wilupdate;
static float LastLBYUpdateTime2 = 0;
#define RandomInt(min, max) (rand() % (max - min + 1) + min)







void DoRealAA(CUserCmd* pCmd, IClientEntity* pLocal, bool& bSendPacket)
{

	if (GetAsyncKeyState(Options::Menu.MiscTab.manualleft.GetKey())) // right
	{
		dir = true;
		back = false;
		up = false;
		bigboi::indicator = 1;
	}
	else if (GetAsyncKeyState(Options::Menu.MiscTab.manualright.GetKey())) // left
	{
		dir = false;
		back = false;
		up = false;
		bigboi::indicator = 2;
	}
	else if (GetAsyncKeyState(Options::Menu.MiscTab.manualback.GetKey()))
	{
		dir = false;
		back = true;
		up = false;
		bigboi::indicator = 3;
	}
	else if (GetAsyncKeyState(Options::Menu.MiscTab.manualfront.GetKey()))
	{
		dir = false;
		back = false;
		up = true;
		bigboi::indicator = 4;
	}

	


	float real = pCmd->viewangles.y;

	static QAngle angles;



	Vector oldAngle = pCmd->viewangles;
	float oldForward = pCmd->forwardmove;
	float oldSideMove = pCmd->sidemove;

	if ((pCmd->buttons & IN_USE) || pLocal->GetMoveType() == MOVETYPE_LADDER)
		return;


	IClientEntity* localp = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	float speed = localp->GetVelocity().Length2D();


	static int Ticks = 0;
	float temp_base = pCmd->viewangles.y;


	/*

	CSlider twitchr;
	CSlider twitchf;
	CSlider spinf;
	CSlider randlbyr;
	CSlider randlbyf;

	*/

	float staticr = Options::Menu.MiscTab.staticr.GetValue();

	float twitchr = Options::Menu.MiscTab.twitchr.GetValue();
	float lby1 = Options::Menu.MiscTab.lby1.GetValue();
	float randlbyr = Options::Menu.MiscTab.randlbyr.GetValue();




	if (!Options::Menu.MiscTab.AntiAimEnable.GetState())
		return;

	if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 2) return;

	if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 2 && Options::Menu.MiscTab.AntiAimEnable.GetState())
	{
		pCmd->viewangles.y -= 180;
	}
	
	
	if (speed <= 10 && (pLocal->GetFlags() & FL_ONGROUND))
	{
		switch (Options::Menu.MiscTab.AntiAimYaw.GetIndex())
		{
		case 1: {
			pCmd->viewangles.y -= staticr;
		}
				break;
		case 2:
		{	
			if (jitter)
				pCmd->viewangles.y -= ((170.f + twitchr) + rand() % 10);
			else
				pCmd->viewangles.y += ((170.f - twitchr) - rand() % 10);

			jitter = !jitter;
		}
		break;
		case 3:
		{
			if (jitter)
				pCmd->viewangles.y -= 110;
			else
				pCmd->viewangles.y += 110;

			jitter = !jitter;

		}break;
		case 4: {
			pCmd->viewangles.y -= Ticks;
			Ticks += 8 + rand() % 3;

			if (Ticks > 240)
				Ticks = 120;
		}
				break;
		case 5:
		{
			if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
				pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - (118 - rand() % 42); //lets break these Delta resorting trash -120 resolvers
			else
			{
				if (Ticks > 70)
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - (randlbyr - rand() % 90);
				else
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + (randlbyr + rand() % 90);
			}

		}
		break;
		case 6:
		{
			if (dir && !back && !up)
				pCmd->viewangles.y -= 90.f;
			else if (!dir && !back && !up)
				pCmd->viewangles.y += 90.f;
			else if (!dir && back && !up)
				pCmd->viewangles.y -= 180.f;
			else if (!dir && !back && up)
				pCmd->viewangles.y -= 0.f;


		}
		break;
		case 7:
		{
			if (dir && !back && !up)
			{
				if (jitter)
					pCmd->viewangles.y -= (95 - twitchr);
				else
					pCmd->viewangles.y -= (95 + twitchr);

				jitter = !jitter;
			}
			else if (!dir && !back && !up)
			{
				if (jitter)
					pCmd->viewangles.y += (95 - twitchr);
				else
					pCmd->viewangles.y += (95 + twitchr);

				jitter = !jitter;
			}
			else if (!dir && back && !up)
			{
				if (jitter)
					pCmd->viewangles.y -= (175 - twitchr);
				else
					pCmd->viewangles.y += (175 + twitchr);

				jitter = !jitter;
			}
			else if (!dir && !back && up)
			{
				if (jitter)
					pCmd->viewangles.y += (5 + twitchr);
				else
					pCmd->viewangles.y -= (5 - twitchr);

				jitter = !jitter;
			}
		}
		break;
		case 8:
		{

			if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
			{

				pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 180;
			}
			else
			{
				pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 180;
			}


		}
		break;
		case 9:
		{

			if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
			{
		
				pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 80;
			}
			else
			{				
				pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 90;
			}
			
		
		}
		break;
		case 10:
		{

			if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
			{
				if (jitter)
				pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 45 + rand() % 35;
				else
				pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 60 + rand() % 45;

				jitter = !jitter;

			}
			else
			{

				if (jitter)
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 95 + rand() % 20;
				else
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 130 - rand() % 20;

				jitter = !jitter;
			}


		}
		break;
	
		}
	
	}
	else if (speed > 10 && (pLocal->GetFlags() & FL_ONGROUND))
	{
		switch (Options::Menu.MiscTab.AntiAimYawrun.GetIndex())
		{
		case 1: {
			pCmd->viewangles.y -= 175.f;
		}	break;

		case 2:
		{
			if (speed < 30 || speed > 60)
			{
				if (jitter)
					pCmd->viewangles.y -= 170.f - twitchr;
				else
					pCmd->viewangles.y += 170.f - twitchr;

				jitter = !jitter;
			}
			else
			{
				if (jitter)
					pCmd->viewangles.y -= 90;
				else
					pCmd->viewangles.y += 90;

				jitter = !jitter;
			}
		}
		break;
		case 3:
		{
			if (jitter)
				pCmd->viewangles.y -= 110;
			else
				pCmd->viewangles.y += 110;

			jitter = !jitter;

		}	break;
		case 4: {
			pCmd->viewangles.y -= Ticks;
			Ticks += 8 + rand() % 3;

			if (Ticks > 240)
				Ticks = 120;
		}
				break;
		case 5:
		{
			if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
				pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - rand() % 30; 
			else
			{
				if (Ticks > 70)
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - (randlbyr + rand() % 120);
				else
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + (randlbyr + rand() % 120);
			}

		}
		break;
		case 6:
		{
			if (dir && !back && !up)
				pCmd->viewangles.y -= 90.f;
			else if (!dir && !back && !up)
				pCmd->viewangles.y += 90.f;
			else if (!dir && back && !up)
				pCmd->viewangles.y -= 180.f;
			else if (!dir && !back && up)
				pCmd->viewangles.y -= 0.f;


		}	break;

		case 7:
		{
			if (dir && !back && !up)
			{
				if (jitter)
					pCmd->viewangles.y -= ((85 - twitchr) - rand() % 10);
				else
					pCmd->viewangles.y -= ((95 + twitchr) + rand() % 10);

				jitter = !jitter;
			}
			else if (!dir && !back && !up)
			{
				if (jitter)
					pCmd->viewangles.y += ((85 - twitchr) - rand() % 10);
				else
					pCmd->viewangles.y += ((95 + twitchr) - rand() % 10);

				jitter = !jitter;
			}
			else if (!dir && back && !up)
			{
				if (jitter)
					pCmd->viewangles.y -= ((175 - twitchr) - rand() % 10);
				else
					pCmd->viewangles.y += ((175 - twitchr) - rand() % 10);

				jitter = !jitter;
			}
			else if (!dir && !back && up)
			{
				if (jitter)
					pCmd->viewangles.y += (5 + twitchr);
				else
					pCmd->viewangles.y -= (5 - twitchr);

				jitter = !jitter;
			}
		}
		break;
	
		}

	}
	else if (speed > 1 && !(pLocal->GetFlags() & FL_ONGROUND))
	{
		switch (Options::Menu.MiscTab.AntiAimYaw3.GetIndex())
		{
		case 1: {
			pCmd->viewangles.y -= 175.f;
		}

		case 2:
		{
			if (jitter)
				pCmd->viewangles.y -= 170.f + twitchr;
			else
				pCmd->viewangles.y += 170.f + twitchr;

			jitter = !jitter;
		}
		break;
		case 3:
		{
			if (jitter)
				pCmd->viewangles.y -= 110;
			else
				pCmd->viewangles.y += 110;

			jitter = !jitter;

		}		break;
		case 4: {
			pCmd->viewangles.y -= Ticks;
			Ticks += 8 + rand() % 3;

			if (Ticks > 240)
				Ticks = 120;
		}
				break;
		case 5:
		{
			if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
				pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - (118 - rand() % 42);
			else
			{
				if (Ticks > 70)
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - (randlbyr - rand() % 90);
				else
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + (randlbyr + rand() % 90);
			}

		}
		break;
		case 6:
		{
			if (dir && !back && !up)
				pCmd->viewangles.y -= 90.f;
			else if (!dir && !back && !up)
				pCmd->viewangles.y += 90.f;
			else if (!dir && back && !up)
				pCmd->viewangles.y -= 180.f;
			else if (!dir && !back && up)
				pCmd->viewangles.y -= 0.f;


		}
		break;
		case 7:
		{
			if (dir && !back && !up)
			{
				if (jitter)
					pCmd->viewangles.y -= (95 - twitchr);
				else
					pCmd->viewangles.y -= (95 + twitchr);

				jitter = !jitter;
			}
			else if (!dir && !back && !up)
			{
				if (jitter)
					pCmd->viewangles.y += (95 - twitchr);
				else
					pCmd->viewangles.y += (95 + twitchr);

				jitter = !jitter;
			}
			else if (!dir && back && !up)
			{
				if (jitter)
					pCmd->viewangles.y -= (175 - twitchr);
				else
					pCmd->viewangles.y += (175 + twitchr);

				jitter = !jitter;
			}
			else if (!dir && !back && up)
			{
				if (jitter)
					pCmd->viewangles.y += (5 + twitchr);
				else
					pCmd->viewangles.y -= (5 - twitchr);

				jitter = !jitter;
			}
		}
		break;
		case 8:
		{

			float lby = hackManager.pLocal()->GetLowerBodyYaw() + 90;
			

			if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
			{

				if (jitter)
					pCmd->viewangles.y = lby + 30;
				else
					pCmd->viewangles.y = lby - 30;
				jitter = !jitter;
			}
			else
			{

				if (jitter)
					pCmd->viewangles.y = lby + 35;
				else
					pCmd->viewangles.y = lby - 35;
				jitter = !jitter;
			}

		}
		break;
	
		} 


	}


	
}






void DoFakeAA(CUserCmd* pCmd, bool& bSendPacket, IClientEntity* pLocal)
{
	static QAngle angles;
	IClientEntity* localp = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	QAngle RealAngle;

	float t = rand() % 100;

	
	static int Ticks;
	static int Side;




	float staticf = Options::Menu.MiscTab.staticf.GetValue();
	float twitchf = Options::Menu.MiscTab.twitchf.GetValue();
	float spinf = Options::Menu.MiscTab.spinf.GetValue();
	float randlbyf = Options::Menu.MiscTab.randlbyf.GetValue();
	float curtime = (float)(pLocal->GetTickBase()  * Interfaces::Globals->interval_per_tick);
	if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 2) return;

	if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 2 && Options::Menu.MiscTab.AntiAimEnable.GetState())
	{
		pCmd->viewangles.y = pCmd->viewangles.y;
	}
	else
	{
		if (pLocal->GetFlags() & FL_ONGROUND)
		{
			if (localp->GetVelocity().Length2D() < 10.0f)
			{
				switch (Options::Menu.MiscTab.FakeYaw.GetIndex())
				{
				case 1:
					pCmd->viewangles.y = staticf;
					break;
				case 2:
				{
					if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
					{
						pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 90;
					}
					else
					{
						pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 135;
					}
				}
					break;
				case 3: {	
					if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
					{
						if (jitter)
							pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 90 - rand() % 5;
						else
							pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 125 + rand() % 5;

						jitter = !jitter;

					}
					else
					{
						if (jitter)
							pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 90 + rand() % 5;
						else
							pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 125 - rand() % 5;

						jitter = !jitter;
					}

				}
						break;
				case 4:
				{
				
					pCmd->viewangles.y = pCmd->viewangles.y - 180;
				}
					break;
				case 5:
				{
					
					if (jitter)
						pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - (85 + twitchf) - rand() % 35;
					else
						pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + (85 + twitchf) + rand() % 35;

					jitter = !jitter;
				

				}

				break;
				case 6:
				{
					if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
						pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - rand() % 120; 
					else
					{
						if (Ticks > 70)
							pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 35 - (randlbyf + rand() % 90);
						else
							pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 35 +(randlbyf + rand() % 90);
					}
				}
				break;
			
				case 7:
				{
					float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
					double speed = 250;
					double exe = fmod(static_cast<double>(server_time)*speed, 180);
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 90 + static_cast<float>(exe);
				} 
				break;
				case 8:
				{
					if (jitter)
						pCmd->viewangles.y = hackManager.pLocal()->GetEyeAnglesXY()->y - 150;
					else
						pCmd->viewangles.y = hackManager.pLocal()->GetEyeAnglesXY()->y + 150;

					jitter != jitter;
				}
				break;
				case 9:
				{

					if (bSendPacket)
					{
						pCmd->viewangles.y -= 90;

						if (bSendPacket)
						{
							pCmd->viewangles.y += 90;
						}
				    }
					if (bSendPacket)
						pCmd->viewangles.y -= 180;
				}
				break;

				default:
					break;
				}
			}
			else
			{
				switch (Options::Menu.MiscTab.FakeYaw2.GetIndex())
				{
				case 1:
					pCmd->viewangles.y -= 180.f;
					break;
				case 2: {	pCmd->viewangles.y = pCmd->viewangles.y;  }

						break;
				case 3:
					pCmd->viewangles.y = hackManager.pLocal()->GetEyeAnglesXY()->y - 180.f;
					break;

				case 4:
				{
					int b = rand() % 100;

					if (b > 15)
					{
						if (jitter)
							pCmd->viewangles.y = hackManager.pLocal()->GetEyeAnglesXY()->y + 175 - twitchf;
						else
							pCmd->viewangles.y = hackManager.pLocal()->GetEyeAnglesXY()->y - 175 - twitchf;
					}
					else
					{
						if (jitter)
							pCmd->viewangles.y = hackManager.pLocal()->GetEyeAnglesXY()->y + 90 + rand() % 60;
						else
							pCmd->viewangles.y = hackManager.pLocal()->GetEyeAnglesXY()->y - 90 + rand() % 60;
					}
				}	break;
				case 5:
				{
	

					if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
						pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - rand() % 45;
					else
					{
						if (Ticks > 70)
							pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - randlbyf - rand() % 90;
						else
							pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + randlbyf + rand() % 90;
					}
				}
				break;
				case 6:
				{
					
						if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
						{
							pCmd->viewangles.y = hackManager.pLocal()->GetEyeAnglesXY()->y + 115.00 + rand() % 10;
						}
						else
						{
							pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 115.00 - rand() % 10;
						}
					
				}
				break;
				
				case 7:
				{
					float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
					double speed = 250/*insert spin speed*/;
					double exe = fmod(static_cast<double>(server_time)*speed, 180/*spin angle*/);
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 90 + static_cast<float>(exe);
				}
			
				case 8:
				{
					if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
					{

						pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 40;
					}
					else
					{
						pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 60;
					}
				}
				break;
				case 9:
				{

					if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
					{
						if (jitter)
							pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 90;
						else
							pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 60;

						jitter = !jitter;

					}
					else
					{
						if (jitter)
							pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 90;
						else
							pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 120;

						jitter = !jitter;
					}
				}
				break;


				default:
					break;
				}
			}
		}
		else
		{
			switch (Options::Menu.MiscTab.FakeYaw3.GetIndex())
			{
			case 1:
				pCmd->viewangles.y = hackManager.pLocal()->GetEyeAnglesXY()->y - 175.f;

			case 2: {	pCmd->viewangles.y = pCmd->viewangles.y;  }

			case 3:
			{
				int idfk = rand() % 100;


				if (idfk > 20)
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 120 + rand() % 40;
				else if (idfk < 20)
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 120 + rand() % 40;
				else
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 30 + rand() % 90;

			}
			case 4:
			{
				float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
				double speed = 250/*insert spin speed*/;
				double exe = fmod(static_cast<double>(server_time)*speed, 180/*spin angle*/);
				pCmd->viewangles.y = hackManager.pLocal()->GetEyeAnglesXY()->y + 90 + static_cast<float>(exe);
			}




			default:
				break;
			}
		}
	}

}

bool chokeed;

static bool respecc = false;


void CRageBot::DoPitch(CUserCmd * pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 0 || Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 4)
	{
		switch (Options::Menu.MiscTab.AntiAimPitch.GetIndex())
		{
		case 0:
			break;
		case 1:
			pCmd->viewangles.x = 89;
			break;
		case 2:
			pCmd->viewangles.x = -89;
			break;
		case 3:
		{
		if (jitter)
			pCmd->viewangles.x = 89;
		else
			pCmd->viewangles.x = -89;
		jitter = !jitter;
		}
		break;
		case 4:
		{
			pCmd->viewangles.x = -89 + rand() % 160;
		}
		break;
		}

	}
	
	



}


void CRageBot::DoAntiAim(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (!Interfaces::Engine->IsInGame() || !Interfaces::Engine->IsConnected())
		return;

	if ((pCmd->buttons & IN_USE) || pLocal->GetMoveType() == MOVETYPE_LADDER)
		return;

	if ((pCmd->buttons & IN_ATTACK))
		return;

	if (GameUtils::IsBomb(pWeapon))
		return;



	if (IsAimStepping || pCmd->buttons & IN_ATTACK)
		return;


	if (warmup)
		return;



	
		


	if (pWeapon)
	{

		if (!GameUtils::IsBallisticWeapon(pWeapon))
		{
			if (!CanOpenFire() || pCmd->buttons & IN_ATTACK2)
				return;
		}
	}

	
	
	

	






	
	if (Options::Menu.MiscTab.AntiAimEnable.GetState())
	{

		DoPitch(pCmd);

		if (bSendPacket)
			DoFakeAA(pCmd, bSendPacket, pLocal);
		else
			DoRealAA(pCmd, pLocal, bSendPacket);
		
		


	}
	


		
		
		
	

}

