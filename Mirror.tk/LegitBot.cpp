#include "LegitBot.h"
#include "RenderManager.h"
#include "MathFunctions.h"

#define HITGROUP_GENERIC    0
#define HITGROUP_HEAD        1
#define HITGROUP_CHEST        2
#define HITGROUP_STOMACH    3
#define HITGROUP_LEFTARM    4    
#define HITGROUP_RIGHTARM    5
#define HITGROUP_LEFTLEG    6
#define HITGROUP_RIGHTLEG    7
#define HITGROUP_GEAR        10

void CLegitBot::Init()
{
	IsLocked = false;
	TargetID = -1;
	HitBox = -1;
}

void CLegitBot::Draw()
{

}

static int CustomDelay = 0;
static int CustomBreak = 0;

void CLegitBot::Move(CUserCmd *pCmd, bool& bSendPacket)
{
	if (!Options::Menu.LegitBotTab.Active.GetState())
		return;

	IClientEntity* pLocal = hackManager.pLocal();
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	static int CustomAimTime = 0;
	static int CustomAimStart = 0;
	if (Options::Menu.LegitBotTab.AimbotEnable.GetState())
	{
		if (StartAim > 0)
		{
			if (CustomAimStart < (StartAim * 333))
			{
				CustomAimStart++;
			}
			else
			{
				if (Aimtime > 0)
				{
					if (CustomAimTime < (Aimtime * 333))
					{
						DoAimbot(pCmd);
						CustomAimTime++;
					}
					if (!GUI.GetKeyState(Options::Menu.LegitBotTab.AimbotKeyBind.GetKey()) && Options::Menu.LegitBotTab.aimbotfiremode.GetIndex() == 0)
					{
						CustomAimTime = 0;
						CustomAimStart = 0;
					}
				}
				else
				{
					DoAimbot(pCmd);
					CustomAimTime = 0;
					CustomAimStart = 0;
				}
			}

			if (!GUI.GetKeyState(Options::Menu.LegitBotTab.AimbotKeyBind.GetKey()) || Options::Menu.LegitBotTab.aimbotfiremode.GetIndex() == 1)
			{
				CustomAimStart = 0;
				CustomAimTime = 0;
			}
		}
		else
		{
			if (Aimtime > 0)
			{
				if (CustomAimTime < (Aimtime * 333))
				{
					DoAimbot(pCmd);
					CustomAimTime++;
				}
				if (!GUI.GetKeyState(Options::Menu.LegitBotTab.AimbotKeyBind.GetKey()) || Options::Menu.LegitBotTab.aimbotfiremode.GetIndex() == 1)
				{
					CustomAimTime = 0;
					CustomAimStart = 0;
				}
			}
			else
			{


				DoAimbot(pCmd);
				CustomAimTime = 0;
				CustomAimStart = 0;
			}
		}
	}
	if (Options::Menu.LegitBotTab.TriggerEnable.GetState() && Options::Menu.LegitBotTab.triggertype.GetIndex() == 0 && GetAsyncKeyState(Options::Menu.LegitBotTab.TriggerKeyBind.GetKey()))
	{
		DoTrigger(pCmd, pLocal, pWeapon);
	}
	else if (Options::Menu.LegitBotTab.TriggerEnable.GetState() && Options::Menu.LegitBotTab.triggertype.GetIndex() == 1)
		DoTrigger(pCmd, pLocal, pWeapon);


	SyncWeaponSettings();
}

