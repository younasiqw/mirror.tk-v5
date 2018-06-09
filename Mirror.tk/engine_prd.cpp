#include "sdk.h"
#include "engine_prd.h"
#include "xor.h"
#include "Hacks.h"

static char* cMoveData = nullptr;

void CPredictionSystem::EnginePrediction(CUserCmd* pCmd)
{

	IClientEntity* localplayer = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	return;

	if (!Interfaces::MoveHelper || !pCmd || !localplayer)
		return;

	CMoveData C_MoveData;

	//iFlagsBackup = *localplayer->GetFlags();
	//iButtonsBackup = pCmd->buttons;

	//localplayer->SetCurrentCommand(pCmd);

	if (Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
	{
		if (Options::Menu.RageBotTab.AimbotEnable.GetState())
		{
			if (!m_pPredictionRandomSeed || !m_pSetPredictionPlayer) {
				m_pPredictionRandomSeed = *reinterpret_cast<int**>(GameUtils::FindPattern1((XorStr("client.dll")), XorStr("A3 ? ? ? ? 66 0F 6E 86")) + 1);
				m_pSetPredictionPlayer = *reinterpret_cast<int**>(GameUtils::FindPattern1((XorStr("client.dll")), XorStr("89 35 ? ? ? ? F3 0F 10 48 20")) + 2);
			}

			if (!cMoveData)
				cMoveData = (char*)(calloc(1, sizeof(CMoveData)));

			Interfaces::MoveHelper->SetHost(localplayer);
			//*m_pPredictionRandomSeed = MD5_PseudoRandom(pCmd->command_number) & 0x7FFFFFFF;
			*m_pSetPredictionPlayer = uintptr_t(localplayer);

			Interfaces::Globals->curtime = localplayer->GetTickBase() * Interfaces::Globals->interval_per_tick;
			Interfaces::Globals->frametime = Interfaces::Globals->interval_per_tick;

			pCmd->buttons |= *reinterpret_cast<uint8_t*>(uintptr_t(localplayer) + 0x3310);

			if (pCmd->impulse)
				*reinterpret_cast<uint8_t*>(uintptr_t(localplayer) + 0x31EC) = pCmd->impulse;

			C_MoveData.m_nButtons = pCmd->buttons;
			int buttonsChanged = pCmd->buttons ^ *reinterpret_cast<int*>(uintptr_t(localplayer) + 0x31E8);
			*reinterpret_cast<int*>(uintptr_t(localplayer) + 0x31DC) = (uintptr_t(localplayer) + 0x31E8);
			*reinterpret_cast<int*>(uintptr_t(localplayer) + 0x31E8) = pCmd->buttons;
			*reinterpret_cast<int*>(uintptr_t(localplayer) + 0x31E0) = pCmd->buttons & buttonsChanged;
			*reinterpret_cast<int*>(uintptr_t(localplayer) + 0x31E4) = buttonsChanged & ~pCmd->buttons;

			Interfaces::GameMovement->StartTrackPredictionErrors(localplayer);

			iTickBaseBackup = localplayer->GetTickBase();

			Interfaces::Prediction1->SetupMove(localplayer, pCmd, Interfaces::MoveHelper, reinterpret_cast<CMoveData*>(cMoveData));
			Interfaces::GameMovement->ProcessMovement(localplayer, reinterpret_cast<CMoveData*>(cMoveData));
			Interfaces::Prediction1->FinishMove(localplayer, pCmd, reinterpret_cast<CMoveData*>(cMoveData));

			//localplayer->GetTickBase() = iTickBaseBackup;

			Interfaces::GameMovement->FinishTrackPredictionErrors(localplayer);

			//localplayer->SetCurrentCommand(nullptr);
			*m_pPredictionRandomSeed = -1;
			*m_pSetPredictionPlayer = 0;
			Interfaces::MoveHelper->SetHost(0);
		}
	}


}