#pragma once

#include "Hacks.h"

class CLegitBot : public CHack
{
public:
	void Init();
	void Draw();
	void Move(CUserCmd *pCmd, bool& bSendPacket);
private:
	// Targetting
	int GetTargetCrosshair();
	void DoTrigger(CUserCmd * pCmd, IClientEntity * pLocal, C_BaseCombatWeapon * pWeapon);
	bool TargetMeetsRequirements(IClientEntity* pEntity);
	bool TargetMeetsTriggerRequirements(IClientEntity* pEntity);
	float FovToPlayer(Vector ViewOffSet, Vector View, IClientEntity* pEntity, int HitBox);
	bool AimAtPoint(IClientEntity* pLocal, Vector point, CUserCmd *pCmd);

	void SyncWeaponSettings();

	// Functionality
	void DoAimbot(CUserCmd *pCmd);


	// Aimbot
	bool IsLocked;
	int TargetID;
	int HitBox;
	Vector AimPoint;
	bool Multihitbox;
	float StartAim;
	float Aimtime;
	int besthitbox;

	float Speed;
	float FoV;
	float RecoilControl;

	bool shoot;

};