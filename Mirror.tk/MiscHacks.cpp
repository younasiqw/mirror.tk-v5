

#include "MiscHacks.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include <chrono>
#include <algorithm>
#include <time.h>
CMiscHacks* g_Misc = new CMiscHacks;
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

inline float bitsToFloat(unsigned long i)
{
	return *reinterpret_cast<float*>(&i);
}

inline float FloatNegate(float f)
{
	return bitsToFloat(FloatBits(f) ^ 0x80000000);
}

Vector AutoStrafeView;

void CMiscHacks::Init()
{
}

void CMiscHacks::Draw()
{
	if (!Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;

	
		switch (Options::Menu.MiscTab.NameChanger.GetIndex())
		{
		case 0:
			break;
		case 1:
			Namespam();
			break;
		case 2:
			NoName();
			break;
		case 3:
			NameSteal();
			break;
		}
	
}


void CMiscHacks::Move(CUserCmd *pCmd, bool &bSendPacket)
{


	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());


	if (Options::Menu.VisualsTab.logs.GetState())
	{

		ConVar* Developer = Interfaces::CVar->FindVar("developer");
		*(float*)((DWORD)&Developer->fnChangeCallback + 0xC) = NULL;
		Developer->SetValue("1");

		ConVar* con_filter_enable = Interfaces::CVar->FindVar("con_filter_enable");
		*(float*)((DWORD)&con_filter_enable->fnChangeCallback + 0xC) = NULL;
		con_filter_enable->SetValue("2");

		ConVar* con_filter_text = Interfaces::CVar->FindVar("con_filter_text");
		*(float*)((DWORD)&con_filter_text->fnChangeCallback + 0xC) = NULL;
		con_filter_text->SetValue("Mirror.tk");

	}

	if (Options::Menu.RageBotTab.AimbotEnable.GetState())
		AutoPistol(pCmd);
	if (Options::Menu.MiscTab.OtherAutoJump.GetState() && Options::Menu.MiscTab.OtherSafeMode.GetIndex() != 2)
		AutoJump(pCmd);

	Interfaces::Engine->GetViewAngles(AutoStrafeView);
	switch (Options::Menu.MiscTab.OtherAutoStrafe.GetIndex())
	{
	case 0:
		break;
	case 1:
		LegitStrafe(pCmd);
		break;
	case 2:
		RageStrafe(pCmd);
		break;
	}

	if (Interfaces::Engine->IsInGame() && Interfaces::Engine->IsConnected() )
	{
		if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() != 2)
		{

			SlowMo(pCmd, bSendPacket); // !crash with freestanding
			FakeWalk0(pCmd, bSendPacket);
		}
	}



	if (Interfaces::Engine->IsInGame() && Interfaces::Engine->IsConnected())
	{
		if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() != 2) 
		fakeping();

		if (Options::Menu.MiscTab.left2.GetState())
		left2();
	}
}


void CMiscHacks::FakeWalk0(CUserCmd* pCmd, bool &bSendPacket) // less autistic fakewalk?
{
	IClientEntity* pLocal = hackManager.pLocal();
	int key1 = Options::Menu.MiscTab.fw.GetKey();
	if (key1 >0 && GetAsyncKeyState(key1))
	{
		static int iChoked = -1;
		iChoked++;

		if (iChoked < 3)
		{
			bSendPacket = false;
			pCmd->tick_count += 10;
			pCmd += 7 + pCmd->tick_count % 2 ? 0 : 1;
			pCmd->buttons |= pLocal->GetMoveType() == IN_BACK;
			pCmd->forwardmove = pCmd->sidemove = 0.f;
		}
		else
		{
			bSendPacket = true;
			iChoked = -1;
			Interfaces::Globals->frametime *= (pLocal->GetVelocity().Length2D()) / 1.5;
			pCmd->buttons |= pLocal->GetMoveType() == IN_FORWARD;
		}

	}
}

static __declspec(naked) void __cdecl Invoke_NET_SetConVar(void* pfn, const char* cvar, const char* value)
{
	__asm 
	{
		push    ebp
			mov     ebp, esp
			and     esp, 0FFFFFFF8h
			sub     esp, 44h
			push    ebx
			push    esi
			push    edi
			mov     edi, cvar
			mov     esi, value
			jmp     pfn
	}
}

