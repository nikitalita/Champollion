#include <vector>
#include <string>

namespace Decompiler{
	namespace Skyrim {
		static const std::vector<std::string> EventNames = {
			"OnAnimationEvent", // ActiveMagicEffect
			"OnAnimationEventUnregistered", // ActiveMagicEffect
			"OnEffectFinish", // ActiveMagicEffect
			"OnEffectStart", // ActiveMagicEffect
			"OnGainLOS", // ActiveMagicEffect
			"OnLostLOS", // ActiveMagicEffect
			"OnUpdate", // ActiveMagicEffect
			"OnCombatStateChanged", // Actor
			"OnDeath", // Actor
			"OnDying", // Actor
			"OnEnterBleedout", // Actor
			"OnGetUp", // Actor
			"OnLocationChange", // Actor
			"OnLycanthropyStateChanged", // Actor
			"OnObjectEquipped", // Actor
			"OnObjectUnequipped", // Actor
			"OnPackageChange", // Actor
			"OnPackageEnd", // Actor
			"OnPackageStart", // Actor
			"OnPlayerBowShot", // Actor
			"OnPlayerFastTravelEnd", // Actor
			"OnPlayerLoadGame", // Actor
			"OnRaceSwitchComplete", // Actor
			"OnSit", // Actor
			"OnVampireFeed", // Actor
			"OnVampirismStateChanged", // Actor
			"OnAnimationEvent", // Alias
			"OnAnimationEventUnregistered", // Alias
			"OnGainLOS", // Alias
			"OnLostLOS", // Alias
			"OnReset", // Alias
			"OnUpdate", // Alias
			"OnAnimationEvent", // Form
			"OnAnimationEventUnregistered", // Form
			"OnGainLOS", // Form
			"OnLostLOS", // Form
			"OnSleepStart", // Form
			"OnSleepStop", // Form
			"OnTrackedStatsEvent", // Form
			"OnUpdate", // Form
			"OnUpdateGameTime", // Form
			"OnActivate", // ObjectReference
			"OnAttachedToCell", // ObjectReference
			"OnCellAttach", // ObjectReference
			"OnCellDetach", // ObjectReference
			"OnCellLoad", // ObjectReference
			"OnClose", // ObjectReference
			"OnContainerChanged", // ObjectReference
			"OnDestructionStageChanged", // ObjectReference
			"OnDetachedFromCell", // ObjectReference
			"OnEquipped", // ObjectReference
			"OnGrab", // ObjectReference
			"OnHit", // ObjectReference
			"OnItemAdded", // ObjectReference
			"OnItemRemoved", // ObjectReference
			"OnLoad", // ObjectReference
			"OnLockStateChanged", // ObjectReference
			"OnMagicEffectApply", // ObjectReference
			"OnOpen", // ObjectReference
			"OnRead", // ObjectReference
			"OnRelease", // ObjectReference
			"OnReset", // ObjectReference
			"OnSell", // ObjectReference
			"OnSpellCast", // ObjectReference
			"OnTranslationAlmostComplete", // ObjectReference
			"OnTranslationComplete", // ObjectReference
			"OnTranslationFailed", // ObjectReference
			"OnTrapHit", // ObjectReference
			"OnTrapHitStart", // ObjectReference
			"OnTrapHitStop", // ObjectReference
			"OnTrigger", // ObjectReference
			"OnTriggerEnter", // ObjectReference
			"OnTriggerLeave", // ObjectReference
			"OnUnequipped", // ObjectReference
			"OnUnload", // ObjectReference
			"OnWardHit", // ObjectReference
			"OnReset", // Quest
			"OnStoryActivateActor", // Quest
			"OnStoryAddToPlayer", // Quest
			"OnStoryArrest", // Quest
			"OnStoryAssaultActor", // Quest
			"OnStoryBribeNPC", // Quest
			"OnStoryCastMagic", // Quest
			"OnStoryChangeLocation", // Quest
			"OnStoryCraftItem", // Quest
			"OnStoryCrimeGold", // Quest
			"OnStoryCure", // Quest
			"OnStoryDialogue", // Quest
			"OnStoryDiscoverDeadBody", // Quest
			"OnStoryEscapeJail", // Quest
			"OnStoryFlatterNPC", // Quest
			"OnStoryHello", // Quest
			"OnStoryIncreaseLevel", // Quest
			"OnStoryIncreaseSkill", // Quest
			"OnStoryInfection", // Quest
			"OnStoryIntimidateNPC", // Quest
			"OnStoryJail", // Quest
			"OnStoryKillActor", // Quest
			"OnStoryNewVoicePower", // Quest
			"OnStoryPayFine", // Quest
			"OnStoryPickLock", // Quest
			"OnStoryPlayerGetsFavor", // Quest
			"OnStoryRelationshipChange", // Quest
			"OnStoryRemoveFromPlayer", // Quest
			"OnStoryScript", // Quest
			"OnStoryServedTime", // Quest
			"OnStoryTrespass", // Quest
			"OnBeginState",
			"OnEndState",
			"OnInit"
		};
	}
	namespace Fallout4{
		static const std::vector<std::string> EventNames = {
			"OnEffectFinish", // ActiveMagicEffect
			"OnEffectStart", // ActiveMagicEffect
			"OnCombatStateChanged", // Actor
			"OnCommandModeCompleteCommand", // Actor
			"OnCommandModeEnter", // Actor
			"OnCommandModeExit", // Actor
			"OnCommandModeGiveCommand", // Actor
			"OnCompanionDismiss", // Actor
			"OnConsciousnessStateChanged", // Actor
			"OnCripple", // Actor
			"OnDeath", // Actor
			"OnDeferredKill", // Actor
			"OnDifficultyChanged", // Actor
			"OnDying", // Actor
			"OnEnterBleedout", // Actor
			"OnEnterSneaking", // Actor
			"OnEscortWaitStart", // Actor
			"OnEscortWaitStop", // Actor
			"OnGetUp", // Actor
			"OnItemEquipped", // Actor
			"OnItemUnequipped", // Actor
			"OnKill", // Actor
			"OnLocationChange", // Actor
			"OnPackageChange", // Actor
			"OnPackageEnd", // Actor
			"OnPackageStart", // Actor
			"OnPartialCripple", // Actor
			"OnPickpocketFailed", // Actor
			"OnPlayerCreateRobot", // Actor
			"OnPlayerEnterVertibird", // Actor
			"OnPlayerFallLongDistance", // Actor
			"OnPlayerFireWeapon", // Actor
			"OnPlayerHealTeammate", // Actor
			"OnPlayerLoadGame", // Actor
			"OnPlayerModArmorWeapon", // Actor
			"OnPlayerModRobot", // Actor
			"OnPlayerSwimming", // Actor
			"OnPlayerUseWorkBench", // Actor
			"OnRaceSwitchComplete", // Actor
			"OnSit", // Actor
			"OnSpeechChallengeAvailable", // Actor
			"OnAliasInit", // Alias
			"OnAliasReset", // Alias
			"OnAliasShutdown", // Alias
			"OnLocationCleared", // Location
			"OnLocationLoaded", // Location
			"OnActivate", // ObjectReference
			"OnCellAttach", // ObjectReference
			"OnCellDetach", // ObjectReference
			"OnCellLoad", // ObjectReference
			"OnClose", // ObjectReference
			"OnContainerChanged", // ObjectReference
			"OnDestructionStageChanged", // ObjectReference
			"OnEquipped", // ObjectReference
			"OnExitFurniture", // ObjectReference
			"OnGrab", // ObjectReference
			"OnHolotapeChatter", // ObjectReference
			"OnHolotapePlay", // ObjectReference
			"OnItemAdded", // ObjectReference
			"OnItemRemoved", // ObjectReference
			"OnLoad", // ObjectReference
			"OnLockStateChanged", // ObjectReference
			"OnOpen", // ObjectReference
			"OnPipboyRadioDetection", // ObjectReference
			"OnPlayerDialogueTarget", // ObjectReference
			"OnPowerOff", // ObjectReference
			"OnPowerOn", // ObjectReference
			"OnRead", // ObjectReference
			"OnRelease", // ObjectReference
			"OnReset", // ObjectReference
			"OnSell", // ObjectReference
			"OnSpellCast", // ObjectReference
			"OnTranslationAlmostComplete", // ObjectReference
			"OnTranslationComplete", // ObjectReference
			"OnTranslationFailed", // ObjectReference
			"OnTrapHitStart", // ObjectReference
			"OnTrapHitStop", // ObjectReference
			"OnTriggerEnter", // ObjectReference
			"OnTriggerLeave", // ObjectReference
			"OnUnequipped", // ObjectReference
			"OnUnload", // ObjectReference
			"OnWorkshopMode", // ObjectReference
			"OnWorkshopNPCTransfer", // ObjectReference
			"OnWorkshopObjectDestroyed", // ObjectReference
			"OnWorkshopObjectGrabbed", // ObjectReference
			"OnWorkshopObjectMoved", // ObjectReference
			"OnWorkshopObjectPlaced", // ObjectReference
			"OnWorkshopObjectRepaired", // ObjectReference
			"OnChange", // Package
			"OnEnd", // Package
			"OnStart", // Package
			"OnEntryRun", // Perk
			"OnQuestInit", // Quest
			"OnQuestShutdown", // Quest
			"OnStageSet", // Quest
			"OnStoryIncreaseLevel", // Quest
			"OnAction", // Scene
			"OnBegin", // Scene
			"OnEnd", // Scene
			"OnPhaseBegin", // Scene
			"OnPhaseEnd", // Scene
			"OnAnimationEvent", // ScriptObject
			"OnAnimationEventUnregistered", // ScriptObject
			"OnBeginState", // ScriptObject
			"OnControlDown", // ScriptObject
			"OnControlUp", // ScriptObject
			"OnDistanceGreaterThan", // ScriptObject
			"OnDistanceLessThan", // ScriptObject
			"OnEndState", // ScriptObject
			"OnFurnitureEvent", // ScriptObject
			"OnGainLOS", // ScriptObject
			"OnHit", // ScriptObject
			"OnInit", // ScriptObject
			"OnKeyDown", // ScriptObject
			"OnKeyUp", // ScriptObject
			"OnLooksMenuEvent", // ScriptObject
			"OnLostLOS", // ScriptObject
			"OnMagicEffectApply", // ScriptObject
			"OnMenuOpenCloseEvent", // ScriptObject
			"OnPlayerCameraState", // ScriptObject
			"OnPlayerSleepStart", // ScriptObject
			"OnPlayerSleepStop", // ScriptObject
			"OnPlayerTeleport", // ScriptObject
			"OnPlayerWaitStart", // ScriptObject
			"OnPlayerWaitStop", // ScriptObject
			"OnRadiationDamage", // ScriptObject
			"OnTimer", // ScriptObject
			"OnTimerGameTime", // ScriptObject
			"OnTrackedStatsEvent", // ScriptObject
			"OnTutorialEvent", // ScriptObject
			"OnMenuItemRun", // Terminal
			"OnBegin", // TopicInfo
			"OnEnd", // TopicInfo
		};
	}
}