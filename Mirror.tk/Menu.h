#pragma once

#include "GUI.h"
#include "Controls.h"

class CRageBotTab : public CTab
{
public:
	void Setup();

	CLabel ActiveLabel;
	CCheckBox Active;


	CComboBox psilent;

	CGroupBox AimbotGroup;
	CCheckBox AimbotEnable;
	CComboBox lag_pred;
	CCheckBox AimbotAutoFire;
	CSlider  AimbotFov;
	CCheckBox AimbotSilentAim;
	CCheckBox PVSFix;
	CComboBox AimbotResolver;
	CCheckBox AdvancedResolver;
	CCheckBox AutoRevolver;
	CSlider AddFakeYaw;
	CSlider CustomPitch;
	CCheckBox LBY;


	//-------------------------

	//--------------------------

	CCheckBox LowerbodyFix;
	CCheckBox PreferBodyAim;
	CCheckBox AWPAtBody;
	CSlider BaimIfUnderXHealth;
	CSlider XShots;
	CCheckBox EnginePrediction;
	CCheckBox AimbotAimStep;

	CSlider AimbotAimStep2;

	CSlider shotlimit;

	CCheckBox AimbotKeyPress;


	CKeyBind  AimbotKeyBind;
	CKeyBind  AimbotStopKey;

	CGroupBox TargetGroup;
	CComboBox TargetSelection;
	CCheckBox TargetFriendlyFire;
	CComboBox TargetHitbox;
	CComboBox TargetHitscan;
	CCheckBox TargetMultipoint;
	CSlider   TargetPointscale;

	CSlider   bruteX;
	CSlider   baim;

	CGroupBox AccuracyGroup;
	CCheckBox AccuracyRecoil;
	CCheckBox AccuracyAutoWall;
	CSlider AccuracyMinimumDamage;
	CCheckBox AccuracyAutoStop;
	CCheckBox AccuracyAutoCrouch;
	CCheckBox AccuracyAutoScope;
	CSlider   AccuracyHitchance;
	CComboBox resolver;
	CComboBox preso;
	CKeyBind KeyOverride;

	CCheckBox LBYCorrection;
	CCheckBox Override;
	CSlider OverrideCount;
	CKeyBind OverrideKey;
	CComboBox OverrideMode;

	CKeyBind bigbaim;
	CCheckBox test_awall;
	CSlider YawRunningAdd;
	CSlider YawAdd;
	CSlider FakeYawAdd;
	CSlider FreestandingDelta;
	CCheckBox Freestand;
	CCheckBox FlipAA;
	CCheckBox AntiAimEdge;
	CSlider AntiAimOffset;
	CCheckBox ManualAAEnable;
	CKeyBind ManualAASwitch;
	CKeyBind ManualLeft;
	CKeyBind ManualRight;
	CKeyBind ManualBack;
	CComboBox LBYbreaker;
	CKeyBind Lbybreakerkey;
	CCheckBox BreakLBY;
	CSlider BreakLBYDelta;
	CCheckBox AntiAimKnife;
	CCheckBox AntiAimTarget;




};

class CLegitBotTab : public CTab
{
public:
	void Setup();

	// Master Switch
	CLabel ActiveLabel;
	CCheckBox Active;

	CGroupBox AimbotGroup;




	CCheckBox AimbotEnable;
	CComboBox aimbotfiremode;
	CCheckBox AimbotFriendlyFire;
	CCheckBox AimbotKeyPress;
	CKeyBind  AimbotKeyBind;
	CCheckBox AimbotSmokeCheck;
	CCheckBox BackTrack;

	CCheckBox legitresolver;

	CGroupBox TriggerGroup;
	CComboBox triggertype;
	CCheckBox TriggerEnable;

	CKeyBind  TriggerKeyBind;

	CSlider   TriggerDelay;
	CSlider   TriggerBurst;
	CSlider   TriggerBreak;
	CSlider TriggerRecoil; CSlider   TriggerHitChanceAmmount;

	CGroupBox TriggerFilterGroup;
	CCheckBox TriggerHead;
	CCheckBox TriggerChest;
	CCheckBox TriggerStomach;
	CCheckBox TriggerArms;
	CCheckBox TriggerLegs;
	CCheckBox TriggerTeammates;
	CCheckBox TriggerFlashCheck;
	CCheckBox TriggerSmokeCheck;

	CCheckBox aaenable;
	CComboBox aatyp;

