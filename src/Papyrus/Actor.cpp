#include "Papyrus/Actor.h"

#include "Serialization/Form/Perks.h"
#include "Util/GraphicsReset.h"
#include "Util/VMErrors.h"


bool papyrusActor::AddBasePerk(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BGSPerk* a_perk)
{
	using namespace Serialization;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return false;
	} else if (!a_perk) {
		a_vm->TraceStack("Perk is None", a_stackID, Severity::kWarning);
		return false;
	}

	auto perks = Form::Perks::GetSingleton();
	return perks->PapyrusApply(a_actor, a_perk, Form::kAdd);
}


bool papyrusActor::AddBaseSpell(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::SpellItem* a_spell)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return false;
	} else if (!a_spell) {
		a_vm->TraceStack("Spell is None", a_stackID, Severity::kWarning);
		return false;
	} else if (a_actor->HasSpell(a_spell)) {
		a_vm->TraceStack(VMError::generic_error(a_actor, "already has spell").c_str(), a_stackID, Severity::kWarning);
		return false;
	}

	auto actorbase = a_actor->GetActorBase();
	if (actorbase) {
		auto actorEffects = actorbase->GetOrCreateSpellList();
		if (actorEffects && actorEffects->AddSpell(a_spell)) {
			auto combatController = a_actor->combatController;
			if (combatController) {
				combatController->data10->unk1C4 = 1;
			}
			if (!actorbase->IsPlayer() || a_spell->GetSpellType() != RE::MagicSystem::SpellType::kLeveledSpell) {
				return true;
			}
			RE::SpellsLearned::SendEvent(a_spell);
			return true;
		}
	}

	return false;
}


std::vector<RE::TESForm*> papyrusActor::AddAllEquippedItemsToArray(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	std::vector<RE::TESForm*> vec;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return vec;
	}

	auto inv = a_actor->GetInventory();
	for (auto& item : inv) {
		auto& [count, entry] = item.second;
		if (count > 0 && entry && entry->GetWorn()) {
			vec.push_back(item.first);
		}
	}

	return vec;
}


void TintFace(RE::Actor* a_actor, const RE::NiColor& a_color)
{
	using HeadPartType = RE::BGSHeadPart::HeadPartType;

	auto object = a_actor->GetHeadPartObject(HeadPartType::kFace);
	if (object) {
		auto geometry = object->AsGeometry();
		if (geometry) {
			geometry->SwitchToFaceTint();
			geometry->UpdateBodyTint(a_color);
		}
	}
}


void AddOrUpdateColorData(RE::NiAVObject* a_root, const RE::BSFixedString& a_name, const RE::NiColor& a_color)
{
	auto data = a_root->GetExtraData<RE::NiIntegerExtraData>(a_name);
	if (!data) {
		auto newData = RE::NiIntegerExtraData::Create(a_name, RE::NiColor::ColorToInt(a_color));
		if (newData) {
			a_root->AddExtraData(newData);
		}
	} else {
		auto color = RE::NiColor(data->value);
		if (a_color != color) {
			data->value = RE::NiColor::ColorToInt(a_color);
		}
	}
}


void papyrusActor::BlendColorWithSkinTone(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BGSColorForm* a_color, std::uint32_t a_blendMode, bool a_autoCalc, float a_opacity)
{
	using BLEND_MODE = RE::NiColor::BLEND_MODE;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return;
	} else if (!a_color) {
		a_vm->TraceStack("Colorform is None", a_stackID, Severity::kWarning);
		return;
	}

	auto actorbase = a_actor->GetActorBase();
	if (actorbase) {
		auto root = a_actor->Get3D(0);
		if (!root) {
			a_vm->TraceStack(VMError::no_3D(a_actor).c_str(), a_stackID, Severity::kWarning);
			return;
		}

		const float opacity = a_autoCalc ? std::clamp(a_opacity * RE::NiColor::CalcLuminance(actorbase->bodyTintColor), 0.0f, 1.0f) : a_opacity;
		auto newColor = RE::NiColor::Blend(actorbase->bodyTintColor, a_color->color, static_cast<BLEND_MODE>(a_blendMode), opacity);

		auto task = SKSE::GetTaskInterface();
		task->AddTask([a_actor, a_color, newColor, root]() {
			TintFace(a_actor, newColor);
			root->UpdateBodyTint(newColor);
		});

		AddOrUpdateColorData(root, "PO3_SKINTINT", newColor);
	}
}


void papyrusActor::DecapitateActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
	}

	a_actor->Decapitate();
}


void papyrusActor::EquipArmorIfSkinVisible(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::TESObjectARMO* a_check, RE::TESObjectARMO* a_equip)
{
	using Feature = RE::BSShaderMaterial::Feature;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return;
	} else if (!a_check) {
		a_vm->TraceStack("ArmorToCheck is None", a_stackID, Severity::kWarning);
		return;
	} else if (!a_equip) {
		a_vm->TraceStack("ArmorToEquip is None", a_stackID, Severity::kWarning);
		return;
	} else if (!a_actor->Is3DLoaded()) {
		a_vm->TraceStack(VMError::no_3D(a_actor).c_str(), a_stackID, Severity::kWarning);
		return;
	}

	auto task = SKSE::GetTaskInterface();
	task->AddTask([a_actor, a_equip, a_check]() {
		for (const auto& arma : a_check->armorAddons) {
			if (arma) {
				auto armorObject = a_actor->VisitArmorAddon(a_check, arma);
				if (armorObject && armorObject->HasShaderType(Feature::kFaceGenRGBTint)) {
					a_actor->AddWornItem(a_equip, 1, false, 0, 0);
					break;
				}
			}
		}
	});
}


void papyrusActor::FreezeActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, std::uint32_t a_type, bool a_enable)
{
	using Flags = RE::CHARACTER_FLAGS;
	using BOOL_BITS = RE::Actor::BOOL_BITS;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return;
	}

	if (a_type == 0) {
		if (!a_enable) {
			a_actor->boolBits.set(BOOL_BITS::kProcessMe);  //enable AI first
		}
		auto charController = a_actor->GetCharController();
		if (charController) {
			if (a_enable) {
				charController->flags.reset(Flags::kRecordHits);
				charController->flags.set(Flags::kHitDamage);
				charController->flags.set(Flags::kHitFlags);
				charController->SetLinearVelocityImpl(RE::hkVector4());
			} else {
				charController->flags.set(Flags::kRecordHits);
				charController->flags.reset(Flags::kHitDamage);
				charController->flags.reset(Flags::kHitFlags);
			}
		}
		if (a_enable) {
			a_actor->boolBits.reset(BOOL_BITS::kProcessMe);	 // disable AI last
		}
	} else {
		auto task = SKSE::GetTaskInterface();
		task->AddTask([a_actor, a_enable]() {
			auto root = a_actor->Get3D(0);
			auto charController = a_actor->GetCharController();
			if (root && charController) {
				if (a_enable) {
					a_actor->boolBits.set(BOOL_BITS::kParalyzed);
				} else {
					a_actor->boolBits.reset(BOOL_BITS::kParalyzed);
				}
				std::uint32_t unk = 0;
				auto flags = *(charController->Unk_08(&unk));
				root->UpdateRigidBodySettings(32, a_enable ? flags + 1 : flags >> 16);
				root->SetStiffSpringConstraints(a_enable ? true : false);
			}
		});
	}
}


