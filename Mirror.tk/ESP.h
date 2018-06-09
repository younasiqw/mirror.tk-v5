#pragma once

#include "Hacks.h"
extern int missedLogHits[65];
struct Box
{
	int x = -1, y = -1, w = -1, h = -1;
};
class CEsp : public CHack
{
public:
	void Init();
	void Draw();
	void Move(CUserCmd *pCmd, bool &bSendPacket);
private:

	IClientEntity * BombCarrier;

	struct ESPBox
	{
		int x, y, w, h;
	};

	void DrawPlayer(IClientEntity* pEntity, player_info_t pinfo);
	void hp1(IClientEntity * pEntity, CEsp::ESPBox size);
	void hp1(IClientEntity * pEntity, Box size);
	void DrawInfo22(IClientEntity * pEntity, Box size);
	void DrawInfo23(IClientEntity * pEntity, Box size);
	void BacktrackingCross(IClientEntity * pEntity);
	void SpecList();
	void draw_hitbones(IClientEntity * m_entity);


	Color GetPlayerColor(IClientEntity* pEntity);

	void DrawGun(IClientEntity * pEntity, CEsp::ESPBox size);
	void Corners(CEsp::ESPBox size, Color color, IClientEntity * pEntity);

	void FilledBox(CEsp::ESPBox size, Color color);
	void DrawBoxx(Box size, Color color);
	void DrawBoxx2(Box size, Color color);
	bool get_box(IClientEntity * m_entity, Box & box, bool dynamic);
	bool GetBox(IClientEntity* pEntity, ESPBox &result);

	void DrawGlow();
	void EntityGlow();

	void DrawName(player_info_t pinfo, Box size);
	void DrawHealth(IClientEntity * pEntity, Box size);


	void DrawHealthOld(IClientEntity * pEntity, Box size);

	void DrawInfo21(IClientEntity * pEntity, Box size);

	void DrawInfo(IClientEntity * pEntity, Box size);

	void DrawCross(IClientEntity* pEntity);

	void DrawSkeleton(IClientEntity* pEntity);
	void DrawHealthText(IClientEntity * pEntity, CEsp::ESPBox size);

	void BoxAndText(IClientEntity * entity, std::string text);
	void DrawThrowable(IClientEntity * throwable);
	
	void DrawDrop(IClientEntity* pEntity, ClientClass* cClass);
	void DrawBombPlanted(IClientEntity* pEntity, ClientClass* cClass);
	void DrawBomb(IClientEntity* pEntity, ClientClass* cClass);
};

