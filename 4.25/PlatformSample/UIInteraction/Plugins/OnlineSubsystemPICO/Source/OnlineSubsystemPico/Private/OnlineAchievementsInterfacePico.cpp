// Copyright Epic Games, Inc. All Rights Reserved.

#include "OnlineAchievementsInterfacePico.h"

//#include "DetailLayoutBuilder.h"
#include "OnlineSubsystemPicoPrivate.h"
#include "OnlineIdentityPico.h"
#include "OnlineMessageMultiTaskPico.h"
#include "OnlineSubsystemPicoPackage.h"
#include "Templates/SharedPointer.h"

class FOnlineMessageMultiTaskPicoWriteAchievements : public FOnlineMessageMultiTaskPico,
                                                     public TSharedFromThis<
	                                                     FOnlineMessageMultiTaskPicoWriteAchievements>
{
private:
	FUniqueNetIdPico PlayerId;
	FOnlineAchievementsWriteRef WriteObject;
	FOnAchievementsWrittenDelegate AchievementDelegate;

	// private to force the use of FOnlineMessageMultiTaskPicoWriteAchievements::Create()
	FOnlineMessageMultiTaskPicoWriteAchievements(FOnlineSubsystemPico& InPicoSubsystem,
	                                             const FUniqueNetIdPico& InPlayerId,
	                                             FOnlineAchievementsWriteRef& InWriteObject,
	                                             const FOnAchievementsWrittenDelegate& InAchievementDelegate)
		: FOnlineMessageMultiTaskPico(InPicoSubsystem,
		                              FOnlineMessageMultiTaskPico::FFinalizeDelegate::CreateRaw(
			                              this, &FOnlineMessageMultiTaskPicoWriteAchievements::Finalize))
		  , PlayerId(InPlayerId)
		  , WriteObject(InWriteObject)
		  , AchievementDelegate(InAchievementDelegate)
	{
	}

	static TSet<TSharedRef<FOnlineMessageMultiTaskPicoWriteAchievements>> ActiveAchievementWriteTasks;

PACKAGE_SCOPE:
	static TSharedRef<FOnlineMessageMultiTaskPicoWriteAchievements> Create(
		FOnlineSubsystemPico& InPicoSubsystem,
		const FUniqueNetIdPico& InPlayerId,
		FOnlineAchievementsWriteRef& InWriteObject,
		const FOnAchievementsWrittenDelegate& InAchievementDelegate)
	{
		TSharedRef<FOnlineMessageMultiTaskPicoWriteAchievements> NewTask = MakeShareable(
			new FOnlineMessageMultiTaskPicoWriteAchievements(InPicoSubsystem, InPlayerId, InWriteObject,
			                                                 InAchievementDelegate));
		ActiveAchievementWriteTasks.Add(NewTask);

		return NewTask;
	}

	void Finalize()
	{
		WriteObject->WriteState = (bDidAllRequestsFinishedSuccessfully)
			                          ? EOnlineAsyncTaskState::Done
			                          : EOnlineAsyncTaskState::Failed;
		AchievementDelegate.ExecuteIfBound(PlayerId, true);

		// this should delete this task object, make sure it happens last
		ActiveAchievementWriteTasks.Remove(AsShared());
	}

	static void ClearAllActiveTasks()
	{
		ActiveAchievementWriteTasks.Empty();
	}
};

TSet<TSharedRef<FOnlineMessageMultiTaskPicoWriteAchievements>>
FOnlineMessageMultiTaskPicoWriteAchievements::ActiveAchievementWriteTasks;

FOnlineAchievementsPico::FOnlineAchievementsPico(class FOnlineSubsystemPico& InSubsystem)
	: PicoSubsystem(InSubsystem)
{
}

FOnlineAchievementsPico::~FOnlineAchievementsPico()
{
	FOnlineMessageMultiTaskPicoWriteAchievements::ClearAllActiveTasks();
}

