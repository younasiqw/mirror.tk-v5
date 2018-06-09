#include "Menu.h"
#include "Controls.h"
#include "Hooks.h" 
#include "Interfaces.h"
#include "CRC32.h"

#define WINDOW_WIDTH 690
#define WINDOW_HEIGHT 507

PhantomWindow Options::Menu;

void SaveLegitCallbk()
{
	switch (Options::Menu.MiscTab.ConfigBox.GetIndex())
	{
	case 0:
		GUI.SaveWindowState(&Options::Menu, "legit_1.cfg");
		break;
	case 1:
		GUI.SaveWindowState(&Options::Menu, "legit_2.cfg");
		break;
	case 2:
		GUI.SaveWindowState(&Options::Menu, "hvh_1.cfg");
		break;
	case 3:
		GUI.SaveWindowState(&Options::Menu, "hvh_2.cfg");
		break;
	case 4:
		GUI.SaveWindowState(&Options::Menu, "casual.cfg");
		break;
	case 5:
		GUI.SaveWindowState(&Options::Menu, "faceit.cfg");
		break;

	}
}

void LoadLegitCallbk()
{
	switch (Options::Menu.MiscTab.ConfigBox.GetIndex())
	{
	case 0:
		GUI.LoadWindowState(&Options::Menu, "legit_1.cfg");
		break;
	case 1:
		GUI.LoadWindowState(&Options::Menu, "legit_2.cfg");
		break;
	case 2:
		GUI.LoadWindowState(&Options::Menu, "hvh_1.cfg");
		break;
	case 3:
		GUI.LoadWindowState(&Options::Menu, "hvh_2.cfg");
		break;
	case 4:
		GUI.LoadWindowState(&Options::Menu, "casual.cfg");
		break;
	case 5:
		GUI.LoadWindowState(&Options::Menu, "faceit.cfg");
		break;

	}
}

void SaveRageCallbk()
{
	GUI.SaveWindowState(&Options::Menu, "testing.cfg");
}

void LoadRageCallbk()
{
	GUI.LoadWindowState(&Options::Menu, "testing.cfg");
}

void UnLoadCallbk()
{
	DoUnload = true;
}



void PhantomWindow::Setup()
{
	SetPosition(350, 50);
	SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	SetTitle("");

	RegisterTab(&RageBotTab);
	RegisterTab(&LegitBotTab);
	RegisterTab(&VisualsTab);
	RegisterTab(&MiscTab);

	RegisterTab(&ColorsTab);

	RECT Client = GetClientArea();
	Client.bottom -= 29;

	RageBotTab.Setup();
	LegitBotTab.Setup();
	VisualsTab.Setup();
	MiscTab.Setup();

	ColorsTab.Setup();

#pragma region Bottom Buttons

	SaveButton.SetText("Save");
	SaveButton.SetCallback(SaveLegitCallbk);
	SaveButton.SetSize(160, 110);
	SaveButton.SetPosition(25, 375);

	LoadButton.SetText("Load");
	LoadButton.SetCallback(LoadLegitCallbk);
	LoadButton.SetSize(160, 110);
	LoadButton.SetPosition(25, 400);

	MiscTab.RegisterControl(&SaveButton);
	MiscTab.RegisterControl(&LoadButton);

#pragma endregion
}

void CRageBotTab::Setup()
{
	SetTitle("A");


#pragma region Aimbot

	AimbotGroup.SetPosition(10, 25);
	AimbotGroup.SetText("Aimbot");
	AimbotGroup.SetSize(650, 413);
	AimbotGroup.AddTab(CGroupTab("Main", 1));
	AimbotGroup.AddTab(CGroupTab("Other", 2));
	RegisterControl(&AimbotGroup);



	AimbotEnable.SetFileId("aim_enable");
	AimbotGroup.PlaceLabledControl(1, "Enable", this, &AimbotEnable);

	AimbotAutoFire.SetFileId("aim_autofire");
	AimbotGroup.PlaceLabledControl(1, "AutoShoot", this, &AimbotAutoFire);

	AimbotSilentAim.SetFileId("aim_silent");
	AimbotGroup.PlaceLabledControl(1, "Silent Aim", this, &AimbotSilentAim);

	AccuracyAutoWall.SetFileId("acc_awall");
	AimbotGroup.PlaceLabledControl(1, "Auto Wall", this, &AccuracyAutoWall);

	AccuracyAutoScope.SetFileId("acc_scope");
	AimbotGroup.PlaceLabledControl(1, "Auto Scope", this, &AccuracyAutoScope);

	TargetSelection.SetFileId("tgt_selection");
	TargetSelection.AddItem("Closest To Crosshair");
	TargetSelection.AddItem("Distance");
	TargetSelection.AddItem("Lowest Health");
	TargetSelection.AddItem("Threat");
	TargetSelection.AddItem("Next Shot");
	AimbotGroup.PlaceLabledControl(1, "Selection", this, &TargetSelection);

	TargetHitbox.SetFileId("tgt_hitbox");
	TargetHitbox.AddItem("Head");
	TargetHitbox.AddItem("Neck");
	TargetHitbox.AddItem("Chest");
	TargetHitbox.AddItem("Pelvis");
	TargetHitbox.AddItem("Shin");
	AimbotGroup.PlaceLabledControl(1, "Hitbox", this, &TargetHitbox);

	TargetHitscan.SetFileId("tgt_hitscan");
	TargetHitscan.AddItem("Off");
	TargetHitscan.AddItem("Minimal");
	TargetHitscan.AddItem("Essential");
	TargetHitscan.AddItem("Maximal");
	AimbotGroup.PlaceLabledControl(1, "Hitscan", this, &TargetHitscan);

	preso.SetFileId("acc_zeusisgay");
	preso.AddItem("Default");
	preso.AddItem("Down");
	preso.AddItem("Up");
	preso.AddItem("Zero");
	preso.AddItem("Automatic Spread");
	AimbotGroup.PlaceLabledControl(1, "Pitch Adjustment", this, &preso);

	resolver.SetFileId("acc_aaa");
	resolver.AddItem("Off");
	resolver.AddItem("LewisFTM - LBY Auto");
	resolver.AddItem("FreaK - Freestanding Prediction");
	resolver.AddItem("LuckyHvH - Delta");
	resolver.AddItem("Alternative");
	AimbotGroup.PlaceLabledControl(1, "Yaw Resolver", this, &resolver);

	AimbotFov.SetFileId("aim_fov");
	AimbotFov.SetBoundaries(0.f, 180.f);
	AimbotFov.SetValue(35.f);
	AimbotGroup.PlaceLabledControl(1, "Maximum Fov", this, &AimbotFov);


	AccuracyHitchance.SetFileId("acc_AccuracyHitchance");
	AccuracyHitchance.SetBoundaries(0, 100);
	AccuracyHitchance.SetValue(20);
	AimbotGroup.PlaceLabledControl(2, "Minimum Hitchance", this, &AccuracyHitchance);

	AccuracyMinimumDamage.SetFileId("acc_AccuracyMinimumDamage");
	AccuracyMinimumDamage.SetBoundaries(0, 100);
	AccuracyMinimumDamage.SetValue(20);
	AimbotGroup.PlaceLabledControl(2, "Minimum Damage", this, &AccuracyMinimumDamage);



	BaimIfUnderXHealth.SetFileId("acc_BaimIfUnderXHealth");
	BaimIfUnderXHealth.SetBoundaries(0, 100);
	BaimIfUnderXHealth.SetValue(0);
	AimbotGroup.PlaceLabledControl(2, "Body Aim If HP Lower Than", this, &BaimIfUnderXHealth);

	baim.SetFileId("tgt_bruteforce2");
	baim.SetBoundaries(0.f, 15.f);
	baim.SetValue(4.f);
	AimbotGroup.PlaceLabledControl(2, "Body Aim After Shots", this, &baim);

	OverrideKey.SetFileId("acc_overrridekey");
	AimbotGroup.PlaceLabledControl(2, "Override Key", this, &OverrideKey);

	bigbaim.SetFileId("acc_bigbaim");
	AimbotGroup.PlaceLabledControl(2, "Force Body Aim Key", this, &bigbaim);






#pragma endregion  AntiAim controls get setup in here
}


