// Fill out your copyright notice in the Description page of Project Settings.


#include "PicoSocialGameInstanceSubsystem.h"
#include "OnlineSubsystem.h"

UPicoSocialGameInstanceSubsystem::UPicoSocialGameInstanceSubsystem()
    : UGameInstanceSubsystem()
{

}

void UPicoSocialGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
    {
        FOnlineSubsystemPico* PicoSubsystem = static_cast<FOnlineSubsystemPico*>(Subsystem);
        if (PicoSubsystem && PicoSubsystem->Init())
        {
            PicoPresenceInterface = PicoSubsystem->GetPicoPresenceInterface();
            PicoUserInterface = PicoSubsystem->GetPicoUserInterface();
            PicoRoomInterface = PicoSubsystem->GetPicoRoomInterface();
            PicoChallengesInterface = PicoSubsystem->GetPicoChallengesInterface();
            PicoApplicationInterface = PicoSubsystem->GetApplicationInterface();
            if (PicoPresenceInterface)
            {
                PicoPresenceInterface->JoinIntentReceivedCallback.AddUObject(this, &UPicoSocialGameInstanceSubsystem::OnJoinIntentChanged);
            }
        }
    }
}

void UPicoSocialGameInstanceSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UPicoSocialGameInstanceSubsystem::OnJoinIntentChanged(const FString& DeeplinkMessage, const FString& DestinationApiName, const FString& LobbySessionId, const FString& MatchSessionId, const FString& Extra)
{
    UE_LOG(LogTemp, Log, TEXT("DeeplinkMessage: %s, DestinationApiName: %s, LobbySessionId: %s, MatchSessionId: %s, Extra: %s.!"), *DeeplinkMessage, *DestinationApiName, *LobbySessionId, *MatchSessionId, *Extra);
}

void UPicoSocialGameInstanceSubsystem::PicoSetPresence(const FString& ApiName, const FString& LobbySessionId, const FString& MatchSessionId, bool bIsJoinable, const FString& Extra)
{
    if (!PicoPresenceInterface)
    {
        return;
    }
    PresenceSetDelegate.Unbind();
    PresenceSetDelegate.BindUObject(this, &UPicoSocialGameInstanceSubsystem::OnPresenceSetComplete);
    PicoPresenceInterface->PresenceSet(ApiName, LobbySessionId, MatchSessionId, bIsJoinable, Extra, PresenceSetDelegate);
}

void UPicoSocialGameInstanceSubsystem::OnPresenceSetComplete(bool bIsSuccessed, const FString& ErrorMessage)
{
    if (!bIsSuccessed)
    {
        UE_LOG(LogTemp, Log, TEXT("Error Message: %s"), *ErrorMessage);
    }
    UE_LOG(LogTemp, Log, TEXT("OnPresenceSetComplete successed"));
}

void UPicoSocialGameInstanceSubsystem::PicoClearPresence()
{
    if (!PicoPresenceInterface)
    {
        return;
    }
    PresenceClearDelegate.Unbind();
    PresenceClearDelegate.BindUObject(this, &UPicoSocialGameInstanceSubsystem::OnPicoClearPresenceComplete);
    PicoPresenceInterface->PresenceClear(PresenceClearDelegate);
}

void UPicoSocialGameInstanceSubsystem::OnPicoClearPresenceComplete(bool bIsSuccessed, const FString& ErrorMessage)
{
    if (!bIsSuccessed)
    {
        UE_LOG(LogTemp, Log, TEXT("Error Message: %s"), *ErrorMessage);
    }
    UE_LOG(LogTemp, Log, TEXT("OnPicoClearPresenceComplete successed"));
}

void UPicoSocialGameInstanceSubsystem::PicoGetUserFriends()
{
    if (!PicoUserInterface)
    {   
        return;
    }
    GetLoggedInUserFriendsDelegate.Clear();
    GetLoggedInUserFriendsDelegate.BindDynamic(this, &UPicoSocialGameInstanceSubsystem::OnGetUserFriendsComplete);
    PicoUserInterface->GetUserFriends(GetLoggedInUserFriendsDelegate);
}