void FOnlineAchievementsPico::WriteAchievements(const FUniqueNetId& PlayerId, FOnlineAchievementsWriteRef& WriteObject,
                                                const FOnAchievementsWrittenDelegate& Delegate)
{
	if (AchievementDescriptions.Num() == 0)
	{
		// we don't have achievements
		WriteObject->WriteState = EOnlineAsyncTaskState::Failed;
		Delegate.ExecuteIfBound(PlayerId, false);
		return;
	}

	auto LoggedInPlayerId = PicoSubsystem.GetIdentityInterface()->GetUniquePlayerId(0);
	if (!(LoggedInPlayerId.IsValid() && PlayerId == *LoggedInPlayerId))
	{
		UE_LOG_ONLINE_ACHIEVEMENTS(Error, TEXT("Can only write achievements for logged in player id"));
		WriteObject->WriteState = EOnlineAsyncTaskState::Failed;
		Delegate.ExecuteIfBound(PlayerId, false);
		return;
	}

	// Nothing to write
	if (WriteObject->Properties.Num() == 0)
	{
		WriteObject->WriteState = EOnlineAsyncTaskState::Done;
		Delegate.ExecuteIfBound(PlayerId, true);
		return;
	}

	WriteObject->WriteState = EOnlineAsyncTaskState::InProgress;
	TSharedRef<FOnlineMessageMultiTaskPicoWriteAchievements> MultiTask =
		FOnlineMessageMultiTaskPicoWriteAchievements::Create(PicoSubsystem, static_cast<FUniqueNetIdPico>(PlayerId),
		                                                     WriteObject, Delegate);

	UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("WriteObject->Properties.Num(): %d"), WriteObject->Properties.Num());
	// treat each achievement as unlocked
	for (FStatPropertyArray::TConstIterator It(WriteObject->Properties); It; ++It)
	{
		const FString AchievementId = It.Key().ToString();
		auto VariantData = It.Value();

		auto AchievementDesc = AchievementDescriptions.Find(AchievementId);
		if (AchievementDesc == nullptr)
		{
			WriteObject->WriteState = EOnlineAsyncTaskState::Failed;
			Delegate.ExecuteIfBound(PlayerId, false);
			return;
		}

		UE_LOG_ONLINE_ACHIEVEMENTS(Verbose, TEXT("WriteObject AchievementId: '%s'"), *AchievementId);

		ppfRequest RequestId = 0;

		switch (AchievementDesc->Type)
		{
		case EAchievementType::Simple:
			{
				RequestId = ppf_Achievements_Unlock(TCHAR_TO_ANSI(*AchievementId), nullptr, 0);
				break;
			}
		case EAchievementType::Count:
			{
				uint64 Count;
				GetWriteAchievementCountValue(VariantData, Count);
				RequestId = ppf_Achievements_AddCount(TCHAR_TO_ANSI(*AchievementId), Count, nullptr, 0);
				break;
			}
		case EAchievementType::Bitfield:
			{
				FString Bitfield;
				GetWriteAchievementBitfieldValue(VariantData, Bitfield, AchievementDesc->BitfieldLength);
				RequestId = ppf_Achievements_AddFields(
					TCHAR_TO_ANSI(*AchievementId), TCHAR_TO_ANSI(*Bitfield), nullptr, 0);
				break;
			}
		default:
			{
				UE_LOG_ONLINE_ACHIEVEMENTS(Warning, TEXT("Unknown achievement type"));
				break;
			}
		}

		if (RequestId != 0)
		{
			MultiTask->AddNewRequest(RequestId);
		}
	}
};