void CLegitBotTab::Setup()
{
	SetTitle("I");

	AimbotGroup.SetText("Main");
	AimbotGroup.SetPosition(15, 25);
	AimbotGroup.SetSize(640, 222);
	AimbotGroup.AddTab(CGroupTab("Aimbot", 1));
	AimbotGroup.AddTab(CGroupTab("Triggerbot", 2));
	AimbotGroup.AddTab(CGroupTab("Filters", 3));

	RegisterControl(&AimbotGroup);
	RegisterControl(&Active);

	Active.SetFileId("active");
	AimbotGroup.PlaceLabledControl(1, "Activate Legitbot", this, &Active);

	AimbotEnable.SetFileId("l_aimbot");
	AimbotGroup.PlaceLabledControl(1, "Aimbot Enable", this, &AimbotEnable);

	aimbotfiremode.SetFileId("l_autoaimbot");
	aimbotfiremode.AddItem("On Key");
	aimbotfiremode.AddItem("Automatic");
	AimbotGroup.PlaceLabledControl(1, "Fire Mode", this, &aimbotfiremode);

	AimbotKeyBind.SetFileId("l_aimkey");
	AimbotGroup.PlaceLabledControl(1, "KeyBind", this, &AimbotKeyBind);

	BackTrack.SetFileId("l_backtrack");
	AimbotGroup.PlaceLabledControl(1, "Backtrack", this, &BackTrack);

	AimbotSmokeCheck.SetFileId("l_smokeaimbot");
	AimbotGroup.PlaceLabledControl(1, "Smoke Check", this, &AimbotSmokeCheck);

	legitresolver.SetFileId("l_B1GresolverTappingSkeet");
	AimbotGroup.PlaceLabledControl(1, "Resolver", this, &legitresolver);

	//---- Trigger ---//

	TriggerEnable.SetFileId("t_triggerbotenable");
	AimbotGroup.PlaceLabledControl(2, "Activate Trigger", this, &TriggerEnable);

	triggertype.SetFileId("t_triggerbottyp");
	triggertype.AddItem("On Key");
	triggertype.AddItem("Automatic");
	AimbotGroup.PlaceLabledControl(2, "Trigger Mode", this, &triggertype);


	TriggerHitChanceAmmount.SetFileId("l_trigHC");
	TriggerHitChanceAmmount.SetBoundaries(0, 100);
	TriggerHitChanceAmmount.SetValue(0);
	AimbotGroup.PlaceLabledControl(2, "Hitchance", this, &TriggerHitChanceAmmount);

	TriggerSmokeCheck.SetFileId("l_trigsmoke");
	AimbotGroup.PlaceLabledControl(2, "Smoke Check", this, &TriggerSmokeCheck);

	TriggerRecoil.SetFileId("l_trigRCS");
	TriggerRecoil.SetBoundaries(0.f, 200.f);
	TriggerRecoil.SetValue(0.00f);
	AimbotGroup.PlaceLabledControl(2, "Recoil", this, &TriggerRecoil);

	TriggerKeyBind.SetFileId("l_trigkey");
	AimbotGroup.PlaceLabledControl(2, "KeyBind", this, &TriggerKeyBind);


	// ---- Hitboxes ---- //

	TriggerHead.SetFileId("l_trighead");
	AimbotGroup.PlaceLabledControl(3, "Head", this, &TriggerHead);

	TriggerChest.SetFileId("l_trigchest");
	AimbotGroup.PlaceLabledControl(3, "Chest", this, &TriggerChest);

	TriggerStomach.SetFileId("l_trigstomach");
	AimbotGroup.PlaceLabledControl(3, "Stomach", this, &TriggerStomach);

	TriggerArms.SetFileId("l_trigarms");
	AimbotGroup.PlaceLabledControl(3, "Arms", this, &TriggerArms);

	TriggerLegs.SetFileId("l_triglegs");
	AimbotGroup.PlaceLabledControl(3, "Legs", this, &TriggerLegs);

	//----------------------B1g---------------------//

	weapongroup.SetText("Main");
	weapongroup.SetPosition(15, 256);
	weapongroup.SetSize(640, 183);
	weapongroup.AddTab(CGroupTab("Rifle", 1));
	weapongroup.AddTab(CGroupTab("Pistol", 2));
	weapongroup.AddTab(CGroupTab("Sniper", 3));
	weapongroup.AddTab(CGroupTab("SMG", 4));
	weapongroup.AddTab(CGroupTab("Heavy", 5));
	RegisterControl(&weapongroup);
	RegisterControl(&Active);

	WeaponMainHitbox.SetFileId("l_rhitbox");
	WeaponMainHitbox.AddItem("Head");
	WeaponMainHitbox.AddItem("Neck");
	WeaponMainHitbox.AddItem("Chest");
	WeaponMainHitbox.AddItem("Stomach");
	WeaponMainHitbox.AddItem("Nearest");
	weapongroup.PlaceLabledControl(1, "Hitbox", this, &WeaponMainHitbox);

	WeaponMainSpeed.SetFileId("l_rspeed");
	WeaponMainSpeed.SetBoundaries(0, 75);
	WeaponMainSpeed.SetValue(5);
	weapongroup.PlaceLabledControl(1, "Speed", this, &WeaponMainSpeed);

	WeaponMainRecoil.SetFileId("l_rRecoil");
	WeaponMainRecoil.SetBoundaries(0, 200);
	WeaponMainRecoil.SetValue(165);
	weapongroup.PlaceLabledControl(1, "Recoil", this, &WeaponMainRecoil);

	WeaponMainFoV.SetFileId("l_fov");
	WeaponMainFoV.SetBoundaries(0, 45);
	WeaponMainFoV.SetValue(10);
	weapongroup.PlaceLabledControl(1, "Field Of View", this, &WeaponMainFoV);


	// --- Pistols --- //

	WeaponPistHitbox.SetFileId("l_phitbox"); // pHitbox cy@media 
	WeaponPistHitbox.AddItem("Head");
	WeaponPistHitbox.AddItem("Neck");
	WeaponPistHitbox.AddItem("Chest");
	WeaponPistHitbox.AddItem("Stomach");
	WeaponPistHitbox.AddItem("Nearest");
	weapongroup.PlaceLabledControl(2, "Hitbox", this, &WeaponPistHitbox);

	WeaponPistSpeed.SetFileId("l_pspeed");
	WeaponPistSpeed.SetBoundaries(0, 75);
	WeaponPistSpeed.SetValue(5);
	weapongroup.PlaceLabledControl(2, "Speed", this, &WeaponPistSpeed);

	WeaponPistRecoil.SetFileId("l_pRecoil");
	WeaponPistRecoil.SetBoundaries(0, 200);
	WeaponPistRecoil.SetValue(165);
	weapongroup.PlaceLabledControl(2, "Recoil", this, &WeaponPistRecoil);

	WeaponPistFoV.SetFileId("l_pfov");
	WeaponPistFoV.SetBoundaries(0, 45);
	WeaponPistFoV.SetValue(10);
	weapongroup.PlaceLabledControl(2, "Field Of View", this, &WeaponPistFoV);

	// --- Sniper --- //

	WeaponSnipHitbox.SetFileId("l_shitbox");
	WeaponSnipHitbox.AddItem("Head");
	WeaponSnipHitbox.AddItem("Neck");
	WeaponSnipHitbox.AddItem("Chest");
	WeaponSnipHitbox.AddItem("Stomach");
	WeaponSnipHitbox.AddItem("Nearest");
	weapongroup.PlaceLabledControl(3, "Hitbox", this, &WeaponSnipHitbox);

	WeaponSnipSpeed.SetFileId("l_sspeed");
	WeaponSnipSpeed.SetBoundaries(0, 75);
	WeaponSnipSpeed.SetValue(5);
	weapongroup.PlaceLabledControl(3, "Speed", this, &WeaponSnipSpeed);

	WeaponSnipRecoil.SetFileId("l_sRecoil");
	WeaponSnipRecoil.SetBoundaries(0, 200);
	WeaponSnipRecoil.SetValue(165);
	weapongroup.PlaceLabledControl(3, "Recoil", this, &WeaponSnipRecoil);

	WeaponSnipFoV.SetFileId("l_sfov");
	WeaponSnipFoV.SetBoundaries(0, 45);
	WeaponSnipFoV.SetValue(10);
	weapongroup.PlaceLabledControl(3, "Field Of View", this, &WeaponSnipFoV);


	// --- SMG --- //

	WeaponMpHitbox.SetFileId("l_sniphitbox");
	WeaponMpHitbox.AddItem("Head");
	WeaponMpHitbox.AddItem("Neck");
	WeaponMpHitbox.AddItem("Chest");
	WeaponMpHitbox.AddItem("Stomach");
	WeaponMpHitbox.AddItem("Nearest");
	weapongroup.PlaceLabledControl(4, "Hitbox", this, &WeaponMpHitbox);

	WeaponMpSpeed.SetFileId("l_sspeed");
	WeaponMpSpeed.SetBoundaries(0, 75);
	WeaponMpSpeed.SetValue(5);
	weapongroup.PlaceLabledControl(4, "Speed", this, &WeaponMpSpeed);

	WeaponMpRecoil.SetFileId("l_sRecoil");
	WeaponMpRecoil.SetBoundaries(0, 200);
	WeaponMpRecoil.SetValue(165);
	weapongroup.PlaceLabledControl(4, "Recoil", this, &WeaponMpRecoil);

	WeaponMpFoV.SetFileId("l_sfov");
	WeaponMpFoV.SetBoundaries(0, 45);
	WeaponMpFoV.SetValue(10);
	weapongroup.PlaceLabledControl(4, "Field Of View", this, &WeaponMpFoV);


	// --- MachineGun --- //

	WeaponMGHitbox.SetFileId("l_mghitbox");
	WeaponMGHitbox.AddItem("Head");
	WeaponMGHitbox.AddItem("Neck");
	WeaponMGHitbox.AddItem("Chest");
	WeaponMGHitbox.AddItem("Stomach");
	WeaponMGHitbox.AddItem("Nearest");
	weapongroup.PlaceLabledControl(5, "Hitbox", this, &WeaponMGHitbox);

	WeaponMGSpeed.SetFileId("l_mgspeed");
	WeaponMGSpeed.SetBoundaries(0, 75);
	WeaponMGSpeed.SetValue(5);
	weapongroup.PlaceLabledControl(5, "Speed", this, &WeaponMGSpeed);

	WeaponMGRecoil.SetFileId("l_mgRecoil");
	WeaponMGRecoil.SetBoundaries(0, 200);
	WeaponMGRecoil.SetValue(165);
	weapongroup.PlaceLabledControl(5, "Recoil", this, &WeaponMGRecoil);

	WeaponMGFoV.SetFileId("l_mgfov");
	WeaponMGFoV.SetBoundaries(0, 45);
	WeaponMGFoV.SetValue(10);
	weapongroup.PlaceLabledControl(5, "Field Of View", this, &WeaponMGFoV);



}