	CSlider aimhp;
	CSlider trighp;

	CGroupBox weapongroup;
	CSlider   WeaponMainSpeed;
	CSlider   WeaponMainFoV;
	CSlider WeaponMainRecoil;
	CComboBox WeaponMainHitbox;
	CSlider WeaponMainAimtime;
	CSlider WeaoponMainStartAimtime;

	CGroupBox WeaponPistGroup;
	CSlider   WeaponPistSpeed;
	CSlider   WeaponPistFoV;
	CSlider WeaponPistRecoil;
	CComboBox WeaponPistHitbox;
	CSlider WeaponPistAimtime;
	CSlider WeaoponPistStartAimtime;

	CGroupBox WeaponSnipGroup;
	CSlider   WeaponSnipSpeed;
	CSlider   WeaponSnipFoV;
	CSlider WeaponSnipRecoil;
	CComboBox WeaponSnipHitbox;
	CSlider WeaponSnipAimtime;
	CSlider WeaoponSnipStartAimtime;

	CGroupBox WeaponMpGroup;
	CSlider   WeaponMpSpeed;
	CSlider   WeaponMpFoV;
	CSlider WeaponMpRecoil;
	CComboBox WeaponMpHitbox;
	CSlider WeaponMpAimtime;
	CSlider WeaoponMpStartAimtime;

	CGroupBox WeaponShotgunGroup;
	CSlider   WeaponShotgunSpeed;
	CSlider   WeaponShotgunFoV;
	CSlider WeaponShotgunRecoil;
	CComboBox WeaponShotgunHitbox;
	CSlider WeaponShotgunAimtime;
	CSlider WeaoponShotgunStartAimtime;

	CGroupBox WeaponMGGroup;
	CSlider   WeaponMGSpeed;
	CSlider   WeaponMGFoV;
	CSlider WeaponMGRecoil;
	CComboBox WeaponMGHitbox;
	CSlider WeaponMGAimtime;
	CSlider WeaoponMGStartAimtime;
};

class CVisualTab : public CTab
{
public:
	void Setup();

	CLabel ActiveLabel;
	CCheckBox Active;

	CCheckBox logs;

	CSlider flashAlpha;
	CGroupBox OptionsGroup;
	CCheckBox OptionsWeapone;
	CComboBox OptionsBox;
	CCheckBox OptionsName;
	CCheckBox HitmarkerSound;
	CCheckBox OtherHitmarker;
	CComboBox OptionsHealth;
	CComboBox OptionsWeapon;

	CCheckBox armorbar;

	CCheckBox manualaa;
	CKeyBind zoom;
	CCheckBox OptionsInfo;
	CCheckBox BulletTrace;
	CCheckBox Barrels;
	CCheckBox ChamsVisibleOnly;
	CCheckBox Ammo;
	CCheckBox Clock;
	CCheckBox HostageGlow;
	CCheckBox Watermark;
	CCheckBox LBYIndicator;
	CCheckBox GrenadePrediction;
	CCheckBox LocalPlayerESP;
	CCheckBox CanHit;
	CCheckBox OptionsHelmet;
	CCheckBox NightMode;
	CCheckBox OptionsKit;
	CCheckBox SpecList;
	CCheckBox IsScoped;
	CCheckBox CompRank;
	CCheckBox HasDefuser;
	CCheckBox GrenadeTrace;
	CCheckBox Distance;
	CCheckBox DiscoLights;
	CCheckBox Money;
	CCheckBox OptionsDefuse;
	CComboBox OptionsChams;
	CCheckBox OptionsArmor;
	CCheckBox OptionsDefusing;
	CCheckBox SniperCrosshair;
	CSlider  GlowZ;
	CComboBox OtherThirdpersonAngle;
	CComboBox FakeAngleChams;
	CCheckBox OptionsSkeleton;
	CCheckBox NightSky;

	CCheckBox hitbone;

	CCheckBox OptionsAimSpot;
	CComboBox WeaponChams;
	CCheckBox OtherNoScope;
	CCheckBox OptionsCompRank;

	CCheckBox resoinfo;
	CCheckBox fakemedia;
	CCheckBox fakemedia2;

	CCheckBox selfglow;

	CGroupBox FiltersGroup;
	CCheckBox FiltersAll;
	CCheckBox FiltersPlayers;
	CCheckBox FiltersEnemiesOnly;
	CCheckBox FiltersWeapons;
	CCheckBox FiltersChickens;
	CCheckBox FiltersNades;
	CCheckBox FiltersC4;

