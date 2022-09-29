// Copyright 2022 Pico Technology Co., Ltd.All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc.In the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2022, Epic Games, Inc.All rights reserved.

#include "ApplicationLifecycleInterface.h"
#include "Pico_User.h"
DEFINE_LOG_CATEGORY(ApplicationLifecycleInterface);
FApplicationLifecycleInterface::FApplicationLifecycleInterface(FOnlineSubsystemPico& InSubsystem) :
    PicoSubsystem(InSubsystem)
{
    OnLaunchIntentChangedNotificationHandle =
        PicoSubsystem.GetOrAddNotify(ppfMessageType_Notification_ApplicationLifecycle_LaunchIntentChanged)
        .AddRaw(this, &FApplicationLifecycleInterface::OnLaunchIntentChangedNotification);
}

FApplicationLifecycleInterface::~FApplicationLifecycleInterface()
{
    if (OnLaunchIntentChangedNotificationHandle.IsValid())
    {
        OnLaunchIntentChangedNotificationHandle.Reset();
    }
}



bool FApplicationLifecycleInterface::ReadLaunchDetails()
{
    FLaunchDetails NewLaunchDetails;
    LaunchDetails = NewLaunchDetails;
#if PLATFORM_ANDROID
    auto LaunchDetailsHandle = ppf_ApplicationLifecycle_GetLaunchDetails();
    auto UserArrayHandle = ppf_LaunchDetails_GetUsers(LaunchDetailsHandle);
    UPico_UserArray* UserArray = NewObject<UPico_UserArray>();
    UserArray->InitParams(UserArrayHandle);
    LaunchDetails.UserArray = UserArray;
    LaunchDetails.DeeplinkMessage = UTF8_TO_TCHAR(ppf_LaunchDetails_GetDeeplinkMessage(LaunchDetailsHandle));
    LaunchDetails.DestinationApiName = UTF8_TO_TCHAR(ppf_LaunchDetails_GetDestinationApiName(LaunchDetailsHandle));
    LaunchDetails.LaunchSource = UTF8_TO_TCHAR(ppf_LaunchDetails_GetLaunchSource(LaunchDetailsHandle));
    LaunchDetails.LobbySessionID = UTF8_TO_TCHAR(ppf_LaunchDetails_GetLobbySessionID(LaunchDetailsHandle));
    LaunchDetails.MatchSessionID = UTF8_TO_TCHAR(ppf_LaunchDetails_GetMatchSessionID(LaunchDetailsHandle));
    LaunchDetails.Extra = UTF8_TO_TCHAR(ppf_LaunchDetails_GetExtra(LaunchDetailsHandle));
    ppfID ID = ppf_LaunchDetails_GetRoomID(LaunchDetailsHandle);
    FString IDstr = FString::Printf(TEXT("%llu"), ID);
    LaunchDetails.RoomID = IDstr;
    LaunchDetails.TrackingID = UTF8_TO_TCHAR(ppf_LaunchDetails_GetTrackingID(LaunchDetailsHandle));
    ELaunchType LaunchType = ELaunchType::Unknown;
    ppfLaunchType pLaunchType = ppf_LaunchDetails_GetLaunchType(LaunchDetailsHandle);
    if (pLaunchType == ppfLaunchType_Normal)
    {
        LaunchType = ELaunchType::Normal;
    }
    else if (pLaunchType == ppfLaunchType_Invite)
    {
        LaunchType = ELaunchType::Invite;
    }
    else if (pLaunchType == ppfLaunchType_Coordinated)
    {
        LaunchType = ELaunchType::Coordinated;
    }
    else if (pLaunchType == ppfLaunchType_Deeplink)
    {
        LaunchType = ELaunchType::Deeplink;
    }
    LaunchDetails.LaunchType = LaunchType;


    return true;
#endif
    return false;
}