std::vector<RE::EffectSetting*> papyrusActor::GetActiveEffects(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, bool a_inactive)
{
	using MGEF = RE::EffectSetting::EffectSettingData::Flag;
	using AE = RE::ActiveEffect::Flag;

	std::vector<RE::EffectSetting*> vec;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return vec;
	}

	auto activeEffects = a_actor->GetActiveEffectList();
	if (!activeEffects) {
		a_vm->TraceStack(VMError::generic_error(a_actor, "has no active effects").c_str(), a_stackID, Severity::kInfo);
		return vec;
	}

	for (auto& activeEffect : *activeEffects) {
		if (activeEffect) {
			auto mgef = activeEffect->GetBaseObject();
			if (mgef) {
				if (!a_inactive && (activeEffect->flags.all(AE::kInactive) || activeEffect->flags.all(AE::kDispelled) ||
									   mgef->data.flags.all(MGEF::kHideInUI))) {
					continue;
				}
				vec.push_back(mgef);
			}
		}
	}

	return vec;
}


float papyrusActor::GetActorAlpha(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return 1.0f;
	}

	auto currentProcess = a_actor->currentProcess;
	if (currentProcess) {
		auto middleProcess = currentProcess->middleHigh;
		if (middleProcess) {
			return middleProcess->alphaMult;
		}
	}

	return 1.0f;
}


float papyrusActor::GetActorRefraction(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return 1.0f;
	}

	auto currentProcess = a_actor->currentProcess;
	if (currentProcess) {
		auto middleProcess = currentProcess->middleHigh;
		if (middleProcess) {
			return middleProcess->scriptRefractPower;
		}
	}

	return 1.0f;
}


std::int32_t papyrusActor::GetActorState(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return -1;
	}

	return to_underlying(a_actor->GetLifeState());
}


std::uint32_t papyrusActor::GetCriticalStage(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return 0;
	}

	return to_underlying(a_actor->criticalStage.get());
}


std::vector<RE::Actor*> papyrusActor::GetCombatAllies(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	std::vector<RE::Actor*> vec;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return vec;
	}

	auto combatGroup = a_actor->GetCombatGroup();
	if (!combatGroup) {
		a_vm->TraceStack(VMError::generic_error(a_actor, "does not have a combat group").c_str(), a_stackID, Severity::kWarning);
		return vec;
	}

	for (auto& allyData : combatGroup->allies) {
		auto allyPtr = allyData.allyHandle.get();
		auto ally = allyPtr.get();
		if (ally) {
			vec.push_back(ally);
		}
	}

	return vec;
}


std::vector<RE::Actor*> papyrusActor::GetCombatTargets(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	std::vector<RE::Actor*> vec;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return vec;
	}

	auto combatGroup = a_actor->GetCombatGroup();
	if (!combatGroup) {
		a_vm->TraceStack(VMError::generic_error(a_actor, "does not have a combat group").c_str(), a_stackID, Severity::kWarning);
		return vec;
	}

	for (auto& targetData : combatGroup->targets) {
		auto targetPtr = targetData.targetHandle.get();
		auto target = targetPtr.get();
		if (target) {
			vec.push_back(target);
		}
	}

	return vec;
}


std::vector<std::int32_t> papyrusActor::GetDeathEffectType(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, std::uint32_t a_type)
{
	using FLAG = RE::EffectSetting::EffectSettingData::Flag;
	namespace FLOAT = SKSE::UTIL::FLOAT;

	std::vector<std::int32_t> vec;
	vec.resize(3, -1);

	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return vec;
	}
	auto activeEffects = a_actor->GetActiveEffectList();
	if (!activeEffects) {
		a_vm->TraceStack(VMError::generic_error(a_actor, "has no active effects").c_str(), a_stackID, Severity::kInfo);
		return vec;
	}

	using deathEffectPair = std::pair<std::uint32_t, RE::EffectSetting*>;
	deathEffectPair effectPair = { -1, nullptr };

	using deathEffectMap = std::map<std::uint32_t, std::vector<std::pair<RE::EffectSetting*, float>>>;
	deathEffectMap effectMap;

	auto killer = a_actor->GetKiller();

	for (auto& activeEffect : *activeEffects) {
		if (activeEffect) {
			auto mgef = activeEffect->GetBaseObject();
			if (mgef && mgef->data.flags.all(FLAG::kHostile) && !mgef->HasKeywordString("FEC_MagicNoEffect")) {
				if (a_type == 0) {
					if (mgef->HasKeywordString("PO3_MagicDamageSun")) {
						effectPair = { DEATH_TYPE::kSun, mgef };  //sun override
						break;
					} else if (mgef->data.resistVariable == RE::ActorValue::kPoisonResist && mgef->data.castingType == RE::MagicSystem::CastingType::kConcentration) {
						effectPair = { DEATH_TYPE::kAcid, mgef };  //acid override
						break;
					} else if (mgef->HasKeywordString("MagicDamageFire")) {
						effectMap[DEATH_TYPE::kFire].emplace_back(mgef, -activeEffect->magnitude);	//flipping the magnitude back to +ve
					} else if (mgef->HasKeywordString("MagicDamageFrost")) {
						effectMap[DEATH_TYPE::kFrost].emplace_back(mgef, -activeEffect->magnitude);
					} else if (mgef->HasKeywordString("MagicDamageShock")) {
						effectMap[DEATH_TYPE::kShock].emplace_back(mgef, -activeEffect->magnitude);
					} else if (mgef->GetArchetype() == RE::Archetype::kAbsorb) {
						effectMap[DEATH_TYPE::kDrain].emplace_back(mgef, -activeEffect->magnitude);
					}
				} else {
					if (mgef->data.resistVariable == RE::ActorValue::kPoisonResist && mgef->data.castingType != RE::MagicSystem::CastingType::kConcentration) {
						effectMap[DEATH_TYPE::kPoison].emplace_back(mgef, -activeEffect->magnitude);
					} else if (mgef->GetArchetype() == RE::Archetype::kDemoralize || killer && killer->HasKeyword("ActorTypeGhost")) {
						effectMap[DEATH_TYPE::kDrain].emplace_back(mgef, -activeEffect->magnitude);
					}
				}
			}
		}
	}

	if (effectPair.first == DEATH_TYPE::kNone && !effectMap.empty()) {
		auto mag_cmp = [&](const auto& a_lhs, const auto& a_rhs) {
			return FLOAT::definitelyLessThan(a_lhs.second, a_rhs.second);
		};

		if (effectMap.size() == 1) {
			const auto& type = effectMap.begin()->first;
			auto it = *std::max_element(effectMap.begin()->second.begin(), effectMap.begin()->second.end(), mag_cmp);
			effectPair = { type, it.first };
		} else {
			if (a_type != 0) {
				bool poison = !effectMap[DEATH_TYPE::kPoison].empty();
				bool fear = !effectMap[DEATH_TYPE::kFear].empty();
				
				if (poison) {
					auto& poisonVec = effectMap[DEATH_TYPE::kPoison];
					auto poisonEffect = *std::max_element(poisonVec.begin(), poisonVec.end(), mag_cmp);

					effectPair = { DEATH_TYPE::kPoison, poisonEffect.first };
					if (fear) {
						effectPair.first = DEATH_TYPE::kPoisonFear;
					}
				} else if (fear) {
					auto& fearVec = effectMap[DEATH_TYPE::kFear];
					auto poisonEffect = *std::max_element(fearVec.begin(), fearVec.end(), mag_cmp);

					effectPair = { DEATH_TYPE::kFear, poisonEffect.first };
				}
			} else {
				bool fire = !effectMap[DEATH_TYPE::kFire].empty();
				bool frost = !effectMap[DEATH_TYPE::kFrost].empty();
				bool shock = !effectMap[DEATH_TYPE::kShock].empty();
				bool drain = !effectMap[DEATH_TYPE::kDrain].empty();

				if (fire) {
					auto& fireVec = effectMap[DEATH_TYPE::kFire];
					auto fireEffect = *std::max_element(fireVec.begin(), fireVec.end(), mag_cmp);

					effectPair = { DEATH_TYPE::kFire, fireEffect.first };
					if (frost) {
						effectPair.first = DEATH_TYPE::kFireFrost;
					} else if (shock) {
						effectPair.first = DEATH_TYPE::kFireShock;
					}
				} else if (drain) {
					auto& drainVec = effectMap[DEATH_TYPE::kDrain];
					auto drainEffect = *std::max_element(drainVec.begin(), drainVec.end(), mag_cmp);

					effectPair = { DEATH_TYPE::kDrain, drainEffect.first };
					if (shock) {
						effectPair.first = DEATH_TYPE::kDrainShock;
						auto& shockVec = effectMap[DEATH_TYPE::kShock];
						auto shockEffect = *std::max_element(shockVec.begin(), shockVec.end(), mag_cmp);

						if (FLOAT::definitelyLessThan(drainEffect.second, shockEffect.second)) {
							effectPair.second = shockEffect.first;
						}
					} else if (frost) {
						effectPair.first = DEATH_TYPE::kDrainFrost;
						auto& frostVec = effectMap[DEATH_TYPE::kFrost];
						auto frostEffect = *std::max_element(frostVec.begin(), frostVec.end(), mag_cmp);

						if (FLOAT::definitelyLessThan(drainEffect.second, frostEffect.second)) {
							effectPair.second = frostEffect.first;
						}
					}
				} else if (frost) {
					auto& frostVec = effectMap[DEATH_TYPE::kFrost];
					auto frostEffect = *std::max_element(frostVec.begin(), frostVec.end(), mag_cmp);

					effectPair = { DEATH_TYPE::kFrost, frostEffect.first };
					if (shock) {
						auto& shockVec = effectMap[DEATH_TYPE::kShock];
						auto shockEffect = *std::max_element(shockVec.begin(), shockVec.end(), mag_cmp);

						if (FLOAT::definitelyLessThan(frostEffect.second, shockEffect.second)) {
							effectPair = { DEATH_TYPE::kShockFrost, shockEffect.first };
						} else {
							effectPair.first = { DEATH_TYPE::kFrostShock };
						}
					}
				} else if (shock) {
					auto& shockVec = effectMap[DEATH_TYPE::kShock];
					auto shockEffect = *std::max_element(shockVec.begin(), shockVec.end(), mag_cmp);

					effectPair = { DEATH_TYPE::kShock, shockEffect.first };
				}
			}
		}
	}

	if (effectPair.first != DEATH_TYPE::kNone) {
		auto& [value, mgef] = effectPair;
		vec[0] = value;
		if (mgef) {
			vec[1] = mgef->GetMinimumSkillLevel();
			auto projectile = mgef->data.projectileBase;
			if (projectile) {
				vec[2] = projectile->GetType();
			}
		}
	}

	return vec;
}


