// // Copyright Epic Games, Inc. All Rights Reserved.
//
// #include "OnlineAchievementsInterfacePico.h"
// #include "OnlineSubsystemPicoPrivate.h"
// #include "OnlineIdentityPico.h"
// #include "OnlineMessageTaskManagerPico.h"
// #include "OnlineSubsystemPicoPackage.h"
// #include "Templates/SharedPointer.h"
//
// class FOnlineMessageMultiTaskOculusWriteAchievements : public FOnlineMessageMultiTaskPico, public TSharedFromThis<FOnlineMessageMultiTaskOculusWriteAchievements>
// {
// private:
//
// 	FUniqueNetIdPico PlayerId;
// 	FOnlineAchievementsWriteRef WriteObject;
// 	FOnAchievementsWrittenDelegate AchievementDelegate;
//
// 	// private to force the use of FOnlineMessageMultiTaskOculusWriteAchievements::Create()
// 	FOnlineMessageMultiTaskOculusWriteAchievements(FOnlineSubsystemPico& InPicoSubsystem, const FUniqueNetIdPico& InPlayerId, FOnlineAchievementsWriteRef& InWriteObject, const FOnAchievementsWrittenDelegate& InAchievementDelegate)
// 		: FOnlineMessageMultiTaskPico(InPicoSubsystem, FOnlineMessageMultiTaskPico::FFinalizeDelegate::CreateRaw(this, &FOnlineMessageMultiTaskOculusWriteAchievements::Finalize))
// 		, PlayerId(InPlayerId)
// 		, WriteObject(InWriteObject)
// 		, AchievementDelegate(InAchievementDelegate)
// 	{}
//
// 	static TSet< TSharedRef<FOnlineMessageMultiTaskOculusWriteAchievements> > ActiveAchievementWriteTasks;
//
// PACKAGE_SCOPE:
//
// 	static TSharedRef<FOnlineMessageMultiTaskOculusWriteAchievements> Create(
// 		FOnlineSubsystemPico& InPicoSubsystem,
// 		const FUniqueNetIdPico& InPlayerId,
// 		FOnlineAchievementsWriteRef& InWriteObject,
// 		const FOnAchievementsWrittenDelegate& InAchievementDelegate)
// 	{
// 		TSharedRef<FOnlineMessageMultiTaskOculusWriteAchievements> NewTask = MakeShareable(new FOnlineMessageMultiTaskOculusWriteAchievements(InPicoSubsystem, InPlayerId, InWriteObject, InAchievementDelegate));
// 		ActiveAchievementWriteTasks.Add(NewTask);
//
// 		return NewTask;
// 	}
//
// 	void Finalize()
// 	{
// 		WriteObject->WriteState = (bDidAllRequestsFinishedSuccessfully) ? EOnlineAsyncTaskState::Done : EOnlineAsyncTaskState::Failed;
// 		AchievementDelegate.ExecuteIfBound(PlayerId, true);
//
// 		// this should delete this task object, make sure it happens last
// 		ActiveAchievementWriteTasks.Remove(AsShared());
// 	}
//
// 	static void ClearAllActiveTasks()
// 	{
// 		ActiveAchievementWriteTasks.Empty();
// 	}
// };
// TSet< TSharedRef<FOnlineMessageMultiTaskOculusWriteAchievements> > FOnlineMessageMultiTaskOculusWriteAchievements::ActiveAchievementWriteTasks;
//
// FOnlineAchievementsPico::FOnlineAchievementsPico(class FOnlineSubsystemPico& InSubsystem)
// : PicoSubsystem(InSubsystem)
// {
// }
//
// FOnlineAchievementsPico::~FOnlineAchievementsPico()
// {
// 	FOnlineMessageMultiTaskOculusWriteAchievements::ClearAllActiveTasks();
// }
//
// void FOnlineAchievementsPico::WriteAchievements(const FUniqueNetId& PlayerId, FOnlineAchievementsWriteRef& WriteObject, const FOnAchievementsWrittenDelegate& Delegate)
// {
// 	if (AchievementDescriptions.Num() == 0)
// 	{
// 		// we don't have achievements
// 		WriteObject->WriteState = EOnlineAsyncTaskState::Failed;
// 		Delegate.ExecuteIfBound(PlayerId, false);
// 		return;
// 	}
//
// 	auto LoggedInPlayerId = PicoSubsystem.GetIdentityInterface()->GetUniquePlayerId(0);
// 	if (!(LoggedInPlayerId.IsValid() && PlayerId == *LoggedInPlayerId))
// 	{
// 		UE_LOG_ONLINE_ACHIEVEMENTS(Error, TEXT("Can only write achievements for logged in player id"));
// 		WriteObject->WriteState = EOnlineAsyncTaskState::Failed;
// 		Delegate.ExecuteIfBound(PlayerId, false);
// 		return;
// 	}
//
// 	// Nothing to write
// 	if (WriteObject->Properties.Num() == 0)
// 	{
// 		WriteObject->WriteState = EOnlineAsyncTaskState::Done;
// 		Delegate.ExecuteIfBound(PlayerId, true);
// 		return;
// 	}
//
// 	WriteObject->WriteState = EOnlineAsyncTaskState::InProgress;
// 	TSharedRef<FOnlineMessageMultiTaskOculusWriteAchievements> MultiTask = FOnlineMessageMultiTaskOculusWriteAchievements::Create(PicoSubsystem, static_cast<FUniqueNetIdPico>(PlayerId), WriteObject, Delegate);
//
// 	// treat each achievement as unlocked
// 	for (FStatPropertyArray::TConstIterator It(WriteObject->Properties); It; ++It)
// 	{
// 		const FString AchievementId = It.Key().ToString();
// 		auto VariantData = It.Value();
//
// 		auto AchievementDesc = AchievementDescriptions.Find(AchievementId);
// 		if (AchievementDesc == nullptr)
// 		{
// 			WriteObject->WriteState = EOnlineAsyncTaskState::Failed;
// 			Delegate.ExecuteIfBound(PlayerId, false);
// 			return;
// 		}
//
// 		UE_LOG_ONLINE_ACHIEVEMENTS(Verbose, TEXT("WriteObject AchievementId: '%s'"), *AchievementId);
//
// 		ppfRequest RequestId = 0;
//
// 		switch (AchievementDesc->Type)
// 		{
// 			case EAchievementType::Simple:
// 			{
// 				RequestId = ppf_Achievements_Unlock(TCHAR_TO_ANSI(*AchievementId));
// 				break;
// 			}
// 			case EAchievementType::Count:
// 			{
// 				uint64 Count;
// 				GetWriteAchievementCountValue(VariantData, Count);
// 				RequestId = ppf_Achievements_AddCount(TCHAR_TO_ANSI(*AchievementId), Count, nullptr, 0); // todo extra data
// 				break;
// 			}
// 			case EAchievementType::Bitfield:
// 			{
// 				FString Bitfield;
// 				GetWriteAchievementBitfieldValue(VariantData, Bitfield, AchievementDesc->BitfieldLength);
// 				RequestId = ppf_Achievements_AddFields(TCHAR_TO_ANSI(*AchievementId), TCHAR_TO_ANSI(*Bitfield), nullptr, 0); // todo extra data
// 				break;
// 			}
// 			default:
// 			{
// 				UE_LOG_ONLINE_ACHIEVEMENTS(Warning, TEXT("Unknown achievement type"));
// 				break;
// 			}
// 		}
//
// 		if (RequestId != 0)
// 		{
// 			MultiTask->AddNewRequest(RequestId);
// 		}
// 	}
// };
//
// void FOnlineAchievementsPico::QueryAchievements(const FUniqueNetId& PlayerId, const FOnQueryAchievementsCompleteDelegate& Delegate)
// {
// 	auto LoggedInPlayerId = PicoSubsystem.GetIdentityInterface()->GetUniquePlayerId(0);
// 	if (!(LoggedInPlayerId.IsValid() && PlayerId == *LoggedInPlayerId))
// 	{
// 		UE_LOG_ONLINE_ACHIEVEMENTS(Error, TEXT("Can only query for logged in player id"));
// 		Delegate.ExecuteIfBound(PlayerId, false);
// 		return;
// 	}
//
// 	auto PicoPlayerId = static_cast<FUniqueNetIdPico>(PlayerId);
// 	PicoSubsystem.AddRequestDelegate(
// 		ppf_Achievements_GetAllProgress(),
// 		FPicoMessageOnCompleteDelegate::CreateLambda([this, PicoPlayerId, Delegate](ppfMessageHandle Message, bool bIsError)
// 	{
// 		if (bIsError)
// 		{
// 			Delegate.ExecuteIfBound(PicoPlayerId, false);
// 			return;
// 		}
//
// 		auto AchievementProgressArray = ppf_Message_GetAchievementProgressArray(Message);
// 		const size_t AchievementProgressNum = ppf_AchievementProgressArray_GetSize(AchievementProgressArray);
//
// 		// new array
// 		TArray<FOnlineAchievement> AchievementsForPlayer;
//
// 		// keep track of existing achievements
// 		TSet<FString> InProgressAchievements;
//
// 		for (size_t AchievementProgressIndex = 0; AchievementProgressIndex < AchievementProgressNum; ++AchievementProgressIndex)
// 		{
// 			auto AchievementProgress = ppf_AchievementProgressArray_GetElement(AchievementProgressArray, AchievementProgressIndex);
// 			FOnlineAchievementPico NewAchievement(AchievementProgress);
// 			NewAchievement.Progress = CalculatePlayerAchievementProgress(NewAchievement);
//
// 			AchievementsForPlayer.Add(NewAchievement);
// 			InProgressAchievements.Add(NewAchievement.Id);
// 		}
//
// 		// if there are any achievements that the player has not made any progress toward,
// 		// fill them out as empty achievements
// 		for (auto const &it : AchievementDescriptions)
// 		{
// 			auto bFoundAchievement = InProgressAchievements.Find(it.Key);
// 			if (bFoundAchievement == nullptr)
// 			{
// 				FOnlineAchievementPico NewAchievement(it.Value);
// 				AchievementsForPlayer.Add(NewAchievement);
// 			}
// 		}
//
// 		// should replace any already existing values
// 		PlayerAchievements.Add(PicoPlayerId, AchievementsForPlayer);
//
// 		Delegate.ExecuteIfBound(PicoPlayerId, true);
// 	}));
// }
//
// void FOnlineAchievementsPico::QueryAchievementDescriptions(const FUniqueNetId& PlayerId, const FOnQueryAchievementsCompleteDelegate& Delegate)
// {
// 	auto PicoPlayerId = static_cast<FUniqueNetIdPico>(PlayerId);
// 	PicoSubsystem.AddRequestDelegate(
// 		ppf_Achievements_GetAllDefinitions(),
// 		FPicoMessageOnCompleteDelegate::CreateLambda([this, PicoPlayerId, Delegate](ppfMessageHandle Message, bool bIsError)
// 	{
// 		if (bIsError)
// 		{
// 			Delegate.ExecuteIfBound(PicoPlayerId, false);
// 			return;
// 		}
//
// 		auto AchievementDefinitionArray = ppf_Message_GetAchievementDefinitionArray(Message);
// 		const size_t AchievementDefinitionNum = ppf_AchievementDefinitionArray_GetSize(AchievementDefinitionArray);
// 		for (size_t AchievementDefinitionIndex = 0; AchievementDefinitionIndex < AchievementDefinitionNum; ++AchievementDefinitionIndex)
// 		{
// 			auto AchievementDefinition = ppf_AchievementDefinitionArray_GetElement(AchievementDefinitionArray, AchievementDefinitionIndex);
// 			FOnlineAchievementDescOculus NewAchievementDesc;
// 			FString Title(ppf_AchievementDefinition_GetName(AchievementDefinition));
// 			NewAchievementDesc.Title = FText::FromString(Title);
// 			NewAchievementDesc.bIsHidden = false;
// 			auto AchievementType = ppf_AchievementDefinition_GetType(AchievementDefinition);
// 			NewAchievementDesc.Type = static_cast<EAchievementType>(AchievementType);
// 			NewAchievementDesc.Target = ppf_AchievementDefinition_GetTarget(AchievementDefinition);
// 			NewAchievementDesc.BitfieldLength = ppf_AchievementDefinition_GetBitfieldLength(AchievementDefinition);
//
// 			AchievementDescriptions.Add(Title, NewAchievementDesc);
// 		}
//
// 		Delegate.ExecuteIfBound(PicoPlayerId, true);
// 	}));
// }
//
// EOnlineCachedResult::Type FOnlineAchievementsPico::GetCachedAchievement(const FUniqueNetId& PlayerId, const FString& AchievementId, FOnlineAchievement& OutAchievement)
// {
// 	if (AchievementDescriptions.Num() == 0)
// 	{
// 		// we don't have achievements
// 		return EOnlineCachedResult::NotFound;
// 	}
//
// 	auto PicoPlayerId = static_cast<const FUniqueNetIdPico&>(PlayerId);
// 	const TArray<FOnlineAchievement> * PlayerAch = PlayerAchievements.Find(PicoPlayerId);
// 	if (PlayerAch == nullptr)
// 	{
// 		// achievements haven't been read for a player
// 		return EOnlineCachedResult::NotFound;
// 	}
//
// 	const int32 AchNum = PlayerAch->Num();
// 	for (int32 AchIdx = 0; AchIdx < AchNum; ++AchIdx)
// 	{
// 		if ((*PlayerAch)[AchIdx].Id == AchievementId)
// 		{
// 			OutAchievement = (*PlayerAch)[AchIdx];
// 			return EOnlineCachedResult::Success;
// 		}
// 	}
//
// 	// no such achievement
// 	return EOnlineCachedResult::NotFound;
// };
//
// EOnlineCachedResult::Type FOnlineAchievementsPico::GetCachedAchievements(const FUniqueNetId& PlayerId, TArray<FOnlineAchievement> & OutAchievements)
// {
// 	if (AchievementDescriptions.Num() == 0)
// 	{
// 		// we don't have achievements
// 		return EOnlineCachedResult::NotFound;
// 	}
//
// 	auto PicoPlayerId = static_cast<const FUniqueNetIdPico&>(PlayerId);
// 	const TArray<FOnlineAchievement> * PlayerAch = PlayerAchievements.Find(PicoPlayerId);
// 	if (PlayerAch == nullptr)
// 	{
// 		// achievements haven't been read for a player
// 		return EOnlineCachedResult::NotFound;
// 	}
//
// 	OutAchievements = *PlayerAch;
// 	return EOnlineCachedResult::Success;
// };
//
// EOnlineCachedResult::Type FOnlineAchievementsPico::GetCachedAchievementDescription(const FString& AchievementId, FOnlineAchievementDesc& OutAchievementDesc)
// {
// 	if (AchievementDescriptions.Num() == 0)
// 	{
// 		// we don't have achievements
// 		return EOnlineCachedResult::NotFound;
// 	}
//
// 	FOnlineAchievementDesc * AchDesc = AchievementDescriptions.Find(AchievementId);
// 	if (AchDesc == nullptr)
// 	{
// 		// no such achievement
// 		return EOnlineCachedResult::NotFound;
// 	}
//
// 	OutAchievementDesc = *AchDesc;
// 	return EOnlineCachedResult::Success;
// };
//
// #if !UE_BUILD_SHIPPING
// bool FOnlineAchievementsPico::ResetAchievements(const FUniqueNetId& PlayerId)
// {
// 	// We cannot reset achievements from the client
// 	UE_LOG_ONLINE_ACHIEVEMENTS(Error, TEXT("Achievements cannot be reset here"));
// 	return false;
// };
// #endif // !UE_BUILD_SHIPPING
//
// void FOnlineAchievementsPico::GetWriteAchievementCountValue(FVariantData VariantData, uint64& OutData) const
// {
// 	switch (VariantData.GetType())
// 	{
// 		case EOnlineKeyValuePairDataType::Int32:
// 		{
// 			int32 Value;
// 			VariantData.GetValue(Value);
// 			OutData = static_cast<uint64>(Value);
// 			break;
// 		}
// 		case EOnlineKeyValuePairDataType::Int64:
// 		{
// 			int64 Value;
// 			VariantData.GetValue(Value);
// 			OutData = static_cast<uint64>(Value);
// 			break;
// 		}
// 		case EOnlineKeyValuePairDataType::UInt32:
// 		{
// 			uint32 Value;
// 			VariantData.GetValue(Value);
// 			OutData = static_cast<uint64>(Value);
// 			break;
// 		}
// 		case EOnlineKeyValuePairDataType::UInt64:
// 		{
// 			VariantData.GetValue(OutData);
// 			break;
// 		}
// 		default:
// 		{
// 			UE_LOG_ONLINE_ACHIEVEMENTS(Warning, TEXT("Could not %s convert to uint64"), VariantData.GetTypeString());
// 			OutData = 0;
// 			break;
// 		}
// 	}
// }
// void FOnlineAchievementsPico::GetWriteAchievementBitfieldValue(FVariantData VariantData, FString& OutData, uint32 BitfieldLength) const
// {
// 	switch (VariantData.GetType())
// 	{
// 		case EOnlineKeyValuePairDataType::Int32:
// 		{
// 			int32 Value;
// 			VariantData.GetValue(Value);
// 			auto UnpaddedBitfield = FString::FromInt(Value);
// 			auto PaddingLength = BitfieldLength - UnpaddedBitfield.Len();
// 			OutData = TEXT("");
// 			for (uint32 i = 0; i < PaddingLength; ++i)
// 			{
// 				OutData.AppendChar('0');
// 			}
// 			OutData.Append(UnpaddedBitfield);
// 			break;
// 		}
// 		case EOnlineKeyValuePairDataType::String:
// 		{
// 			VariantData.GetValue(OutData);
// 			break;
// 		}
// 		default:
// 		{
// 			UE_LOG_ONLINE_ACHIEVEMENTS(Warning, TEXT("Could not %s convert to string"), VariantData.GetTypeString());
// 			break;
// 		}
// 	}
// }
//
// double FOnlineAchievementsPico::CalculatePlayerAchievementProgress(const FOnlineAchievementOculus Achievement)
// {
// 	if (Achievement.bIsUnlocked)
// 	{
// 		return 100.0;
// 	}
//
// 	auto Desc = AchievementDescriptions.Find(Achievement.Id);
// 	if (Desc == nullptr)
// 	{
// 		UE_LOG_ONLINE_ACHIEVEMENTS(Warning, TEXT("Could not calculate progress for Achievement: '%s'"), *Achievement.Id);
// 		return 0.0;
// 	}
//
// 	double Progress;
// 	switch (Desc->Type)
// 	{
// 		case EAchievementType::Count:
// 		{
// 			Progress = Achievement.Count * 100.0 / Desc->Target;
// 			break;
// 		}
// 		case EAchievementType::Bitfield:
// 		{
// 			int BitfieldCount = 0;
// 			for (int32 i = 0; i < Achievement.Bitfield.Len(); ++i)
// 			{
// 				if (Achievement.Bitfield[i] == '1')
// 				{
// 					++BitfieldCount;
// 				}
// 			}
// 			Progress = BitfieldCount * 100.0 / Desc->Target;
// 			break;
// 		}
// 		default:
// 		{
// 			Progress = 0.0;
// 			break;
// 		}
// 	}
//
// 	// Cap the progress to 100
// 	return (Progress <= 100.0) ? Progress : 100.0;
// }
