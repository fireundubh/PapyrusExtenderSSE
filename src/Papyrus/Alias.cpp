#include "Papyrus/Alias.h"


void papyrusAlias::RegisterForActorKilled(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnActorKillRegSet::GetSingleton();
	regs->Register(a_alias);
}


void papyrusAlias::RegisterForBookRead(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnBooksReadRegSet::GetSingleton();
	regs->Register(a_alias);
}


void papyrusAlias::RegisterForCellFullyLoaded(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = ScriptEvents::OnCellFullyLoadedRegSet::GetSingleton();
	regs->Register(a_alias);
}


void papyrusAlias::RegisterForCriticalHit(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnCriticalHitRegSet::GetSingleton();
	regs->Register(a_alias);
}


void papyrusAlias::RegisterForDisarmed(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnDisarmedRegSet::GetSingleton();
	regs->Register(a_alias);
}


void papyrusAlias::RegisterForDragonSoulGained(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnDragonSoulsGainedRegSet::GetSingleton();
	regs->Register(a_alias);
}


void papyrusAlias::RegisterForItemHarvested(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnItemHarvestedRegSet::GetSingleton();
	regs->Register(a_alias);
}


void papyrusAlias::RegisterForLevelIncrease(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnLevelIncreaseRegSet::GetSingleton();
	regs->Register(a_alias);
}


void papyrusAlias::RegisterForLocationDiscovery(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnLocationDiscoveryRegSet::GetSingleton();
	regs->Register(a_alias);
}


void papyrusAlias::RegisterForObjectGrab(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto grab = ScriptEvents::OnGrabRegSet::GetSingleton();
	grab->Register(a_alias);

	auto release = ScriptEvents::OnReleaseRegSet::GetSingleton();
	release->Register(a_alias);
}


void papyrusAlias::RegisterForObjectLoaded(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias, std::uint32_t a_formType)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto formType = static_cast<RE::FormType>(a_formType);

	auto load = ScriptEvents::OnObjectLoadedRegMap::GetSingleton();
	load->Register(a_alias, formType);

	auto unload = ScriptEvents::OnObjectUnloadedRegMap::GetSingleton();
	unload->Register(a_alias, formType);
}


void papyrusAlias::RegisterForQuest(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias, RE::TESQuest* a_quest)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	} else if (!a_quest) {
		a_vm->TraceStack("Quest is None", a_stackID, Severity::kWarning);
		return;
	}

	auto start = ScriptEvents::OnQuestStartRegMap::GetSingleton();
	start->Register(a_alias, a_quest->GetFormID());

	auto stop = ScriptEvents::OnQuestStopRegMap::GetSingleton();
	stop->Register(a_alias, a_quest->GetFormID());
}


void papyrusAlias::RegisterForQuestStage(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias, RE::TESQuest* a_quest)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	} else if (!a_quest) {
		a_vm->TraceStack("Quest is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = ScriptEvents::OnQuestStageRegMap::GetSingleton();
	regs->Register(a_alias, a_quest->GetFormID());
}


void papyrusAlias::RegisterForShoutAttack(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnShoutAttackRegSet::GetSingleton();
	regs->Register(a_alias);
}


void papyrusAlias::RegisterForSkillIncrease(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnSkillIncreaseRegSet::GetSingleton();
	regs->Register(a_alias);
}


void papyrusAlias::RegisterForSoulTrapped(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnSoulsTrappedRegSet::GetSingleton();
	regs->Register(a_alias);
}

void papyrusAlias::RegisterForSpellLearned(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnSpellsLearnedRegSet::GetSingleton();
	regs->Register(a_alias);
}


void papyrusAlias::UnregisterForActorKilled(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnActorKillRegSet::GetSingleton();
	regs->Unregister(a_alias);
}


void papyrusAlias::UnregisterForBookRead(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnBooksReadRegSet::GetSingleton();
	regs->Unregister(a_alias);
}


void papyrusAlias::UnregisterForCellFullyLoaded(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = ScriptEvents::OnCellFullyLoadedRegSet::GetSingleton();
	regs->Unregister(a_alias);
}


void papyrusAlias::UnregisterForCriticalHit(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnCriticalHitRegSet::GetSingleton();
	regs->Unregister(a_alias);
}


void papyrusAlias::UnregisterForDisarmed(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnDisarmedRegSet::GetSingleton();
	regs->Unregister(a_alias);
}


void papyrusAlias::UnregisterForDragonSoulGained(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnDragonSoulsGainedRegSet::GetSingleton();
	regs->Unregister(a_alias);
}


void papyrusAlias::UnregisterForItemHarvested(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnItemHarvestedRegSet::GetSingleton();
	regs->Unregister(a_alias);
}


void papyrusAlias::UnregisterForLevelIncrease(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnLevelIncreaseRegSet::GetSingleton();
	regs->Unregister(a_alias);
}


void papyrusAlias::UnregisterForLocationDiscovery(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnLocationDiscoveryRegSet::GetSingleton();
	regs->Unregister(a_alias);
}


void papyrusAlias::UnregisterForObjectGrab(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto grab = ScriptEvents::OnGrabRegSet::GetSingleton();
	grab->Unregister(a_alias);

	auto release = ScriptEvents::OnReleaseRegSet::GetSingleton();
	release->Unregister(a_alias);
}


void papyrusAlias::UnregisterForObjectLoaded(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias, std::uint32_t a_formType)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto formType = static_cast<RE::FormType>(a_formType);

	auto load = ScriptEvents::OnObjectLoadedRegMap::GetSingleton();
	load->Unregister(a_alias, formType);

	auto unload = ScriptEvents::OnObjectUnloadedRegMap::GetSingleton();
	unload->Unregister(a_alias, formType);
}