void CVisualTab::Setup()
{
	SetTitle("D");



#pragma region Options
	OptionsGroup.SetText("Options");
	OptionsGroup.SetPosition(15, 30);
	OptionsGroup.SetSize(200, 390);
	OptionsGroup.AddTab(CGroupTab("Main", 1));
	OptionsGroup.AddTab(CGroupTab("Filters", 2));
	RegisterControl(&OptionsGroup);
	RegisterControl(&Active);

	Active.SetFileId("active");
	OptionsGroup.PlaceLabledControl(1, "Active Visuals", this, &Active);

	OptionsBox.SetFileId("opt_box");
	OptionsBox.AddItem("Off");
	OptionsBox.AddItem("Default");
	OptionsBox.AddItem("Genuine");
	OptionsGroup.PlaceLabledControl(1, "Box", this, &OptionsBox);

	OptionsName.SetFileId("opt_name");
	OptionsGroup.PlaceLabledControl(1, "Name", this, &OptionsName);


	OptionsHealth.SetFileId("opt_hp");
	OptionsHealth.AddItem("Off");
	OptionsHealth.AddItem("Default");
	OptionsHealth.AddItem("Battery");
	OptionsHealth.AddItem("Text");
	OptionsHealth.AddItem("Both - Default");
	OptionsHealth.AddItem("Both - Battery");
	OptionsGroup.PlaceLabledControl(1, "Health", this, &OptionsHealth);

	OptionsInfo.SetFileId("opt_info");
	OptionsGroup.PlaceLabledControl(1, "Info", this, &OptionsInfo);

	OtherHitmarker.SetFileId("otr_hitmarker");
	OptionsGroup.PlaceLabledControl(1, "Hitmarker", this, &OtherHitmarker);

	OptionsSkeleton.SetFileId("opt_bone");
	OptionsGroup.PlaceLabledControl(1, "Skeleton", this, &OptionsSkeleton);


	OptionsGlow.SetFileId("opt_glow");
	OptionsGroup.PlaceLabledControl(1, "Player Glow", this, &OptionsGlow);




	GlowZ.SetFileId("opt_glowz");
	GlowZ.SetBoundaries(0.f, 255.f);
	GlowZ.SetValue(220);
	OptionsGroup.PlaceLabledControl(1, "Glow Opacity", this, &GlowZ);

	LBYIndicator.SetFileId("opt_lbywhatever");
	OptionsGroup.PlaceLabledControl(1, "Lowerbody Indicator", this, &LBYIndicator);



	fakemedia.SetFileId("opt_penguware_fucking_sucks");
	OptionsGroup.PlaceLabledControl(1, "Fake Enemy AA", this, &fakemedia);

	cheatinfo.SetFileId("nigger");
	OptionsGroup.PlaceLabledControl(1, "Local Info", this, &cheatinfo);

	sound.SetFileId("hitmarkersound");
	sound.AddItem("Off");
	sound.AddItem("Default");
	sound.AddItem("Metalic");
	sound.AddItem("Bubble");
	sound.AddItem("Bame");
	OptionsGroup.PlaceLabledControl(1, "Hit Sound", this, &sound);

//	SpecList.SetFileId("all_eyes_on_you__now_good_luck_clutching_ehehe"); // rip fps for some reason
//	OptionsGroup.PlaceLabledControl(1, "Spectator List", this, &SpecList);


	FiltersAll.SetFileId("ftr_all");
	OptionsGroup.PlaceLabledControl(2, "All", this, &FiltersAll);

	FiltersPlayers.SetFileId("ftr_players");
	FiltersPlayers.SetState(true);
	OptionsGroup.PlaceLabledControl(2, "Players", this, &FiltersPlayers);

	FiltersEnemiesOnly.SetFileId("ftr_enemyonly");
	OptionsGroup.PlaceLabledControl(2, "Enemies only", this, &FiltersEnemiesOnly);

	FiltersWeapons.SetFileId("ftr_weaps");
	OptionsGroup.PlaceLabledControl(2, "Weapons", this, &FiltersWeapons);

	FiltersNades.SetFileId("ftr_nades");
	OptionsGroup.PlaceLabledControl(2, "Nades", this, &FiltersNades);

	FiltersC4.SetFileId("ftr_c4");
	OptionsGroup.PlaceLabledControl(2, "C4", this, &FiltersC4);

#pragma endregion Setting up the Options controls

	ChamsGroup.SetText("Model");
	ChamsGroup.SetPosition(225, 30);
	ChamsGroup.SetSize(200, 225);
	RegisterControl(&ChamsGroup);

	Chams.SetFileId("esp_chams");
	Chams.AddItem("None");
	Chams.AddItem("Default");
	Chams.AddItem("Wire");
	ChamsGroup.PlaceLabledControl(0, "Chams", this, &Chams);

	ChamsPlayers.SetFileId("esp_chamsplayers");
	ChamsGroup.PlaceLabledControl(0, "Apply Material", this, &ChamsPlayers);

	ChamsEnemyOnly.SetFileId("esp_chamsenemyonly");
	ChamsGroup.PlaceLabledControl(0, "Restrict To Enemy Team", this, &ChamsEnemyOnly); //to-do

	OtherNoHands.SetFileId("esp_handymandy");
	OtherNoHands.AddItem("Default");
	OtherNoHands.AddItem("Clear");
	OtherNoHands.AddItem("Cham");
	OtherNoHands.AddItem("Wireframe");
	OtherNoHands.AddItem("Remove");
	OtherNoHands.AddItem("LSD");
	ChamsGroup.PlaceLabledControl(0, "Hand Materials", this, &OtherNoHands);

	WeaponChams.SetFileId("esp_jews");
	WeaponChams.AddItem("Default");
	WeaponChams.AddItem("Clear");
	WeaponChams.AddItem("Cham");
	WeaponChams.AddItem("Wireframe");
	WeaponChams.AddItem("Remove");
	WeaponChams.AddItem("LSD");
	ChamsGroup.PlaceLabledControl(0, "Weapon Materials", this, &WeaponChams);

	localmaterial.SetFileId("esp_localscopedmat");
	localmaterial.AddItem("Default");
	localmaterial.AddItem("Clear");
	localmaterial.AddItem("Cham");
	localmaterial.AddItem("Wireframe");
	localmaterial.AddItem("Remove");
	localmaterial.AddItem("LSD");
	ChamsGroup.PlaceLabledControl(0, "Scoped Materials", this, &localmaterial);

	scopemat.SetFileId("esp_localscopedmat");
	scopemat.AddItem("Cham");
	scopemat.AddItem("Wireframe");
	ChamsGroup.PlaceLabledControl(0, "Angle Materials", this, &scopemat);

#pragma region WORLD IS DOOMED AND FLAT AND GAY AND TRUMP IS LITERALLY HITLER AND FREAK IS THE QUEEN OF MARS

	worldgroup.SetText("World");
	worldgroup.SetPosition(225, 265);
	worldgroup.SetSize(200, 155);
	RegisterControl(&worldgroup);


	colmodupdate.SetFileId("otr_night");
	worldgroup.PlaceLabledControl(0, "Refresh World", this, &colmodupdate);

//	customskies2.SetFileId("otr_skycustom2");
//	worldgroup.PlaceLabledControl(0, "Refresh Sky", this, &customskies2);

	customskies.SetFileId("otr_skycustom");
	customskies.AddItem("Default");
	customskies.AddItem("Night 1");
	customskies.AddItem("Night 2");
	customskies.AddItem("NoSky");
	worldgroup.PlaceLabledControl(0, "Change Sky", this, &customskies);


	colmod.SetFileId("night_amm");
	colmod.SetBoundaries(000.000f, 100.00f);
	colmod.SetValue(020.0f);
	worldgroup.PlaceLabledControl(0, "Night Mod", this, &colmod);

	asusamount.SetFileId("otr_asusprops");
	asusamount.SetBoundaries(1.f, 100.f);
	asusamount.SetValue(95.f);
	worldgroup.PlaceLabledControl(0, "Asus Amount", this, &asusamount);



#pragma region Other

	OtherGroup.SetText("Other");
	OtherGroup.SetPosition(436, 30);
	OtherGroup.SetSize(210, 390);
	RegisterControl(&OtherGroup);

	AutowallCrosshair.SetFileId("otr_autowallcross"); //to-do
	OtherGroup.PlaceLabledControl(0, "Autowall crosshair", this, &AutowallCrosshair);

	SpreadCross.SetFileId("otr_spreadcross");
	OtherGroup.PlaceLabledControl(0, "Spread crosshair", this, &SpreadCross);

	OtherThirdpersonAngle.SetFileId("aa_tpAngle");
	OtherThirdpersonAngle.AddItem("Real");
	OtherGroup.PlaceLabledControl(0, "Thirdperson angles", this, &OtherThirdpersonAngle);

	FakeAngleChams.SetFileId("otr_anglechams");
	FakeAngleChams.AddItem("Off");
	FakeAngleChams.AddItem("Fake angles");
	FakeAngleChams.AddItem("LBY angles");
	OtherGroup.PlaceLabledControl(0, "Angle chams", this, &FakeAngleChams);

	logs.SetFileId("otr_skeetpaste");
	OtherGroup.PlaceLabledControl(0, "Event Log", this, &logs);

	OtherNoVisualRecoil.SetFileId("otr_visrecoil");
	OtherGroup.PlaceLabledControl(0, "No visual recoil", this, &OtherNoVisualRecoil);

	OtherNoFlash.SetFileId("otr_noflash");
	OtherGroup.PlaceLabledControl(0, "Remove flash effect", this, &OtherNoFlash);

	flashAlpha.SetFileId("otr_stolen_from_punknown_muahahaha");
	flashAlpha.SetBoundaries(0, 255);
	flashAlpha.SetValue(10);
	OtherGroup.PlaceLabledControl(0, "Flash Alpha", this, &flashAlpha); //to-do


	OtherNoSmoke.SetFileId("otr_nosmoke");
	OtherGroup.PlaceLabledControl(0, "Remove smoke effect", this, &OtherNoSmoke); //to-do

	OtherNoScope.SetFileId("otr_noscope");
	OtherGroup.PlaceLabledControl(0, "Remove scope", this, &OtherNoScope);

	OtherViewmodelFOV.SetFileId("otr_viewfov");
	OtherViewmodelFOV.SetBoundaries(30.f, 120.f);
	OtherViewmodelFOV.SetValue(90.f);
	OtherGroup.PlaceLabledControl(0, "Viewmodel fov", this, &OtherViewmodelFOV);

	OtherFOV.SetFileId("otr_fov");
	OtherFOV.SetBoundaries(0.f, 50.f);
	OtherFOV.SetValue(0.f);
	OtherGroup.PlaceLabledControl(0, "Override fov", this, &OtherFOV);

	BulletTracers.SetFileId("otr_btracers");
	OtherGroup.PlaceLabledControl(0, "Bullet tracers", this, &BulletTracers);

	beamtime.SetFileId("otr_beamtime");
	beamtime.SetBoundaries(0.1, 10);
	beamtime.SetValue(2);
	OtherGroup.PlaceLabledControl(0, "Tracer Time", this, &beamtime);



	manualaa.SetFileId("manualaa");
	OtherGroup.PlaceLabledControl(0, "Manual Indicator", this, &manualaa);

	zoom.SetFileId("zoom");
	OtherGroup.PlaceLabledControl(0, "Zoom", this, &zoom);


#pragma endregion Setting up the Other controls
}

