#include "Hacks.h"
#include "Chams.h"
#include "Menu.h"
#include "CBulletListener.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "MiscHacks.h"
#include "CRC32.h"
#include "Resolver.h"
#include "hitmarker.h"
#include <intrin.h>
#include "DamageIndicator.h"
#include "RageBot.h"
#include "GlowManager.h"
#include "LagCompensation2.h"
#include "edge.h"
#include "engine_prd.h"
#include "lin_extp.h"

static CPredictionSystem* Prediction = new CPredictionSystem();
CLagcompensation lagcompensation;
HANDLE worldmodel_handle;
C_BaseCombatWeapon* worldmodel;
#define MakePtr(cast, ptr, addValue) (cast)( (DWORD)(ptr) + (DWORD)(addValue))

#ifdef NDEBUG
#define strenc( s ) std::string( cx_make_encrypted_string( s ) )
#define charenc( s ) strenc( s ).c_str()
#define wstrenc( s ) std::wstring( strenc( s ).begin(), strenc( s ).end() )
#define wcharenc( s ) wstrenc( s ).c_str()
#else
#define strenc( s ) ( s )
#define charenc( s ) ( s )
#define wstrenc( s ) ( s )
#define wcharenc( s ) ( s )
#endif

#ifdef NDEBUG
#define XorStr( s ) ( XorCompileTime::XorString< sizeof( s ) - 1, __COUNTER__ >( s, std::make_index_sequence< sizeof( s ) - 1>() ).decrypt() )
#else
#define XorStr( s ) ( s )
#endif


int bigboi::indicator;
std::vector<trace_info> trace_logs;
int currentfov;
Vector LastAngleAA;
Vector LastAngleAAFake;
bool Resolver::didhitHS;
CUserCmd* Globals::UserCmd;
IClientEntity* Globals::Target;
int Globals::Shots;
bool Globals::change;
int Globals::TargetID;
bool Resolver::hitbaim;
bool Globals::Up2date;
int Globals::fired;
int Globals::hit;

extern float lineLBY;
extern float lineRealAngle;
extern float lineFakeAngle;
extern float last_real;
extern float lspeed;
extern float pitchmeme;
extern float lby2;
extern float inaccuracy;

Vector LastAngleAAReal;
Vector LBYThirdpersonAngle;

#define STUDIO_RENDER					0x00000001

std::map<int, QAngle>Globals::storedshit;
int Globals::missedshots;
static int missedLogHits[65];
float fakeangle;
typedef void(__thiscall* DrawModelEx_)(void*, void*, void*, const ModelRenderInfo_t&, matrix3x4*);
typedef void(__thiscall* PaintTraverse_)(PVOID, unsigned int, bool, bool);
typedef bool(__thiscall* InPrediction_)(PVOID);
typedef void(__stdcall *FrameStageNotifyFn)(ClientFrameStage_t);
typedef bool(__thiscall *FireEventClientSideFn)(PVOID, IGameEvent*);
typedef void(__thiscall* RenderViewFn)(void*, CViewSetup&, CViewSetup&, int, int);
using OverrideViewFn = void(__fastcall*)(void*, void*, CViewSetup*);
typedef float(__stdcall *oGetViewModelFOV)();
typedef void(__thiscall *SceneEnd_t)(void *pEcx);

SceneEnd_t pSceneEnd;
PaintTraverse_ oPaintTraverse;
DrawModelEx_ oDrawModelExecute;
FrameStageNotifyFn oFrameStageNotify;
OverrideViewFn oOverrideView;
FireEventClientSideFn oFireEventClientSide;
RenderViewFn oRenderView;


void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce);
bool __stdcall Hooked_InPrediction();
bool __fastcall Hooked_FireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *Event);
void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld);
bool __stdcall CreateMoveClient_Hooked(float frametime, CUserCmd* pCmd);
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage);
void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup);
float __stdcall GGetViewModelFOV();
void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw);
void __fastcall	hkSceneEnd(void *pEcx, void *pEdx);
namespace Hooks
{
	Utilities::Memory::VMTManager VMTPanel;
	Utilities::Memory::VMTManager VMTClient;
	Utilities::Memory::VMTManager VMTClientMode;
	Utilities::Memory::VMTManager VMTModelRender;
	Utilities::Memory::VMTManager VMTPrediction;
	Utilities::Memory::VMTManager VMTRenderView;
	Utilities::Memory::VMTManager VMTEventManager;
};

void Hooks::UndoHooks()
{
	VMTPanel.RestoreOriginal();
	VMTPrediction.RestoreOriginal();
	VMTModelRender.RestoreOriginal();
	VMTClientMode.RestoreOriginal();
}

void Hooks::Initialise()
{
	Interfaces::Engine->ExecuteClientCmd("clear");

	VMTPanel.Initialise((DWORD*)Interfaces::Panels);
	oPaintTraverse = (PaintTraverse_)VMTPanel.HookMethod((DWORD)&PaintTraverse_Hooked, Offsets::VMT::Panel_PaintTraverse);

	VMTPrediction.Initialise((DWORD*)Interfaces::Prediction);
	VMTPrediction.HookMethod((DWORD)&Hooked_InPrediction, 14);

	VMTModelRender.Initialise((DWORD*)Interfaces::ModelRender);
	oDrawModelExecute = (DrawModelEx_)VMTModelRender.HookMethod((DWORD)&Hooked_DrawModelExecute, Offsets::VMT::ModelRender_DrawModelExecute);

	VMTClientMode.Initialise((DWORD*)Interfaces::ClientMode);
	VMTClientMode.HookMethod((DWORD)CreateMoveClient_Hooked, 24);

	oOverrideView = (OverrideViewFn)VMTClientMode.HookMethod((DWORD)&Hooked_OverrideView, 18);
	VMTClientMode.HookMethod((DWORD)&GGetViewModelFOV, 35);

	VMTClient.Initialise((DWORD*)Interfaces::Client);
	oFrameStageNotify = (FrameStageNotifyFn)VMTClient.HookMethod((DWORD)&Hooked_FrameStageNotify, 36);

	VMTEventManager.Initialise((DWORD*)Interfaces::EventManager);
	oFireEventClientSide = (FireEventClientSideFn)VMTEventManager.HookMethod((DWORD)&Hooked_FireEventClientSide, 9);

	VMTRenderView.Initialise((DWORD*)Interfaces::RenderView);
	pSceneEnd = (SceneEnd_t)VMTRenderView.HookMethod((DWORD)&hkSceneEnd, 9);




}

void MovementCorrection(CUserCmd* pCmd)
{


}

float clip(float n, float lower, float upper)
{
	return (std::max)(lower, (std::min)(n, upper));
}



int kek = 0;
int autism = 0;

int speed = 0;
static float testtimeToTick;
static float testServerTick;
static float testTickCount64 = 1;