RE::BGSColorForm* papyrusActor::GetHairColor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return nullptr;
	}

	auto actorbase = a_actor->GetActorBase();
	if (actorbase) {
		auto root = a_actor->Get3D(0);
		if (root) {
			auto data = root->GetExtraData<RE::NiIntegerExtraData>("PO3_HAIRTINT");
			if (data) {
				auto factory = RE::IFormFactory::GetFormFactoryByType(RE::FormType::ColorForm);
				auto color = static_cast<RE::BGSColorForm*>(factory->Create());
				if (color) {
					color->flags.reset(RE::BGSColorForm::Flag::kPlayable);
					color->color = RE::Color(data->value);
					return color;
				}
			}
		} else {
			auto headData = actorbase->headRelatedData;
			if (headData) {
				return headData->hairColor;
			}
		}
	}

	return nullptr;
}


RE::BGSTextureSet* papyrusActor::GetHeadPartTextureSet(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, std::uint32_t a_type)
{
	using HeadPartType = RE::BGSHeadPart::HeadPartType;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return nullptr;
	}

	auto actorBase = a_actor->GetActorBase();
	if (actorBase) {
		auto headpart = actorBase->GetCurrentHeadPartByType(static_cast<HeadPartType>(a_type));
		if (headpart) {
			return headpart->textureSet;
		}
	}

	return nullptr;
}


float papyrusActor::GetLocalGravityActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return 1.0f;
	}

	auto charProxy = static_cast<RE::bhkCharProxyController*>(a_actor->GetCharController());
	if (charProxy) {
		return charProxy->gravity;
	}

	return 0.0f;
}


RE::TESObjectREFR* papyrusActor::GetObjectUnderFeet(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return nullptr;
	}

	auto charProxy = static_cast<RE::bhkCharProxyController*>(a_actor->GetCharController());
	if (charProxy) {
		auto supportBody = charProxy->supportBody.get();
		if (supportBody) {
			auto owner = supportBody->GetUserData();
			if (owner) {
				return owner;
			}
		}
	}

	return nullptr;
}


RE::TESPackage* papyrusActor::GetRunningPackage(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return nullptr;
	}

	auto currentProcess = a_actor->currentProcess;
	if (currentProcess) {
		return currentProcess->GetRunningPackage();
	}

	return nullptr;
}


RE::BGSColorForm* papyrusActor::GetSkinColor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return nullptr;
	}

	auto actorBase = a_actor->GetActorBase();
	if (actorBase) {
		auto factory = RE::IFormFactory::GetFormFactoryByType(RE::FormType::ColorForm);
		auto color = static_cast<RE::BGSColorForm*>(factory->Create());
		if (color) {
			color->flags &= ~RE::BGSColorForm::Flag::kPlayable;
			color->color = actorBase->bodyTintColor;
			auto root = a_actor->Get3D(0);
			if (root) {
				auto data = root->GetExtraData<RE::NiIntegerExtraData>("PO3_SKINTINT");
				if (data) {
					color->color = RE::Color(data->value);
				}
			}
			return color;
		}
	}

	return nullptr;
}


float papyrusActor::GetTimeDead(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return 0.0f;
	}

	auto currentProcess = a_actor->currentProcess;
	if (currentProcess) {
		float timeOfDeath = currentProcess->deathTime;
		if (timeOfDeath > 0.0f) {
			auto calendar = RE::Calendar::GetSingleton();
			if (calendar) {
				auto g_gameDaysPassed = calendar->gameDaysPassed;
				return g_gameDaysPassed ? floorf(g_gameDaysPassed->value * 24.0f) - timeOfDeath : 0.0f;
			}
		} else {
			a_vm->TraceStack(VMError::generic_error(a_actor, "is not dead").c_str(), a_stackID, Severity::kWarning);
		}
	}

	return 0.0f;
}


float papyrusActor::GetTimeOfDeath(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return 0.0f;
	}

	auto currentProcess = a_actor->currentProcess;
	if (currentProcess) {
		float timeOfDeath = currentProcess->deathTime;
		if (timeOfDeath > 0.0f) {
			return timeOfDeath / 24.0f;
		} else {
			a_vm->TraceStack(VMError::generic_error(a_actor, "is not dead").c_str(), a_stackID, Severity::kWarning);
		}
	}

	return 0.0f;
}


bool papyrusActor::HasActiveSpell(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::SpellItem* a_spell)
{
	using AE = RE::ActiveEffect::Flag;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return false;
	} else if (!a_spell) {
		a_vm->TraceStack("Spell is None", a_stackID, Severity::kWarning);
		return false;
	}

	auto activeEffects = a_actor->GetActiveEffectList();
	if (!activeEffects) {
		a_vm->TraceStack(VMError::generic_error(a_actor, "has no active effects").c_str(), a_stackID, Severity::kInfo);
		return false;
	}

	for (auto& activeEffect : *activeEffects) {
		if (activeEffect && activeEffect->spell && activeEffect->spell == a_spell) {
			if (activeEffect->flags.none(AE::kInactive) && activeEffect->flags.none(AE::kDispelled)) {
				return true;
			}
		}
	}

	return false;
}