	CGroupBox ChamsGroup;
	CCheckBox ChamsThruWalls;
	CComboBox Chams;
	CCheckBox ChamsPlayers;
	CCheckBox ChamsEnemyOnly;


	CGroupBox OtherGroup;
	CComboBox OtherCrosshair;
	CComboBox OtherRecoilCrosshair;
	CCheckBox AutowallCrosshair;
	CCheckBox OptionsGlow;
	CCheckBox EntityGlow;
	CCheckBox DisablePostProcess;
	CCheckBox OtherRadar;
	CCheckBox OtherNoVisualRecoil;
	CCheckBox OtherNoSky;
	CCheckBox OtherNoFlash;
	CCheckBox OtherNoSmoke;
	CCheckBox OtherAsusWalls;
	CComboBox OtherNoHands;
	CCheckBox AAIndicators;
	CCheckBox BulletTracers;

	CComboBox localmaterial;

	CComboBox scopemat;


	CSlider asusamount;
	CComboBox asustype;

	CSlider beamtime;
	CSlider beamsize;

	CCheckBox cheatinfo;
	CComboBox sound;
	CCheckBox SpreadCross;
	CCheckBox DamageIndicator;
	CSlider OtherViewmodelFOV;
	CSlider OtherFOV;


	CGroupBox worldgroup;
	CCheckBox colmodupdate;
	CSlider colmod;

	CCheckBox customskies2;
	CComboBox customskies;
};

class CColorTab : public CTab
{
public:
	void Setup();

	CGroupBox boxgroup;
	CSlider boxw;
	CSlider boxh;

	CGroupBox ColorsGroup;
	CSlider BoxR;
	CSlider BoxG;
	CSlider BoxB;
	CSlider GlowR;
	CSlider GlowG;
	CSlider GlowB;
	CSlider ChamsVisRed;
	CSlider ChamsVisGreen;
	CSlider ChamsVisBlue;
	CSlider ChamsNotVisRed;
	CSlider ChamsNotVisGreen;
	CSlider ChamsNotVisBlue;

	CGroupBox beamcol;
	CSlider beamplayer_r;
	CSlider beamplayer_g;
	CSlider beamplayer_b;

	CSlider scope_r;
	CSlider scope_g;
	CSlider scope_b;


	CSlider hand_r;
	CSlider hand_g;
	CSlider hand_b;

	CSlider gun_r;
	CSlider gun_g;
	CSlider gun_b;

	CGroupBox gcol;
	CSlider outl_r;
	CSlider outl_g;
	CSlider outl_b;

	CSlider inl_r;
	CSlider inl_g;
	CSlider inl_b;

	CSlider inr_r;
	CSlider inr_g;
	CSlider inr_b;


	CSlider outr_r;
	CSlider outr_g;
	CSlider outr_b;

	CSlider cr;
	CSlider cg;
	CSlider cb;


	CGroupBox Gloves;
	CCheckBox EnableGloves;
	CComboBox GloveSkin;
	CComboBox GloveModel;
	CButton GlovesApply;


};

class CMiscTab : public CTab
{
public:
	void Setup();

	CGroupBox OtherGroup;
	CCheckBox OtherAutoJump;
	CCheckBox OtherEdgeJump;
	CComboBox OtherAutoStrafe;
	CComboBox OtherSafeMode;
	CCheckBox ChatSpam;
	CKeyBind OtherCircleButton;
	CCheckBox OtherCircleStrafe;
	CKeyBind OtherCircle;
	CTextField CustomClantag;
	CCheckBox CheatsByPass;
	CCheckBox AutoAccept;

	CComboBox namechange;
	CCheckBox left2;
	CSlider CircleAmount;
	CKeyBind OtherSlowMotion;
	CKeyBind FakeWalk;
	CComboBox FakeLagTyp;

	CCheckBox autofw;
	CKeyBind fw;

	CComboBox FakeYaw;
	CComboBox FakeYaw2;
	CComboBox FakeYaw3;

	CKeyBind manualleft;
	CKeyBind manualright;
	CKeyBind manualback;
	CKeyBind manualfront;

	CGroupBox AntiAimGroup;
	CCheckBox AntiAimEnable;
	CComboBox AntiAimPitch;
	CComboBox AntiAimYaw;