void UPicoSocialGameInstanceSubsystem::OnGetUserFriendsComplete(bool bIsError, const FString& ErrorMessage, UPico_UserArray* UserArray)
{
    if (bIsError)
    {
        UE_LOG(LogTemp, Log, TEXT("Error Message: %s"), *ErrorMessage);
    }
    UE_LOG(LogTemp, Log, TEXT("OnPresenceSetComplete successed"));
    if (UserArray->GetSize() > 0)
    {
        for (auto i = 0; i < UserArray->GetSize(); i++)
        {
            UserArray->GetElement(i)->GetPresence();
        }
    }
}

void UPicoSocialGameInstanceSubsystem::PicoGetUserInfo(const FString& UserID)
{
    if (!PicoUserInterface)
    {
        return;
    }
    GetUserInfoDelegate.Clear();
    GetUserInfoDelegate.BindDynamic(this, &UPicoSocialGameInstanceSubsystem::OnPicoGetUserInfoComplete);
    PicoUserInterface->GetUserInfo(UserID, GetUserInfoDelegate);
}

void UPicoSocialGameInstanceSubsystem::OnPicoGetUserInfoComplete(bool bIsError, const FString& ErrorMessage, UPico_User* User)
{
    if (bIsError)
    {
        UE_LOG(LogTemp, Log, TEXT("Error Message: %s"), *ErrorMessage);
    }
    UE_LOG(LogTemp, Log, TEXT("OnPicoGetUserInfoComplete successed"));
    User->GetPresence();
}

void UPicoSocialGameInstanceSubsystem::PresenceInviteUsers(TArray<FString> UserIdArray)
{
    if (!PicoPresenceInterface)
    {
        return;
    }
    PresenceSendInviteDelegate.Unbind();
    PresenceSendInviteDelegate.BindUObject(this, &UPicoSocialGameInstanceSubsystem::OnPresenceSendInvitesComplete);
    PicoPresenceInterface->PresenceSendInvites(UserIdArray, PresenceSendInviteDelegate);
}

void UPicoSocialGameInstanceSubsystem::OnPresenceSendInvitesComplete(bool bIsSuccessed, const FString& ErrorMessage)
{
    if (!bIsSuccessed)
    {
        UE_LOG(LogTemp, Log, TEXT("Error Message: %s"), *ErrorMessage);
    }
    UE_LOG(LogTemp, Log, TEXT("OnPresenceSendInvitesComplete successed"));
}

void UPicoSocialGameInstanceSubsystem::LaunchPicoInvitePanel()
{
    if (!PicoPresenceInterface)
    {
        return;
    }
    LaunchInvitePanelDelegate.Unbind();
    LaunchInvitePanelDelegate.BindUObject(this, &UPicoSocialGameInstanceSubsystem::OnLaunchInvitePanelComplete);
    PicoPresenceInterface->LaunchInvitePanel(LaunchInvitePanelDelegate);

}

void UPicoSocialGameInstanceSubsystem::OnLaunchInvitePanelComplete(bool bIsSuccessed, const FString& ErrorMessage)
{
    if (!bIsSuccessed)
    {
        UE_LOG(LogTemp, Log, TEXT("Error Message: %s"), *ErrorMessage);
    }
    UE_LOG(LogTemp, Log, TEXT("OnLaunchInvitePanelComplete successed"));
}

void UPicoSocialGameInstanceSubsystem::PicoInviteUserJoinPriviteRoom(UPico_User* User, UPico_Room* PrivateRoom)
{
    if (!PicoRoomInterface)
    {
        return;
    }
    InviteUserDelegate.Clear();
    InviteUserDelegate.BindDynamic(this, &UPicoSocialGameInstanceSubsystem::OnPicoInviteUserJoinPriviteRoomComplete);
    PicoRoomInterface->InviteUser(PrivateRoom->GetRoomID(), User->GetInviteToken(), InviteUserDelegate);
}