bool papyrusActor::HasDeferredKill(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return false;
	}

	auto currentProcess = a_actor->currentProcess;
	if (currentProcess) {
		auto middleHighProcess = currentProcess->middleHigh;
		return middleHighProcess && middleHighProcess->inDeferredKill;
	}

	return false;
}


bool papyrusActor::HasMagicEffectWithArchetype(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_archetype)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return false;
	} else if (a_archetype.empty()) {
		a_vm->TraceStack("Archetype is None", a_stackID, Severity::kWarning);
		return false;
	}

	auto activeEffects = a_actor->GetActiveEffectList();
	if (!activeEffects) {
		a_vm->TraceStack(VMError::generic_error(a_actor, "has no active effects").c_str(), a_stackID, Severity::kInfo);
		return false;
	}

	for (auto& activeEffect : *activeEffects) {
		if (activeEffect) {
			const auto mgef = activeEffect->GetBaseObject();
			return mgef && mgef->GetArchetypeAsString() == a_archetype;
		}
	}

	return false;
}


bool papyrusActor::IsActorInWater(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return false;
	}

	return a_actor->boolBits.all(RE::Actor::BOOL_BITS::kInWater);
}


bool papyrusActor::IsActorUnderwater(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return false;
	}

	return a_actor->boolFlags.all(RE::Actor::BOOL_FLAGS::kUnderwater);
}


bool papyrusActor::IsLimbGone(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, std::int32_t a_limbEnum)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return false;
	}

	return a_actor->IsLimbGone(a_limbEnum);
}


bool papyrusActor::IsQuadruped(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return false;
	}

	auto charController = a_actor->GetCharController();
	if (charController) {
		return charController->flags.all(RE::CHARACTER_FLAGS::kQuadruped);
	}

	return false;
}


bool papyrusActor::IsSoulTrapped(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	using Archetype = RE::EffectArchetypes::ArchetypeID;
	using Flags = RE::TESSoulGem::RecordFlags;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return false;
	}

	auto currentProcess = a_actor->currentProcess;
	if (currentProcess) {
		auto middleHighProcess = currentProcess->middleHigh;
		if (middleHighProcess && middleHighProcess->soulTrapped) {
			return true;
		}
	}

	bool isBeingSoulTrapped = false;

	auto processLists = RE::ProcessLists::GetSingleton();
	if (processLists) {
		processLists->GetMagicEffects([&](RE::BSTempEffect* a_tempEffect) {
			auto modelEffect = a_tempEffect->As<RE::ModelReferenceEffect>();
			if (modelEffect) {
				auto handle = a_actor->CreateRefHandle();
				if (modelEffect->target.native_handle() == handle.native_handle()) {
					auto modelArt = modelEffect->artObject;
					if (modelArt && modelArt->GetFormID() == SoulTrapHitArtID) {
						isBeingSoulTrapped = true;
						return false;
					}
				}
			}
			return true;
		});
	}

	return isBeingSoulTrapped;
}


void papyrusActor::KillNoWait(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None", a_stackID, Severity::kWarning);
		return;
	}

	a_actor->KillImmediate();
	a_actor->boolBits.set(RE::Actor::BOOL_BITS::kDead);
}


void papyrusActor::MixColorWithSkinTone(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BGSColorForm* a_color, bool a_manual, float a_percent)
{
	a_vm->TraceStack("Function is deprecated, use BlendColorWithSkinTone instead", a_stackID, Severity::kError);

	if (a_actor && a_color) {
		auto actorbase = a_actor->GetActorBase();
		if (actorbase) {
			auto root = a_actor->Get3D(0);
			if (root) {
				float skinLuminance = a_manual ? a_percent : RE::NiColor::CalcLuminance(actorbase->bodyTintColor);
				auto newColor = RE::NiColor::Mix(actorbase->bodyTintColor, a_color->color, skinLuminance);

				auto task = SKSE::GetTaskInterface();
				task->AddTask([a_actor, newColor, root]() {
					TintFace(a_actor, newColor);
					root->UpdateBodyTint(newColor);
				});

				AddOrUpdateColorData(root, "PO3_SKINTINT", newColor);
			}
		}
	}
}


bool papyrusActor::RemoveBasePerk(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BGSPerk* a_perk)
{
	using namespace Serialization;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return false;
	} else if (!a_perk) {
		a_vm->TraceStack("Perk is None", a_stackID, Severity::kWarning);
		return false;
	}

	auto perks = Form::Perks::GetSingleton();
	return perks->PapyrusApply(a_actor, a_perk, Form::kRemove);
}


bool papyrusActor::RemoveBaseSpell(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::SpellItem* a_spell)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return false;
	} else if (!a_spell) {
		a_vm->TraceStack("Spell is None", a_stackID, Severity::kWarning);
		return false;
	}

	auto activeEffects = a_actor->GetActiveEffectList();
	if (activeEffects) {
		for (auto& activeEffect : *activeEffects) {
			if (activeEffect && activeEffect->spell && activeEffect->spell == a_spell) {
				activeEffect->Dispel(true);
			}
		}
	}
	auto actorbase = a_actor->GetActorBase();
	if (actorbase) {
		auto actorEffects = actorbase->actorEffects;
		if (actorEffects) {
			if (actorEffects->GetIndex(a_spell) == std::nullopt) {
				return false;
			}
			auto combatController = a_actor->combatController;
			if (combatController) {
				combatController->data10->unk1C4 = 1;
			}
			a_actor->RemoveSelectedSpell(a_spell);
			return actorEffects->RemoveSpell(a_spell);
		}
	}

	return false;
}


void StopAllSkinAlphaShaders_Impl(RE::TESObjectREFR* a_ref)
{
	using Flags = RE::EffectShaderData::Flags;

	auto processLists = RE::ProcessLists::GetSingleton();
	if (processLists) {
		processLists->GetMagicEffects([&](RE::BSTempEffect* a_tempEffect) {
			auto shaderEffect = a_tempEffect->As<RE::ShaderReferenceEffect>();
			if (shaderEffect) {
				auto handle = a_ref->CreateRefHandle();
				if (shaderEffect->target == handle) {
					auto effectData = shaderEffect->effectData;
					if (effectData && effectData->data.flags.all(Flags::kSkinOnly) && !effectData->holesTexture.textureName.empty()) {
						shaderEffect->finished = true;
					}
				}
			}
			return true;
		});
	}
}