void papyrusAlias::UnregisterForAllObjectsLoaded(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto load = ScriptEvents::OnObjectLoadedRegMap::GetSingleton();
	load->UnregisterAll(a_alias);

	auto unload = ScriptEvents::OnObjectUnloadedRegMap::GetSingleton();
	unload->UnregisterAll(a_alias);
}


void papyrusAlias::UnregisterForQuest(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias, RE::TESQuest* a_quest)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	} else if (!a_quest) {
		a_vm->TraceStack("Quest is None", a_stackID, Severity::kWarning);
		return;
	}

	auto start = ScriptEvents::OnQuestStartRegMap::GetSingleton();
	start->Unregister(a_alias, a_quest->GetFormID());

	auto stop = ScriptEvents::OnQuestStartRegMap::GetSingleton();
	stop->Unregister(a_alias, a_quest->GetFormID());
}


void papyrusAlias::UnregisterForAllQuests(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto start = ScriptEvents::OnQuestStartRegMap::GetSingleton();
	start->UnregisterAll(a_alias);

	auto stop = ScriptEvents::OnQuestStartRegMap::GetSingleton();
	stop->UnregisterAll(a_alias);
}


void papyrusAlias::UnregisterForQuestStage(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias, RE::TESQuest* a_quest)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	} else if (!a_quest) {
		a_vm->TraceStack("Quest is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = ScriptEvents::OnQuestStageRegMap::GetSingleton();
	regs->Unregister(a_alias, a_quest->GetFormID());
}


void papyrusAlias::UnregisterForAllQuestStages(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = ScriptEvents::OnQuestStageRegMap::GetSingleton();
	regs->UnregisterAll(a_alias);
}


void papyrusAlias::UnregisterForShoutAttack(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnShoutAttackRegSet::GetSingleton();
	regs->Unregister(a_alias);
}


void papyrusAlias::UnregisterForSkillIncrease(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnShoutAttackRegSet::GetSingleton();
	regs->Unregister(a_alias);
}


void papyrusAlias::UnregisterForSoulTrapped(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnSoulsTrappedRegSet::GetSingleton();
	regs->Unregister(a_alias);
}


void papyrusAlias::UnregisterForSpellLearned(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, const RE::BGSBaseAlias* a_alias)
{
	if (!a_alias) {
		a_vm->TraceStack("Alias is None", a_stackID, Severity::kWarning);
		return;
	}

	auto regs = StoryEvents::OnSpellsLearnedRegSet::GetSingleton();
	regs->Unregister(a_alias);
}


bool papyrusAlias::RegisterFuncs(VM* a_vm)
{
	if (!a_vm) {
		logger::critical("papyrusAlias - couldn't get VMState");
		return false;
	}

	a_vm->RegisterFunction("RegisterForActorKilled", "PO3_Events_Alias", RegisterForActorKilled, true);

	a_vm->RegisterFunction("RegisterForBookRead", "PO3_Events_Alias", RegisterForBookRead, true);

	a_vm->RegisterFunction("RegisterForCellFullyLoaded", "PO3_Events_Alias", RegisterForCellFullyLoaded, true);

	a_vm->RegisterFunction("RegisterForCriticalHit", "PO3_Events_Alias", RegisterForCriticalHit, true);

	a_vm->RegisterFunction("RegisterForDisarmed", "PO3_Events_Alias", RegisterForDisarmed, true);

	a_vm->RegisterFunction("RegisterForDragonSoulGained", "PO3_Events_Alias", RegisterForDragonSoulGained, true);

	a_vm->RegisterFunction("RegisterForItemHarvested", "PO3_Events_Alias", RegisterForItemHarvested, true);

	a_vm->RegisterFunction("RegisterForLevelIncrease", "PO3_Events_Alias", RegisterForLevelIncrease, true);

	a_vm->RegisterFunction("RegisterForLocationDiscovery", "PO3_Events_Alias", RegisterForLocationDiscovery, true);

	a_vm->RegisterFunction("RegisterForObjectGrab", "PO3_Events_Alias", RegisterForObjectGrab, true);

	a_vm->RegisterFunction("RegisterForObjectLoaded", "PO3_Events_Alias", RegisterForObjectLoaded, true);

	a_vm->RegisterFunction("RegisterForQuest", "PO3_Events_Alias", RegisterForQuest, true);

	a_vm->RegisterFunction("RegisterForQuestStage", "PO3_Events_Alias", RegisterForQuestStage, true);

	a_vm->RegisterFunction("RegisterForShoutAttack", "PO3_Events_Alias", RegisterForShoutAttack, true);

	a_vm->RegisterFunction("RegisterForSkillIncrease", "PO3_Events_Alias", RegisterForSkillIncrease, true);

	a_vm->RegisterFunction("RegisterForSoulTrapped", "PO3_Events_Alias", RegisterForSoulTrapped, true);

	a_vm->RegisterFunction("RegisterForSpellLearned", "PO3_Events_Alias", RegisterForSpellLearned, true);


	a_vm->RegisterFunction("UnregisterForActorKilled", "PO3_Events_Alias", UnregisterForActorKilled, true);

	a_vm->RegisterFunction("UnregisterForBookRead", "PO3_Events_Alias", UnregisterForBookRead, true);

	a_vm->RegisterFunction("UnregisterForCellFullyLoaded", "PO3_Events_Alias", UnregisterForCellFullyLoaded, true);

	a_vm->RegisterFunction("UnregisterForCriticalHit", "PO3_Events_Alias", UnregisterForCriticalHit, true);

	a_vm->RegisterFunction("UnregisterForDisarmed", "PO3_Events_Alias", UnregisterForDisarmed, true);

	a_vm->RegisterFunction("UnregisterForDragonSoulGained", "PO3_Events_Alias", UnregisterForDragonSoulGained, true);

	a_vm->RegisterFunction("UnregisterForItemHarvested", "PO3_Events_Alias", UnregisterForItemHarvested, true);

	a_vm->RegisterFunction("UnregisterForLevelIncrease", "PO3_Events_Alias", UnregisterForLevelIncrease, true);

	a_vm->RegisterFunction("UnregisterForLocationDiscovery", "PO3_Events_Alias", UnregisterForLocationDiscovery, true);

	a_vm->RegisterFunction("UnregisterForObjectGrab", "PO3_Events_Alias", UnregisterForObjectGrab, true);

	a_vm->RegisterFunction("UnregisterForObjectLoaded", "PO3_Events_Alias", UnregisterForObjectLoaded, true);

	a_vm->RegisterFunction("UnregisterForAllObjectsLoaded", "PO3_Events_Alias", UnregisterForAllObjectsLoaded, true);

	a_vm->RegisterFunction("UnregisterForQuest", "PO3_Events_Alias", UnregisterForQuest, true);

	a_vm->RegisterFunction("UnregisterForAllQuests", "PO3_Events_Alias", UnregisterForAllQuests, true);

	a_vm->RegisterFunction("UnregisterForQuestStage", "PO3_Events_Alias", UnregisterForQuestStage, true);

	a_vm->RegisterFunction("UnregisterForAllQuestStages", "PO3_Events_Alias", UnregisterForAllQuestStages, true);

	a_vm->RegisterFunction("UnregisterForShoutAttack", "PO3_Events_Alias", UnregisterForShoutAttack, true);

	a_vm->RegisterFunction("UnregisterForSkillIncrease", "PO3_Events_Alias", UnregisterForSkillIncrease, true);

	a_vm->RegisterFunction("UnregisterForSoulTrapped", "PO3_Events_Alias", UnregisterForSoulTrapped, true);

	a_vm->RegisterFunction("UnregisterForSpellLearned", "PO3_Events_Alias", UnregisterForSpellLearned, true);

	return true;
}