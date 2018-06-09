#pragma once

#include "Hacks.h"




struct AntiaimData_t
{
	AntiaimData_t(const float& dist, const bool& inair, int player)
	{
		this->flDist = dist;
		this->bInAir = inair;
		this->iPlayer = player;
	}

	float flDist;
	bool bInAir;
	int	iPlayer;
};

class CRageBot : public CHack
{
public:
	void Init();
	void Draw();

	void Fakelag(CUserCmd * pCmd, bool & bSendPacket);

	void linear_extp();

	void Move(CUserCmd *pCmd, bool &bSendPacket);

private:
	std::vector<AntiaimData_t> Entities;
	int GetTargetCrosshair();
	int GetTargetDistance();
	int GetTargetNextShot();
	int GetTargetThreat(CUserCmd * pCmd);
	int GetTargetHealth();
	bool TargetMeetsRequirements(IClientEntity* pEntity);
	void aimAtPlayer(CUserCmd * pCmd);
	bool AimAtPoint(IClientEntity * pLocal, Vector point, CUserCmd * pCmd, bool & bSendPacket);

	void Freestanding(CUserCmd * cmd);

	
	float FovToPlayer(Vector ViewOffSet, Vector View, IClientEntity* pEntity, int HitBox);
	int HitScan(IClientEntity* pEntity);




	void DoAimbot(CUserCmd *pCmd, bool &bSendPacket);
	void DoNoRecoil(CUserCmd *pCmd);

	void DoPitch(CUserCmd * pCmd);

	void fw(CUserCmd * pCmd, bool & bSendPacket);

	void DoAntiAim(CUserCmd *pCmd, bool&bSendPacket);


	bool IsAimStepping;
	Vector LastAimstepAngle;
	Vector LastAngle;
	IClientEntity * pTarget;
	bool IsLocked;
	int TargetID;
	int HitBox;
	Vector AimPoint;
};

extern CRageBot ragebot;