void papyrusActor::RemoveEffectsNotOfType(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, std::int32_t a_type)
{
	using namespace GraphicsReset;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return;
	}

	auto root = a_actor->Get3D(0);
	if (!root) {
		a_vm->TraceStack(VMError::no_3D(a_actor).c_str(), a_stackID, Severity::kWarning);
		return;
	}

	auto [toggleData, skinTintData, hairTintData, alphaData, headpartAlphaData, txstFaceData, txstVec, txstSkinVec, shaderVec] = GetResetData(root);

	switch (static_cast<EFFECT>(a_type)) {
	case EFFECT::kCharred:
		{
			if (skinTintData) {
				ResetSkinTintData(a_actor, root, skinTintData);
			}
			if (hairTintData) {
				ResetHairTintData(a_actor, root, hairTintData);
			}
			if (txstFaceData) {
				ResetFaceTXSTData(a_actor, root, txstFaceData);
			}
			if (!txstSkinVec.empty()) {
				ResetSkinTXSTData(a_actor, root, txstSkinVec);
			}
		}
		break;
	case EFFECT::kDrained:
		{
			if (toggleData) {
				ResetToggleData(root, toggleData);
			}
			if (skinTintData) {
				ResetSkinTintData(a_actor, root, skinTintData);
			}
			if (hairTintData) {
				ResetHairTintData(a_actor, root, hairTintData);
			}
			if (txstFaceData) {
				ResetFaceTXSTData(a_actor, root, txstFaceData);
			}
			if (!txstSkinVec.empty()) {
				ResetSkinTXSTData(a_actor, root, txstSkinVec);
			}
		}
		break;
	case EFFECT::kPoisoned:
		{
			if (!a_actor->IsPlayerRef()) {
				StopAllSkinAlphaShaders_Impl(a_actor);
			}
			if (toggleData) {
				ResetToggleData(root, toggleData);
			}
			if (alphaData) {
				ResetAlphaData(a_actor, root, alphaData, "po3_FEC");
			}
			if (headpartAlphaData) {
				ResetHeadPartAlphaData(a_actor, root, headpartAlphaData);
			}
			if (txstFaceData) {
				ResetFaceTXSTData(a_actor, root, txstFaceData);
			}
			if (!txstSkinVec.empty()) {
				ResetSkinTXSTData(a_actor, root, txstSkinVec);
			}
		}
		break;
	case EFFECT::kAged:
		{
			if (!a_actor->IsPlayerRef()) {
				StopAllSkinAlphaShaders_Impl(a_actor);
			}
			if (toggleData) {
				ResetToggleData(root, toggleData);
			}
			if (alphaData) {
				ResetAlphaData(a_actor, root, alphaData, "po3_FEC");
			}
			if (headpartAlphaData) {
				ResetHeadPartAlphaData(a_actor, root, headpartAlphaData);
			}
		}
		break;
	case EFFECT::kCharredCreature:
		{
			if (!shaderVec.empty()) {
				ResetShaderData(root, shaderVec);
			}
		}
		break;
	default:
		{
			a_vm->TraceStack("Invalid effect type", a_stackID, Severity::kWarning);
		}
		break;
	}
}


void SetTXST(RE::NiAVObject* a_object, RE::BGSTextureSet* a_txst, std::int32_t a_type, std::string_view a_tgtPath, bool& replaced)
{
	using State = RE::BSGeometry::States;
	using Feature = RE::BSShaderMaterial::Feature;
	using Texture = RE::BSTextureSet::Texture;

	RE::BSVisit::TraverseScenegraphGeometries(a_object, [&](RE::BSGeometry* a_geometry) -> RE::BSVisit::BSVisitControl {
		auto effect = a_geometry->properties[State::kEffect].get();
		if (effect) {
			auto lightingShader = netimmerse_cast<RE::BSLightingShaderProperty*>(effect);
			if (lightingShader) {
				auto material = static_cast<RE::BSLightingShaderMaterialBase*>(lightingShader->material);
				if (material) {
					std::string sourcePath = material->textureSet->GetTexturePath(Texture::kDiffuse);
					RE::Util::SanitizeTexturePath(sourcePath);

					if (sourcePath == a_tgtPath) {
						auto newMaterial = static_cast<RE::BSLightingShaderMaterialBase*>(material->Create());
						if (newMaterial) {
							newMaterial->CopyMembers(material);
							newMaterial->ClearTextures();

							if (a_type == -1) {
								newMaterial->OnLoadTextureSet(0, a_txst);
							} else {
								auto newTextureSet = RE::BSShaderTextureSet::Create();
								if (newTextureSet) {
									for (auto i = Texture::kDiffuse; i < Texture::kTotal; ++i) {
										newTextureSet->SetTexturePath(i, material->textureSet->GetTexturePath(i));
									}
									auto BSTextureType = static_cast<Texture>(a_type);
									newTextureSet->SetTexturePath(BSTextureType, a_txst->GetTexturePath(BSTextureType));

									newMaterial->OnLoadTextureSet(0, newTextureSet);
								}
							}

							lightingShader->SetMaterial(newMaterial, 1);
							lightingShader->InitializeGeometry(a_geometry);
							lightingShader->InitializeShader(a_geometry);
							newMaterial->~BSLightingShaderMaterialBase();
							RE::free(newMaterial);

							replaced = true;
						}
					}
				}
			}
		}
		return RE::BSVisit::BSVisitControl::kContinue;
	});
}


void papyrusActor::ReplaceArmorTextureSet(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::TESObjectARMO* a_armor, RE::BGSTextureSet* a_srcTXST, RE::BGSTextureSet* a_tgtTXST, std::int32_t a_type)
{
	using Texture = RE::BSShaderTextureSet::Textures::Texture;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return;
	} else if (!a_armor) {
		a_vm->TraceStack("Armor is None", a_stackID, Severity::kWarning);
		return;
	} else if (!a_srcTXST) {
		a_vm->TraceStack("Source TextureSet is None", a_stackID, Severity::kWarning);
		return;
	} else if (!a_tgtTXST) {
		a_vm->TraceStack("Target TextureSet is None", a_stackID, Severity::kWarning);
		return;
	} else if (!a_actor->Is3DLoaded()) {
		a_vm->TraceStack(VMError::no_3D(a_actor).c_str(), a_stackID, Severity::kWarning);
		return;
	}

	std::string targetPath = a_srcTXST->GetTexturePath(Texture::kDiffuse);
	RE::Util::SanitizeTexturePath(targetPath);

	auto task = SKSE::GetTaskInterface();
	task->AddTask([a_actor, a_armor, a_srcTXST, a_tgtTXST, a_type, targetPath]() {
		bool replaced = false;
		for (const auto& armorAddon : a_armor->armorAddons) {
			if (armorAddon) {
				auto armorObject = a_actor->VisitArmorAddon(a_armor, armorAddon);
				if (armorObject) {
					SetTXST(armorObject, a_tgtTXST, a_type, targetPath, replaced);
				}
			}
		}

		auto root = a_actor->Get3D(0);
		if (replaced && root) {
			auto armorID = std::to_string(a_armor->formID);
			std::string name = "PO3_TXST - " + armorID;

			auto data = root->GetExtraData<RE::NiStringsExtraData>(name.c_str());
			if (!data) {
				std::vector<RE::BSFixedString> vec;
				vec.reserve(Texture::kTotal);
				for (auto i = Texture::kDiffuse; i < Texture::kTotal; ++i) {
					vec.emplace_back(a_srcTXST->GetTexturePath(i));
				}
				vec.emplace_back(armorID);
				auto newData = RE::NiStringsExtraData::Create(name.c_str(), vec);
				if (newData) {
					root->AddExtraData(newData);
				}
			}
		}
	});
}