bool __stdcall CreateMoveClient_Hooked(float frametime, CUserCmd* pCmd)
{
	if (!pCmd->command_number)
		return true;

	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());




	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && pLocal)
	{

		static bool abc = false;
		static bool freakware = false;
		if (Options::Menu.VisualsTab.colmodupdate.GetState())
		{
			if (!freakware)
			{

				ConVar* staticdrop = Interfaces::CVar->FindVar("r_DrawSpecificStaticProp");
				SpoofedConvar* staticdrop_spoofed = new SpoofedConvar(staticdrop);
				staticdrop_spoofed->SetInt(0);


				for (MaterialHandle_t i = Interfaces::MaterialSystem->FirstMaterial(); i != Interfaces::MaterialSystem->InvalidMaterial(); i = Interfaces::MaterialSystem->NextMaterial(i))
				{
					IMaterial *pMaterial = Interfaces::MaterialSystem->GetMaterial(i);

					if (!pMaterial)
						continue;

				
				



					float test = Options::Menu.VisualsTab.asusamount.GetValue() / 100;
				
					float amountr = Options::Menu.VisualsTab.colmod.GetValue() / 100;

					if (Options::Menu.VisualsTab.colmod.GetValue() < 99)
					{


						if (Options::Menu.VisualsTab.customskies.GetIndex() == 0)
						{


							ConVar* NightSkybox1 = Interfaces::CVar->FindVar("sv_skyname");
							*(float*)((DWORD)&NightSkybox1->fnChangeCallback + 0xC) = NULL;
							NightSkybox1->SetValue("sky_csgo_night02b");

							if (!strcmp(pMaterial->GetTextureGroupName(), "Skybox"))
							{
								pMaterial->ColorModulation(0.6, 0, 0.9);
							}

							if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls	
							{
								pMaterial->ColorModulation(amountr, amountr, amountr);
							}
							if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
							{
								pMaterial->AlphaModulate(test);
								pMaterial->ColorModulation(amountr, amountr, amountr);
							}
						}
							if (Options::Menu.VisualsTab.customskies.GetIndex() == 1)
							{


								ConVar* NightSkybox1 = Interfaces::CVar->FindVar("sv_skyname");
								*(float*)((DWORD)&NightSkybox1->fnChangeCallback + 0xC) = NULL;
								NightSkybox1->SetValue("sky_csgo_night02b");

								if (!strcmp(pMaterial->GetTextureGroupName(), "Skybox"))
								{
									pMaterial->ColorModulation(0.6, 0, 0.9);
								}

								if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls	
								{
									pMaterial->ColorModulation(amountr, amountr, amountr);
								}
								if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
								{
									pMaterial->AlphaModulate(test);
									pMaterial->ColorModulation(amountr, amountr, amountr);
								}
							}
							if (Options::Menu.VisualsTab.customskies.GetIndex() == 2)
							{

								ConVar* NightSkybox1 = Interfaces::CVar->FindVar("sv_skyname");
								*(float*)((DWORD)&NightSkybox1->fnChangeCallback + 0xC) = NULL;
								NightSkybox1->SetValue("sky_csgo_night02");

								if (!strcmp(pMaterial->GetTextureGroupName(), "Skybox"))
								{
									pMaterial->ColorModulation(0.6, 0, 0.9);
								}

								if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls	
								{
									pMaterial->ColorModulation(amountr, amountr, amountr);
								}
								if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
								{
									pMaterial->AlphaModulate(test);
									pMaterial->ColorModulation(amountr, amountr, amountr);
								}


							}
							if (Options::Menu.VisualsTab.customskies.GetIndex() == 3)
							{

								ConVar* NightSkybox1 = Interfaces::CVar->FindVar("sv_skyname");
								*(float*)((DWORD)&NightSkybox1->fnChangeCallback + 0xC) = NULL;
								NightSkybox1->SetValue("sky_l4d_rural02_ldr");

								if (!strcmp(pMaterial->GetTextureGroupName(), "Skybox"))
								{
									pMaterial->ColorModulation(0.6, 0, 0.9);
								}
								if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls	
								{
									pMaterial->ColorModulation(amountr, amountr, amountr);
								}
								if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
								{
									pMaterial->AlphaModulate(test);
									pMaterial->ColorModulation(amountr, amountr, amountr);
								}

							}
					




					}
					else
					{
						if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls	
						{

							pMaterial->ColorModulation(amountr, amountr, amountr);



						}
						if (!strcmp(pMaterial->GetTextureGroupName(), "Skybox"))  
						{
							pMaterial->ColorModulation(0.6, 0, 0.9);
						}
						if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
						{
							pMaterial->AlphaModulate(test);
							pMaterial->ColorModulation(amountr, amountr, amountr);




						}

					}
				}
					
					

			}

			freakware = true; 
			Options::Menu.VisualsTab.colmodupdate.SetState(false);
		//	Options::Menu.VisualsTab.customskies2.SetState(false);
		

		}
		
		
		else
		{
			freakware = false;
		}








		PVOID pebp;
		__asm mov pebp, ebp;
		bool* pbSendPacket = (bool*)(*(DWORD*)pebp - 0x1C);
		bool& bSendPacket = *pbSendPacket;

		//  bSendPacket = ((byte*)_bp->next - 0x1C);
		Vector origView = pCmd->viewangles;
		Vector viewforward, viewright, viewup, aimforward, aimright, aimup;
		Vector qAimAngles;
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);

		IClientEntity* pEntity;
		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && pLocal && pLocal->IsAlive())
		{
			Hacks::MoveHacks(pCmd, bSendPacket);
			ResolverSetup::GetInst().CM(pEntity);
		}
		backtracking->rageBackTrack(pCmd, pLocal);
		Prediction->EnginePrediction(pCmd);



		if (Interfaces::Engine->IsInGame() && Interfaces::Engine->IsConnected())
		{

			static auto SetClanTag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>(((DWORD)(Utilities::Memory::FindPatternV2("engine.dll", "53 56 57 8B DA 8B F9 FF 15"))));
			if (Options::Menu.MiscTab.ClanTag.GetIndex() == 1)
			{
				static size_t lastTime = 0;

				if (GetTickCount() > lastTime)
				{
					static int counter = 0;
					IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

					static int motion = 0;
					int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 2.5;
					if (counter % 48 == 0)
						motion++;
					int value = ServerTime % 7;
					switch (value) { //mirror.tk
					case 0:SetClanTag("[+ Mirror.tk +]", "[Mirror.tk v5]"); break;
					case 1:SetClanTag("[+ Mirror.tk -]", "[Mirror.tk v5]"); break;
					case 2:SetClanTag("[- Mirror.tk -]", "[Mirror.tk v5]"); break;
					case 3:SetClanTag("[- Mirror.tk +]", "[Mirror.tk v5]"); break;
					case 4:SetClanTag("[+ Mirror.tk +]", "[Mirror.tk v5]"); break;
					case 5:SetClanTag("[- Mirror.tk -]", "[Mirror.tk v5]"); break;
					case 6:SetClanTag("[- Mirror.tk +]", "[Mirror.tk v5]"); break;
					}

				}
			}
			else if (Options::Menu.MiscTab.ClanTag.GetIndex() == 2)
			{
				static size_t lastTime = 0;

				if (GetTickCount() > lastTime)
				{
					static int counter = 0;
					IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

					static int motion = 0;
					int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 2.5;
					if (counter % 48 == 0)
						motion++;
					int value = ServerTime % 17;
					switch (value)
					{
					case 0:SetClanTag("Mirror.tk    ", "Mirror.tk v5"); break;
					case 1:SetClanTag(" Mirror.tk   ", "Mirror.tk v5"); break;
					case 2:SetClanTag("  Mirror.tk  ", "Mirror.tk v5"); break;
					case 3:SetClanTag("   Mirror.tk ", "Mirror.tk v5");; break;
					case 4:SetClanTag("    Mirror.tk", "Mirror.tk v5"); break;
					case 5:SetClanTag("   M irror.tk", "Mirror.tk v5"); break;
					case 6:SetClanTag("  M i rror.tk", "Mirror.tk v5"); break;
					case 7:SetClanTag(" M i r ror.tk", "Mirror.tk v5"); break;
					case 8:SetClanTag("M i r r or.tk", "Mirror.tk v5"); break;
					case 9:SetClanTag("Mi r r o r.tk", "FREAKWARE"); break;
					case 10:SetClanTag("Mir r o r .tk", "Mirror.tk v5"); break;
					case 11:SetClanTag("Mirr o r . tk", "by FreaK"); break;
					case 12:SetClanTag("Mirro r . t k", "Mirror.tk v5"); break;
					case 13:SetClanTag("Mirror . t k ", "Mirror.tk v5"); break;
					case 14:SetClanTag("Mirror. t k  ", "Mirror.tk v5"); break;
					case 15:SetClanTag("Mirror.t k   ", "Mirror.tk v5"); break;
					case 16:SetClanTag("Mirror.tk    ", "Mirror.tk v5"); break;




					}
					counter++;

				}
			}
			else if (Options::Menu.MiscTab.ClanTag.GetIndex() == 3)
			{
				static size_t lastTime = 0;

				if (GetTickCount() > lastTime)
				{
					static int counter = 0;
					IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());



					static int motion = 0;
					int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 2.5;
					if (counter % 48 == 0)
						motion++;
					int value = ServerTime % 10;
					switch (value) { //mirror.tk
					case 0:SetClanTag("Testing[0]", "Testing[9]"); break;
					case 1:SetClanTag("Testing[1]", "Testing[8]"); break;
					case 2:SetClanTag("Testing[2]", "Testing[7]"); break;
					case 3:SetClanTag("Testing[3]", "Testing[6]"); break;
					case 4:SetClanTag("Testing[4]", "Testing[5]"); break;
					case 5:SetClanTag("Testing[5]", "Testing[4]"); break;
					case 6:SetClanTag("Testing[6]", "Testing[3]"); break;
					case 7:SetClanTag("Testing[7]", "Testing[2]"); break;
					case 8:SetClanTag("Testing[8]", "Testing[1]"); break;
					case 9:SetClanTag("Testing[9]", "Testing[0]"); break;
					}

				}
			}
			else if (Options::Menu.MiscTab.ClanTag.GetIndex() == 4)
			{
				static int motion = 0;
				static int counter = 0;
				int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 2.5;
				if (counter % 48 == 0)
					motion++;
				int value = ServerTime % 2;
				switch (value)
				{
				case 0:SetClanTag("-INTERWEBZ", "Mirror.tk v5"); break;
				case 1:SetClanTag("INTERWEBZ-", "Mirror by FreaK"); break;
				}
				counter++;
			
			}
			else if (Options::Menu.MiscTab.ClanTag.GetIndex() == 5)
			{

				static int counter = 0;
				IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());



				static int motion = 0;
				int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 2.5;
				if (counter % 48 == 0)
					motion++;
				int value = ServerTime % 14;
				switch (value) { //mirror.tk
				case 0:SetClanTag("->", "Mirror v5"); break;
				case 1:SetClanTag("->C", "Mirror v5"); break;
				case 2:SetClanTag("->Ca", "Mirror v5"); break;
				case 3:SetClanTag("->Cat", "Mirror v5"); break;
				case 4:SetClanTag("->Ca", "Mirror v5"); break;
				case 5:SetClanTag("->C", "Mirror v5"); break;
				case 6:SetClanTag("->", "Mirror v5"); break;
				case 7:SetClanTag("->H", "Mirror v5"); break;
				case 8:SetClanTag("->Ha", "Mirror v5"); break;
				case 9:SetClanTag("->Hac", "Mirror v5"); break;
				case 10:SetClanTag("->Hack", "Mirror v5"); break;
				case 11:SetClanTag("->Hac", "Mirror v5"); break;
				case 12:SetClanTag("->Ha", "Mirror v5"); break;
				case 13:SetClanTag("->H", "Mirror v5"); break;

				
				}

			
			}
			else if (Options::Menu.MiscTab.ClanTag.GetIndex() == 6)
			{

				static int counter = 0;
				IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());



				static int motion = 0;
				int ServerTime = (float)Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase() * 2.5;
				if (counter % 48 == 0)
					motion++;
				int value = ServerTime % 1;
				switch (value) { //mirror.tk
				case 0:SetClanTag("Emily AA", "Mirror v5"); break;
			


				}


			}
			
		


				
		}