	CSlider staticr;
	CSlider staticf;
	CComboBox AntiAimYaw3;
	CComboBox AntiAimYawrun;
	CCheckBox OtherTeamChat;
	CSlider ClanTagSpeed;
	CSlider	  OtherChatDelay;
	CComboBox NameChanger;
	CCheckBox AutoPistol;
	CComboBox ClanTag;
	CCheckBox TrashTalk;
	CKeyBind  OtherAirStuck;
	CCheckBox OtherSpectators;
	CCheckBox OtherThirdperson;
	CComboBox OtherClantag;
	CCheckBox EnableBuyBot;
	CComboBox BuyBot;
	CCheckBox AutoDefuse;
	CCheckBox FakePingExploit;
	CKeyBind FakePingValue;
	CCheckBox TestExploit;
	CGroupBox BuyBotGroup;
	CKeyBind ThirdPersonKeyBind;
	CSlider twitchr; // 0-90
	CSlider twitchf;
	CSlider spinf; // 5-40
	CSlider randlbyr; // 20 - 120
	CSlider randlbyf;
	CSlider lby1;
	CCheckBox squaredance;
	CCheckBox antilby;
	CSlider BreakLBYDelta;
	CGroupBox FakeLagGroup;
	CCheckBox FakeLagEnable;
	CSlider   FakeLagChoke;
	CCheckBox FakelagOnground;

	CComboBox antilby2;


	CSlider FakeLagChoke2;
	CKeyBind lagkey;

	CComboBox inverse_type;

	CComboBox ConfigBox;
	CGroupBox ConfigGroup;


	CGroupBox LegitAAs;
	CComboBox legitaa;
};


class Cskintab : public CTab
{
public:
	void Setup();
	CLabel SkinActive;
	CCheckBox SkinEnable;
	

	// Knife
	CGroupBox KnifeGroup;

	CGroupBox applygrp;
	CButton   SkinApply;
	CComboBox KnifeModel;
	CComboBox KnifeSkin;

	CTextField knifeskin2;

	CGroupBox PistolGroup;
	CComboBox GLOCKSkin;
	CComboBox USPSSkin;
	CComboBox DEAGLESkin;
	CComboBox MAGNUMSkin;
	CComboBox DUALSSkin;
	CComboBox FIVESEVENSkin;

	CComboBox P2000Skin;
	CComboBox P250Skin;
	CComboBox CZSkin;

	// MPs


	// Rifles
	CGroupBox Riflegroup;
	CComboBox M41SSkin;
	CComboBox M4A4Skin;
	CComboBox AK47Skin;
	CComboBox AUGSkin;
	CComboBox FAMASSkin;
	CComboBox GALILSkin;
	CComboBox SG553Skin;


	// Machineguns
	CGroupBox MachinegunsGroup;
	CComboBox NEGEVSkin;
	CComboBox M249Skin;

	CGroupBox MPGroup;
	CComboBox MAC10Skin;
	CComboBox P90Skin;
	CComboBox UMP45Skin;
	CComboBox BIZONSkin;
	CComboBox MP7Skin;
	CComboBox MP9Skin;

	// Snipers
	CGroupBox Snipergroup;
	CComboBox SCAR20Skin;
	CComboBox G3SG1Skin;
	CComboBox SSG08Skin;
	CComboBox r8skin;
	CComboBox AWPSkin;

	// Shotguns
	CGroupBox Shotgungroup;
	CComboBox MAG7Skin;
	CComboBox XM1014Skin;
	CComboBox SAWEDOFFSkin;
	CComboBox NOVASkin;

	// Skinsettings
	CGroupBox SkinsettingsGroup;
	CCheckBox StatTrakEnable;

};



class PhantomWindow : public CWindow
{
public:
	void Setup();

	CRageBotTab RageBotTab;
	CLegitBotTab LegitBotTab;
	CVisualTab VisualsTab;
	CMiscTab MiscTab;
	Cskintab skintab;
	CColorTab ColorsTab;



	CButton   SkinApply;
	CButton SaveButton1;
	CButton LoadButton1;
	CButton SaveButton;
	CButton LoadButton;
	CButton UnloadButton;
	CButton PanicButton;
	CButton ThirdButton;
	CButton ByPass;
	CButton FirstButton;
	CComboBox ConfigBox;
};



namespace Options
{
	void SetupMenu();
	void DoUIFrame();

	extern PhantomWindow Menu;
};