#pragma once

#include "Hacks.h"

Vector GetAutostrafeView();

class CMiscHacks : public CHack
{
public:
	void Init();
	void Draw();
	void Move(CUserCmd *pCmd, bool &bSendPacket);
	void FakeWalk0(CUserCmd * pCmd, bool & bSendPacket);
	bool FakeLag();
private:
	void walkbot(CUserCmd * pCmd, IClientEntity * pLocal);
	void AutoPistol(CUserCmd * pCmd);
	void PostProcces();
	void SvCheats();
	void SetClanTag(const char * tag, const char * name);
	void FakeWalk(CUserCmd * pCmd, bool & bSendPacket);

	void SlowMo(CUserCmd * pCmd, bool & bSendPacket);
	void Namespam();
	void NoName();
	void NameSteal();
	void RotateMovement(CUserCmd * pCmd, float rotation);
	bool CircularStrafe(CUserCmd * pCmd, Vector & OriginalView);
	void AutoJump(CUserCmd *pCmd);
	void LegitStrafe(CUserCmd *pCmd);
	void RageStrafe(CUserCmd *pCmd);
	void left2();
	void fakeping();
	int CircleFactor = 0;
	void Fakelag(CUserCmd *pCmd, bool &bSendPacket);
	void ChatSpam();
}; extern CMiscHacks* g_Misc;