#pragma region Timer4LBY
		static float myOldLby;
		static float myoldTime;
		testtimeToTick = TIME_TO_TICKS(0.1);
		testServerTick = TIME_TO_TICKS(1);
		static int timerino;
		static float oneTickMinues;

		if (testServerTick == 128) {
			oneTickMinues = testServerTick / 128;
		}
		else {
			oneTickMinues = testServerTick / 64;
		}
		//IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());


#pragma endregion
		IClientEntity* LocalPlayer = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
		float flServerTime = (float)(LocalPlayer->GetTickBase()  * Interfaces::Globals->interval_per_tick);
		static float next_time = 0;
		if (!bSendPacket && LocalPlayer->IsAlive() && LocalPlayer->GetVelocity().Length2D() == 0) {

			float TickStuff = TICKS_TO_TIME(LocalPlayer->GetTickBase());
			Globals::Up2date = false;
			//flServerTime = next_time;

			if (next_time - TICKS_TO_TIME(LocalPlayer->GetTickBase()) > 1.1)
			{
				next_time = 0;
			}

			if (TickStuff > next_time + 1.1f)
			{
				next_time = TickStuff + TICKS_TO_TIME(1);
				Globals::Up2date = true;
			}
		}


		qAimAngles.Init(0.0f, GetAutostrafeView().y, 0.0f);
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &aimforward, &aimright, &aimup);
		Vector vForwardNorm;		Normalize(viewforward, vForwardNorm);
		Vector vRightNorm;			Normalize(viewright, vRightNorm);
		Vector vUpNorm;				Normalize(viewup, vUpNorm);

		float forward = pCmd->forwardmove;
		float right = pCmd->sidemove;
		float up = pCmd->upmove;
		if (forward > 450) forward = 450;
		if (right > 450) right = 450;
		if (up > 450) up = 450;
		if (forward < -450) forward = -450;
		if (right < -450) right = -450;
		if (up < -450) up = -450;
		pCmd->forwardmove = DotProduct(forward * vForwardNorm, aimforward) + DotProduct(right * vRightNorm, aimforward) + DotProduct(up * vUpNorm, aimforward);
		pCmd->sidemove = DotProduct(forward * vForwardNorm, aimright) + DotProduct(right * vRightNorm, aimright) + DotProduct(up * vUpNorm, aimright);
		pCmd->upmove = DotProduct(forward * vForwardNorm, aimup) + DotProduct(right * vRightNorm, aimup) + DotProduct(up * vUpNorm, aimup);

		Vector aim_point;
		backtracking->RunLBYBackTrack(pCmd, aim_point);
	
	

		if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 0 || Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 1 || Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 2)
		{
			GameUtils::NormaliseViewAngle(pCmd->viewangles);

			if (pCmd->viewangles.z != 0.0f)
			{
				pCmd->viewangles.z = 0.00;
			}

			if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
			{
				Utilities::Log(" Re-calculating angles");
				GameUtils::NormaliseViewAngle(pCmd->viewangles);
				Beep(750, 800);
				if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
				{
					pCmd->viewangles = origView;
					pCmd->sidemove = right;
					pCmd->forwardmove = forward;
				}
			}
		}

		if (pCmd->viewangles.x > 90)
		{
			pCmd->forwardmove = -pCmd->forwardmove;
		}

		if (pCmd->viewangles.x < -90)
		{
			pCmd->forwardmove = -pCmd->forwardmove;
		}

		if (bSendPacket == true) {
			LastAngleAA = pCmd->viewangles;
		}
		else if (bSendPacket == false) { //
			LastAngleAAReal = pCmd->viewangles;
		
		}

		if (bSendPacket == true) {
			lineFakeAngle = pCmd->viewangles.y;
			LastAngleAAFake = Vector(pLocal->GetEyeAnglesXY()->x, pLocal->GetEyeAnglesXY()->y, pLocal->GetEyeAnglesXY()->z);
			
		}
		else if (bSendPacket == false) {
			lineRealAngle = pCmd->viewangles.y;
		
		}

		lineLBY = pLocal->GetLowerBodyYaw();
	

		if (pLocal->IsAlive() && !GameUtils::IsGrenade(pWeapon) && GameUtils::IsBallisticWeapon(pWeapon))
		{
		    inaccuracy = pWeapon->GetInaccuracy() * 1000;
			lspeed = pLocal->GetVelocity().Length2D();
			pitchmeme = pCmd->viewangles.x;
		}
		else if (pLocal->IsAlive() && !GameUtils::IsGrenade(pWeapon) && !GameUtils::IsBallisticWeapon(pWeapon))
		{
			lspeed = pLocal->GetVelocity().Length2D();
			pitchmeme = pCmd->viewangles.x;
		}



	}
	return false;
}



void StartPrediction(IClientEntity* LocalPlayer, CUserCmd* pCmd) //in case you cant tell, this is pasted
{
	static bool done = false;
	if (LocalPlayer->IsAlive() && Options::Menu.RageBotTab.resolver.GetIndex() != 0 && !done)
	{
	
		Interfaces::CVar->FindVar("cl_interp")->SetValue(0.01f);
		Interfaces::CVar->FindVar("cl_cmdrate")->SetValue(66);
		Interfaces::CVar->FindVar("cl_updaterate")->SetValue(66);
		Interfaces::CVar->FindVar("cl_interp_all")->SetValue(0.0f);
		Interfaces::CVar->FindVar("cl_interp_ratio")->SetValue(1.0f);
		Interfaces::CVar->FindVar("cl_smooth")->SetValue(0.0f);
		Interfaces::CVar->FindVar("cl_smoothtime")->SetValue(0.01f);
		done = true;
	}
}


std::string GetTimeString()
{
	time_t current_time;
	struct tm *time_info;
	static char timeString[10];
	time(&current_time);
	time_info = localtime(&current_time);
	strftime(timeString, sizeof(timeString), "%X", time_info);
	return timeString;
}