void DECLSPEC_NOINLINE NET_SetConVar(const char* value, const char* cvar)
{
	static DWORD setaddr = Utilities::Memory::FindPattern("engine.dll", (PBYTE)"\x8D\x4C\x24\x1C\xE8\x00\x00\x00\x00\x56", "xxxxx????x");
	if (setaddr != 0) 
	{
		void* pvSetConVar = (char*)setaddr;
		Invoke_NET_SetConVar(pvSetConVar, cvar, value);
	}
}

void change_name(const char* name)
{
	if (Interfaces::Engine->IsInGame() && Interfaces::Engine->IsConnected())
		NET_SetConVar(name, "name");
}
void CMiscHacks::AutoPistol(CUserCmd* pCmd)
{
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
	static bool WasFiring = false;
	if (GameUtils::IsPistol)
	{
		if (pCmd->buttons & IN_ATTACK)
		{
			if (WasFiring)
			{
				pCmd->buttons &= ~IN_ATTACK;
			}
		}
		WasFiring = pCmd->buttons & IN_ATTACK ? true : false;
	}
	else
		return;
}
void CMiscHacks::SlowMo(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();
	if (GetAsyncKeyState(Options::Menu.MiscTab.FakeWalk.GetKey()))
	{

		static int iChoked = -1;
		iChoked++;

		static bool slowmo;
		slowmo = !slowmo;


		const auto lag = 90;

	
		if (iChoked < lag)
		{
			
			bSendPacket = false;
			if (slowmo)
			{
				pCmd->tick_count = INT_MAX;
				pCmd->command_number += INT_MAX + pCmd->tick_count % 2 ? 1 : 0;
				pCmd->buttons |= pLocal->GetMoveType() == IN_FORWARD;
				pCmd->buttons |= pLocal->GetMoveType() == IN_LEFT;
				pCmd->buttons |= pLocal->GetMoveType() == IN_BACK;
				pCmd->forwardmove = pCmd->sidemove = 0.f;	
			}
			else
			{
				bSendPacket = true;
				iChoked = -1;
				Interfaces::Globals->frametime *= (pLocal->GetVelocity().Length2D()) / 1.2;
				pCmd->buttons |= pLocal->GetMoveType() == IN_FORWARD;
			}
		
		}
		else
		{
			if (!bSendPacket)
			{

				if (slowmo)
				{
					pCmd->tick_count = INT_MAX;
					pCmd->command_number += INT_MAX + pCmd->tick_count % 2 ? 1 : 0;
					pCmd->buttons |= pLocal->GetMoveType() == IN_FORWARD;
					pCmd->buttons |= pLocal->GetMoveType() == IN_LEFT;
					pCmd->buttons |= pLocal->GetMoveType() == IN_BACK;
					pCmd->forwardmove = pCmd->sidemove = 0.f;
				}

			}
			else
			{

				if (slowmo)
				{
					bSendPacket = true;
					iChoked = -1;
					pCmd->tick_count = INT_MAX;
					Interfaces::Globals->frametime *= (pLocal->GetVelocity().Length2D()) / 1.25;
					pCmd->buttons |= pLocal->GetMoveType() == IN_FORWARD;
				}

			}
		}
		
	}
}

		

	


void CMiscHacks::Namespam()
{
	static clock_t start_t = clock();
	double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
	if (timeSoFar < 0.001)
		return;

	static bool wasSpamming = true;

	if (wasSpamming)
	{
		static bool useSpace = true;
		if (useSpace)
		{
			change_name("™Mirror");
			useSpace = !useSpace;
		}
		else
		{
			change_name("™ Mirror");
			useSpace = !useSpace;
		}
	}

	start_t = clock();
}

void CMiscHacks::NoName()
{
	change_name("\n­­­");
}