void FOnlineAchievementsPico::QueryAchievements(const FUniqueNetId& PlayerId,
                                                const FOnQueryAchievementsCompleteDelegate& Delegate)
{
	auto LoggedInPlayerId = PicoSubsystem.GetIdentityInterface()->GetUniquePlayerId(0);
	if (!(LoggedInPlayerId.IsValid() && PlayerId == *LoggedInPlayerId))
	{
		GetAllProgressIndex = 0;
		UE_LOG_ONLINE_ACHIEVEMENTS(Error, TEXT("Can only query for logged in player id"));
		Delegate.ExecuteIfBound(PlayerId, false);
		return;
	}

	const FUniqueNetIdPico& PicoPlayerId = static_cast<const FUniqueNetIdPico&>(PlayerId);
    UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("QueryAchievements GetAllProgressIndex: %d, MAX_REQUEST_SIZE: %d, ppf_Achievements_GetAllProgress PicoPlayerId: %s"), GetAllProgressIndex, MAX_REQUEST_SIZE, *PicoPlayerId.ToString());
	PicoSubsystem.AddAsyncTask(
		ppf_Achievements_GetAllProgress(GetAllProgressIndex, MAX_REQUEST_SIZE),
		FPicoMessageOnCompleteDelegate::CreateLambda(
			[this, PicoPlayerId, Delegate](ppfMessageHandle Message, bool bIsError)
			{
				UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("QueryAchievements ppf_Achievements_GetAllProgress OnComplete"));
				if (bIsError)
				{
					GetAllProgressIndex = 0;
					UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("ppf_Achievements_GetAllProgress is error: true"));
					Delegate.ExecuteIfBound(PicoPlayerId, false);
					return;
				}

				auto AchievementProgressArray = ppf_Message_GetAchievementProgressArray(Message);
				const size_t AchievementProgressNum = ppf_AchievementProgressArray_GetSize(AchievementProgressArray);
				UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("ppf_AchievementProgressArray_GetSize: %zu"), AchievementProgressNum);
				// new array
				TArray<FOnlineAchievement> AchievementsForPlayer;

				// keep track of existing achievements
				TSet<FString> InProgressAchievements;

				for (size_t AchievementProgressIndex = 0; AchievementProgressIndex < AchievementProgressNum; ++AchievementProgressIndex)
				{
					auto AchievementProgress = ppf_AchievementProgressArray_GetElement(AchievementProgressArray, AchievementProgressIndex);
					FOnlineAchievementPico NewAchievement(AchievementProgress);
					NewAchievement.Progress = CalculatePlayerAchievementProgress(NewAchievement);

					AchievementsForPlayer.Add(NewAchievement);
					InProgressAchievements.Add(NewAchievement.Id);
					UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("%zu, Id: %s"), AchievementProgressIndex, *NewAchievement.Id);
				}

				// if there are any achievements that the player has not made any progress toward,
				// fill them out as empty achievements
				for (auto const& it : AchievementDescriptions)
				{
					auto bFoundAchievement = InProgressAchievements.Find(it.Key);
					if (bFoundAchievement == nullptr)
					{
						FOnlineAchievementPico NewAchievement(it.Value);
						AchievementsForPlayer.Add(NewAchievement);
					}
				}

				UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("PlayerAchievements add: PicoPlayerId: %s"), *PicoPlayerId.ToString());
				// should replace any already existing values
				PlayerAchievements.Add(PicoPlayerId, AchievementsForPlayer);

				GetAllProgressIndex++;
				size_t TotalSize = ppf_AchievementProgressArray_GetTotalSize(AchievementProgressArray);
				UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("TotalSize: %zu"), TotalSize);
				if (GetAllProgressIndex * MAX_REQUEST_SIZE >= TotalSize)
				{
					UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("GetAllProgressIndex * MAX_REQUEST_SIZE >= TotalSize"));
					GetAllProgressIndex = 0;
					Delegate.ExecuteIfBound(PicoPlayerId, true);
				}
				else
				{
					UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("GetAllProgressIndex: %d"), GetAllProgressIndex);
					QueryAchievements(PicoPlayerId, Delegate);
				}
			}));
}

