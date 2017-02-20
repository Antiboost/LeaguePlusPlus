#include "PluginSDK.h"
#include "SimpleLib.h"

PluginSetup("Graves");

IMenu* MainMenu;

IMenu* ComboMenu;
IMenuOption* ComboQ;
IMenuOption* ComboQRange;
IMenuOption* ComboW;
IMenuOption* ComboE;

IMenu* HarassMenu;
IMenuOption* HarassQ;
IMenuOption* HarassQRange;
IMenuOption* HarassW;
IMenuOption* HarassE;
IMenuOption* HarassMana;

IMenu* JungleMenu;
IMenuOption* JungleE;
IMenuOption* JungleMana;

IMenu* UltMenu;
IMenuOption* UltOnKey;
IMenuOption* KSWithUlt;
// IMenuOption* KSWithUltToggle;

IMenu* MiscMenu;
IMenuOption* WOnCC;
IMenuOption* WOnGapclose;
IMenuOption* KSWithW;
// IMenuOption* EOnGapclose;

void InitializeMenu()
{
	MainMenu = GPluginSDK->AddMenu("Graves");
	ComboMenu = MainMenu->AddMenu("Combo Settngs");
	{
		ComboQ = ComboMenu->CheckBox("Use Q", true);
		ComboQRange = ComboMenu->AddInteger("Q Range", 0, 808, 750);
		ComboW = ComboMenu->CheckBox("Use W", true);
		ComboE = ComboMenu->CheckBox("Use E", true);
	}

	HarassMenu = MainMenu->AddMenu("Harass Settings");
	{
		HarassQ = HarassMenu->CheckBox("Use Q", false);
		HarassQRange = HarassMenu->AddInteger("Q Range", 0, 808, 750);
		HarassW = HarassMenu->CheckBox("Use W", false);
		HarassE = HarassMenu->CheckBox("Use E", true);
		HarassMana = HarassMenu->AddInteger("Mana Percent for Harass", 1, 100, 20);
	}

	JungleMenu = MainMenu->AddMenu("Jungle Settings");
	{
		JungleE = JungleMenu->CheckBox("Use E", true);
		JungleMana = JungleMenu->AddInteger("Mana Percent for Jungle Clear", 1, 100, 20);
	}

	UltMenu = MainMenu->AddMenu("Ult Settings");
	{
		UltOnKey = UltMenu->AddKey("Ult on Key", 'T');
		KSWithUlt = UltMenu->CheckBox("KS with R", true);
		//	KSWithUltToggle
	}

	MiscMenu = MainMenu->AddMenu("Misc Settings");
	{
		WOnCC = MiscMenu->CheckBox("Cast W on Crowd Controled Targets", true);
		WOnGapclose = MiscMenu->CheckBox("Cast W on Gap Closers", true);
		KSWithW = MiscMenu->CheckBox("KS with W", true);
		// EOnGapclose = MiscMenu->CheckBox("Cast E on Gap Closers");
	}
}

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;

void InitializeSpells()
{
	Q = SimpleLib::SimpleLib::LoadSkillshot('Q', 0.25, 808, 3000, 40, kLineCast, true, true, static_cast<eCollisionFlags>(kCollidesWithWalls | kCollidesWithYasuoWall));
	W = GPluginSDK->CreateSpell2(kSlotW, kCircleCast, false, true, static_cast<eCollisionFlags>(kCollidesWithNothing));
	E = GPluginSDK->CreateSpell2(kSlotE, kTargetCast, false, false, kCollidesWithNothing);
	R = SimpleLib::SimpleLib::LoadSkillshot('R', 0.25, 1100, 2100, 100, kLineCast, true, false, static_cast<eCollisionFlags>(kCollidesWithYasuoWall));
}

IUnit* myHero;

void Combo()
{
	if (ComboQ->Enabled())
	{
		if (Q->IsReady())
		{
			auto target = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, ComboQRange->GetInteger());
			Q->CastOnTarget(target, kHitChanceHigh);
		}
	}

	if (ComboW->Enabled())
	{
		if (W->IsReady())
		{
			auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, W->Range());
			W->CastOnTarget(target);
		}
	}
}

void Mixed()
{
	if (myHero->ManaPercent() > HarassMana->GetInteger())

		if (HarassQ->Enabled())
			if (Q->IsReady())
			{
				auto target = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, HarassQRange->GetInteger());
				Q->CastOnTarget(target, kHitChanceHigh);
			}

	if (HarassW->Enabled())
		if (W->IsReady())
		{
			auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, W->Range());
			W->CastOnTarget(target);
		}
}