void SetSkinTXST(RE::NiAVObject* a_object, RE::BGSTextureSet* a_txst, std::vector<RE::BSFixedString>& a_vec, std::int32_t a_type)
{
	using State = RE::BSGeometry::States;
	using Feature = RE::BSShaderMaterial::Feature;
	using Texture = RE::BSTextureSet::Texture;

	RE::BSVisit::TraverseScenegraphGeometries(a_object, [&](RE::BSGeometry* a_geometry) -> RE::BSVisit::BSVisitControl {
		auto effect = a_geometry->properties[State::kEffect].get();
		if (effect) {
			auto lightingShader = netimmerse_cast<RE::BSLightingShaderProperty*>(effect);
			if (lightingShader) {
				auto material = static_cast<RE::BSLightingShaderMaterialBase*>(lightingShader->material);
				if (material) {
					auto type = material->GetFeature();
					if (type == Feature::kFaceGenRGBTint || type == Feature::kFaceGen) {
						if (a_vec.empty()) {
							for (auto i = Texture::kDiffuse; i < Texture::kTotal; ++i) {
								a_vec.emplace_back(material->textureSet->GetTexturePath(i));
							}
						}

						auto newMaterial = static_cast<RE::BSLightingShaderMaterialBase*>(material->Create());
						if (newMaterial) {
							newMaterial->CopyMembers(material);
							newMaterial->ClearTextures();

							if (a_type == -1) {
								if (type == Feature::kFaceGen) {
									auto newTextureSet = RE::BSShaderTextureSet::Create();
									if (newTextureSet) {
										for (auto i = Texture::kDiffuse; i < Texture::kTotal; ++i) {
											newTextureSet->SetTexturePath(i, a_txst->GetTexturePath(i));
											newTextureSet->SetTexturePath(Texture::kMultilayer, material->textureSet->GetTexturePath(Texture::kMultilayer));
											newMaterial->OnLoadTextureSet(0, newTextureSet);
										}
									}
								} else {
									newMaterial->OnLoadTextureSet(0, a_txst);
								}
							} else {
								auto newTextureSet = RE::BSShaderTextureSet::Create();
								if (newTextureSet) {
									for (auto i = Texture::kDiffuse; i < Texture::kTotal; ++i) {
										newTextureSet->SetTexturePath(i, material->textureSet->GetTexturePath(i));
									}
									auto BSTextureType = static_cast<Texture>(a_type);
									newTextureSet->SetTexturePath(BSTextureType, a_txst->GetTexturePath(BSTextureType));

									newMaterial->OnLoadTextureSet(0, newTextureSet);
								}
							}

							lightingShader->SetMaterial(newMaterial, 1);
							lightingShader->InitializeGeometry(a_geometry);
							lightingShader->InitializeShader(a_geometry);
							newMaterial->~BSLightingShaderMaterialBase();
							RE::free(newMaterial);
						}
					}
				}
			}
		}
		return RE::BSVisit::BSVisitControl::kContinue;
	});
}


void papyrusActor::ReplaceFaceTextureSet(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BGSTextureSet* a_maleTXST, RE::BGSTextureSet* a_femaleTXST, std::int32_t a_type)
{
	using Texture = RE::BSShaderTextureSet::Texture;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return;
	} else if (!a_actor->Is3DLoaded()) {
		a_vm->TraceStack(VMError::no_3D(a_actor).c_str(), a_stackID, Severity::kWarning);
		return;
	}

	auto actorBase = a_actor->GetActorBase();
	bool isFemale = actorBase ? actorBase->IsFemale() : false;

	auto txst = isFemale ? a_femaleTXST : a_maleTXST;
	if (!txst) {
		a_vm->TraceStack("TextureSet is None", a_stackID, Severity::kWarning);
		return;
	}

	auto faceObject = a_actor->GetHeadPartObject(RE::BGSHeadPart::HeadPartType::kFace);
	if (faceObject) {
		auto task = SKSE::GetTaskInterface();
		task->AddTask([faceObject, txst, a_type, a_actor]() {
			std::vector<RE::BSFixedString> vec;
			vec.reserve(Texture::kTotal);
			SetSkinTXST(faceObject, txst, vec, a_type);

			auto root = a_actor->Get3D(0);
			if (!vec.empty() && root) {
				auto data = root->GetExtraData<RE::NiStringsExtraData>("PO3_FACETXST");
				if (!data) {
					auto newData = RE::NiStringsExtraData::Create("PO3_FACETXST", vec);
					if (newData) {
						root->AddExtraData(newData);
					}
				}
			}
		});
	} else {
		a_vm->TraceStack("Cannot get face headpart", a_stackID, Severity::kWarning);
	}
}


void SetArmorSkinTXST(RE::Actor* a_actor, RE::BGSTextureSet* a_txst, RE::BGSBipedObjectForm::BipedObjectSlot a_slot, std::int32_t a_type)
{
	auto skinArmor = a_actor->GetSkin(a_slot);
	if (!skinArmor) {
		return;
	}

	auto foundAddon = skinArmor->GetArmorAddonByMask(a_actor->race, a_slot);
	if (!foundAddon) {
		return;
	}

	auto armorObject = a_actor->VisitArmorAddon(skinArmor, foundAddon);
	if (armorObject) {
		auto task = SKSE::GetTaskInterface();
		task->AddTask([a_actor, a_txst, a_slot, a_type, armorObject]() {
			std::vector<RE::BSFixedString> vec;
			vec.reserve(10);
			SetSkinTXST(armorObject, a_txst, vec, a_type);

			auto root = a_actor->Get3D(0);
			if (!vec.empty() && root) {
				auto slotMaskStr = std::to_string(to_underlying(a_slot));
				std::string name = "PO3_SKINTXST - " + slotMaskStr;
				vec.emplace_back(slotMaskStr.c_str());

				auto data = root->GetExtraData<RE::NiStringsExtraData>(name.c_str());
				if (!data) {
					auto newData = RE::NiStringsExtraData::Create(name.c_str(), vec);
					if (newData) {
						root->AddExtraData(newData);
					}
				}
			}
		});
	}
}


void papyrusActor::ReplaceSkinTextureSet(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BGSTextureSet* a_maleTXST, RE::BGSTextureSet* a_femaleTXST, std::uint32_t a_slot, std::int32_t a_type)
{
	using BipedSlot = RE::BGSBipedObjectForm::BipedObjectSlot;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return;
	} else if (!a_actor->Is3DLoaded()) {
		a_vm->TraceStack(VMError::no_3D(a_actor).c_str(), a_stackID, Severity::kWarning);
		return;
	}

	auto actorBase = a_actor->GetActorBase();
	bool isFemale = actorBase ? actorBase->IsFemale() : false;

	if (isFemale) {
		if (!a_femaleTXST) {
			a_vm->TraceStack("Female TextureSet is None", a_stackID, Severity::kWarning);
			return;
		}
		SetArmorSkinTXST(a_actor, a_femaleTXST, static_cast<BipedSlot>(a_slot), a_type);
	} else {
		if (!a_maleTXST) {
			a_vm->TraceStack("Male TextureSet is None", a_stackID, Severity::kWarning);
			return;
		}
		SetArmorSkinTXST(a_actor, a_maleTXST, static_cast<BipedSlot>(a_slot), a_type);
	}
}


bool papyrusActor::ResetActor3D(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_folderName)
{
	using namespace GraphicsReset;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return false;
	}

	auto root = a_actor->Get3D(0);
	if (!root) {
		a_vm->TraceStack(VMError::no_3D(a_actor).c_str(), a_stackID, Severity::kWarning);
		return false;
	}

	auto [toggleData, skinTintData, hairTintData, alphaData, headpartAlphaData, txstFaceData, txstVec, txstSkinVec, shaderVec] = GetResetData(root);

	if (!toggleData && !alphaData && !headpartAlphaData && !skinTintData && !hairTintData && !txstFaceData && txstVec.empty() && txstSkinVec.empty() && shaderVec.empty()) {
		return false;
	} else {
		if (!a_actor->IsPlayerRef()) {
			auto processLists = RE::ProcessLists::GetSingleton();
			if (processLists) {
				processLists->StopAllShaders(a_actor);
			}
		}

		if (toggleData) {
			ResetToggleData(root, toggleData);
		}
		if (alphaData) {
			ResetAlphaData(a_actor, root, alphaData, a_folderName);
		}
		if (headpartAlphaData) {
			ResetHeadPartAlphaData(a_actor, root, headpartAlphaData);
		}
		if (skinTintData) {
			ResetSkinTintData(a_actor, root, skinTintData);
		}
		if (hairTintData) {
			ResetHairTintData(a_actor, root, hairTintData);
		}
		if (txstFaceData) {
			ResetFaceTXSTData(a_actor, root, txstFaceData);
		}
		if (!txstSkinVec.empty()) {
			ResetSkinTXSTData(a_actor, root, txstSkinVec);
		}
		if (!txstVec.empty() && !a_folderName.empty()) {
			ResetTXSTData(a_actor, root, a_folderName, txstVec);
		}
		if (!shaderVec.empty()) {
			ResetShaderData(root, shaderVec);
		}

		return true;
	}

	return false;
}