void FOnlineAchievementsPico::QueryAchievementDescriptions(const FUniqueNetId& PlayerId,
                                                           const FOnQueryAchievementsCompleteDelegate& Delegate)
{
	const FUniqueNetIdPico& PicoPlayerId = static_cast<const FUniqueNetIdPico&>(PlayerId);
	UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("QueryAchievementDescriptions ppf_Achievements_GetAllDefinitions GetAllDefinitionIndex: %d, MAX_REQUEST_SIZE: %d,  PicoPlayerId: %s"), GetAllDefinitionIndex, MAX_REQUEST_SIZE, *PicoPlayerId.ToString());
	PicoSubsystem.AddAsyncTask(
		ppf_Achievements_GetAllDefinitions(GetAllDefinitionIndex, MAX_REQUEST_SIZE),
		FPicoMessageOnCompleteDelegate::CreateLambda(
			[this, PicoPlayerId, Delegate](ppfMessageHandle Message, bool bIsError)
			{
				UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("QueryAchievementDescriptions ppf_Achievements_GetAllDefinitions OnComplete"));
				if (bIsError)
				{
					GetAllDefinitionIndex = 0;
					UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("QueryAchievementDescriptions is error"));
					Delegate.ExecuteIfBound(PicoPlayerId, false);
					return;
				}

				const auto AchievementDefinitionArray = ppf_Message_GetAchievementDefinitionArray(Message);
				const size_t AchievementDefinitionNum = ppf_AchievementDefinitionArray_GetSize(AchievementDefinitionArray);
				UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("QueryAchievementDescriptions AchievementDefinitionNum: %zu"), AchievementDefinitionNum);
				for (size_t AchievementDefinitionIndex = 0; AchievementDefinitionIndex < AchievementDefinitionNum; ++AchievementDefinitionIndex)
				{
					const auto AchievementDefinition = ppf_AchievementDefinitionArray_GetElement(AchievementDefinitionArray, AchievementDefinitionIndex);
					FOnlineAchievementDescPico NewAchievementDesc;
					FString Title(UTF8_TO_TCHAR(ppf_AchievementDefinition_GetTitle(AchievementDefinition)));
					NewAchievementDesc.Title = FText::FromString(Title);
					auto AchievementType = ppf_AchievementDefinition_GetType(AchievementDefinition);
					NewAchievementDesc.Type = static_cast<EAchievementType>(AchievementType);
					NewAchievementDesc.Target = ppf_AchievementDefinition_GetTarget(AchievementDefinition);
					NewAchievementDesc.BitfieldLength = ppf_AchievementDefinition_GetBitfieldLength(AchievementDefinition);
					NewAchievementDesc.WritePolicy = static_cast<EAchievementWritePolicy>(ppf_AchievementDefinition_GetWritePolicy(AchievementDefinition));
					// FString Name(UTF8_TO_TCHAR(ppf_AchievementDefinition_GetName(AchievementDefinition)));
					NewAchievementDesc.Name = UTF8_TO_TCHAR(ppf_AchievementDefinition_GetName(AchievementDefinition));
					NewAchievementDesc.IsArchived = ppf_AchievementDefinition_IsArchived(AchievementDefinition);
					NewAchievementDesc.bIsHidden = ppf_AchievementDefinition_IsSecret(AchievementDefinition);
					NewAchievementDesc.LockedDesc = FText::FromString(UTF8_TO_TCHAR(ppf_AchievementDefinition_GetDescription(AchievementDefinition)));
					NewAchievementDesc.UnlockedDesc = FText::FromString(UTF8_TO_TCHAR(ppf_AchievementDefinition_GetUnlockedDescription(AchievementDefinition)));
					NewAchievementDesc.LockedImageURL = UTF8_TO_TCHAR(ppf_AchievementDefinition_GetLockedImageURL(AchievementDefinition));
					NewAchievementDesc.UnlockedImageURL = UTF8_TO_TCHAR(ppf_AchievementDefinition_GetUnlockedImageURL(AchievementDefinition));
					
					UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("QueryAchievementDescriptions AchievementDescriptions add Name: %s, DebugString: %s")
						, *NewAchievementDesc.Name, *NewAchievementDesc.ToDebugString());
					AchievementDescriptions.Add(NewAchievementDesc.Name, NewAchievementDesc);
				}
				UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("QueryAchievementDescriptions AchievementDescriptions.Num(): %d"), AchievementDescriptions.Num());

				GetAllDefinitionIndex++;
				size_t TotalSize = ppf_AchievementDefinitionArray_GetTotalSize(AchievementDefinitionArray);
				UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("TotalSize: %zu"), TotalSize);
				if (GetAllDefinitionIndex * MAX_REQUEST_SIZE >= TotalSize)
				{
					UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("GetAllDefinitionIndex * MAX_REQUEST_SIZE >= TotalSize"));
					GetAllDefinitionIndex = 0;
					Delegate.ExecuteIfBound(PicoPlayerId, true);
				}
				else
				{
					UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("GetAllDefinitionIndex: %d"), GetAllDefinitionIndex);
					QueryAchievementDescriptions(PicoPlayerId, Delegate);
				}
			}));
}