void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{
	if (Options::Menu.VisualsTab.Active.GetState() && Options::Menu.VisualsTab.OtherNoScope.GetState() && strcmp("HudZoom", Interfaces::Panels->GetName(vguiPanel)) == 0)
		return;
	int w, h;
	int centerW, centerh, topH;
	Interfaces::Engine->GetScreenSize(w, h);
	centerW = w / 2;
	centerh = h / 2;
	static unsigned int FocusOverlayPanel = 0;
	static bool FoundPanel = false;
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (!FoundPanel)
	{
		PCHAR szPanelName = (PCHAR)Interfaces::Panels->GetName(vguiPanel);
		if (strstr(szPanelName, XorStr("MatSystemTopPanel")))
		{
			FocusOverlayPanel = vguiPanel;
			FoundPanel = true;
		}
	}
	else if (FocusOverlayPanel == vguiPanel)
	{
		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && pLocal->IsAlive())
		{
			CUserCmd* cmdlist = *(CUserCmd**)((DWORD)Interfaces::pInput + 0xEC);
			CUserCmd* pCmd = cmdlist;
		
			RECT scrn = Render::GetViewport();
			

			if (Options::Menu.VisualsTab.LBYIndicator.GetState())
			{

				if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && pLocal->IsAlive())
				{


					char angle[50];
					sprintf_s(angle, sizeof(angle), "%i", *pLocal->GetLowerBodyYawTarget());

					RECT TextSize = Render::GetTextSize(Render::Fonts::LBYIndicator, "LBY");


					if (pCmd->viewangles.y - *pLocal->GetLowerBodyYawTarget() >= -35 && pCmd->viewangles.y - *pLocal->GetLowerBodyYawTarget() <= 35)
					{
						Render::Text(20, scrn.bottom - 55, Color(255, 0, 30, 255), Render::Fonts::LBY, "LBY");
					}
					else
					{
						if (pCmd->viewangles.y - *pLocal->GetLowerBodyYawTarget() > 120)
						{
							Render::Text(20, scrn.bottom - 55, Color(0, 250, 80, 255), Render::Fonts::LBY, "LBY");

						}
						else
						{
							Render::Text(20, scrn.bottom - 55, Color(0, 170, 250, 255), Render::Fonts::LBY, "LBY");

						}
					}
				}
			}
			if (Options::Menu.VisualsTab.manualaa.GetState())
			{
				if (bigboi::indicator == 1)
				{
				
					Render::Text(centerW - 40, centerh - 40, Color(200, 200, 200, 250), Render::Fonts::jew, "Right");
					
				}
				else if (bigboi::indicator == 2)
				{
					Render::Text(centerW - 34, centerh - 40, Color(200, 200, 200, 250), Render::Fonts::jew, "Left");
				
				}
				else if (bigboi::indicator == 3)
				{
					
					Render::Text(centerW - 34, centerh - 40, Color(200, 200, 200, 250), Render::Fonts::jew, "Back");
					
				}
				else if (bigboi::indicator == 4)
				{
			
					Render::Text(centerW - 60, centerh - 40, Color(200, 200, 200, 200), Render::Fonts::jew, "Forward");
				}
			}
			if (Options::Menu.VisualsTab.cheatinfo.GetState())
			{

					char jew[64];
					char bufferlineRealAngle[64];
					char bufferlineFakeAngle[64];
					char bufferLineLBY[64];
					char eh[64];
					char trans[64];
					char innac[64];
					float blob = Options::Menu.MiscTab.FakeLagChoke.GetValue();
					float blob2 = Options::Menu.MiscTab.FakeLagChoke2.GetValue();
					
						if (pLocal->GetFlags() & FL_ONGROUND)
						{
							sprintf_s(jew, "Pitch:  %.1f", pitchmeme);
							Render::Text(9, (centerh - 60), Color(250, 250, 250, 255), Render::Fonts::gay, jew);


							sprintf_s(bufferlineRealAngle, "Real: %.1f", lineRealAngle);
							Render::Text(9, (centerh - 47), Color(250, 250, 250, 255), Render::Fonts::gay, bufferlineRealAngle);


							sprintf_s(bufferlineFakeAngle, "Fake: %.1f", lineFakeAngle);
							Render::Text(9, (centerh - 34), Color(250, 250, 250, 255), Render::Fonts::gay, bufferlineFakeAngle);


							sprintf_s(bufferLineLBY, "Lby: %.1f", lineLBY);
							Render::Text(9, (centerh - 21), Color(250, 250, 250, 255), Render::Fonts::gay, bufferLineLBY);


							sprintf_s(eh, "Speed:  %.1f", lspeed);
							Render::Text(9, (centerh - 8), Color(250, 250, 250, 255), Render::Fonts::gay, eh);

							Render::Text(9, (centerh + 5), Color(0, 250, 25, 255), Render::Fonts::gay, "OnGround: True");
							Render::Text(9, (centerh + 5), Color(250, 250, 250, 255), Render::Fonts::gay, "OnGround:");

							sprintf_s(trans, "FakeLag:  %.1f", blob);
							Render::Text(9, (centerh + 18), Color(250, 250, 250, 255), Render::Fonts::gay, trans);

							sprintf_s(innac, "Innacuracy:  %.1f", inaccuracy);
							Render::Text(9, (centerh + 31), Color(250, 250, 250, 255), Render::Fonts::gay, innac);
						}
						else
						{
							sprintf_s(jew, "Pitch: %.1f", pitchmeme);
							Render::Text(9, (centerh - 60), Color(250, 250, 250, 255), Render::Fonts::gay, jew);


							sprintf_s(bufferlineRealAngle, "Real: %.1f", lineRealAngle);
							Render::Text(9, (centerh - 47), Color(250, 250, 250, 255), Render::Fonts::gay, bufferlineRealAngle);


							sprintf_s(bufferlineFakeAngle, "Fake: %.1f", lineFakeAngle);
							Render::Text(9, (centerh - 34), Color(250, 250, 250, 255), Render::Fonts::gay, bufferlineFakeAngle);


							sprintf_s(bufferLineLBY, "Lby: %.1f", lineLBY);
							Render::Text(9, (centerh - 21), Color(250, 250, 250, 255), Render::Fonts::gay, bufferLineLBY);


							sprintf_s(eh, "Speed: %.1f", lspeed);
							Render::Text(9, (centerh - 8), Color(250, 250, 250, 255), Render::Fonts::gay, eh);

							Render::Text(9, (centerh + 5), Color(250, 0, 25, 255), Render::Fonts::gay, "OnGround: False");
							Render::Text(9, (centerh + 5), Color(250, 250, 250, 255), Render::Fonts::gay, "OnGround:");

							sprintf_s(trans, "FakeLag:  %.1f", blob2);
							Render::Text(9, (centerh + 18), Color(250, 250, 250, 255), Render::Fonts::gay, trans);

							sprintf_s(innac, "Innacuracy:  %.1f", inaccuracy);
							Render::Text(9, (centerh + 31), Color(250, 250, 250, 255), Render::Fonts::gay, innac);
						}
					
				
			}
		}
		
			if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
				Hacks::DrawHacks();

			Options::DoUIFrame();

			if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && Options::Menu.VisualsTab.OtherHitmarker.GetState())
				hitmarker::singleton()->on_paint();

			if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && Options::Menu.VisualsTab.DamageIndicator.GetState())
				damage_indicators.paint();
	}
	oPaintTraverse(pPanels, vguiPanel, forceRepaint, allowForce);
}

bool __stdcall Hooked_InPrediction()
{
	bool result;
	static InPrediction_ origFunc = (InPrediction_)Hooks::VMTPrediction.GetOriginalFunction(14);
	static DWORD *ecxVal = Interfaces::Prediction;
	result = origFunc(ecxVal);

	if (Options::Menu.VisualsTab.OtherNoVisualRecoil.GetState() && (DWORD)(_ReturnAddress()) == Offsets::Functions::dwCalcPlayerView)
	{
		IClientEntity* pLocalEntity = NULL;

		float* m_LocalViewAngles = NULL;

		__asm
		{
			MOV pLocalEntity, ESI
			MOV m_LocalViewAngles, EBX
		}

		Vector viewPunch = pLocalEntity->localPlayerExclusive()->GetViewPunchAngle();
		Vector aimPunch = pLocalEntity->localPlayerExclusive()->GetAimPunchAngle();

		m_LocalViewAngles[0] -= (viewPunch[0] + (aimPunch[0] * 2 * 0.4499999f));
		m_LocalViewAngles[1] -= (viewPunch[1] + (aimPunch[1] * 2 * 0.4499999f));
		m_LocalViewAngles[2] -= (viewPunch[2] + (aimPunch[2] * 2 * 0.4499999f));
		return true;
	}

	return result;
}

int Kills2 = 0;
int Kills = 0;
bool RoundInfo = false;
size_t Delay = 0;
bool flipAA;

player_info_t GetInfo(int Index) {
	player_info_t Info;
	Interfaces::Engine->GetPlayerInfo(Index, &Info);
	return Info;
}


typedef void(__cdecl* MsgFn)(const char* msg, va_list);
void Msg(const char* msg, ...)
{

	if (msg == nullptr)
		return; //If no string was passed, or it was null then don't do anything
	static MsgFn fn = (MsgFn)GetProcAddress(GetModuleHandle("tier0.dll"), "Msg"); 	char buffer[989];
	va_list list; 
	va_start(list, msg);

	vsprintf(buffer, msg, list);
	va_end(list);

	fn(buffer, list); //Calls the function, we got the address above.
}


bool warmup = false;