void UltLogic()
{
	for (auto enemy : GEntityList->GetAllHeros(false, true))
		if (KSWithUlt->Enabled() && R->IsReady())
		{
			if (enemy->IsEnemy(GEntityList->Player()) && GEntityList->Player()->IsValidTarget(enemy, R->Range()))
			{
				auto BaseDamage = ((GEntityList->Player()->GetSpellLevel(kSlotR) - 1) * 150) + 215;
				auto BonusDamage = (1.5 * GEntityList->Player()->BonusDamage());
				auto UltDamage = BaseDamage + BonusDamage;

				if (enemy->GetHealth() < UltDamage)
				{
					R->CastOnTarget(enemy);
				}

			}
		}
}


void Misc()
{
	if (W->IsReady() && WOnCC->Enabled())
	{
		auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, W->Range());
		if (target != nullptr)
		{
			if (target->HasBuffOfType(BUFF_Stun) || target->HasBuffOfType(BUFF_Snare) || target->HasBuffOfType(BUFF_Fear) || target->HasBuffOfType(BUFF_Knockup) || target->HasBuffOfType(BUFF_Knockback) || target->HasBuffOfType(BUFF_Charm) || target->HasBuffOfType(BUFF_Taunt) || target->HasBuffOfType(BUFF_Suppression))
			{
				W->CastOnPosition(target->ServerPosition());
			}
		}
	}

	if (W->IsReady() && KSWithW->Enabled())
	{
		for (auto enemy : GEntityList->GetAllHeros(false, true))
		{
			if (enemy == nullptr || !enemy->IsValidTarget(myHero, W->Range()) || enemy->IsInvulnerable() || enemy->IsDead())
				return;
			auto damage = GHealthPrediction->GetKSDamage(enemy, kSlotW, W->GetDelay(), false);
			if (damage > enemy->GetHealth())
				W->CastOnTarget(enemy);
		}
	}
}

PLUGIN_EVENT(void) OnGameUpdate()
{
	Misc();
	UltLogic();
	switch (GOrbwalking->GetOrbwalkingMode())
	{
	case kModeCombo:
		Combo();
		break;
	case kModeMixed:
		Mixed();
		break;
	default:;
	}

	if (GetAsyncKeyState(UltOnKey->GetInteger()) && R->IsReady())
	{
		auto target = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, R->Range());
		if (target != nullptr && myHero->IsValidTarget(target, R->Range()))
		{
			R->CastOnTarget(target);
		}
	}

}

PLUGIN_EVENT(void) OnGapcloser(GapCloserSpell const& args)
{
	if (args.Sender->IsEnemy(myHero) && args.Sender->IsHero())
	{
		if (WOnGapclose->Enabled() && W->IsReady() && !args.IsTargeted && SimpleLib::SimpleLib::GetDistanceVectors(myHero->GetPosition(), args.EndPosition) < W->Range())
		{
			W->CastOnTarget(args.Sender);
		}
	}
}

PLUGIN_EVENT(void) OnAttack(IUnit* source, IUnit* target)
{
	if (source != myHero || target == nullptr)
		return;

	switch (GOrbwalking->GetOrbwalkingMode())
	{
	case kModeCombo:
		if (ComboE->Enabled() && E->IsReady())
		{
			E->CastOnPosition(GGame->CursorPosition());
		}
		break;
	case kModeMixed:
		if (HarassE->Enabled() && E->IsReady() && (myHero->ManaPercent() > HarassMana->GetInteger()))
		{
			E->CastOnPosition(GGame->CursorPosition());
		}
		break;
	case kModeLaneClear:
		if (JungleE->Enabled() && E->IsReady() && target->IsJungleCreep() && (myHero->ManaPercent() > JungleMana->GetInteger()))
		{
			E->CastOnPosition(GGame->CursorPosition());
		}
		break;
	default:;
	}
}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	InitializeMenu();
	InitializeSpells();
	myHero = GEntityList->Player();

	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOrbwalkOnAttack, OnAttack);
	GEventManager->AddEventHandler(kEventOnGapCloser, OnGapcloser);
	// GEventManager->AddEventHandler(kEventOnRender, OnRender);
}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnGapCloser, OnGapcloser);
	GEventManager->RemoveEventHandler(kEventOnGapCloser, OnAttack);
}