void CMiscTab::Setup()
{
	SetTitle("G");

#pragma region Other

	OtherGroup.SetPosition(10, 30);
	OtherGroup.SetSize(320, 400);
	OtherGroup.SetText("Other");
	RegisterControl(&OtherGroup);

	OtherSafeMode.SetFileId("otr_safemode");
	OtherSafeMode.AddItem("MatchMaking");
	OtherSafeMode.AddItem("Anti VAC Kick");
	OtherSafeMode.AddItem("FaceIt Server");
	OtherGroup.PlaceLabledControl(0, "Safety Mode", this, &OtherSafeMode);

	OtherAutoJump.SetFileId("otr_autojump");
	OtherGroup.PlaceLabledControl(0, "BunnyHop", this, &OtherAutoJump);

	OtherAutoStrafe.SetFileId("otr_strafe");
	OtherAutoStrafe.AddItem("Off");
	OtherAutoStrafe.AddItem("Legit");
	OtherAutoStrafe.AddItem("Rage");
	OtherGroup.PlaceLabledControl(0, "Auto strafer", this, &OtherAutoStrafe);


	/*	ClanTag.SetFileId("otr_clantag");
	ClanTag.AddItem("Off");
	ClanTag.AddItem("Default");
	ClanTag.AddItem("Genuine");
	ClanTag.AddItem("Testing[]");
	ClanTag.AddItem("Interwebz");
	ClanTag.AddItem("Cat Hack");
	ClanTag.AddItem("Emily AA");
	OtherGroup.PlaceLabledControl(0, "Clantag", this, &ClanTag);  */

	left2.SetFileId("aa_l2");
	OtherGroup.PlaceLabledControl(0, "Left Hand On Knife", this, &left2);

	OtherThirdperson.SetFileId("aa_1thirpbind");
	OtherGroup.PlaceLabledControl(0, "Thirdperson", this, &OtherThirdperson);

	ThirdPersonKeyBind.SetFileId("aa_thirpbind");
	OtherGroup.PlaceLabledControl(0, "Thirdperson key", this, &ThirdPersonKeyBind);

	FakePingValue.SetFileId("otr_fakepingspike");
	OtherGroup.PlaceLabledControl(0, "Ping Spike", this, &FakePingValue);

	namechange.SetFileId("otr_namechange");
	namechange.AddItem("Off");
	namechange.AddItem("Steal");
	namechange.AddItem("Spam");
	namechange.AddItem("Hide");
	OtherGroup.PlaceLabledControl(0, "Name Changer", this, &namechange);

	ConfigBox.SetFileId("cfg_box");
	ConfigBox.AddItem("legit 1");
	ConfigBox.AddItem("legit 2");
	ConfigBox.AddItem("hvh 1");
	ConfigBox.AddItem("hvh 2");
	ConfigBox.AddItem("casual");
	ConfigBox.AddItem("faceit");
	OtherGroup.PlaceLabledControl(0, "Config", this, &ConfigBox);



	AntiAimGroup.SetPosition(346, 30);
	AntiAimGroup.SetText("Anti Aim");
	AntiAimGroup.SetSize(320, 400);
	AntiAimGroup.AddTab(CGroupTab("Main", 1));
	AntiAimGroup.AddTab(CGroupTab("Builder 1", 2));
	AntiAimGroup.AddTab(CGroupTab("Builder 2", 3));
	AntiAimGroup.AddTab(CGroupTab("Misc", 4));
	RegisterControl(&AntiAimGroup);

	AntiAimEnable.SetFileId("aa_enable");
	AntiAimGroup.PlaceLabledControl(1, "Enable", this, &AntiAimEnable);

	AntiAimPitch.SetFileId("aa_x");
	AntiAimPitch.AddItem("Off");
	AntiAimPitch.AddItem("Down");
	AntiAimPitch.AddItem("Up");
	AntiAimPitch.AddItem("Jitter");
	AntiAimPitch.AddItem("Random");
	AntiAimGroup.PlaceLabledControl(1, "Pitch", this, &AntiAimPitch);

	AntiAimYaw.SetFileId("aa_y");
	AntiAimYaw.AddItem("Off");
	AntiAimYaw.AddItem("Static");
	AntiAimYaw.AddItem("Twitch");
	AntiAimYaw.AddItem("Jitter");
	AntiAimYaw.AddItem("180 Z");
	AntiAimYaw.AddItem("Random Lowerbody");
	AntiAimYaw.AddItem("Manual Static");
	AntiAimYaw.AddItem("Manual Twitch");
	AntiAimYaw.AddItem("Lowerbody 180");
	AntiAimYaw.AddItem("Lowerbody Classic");
	AntiAimYaw.AddItem("Lowerbody Magic");
	AntiAimGroup.PlaceLabledControl(1, "Real_Stand", this, &AntiAimYaw);


	AntiAimYawrun.SetFileId("aa_y2");
	AntiAimYawrun.AddItem("Off");
	AntiAimYawrun.AddItem("Static");
	AntiAimYawrun.AddItem("Twitch");
	AntiAimYawrun.AddItem("Jitter");
	AntiAimYawrun.AddItem("180 Z");
	AntiAimYawrun.AddItem("Random Lowerbody");
	AntiAimYawrun.AddItem("Manual Static");
	AntiAimYawrun.AddItem("Manual Twitch");
	AntiAimGroup.PlaceLabledControl(1, "Real_Move", this, &AntiAimYawrun);

	AntiAimYaw3.SetFileId("aa_y3");
	AntiAimYaw3.AddItem("Off");
	AntiAimYaw3.AddItem("Static");
	AntiAimYaw3.AddItem("Twitch");
	AntiAimYaw3.AddItem("Jitter");
	AntiAimYaw3.AddItem("180 Z");
	AntiAimYaw3.AddItem("Random Lowerbody");
	AntiAimYaw3.AddItem("Manual Static");
	AntiAimYaw3.AddItem("Manual Twitch");

	AntiAimGroup.PlaceLabledControl(1, "Real_Air", this, &AntiAimYaw3);

	FakeYaw.SetFileId("fakeaa");
	FakeYaw.AddItem("Off");
	FakeYaw.AddItem("Static");
	FakeYaw.AddItem("Lowerbody");
	FakeYaw.AddItem("Lowerbody Twitch");
	FakeYaw.AddItem("Inverse Twitch");
	FakeYaw.AddItem("Random Lowerbody");
	FakeYaw.AddItem("Half Spin");
	FakeYaw.AddItem("180 Autist");
	FakeYaw.AddItem("Magic");
	AntiAimGroup.PlaceLabledControl(1, "Fake_Stand", this, &FakeYaw);

	FakeYaw2.SetFileId("fakeaa2");
	FakeYaw2.AddItem("Off");
	FakeYaw2.AddItem("Static");
	FakeYaw2.AddItem("Local View");
	FakeYaw2.AddItem("Inverse");
	FakeYaw2.AddItem("Inverse Twitch");
	FakeYaw2.AddItem("Random Lowerbody");
	FakeYaw2.AddItem("Half Spin");
	FakeYaw2.AddItem("LBY Spin");
	FakeYaw2.AddItem("LBY Magic");
	AntiAimGroup.PlaceLabledControl(1, "Fake_Move", this, &FakeYaw2);


	FakeYaw3.SetFileId("fakeaaaa"); // aaaaaaaaaaaaaaaaaaa sp00ky
	FakeYaw3.AddItem("Off");
	FakeYaw3.AddItem("Inverse");
	FakeYaw3.AddItem("Local View");
	FakeYaw3.AddItem("Random Lowerbody");
	FakeYaw3.AddItem("Half Spin");
	AntiAimGroup.PlaceLabledControl(1, "Fake_Air", this, &FakeYaw3);


	//----- 

	staticr.SetFileId("b_st1");
	staticr.SetBoundaries(0, 360);
	staticr.SetValue(180);
	AntiAimGroup.PlaceLabledControl(2, "Real Static", this, &staticr);

	twitchr.SetFileId("b_twitch1");
	twitchr.SetBoundaries(0, 60);
	twitchr.SetValue(35);
	AntiAimGroup.PlaceLabledControl(2, "Real Twitch Range", this, &twitchr);

	randlbyr.SetFileId("b_randlbyr");
	randlbyr.SetBoundaries(20, 120);
	randlbyr.SetValue(90);
	AntiAimGroup.PlaceLabledControl(2, "Real Random LBY Range", this, &randlbyr);

	staticf.SetFileId("b_st2");
	staticf.SetBoundaries(0, 360);
	staticf.SetValue(90);
	AntiAimGroup.PlaceLabledControl(2, "Fake Static", this, &staticf);

	twitchf.SetFileId("b_twitch2");
	twitchf.SetBoundaries(0, 90);
	twitchf.SetValue(40);
	AntiAimGroup.PlaceLabledControl(2, "Fake Twitch Range", this, &twitchf);

	spinf.SetFileId("b_spinf");
	spinf.SetBoundaries(1, 10);
	spinf.SetValue(5);
	AntiAimGroup.PlaceLabledControl(2, "Fake Spinbot Speed", this, &spinf);


	randlbyf.SetFileId("b_randlbyf");
	randlbyf.SetBoundaries(20, 120);
	randlbyf.SetValue(90);
	AntiAimGroup.PlaceLabledControl(2, "Fake Random LBY Range", this, &randlbyf);


	FakeLagChoke.SetFileId("fakelag_choke");
	FakeLagChoke.SetBoundaries(0, 14);
	FakeLagChoke.SetValue(7);
	AntiAimGroup.PlaceLabledControl(2, "FakeLag_Moving", this, &FakeLagChoke);

	FakeLagChoke2.SetFileId("fakelag_choke2");
	FakeLagChoke2.SetBoundaries(0, 14);
	FakeLagChoke2.SetValue(7);
	AntiAimGroup.PlaceLabledControl(2, "FakeLag_Air", this, &FakeLagChoke2);





	//-----

	antilby.SetFileId("otr_meh");
	AntiAimGroup.PlaceLabledControl(3, "Anti-LBY", this, &antilby);

	BreakLBYDelta.SetFileId("b_antilby");
	BreakLBYDelta.SetBoundaries(35, 180);
	BreakLBYDelta.SetValue(90);
	AntiAimGroup.PlaceLabledControl(3, "Anti-LBY Range", this, &BreakLBYDelta);







	manualleft.SetFileId("otr_keybasedleft");
	AntiAimGroup.PlaceLabledControl(4, "Manual_Right", this, &manualleft);

	manualright.SetFileId("otr_keybasedright");
	AntiAimGroup.PlaceLabledControl(4, "Manual_Left", this, &manualright);

	manualback.SetFileId("otr_keybasedback");
	AntiAimGroup.PlaceLabledControl(4, "Manual_Back", this, &manualback);

	manualfront.SetFileId("otr_keybasedfwd");
	AntiAimGroup.PlaceLabledControl(4, "Manual_Front", this, &manualfront);


	fw.SetFileId("fakewalk_key");
	AntiAimGroup.PlaceLabledControl(4, "FakeWalk Key", this, &fw);

	FakeWalk.SetFileId("fakewalk_key2");
	AntiAimGroup.PlaceLabledControl(4, "GlitchWalk Key", this, &FakeWalk);



}