bool __fastcall Hooked_FireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *Event)
{
	CBulletListener::singleton()->OnStudioRender();
	IClientEntity* localplayer = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());


	


	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{
		std::string event_name = Event->GetName();
		if (event_name.find("round_prestart") != std::string::npos || event_name.find("round_end") != std::string::npos)
		{
			if (event_name.find("round_end") != std::string::npos)
			{

				warmup = false;
			}
			else
			{

				warmup = true;
			}

		}

		if (event_name.find("round_freeze_end") != std::string::npos)
		{
			warmup = false;
		}

		if (event_name.find("round_end") != std::string::npos)
		{
			warmup = true;
		}


		if (Options::Menu.VisualsTab.DamageIndicator.GetState())
		{
			if (strcmp(Event->GetName(), "player_hurt") == 0)
			{

				IClientEntity* hurt = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetPlayerForUserID(Event->GetInt("userid")));
				IClientEntity* attacker = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetPlayerForUserID(Event->GetInt("attacker")));

				if (hurt != localplayer && attacker == localplayer)
				{
					DamageIndicator_t DmgIndicator;
					DmgIndicator.iDamage = Event->GetInt("dmg_health");
					DmgIndicator.Player = hurt;
					DmgIndicator.flEraseTime = localplayer->GetTickBase() * Interfaces::Globals->interval_per_tick + 3.f;
					DmgIndicator.bInitialized = false;
					damage_indicators.data.push_back(DmgIndicator);
				}
			}
		}
		if (!strcmp(Event->GetName(), "player_hurt"))
		{

			int attackerid = Event->GetInt("attacker");
			int entityid = Interfaces::Engine->GetPlayerForUserID(attackerid);


			if (entityid == Interfaces::Engine->GetLocalPlayer())
			{

				int nUserID = Event->GetInt("attacker");
				int nDead = Event->GetInt("userid");
				if (nUserID || nDead)
				{

					player_info_t killed_info = GetInfo(Interfaces::Engine->GetPlayerForUserID(nDead));
					player_info_t killer_info = GetInfo(Interfaces::Engine->GetPlayerForUserID(nUserID));
					std::string before = ("Mirror.tk ");
					std::string one = ("-> ");
					std::string two = ("hit ");
					std::string three = killed_info.name;
					std::string foura = " for: ";
					std::string fivea = Event->GetString("dmg_health");
					std::string damage = " damage with a ";
					std::string damageb = Event->GetString("weapon");;
					std::string fourb = " (";
					std::string fiveb = Event->GetString("health");
					std::string six = " hp remaining)";
					std::string newline = "\n";






					if (Options::Menu.VisualsTab.logs.GetState())
					{
						Msg((before + one + two + three + foura + fivea + damage + damageb + fourb + fiveb + six + newline).c_str());


					}
				}
			}
		}
		if (!strcmp(Event->GetName(), "item_purchase"))
		{

			int nUserID = Event->GetInt("attacker");
			int nDead = Event->GetInt("userid");
			if (nUserID || nDead)
			{

				player_info_t killed_info = GetInfo(Interfaces::Engine->GetPlayerForUserID(nDead));
				player_info_t killer_info = GetInfo(Interfaces::Engine->GetPlayerForUserID(nUserID));
				std::string before = ("Mirror.tk ");
				std::string beforeb = ("-> ");
				std::string one = killed_info.name;
				std::string two = ("has purchased: ");
				std::string three = Event->GetString("weapon");
				std::string six = "\n";
				if (Options::Menu.VisualsTab.logs.GetState())
				{

					Msg((before + beforeb + one + two + three + six).c_str());

				}

			}
		}
		if (Options::Menu.RageBotTab.resolver.GetIndex() != 0 || Options::Menu.LegitBotTab.legitresolver.GetState())
		{
			IClientEntity* pLocal = hackManager.pLocal();

			if (!strcmp(Event->GetName(), "weapon_fire")) {
				auto userID = Event->GetInt("userid");
				auto attacker = Interfaces::Engine->GetPlayerForUserID(userID);

				if (attacker) {
					if (attacker == Interfaces::Engine->GetLocalPlayer()) {

						if (Globals::Target) {
							Globals::fired++;
						}
					}
				}
			}

			if (!strcmp(Event->GetName(), "player_hurt")) {
				int deadfag = Event->GetInt("userid");
				int attackingfag = Event->GetInt("attacker");

				if (Interfaces::Engine->GetPlayerForUserID(deadfag) != Interfaces::Engine->GetLocalPlayer() && Interfaces::Engine->GetPlayerForUserID(attackingfag) == Interfaces::Engine->GetLocalPlayer()) {
					Globals::hit++;
				}
			}

			if (Globals::fired > 12) {
				Globals::fired = 0;
				Globals::hit = 0;
			}

			Globals::missedshots = Globals::fired - Globals::hit;
		}
	
	}
	return oFireEventClientSide(ECX, Event);

		
}


#define TEXTURE_GROUP_LIGHTMAP                      "Lightmaps"
#define TEXTURE_GROUP_WORLD                         "World textures"
#define TEXTURE_GROUP_MODEL                         "Model textures"
#define TEXTURE_GROUP_VGUI                          "VGUI textures"
#define TEXTURE_GROUP_PARTICLE                      "Particle textures"
#define TEXTURE_GROUP_DECAL                         "Decal textures"
#define TEXTURE_GROUP_SKYBOX                        "SkyBox textures"
#define TEXTURE_GROUP_CLIENT_EFFECTS                "ClientEffect textures"
#define TEXTURE_GROUP_OTHER                         "Other textures"
#define TEXTURE_GROUP_PRECACHED                     "Precached"             // TODO: assign texture groups to the precached materials
#define TEXTURE_GROUP_CUBE_MAP                      "CubeMap textures"
#define TEXTURE_GROUP_RENDER_TARGET                 "RenderTargets"
#define TEXTURE_GROUP_UNACCOUNTED                   "Unaccounted textures"  // Textures that weren't assigned a texture group.
//#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER        "Static Vertex"
#define TEXTURE_GROUP_STATIC_INDEX_BUFFER           "Static Indices"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_DISP     "Displacement Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_COLOR    "Lighting Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_WORLD    "World Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_MODELS   "Model Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_OTHER    "Other Verts"
#define TEXTURE_GROUP_DYNAMIC_INDEX_BUFFER          "Dynamic Indices"
#define TEXTURE_GROUP_DYNAMIC_VERTEX_BUFFER         "Dynamic Verts"
#define TEXTURE_GROUP_DEPTH_BUFFER                  "DepthBuffer"
#define TEXTURE_GROUP_VIEW_MODEL                    "ViewModel"
#define TEXTURE_GROUP_PIXEL_SHADERS                 "Pixel Shaders"
#define TEXTURE_GROUP_VERTEX_SHADERS                "Vertex Shaders"
#define TEXTURE_GROUP_RENDER_TARGET_SURFACE         "RenderTarget Surfaces"
#define TEXTURE_GROUP_MORPH_TARGETS                 "Morph Targets"