EOnlineCachedResult::Type FOnlineAchievementsPico::GetCachedAchievement(const FUniqueNetId& PlayerId,
                                                                        const FString& AchievementId,
                                                                        FOnlineAchievement& OutAchievement)
{
	if (AchievementDescriptions.Num() == 0)
	{
		UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("GetCachedAchievement AchievementDescriptions.Num() is 0"));
		// we don't have achievements
		return EOnlineCachedResult::NotFound;
	}

	const FUniqueNetIdPico& PicoPlayerId = static_cast<const FUniqueNetIdPico&>(PlayerId);
	const TArray<FOnlineAchievement>* PlayerAch = PlayerAchievements.Find(PicoPlayerId);
	if (PlayerAch == nullptr)
	{
		UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("GetCachedAchievement achievements haven't been read for a player"));
		// achievements haven't been read for a player
		return EOnlineCachedResult::NotFound;
	}

	const int32 AchNum = PlayerAch->Num();
	for (int32 AchIdx = 0; AchIdx < AchNum; ++AchIdx)
	{
		UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("GetCachedAchievement (*PlayerAch)[AchIdx].Id: %s, AchievementId: %s"), *((*PlayerAch)[AchIdx].Id), *AchievementId);
		if ((*PlayerAch)[AchIdx].Id == AchievementId)
		{
			OutAchievement = (*PlayerAch)[AchIdx];
			return EOnlineCachedResult::Success;
		}
	}

	UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("GetCachedAchievement no such achievement"));
	// no such achievement
	return EOnlineCachedResult::NotFound;
};

EOnlineCachedResult::Type FOnlineAchievementsPico::GetCachedAchievements(
	const FUniqueNetId& PlayerId, TArray<FOnlineAchievement>& OutAchievements)
{
	if (AchievementDescriptions.Num() == 0)
	{
		// we don't have achievements
		return EOnlineCachedResult::NotFound;
	}

	const FUniqueNetIdPico& PicoPlayerId = static_cast<const FUniqueNetIdPico&>(PlayerId);
	const TArray<FOnlineAchievement>* PlayerAch = PlayerAchievements.Find(PicoPlayerId);
	if (PlayerAch == nullptr)
	{
		// achievements haven't been read for a player
		return EOnlineCachedResult::NotFound;
	}

	OutAchievements = *PlayerAch;
	return EOnlineCachedResult::Success;
};

EOnlineCachedResult::Type FOnlineAchievementsPico::GetCachedAchievementDescription(
	const FString& AchievementId, FOnlineAchievementDesc& OutAchievementDesc)
{
	if (AchievementDescriptions.Num() == 0)
	{
		// we don't have achievements
		return EOnlineCachedResult::NotFound;
	}

	FOnlineAchievementDescPico* AchDesc = AchievementDescriptions.Find(AchievementId);
	if (AchDesc == nullptr)
	{
		// no such achievement
		return EOnlineCachedResult::NotFound;
	}

	if (OutAchievementDesc.Title.EqualTo(FText::FromName(USE_PICO_DESC)))
	{
		FOnlineAchievementDescPico& temp = static_cast<FOnlineAchievementDescPico&>(OutAchievementDesc);
		temp = *AchDesc;
	}
	else
	{
		OutAchievementDesc.Title = FText::FromString(AchDesc->Name);
		OutAchievementDesc.bIsHidden = AchDesc->bIsHidden;
		OutAchievementDesc.LockedDesc = AchDesc->LockedDesc;
		OutAchievementDesc.UnlockedDesc = AchDesc->UnlockedDesc;
	}
	return EOnlineCachedResult::Success;
};