void papyrusActor::SetActorRefraction(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, float a_refraction)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return;
	}

	auto currentProcess = a_actor->currentProcess;
	if (currentProcess) {
		a_refraction = std::clamp(a_refraction, 0.0f, 1.0f);
		currentProcess->SetRefraction(a_refraction);

		float invisibility = a_actor->GetActorValue(RE::ActorValue::kInvisibility);
		if (invisibility < 0.0 || (invisibility <= 1.0 && invisibility <= 0.0) || !a_actor->IsPlayerRef()) {
			if (a_refraction <= 0.0) {
				a_actor->SetRefraction(0, a_refraction);
				a_actor->UpdateAlpha();
			} else {
				a_actor->SetRefraction(1, a_refraction);
			}
		} else {
			a_actor->SetAlpha(1.0);

			a_refraction = 1.0f - a_refraction / 100.0f;
			a_refraction = 1.0f + (0.01f - 1.0f) * ((a_refraction - 0.0f) / (1.0f - 0.0f));

			a_actor->SetRefraction(1, a_refraction);
		}
	}
}


void papyrusActor::SetHairColor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BGSColorForm* a_color)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return;
	} else if (!a_color) {
		a_vm->TraceStack("Colorform is None", a_stackID, Severity::kWarning);
		return;
	}

	auto root = a_actor->Get3D(0);
	if (root) {
		auto task = SKSE::GetTaskInterface();
		task->AddTask([root, a_color]() {
			root->UpdateHairColor(a_color->color);
		});

		AddOrUpdateColorData(root, "PO3_HAIRTINT", a_color->color);
	} else {
		a_vm->TraceStack(VMError::no_3D(a_actor).c_str(), a_stackID, Severity::kWarning);
	}
}


void papyrusActor::SetHeadPartAlpha(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, std::int32_t a_type, float a_alpha)
{
	using HeadPartType = RE::BGSHeadPart::HeadPartType;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return;
	} else if (a_type < 0 || a_type > 6) {
		a_vm->TraceStack("Invalid headpart type", a_stackID, Severity::kWarning);
		return;
	}

	auto root = a_actor->Get3D(0);
	if (!root) {
		a_vm->TraceStack(VMError::no_3D(a_actor).c_str(), a_stackID, Severity::kWarning);
		return;
	}

	auto object = a_actor->GetHeadPartObject(static_cast<HeadPartType>(a_type));
	if (object) {
		auto task = SKSE::GetTaskInterface();
		task->AddTask([object, a_alpha]() {
			object->UpdateAlpha(a_alpha, false);
		});

		auto data = root->GetExtraData<RE::NiIntegersExtraData>("PO3_HEADPARTALPHA");
		if (!data) {
			if (a_alpha == 0.0f) {
				std::vector<std::int32_t> vec;
				vec.push_back(a_type);
				auto newData = RE::NiIntegersExtraData::Create("PO3_HEADPARTALPHA", vec);
				if (newData) {
					root->AddExtraData(newData);
				}
			}
		} else {
			a_alpha == 0.0 ? data->InsertElement(a_type) : data->RemoveElement(a_type);
		}
	} else {
		a_vm->TraceStack("Could not find matching headpart object", a_stackID, Severity::kWarning);
	}
}


void papyrusActor::SetHeadPartTextureSet(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BGSTextureSet* a_txst, std::int32_t a_type)
{
	using HeadPartType = RE::BGSHeadPart::HeadPartType;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return;
	} else if (!a_txst) {
		a_vm->TraceStack("Textureset is None", a_stackID, Severity::kWarning);
		return;
	} else if (a_type < 0 || a_type > 6) {
		a_vm->TraceStack("Invalid headpart type", a_stackID, Severity::kWarning);
		return;
	}

	auto actorBase = a_actor->GetActorBase();
	if (actorBase) {
		auto headpart = actorBase->GetCurrentHeadPartByType(static_cast<HeadPartType>(a_type));
		if (headpart) {
			headpart->textureSet = a_txst;
		} else {
			a_vm->TraceStack("Could not find matching headpart", a_stackID, Severity::kWarning);
		}
	}
}


void papyrusActor::SetLinearVelocity(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, float a_x, float a_y, float a_z)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return;
	}

	auto currentProcess = a_actor->currentProcess;
	if (currentProcess) {
		auto charProxy = static_cast<RE::bhkCharProxyController*>(currentProcess->GetCharController());
		if (charProxy) {
			charProxy->SetLinearVelocityImpl(RE::hkVector4(a_x * 0.0142875f, a_y * 0.0142875f, a_z * 0.0142875f, 0.0f));
		}
		currentProcess->Update3DModel(a_actor);
	}
}


void papyrusActor::SetLocalGravityActor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, float a_value, bool a_disableGravityOnGround)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return;
	}

	auto currentProcess = a_actor->currentProcess;
	if (currentProcess) {
		auto charProxy = static_cast<RE::bhkCharProxyController*>(currentProcess->GetCharController());
		if (charProxy) {
			a_disableGravityOnGround ? charProxy->flags.reset(RE::CHARACTER_FLAGS::kNoGravityOnGround) : charProxy->flags.set(RE::CHARACTER_FLAGS::kNoGravityOnGround);
			charProxy->gravity = a_value;
		}
		currentProcess->Update3DModel(a_actor);
	}
}


void papyrusActor::SetSkinAlpha(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, float a_alpha)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return;
	}

	auto root = a_actor->Get3D(0);
	if (root) {
		auto task = SKSE::GetTaskInterface();
		task->AddTask([root, a_alpha]() {
			root->UpdateAlpha(a_alpha, true);
		});

		auto data = root->GetExtraData<RE::NiFloatExtraData>("PO3_ALPHA");
		if (data) {
			if (a_alpha == 1.0) {
				root->RemoveExtraData(data);
			}
		} else {
			auto newData = RE::NiFloatExtraData::Create("PO3_ALPHA", a_alpha);
			if (newData) {
				root->AddExtraData(newData);
			}
		}
	} else {
		a_vm->TraceStack(VMError::no_3D(a_actor).c_str(), a_stackID, Severity::kWarning);
	}
}


void papyrusActor::SetSkinColor(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BGSColorForm* a_color)
{
	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return;
	} else if (!a_color) {
		a_vm->TraceStack("Colorform is None", a_stackID, Severity::kWarning);
		return;
	}

	auto actorbase = a_actor->GetActorBase();
	if (actorbase) {
		auto root = a_actor->Get3D(0);
		if (!root) {
			a_vm->TraceStack(VMError::no_3D(a_actor).c_str(), a_stackID, Severity::kWarning);
			return;
		}

		auto task = SKSE::GetTaskInterface();
		task->AddTask([a_actor, a_color, root]() {
			TintFace(a_actor, a_color->color);
			root->UpdateBodyTint(a_color->color);
		});

		AddOrUpdateColorData(root, "PO3_SKINTINT", a_color->color);
	}
}