void CMiscHacks::NameSteal()
{
	static clock_t start_t = clock();
	double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
	if (timeSoFar < 0.001)
		return;

	std::vector < std::string > Names;

	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{

		IClientEntity *entity = Interfaces::EntList->GetClientEntity(i);

		player_info_t pInfo;

		if (entity && hackManager.pLocal()->GetTeamNum() == entity->GetTeamNum() && entity != hackManager.pLocal())
		{
			ClientClass* cClass = (ClientClass*)entity->GetClientClass();

			if (cClass->m_ClassID == (int)CSGOClassID::CCSPlayer)
			{
				if (Interfaces::Engine->GetPlayerInfo(i, &pInfo))
				{
					if (!strstr(pInfo.name, "GOTV"))
						Names.push_back(pInfo.name);
				}
			}
		}
	}

	static bool wasSpamming = true;

	int randomIndex = rand() % Names.size();
	char buffer[128];
	sprintf_s(buffer, "%s ", Names[randomIndex].c_str());

	if (wasSpamming)
	{
		change_name(buffer);
	}
	else
	{
		change_name("p$i 1337");
	}

	start_t = clock();
}

void CMiscHacks::AutoJump(CUserCmd *pCmd)
{
	if (pCmd->buttons & IN_JUMP && GUI.GetKeyState(VK_SPACE))
	{
		int iFlags = hackManager.pLocal()->GetFlags();
		if (!(iFlags & FL_ONGROUND))
			pCmd->buttons &= ~IN_JUMP;

		if (hackManager.pLocal()->GetVelocity().Length() <= 50)
		{
			pCmd->forwardmove = 450.f;
		}
	}
}

void CMiscHacks::LegitStrafe(CUserCmd *pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();
	if (!(pLocal->GetFlags() & FL_ONGROUND))
	{
		pCmd->forwardmove = 0.0f;

		if (pCmd->mousedx < 0)
		{
			pCmd->sidemove = -450.0f;
		}
		else if (pCmd->mousedx > 0)
		{
			pCmd->sidemove = 450.0f;
		}
	}
}
template<class T, class U>
inline T clampangle(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}

void CMiscHacks::RageStrafe(CUserCmd *userCMD) // pasted from kalkware xd
{

	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	static bool bDirection = true;

	static float move = 450.f;
	float s_move = move * 0.5065f;
	static float strafe = userCMD->viewangles.y;
	float rt = userCMD->viewangles.y, rotation;
	if (!(pLocal->GetFlags() & FL_ONGROUND))
	{
		if (userCMD->mousedx >= 1 || userCMD->mousedx <= -1) {
			userCMD->sidemove = clampangle(userCMD->mousedx < 0.f ? -400.f : 400.f, -400, 400);
		}
		else {
			if (pLocal->GetVelocity().Length2D() == 0 || pLocal->GetVelocity().Length2D() == NAN || pLocal->GetVelocity().Length2D() == INFINITE)
			{
				userCMD->forwardmove = 400;
				return;
			}
			userCMD->forwardmove = clampangle(5850.f / pLocal->GetVelocity().Length2D(), -400, 400);
			if (userCMD->forwardmove < -400 || userCMD->forwardmove > 400)
				userCMD->forwardmove = 0;
			userCMD->sidemove = clampangle((userCMD->command_number % 2) == 0 ? -400.f : 400.f, -400, 400);
			if (userCMD->sidemove < -400 || userCMD->sidemove > 400)
				userCMD->sidemove = 0;

		}
	}
}

Vector GetAutostrafeView()
{
	return AutoStrafeView;
}


void CMiscHacks::left2()
{
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	if (!GameUtils::IsBallisticWeapon(pWeapon))
		Interfaces::Engine->ExecuteClientCmd("cl_righthand 0");
	else
		Interfaces::Engine->ExecuteClientCmd("cl_righthand 1");
	

}

void CMiscHacks::fakeping()
{

	int value1337 = 150;
	ConVar* net_fakelag = Interfaces::CVar->FindVar("net_fakelag");
	static SpoofedConvar* fakelagspoof = new SpoofedConvar(net_fakelag);
	if (GetAsyncKeyState(Options::Menu.MiscTab.FakePingValue.GetKey()))
	{

		fakelagspoof->SetInt(value1337); //value

	}
	else
	{
		fakelagspoof->SetInt(0);
	}
}