#if !UE_BUILD_SHIPPING
bool FOnlineAchievementsPico::ResetAchievements(const FUniqueNetId& PlayerId)
{
	// We cannot reset achievements from the client
	UE_LOG_ONLINE_ACHIEVEMENTS(Error, TEXT("Achievements cannot be reset here"));
	return false;
};
#endif // !UE_BUILD_SHIPPING

void FOnlineAchievementsPico::GetWriteAchievementCountValue(FVariantData VariantData, uint64& OutData) const
{
	switch (VariantData.GetType())
	{
	case EOnlineKeyValuePairDataType::Int32:
		{
			int32 Value;
			VariantData.GetValue(Value);
			OutData = static_cast<uint64>(Value);
			break;
		}
	case EOnlineKeyValuePairDataType::Int64:
		{
			int64 Value;
			VariantData.GetValue(Value);
			OutData = static_cast<uint64>(Value);
			break;
		}
	case EOnlineKeyValuePairDataType::UInt32:
		{
			uint32 Value;
			VariantData.GetValue(Value);
			OutData = static_cast<uint64>(Value);
			break;
		}
	case EOnlineKeyValuePairDataType::UInt64:
		{
			VariantData.GetValue(OutData);
			break;
		}
	default:
		{
			UE_LOG_ONLINE_ACHIEVEMENTS(Warning, TEXT("Could not %s convert to uint64"), VariantData.GetTypeString());
			OutData = 0;
			break;
		}
	}
}

void FOnlineAchievementsPico::GetWriteAchievementBitfieldValue(FVariantData VariantData, FString& OutData,
                                                               uint32 BitfieldLength) const
{
	switch (VariantData.GetType())
	{
	case EOnlineKeyValuePairDataType::Int32:
		{
			int32 Value;
			VariantData.GetValue(Value);
			auto UnpaddedBitfield = FString::FromInt(Value);
			auto PaddingLength = BitfieldLength - UnpaddedBitfield.Len();
			OutData = TEXT("");
			for (uint32 i = 0; i < PaddingLength; ++i)
			{
				OutData.AppendChar('0');
			}
			OutData.Append(UnpaddedBitfield);
			break;
		}
	case EOnlineKeyValuePairDataType::String:
		{
			VariantData.GetValue(OutData);
			break;
		}
	default:
		{
			UE_LOG_ONLINE_ACHIEVEMENTS(Warning, TEXT("Could not %s convert to string"), VariantData.GetTypeString());
			break;
		}
	}
}

double FOnlineAchievementsPico::CalculatePlayerAchievementProgress(const FOnlineAchievementPico Achievement)
{
	if (Achievement.bIsUnlocked)
	{
		UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("Id: %s is unlocked."), *Achievement.Id);
		return 100.0;
	}

	auto Desc = AchievementDescriptions.Find(Achievement.Id);
	if (Desc == nullptr)
	{
		UE_LOG_ONLINE_ACHIEVEMENTS(Warning, TEXT("Could not calculate progress for Achievement: '%s'"), *Achievement.Id);
		return 0.0;
	}

	double Progress;
	switch (Desc->Type)
	{
	case EAchievementType::Count:
		{
			Progress = Achievement.Count * 100.0 / Desc->Target;
			break;
		}
	case EAchievementType::Bitfield:
		{
			int BitfieldCount = 0;
			for (int32 i = 0; i < Achievement.Bitfield.Len(); ++i)
			{
				if (Achievement.Bitfield[i] == '1')
				{
					++BitfieldCount;
				}
			}
			Progress = BitfieldCount * 100.0 / Desc->Target;
			break;
		}
	default:
		{
			Progress = 0.0;
			break;
		}
	}
	UE_LOG_ONLINE_ACHIEVEMENTS(Display, TEXT("Id: %s, Progress: %f"), *Achievement.Id, (Progress <= 100.0) ? Progress : 100.0);
	// Cap the progress to 100
	return (Progress <= 100.0) ? Progress : 100.0;
}