void CLegitBot::SyncWeaponSettings()
{
	IClientEntity* pLocal = hackManager.pLocal();
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	if (!pWeapon)
		return;

	if (GameUtils::IsPistol(pWeapon))
	{
		Speed = Options::Menu.LegitBotTab.WeaponPistSpeed.GetValue() / 100;
		FoV = Options::Menu.LegitBotTab.WeaponPistFoV.GetValue() * 2;
		RecoilControl = Options::Menu.LegitBotTab.WeaponPistRecoil.GetValue() / 100;

		switch (Options::Menu.LegitBotTab.WeaponPistHitbox.GetIndex())
		{
		case 0:
			HitBox = ((int)CSGOHitboxID::HITBOX_HEAD);
			Multihitbox = false;
			break;
		case 1:
			HitBox = ((int)CSGOHitboxID::HITBOX_NECK);
			Multihitbox = false;
			break;
		case 2:
			HitBox = ((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
			Multihitbox = false;
			break;
		case 3:
			HitBox = ((int)CSGOHitboxID::HITBOX_BELLY);
			Multihitbox = false;
			break;
		case 4:
			Multihitbox = true;
			break;
		}

		Aimtime = Options::Menu.LegitBotTab.WeaponPistAimtime.GetValue() / 10;
		StartAim = Options::Menu.LegitBotTab.WeaoponPistStartAimtime.GetValue() / 10;
	}
	else if (GameUtils::IsSniper(pWeapon))
	{
		Speed = Options::Menu.LegitBotTab.WeaponSnipSpeed.GetValue() / 100;
		FoV = Options::Menu.LegitBotTab.WeaponSnipFoV.GetValue() * 2;
		RecoilControl = Options::Menu.LegitBotTab.WeaponSnipRecoil.GetValue() / 100;

		switch (Options::Menu.LegitBotTab.WeaponSnipHitbox.GetIndex())
		{
		case 0:
			HitBox = ((int)CSGOHitboxID::HITBOX_HEAD);
			Multihitbox = false;
			break;
		case 1:
			HitBox = ((int)CSGOHitboxID::HITBOX_NECK);
			Multihitbox = false;
			break;
		case 2:
			HitBox = ((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
			Multihitbox = false;
			break;
		case 3:
			HitBox = ((int)CSGOHitboxID::HITBOX_BELLY);
			Multihitbox = false;
			break;
		case 4:
			Multihitbox = true;
			break;
		}

		Aimtime = Options::Menu.LegitBotTab.WeaponSnipAimtime.GetValue() / 10;
		StartAim = Options::Menu.LegitBotTab.WeaoponSnipStartAimtime.GetValue() / 10;
	}
	else if (GameUtils::IsMachinegun(pWeapon))
	{
		Speed = Options::Menu.LegitBotTab.WeaponMGSpeed.GetValue() / 100;
		FoV = Options::Menu.LegitBotTab.WeaponMGFoV.GetValue() * 2;
		RecoilControl = Options::Menu.LegitBotTab.WeaponMGRecoil.GetValue() / 100;

		switch (Options::Menu.LegitBotTab.WeaponMGHitbox.GetIndex())
		{
		case 0:
			HitBox = ((int)CSGOHitboxID::HITBOX_HEAD);
			Multihitbox = false;
			break;
		case 1:
			HitBox = ((int)CSGOHitboxID::HITBOX_NECK);
			Multihitbox = false;
			break;
		case 2:
			HitBox = ((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
			Multihitbox = false;
			break;
		case 3:
			HitBox = ((int)CSGOHitboxID::HITBOX_BELLY);
			Multihitbox = false;
			break;
		case 4:
			Multihitbox = true;
			break;
		}

		Aimtime = Options::Menu.LegitBotTab.WeaponMGAimtime.GetValue() / 10;
		StartAim = Options::Menu.LegitBotTab.WeaoponMGStartAimtime.GetValue() / 10;
	}
	else if (GameUtils::IsShotgun(pWeapon))
	{
		Speed = Options::Menu.LegitBotTab.WeaponShotgunSpeed.GetValue() / 100;
		FoV = Options::Menu.LegitBotTab.WeaponShotgunFoV.GetValue() * 2;
		RecoilControl = Options::Menu.LegitBotTab.WeaponShotgunRecoil.GetValue() / 100;

		switch (Options::Menu.LegitBotTab.WeaponShotgunHitbox.GetIndex())
		{
		case 0:
			HitBox = ((int)CSGOHitboxID::HITBOX_HEAD);
			Multihitbox = false;
			break;
		case 1:
			HitBox = ((int)CSGOHitboxID::HITBOX_NECK);
			Multihitbox = false;
			break;
		case 2:
			HitBox = ((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
			Multihitbox = false;
			break;
		case 3:
			HitBox = ((int)CSGOHitboxID::HITBOX_BELLY);
			Multihitbox = false;
			break;
		case 4:
			Multihitbox = true;
			break;
		}

		Aimtime = Options::Menu.LegitBotTab.WeaponShotgunAimtime.GetValue() / 10;
		StartAim = Options::Menu.LegitBotTab.WeaoponShotgunStartAimtime.GetValue() / 10;
	}
	else if (GameUtils::IsMP(pWeapon))
	{
		Speed = Options::Menu.LegitBotTab.WeaponMpSpeed.GetValue() / 100;
		FoV = Options::Menu.LegitBotTab.WeaponMpFoV.GetValue() * 2;
		RecoilControl = Options::Menu.LegitBotTab.WeaponMpRecoil.GetValue() / 100;

		switch (Options::Menu.LegitBotTab.WeaponMpHitbox.GetIndex())
		{
		case 0:
			HitBox = ((int)CSGOHitboxID::HITBOX_HEAD);
			Multihitbox = false;
			break;
		case 1:
			HitBox = ((int)CSGOHitboxID::HITBOX_NECK);
			Multihitbox = false;
			break;
		case 2:
			HitBox = ((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
			Multihitbox = false;
			break;
		case 3:
			HitBox = ((int)CSGOHitboxID::HITBOX_BELLY);
			Multihitbox = false;
			break;
		case 4:
			Multihitbox = true;
			break;
		}


	}
	else
	{
		Speed = Options::Menu.LegitBotTab.WeaponMainSpeed.GetValue() / 100;
		FoV = Options::Menu.LegitBotTab.WeaponMainFoV.GetValue() * 2;
		RecoilControl = Options::Menu.LegitBotTab.WeaponMainRecoil.GetValue() / 100;

		switch (Options::Menu.LegitBotTab.WeaponMainHitbox.GetIndex())
		{
		case 0:
			HitBox = ((int)CSGOHitboxID::HITBOX_HEAD);
			Multihitbox = false;
			break;
		case 1:
			HitBox = ((int)CSGOHitboxID::HITBOX_NECK);
			Multihitbox = false;
			break;
		case 2:
			HitBox = ((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
			Multihitbox = false;
			break;
		case 3:
			HitBox = ((int)CSGOHitboxID::HITBOX_BELLY);
			Multihitbox = false;
			break;
		case 4:
			Multihitbox = true;
			break;
		}
		Aimtime = Options::Menu.LegitBotTab.WeaponMainAimtime.GetValue() / 10;
		StartAim = Options::Menu.LegitBotTab.WeaoponMainStartAimtime.GetValue() / 10;
	}
}

void CLegitBot::DoAimbot(CUserCmd *pCmd)
{
	IClientEntity* pTarget = nullptr;
	IClientEntity* pLocal = hackManager.pLocal();
	bool FindNewTarget = true;

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	if (pWeapon)
	{
		if (pWeapon->GetAmmoInClip() == 0 || !GameUtils::IsBallisticWeapon(pWeapon))
		{
			return;
		}
		SyncWeaponSettings();

	}
	else
		return;

	if (IsLocked && TargetID >= 0 && HitBox >= 0)
	{
		pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		if (pTarget  && TargetMeetsRequirements(pTarget))
		{
			SyncWeaponSettings();
			if (HitBox >= 0)
			{
				Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
				Vector View; Interfaces::Engine->GetViewAngles(View);
				if (pLocal->GetVelocity().Length() > 45.f);
				View += pLocal->localPlayerExclusive()->GetAimPunchAngle() * RecoilControl;
				float nFoV = FovToPlayer(ViewOffset, View, pTarget, HitBox);
				if (nFoV < FoV)
					FindNewTarget = false;
			}
		}
	}

	if (FindNewTarget)
	{
		TargetID = 0;
		pTarget = nullptr;
		HitBox = -1;

		TargetID = GetTargetCrosshair();

		if (TargetID >= 0)
		{
			pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		}
		else
		{
			pTarget = nullptr;
			HitBox = -1;
		}
	}

	SyncWeaponSettings();

	if (TargetID >= 0 && pTarget)
	{
		SyncWeaponSettings();

		if (Options::Menu.LegitBotTab.AimbotKeyPress.GetState())
		{
			int Key = Options::Menu.LegitBotTab.AimbotKeyBind.GetKey();
			if (Key >= 0 && !GUI.GetKeyState(Key))
			{
				TargetID = -1;
				pTarget = nullptr;
				HitBox = -1;
				return;
			}
		}

		Vector AimPoint;

		if (Multihitbox)
		{
			AimPoint = GetHitboxPosition(pTarget, besthitbox);
		}
		else
		{
			AimPoint = GetHitboxPosition(pTarget, HitBox);
		}

		if (AimAtPoint(pLocal, AimPoint, pCmd))
		{
			if (Options::Menu.LegitBotTab.aimbotfiremode.GetIndex() == 0 && GetAsyncKeyState(Options::Menu.LegitBotTab.AimbotKeyBind.GetKey()))
			{
				pCmd->buttons |= IN_ATTACK;
			}
			else if (Options::Menu.LegitBotTab.aimbotfiremode.GetIndex() == 1 && !(pCmd->buttons & IN_ATTACK))
			{
				pCmd->buttons |= IN_ATTACK;
			}
		}
	}
}

bool CLegitBot::TargetMeetsTriggerRequirements(IClientEntity* pEntity)
{
	if (pEntity && pEntity->IsDormant() == false && pEntity->IsAlive() && pEntity->GetIndex() != hackManager.pLocal()->GetIndex() && pEntity->GetIndex() < 65)
	{
		if (pEntity->GetTeamNum() != hackManager.pLocal()->GetTeamNum() || Options::Menu.LegitBotTab.AimbotFriendlyFire.GetState())
		{
			if (!pEntity->HasGunGameImmunity())
			{


				return true;
			}
		}
	}

	return false;
}

float hitchance2(IClientEntity* pLocal, C_BaseCombatWeapon* pWeapon)
{
	float hitchance = 101;
	if (!pWeapon) return 0;
	if (Options::Menu.LegitBotTab.TriggerHitChanceAmmount.GetValue() >= 1)
	{
		float inaccuracy = pWeapon->GetInaccuracy();
		if (inaccuracy == 0) inaccuracy = 0.00000001;
		inaccuracy = 1 / inaccuracy;
		hitchance = inaccuracy;
	}
	return hitchance;
}

void CLegitBot::DoTrigger(CUserCmd *pCmd, IClientEntity* pLocal, C_BaseCombatWeapon* pWeapon)
{
	IClientEntity* LocalPlayer = hackManager.pLocal();




	auto LocalPlayerWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(LocalPlayer->GetWeaponHandle());
	auto WeaponEntity = (IClientEntity*)LocalPlayerWeapon;

	if (LocalPlayerWeapon) {
		if (LocalPlayerWeapon->GetAmmoInClip() == 0)
			return;

		auto ClientClass = WeaponEntity->GetClientClass2();
		if (ClientClass->m_ClassID == (int)CSGOClassID::CKnife ||
			ClientClass->m_ClassID == (int)CSGOClassID::CHEGrenade ||
			ClientClass->m_ClassID == (int)CSGOClassID::CDecoyGrenade ||
			ClientClass->m_ClassID == (int)CSGOClassID::CIncendiaryGrenade ||
			ClientClass->m_ClassID == (int)CSGOClassID::CSmokeGrenade ||
			ClientClass->m_ClassID == (int)CSGOClassID::CC4) {
			return;
		}
	}
	else
		return;

	Vector ViewAngles = pCmd->viewangles;
	if (Options::Menu.LegitBotTab.TriggerRecoil.GetValue() >= 0.01)
		ViewAngles += LocalPlayer->GetAimPunch() * Options::Menu.LegitBotTab.TriggerRecoil.GetValue() / 100;

	Vector CrosshairForward;
	AngleVectors(ViewAngles, &CrosshairForward);
	CrosshairForward *= 7995.f;


	Vector TraceSource = LocalPlayer->GetEyePosition();
	Vector TraceDestination = TraceSource + CrosshairForward;

	Ray_t Ray;
	trace_t Trace;
	CTraceFilter Filter;

	Filter.pSkip = LocalPlayer;

	Ray.Init(TraceSource, TraceDestination);
	Interfaces::Trace->TraceRay(Ray, MASK_SHOT, &Filter, &Trace);

	float hpt = Options::Menu.LegitBotTab.trighp.GetValue();

	if (!Trace.m_pEnt)
		return;
	if (!Trace.m_pEnt->IsAlive())
		return;
	if (Trace.m_pEnt->GetHealth() <= 0 || Trace.m_pEnt->GetHealth() > hpt)
		return;
	if (Trace.m_pEnt->IsImmune())
		return;


	if (LocalPlayer->GetTeamNum() == Trace.m_pEnt->GetTeamNum())
		return;


	if (Options::Menu.LegitBotTab.TriggerSmokeCheck.GetState())
	{
		typedef bool(__cdecl* GoesThroughSmoke)(Vector, Vector);

		static uint32_t GoesThroughSmokeOffset = (uint32_t)Utilities::Memory::FindPatternV2("client.dll", "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");
		static GoesThroughSmoke GoesThroughSmokeFunction = (GoesThroughSmoke)GoesThroughSmokeOffset;

	

		if (GoesThroughSmokeFunction(hackManager.pLocal()->GetEyePosition(), Trace.m_pEnt->GetBonePos(6)))
			return;
	}

	if ((Options::Menu.LegitBotTab.TriggerHead.GetState() && Trace.hitgroup == HITGROUP_HEAD) ||
		(Options::Menu.LegitBotTab.TriggerChest.GetState() && Trace.hitgroup == HITGROUP_CHEST) ||
		(Options::Menu.LegitBotTab.TriggerStomach.GetState() && Trace.hitgroup == HITGROUP_STOMACH) ||
		(Options::Menu.LegitBotTab.TriggerArms.GetState() && (Trace.hitgroup == HITGROUP_LEFTARM || Trace.hitgroup == HITGROUP_RIGHTARM)) ||
		(Options::Menu.LegitBotTab.TriggerLegs.GetState() && (Trace.hitgroup == HITGROUP_LEFTLEG || Trace.hitgroup == HITGROUP_RIGHTLEG))) {
		pCmd->buttons |= IN_ATTACK;
	}

	if (Options::Menu.LegitBotTab.TriggerHitChanceAmmount.GetValue() <= hitchance2(pLocal, pWeapon))
	{
		if ((Options::Menu.LegitBotTab.TriggerHead.GetState() && (Trace.hitgroup == HITGROUP_HEAD) ||
			(Options::Menu.LegitBotTab.TriggerChest.GetState() && Trace.hitgroup == HITGROUP_CHEST) ||
			(Options::Menu.LegitBotTab.TriggerStomach.GetState() && Trace.hitgroup == HITGROUP_STOMACH) ||
			(Options::Menu.LegitBotTab.TriggerArms.GetState() && (Trace.hitgroup == HITGROUP_LEFTARM || Trace.hitgroup == HITGROUP_RIGHTARM)) ||
			(Options::Menu.LegitBotTab.TriggerLegs.GetState() && (Trace.hitgroup == HITGROUP_LEFTLEG || Trace.hitgroup == HITGROUP_RIGHTLEG))))
			pCmd->buttons |= IN_ATTACK;


	}

}


bool CLegitBot::TargetMeetsRequirements(IClientEntity* pEntity)
{
	if (pEntity && pEntity->IsDormant() == false && pEntity->IsAlive() && pEntity->GetIndex() != hackManager.pLocal()->GetIndex())
	{
		ClientClass *pClientClass = pEntity->GetClientClass();
		player_info_t pinfo;
		if (pClientClass->m_ClassID == (int)CSGOClassID::CCSPlayer && Interfaces::Engine->GetPlayerInfo(pEntity->GetIndex(), &pinfo))
		{
			if (pEntity->GetTeamNum() != hackManager.pLocal()->GetTeamNum() || Options::Menu.LegitBotTab.AimbotFriendlyFire.GetState())
			{
				if (Options::Menu.LegitBotTab.AimbotSmokeCheck.GetState()) {
					typedef bool(__cdecl* GoesThroughSmoke)(Vector, Vector);

					static uint32_t GoesThroughSmokeOffset = (uint32_t)Utilities::Memory::FindPatternV2("client.dll", "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");
					static GoesThroughSmoke GoesThroughSmokeFunction = (GoesThroughSmoke)GoesThroughSmokeOffset;

					if (GoesThroughSmokeFunction(hackManager.pLocal()->GetEyePosition(), pEntity->GetBonePos(8)))
						return false;
				}

				if (Multihitbox)
				{
					if (!pEntity->HasGunGameImmunity())
					{
						if (GameUtils::IsVisible(hackManager.pLocal(), pEntity, 0) || GameUtils::IsVisible(hackManager.pLocal(), pEntity, 8) || GameUtils::IsVisible(hackManager.pLocal(), pEntity, 4 || GameUtils::IsVisible(hackManager.pLocal(), pEntity, 1 || GameUtils::IsVisible(hackManager.pLocal(), pEntity, 2 || GameUtils::IsVisible(hackManager.pLocal(), pEntity, 3 || GameUtils::IsVisible(hackManager.pLocal(), pEntity, 5 || GameUtils::IsVisible(hackManager.pLocal(), pEntity, 7 || GameUtils::IsVisible(hackManager.pLocal(), pEntity, 9 || GameUtils::IsVisible(hackManager.pLocal(), pEntity, 10 || GameUtils::IsVisible(hackManager.pLocal(), pEntity, 6))))))))))
							return true;
						else
							return false;
					}
				}
				else
				{
					if (!pEntity->HasGunGameImmunity() && GameUtils::IsVisible(hackManager.pLocal(), pEntity, HitBox))
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

float Get3dDistance(Vector me, Vector ent)
{
	return sqrt(pow(double(ent.x - me.x), 2.0) + pow(double(ent.y - me.y), 2.0) + pow(double(ent.z - me.z), 2.0));
}

float CLegitBot::FovToPlayer(Vector ViewOffSet, Vector View, IClientEntity* pEntity, int aHitBox)
{
	CONST FLOAT MaxDegrees = 180.0f;

	Vector Angles = View;

	Vector Origin = ViewOffSet;

	Vector Delta(0, 0, 0);

	Vector Forward(0, 0, 0);

	AngleVectors(Angles, &Forward);
	Vector AimPos = GetHitboxPosition(pEntity, aHitBox);

	VectorSubtract(AimPos, Origin, Delta);

	Normalize(Delta, Delta);

	float Distance = Get3dDistance(Origin, AimPos);

	float pitch = sin(Forward.x - Delta.x) * Distance;
	float yaw = sin(Forward.y - Delta.y) * Distance;
	float zaw = sin(Forward.z - Delta.z) * Distance;

	float mag = sqrt((pitch*pitch) + (yaw*yaw) + (zaw*zaw));
	return mag;
}

int CLegitBot::GetTargetCrosshair()
{
	SyncWeaponSettings();
	int target = -1;


	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);
	View += pLocal->localPlayerExclusive()->GetAimPunchAngle() * 2.f;

	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);

		if (TargetMeetsRequirements(pEntity))
		{
			if (Multihitbox)
			{

				float fov1 = FovToPlayer(ViewOffset, View, pEntity, 0);
				float fov2 = FovToPlayer(ViewOffset, View, pEntity, 4);
				float fov3 = FovToPlayer(ViewOffset, View, pEntity, 6);

				if (fov1 < FoV || fov2 < FoV && fov1 < FoV || fov3 < FoV)
				{
					FoV = fov1;
					target = i;
					besthitbox = 0;
				}

				if (fov2 < FoV || fov1 < FoV && fov2 < FoV || fov3 < FoV)
				{
					FoV = fov2;
					target = i;
					besthitbox = 4;
				}

				if (fov3 < FoV || fov1 < FoV && fov3 < FoV || fov2 < FoV)
				{
					FoV = fov3;
					target = i;
					besthitbox = 6;
				}

			}
			else
			{
				int NewHitBox = HitBox;
				if (NewHitBox >= 0)
				{
					float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
					if (fov < FoV)
					{
						FoV = fov;
						target = i;
					}
				}
			}
		}
	}

	return target;
}

bool CLegitBot::AimAtPoint(IClientEntity* pLocal, Vector point, CUserCmd *pCmd)
{
	if (point.Length() == 0) return false;

	Vector angles;
	Vector src = pLocal->GetOrigin() + pLocal->GetViewOffset();

	CalcAngle(src, point, angles);
	GameUtils::NormaliseViewAngle(angles);

	if (angles[0] != angles[0] || angles[1] != angles[1])
	{
		return false;
	}

	if (RecoilControl > 0)
	{
		Vector AimPunch = pLocal->localPlayerExclusive()->GetAimPunchAngle();
		if (AimPunch.Length2D() > 0 && AimPunch.Length2D() < 150)
		{
			angles -= AimPunch * RecoilControl;
			GameUtils::NormaliseViewAngle(angles);
		}
	}

	IsLocked = true;

	Vector shit = angles - pCmd->viewangles;
	bool v = false;
	GameUtils::NormaliseViewAngle(shit);
	if (shit.Length() > Speed)
	{
		Normalize(shit, shit);
		shit *= Speed;
	}
	else
	{
		v = true;
	}

	pCmd->viewangles += shit;
	Interfaces::Engine->SetViewAngles(pCmd->viewangles);

	return v;
}