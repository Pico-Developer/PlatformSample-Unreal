// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PicoPresenceInterface.h"
#include "Pico_User.h"
#include "Pico_Room.h"
#include "Pico_Challenges.h"
#include "PicoApplicationInterface.h"
#include "PicoSocialGameInstanceSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class UIINTERACTIONDEMO_API UPicoSocialGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    UPicoSocialGameInstanceSubsystem();

    // Begin USubsytem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    TSharedPtr<class FPicoPresenceInterface> PicoPresenceInterface;
    TSharedPtr<class FPicoUserInterface> PicoUserInterface;
    TSharedPtr<class FPicoRoomInterface> PicoRoomInterface;
    TSharedPtr<class FPicoChallengesInterface> PicoChallengesInterface;
    TSharedPtr<class FPicoApplicationInterface> PicoApplicationInterface;

public:
    FOnPresenceSetComplete PresenceSetDelegate;
    FOnPresenceClearComplete PresenceClearDelegate;
    FOnSentInvitesComplete PresenceSendInviteDelegate;
    FOnLaunchInvitePanelComplete LaunchInvitePanelDelegate;
    FOnShareMediaComplete ShareMediaDelegate;

    FGetLoggedInUserFriends GetLoggedInUserFriendsDelegate;
    FGetUserInfo GetUserInfoDelegate;

    FRoomInviteUser InviteUserDelegate;
    FRoomLaunchInvitableUserFlow RoomLaunchInvitableUserFlowDelegate;

    FChallengeInvite ChallengeInviteDelegate;
    FChallengeLaunchInvitableUserFlow ChallengeLaunchInvitableUserFlowDelegate;

    FOnLaunchOtherAppComplete LaunchOtherAppDelegate;
    FOnLaunchOtherAppByAppIdComplete LaunchOtherAppByAppIdDelegate;
    FOnGetVersionComplete GetVersionDelegate;
    FOnLaunchStoreComplete LaunchStoreDelegate;



public:
    void OnJoinIntentChanged(const FString& DeeplinkMessage, const FString& DestinationApiName, const FString& LobbySessionId, const FString& MatchSessionId, const FString& Extra);
    
    void PicoSetPresence(const FString& ApiName, const FString& LobbySessionId, const FString& MatchSessionId, bool bIsJoinable, const FString& Extra);
    void OnPresenceSetComplete(bool bIsSuccessed, const FString& ErrorMessage);

    void PicoClearPresence();
    void OnPicoClearPresenceComplete(bool bIsSuccessed, const FString& ErrorMessage);

    void PicoGetUserFriends();

    UFUNCTION()
    void OnGetUserFriendsComplete(bool bIsError, const FString& ErrorMessage, UPico_UserArray* UserArray);

    void PicoGetUserInfo(const FString& UserID);

    UFUNCTION()
    void OnPicoGetUserInfoComplete(bool bIsError, const FString& ErrorMessage, UPico_User* User);

    void PresenceInviteUsers(TArray<FString> UserIdArray);
    void OnPresenceSendInvitesComplete(bool bIsSuccessed, const FString& ErrorMessage);


    void LaunchPicoInvitePanel();
    void OnLaunchInvitePanelComplete(bool bIsSuccessed, const FString& ErrorMessage);

    void PicoInviteUserJoinPriviteRoom(UPico_User* User, UPico_Room* PrivateRoom);
    
    UFUNCTION()
    void OnPicoInviteUserJoinPriviteRoomComplete(bool bIsError, int ErrorCode, const FString& ErrorMessage, UPico_Room* Room);

    void ChallengeInvite(const FString& ChallengeID, const TArray<FString>& UserIDs);

    UFUNCTION()
    void OnChallengeInviteComplete(bool bIsError, const FString& ErrorMessage, UPico_Challenge* Challenge);

    void LaunchOtherApp(const FString& PackageName, const FString& Message);
    void OnLaunchOtherAppComlete(const FString& Info, bool bIsSuccessed, const FString& ErrorMessage);

    void LaunchOtherAppByAppId(const FString& AppId, const FString& Message);
    void OnLaunchOtherAppByAppIdComlete(const FString& Info, bool bIsSuccessed, const FString& ErrorMessage);

    void PicoGetVersion();
    void OnPicoGetVersionComplete(int64 CurrentCode, const FString& CurrentName, int64 LatestCode, const FString& LatestName, bool IsSuccessed, const FString& ErrorMessage);

    void LaunchPicoStore();
    void OnLaunchPicoStoreComplete(const FString& StringMessage, bool IsSuccessed, const FString& ErrorMessage);

    void PicoShareMedia(EShareMediaType InMediaType, const FString& InVideoPath, const FString& InVideoThumbPath, TArray<FString> InImagePaths, EShareAppTyp InShareType);
    void OnPicoShareMediaComplete(bool IsSuccessed, const FString& ErrorMessage);

    void PicoRoomLaunchInvitableUserFlow(const FString& RoomId);

    UFUNCTION()
    void OnPicoRoomLaunchInvitableUserFlow(bool bIsError, int ErrorCode, const FString& ErrorMessage);


    void PicoLaunchChallengeInvite(const FString& ChallengeID);

    UFUNCTION()
    void OnPicoLaunchChallengeInviteComplete(bool bIsError, int ErrorCode, const FString& ErrorMessage);
};