void papyrusActor::UnequipAllOfType(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, std::uint32_t a_armorType, std::vector<std::uint32_t> a_slotsToSkip)
{
	using Slot = RE::BGSBipedObjectForm::BipedObjectSlot;

	if (!a_actor) {
		a_vm->TraceStack("Actor is None ", a_stackID, Severity::kWarning);
		return;
	}

	auto inv = a_actor->GetInventory([a_armorType, a_slotsToSkip](RE::TESBoundObject& a_object) {
		auto armor = a_object.As<RE::TESObjectARMO>();
		if (armor && armor->bipedModelData.armorType.underlying() == a_armorType) {
			if (a_slotsToSkip.empty() || std::none_of(a_slotsToSkip.begin(), a_slotsToSkip.end(),
											 [&](const std::uint32_t& slot) {
												 return armor->HasPartOf(static_cast<Slot>(slot));
											 }))
				return true;
		}
		return false;
	});

	auto objectManager = RE::BGSDefaultObjectManager::GetSingleton();
	auto equipManager = RE::ActorEquipManager::GetSingleton();
	RE::BGSEquipSlot* rightHandSlot = nullptr;
	if (objectManager) {
		rightHandSlot = objectManager->GetObject<RE::BGSEquipSlot>(RE::BGSDefaultObjectManager::DefaultObject::kRightHandEquip);
	}
	if (rightHandSlot && equipManager) {
		for (auto& item : inv) {
			auto& [count, entry] = item.second;
			if (count > 0 && entry && entry->GetWorn()) {
				equipManager->UnequipObject(a_actor, item.first, nullptr, 1, rightHandSlot, true, false, false, false, nullptr);
			}
		}
	}
}


bool papyrusActor::RegisterFuncs(VM* a_vm)
{
	if (!a_vm) {
		logger::critical("papyrusActor - couldn't get VMState");
		return false;
	}

	a_vm->RegisterFunction("AddBasePerk", "PO3_SKSEFunctions", AddBasePerk);

	a_vm->RegisterFunction("AddBaseSpell", "PO3_SKSEFunctions", AddBaseSpell);

	a_vm->RegisterFunction("AddAllEquippedItemsToArray", "PO3_SKSEFunctions", AddAllEquippedItemsToArray);

	a_vm->RegisterFunction("BlendColorWithSkinTone", "PO3_SKSEFunctions", BlendColorWithSkinTone);

	a_vm->RegisterFunction("DecapitateActor", "PO3_SKSEFunctions", DecapitateActor);

	a_vm->RegisterFunction("EquipArmorIfSkinVisible", "PO3_SKSEFunctions", EquipArmorIfSkinVisible);

	a_vm->RegisterFunction("FreezeActor", "PO3_SKSEFunctions", FreezeActor);

	a_vm->RegisterFunction("GetActiveEffects", "PO3_SKSEFunctions", GetActiveEffects);

	a_vm->RegisterFunction("GetActorAlpha", "PO3_SKSEFunctions", GetActorAlpha);

	a_vm->RegisterFunction("GetActorRefraction", "PO3_SKSEFunctions", GetActorRefraction);

	a_vm->RegisterFunction("GetActorState", "PO3_SKSEFunctions", GetActorState, true);

	a_vm->RegisterFunction("GetCriticalStage", "PO3_SKSEFunctions", GetCriticalStage, true);

	a_vm->RegisterFunction("GetCombatAllies", "PO3_SKSEFunctions", GetCombatAllies);

	a_vm->RegisterFunction("GetCombatTargets", "PO3_SKSEFunctions", GetCombatTargets);

	a_vm->RegisterFunction("GetDeathEffectType", "PO3_SKSEFunctions", GetDeathEffectType);

	a_vm->RegisterFunction("GetHairColor", "PO3_SKSEFunctions", GetHairColor);

	a_vm->RegisterFunction("GetHeadPartTextureSet", "PO3_SKSEFunctions", GetHeadPartTextureSet);

	a_vm->RegisterFunction("GetLocalGravityActor", "PO3_SKSEFunctions", GetLocalGravityActor);

	a_vm->RegisterFunction("GetObjectUnderFeet", "PO3_SKSEFunctions", GetObjectUnderFeet);

	a_vm->RegisterFunction("GetRunningPackage", "PO3_SKSEFunctions", GetRunningPackage);

	a_vm->RegisterFunction("GetSkinColor", "PO3_SKSEFunctions", GetSkinColor);

	a_vm->RegisterFunction("GetTimeDead", "PO3_SKSEFunctions", GetTimeDead);

	a_vm->RegisterFunction("GetTimeOfDeath", "PO3_SKSEFunctions", GetTimeOfDeath);

	a_vm->RegisterFunction("HasActiveSpell", "PO3_SKSEFunctions", HasActiveSpell);

	a_vm->RegisterFunction("IsQuadruped", "PO3_SKSEFunctions", IsQuadruped, true);

	a_vm->RegisterFunction("HasDeferredKill", "PO3_SKSEFunctions", HasDeferredKill);

	a_vm->RegisterFunction("HasMagicEffectWithArchetype", "PO3_SKSEFunctions", HasMagicEffectWithArchetype);

	a_vm->RegisterFunction("IsActorInWater", "PO3_SKSEFunctions", IsActorInWater, true);

	a_vm->RegisterFunction("IsActorUnderwater", "PO3_SKSEFunctions", IsActorUnderwater, true);

	a_vm->RegisterFunction("IsLimbGone", "PO3_SKSEFunctions", IsLimbGone);

	a_vm->RegisterFunction("IsSoulTrapped", "PO3_SKSEFunctions", IsSoulTrapped);

	a_vm->RegisterFunction("KillNoWait", "PO3_SKSEFunctions", KillNoWait);

	a_vm->RegisterFunction("MixColorWithSkinTone", "PO3_SKSEFunctions", MixColorWithSkinTone);

	a_vm->RegisterFunction("RemoveBasePerk", "PO3_SKSEFunctions", RemoveBasePerk);

	a_vm->RegisterFunction("RemoveBaseSpell", "PO3_SKSEFunctions", RemoveBaseSpell);

	a_vm->RegisterFunction("RemoveEffectsNotOfType", "PO3_SKSEFunctions", RemoveEffectsNotOfType);

	a_vm->RegisterFunction("ReplaceArmorTextureSet", "PO3_SKSEFunctions", ReplaceArmorTextureSet);

	a_vm->RegisterFunction("ReplaceFaceTextureSet", "PO3_SKSEFunctions", ReplaceFaceTextureSet);

	a_vm->RegisterFunction("ReplaceSkinTextureSet", "PO3_SKSEFunctions", ReplaceSkinTextureSet);

	a_vm->RegisterFunction("ResetActor3D", "PO3_SKSEFunctions", ResetActor3D);

	a_vm->RegisterFunction("SetActorRefraction", "PO3_SKSEFunctions", SetActorRefraction);

	a_vm->RegisterFunction("SetHairColor", "PO3_SKSEFunctions", SetHairColor);

	a_vm->RegisterFunction("SetHeadPartAlpha", "PO3_SKSEFunctions", SetHeadPartAlpha);

	a_vm->RegisterFunction("SetHeadPartTextureSet", "PO3_SKSEFunctions", SetHeadPartTextureSet);

	a_vm->RegisterFunction("SetLinearVelocity", "PO3_SKSEFunctions", SetLinearVelocity);

	a_vm->RegisterFunction("SetLocalGravityActor", "PO3_SKSEFunctions", SetLocalGravityActor);

	a_vm->RegisterFunction("SetSkinAlpha", "PO3_SKSEFunctions", SetSkinAlpha);

	a_vm->RegisterFunction("SetSkinColor", "PO3_SKSEFunctions", SetSkinColor);

	a_vm->RegisterFunction("UnequipAllOfType", "PO3_SKSEFunctions", UnequipAllOfType);

	return true;
}