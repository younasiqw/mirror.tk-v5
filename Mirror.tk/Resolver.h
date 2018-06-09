#include <array>
#include <string>
#include <deque>
#include <algorithm>
#include "Entities.h"
#include "CommonIncludes.h"
#include "Entities.h"
#include "Vector.h"
#include <map>
#include "Interfaces.h"
#include "Hacks.h"
#include "Hooks.h"

class ResolverSetup
{
public:
	bool didhit;
	void FSN(IClientEntity* pEntity, ClientFrameStage_t stage); //used to get the fresh THINGS and the resolve
	void CM(IClientEntity* pEntity);
	void preso(IClientEntity * pEntity);
	bool lbyDeltaOver120(int plID);
	void calculatePelvisVelocity(IClientEntity * player, int plID);
	bool didLBYUpdate(IClientEntity * player);
	//cause this is slower than FSN so we are going to get older info and not updated at all ez to compare between etc.
	std::map<int, float>badangle;
	void Resolve(IClientEntity * pEntity, int CurrentTarget);
	//resolve
	void OverrideResolver(IClientEntity * pEntity);


	void StoreThings(IClientEntity* pEntity);

	std::map<int, QAngle>StoredAngles; //y and x lol (stored)
	std::map<int, QAngle>NewANgles; //y and x lol (new)
	std::map<int, float>storedlby;
	std::map<int, float>newlby;
	std::map<int, float>storeddelta;
	std::map<int, float>newdelta;
	std::map<int, float>finaldelta;
	std::map<int, float>storedlbydelta;
	std::map<int, float>newlbydelta;
	std::map<int, float>finallbydelta;
	std::map<int, float>storedhealth;
	std::map<int, bool>lbyUpdate;
	std::map<int, float>storedhp;


	float newsimtime;
	float storedsimtime;
	bool lbyupdated;
	float storedlbyFGE;
	float storedanglesFGE;
	float storedsimtimeFGE;
	float movinglbyFGE;
	bool didhitHS;
	void StoreFGE(IClientEntity* pEntity);
	static ResolverSetup GetInst()
	{
		static ResolverSetup instance;
		return instance;
	}
};
namespace Globals
{
	extern CUserCmd* UserCmd;
	extern IClientEntity* Target;
	extern int Shots;
	extern int fired;
	extern int hit;
	extern bool change;
	extern int TargetID;
	extern bool didhitHS;
	extern int missedshots;
	extern std::map<int, QAngle> storedshit;
	extern bool Up2date;
}