void __fastcall  hkSceneEnd(void *pEcx, void *pEdx) {
	Hooks::VMTRenderView.GetMethod<SceneEnd_t>(9)(pEcx);
	pSceneEnd(pEcx);
	static float vis_col[3] = { 0.f, 0.f, 0.f };
	static float hid_col[3] = { 0.f, 0.f, 0.f };
	
	IClientEntity* pLocalPlayer = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (Options::Menu.VisualsTab.Chams.GetIndex() == 0)
	{

	}
	else if (Options::Menu.VisualsTab.Chams.GetIndex() == 1 && Options::Menu.VisualsTab.ChamsPlayers.GetState())
	{

		for (int i = 1; i <= Interfaces::Globals->maxClients; ++i) {
			auto ent = Interfaces::EntList->GetClientEntity(i);
			IMaterial *material;
			if (!ent) continue;

			if (pLocalPlayer && (!Options::Menu.VisualsTab.ChamsEnemyOnly.GetState() || ent->GetTeamNum() != pLocalPlayer->GetTeamNum()))
			{

				if (ent)
				{	
					material = Interfaces::MaterialSystem->FindMaterial(("chams_ignorez"), TEXTURE_GROUP_MODEL);
					if (material)
					{

						hid_col[0] = Options::Menu.ColorsTab.ChamsNotVisRed.GetValue() / 255.f;
						hid_col[1] = Options::Menu.ColorsTab.ChamsNotVisGreen.GetValue() / 255.f;
						hid_col[2] = Options::Menu.ColorsTab.ChamsNotVisBlue.GetValue() / 255.f;

						Interfaces::RenderView->SetColorModulation(hid_col);
						Interfaces::ModelRender->ForcedMaterialOverride(material);
						ent->draw_model(1, 255);
					}
					material = Interfaces::MaterialSystem->FindMaterial(("chams"), TEXTURE_GROUP_MODEL);
					if (material)
					{

						vis_col[0] = Options::Menu.ColorsTab.ChamsVisRed.GetValue() / 255.f;
						vis_col[1] = Options::Menu.ColorsTab.ChamsVisGreen.GetValue() / 255.f;
						vis_col[2] = Options::Menu.ColorsTab.ChamsVisBlue.GetValue() / 255.f;

						Interfaces::RenderView->SetColorModulation(vis_col);
						Interfaces::ModelRender->ForcedMaterialOverride(material);
						ent->draw_model(1, 255);
					}

					Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
				}
			}
		}
	}
	else if (Options::Menu.VisualsTab.Chams.GetIndex() == 2 && Options::Menu.VisualsTab.ChamsPlayers.GetState())
	{

		for (int i = 1; i <= Interfaces::Globals->maxClients; ++i) {
			auto ent = Interfaces::EntList->GetClientEntity(i);
			static  IMaterial* material = CreateMaterial(true, false, true);
			if (!ent) continue;

			if (pLocalPlayer && (!Options::Menu.VisualsTab.ChamsEnemyOnly.GetState() || ent->GetTeamNum() != pLocalPlayer->GetTeamNum()))
			{

				if (ent)
				{
				
					if (material)
					{

						hid_col[0] = Options::Menu.ColorsTab.ChamsNotVisRed.GetValue() / 255.f;
						hid_col[1] = Options::Menu.ColorsTab.ChamsNotVisGreen.GetValue() / 255.f;
						hid_col[2] = Options::Menu.ColorsTab.ChamsNotVisBlue.GetValue() / 255.f;

						Interfaces::RenderView->SetColorModulation(hid_col);
						Interfaces::ModelRender->ForcedMaterialOverride(material);
						ent->draw_model(1, 255);
					}
			
					if (material)
					{

						vis_col[0] = Options::Menu.ColorsTab.ChamsVisRed.GetValue() / 255.f;
						vis_col[1] = Options::Menu.ColorsTab.ChamsVisGreen.GetValue() / 255.f;
						vis_col[2] = Options::Menu.ColorsTab.ChamsVisBlue.GetValue() / 255.f;

						Interfaces::RenderView->SetColorModulation(vis_col);
						Interfaces::ModelRender->ForcedMaterialOverride(material);
						ent->draw_model(1, 255);
					}

					Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
				}
			}
		}
	}
	
	
	IClientEntity* pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Options::Menu.VisualsTab.FakeAngleChams.GetIndex() == 0)
	{

	}
	else if (Options::Menu.VisualsTab.FakeAngleChams.GetIndex() == 1)
	{
		if (warmup) return;

		if (pLocal)
		{
			if (Options::Menu.VisualsTab.scopemat.GetIndex() == 0)
			{
				static  IMaterial* CoveredLit = CreateMaterial(false, true, false);

				if (CoveredLit)
				{

					Vector OrigAng;
					OrigAng = pLocal->GetEyeAngles();
					pLocal->SetAngle2(Vector(0, hackManager.pLocal()->GetEyeAnglesXY()->y, 0));
					float	NormalColor[3] = { 0.75, 0.75 , 0.75 };
					bool LbyColor = false;
					float lbyUpdateColor[3] = { 0, 1, 0 };
					Interfaces::RenderView->SetColorModulation(LbyColor ? lbyUpdateColor : NormalColor);
					Interfaces::ModelRender->ForcedMaterialOverride(CoveredLit);
					pLocal->draw_model(STUDIO_RENDER, 255);
					Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
					pLocal->SetAngle2(OrigAng);
				}
			}
			else if (Options::Menu.VisualsTab.scopemat.GetIndex() == 1)
			{
				static  IMaterial* CoveredLit = CreateMaterial(false, true, true);

				if (CoveredLit)
				{

					Vector OrigAng;
					OrigAng = pLocal->GetEyeAngles();
					pLocal->SetAngle2(Vector(0, hackManager.pLocal()->GetEyeAnglesXY()->y, 0));
					float	NormalColor[3] = { 0.75, 0.75 , 0.75 };
					bool LbyColor = false;
					float lbyUpdateColor[3] = { 0, 1, 0 };
					Interfaces::RenderView->SetColorModulation(LbyColor ? lbyUpdateColor : NormalColor);
					Interfaces::ModelRender->ForcedMaterialOverride(CoveredLit);
					pLocal->draw_model(STUDIO_RENDER, 255);
					Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
					pLocal->SetAngle2(OrigAng);
				}
			}
			else if (Options::Menu.VisualsTab.scopemat.GetIndex() == 2)
			{
				static  IMaterial* CoveredLit = CreateMaterial(false, true, false);

				if (CoveredLit)
				{

					Vector OrigAng;
					OrigAng = pLocal->GetEyeAngles();
					pLocal->SetAngle2(Vector(0, hackManager.pLocal()->GetEyeAnglesXY()->y, 0));
					float	NormalColor[3] = { rand() % 10 / 10,  rand() % 10 / 10 ,  rand() % 10 / 10 };
					bool LbyColor = false;
					float lbyUpdateColor[3] = { 0, 1, 0 };
					Interfaces::RenderView->SetColorModulation(LbyColor ? lbyUpdateColor : NormalColor);
					Interfaces::ModelRender->ForcedMaterialOverride(CoveredLit);
					pLocal->draw_model(STUDIO_RENDER, 220);
					Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
					pLocal->SetAngle2(OrigAng);
				}
			}
			
		}
	}
	else if (Options::Menu.VisualsTab.FakeAngleChams.GetIndex() == 2)
	{
		if (warmup) return;

		if (pLocal)
		{
			if (Options::Menu.VisualsTab.scopemat.GetIndex() == 0)
			{
				static  IMaterial* CoveredLit = CreateMaterial(false, true, false);
				Vector OrigAng;
				OrigAng = pLocal->GetEyeAngles();

				if (CoveredLit)
				{

					pLocal->SetAngle2(Vector(0, pLocal->GetLowerBodyYaw(), 0));
					bool LbyColor = false;
					float NormalColor[3] = { 1, 1, 1 };
					float lbyUpdateColor[3] = { 0, 1, 0 };
					Interfaces::RenderView->SetColorModulation(LbyColor ? lbyUpdateColor : NormalColor);
					Interfaces::ModelRender->ForcedMaterialOverride(CoveredLit);
					pLocal->draw_model(1, 0);
					Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
					pLocal->SetAngle2(OrigAng);

				}
			}
			else if(Options::Menu.VisualsTab.scopemat.GetIndex() == 1)
			{
				static  IMaterial* CoveredLit = CreateMaterial(false, true, true);
				Vector OrigAng;
				OrigAng = pLocal->GetEyeAngles();

				if (CoveredLit)
				{

					pLocal->SetAngle2(Vector(0, pLocal->GetLowerBodyYaw(), 0));
					bool LbyColor = false;
					float NormalColor[3] = { 1, 1, 1 };
					float lbyUpdateColor[3] = { 0, 1, 0 };
					Interfaces::RenderView->SetColorModulation(LbyColor ? lbyUpdateColor : NormalColor);
					Interfaces::ModelRender->ForcedMaterialOverride(CoveredLit);
					pLocal->draw_model(1, 0);
					Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
					pLocal->SetAngle2(OrigAng);

				}
			}
			else if (Options::Menu.VisualsTab.scopemat.GetIndex() == 2)
			{
				static  IMaterial* CoveredLit = CreateMaterial(false, true, false);
				Vector OrigAng;
				OrigAng = pLocal->GetEyeAngles();

				if (CoveredLit)
				{

					pLocal->SetAngle2(Vector(0, pLocal->GetLowerBodyYaw(), 0));
					bool LbyColor = false;
					float NormalColor[3] = { rand() % 10 / 10, rand() % 10 / 10, rand() % 10 / 10 };
					float lbyUpdateColor[3] = { 0, 1, 0 };
					Interfaces::RenderView->SetColorModulation(LbyColor ? lbyUpdateColor : NormalColor);
					Interfaces::ModelRender->ForcedMaterialOverride(CoveredLit);
					pLocal->draw_model(1, 0);
					Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
					pLocal->SetAngle2(OrigAng);

				}
			}
			
		}



	
	}
}