//void FApplicationLifecycleInterface::OnQueryReadLaunchDetailsComplete(ppfMessageHandle Message, ppfUserArrayHandle UserArrayHandle, bool bIsError, FLaunchDetails& OutLaunchDetails, TArray<FPicoUserInfo>& OutList, bool bAppendToExistingArray, const FOnReadDetailsComplete& Delegate)
//{
//#if PLATFORM_ANDROID
//    FString ErrorStr;
//    if (UserArrayHandle == nullptr)
//    {
//
//        if (bIsError)
//        {
//            auto Error = ppf_Message_GetError(Message);
//            auto ErrorMessage = ppf_Error_GetMessage(Error);
//            ErrorStr = UTF8_TO_TCHAR(ErrorMessage);
//            Delegate.ExecuteIfBound(false, ErrorStr);
//            return;
//        }
//        UserArrayHandle = ppf_Message_GetUserArray(Message);
//    }
//    auto UserNum = ppf_UserArray_GetSize(UserArrayHandle);
//    if (!bAppendToExistingArray)
//    {
//        OutList.Empty(UserNum);
//    }
//
//    for (size_t FriendIndex = 0; FriendIndex < UserNum; ++FriendIndex)
//    {
//        auto Friend = ppf_UserArray_GetElement(UserArrayHandle, FriendIndex);
//        FString FriendId = UTF8_TO_TCHAR(ppf_User_GetID(Friend));
//        FString FriendDisplayName = UTF8_TO_TCHAR(ppf_User_GetDisplayName(Friend));
//        auto FriendInviteToken = ppf_User_GetInviteToken(Friend);
//        FString FriendInviteTokenString(UTF8_TO_TCHAR((FriendInviteToken != nullptr) ? FriendInviteToken : ""));
//        FString SmallImageUrl = UTF8_TO_TCHAR(ppf_User_GetSmallImageUrl(Friend));
//        FString PresencePackage = UTF8_TO_TCHAR(ppf_User_GetPresencePackage(Friend));
//        FString ImageUrl = UTF8_TO_TCHAR(ppf_User_GetImageUrl(Friend));
//        ppfGender Gender = ppf_User_GetGender(Friend);
//        FString Presence = UTF8_TO_TCHAR(ppf_User_GetPresence(Friend));
//        FString PresenceDeeplinkMessage = UTF8_TO_TCHAR(ppf_User_GetPresenceDeeplinkMessage(Friend));
//        FString PresenceDestinationApiName = UTF8_TO_TCHAR(ppf_User_GetPresenceDestinationApiName(Friend));
//        FString PresenceLobbySessionId = UTF8_TO_TCHAR(ppf_User_GetPresenceLobbySessionId(Friend));
//        FString PresenceMatchSessionId = UTF8_TO_TCHAR(ppf_User_GetPresenceMatchSessionId(Friend));
//        auto FriendPresenceStatus = ppf_User_GetPresenceStatus(Friend);
//        FPicoUserInfo PicoFriend;
//        if (FriendPresenceStatus == ppfUserPresenceStatus_OffLine)
//        {
//            PicoFriend.FriendPresenceStatus = EPresenceStatus::OffLine;
//        }
//        else if (FriendPresenceStatus == ppfUserPresenceStatus_OnLine)
//        {
//            PicoFriend.FriendPresenceStatus = EPresenceStatus::OnLine;
//        }
//        else
//        {
//            PicoFriend.FriendPresenceStatus = EPresenceStatus::Unknow;
//        }
//        PicoFriend.DisPlayName = FriendDisplayName;
//        PicoFriend.UserId = FriendId;
//        OutList.Add(PicoFriend);
//    }
//    bool bHasPaging = ppf_UserArray_HasNextPage(UserArrayHandle);
//    if (bHasPaging)
//    {
//        PicoSubsystem.AddAsyncTask
//        (
//            ppf_User_GetNextUserArrayPage(ppf_UserArray_GetNextPageParam(UserArrayHandle)),
//            FPicoMessageOnCompleteDelegate::CreateLambda
//            (
//                [this, &OutLaunchDetails, &OutList, Delegate](ppfMessageHandle InMessage, bool bInIsError)
//                {
//                    OnQueryReadLaunchDetailsComplete(InMessage, nullptr, bInIsError, OutLaunchDetails, OutList, true, Delegate);
//                }
//            )
//        );
//    }
//    else
//    {
//        Delegate.ExecuteIfBound(true, ErrorStr);
//    }
//#endif
//
//}

bool FApplicationLifecycleInterface::GetLaunchDetails(FLaunchDetails& OutLaunchDetails)
{
    ReadLaunchDetails();
    OutLaunchDetails = LaunchDetails;
    return true;
}

bool FApplicationLifecycleInterface::LogDeeplinkResult(const FString& TrackingID, ELaunchResult LaunchResult)
{
#if PLATFORM_ANDROID
    ppfLaunchResult Result = ppfLaunchResult_Unknown;
    switch (LaunchResult)
    {
    case ELaunchResult::Unknown:
        Result = ppfLaunchResult_Unknown;
    	break;
    case ELaunchResult::Success:
        Result = ppfLaunchResult_Success;
        break;
    case ELaunchResult::FailedUserDeclined:
        Result = ppfLaunchResult_FailedUserDeclined;
        break;
    case ELaunchResult::FailedOtherReason:
        Result = ppfLaunchResult_FailedOtherReason;
        break;
    case ELaunchResult::FailedRoomNotFound:
        Result = ppfLaunchResult_FailedRoomNotFound;
        break;
    case ELaunchResult::FailedGameAlreadyStarted:
        Result = ppfLaunchResult_FailedGameAlreadyStarted;
        break;
    case ELaunchResult::FailedRoomFull:
        Result = ppfLaunchResult_FailedRoomFull;
        break;
    default:
        break;
    }
    ppf_ApplicationLifecycle_LogDeeplinkResult(TCHAR_TO_UTF8(*TrackingID), Result);
    return true;
#endif
    return false;
}

void FApplicationLifecycleInterface::OnLaunchIntentChangedNotification(ppfMessageHandle Message, bool bIsError)
{
#if PLATFORM_ANDROID
    if (bIsError)
    {
        UE_LOG(ApplicationLifecycleInterface, Error, TEXT("OnLaunchIntentChangedNotification error!"));
        return;
    }
    FString MessageString = UTF8_TO_TCHAR(ppf_Message_GetString(Message));
    LaunchIntentChangedCallback.Broadcast(MessageString);
#endif
}
