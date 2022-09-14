// // Copyright Epic Games, Inc. All Rights Reserved.
//
// #pragma once
//
// #include "Interfaces/OnlineAchievementsInterface.h"
// #include "OnlineIdentityPico.h"
// #include "OnlineSubsystemPicoPackage.h"
//
// /**
// * Enum that signifies how an achievement is to be unlocked
// */
// enum class EAchievementType {
// 	Unknown = 0,
// 	Simple,
// 	Bitfield,
// 	Count
// };
//
// /**
// *	FOnlineAchievementDescPico - Interface class for accessing the oculus achievement description information
// */
// struct FOnlineAchievementDescPico : FOnlineAchievementDesc
// {
// 	/** The way this achievement is unlocked */
// 	EAchievementType Type;
//
// 	/** The value that needs to be reached for "Count" Type achievements to unlock */
// 	uint64 Target;
//
// 	/** How many fields needs to be set for "Bitfield" Type achievements to unlock */
// 	uint32 BitfieldLength;
//
// };
//
// /**
// *	FOnlineAchievementPico - Interface class for accessing the oculus achievement information
// */
// struct FOnlineAchievementPico : FOnlineAchievement
// {
// 	FOnlineAchievementPico(const ppfAchievementProgressHandle& AchievementProgress) :
// 		Count(ppf_AchievementProgress_GetCount(AchievementProgress)),
// 		Bitfield(ppf_AchievementProgress_GetBitfield(AchievementProgress)),
// 		bIsUnlocked(ppf_AchievementProgress_GetIsUnlocked(AchievementProgress))
// 	{
// 		FString AchievementName(ppf_AchievementProgress_GetName(AchievementProgress));
// 		Id = AchievementName;
// 	}
//
// 	FOnlineAchievementPico(const FOnlineAchievementDescPico& AchievementDesc) :
// 		Count(0),
// 		bIsUnlocked(false)
// 	{
// 		Id = AchievementDesc.Title.ToString();
// 		Progress = 0;
// 		if (AchievementDesc.Type == EAchievementType::Bitfield)
// 		{
// 			Bitfield = TEXT("");
// 			for (uint32 i = 0; i < AchievementDesc.BitfieldLength; ++i)
// 			{
// 				Bitfield.AppendChar('0');
// 			}
// 		}
// 	}
//
// 	/** The player's current progress toward a targeted numeric goal */
// 	uint64 Count;
//
// 	/** The player's current progress toward a set of goals that doesn't have to be completed in order */
// 	FString Bitfield;
//
// 	/** Whether or not this achievement was unlocked */
// 	bool bIsUnlocked;
//
// };
//
// /**
// *	IOnlineAchievements - Interface class for achievements
// */
// class FOnlineAchievementsPico : public IOnlineAchievements
// {
// private:
// 	
// 	/** Reference to the owning subsystem */
// 	FOnlineSubsystemPico& PicoSubsystem;
//
// 	/** Mapping of players to their achievements */
// 	TMap<FUniqueNetIdPico, TArray<FOnlineAchievement>> PlayerAchievements;
//
// 	/** Cached achievements (not player-specific) */
// 	TMap<FString, FOnlineAchievementDescPico> AchievementDescriptions;
//
// 	void GetWriteAchievementCountValue(FVariantData VariantData, uint64& OutData) const;
// 	void GetWriteAchievementBitfieldValue(FVariantData VariantData, FString& OutData, uint32 BitfieldLength) const;
// 	double CalculatePlayerAchievementProgress(const FOnlineAchievementPico Achievement);
//
// public:
//
// 	/**
// 	* Constructor
// 	*
// 	* @param InSubsystem - A reference to the owning subsystem
// 	*/
// 	FOnlineAchievementsPico(FOnlineSubsystemPico& InSubsystem);
//
// 	/**
// 	* Default destructor
// 	*/
// 	virtual ~FOnlineAchievementsPico();
//
// 	// Begin IOnlineAchievements interface
// 	virtual void WriteAchievements(const FUniqueNetId& PlayerId, FOnlineAchievementsWriteRef& WriteObject, const FOnAchievementsWrittenDelegate& Delegate = FOnAchievementsWrittenDelegate()) override;
// 	virtual void QueryAchievements(const FUniqueNetId& PlayerId, const FOnQueryAchievementsCompleteDelegate& Delegate = FOnQueryAchievementsCompleteDelegate()) override;
// 	virtual void QueryAchievementDescriptions(const FUniqueNetId& PlayerId, const FOnQueryAchievementsCompleteDelegate& Delegate = FOnQueryAchievementsCompleteDelegate()) override;
// 	virtual EOnlineCachedResult::Type GetCachedAchievement(const FUniqueNetId& PlayerId, const FString& AchievementId, FOnlineAchievement& OutAchievement) override;
// 	virtual EOnlineCachedResult::Type GetCachedAchievements(const FUniqueNetId& PlayerId, TArray<FOnlineAchievement> & OutAchievements) override;
// 	virtual EOnlineCachedResult::Type GetCachedAchievementDescription(const FString& AchievementId, FOnlineAchievementDesc& OutAchievementDesc) override;
// #if !UE_BUILD_SHIPPING
// 	virtual bool ResetAchievements(const FUniqueNetId& PlayerId) override;
// #endif // !UE_BUILD_SHIPPING
// 	// End IOnlineAchievements interface
// };