void UPicoSocialGameInstanceSubsystem::OnPicoInviteUserJoinPriviteRoomComplete(bool bIsError, int ErrorCode, const FString& ErrorMessage, UPico_Room* Room)
{
    if (bIsError)
    {
        UE_LOG(LogTemp, Log, TEXT("Error Message: %s"), *ErrorMessage);
    }
    UE_LOG(LogTemp, Log, TEXT("OnPicoInviteUserJoinPriviteRoomComplete successed"));
}

void UPicoSocialGameInstanceSubsystem::ChallengeInvite(const FString& ChallengeID, const TArray<FString>& UserIDs)
{
    if (!PicoChallengesInterface)
    {
        return;
    }
    ChallengeInviteDelegate.Clear();
    ChallengeInviteDelegate.BindDynamic(this, &UPicoSocialGameInstanceSubsystem::OnChallengeInviteComplete);
    PicoChallengesInterface->Invite(ChallengeID, UserIDs, ChallengeInviteDelegate);
}

void UPicoSocialGameInstanceSubsystem::OnChallengeInviteComplete(bool bIsError, const FString& ErrorMessage, UPico_Challenge* Challenge)
{
    if (bIsError)
    {
        UE_LOG(LogTemp, Log, TEXT("Error Message: %s"), *ErrorMessage);
    }
    UE_LOG(LogTemp, Log, TEXT("OnChallengeInviteComplete successed"));
}

void UPicoSocialGameInstanceSubsystem::LaunchOtherApp(const FString& PackageName, const FString& Message)
{
    if (!PicoApplicationInterface)
    {
        return;
    }
    LaunchOtherAppDelegate.Unbind();
    LaunchOtherAppDelegate.BindUObject(this, &UPicoSocialGameInstanceSubsystem::OnLaunchOtherAppComlete);
    PicoApplicationInterface->LaunchOtherApp(PackageName, Message, LaunchOtherAppDelegate);
}

void UPicoSocialGameInstanceSubsystem::OnLaunchOtherAppComlete(const FString& Info, bool bIsSuccessed, const FString& ErrorMessage)
{
    if (!bIsSuccessed)
    {
        UE_LOG(LogTemp, Log, TEXT("Error Message: %s"), *ErrorMessage);
    }
    UE_LOG(LogTemp, Log, TEXT("OnLaunchOtherAppComlete successed"));
}

void UPicoSocialGameInstanceSubsystem::LaunchOtherAppByAppId(const FString& AppId, const FString& Message)
{
    if (!PicoApplicationInterface)
    {
        return;
    }
    LaunchOtherAppByAppIdDelegate.Unbind();
    LaunchOtherAppByAppIdDelegate.BindUObject(this, &UPicoSocialGameInstanceSubsystem::OnLaunchOtherAppByAppIdComlete);
    PicoApplicationInterface->LaunchOtherAppByAppId(AppId, Message, LaunchOtherAppByAppIdDelegate);
}

void UPicoSocialGameInstanceSubsystem::OnLaunchOtherAppByAppIdComlete(const FString& Info, bool bIsSuccessed, const FString& ErrorMessage)
{
    if (!bIsSuccessed)
    {
        UE_LOG(LogTemp, Log, TEXT("Error Message: %s"), *ErrorMessage);
    }
    UE_LOG(LogTemp, Log, TEXT("OnLaunchOtherAppByAppIdComlete successed"));
}

void UPicoSocialGameInstanceSubsystem::PicoGetVersion()
{
    if (!PicoApplicationInterface)
    {
        return;
    }
    GetVersionDelegate.Unbind();
    GetVersionDelegate.BindUObject(this, &UPicoSocialGameInstanceSubsystem::OnPicoGetVersionComplete);
    PicoApplicationInterface->GetVersion(GetVersionDelegate);
}

void UPicoSocialGameInstanceSubsystem::OnPicoGetVersionComplete(int64 CurrentCode, const FString& CurrentName, int64 LatestCode, const FString& LatestName, bool IsSuccessed, const FString& ErrorMessage)
{
    if (!IsSuccessed)
    {
        UE_LOG(LogTemp, Log, TEXT("Error Message: %s"), *ErrorMessage);
    }
    UE_LOG(LogTemp, Log, TEXT("OnPicoGetVersionComplete successed"));
}