void Hooks::DrawBeamd(Vector src, Vector end, Color color)
{
	float time = Options::Menu.VisualsTab.beamtime.GetValue();
	float size = Options::Menu.VisualsTab.beamsize.GetValue() / 100;
	BeamInfo_t beamInfo;
	beamInfo.m_nType = TE_BEAMPOINTS;
	beamInfo.m_pszModelName = "sprites/physbeam.vmt";
	beamInfo.m_nModelIndex = -1; // will be set by CreateBeamPoints if its -1
	beamInfo.m_flHaloScale = 0.0f;
	beamInfo.m_flLife = time;
	beamInfo.m_flWidth = 1.1;
	beamInfo.m_flEndWidth = 1.0;

	beamInfo.m_flFadeLength = 0.0f;
	beamInfo.m_flAmplitude = 2.0f;
	beamInfo.m_flBrightness = color.a();
	beamInfo.m_flSpeed = 0.2f;
	beamInfo.m_nStartFrame = 0;
	beamInfo.m_flFrameRate = 0.f;
	beamInfo.m_flRed = Options::Menu.ColorsTab.beamplayer_r.GetValue();
	beamInfo.m_flGreen = Options::Menu.ColorsTab.beamplayer_g.GetValue();
	beamInfo.m_flBlue = Options::Menu.ColorsTab.beamplayer_b.GetValue();
	beamInfo.m_nSegments = 2;
	beamInfo.m_bRenderable = true;
	beamInfo.m_nFlags = 0;
	beamInfo.m_vecStart = src;
	beamInfo.m_vecEnd = end;



	Beam_t* myBeam = Interfaces::g_pViewRenderBeams->CreateBeamPoints(beamInfo);

	if (myBeam)
		Interfaces::g_pViewRenderBeams->DrawBeam(myBeam);
}
void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld)
{
	if (!Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;

	Color color;

	float flColor[3] = { 0.f };

	static IMaterial* CoveredLit = CreateMaterial(true);
	static IMaterial* OpenLit = CreateMaterial(false);
	static IMaterial* CoveredFlat = CreateMaterial(true, false);
	static IMaterial* OpenFlat = CreateMaterial(false, false);
	static IMaterial* wire = CreateMaterial(true, false, true);

	bool DontDraw = false;

	int ChamsStyle = Options::Menu.VisualsTab.OptionsChams.GetIndex();
	int HandsStyle = Options::Menu.VisualsTab.OtherNoHands.GetIndex();
	int WeaponsChams = Options::Menu.VisualsTab.WeaponChams.GetIndex();

	IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;

	const char* ModelName = Interfaces::ModelInfo->GetModelName((model_t*)pInfo.pModel);
	IClientEntity* pModelEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(pInfo.entity_index);
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Options::Menu.VisualsTab.Active.GetState())
	{




		float sr = Options::Menu.ColorsTab.scope_r.GetValue();
		float sg = Options::Menu.ColorsTab.scope_g.GetValue();
		float sb = Options::Menu.ColorsTab.scope_b.GetValue();


		float hr = Options::Menu.ColorsTab.hand_r.GetValue();
		float hg = Options::Menu.ColorsTab.hand_g.GetValue();
		float hb = Options::Menu.ColorsTab.hand_b.GetValue();

		float gr = Options::Menu.ColorsTab.gun_r.GetValue();
		float gg = Options::Menu.ColorsTab.gun_g.GetValue();
		float gb = Options::Menu.ColorsTab.gun_b.GetValue();


		if (pLocal->IsScoped() && strstr(ModelName, "models/player") && pModelEntity != nullptr && pModelEntity->GetTeamNum() == pLocal->GetTeamNum() && pModelEntity == pLocal)
		{
		
				if (Options::Menu.VisualsTab.localmaterial.GetIndex() == 1)
					Interfaces::RenderView->SetBlend(0.20f);
				else if (Options::Menu.VisualsTab.localmaterial.GetIndex() == 2)
				{
					if (Options::Menu.VisualsTab.FakeAngleChams.GetIndex() != 0)
					{
						Interfaces::RenderView->SetBlend(0.25f);
					}
					else
					{
						flColor[0] = sr;
						flColor[1] = sg;
						flColor[2] = sb;

						Interfaces::RenderView->SetBlend(0.10);
						Interfaces::RenderView->SetColorModulation(flColor);
						Interfaces::ModelRender->ForcedMaterialOverride(covered);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
					}
				}
				else if (Options::Menu.VisualsTab.localmaterial.GetIndex() == 3)
				{
					if (Options::Menu.VisualsTab.FakeAngleChams.GetIndex() != 0)
					{
						Interfaces::RenderView->SetBlend(0.15f);
					}
					else
					{
						flColor[0] = sr / 255.f;
						flColor[1] = sg / 255.f;
						flColor[2] = sb / 255.f;
						Interfaces::RenderView->SetColorModulation(flColor);
						Interfaces::RenderView->SetBlend(1.f);
						Interfaces::ModelRender->ForcedMaterialOverride(wire);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
					}
				}
				else if (Options::Menu.VisualsTab.localmaterial.GetIndex() == 4)
				{
					Interfaces::RenderView->SetBlend(0);
				}
				else if (Options::Menu.VisualsTab.localmaterial.GetIndex() == 5)
				{
					if (Options::Menu.VisualsTab.FakeAngleChams.GetIndex() != 0)
					{
						Interfaces::RenderView->SetBlend(0.15f);
					}
					else
					{
						flColor[0] = rand() % 250 / 255.f;
						flColor[1] = rand() % 250 / 255.f;
						flColor[2] = rand() % 250 / 255.f;
						Interfaces::RenderView->SetColorModulation(flColor);
						Interfaces::RenderView->SetBlend(0.10);
						Interfaces::ModelRender->ForcedMaterialOverride(covered);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
					}
				}
			

			if (ChamsStyle != 0 && Options::Menu.VisualsTab.FiltersPlayers.GetState() && strstr(ModelName, "models/player"))
			{
				if (pModelEntity == nullptr)
					return;

				if (pLocal && (!Options::Menu.VisualsTab.FiltersEnemiesOnly.GetState() ||
					pModelEntity->GetTeamNum() != pLocal->GetTeamNum()))
				{
					IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;
					IMaterial *open = ChamsStyle == 1 ? OpenLit : OpenFlat;


					if (pModelEntity)
					{
						if (pModelEntity == nullptr)
							return;


						else if (pModelEntity->IsAlive())
						{
							float alpha = 1.f;

							if (pModelEntity->HasGunGameImmunity())
								alpha = 0.5f;

							if (pModelEntity->GetTeamNum() == 2)
							{
								flColor[0] = 255.f;
								flColor[1] = 255.f;
								flColor[2] = 255.f;
							}
							else
							{
								flColor[0] = 255.f;
								flColor[1] = 255.f;
								flColor[2] = 255.f;
							}

							if (Options::Menu.VisualsTab.ChamsVisibleOnly.GetState())
							{
								Interfaces::RenderView->SetColorModulation(flColor);
								Interfaces::RenderView->SetBlend(1);
								Interfaces::ModelRender->ForcedMaterialOverride(covered);
								oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
							}
							else
							{
								Interfaces::RenderView->SetColorModulation(flColor);
								Interfaces::RenderView->SetBlend(0);
								Interfaces::ModelRender->ForcedMaterialOverride(covered);
								oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
							}

							if (pModelEntity->GetTeamNum() == 2)
							{
								flColor[0] = 255.f;
								flColor[1] = 255.f;
								flColor[2] = 255.f;
							}
							else
							{
								flColor[0] = 255.f;
								flColor[1] = 255.f;
								flColor[2] = 255.f;
							}

							Interfaces::RenderView->SetColorModulation(flColor);
							Interfaces::RenderView->SetBlend(alpha);
							Interfaces::ModelRender->ForcedMaterialOverride(open);
						}
						else
						{
							color.SetColor(255, 255, 255, 255);
							ForceMaterial(color, open);
						}

					}
				}
			}
		}

		else
		{
			if (ChamsStyle != 0 && Options::Menu.VisualsTab.FiltersPlayers.GetState() && strstr(ModelName, "models/player"))
			{
				if (pModelEntity == nullptr)
					return;

				if (pLocal && (!Options::Menu.VisualsTab.FiltersEnemiesOnly.GetState() ||
					pModelEntity->GetTeamNum() != pLocal->GetTeamNum()))
				{
					IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;
					IMaterial *open = ChamsStyle == 1 ? OpenLit : OpenFlat;


					if (pModelEntity)
					{
						if (pModelEntity == nullptr)
							return;


						else if (pModelEntity->IsAlive() && pModelEntity->GetHealth() > 0)
						{
							float alpha = 1.f;

							if (pModelEntity->HasGunGameImmunity())
								alpha = 0.5f;

							if (pModelEntity->GetTeamNum() == 2)
							{
								flColor[0] = 255.f;
								flColor[1] = 255.f;
								flColor[2] = 255.f;
							}
							else
							{
								flColor[0] = 255.f;
								flColor[1] = 255.f;
								flColor[2] = 255.f;
							}

							if (Options::Menu.VisualsTab.ChamsVisibleOnly.GetState())
							{
								Interfaces::RenderView->SetColorModulation(flColor);
								Interfaces::RenderView->SetBlend(1);
								Interfaces::ModelRender->ForcedMaterialOverride(covered);
								oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
							}
							else
							{
								Interfaces::RenderView->SetColorModulation(flColor);
								Interfaces::RenderView->SetBlend(0);
								Interfaces::ModelRender->ForcedMaterialOverride(covered);
								oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
							}

							if (pModelEntity->GetTeamNum() == 2)
							{
								flColor[0] = 255.f;
								flColor[1] = 255.f;
								flColor[2] = 255.f;
							}
							else
							{
								flColor[0] = 255.f;
								flColor[1] = 255.f;
								flColor[2] = 255.f;
							}

							Interfaces::RenderView->SetColorModulation(flColor);
							Interfaces::RenderView->SetBlend(alpha);
							Interfaces::ModelRender->ForcedMaterialOverride(open);
						}
						else
						{
							color.SetColor(255, 255, 255, 255);
							ForceMaterial(color, open);
						}

					}
				}
			}
			else if (HandsStyle != 0 && strstr(ModelName, XorStr("arms")))
			{
				if (HandsStyle == 1)
				{
					Interfaces::RenderView->SetBlend(0.20f);
				}
				else if (HandsStyle == 2)
				{
					flColor[0] = hr / 255.f;
					flColor[1] = hg / 255.f;
					flColor[2] = hb / 255.f;
					Interfaces::RenderView->SetColorModulation(flColor);
					Interfaces::RenderView->SetBlend(0.30);
					Interfaces::ModelRender->ForcedMaterialOverride(covered);
					oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
				}

				else if (HandsStyle == 3)
				{
					flColor[0] = hr / 255.f;
					flColor[1] = hg / 255.f;
					flColor[2] = hb / 255.f;
					Interfaces::RenderView->SetColorModulation(flColor);
					Interfaces::RenderView->SetBlend(1.f);
					Interfaces::ModelRender->ForcedMaterialOverride(wire);
					oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
				}

				else if (HandsStyle == 4)
				{
					Interfaces::RenderView->SetBlend(0);
				}
				else if (HandsStyle == 5)
				{
					flColor[0] = rand() % 250 / 255.f;
					flColor[1] = rand() % 250 / 255.f;
					flColor[2] = rand() % 250 / 255.f;
					Interfaces::RenderView->SetColorModulation(flColor);
					Interfaces::RenderView->SetBlend(0.35);
					Interfaces::ModelRender->ForcedMaterialOverride(covered);
					oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
				}
			}
			else if (WeaponsChams == 1 && strstr(ModelName, "models/weapons/v_") && !strstr(ModelName, "arms"))
			{
				IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;
				color.SetColor(255, 255, 255, 255);
				ForceMaterial(color, covered);
				Interfaces::ModelRender->ForcedMaterialOverride(OpenLit);
				Interfaces::RenderView->SetBlend(0.20f);
			}
			else if (WeaponsChams == 2 && strstr(ModelName, "models/weapons/v_") && !strstr(ModelName, "arms"))
			{
				flColor[0] = gr;
				flColor[1] = gg;
				flColor[2] = gb;

				Interfaces::RenderView->SetBlend(0.30);
				Interfaces::RenderView->SetColorModulation(flColor);
				Interfaces::ModelRender->ForcedMaterialOverride(OpenLit);


			}
			else if (WeaponsChams == 3 && strstr(ModelName, "models/weapons/v_") && !strstr(ModelName, "arms"))
			{
				flColor[0] = gr / 255.f;
				flColor[1] = gg / 255.f;
				flColor[2] = gb / 255.f;
				Interfaces::RenderView->SetColorModulation(flColor);
				Interfaces::RenderView->SetBlend(0.35);
				ForceMaterial(color, covered);
				Interfaces::ModelRender->ForcedMaterialOverride(wire);


			}
			else if (WeaponsChams == 4 && strstr(ModelName, "models/weapons/v_") && !strstr(ModelName, "arms"))
			{
				IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;
				color.SetColor(255, 255, 255, 0);
				ForceMaterial(color, covered);
				Interfaces::ModelRender->ForcedMaterialOverride(OpenLit);
				Interfaces::RenderView->SetBlend(0.0f);
			}
			else if (WeaponsChams == 5 && strstr(ModelName, "models/weapons/v_") && !strstr(ModelName, "arms"))
			{

				IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;
				color.SetColor(rand() % 255, rand() % 255, rand() % 255, 95);
				ForceMaterial(color, covered);
				Interfaces::ModelRender->ForcedMaterialOverride(OpenLit);
				Interfaces::RenderView->SetBlend(1);
			}
			
		}
	}

	if (!DontDraw)
		oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
	Interfaces::ModelRender->ForcedMaterialOverride(NULL);
}