void CColorTab::Setup()
{
	SetTitle("H");
#pragma region Visual Colors

	ColorsGroup.SetText("Entity");
	ColorsGroup.SetPosition(16, 30);
	ColorsGroup.AddTab(CGroupTab("ESP", 1));
	ColorsGroup.AddTab(CGroupTab("Materials 1", 2));
	ColorsGroup.AddTab(CGroupTab("Materials 2", 3));
	ColorsGroup.SetSize(314, 390);
	RegisterControl(&ColorsGroup);


	BoxR.SetFileId("BoxRed");
	BoxR.SetBoundaries(0.f, 255.f);
	BoxR.SetValue(250.f);
	ColorsGroup.PlaceLabledControl(1, "Bounding box red", this, &BoxR);

	BoxG.SetFileId("BoxGreen");
	BoxG.SetBoundaries(0.f, 255.f);
	BoxG.SetValue(250.f);
	ColorsGroup.PlaceLabledControl(1, "Bounding box green", this, &BoxG);

	BoxB.SetFileId("BoxBlue");
	BoxB.SetBoundaries(0.f, 255.f);
	BoxB.SetValue(250.f);
	ColorsGroup.PlaceLabledControl(1, "Bounding box blue", this, &BoxB);



	GlowR.SetFileId("GlowR");
	GlowR.SetBoundaries(0.f, 255.f);
	GlowR.SetValue(250.f);
	ColorsGroup.PlaceLabledControl(1, "Glow red", this, &GlowR);

	GlowG.SetFileId("GlowG");
	GlowG.SetBoundaries(0.f, 255.f);
	GlowG.SetValue(250.f);
	ColorsGroup.PlaceLabledControl(1, "Glow green", this, &GlowG);

	GlowB.SetFileId("GlowB");
	GlowB.SetBoundaries(0.f, 255.f);
	GlowB.SetValue(250.f);
	ColorsGroup.PlaceLabledControl(1, "Glow blue", this, &GlowB);



	ChamsVisRed.SetFileId("chamsvisred");
	ChamsVisRed.SetBoundaries(0.f, 255.f);
	ChamsVisRed.SetValue(5.f);
	ColorsGroup.PlaceLabledControl(2, "Chams visible red", this, &ChamsVisRed);

	ChamsVisGreen.SetFileId("chamsvisgreen");
	ChamsVisGreen.SetBoundaries(0.f, 255.f);
	ChamsVisGreen.SetValue(185.f);
	ColorsGroup.PlaceLabledControl(2, "Chams visible green", this, &ChamsVisGreen);

	ChamsVisBlue.SetFileId("chamsvisblue");
	ChamsVisBlue.SetBoundaries(0.f, 255.f);
	ChamsVisBlue.SetValue(255.f);
	ColorsGroup.PlaceLabledControl(2, "Chams visible blue", this, &ChamsVisBlue);



	ChamsNotVisRed.SetFileId("chamsnotvisred");
	ChamsNotVisRed.SetBoundaries(0.f, 255.f);
	ChamsNotVisRed.SetValue(140.f);
	ColorsGroup.PlaceLabledControl(2, "Chams not visible red", this, &ChamsNotVisRed);

	ChamsNotVisGreen.SetFileId("chamsnotvisgreen");
	ChamsNotVisGreen.SetBoundaries(0.f, 255.f);
	ChamsNotVisGreen.SetValue(5.f);
	ColorsGroup.PlaceLabledControl(2, "Chams not visible green", this, &ChamsNotVisGreen);

	ChamsNotVisBlue.SetFileId("chamsnotvisblue");
	ChamsNotVisBlue.SetBoundaries(0.f, 255.f);
	ChamsNotVisBlue.SetValue(235.f);
	ColorsGroup.PlaceLabledControl(2, "Chams not visible blue", this, &ChamsNotVisBlue);

	//----------------------------------------------------------------------------//

	hand_r.SetFileId("hand_r");
	hand_r.SetBoundaries(0.f, 255.f);
	hand_r.SetValue(200.f);
	ColorsGroup.PlaceLabledControl(2, "Hand material: Red", this, &hand_r);

	hand_g.SetFileId("hand_g");
	hand_g.SetBoundaries(0.f, 255.f);
	hand_g.SetValue(200.f);
	ColorsGroup.PlaceLabledControl(2, "Hand material: Green", this, &hand_g);

	hand_b.SetFileId("hand_b");
	hand_b.SetBoundaries(0.f, 255.f);
	hand_b.SetValue(200.f);
	ColorsGroup.PlaceLabledControl(2, "Hand material: Blue", this, &hand_b);

	//----------------------------------------------------------------------------//

	gun_r.SetFileId("gun_r");
	gun_r.SetBoundaries(0.f, 255.f);
	gun_r.SetValue(200.f);
	ColorsGroup.PlaceLabledControl(3, "Gun material: Red", this, &gun_r);

	gun_g.SetFileId("gun_g");
	gun_g.SetBoundaries(0.f, 255.f);
	gun_g.SetValue(200.f);
	ColorsGroup.PlaceLabledControl(3, "Gun material: Green", this, &gun_g);

	gun_b.SetFileId("gun_b");
	gun_b.SetBoundaries(0.f, 255.f);
	gun_b.SetValue(200.f);
	ColorsGroup.PlaceLabledControl(3, "Gun material: Blue", this, &gun_b);

	//----------------------------------------------------------------------------//

	scope_r.SetFileId("scope_r");
	scope_r.SetBoundaries(0.f, 255.f);
	scope_r.SetValue(160.f);
	ColorsGroup.PlaceLabledControl(3, "Scoped material: Red", this, &scope_r);

	scope_g.SetFileId("scope_g");
	scope_g.SetBoundaries(0.f, 255.f);
	scope_g.SetValue(25.f);
	ColorsGroup.PlaceLabledControl(3, "Scoped material: Green", this, &scope_g);

	scope_b.SetFileId("scope_b");
	scope_b.SetBoundaries(0.f, 255.f);
	scope_b.SetValue(250.f);
	ColorsGroup.PlaceLabledControl(3, "Scoped material: Blue", this, &scope_b);



	//----------------------------------------------------------------------------//


	beamcol.SetText("Bullet Tracer");
	beamcol.SetPosition(340, 30);
	beamcol.SetSize(310, 120);
	RegisterControl(&beamcol);


	beamplayer_r.SetFileId("beamplayerred");
	beamplayer_r.SetBoundaries(0.f, 255.f);
	beamplayer_r.SetValue(180.f);
	beamcol.PlaceLabledControl(0, "Player: Red", this, &beamplayer_r);

	beamplayer_g.SetFileId("beamplayergreen");
	beamplayer_g.SetBoundaries(0.f, 255.f);
	beamplayer_g.SetValue(5.f);
	beamcol.PlaceLabledControl(0, "Player: Green", this, &beamplayer_g);

	beamplayer_b.SetFileId("beamplayerblue");
	beamplayer_b.SetBoundaries(0.f, 255.f);
	beamplayer_b.SetValue(255.f);
	beamcol.PlaceLabledControl(0, "Player: Blue", this, &beamplayer_b);

	//---------//	
	gcol.SetText("Interface");
	gcol.SetPosition(340, 160);
	gcol.AddTab(CGroupTab("Bar Left", 1));
	gcol.AddTab(CGroupTab("Bar Right", 2));
	gcol.AddTab(CGroupTab("Controls", 3));
	gcol.SetSize(310, 260);
	RegisterControl(&gcol);


	outl_r.SetFileId("outlred");
	outl_r.SetBoundaries(0.f, 255.f);
	outl_r.SetValue(150.f);
	gcol.PlaceLabledControl(1, "Outer: Red", this, &outl_r);

	outl_g.SetFileId("outlgreen");
	outl_g.SetBoundaries(0.f, 255.f);
	outl_g.SetValue(150.f);
	gcol.PlaceLabledControl(1, "Outer: Green", this, &outl_g);

	outl_b.SetFileId("outlblue");
	outl_b.SetBoundaries(0.f, 255.f);
	outl_b.SetValue(150.f);
	gcol.PlaceLabledControl(1, "Outer: Blue", this, &outl_b);



	inl_r.SetFileId("inlred");
	inl_r.SetBoundaries(0.f, 255.f);
	inl_r.SetValue(250.f);
	gcol.PlaceLabledControl(1, "Inner: Red", this, &inl_r);

	inl_g.SetFileId("inlgreen");
	inl_g.SetBoundaries(0.f, 255.f);
	inl_g.SetValue(250.f);
	gcol.PlaceLabledControl(1, "Inner: Green", this, &inl_g);

	inl_b.SetFileId("inlblue");
	inl_b.SetBoundaries(0.f, 255.f);
	inl_b.SetValue(250.f);
	gcol.PlaceLabledControl(1, "Inner: Blue", this, &inl_b);

	//--//


	inr_r.SetFileId("inrred");
	inr_r.SetBoundaries(0.f, 255.f);
	inr_r.SetValue(250.f);
	gcol.PlaceLabledControl(2, "Inner: Red", this, &inr_r);

	inr_g.SetFileId("inrgreen");
	inr_g.SetBoundaries(0.f, 255.f);
	inr_g.SetValue(250.f);
	gcol.PlaceLabledControl(2, "Inner: Green", this, &inr_g);

	inr_b.SetFileId("inrblue");
	inr_b.SetBoundaries(0.f, 255.f);
	inr_b.SetValue(250.f);
	gcol.PlaceLabledControl(2, "Inner: Blue", this, &inr_b);



	outr_r.SetFileId("outrred");
	outr_r.SetBoundaries(0.f, 255.f);
	outr_r.SetValue(150.f);
	gcol.PlaceLabledControl(2, "Outer: Red", this, &outr_r);

	outr_g.SetFileId("outrgreen");
	outr_g.SetBoundaries(0.f, 255.f);
	outr_g.SetValue(150.f);
	gcol.PlaceLabledControl(2, "Outer: Green", this, &outr_g);

	outr_b.SetFileId("outrblue");
	outr_b.SetBoundaries(0.f, 255.f);
	outr_b.SetValue(150.f);
	gcol.PlaceLabledControl(2, "Outer: Blue", this, &outr_b);





	cr.SetFileId("c_red");
	cr.SetBoundaries(10.f, 255.f);
	cr.SetValue(250.f);
	gcol.PlaceLabledControl(3, "Red", this, &cr);

	cg.SetFileId("c_green");
	cg.SetBoundaries(10.f, 255.f);
	cg.SetValue(250.f);
	gcol.PlaceLabledControl(3, "Green", this, &cg);

	cb.SetFileId("c_blue");
	cb.SetBoundaries(10.f, 255.f);
	cb.SetValue(250.f);
	gcol.PlaceLabledControl(3, "Blue", this, &cb);


#pragma endregion

#pragma region GloveModel




#pragma endregion
}



//_______________________________________________//
                  //---------//	
				  //---------//	
				  //---------//	
				  //---------//	
				  //---------//	
				  //---------//	
				  //---------//	
				  //---------//	
				  //---------//	
				  //---------//	
				  //---------//	
				  //---------//	
				  //---------//	
				  //---------//	
				  //---------//	
//_______________________________________________//
/*Whoever can climb this ladder is not fat*/

void Options::SetupMenu()
{
	Menu.Setup();

	GUI.RegisterWindow(&Menu);
	GUI.BindWindow(VK_INSERT, &Menu);
}

void Options::DoUIFrame()
{
	if (Menu.VisualsTab.FiltersAll.GetState())
	{
		Menu.VisualsTab.FiltersC4.SetState(true);
		Menu.VisualsTab.FiltersPlayers.SetState(true);
		Menu.VisualsTab.FiltersWeapons.SetState(true);
		Menu.VisualsTab.FiltersNades.SetState(true);
	}

	GUI.Update();
	GUI.Draw();

}
