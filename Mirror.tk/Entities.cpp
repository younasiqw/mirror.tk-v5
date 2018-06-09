#include "Entities.h"
#include "Offsets.h"
#include "Interfaces.h"
#include "Hooks.h"



C_BaseCombatWeapon* IClientEntity::GetWeapon2()
{
	static int iOffset = GET_NETVAR("DT_BaseCombatCharacter", "m_hActiveWeapon");
	ULONG pWeepEhandle = *(PULONG)((DWORD)this + iOffset);
	return (C_BaseCombatWeapon*)(Interfaces::EntList->GetClientEntityFromHandle((HANDLE)pWeepEhandle));
}