int RandomInt(int min, int max)
{
	return rand() % max + min;
}


std::vector<const char*> vistasmoke_mats =
{
	"particle/vistasmokev1/vistasmokev1_fire",
	"particle/vistasmokev1/vistasmokev1_smokegrenade",
	"particle/vistasmokev1/vistasmokev1_emods",
	"particle/vistasmokev1/vistasmokev1_emods_impactdust",
};

void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage)
{
	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	IClientEntity* pEntity = nullptr;


	static auto linegoesthrusmoke = Utilities::Memory::FindPattern("client.dll", (PBYTE)"\x55\x8B\xEC\x83\xEC\x08\x8B\x15\x00\x00\x00\x00\x0F\x57\xC0", "xxxxxxxx????xxx");
	static auto smokecout = *(DWORD*)(linegoesthrusmoke + 0x8);

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{
	if (Options::Menu.RageBotTab.resolver.GetIndex() != 0) 
	{
		for (int i = 0; i < Interfaces::Globals->maxClients; i++) 
		{
			auto m_entity = Interfaces::EntList->GetClientEntity(i);
			if (!m_entity || m_entity == pLocal || m_entity->GetClientClass()->m_ClassID != (int)CSGOClassID::CCSPlayer || !m_entity->IsAlive()) continue;
			{
			
				lagcompensation.disable_interpolation(m_entity);
			}
		}
		
	}
	if (Options::Menu.VisualsTab.OtherNoSmoke.GetState())
	{
		std::vector<const char*> vistasmoke_mats =
		{
			"particle/vistasmokev1/vistasmokev1_fire",
			"particle/vistasmokev1/vistasmokev1_smokegrenade",
			"particle/vistasmokev1/vistasmokev1_emods",
			"particle/vistasmokev1/vistasmokev1_emods_impactdust",
		};

		for (auto matName : vistasmoke_mats) {
			IMaterial* mat = Interfaces::MaterialSystem->FindMaterial(matName, "Other textures");
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
		}


		if (Options::Menu.VisualsTab.OtherNoSmoke.GetState()) {
			*(int*)(smokecout) = 0;
		}
	}

		
		
	

		if (Options::Menu.MiscTab.OtherThirdperson.GetState())
		{
			Vector thirdpersonMode;
			thirdpersonMode = LastAngleAAReal;
			static bool rekt = false;
			if (!rekt)
			{
				ConVar* sv_cheats = Interfaces::CVar->FindVar("sv_cheats");
				SpoofedConvar* sv_cheats_spoofed = new SpoofedConvar(sv_cheats);
				sv_cheats_spoofed->SetInt(1);
				rekt = true;
			}


			static bool kek = false;

			if (!kek)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("thirdperson");
				kek = true;
			}

			static bool toggleThirdperson;
			static float memeTime;
			int ThirdPersonKey = Options::Menu.MiscTab.ThirdPersonKeyBind.GetKey();
			if (ThirdPersonKey >= 0 && GUI.GetKeyState(ThirdPersonKey) && abs(memeTime - Interfaces::Globals->curtime) > 0.5)
			{
				toggleThirdperson = !toggleThirdperson;
				memeTime = Interfaces::Globals->curtime;
			}


			if (toggleThirdperson)
			{
				Interfaces::pInput->m_fCameraInThirdPerson = true;
				if (*(bool*)((DWORD)Interfaces::pInput + 0xA5))
					*(Vector*)((DWORD)pLocal + 0x31C8) = thirdpersonMode;
			}
			else
			{
					Interfaces::Engine->ClientCmd_Unrestricted("firstperson");
			}
			


		}

		if (curStage == FRAME_RENDER_START)
		{
			for (int i = 1; i <= Interfaces::Globals->maxClients; i++)
			{
				if (i == Interfaces::Engine->GetLocalPlayer()) continue;

				IClientEntity* pEnt = Interfaces::EntList->GetClientEntity(i);
				if (!pEnt) continue;

				*(int*)((uintptr_t)pEnt + 0xA30) = Interfaces::Globals->framecount; //we'll skip occlusion checks now
				*(int*)((uintptr_t)pEnt + 0xA28) = 0;//clear occlusion flags
			}
			for (auto matName : vistasmoke_mats)
			{
				IMaterial* mat = Interfaces::MaterialSystem->FindMaterial(matName, "Other textures");
				mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
			}
		}
	
	if (Options::Menu.RageBotTab.resolver.GetIndex() != 0)
	{
		for (int i = 1; i < Interfaces::Globals->maxClients; i++)
		{
			if (i == Interfaces::Engine->GetLocalPlayer()) continue;
			IClientEntity* pCurEntity = Interfaces::EntList->GetClientEntity(i);
			if (!pCurEntity) continue;
			*(int*)((uintptr_t)pCurEntity + 0xA30) = Interfaces::Globals->framecount;
			*(int*)((uintptr_t)pCurEntity + 0xA28) = 0;
		}
	}

	
	
		ResolverSetup::GetInst().FSN(pEntity, curStage);


	}
	
	
	

	oFrameStageNotify(curStage);

}
	




void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup)
{
	auto local = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!local || !Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;

	if (local)
	{
		if (Options::Menu.VisualsTab.Active.GetState())
		{
			if (local->IsScoped())
			{
				if (GetAsyncKeyState(Options::Menu.VisualsTab.zoom.GetKey()))
					pSetup->fov = 5;
				else
				{
					if (Options::Menu.VisualsTab.OtherFOV.GetValue() > 0)
					pSetup->fov = (pSetup->fov + Options::Menu.VisualsTab.OtherFOV.GetValue());
					else
					pSetup->fov += Options::Menu.VisualsTab.OtherFOV.GetValue();
				}
			}
			else
			{
				if (GetAsyncKeyState(Options::Menu.VisualsTab.zoom.GetKey()))
					pSetup->fov = 30;
				else
					pSetup->fov += Options::Menu.VisualsTab.OtherFOV.GetValue();
			}
		}
	}

	oOverrideView(ecx, edx, pSetup);
}

void GetViewModelFOV(float& fov)
{
	IClientEntity* localplayer = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{

		if (!localplayer)
			return;


		if (Options::Menu.VisualsTab.Active.GetState())
			fov = Options::Menu.VisualsTab.OtherViewmodelFOV.GetValue();
	}
}

float __stdcall GGetViewModelFOV()
{
	float fov = Hooks::VMTClientMode.GetMethod<oGetViewModelFOV>(35)();

	GetViewModelFOV(fov);

	return fov;
}

void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw)
{
	static DWORD oRenderView = Hooks::VMTRenderView.GetOriginalFunction(6);

	IClientEntity* pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	__asm
	{
		PUSH whatToDraw
		PUSH nClearFlags
		PUSH hudViewSetup
		PUSH setup
		MOV ECX, ecx
		CALL oRenderView
	}
}