void UPicoSocialGameInstanceSubsystem::LaunchPicoStore()
{
    if (!PicoApplicationInterface)
    {
        return;
    }
    LaunchStoreDelegate.Unbind();
    LaunchStoreDelegate.BindUObject(this, &UPicoSocialGameInstanceSubsystem::OnLaunchPicoStoreComplete);
    PicoApplicationInterface->LaunchStore(LaunchStoreDelegate);
}

void UPicoSocialGameInstanceSubsystem::OnLaunchPicoStoreComplete(const FString& StringMessage, bool IsSuccessed, const FString& ErrorMessage)
{
    if (!IsSuccessed)
    {
        UE_LOG(LogTemp, Log, TEXT("Error Message: %s"), *ErrorMessage);
    }
    UE_LOG(LogTemp, Log, TEXT("OnLaunchPicoStoreComplete successed"));
}

void UPicoSocialGameInstanceSubsystem::PicoShareMedia(EShareMediaType InMediaType, const FString& InVideoPath, const FString& InVideoThumbPath, TArray<FString> InImagePaths, EShareAppTyp InShareType)
{
    if (!PicoPresenceInterface)
    {
        return;
    }
    ShareMediaDelegate.Unbind();
    ShareMediaDelegate.BindUObject(this, &UPicoSocialGameInstanceSubsystem::OnPresenceSendInvitesComplete);
    PicoPresenceInterface->ShareMedia(InMediaType, InVideoPath, InVideoThumbPath, InImagePaths, InShareType, ShareMediaDelegate);
}

void UPicoSocialGameInstanceSubsystem::OnPicoShareMediaComplete(bool IsSuccessed, const FString& ErrorMessage)
{
    if (!IsSuccessed)
    {
        UE_LOG(LogTemp, Log, TEXT("Error Message: %s"), *ErrorMessage);
    }
    UE_LOG(LogTemp, Log, TEXT("OnPicoShareMediaComplete successed"));
}

void UPicoSocialGameInstanceSubsystem::PicoRoomLaunchInvitableUserFlow(const FString& RoomId)
{
    if (!PicoRoomInterface)
    {
        return;
    }
    RoomLaunchInvitableUserFlowDelegate.Clear();
    RoomLaunchInvitableUserFlowDelegate.BindDynamic(this, &UPicoSocialGameInstanceSubsystem::OnPicoRoomLaunchInvitableUserFlow);
    PicoRoomInterface->LaunchInvitableUserFlow(RoomId, RoomLaunchInvitableUserFlowDelegate);
}

void UPicoSocialGameInstanceSubsystem::OnPicoRoomLaunchInvitableUserFlow(bool bIsError, int ErrorCode, const FString& ErrorMessage)
{
    if (bIsError)
    {
        UE_LOG(LogTemp, Log, TEXT("Error Message: %s"), *ErrorMessage);
    }
    UE_LOG(LogTemp, Log, TEXT("OnPicoRoomLaunchInvitableUserFlow successed"));
}

void UPicoSocialGameInstanceSubsystem::PicoLaunchChallengeInvite(const FString& ChallengeID)
{
    if (!PicoChallengesInterface)
    {
        return;
    }
    ChallengeLaunchInvitableUserFlowDelegate.Clear();
    ChallengeLaunchInvitableUserFlowDelegate.BindDynamic(this, &UPicoSocialGameInstanceSubsystem::OnPicoLaunchChallengeInviteComplete);
    PicoChallengesInterface->LaunchInvitableUserFlow(ChallengeID, ChallengeLaunchInvitableUserFlowDelegate);
}

void UPicoSocialGameInstanceSubsystem::OnPicoLaunchChallengeInviteComplete(bool bIsError, int ErrorCode, const FString& ErrorMessage)
{
    if (bIsError)
    {
        UE_LOG(LogTemp, Log, TEXT("Error Message: %s"), *ErrorMessage);
    }
    UE_LOG(LogTemp, Log, TEXT("OnPicoLaunchChallengeInviteComplete successed"));
}

