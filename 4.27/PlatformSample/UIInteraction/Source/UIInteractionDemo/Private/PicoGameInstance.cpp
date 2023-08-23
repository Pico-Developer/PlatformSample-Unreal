// Fill out your copyright notice in the Description page of Project Settings.


#include "PicoGameInstance.h"
#include "OnlineSubsystem.h"
#include "Core/Public/Serialization/BufferArchive.h"
#include "Pico_Networking.h"


UPicoGameInstance::UPicoGameInstance()
{
}

void UPicoGameInstance::Init()
{
    Super::Init();
    DebugShowA = DebugShowB = TEXT("Not Get Pico");
    UE_LOG(LogTemp, Log, TEXT("GameInstance  Init"));
    // Text Pico Use on the Mobile
    if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
    {
        PicoSubsystem = static_cast<FOnlineSubsystemPico*>(Subsystem);

        if (!PicoSubsystem)
        {
            return;
        }
        if (PicoSubsystem->Init())
        {
            DebugShowA = TEXT("Pico Init Success!");
        }
        else
        {
            return;
        }
        if (PicoSubsystem)
        {
            RtcInterface = PicoSubsystem->GetRtcUserInterface();
            FriendInterface = PicoSubsystem->GetFriendsInterface();
            IdentityInterface = PicoSubsystem->GetIdentityInterface();
            if (RtcInterface && FriendInterface && IdentityInterface)
            {
                DebugShowA = TEXT("Pico Init Success and Interface Get");
            }
            //GameInterface = PicoSubsystem->GetSessionInterface();
            GameInterface = PicoSubsystem->GetGameSessionInterface();
            LeaderboardInterface = PicoSubsystem->GetLeaderboardsInterface();
            AchievementInterface = PicoSubsystem->GetAchievementsInterface();
            PicoUserInterface = PicoSubsystem->GetPicoUserInterface();
            PicoPresenceInterface = PicoSubsystem->GetPicoPresenceInterface();
        }
        if (RtcInterface)
        {
            // BindNofitity
            RtcInterface->RtcJoinRoomCallback.AddUObject(this, &UPicoGameInstance::OnRtcJoinRoomResult);
            RtcInterface->RtcLeaveRoomCallback.AddUObject(this, &UPicoGameInstance::OnRtcLeaveRoomResult);
            RtcInterface->RtcRoomStateCallback.AddUObject(this, &UPicoGameInstance::OnRtcRoomState);
            RtcInterface->RtcUserJoinInfoCallback.AddUObject(this, &UPicoGameInstance::OnRtcUserJoinInfo);
            RtcInterface->RtcUserLeaveInfoCallback.AddUObject(this, &UPicoGameInstance::OnRtcUserLeaveInfo);
            RtcInterface->RtcRoomWarnCallback.AddUObject(this, &UPicoGameInstance::OnRtcRoomWarn);
            RtcInterface->RtcRoomErrorCallback.AddUObject(this, &UPicoGameInstance::OnRtcRoomError);
            RtcInterface->RtcUserMuteAudioInfoCallback.AddUObject(this, &UPicoGameInstance::OnRtcMuteState);
            RtcInterface->RtcAudioPlaybackDeviceChangeCallback.AddUObject(this, &UPicoGameInstance::OnRtcAudioChangePlaybackDevice);
            RtcInterface->RtcMediaDeviceChangeInfoCallback.AddUObject(this, &UPicoGameInstance::OnRtcMediaDeviceChangeInfo);
            RtcInterface->RtcLocalAudioPropertiesReportCallback.AddUObject(this, &UPicoGameInstance::OnRtcLocalAudioPropertiesReport);
            RtcInterface->RtcRemoteAudioPropertiesReportCallback.AddUObject(this, &UPicoGameInstance::OnRtcRemoteAudioPropertiesReport);
            RtcInterface->RtcWarnCallback.AddUObject(this, &UPicoGameInstance::OnRtcWarn);
            RtcInterface->RtcErrorCallback.AddUObject(this, &UPicoGameInstance::OnRtcError);
            RtcInterface->RtcConnectStateChangedCallback.AddUObject(this, &UPicoGameInstance::OnRtcConnectStateChanged);
            RtcInterface->RtcUserStartAudioCaptureCallback.AddUObject(this, &UPicoGameInstance::OnRtcUserStartAudioCapture);
            RtcInterface->RtcUserStopAudioCaptureCallback.AddUObject(this, &UPicoGameInstance::OnRtcUserStopAudioCapture);
        }
        if (GameInterface)
        {
            OnSessionUserInviteAcceptedDelegate = FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UPicoGameInstance::OnSessionUserInviteAccepted);
            GameInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(OnSessionUserInviteAcceptedDelegate);
            //GameInterface->GameConnectionCallback.AddUObject(this, &UPicoGameInstance::OnGameConnectionComplete);
            //GameInterface->GameStateResetCallback.AddUObject(this, &UPicoGameInstance::OnGameStateResetComplete);
        }
        if (PicoUserInterface)
        {
            GetAccessTokenDelegate.BindDynamic(this, &UPicoGameInstance::OnGetAccessTokenComplete);
        }

        if (PicoPresenceInterface)
        {
            PicoPresenceInterface->JoinIntentReceivedCallback.AddUObject(this, &UPicoGameInstance::OnJoinIntentChanged);
        }
    }
}


bool UPicoGameInstance::ReadFriendList(int32 LocalUserNum, const FString& ListName)
{
    if (!bIsLoggedIn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Not Logged In"));
    }
    if (FriendInterface.IsValid())
    {
        ReadCompleteDelegate.BindUObject(this, &UPicoGameInstance::OnReadListComplete);
        UE_LOG(LogTemp, Log, TEXT("Call Read Friend List"));
        return FriendInterface->ReadFriendsList(LocalUserNum, ListName, ReadCompleteDelegate);
    }
    return false;
}

void UPicoGameInstance::OnReadListComplete(int32 InLocalUserNum/*LocalUserNum*/, bool bWasSuccessful/*bWasSuccessful*/, const FString& ListName/*ListName*/, const FString& ErrorStr/*ErrorStr*/)
{
    if (bWasSuccessful)
    {
        ShowErrorString = ErrorStr;
        ReadCompleteDelegate.Unbind();
        TArray< TSharedRef<FOnlineFriend> > OutFriends;
        FriendInterface->GetFriendsList(InLocalUserNum, ListName, OutFriends);
        UE_LOG(LogTemp, Log, TEXT("Friend Num: %d"), OutFriends.Num());
        for (int i = 0; i < OutFriends.Num(); i++)
        {
            DisplayNameArray.Add(OutFriends[i]->GetDisplayName());
            IdArray.Add((OutFriends[i]->GetUserId()).Get().ToString());
        }
        BPReadFriendListComplete(InLocalUserNum, bWasSuccessful, ListName, ErrorStr);
    }
}

void UPicoGameInstance::SaveLoadData(FArchive& Ar, float& TargetValue, int32& Num, FVector& TargetLocation)
{
    Ar << TargetValue;

    Ar << Num;

    Ar << TargetLocation;
}

bool UPicoGameInstance::NetworkingSendPacketToCurrentRoom(float TargetValue, int32 Num, FVector TargetLocation, bool bReliable)
{
    FBufferArchive ToBinary;
    SaveLoadData(ToBinary, TargetValue, Num, TargetLocation);
    if (ToBinary.Num() <= 0)
    {
        return false;
    }
    bool SendResult = UOnlinePicoNetworkingFunction::SendPacketToCurrentRoom(this, ToBinary, bReliable);
    ToBinary.FlushCache();
    ToBinary.Empty();
    return SendResult;
}

bool UPicoGameInstance::NetworkingSendPacketToUser(const FString& UserID, float TargetValue, int32 Num, FVector TargetLocation, bool bReliable)
{
    FBufferArchive ToBinary;
    SaveLoadData(ToBinary, TargetValue, Num, TargetLocation);
    if (ToBinary.Num() <= 0)
    {
        return false;
    }
    bool SendResult = UOnlinePicoNetworkingFunction::SendPacket(this, UserID, ToBinary, bReliable);
    ToBinary.FlushCache();
    ToBinary.Empty();
    return SendResult;
}



bool UPicoGameInstance::NetworkingReadPacket(float& OutValue, int32& OutNum, FVector& OutTargetLocation, FString& SendUserId)
{
    TArray<uint8> BinaryArray;
    bool ReadResult = UOnlinePicoNetworkingFunction::ReadPacket(this, BinaryArray, SendUserId);
    if (ReadResult)
    {
        if (BinaryArray.Num() > 0)
        {
            FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
            FromBinary.Seek(0);         
            SaveLoadData(FromBinary, OutValue, OutNum, OutTargetLocation);
            FromBinary.FlushCache();
            BinaryArray.Empty();
            FromBinary.Close();    
            return true;
        }
    }
    return false;
}

void UPicoGameInstance::PicoLogin(FString LocalUserNum, FString InType, FString ID, FString InToken)
{
    if (IdentityInterface.IsValid())
    {
        FOnlineAccountCredentials Credentials;
        Credentials.Id = ID;
        Credentials.Token = InToken;
        Credentials.Type = InType;
        IdentityInterface->OnLoginCompleteDelegates->AddUObject(this, &UPicoGameInstance::OnLoginComplete);
        IdentityInterface->Login(0, Credentials);
    }
    else
    {
        UE_LOG_ONLINE(Display, TEXT("PPF_GAME PicoLogin IdentityInterface is invalid"));
    }
}

void UPicoGameInstance::OnLoginComplete(int LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& ErrorString)
{

    UE_LOG(LogTemp, Warning, TEXT("LoggedIn: %d"), bWasSuccessful);
    bIsLoggedIn = bWasSuccessful;
    if (IdentityInterface.IsValid())
    {
        IdentityInterface->ClearOnLoginCompleteDelegates(0, this);
    }
    if (!bWasSuccessful)
    {
        UE_LOG(LogTemp, Error, TEXT("LoggedIn Failed : %s"), *ErrorString);
    }

    UE_LOG(LogTemp, Display, TEXT("OnLoginComplete UserId : %s"), *UserId.ToString());
    BPLoginComplete(LocalUserNum, bWasSuccessful, UserId.ToString(), ErrorString);

    ShowErrorString = ErrorString;
}

void UPicoGameInstance::OnGetAccessTokenComplete(bool bIsError, const FString& ErrorMessage, const FString& AccessToken)
{
    if (bIsError)
    {
        UE_LOG(LogTemp, Error, TEXT("OnGetAccessTokenComplete Failed : %s"), *ErrorMessage);
    }
    UE_LOG(LogTemp, Display, TEXT("OnGetAccessTokenComplete AccessToken : %s"), *AccessToken);
}

ERtcEngineInitResult UPicoGameInstance::RtcEngineInit()
{
    ERtcEngineInitResult Result = ERtcEngineInitResult::None;
    if (RtcInterface)
    {
        Result = RtcInterface->RtcEngineInit();
    }
    return Result;
}

int UPicoGameInstance::RtcJoinRoom(const FString& RoomId, const FString& UserId, const FString& Token, const FString& UserExtra, ERtcRoomProfileType InRoomProfileType, bool bIsAutoSubscribeAudio)
{
    int ReturnCode = 0;
    if (RtcInterface)
    {
        ReturnCode = RtcInterface->RtcJoinRoom(RoomId, UserId, Token, UserExtra, InRoomProfileType, bIsAutoSubscribeAudio);
    }
    return ReturnCode;
    
}

void UPicoGameInstance::OnRtcJoinRoomResult(const FString& RoomId, const FString& UserId, int ErrorCode, int Elapsed, ERtcJoinRoomType InJoinRoomType)
{

    BPOnRtcJoinRoomResult(RoomId, UserId, ErrorCode, Elapsed, InJoinRoomType);
}


void UPicoGameInstance::OnRtcLeaveRoomResult(const FString& RoomId)
{
    BPOnRtcLeaveRoomResult(RoomId);
}

void UPicoGameInstance::OnRtcRoomState(int TotalDuration, int UserCount, const FString& RoomId)
{
    BPOnRtcRoomState(TotalDuration, UserCount, RoomId);
}

void UPicoGameInstance::OnRtcUserJoinInfo(const FString& UserId, const FString& UserExtra, int Elapsed, const FString& RoomId)
{
    BPOnRtcUserJoinInfo(UserId, UserExtra, Elapsed, RoomId);
}

void UPicoGameInstance::OnRtcUserLeaveInfo(const FString& UserId, ERtcUserLeaveReasonType RtcUserLeaveReasonType, const FString& RoomId)
{
    BPOnRtcUserLeaveInfo(UserId, RtcUserLeaveReasonType, RoomId);
}

void UPicoGameInstance::OnRtcRoomWarn(int Code, const FString& RoomId)
{
    BPOnRtcRoomWarn(Code, RoomId);
}

void UPicoGameInstance::OnRtcRoomError(int Code, const FString& RoomId)
{
    BPOnRtcRoomError(Code, RoomId);
}

void UPicoGameInstance::OnRtcMuteState(const FString& UserId, ERtcMuteState RtcMuteState)
{
    BPOnRtcMuteState(UserId, RtcMuteState);
}

void UPicoGameInstance::OnRtcAudioChangePlaybackDevice(ERtcAudioPlaybackDevice RtcAudioPlaybackDevice)
{
    BPOnRtcAudioChangePlaybackDevice(RtcAudioPlaybackDevice);
}

void UPicoGameInstance::OnRtcMediaDeviceChangeInfo(const FString& DeviceId, ERtcMediaDeviceType MediaDeciveType, ERtcMediaDeviceState MediaDeviceState, ERtcMediaDeviceError MediaDeviceError)
{
    BPOnRtcMediaDeviceChangeInfo(DeviceId, MediaDeciveType, MediaDeviceState, MediaDeviceError);
}

void UPicoGameInstance::OnRtcLocalAudioPropertiesReport(TArray<ERtcStreamIndex> StreamIndexs, TArray<int> Volumes)
{
    BPOnRtcLocalAudioPropertiesReport(StreamIndexs, Volumes);
}

void UPicoGameInstance::OnRtcRemoteAudioPropertiesReport(int TotalRemoteVolume, TArray<int> Volumes, const TArray<FString>& RoomIds, const TArray<FString>& UserIds, TArray<ERtcStreamIndex> StreamIndexs)
{
    BPOnRtcRemoteAudioPropertiesReport(TotalRemoteVolume, Volumes, RoomIds, UserIds, StreamIndexs);
}

void UPicoGameInstance::OnRtcWarn(int MessageCode)
{
    BPOnRtcWarn(MessageCode);
}

void UPicoGameInstance::OnRtcError(int MessageCode)
{
    BPOnRtcError(MessageCode);
}

void UPicoGameInstance::OnRtcConnectStateChanged(const FString& StringMessage)
{
    BPOnRtcConnectStateChanged(StringMessage);
}

void UPicoGameInstance::OnRtcUserStartAudioCapture(const FString& StringMessage)
{
    BPOnRtcUserStartAudioCapture(StringMessage);
}

void UPicoGameInstance::OnRtcUserStopAudioCapture(const FString& StringMessage)
{
    BPOnRtcUserStopAudioCapture(StringMessage);
}

void UPicoGameInstance::OnJoinIntentChanged(const FString& DeeplinkMessage, const FString& DestinationApiName, const FString& LobbySessionId, const FString& MatchSessionId, const FString& Extra)
{
    UE_LOG(LogTemp, Log, TEXT("UPicoGameInstance::OnJoinIntentChanged DeeplinkMessage: %s, DestinationApiName: %s, LobbySessionId: %s, MatchSessionId: %s, Extra: %s.!"), *DeeplinkMessage, *DestinationApiName, *LobbySessionId, *MatchSessionId, *Extra);
}

int UPicoGameInstance::RtcDestroyRoom(const FString& RoomId)
{
    int ReturenCode = 0;
    if (RtcInterface)
    {
        ReturenCode = RtcInterface->RtcDestroyRoom(RoomId);
    }
    return ReturenCode;
}

void UPicoGameInstance::RtcEnableAudioPropertiesReport(int Interval)
{
    if (RtcInterface)
    {
        RtcInterface->RtcEnableAudioPropertiesReport(Interval);
    }
}

int UPicoGameInstance::RtcLeaveRoom(const FString& RoomId)
{
    int ReturenCode = 0;
    if (RtcInterface)
    {
        ReturenCode = RtcInterface->RtcLeaveRoom(RoomId);
    }
    return ReturenCode;
}

void UPicoGameInstance::RtcMuteLocalAudio(ERtcMuteState InRtcMuteState)
{
    if (RtcInterface)
    {
        RtcInterface->RtcMuteLocalAudio(InRtcMuteState);
    }
}

void UPicoGameInstance::RtcPublishRoom(const FString& RoomId)
{
    if (RtcInterface)
    {
        RtcInterface->RtcPublishRoom(RoomId);
    }
}

void UPicoGameInstance::RtcRoomPauseAllSubscribedStream(const FString& RoomId, ERtcPauseResumeMediaType InPauseResumeMediaType)
{
    if (RtcInterface)
    {
        RtcInterface->RtcRoomPauseAllSubscribedStream(RoomId, InPauseResumeMediaType);
    }
}

void UPicoGameInstance::RtcRoomResumeAllSubscribedStream(const FString& RoomId, ERtcPauseResumeMediaType InPauseResumeMediaType)
{
    if (RtcInterface)
    {
        RtcInterface->RtcRoomResumeAllSubscribedStream(RoomId, InPauseResumeMediaType);
    }
}

void UPicoGameInstance::RtcSetAudioPlaybackDevice(ERtcAudioPlaybackDevice InRtcAudioPlaybackDevice)
{
    if (RtcInterface)
    {
        RtcInterface->RtcSetAudioPlaybackDevice(InRtcAudioPlaybackDevice);
    }
}

void UPicoGameInstance::RtcSetAudioScenario(ERtcAudioScenarioType InRtcAudioScenarioType)
{
    if (RtcInterface)
    {
        RtcInterface->RtcSetAudioScenario(InRtcAudioScenarioType);
    }
}

void UPicoGameInstance::RtcSetCaptureVolume(ERtcStreamIndex InRtcStreamIndex, int InVolume)
{
    if (RtcInterface)
    {
        RtcInterface->RtcSetCaptureVolume(InRtcStreamIndex, InVolume);
    }
}

void UPicoGameInstance::RtcSetEarMonitorMode(ERtcEarMonitorMode InRtcEarMonitorMode)
{
    if (RtcInterface)
    {
        RtcInterface->RtcSetEarMonitorMode(InRtcEarMonitorMode);
    }
}

void UPicoGameInstance::RtcSetEarMonitorVolume(int InVolume)
{
    if (RtcInterface)
    {
        RtcInterface->RtcSetEarMonitorVolume(InVolume);
    }
}

void UPicoGameInstance::RtcSetPlaybackVolume(int InVolume)
{
    if (RtcInterface)
    {
        RtcInterface->RtcSetPlaybackVolume(InVolume);
    }
}

void UPicoGameInstance::RtcStartAudioCapture()
{
    if (RtcInterface)
    {
        RtcInterface->RtcStartAudioCapture();
    }
}

void UPicoGameInstance::RtcStopAudioCatpure()
{
    if (RtcInterface)
    {
        RtcInterface->RtcStopAudioCatpure();
    }
}

void UPicoGameInstance::RtcUnPublishRoom(const FString& RoomId)
{
    if (RtcInterface)
    {
        RtcInterface->RtcUnPublishRoom(RoomId);
    }
}

void UPicoGameInstance::RtcUpdateToken(const FString& RoomId, const FString& Token)
{
    if (RtcInterface)
    {
        RtcInterface->RtcUpdateToken(RoomId, Token);
    }
}

// Game Function
void UPicoGameInstance::GameInitialize()
{ 
    /*if (GameInterface)
    {
        GameInterface->Initialize();
    }*/
}
bool UPicoGameInstance::GameUninitialize()
{
    /*if (GameInterface)
    {
        return GameInterface->Uninitialize();
    }*/
    return false;
}

// Game Notification
void UPicoGameInstance::OnGameSessionStateChanged(const FString& Log)
{
    UE_LOG_ONLINE(Display, TEXT("PPF_GAME %s"), *Log);
    OnGameSessionStateChangedDelegates.Broadcast(FString::Printf(TEXT("%s\n"), *Log));

    //FString TextPath = FPaths::ProjectPersistentDownloadDir() + TEXT("Log-PicoGameInstance.txt");
	//FString WriteLog = FString::Printf(TEXT("%s\n"), *Log);
    //FFileHelper::SaveStringToFile(*WriteLog, *TextPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
}
void UPicoGameInstance::OnGameConnectionComplete(int Result, bool bWasSuccessful)
{
    OnGameSessionStateChanged(FString::Printf(TEXT("OnGameConnectionComplete Result: %d, bWasSuccessful: %d"), Result, bWasSuccessful));
}
void UPicoGameInstance::OnGameStateResetComplete(bool bWasSuccessful)
{
    OnGameSessionStateChanged(FString::Printf(TEXT("OnGameStateResetComplete bWasSuccessful: %d"), bWasSuccessful));
}

// Game OnComplete
void UPicoGameInstance::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
    OnGameSessionStateChanged(FString::Printf(TEXT("OnStartSessionComplete %s, SessionName: %s"), bWasSuccessful?TEXT("success"):TEXT("failed"), *SessionName.ToString()));
    if (bWasSuccessful)
    {
        LogSessionData(SessionName);
    }
}

void UPicoGameInstance::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
    OnGameSessionStateChanged(FString::Printf(TEXT("OnEndSessionComplete SessionName: %s, bWasSuccessful: %d"), *SessionName.ToString(), bWasSuccessful));
    if (bWasSuccessful)
    {
        LogSessionData(SessionName);
    }
}

void UPicoGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    OnGameSessionStateChanged(FString::Printf(TEXT("OnDestroySessionComplete SessionName: %s, bWasSuccessful: %d"), *SessionName.ToString(), bWasSuccessful));
    if (bWasSuccessful)
    {
        LogSessionData(SessionName);
    }
}

void UPicoGameInstance::OnCancelMatchmakingComplete(FName SessionName, bool bWasSuccessful) {
    OnGameSessionStateChanged(FString::Printf(TEXT("OnCancelMatchmakingComplete SessionName: %s, bWasSuccessful: %d"), *SessionName.ToString(), bWasSuccessful));
}

void UPicoGameInstance::OnMatchmakingComplete(FName SessionName, bool bWasSuccessful) {
    OnGameSessionStateChanged(FString::Printf(TEXT("OnMatchmakingComplete SessionName: %s, bWasSuccessful: %d"), *SessionName.ToString(), bWasSuccessful));
    if (!GameInterface)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("OnMatchmakingComplete GameInterface is invalid")));
        return;
    }
    if (!(bWasSuccessful && SearchSettings->SearchResults.Num() > 0))
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("OnMatchmakingComplete Error! Did not successfully find a matchmaking session!")));
        return;
    }
    if (!OnJoinSessionCompleteDelegate.IsBound()) {
        OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnJoinSessionComplete);
        GameInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
    }
    bool Result = GameInterface->JoinSession(0, SessionName, SearchSettings->SearchResults[0]);
    FString RoomId = SearchSettings->SearchResults[0].GetSessionIdStr();
    OnGameSessionStateChanged(FString::Printf(TEXT("OnMatchmakingComplete Found a matchmaking session.  JoiningSession RoomId: %s, ExecuteResult: %d"), *RoomId, Result));
}

void UPicoGameInstance::OnFindSessionComplete(bool Result)
{
    OnGameSessionStateChanged(FString::Printf(TEXT("OnFindSessionComplete %s\nSearchResults.Num: %d\nSearchResults:"), Result?TEXT("success"):TEXT("failed"), SearchSettings->SearchResults.Num()));
    for (int i = 0; i < SearchSettings->SearchResults.Num(); i++)
    {
#if ENGINE_MINOR_VERSION > 26
        LogSessionData(SearchSettings->SearchResults[i].Session, false);
#elif ENGINE_MINOR_VERSION > 24
        OnGameSessionStateChanged(FString::Printf(TEXT("OnFindSessionComplete RoomId: %s"), *SearchSettings->SearchResults[i].Session.SessionInfo->ToString()));
#endif       
    }
}

void UPicoGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful) {
    OnGameSessionStateChanged(FString::Printf(TEXT("OnCreateSessionComplete SessionName: %s, bWasSuccessful: %d"), *SessionName.ToString(), bWasSuccessful));
    if (bWasSuccessful)
    {
        LogSessionData(SessionName);
    }
}

void UPicoGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
    OnGameSessionStateChanged(FString::Printf(TEXT("OnJoinSessionComplete JoinResult: %d, IsSuccess: %d"), JoinResult, JoinResult == EOnJoinSessionCompleteResult::Success));
    LogSessionData(SessionName);
}

void UPicoGameInstance::OnFindFriendSessionComplete(int32 LocalPlayerNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& FriendSearchResults)
{
    OnGameSessionStateChanged(FString::Printf(TEXT("[OnFindFriendSessionComplete]\nLocalPlayerNum: %d\nbWasSuccessful: %d\nFriendSearchResults.Num(): %d\n"), LocalPlayerNum, bWasSuccessful, FriendSearchResults.Num()));
    if (bWasSuccessful)
    {
        if (FriendSearchResults.Num() > 0)
        {
            OnGameSessionStateChanged(FString::Printf(TEXT("FriendsSessions:\n")));
            for (auto FriendSearchResult : FriendSearchResults)
            {
                OnGameSessionStateChanged(FString::Printf(TEXT("key: %s"), *FriendSearchResult.Session.OwningUserId->ToString()));
                FriendsSessions.Add(FriendSearchResult.Session.OwningUserId->ToString(), FriendSearchResult);
            }
        }
    }
}

void UPicoGameInstance::OnReadFriendsListComplete(int32 LocalUserNum, const bool bWasSuccessful, const FString& ListName, const FString& ErrorStr) {
    OnGameSessionStateChanged(FString::Printf(TEXT("OnReadFriendsListComplete bWasSuccessful: %d, ErrorStr: %s"), bWasSuccessful, *ErrorStr));
    if (bWasSuccessful)
    {
        if (!FriendInterface.IsValid())
        {
            OnGameSessionStateChanged(FString::Printf(TEXT("OnReadFriendsListComplete FriendInterface is invalid")));
            return;
        }
        TArray<TSharedRef<FOnlineFriend>> Friends;
        FriendInterface->GetFriendsList(0, ListName, Friends);
        OnGameSessionStateChanged(FString::Printf(TEXT("OnReadFriendsListComplete bWasSuccessful: %d, Count of friends: %d"), bWasSuccessful, Friends.Num()));
        if (!GameInterface.IsValid())
        {
            OnGameSessionStateChanged(FString::Printf(TEXT("OnReadFriendsListComplete GameInterface is invalid")));
            return;
        }
        if (!OnFindFriendSessionCompleteDelegate.IsBound()) {
            OnFindFriendSessionCompleteDelegate = FOnFindFriendSessionCompleteDelegate::CreateUObject(
                this, &UPicoGameInstance::OnFindFriendSessionComplete);
            GameInterface->AddOnFindFriendSessionCompleteDelegate_Handle(
                0, OnFindFriendSessionCompleteDelegate);
        }

        for (auto Friend : Friends) {
            GameInterface->FindFriendSession(0, Friend->GetUserId().Get());
            OnGameSessionStateChanged(FString::Printf(TEXT("OnReadFriendsListComplete key:Friend->GetDisplayName(): %s, value:Friend StrID: %s, ID: %s"), 
                *Friend->GetDisplayName(), *Friend->GetUserId()->ToDebugString(), *Friend->GetUserId()->ToString()));
            FriendsToInvite.Add(Friend->GetDisplayName(), Friend->GetUserId());
        }
    }
}

void UPicoGameInstance::OnFindSessionByIdComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& SearchResult)
{
    OnGameSessionStateChanged(FString::Printf(TEXT("OnFindSessionByIdComplete LocalUserNum: %d, bWasSuccessful: %d"), LocalUserNum, bWasSuccessful));
    if (bWasSuccessful && SearchResult.IsValid())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("OnFindSessionByIdComplete RoomID: %s"), *SearchResult.Session.SessionInfo->GetSessionId().ToString()));
        FindSessionByIdResult = SearchResult;
    }
}

void UPicoGameInstance::OnFindSessionByIdCompleteAndJoin(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& SearchResult)
{
    OnGameSessionStateChanged(FString::Printf(TEXT("OnFindSessionByIdCompleteAndJoin LocalUserNum: %d, bWasSuccessful: %d"), LocalUserNum, bWasSuccessful));
    // if (bWasSuccessful && SearchResult.IsValid())
    if (SearchResult.IsValid())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("OnFindSessionByIdCompleteAndJoin RoomID: %s"), *SearchResult.Session.SessionInfo->GetSessionId().ToString()));
        FindSessionByIdResult = SearchResult;
        if (!GameInterface)
        {
            OnGameSessionStateChanged(FString::Printf(TEXT("JoinFriendSession GameInterface is invalid")));
            return;
        }
        if (!OnJoinSessionCompleteDelegate.IsBound()) {
            OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnJoinSessionComplete);
            GameInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
        }
        auto Result = GameInterface->JoinSession(0, TEXT("Game"), SearchResult);
        OnGameSessionStateChanged(FString::Printf(TEXT("OnFindSessionByIdCompleteAndJoin JoinSession ExecuteResult: %d"), Result));
    }
}

void UPicoGameInstance::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId,
                                                    TSharedPtr<const FUniqueNetId> UserId,
                                                    const FOnlineSessionSearchResult& InviteResult)
{
    OnGameSessionStateChanged(FString::Printf(TEXT("OnSessionUserInviteAccepted begin. Execute PicoLogin")));
    PicoLogin(TEXT("0"), TEXT(""), TEXT(""), TEXT(""));
    if (!GameInterface)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("OnSessionUserInviteAccepted GameInterface is invalid")));
        return;
    }
    if (!bWasSuccessful)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("OnSessionUserInviteAccepted bWasSuccessful is false")));
        return;
    }
    OnGameSessionStateChanged(FString::Printf(TEXT("OnSessionUserInviteAccepted RoomID: %s"), *InviteResult.GetSessionIdStr()));
    // OnGameSessionStateChanged(FString::Printf(TEXT("OnSessionUserInviteAccepted Start Joining session....")));
    // if (!OnJoinSessionCompleteDelegate.IsBound()) {
    //     OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnJoinSessionComplete);
    //     GameInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
    // }
    // bool Result = GameInterface->JoinSession(ControllerId, TEXT("Game"), InviteResult);
    // OnGameSessionStateChanged(FString::Printf(TEXT("OnSessionUserInviteAccepted JoinSession return: %d"), Result));
}

// Game tool
void UPicoGameInstance::LogSessionData(FName SessionName)
{
    if (!GameInterface.IsValid())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("LogSessionData GameInterface is invalid")));
        return;
    }
    auto NamedSession = GameInterface->GetNamedSession(SessionName);
    TestDumpNamedSession(NamedSession);
}

void UPicoGameInstance::TestDumpNamedSession(const FNamedOnlineSession* NamedSession)
{
    FString Log;
	if (NamedSession != NULL)
	{
		Log = FString::Printf(TEXT("dumping NamedSession: \n"));
		Log.Append(FString::Printf(TEXT("	SessionName: %s\n"), *NamedSession->SessionName.ToString()));
		Log.Append(FString::Printf(TEXT("	HostingPlayerNum: %d\n"), NamedSession->HostingPlayerNum));
		Log.Append(FString::Printf(TEXT("	SessionState: %s\n"), EOnlineSessionState::ToString(NamedSession->SessionState)));
		Log.Append(FString::Printf(TEXT("	RegisteredPlayers: \n")));
	    OnGameSessionStateChanged(FString::Printf(TEXT("		num: %d"), NamedSession->RegisteredPlayers.Num()));
		if (NamedSession->RegisteredPlayers.Num() > 0)
		{
			for (int32 UserIdx = 0; UserIdx < NamedSession->RegisteredPlayers.Num(); UserIdx++)
			{
			    if (NamedSession->RegisteredPlayers[UserIdx].Get().IsValid())
			    {
			        Log.Append(FString::Printf(TEXT("	    %d: %s\n"), UserIdx, *NamedSession->RegisteredPlayers[UserIdx]->ToString()));
			    }
			}
		}
		else
		{
			Log.Append(FString::Printf(TEXT("	    0 registered players\n")));
		}
		TestDumpSession(NamedSession, Log);
	}
    else
    {
        Log = FString("NamedSession is NULL\n");
    }
    OnGameSessionStateChanged(FString::Printf(TEXT("%s"), *Log));
}
void UPicoGameInstance::TestDumpSession(const FOnlineSession* Session, FString& Log)
{
	if (Session != NULL)
	{
		Log.Append(FString::Printf(TEXT("dumping Session: \n")));
		Log.Append(FString::Printf(TEXT("	OwningPlayerName: %s\n"), *Session->OwningUserName));
		Log.Append(FString::Printf(TEXT("	OwningPlayerId: %s\n"), Session->OwningUserId.IsValid() ? *Session->OwningUserId->ToString() : TEXT("")));
		Log.Append(FString::Printf(TEXT("	NumOpenPrivateConnections: %d\n"), Session->NumOpenPrivateConnections));
		Log.Append(FString::Printf(TEXT("	NumOpenPublicConnections: %d\n"), Session->NumOpenPublicConnections));
		Log.Append(FString::Printf(TEXT("	SessionInfo: %s\n"), Session->SessionInfo.IsValid() ? *Session->SessionInfo->ToDebugString() : TEXT("NULL")));
		Log.Append(FString::Printf(TEXT("	SessionInfo: RoomId: %s\n"), *Session->SessionInfo->GetSessionId().ToString()));
	    
		TestDumpSessionSettings(&Session->SessionSettings, Log);
	}
	else
	{
		Log.Append(FString::Printf(TEXT("dumping Session is null\n")));
	}
}
void UPicoGameInstance::TestDumpSessionSettings(const FOnlineSessionSettings* SessionSettings, FString& Log)
{
	if (SessionSettings != NULL)
	{
		Log.Append(FString::Printf(TEXT("dumping SessionSettings: \n")));
		Log.Append(FString::Printf(TEXT("\tNumPublicConnections: %d\n"), SessionSettings->NumPublicConnections));
		Log.Append(FString::Printf(TEXT("\tNumPrivateConnections: %d\n"), SessionSettings->NumPrivateConnections));
		Log.Append(FString::Printf(TEXT("\tbIsLanMatch: %s\n"), SessionSettings->bIsLANMatch ? TEXT("true") : TEXT("false")));
		Log.Append(FString::Printf(TEXT("\tbIsDedicated: %s\n"), SessionSettings->bIsDedicated ? TEXT("true") : TEXT("false")));
		Log.Append(FString::Printf(TEXT("\tbUsesStats: %s\n"), SessionSettings->bUsesStats ? TEXT("true") : TEXT("false")));
		Log.Append(FString::Printf(TEXT("\tbShouldAdvertise: %s\n"), SessionSettings->bShouldAdvertise ? TEXT("true") : TEXT("false")));
		Log.Append(FString::Printf(TEXT("\tbAllowJoinInProgress: %s\n"), SessionSettings->bAllowJoinInProgress ? TEXT("true") : TEXT("false")));
		Log.Append(FString::Printf(TEXT("\tbAllowInvites: %s\n"), SessionSettings->bAllowInvites ? TEXT("true") : TEXT("false")));
		Log.Append(FString::Printf(TEXT("\tbUsesPresence: %s\n"), SessionSettings->bUsesPresence ? TEXT("true") : TEXT("false")));
		Log.Append(FString::Printf(TEXT("\tbAllowJoinViaPresence: %s\n"), SessionSettings->bAllowJoinViaPresence ? TEXT("true") : TEXT("false")));
		Log.Append(FString::Printf(TEXT("\tbAllowJoinViaPresenceFriendsOnly: %s\n"), SessionSettings->bAllowJoinViaPresenceFriendsOnly ? TEXT("true") : TEXT("false")));
		Log.Append(FString::Printf(TEXT("\tBuildUniqueId: 0x%08x\n"), SessionSettings->BuildUniqueId));
		Log.Append(FString::Printf(TEXT("\tSettings:\n")));
		for (FSessionSettings::TConstIterator It(SessionSettings->Settings); It; ++It)
		{
			FName Key = It.Key();
			const FOnlineSessionSetting& Setting = It.Value();
			Log.Append(FString::Printf(TEXT("PPF_GAME \t\t%s=%s\n"), *Key.ToString(), *Setting.ToString()));
		}
	}
}

void UPicoGameInstance::LogSessionData(FOnlineSession& Session, bool ForceToNamedOnlineSession)
{
    OnGameSessionStateChanged(FString::Printf(TEXT("LogSessionData ForceToNamedOnlineSession: %d"), ForceToNamedOnlineSession));
    if (!ForceToNamedOnlineSession)
    {
        FString Log;
        TestDumpSession(&Session, Log);
        OnGameSessionStateChanged(FString::Printf(TEXT("%s"), *Log));
        return;
    }
    
    FNamedOnlineSession& NamedSession = static_cast<FNamedOnlineSession&>(Session);
    TestDumpNamedSession(&NamedSession);
}

// Game IOnlineSession Function
void UPicoGameInstance::CreateSession(const FString& Keys, const FString& Values, bool bShouldAdvertise, bool bAllowJoinViaPresenceFriendsOnly, bool bAllowInvites, bool bAllowJoinViaPresence, int NumPublicConnections) {
    if (!GameInterface.IsValid())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("CreateSession GameInterface is invalid")));
        return;
    }
    if (!OnCreateSessionCompleteDelegate.IsBound())
    {
        OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnCreateSessionComplete);
        GameInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
    }

    TSharedPtr<class FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
    SessionSettings->NumPublicConnections = NumPublicConnections;
    SessionSettings->bShouldAdvertise = bShouldAdvertise;
    SessionSettings->bAllowJoinViaPresenceFriendsOnly = bAllowJoinViaPresenceFriendsOnly;
    SessionSettings->bAllowInvites = bAllowInvites;
    SessionSettings->bAllowJoinViaPresence = bAllowJoinViaPresence;
    TArray<FString> KeyArr, ValueArr;
    Keys.ParseIntoArray(KeyArr, TEXT(","), true);
    Values.ParseIntoArray(ValueArr, TEXT(","), true);
    if (ValueArr.Num() != KeyArr.Num())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("CreateSession Input Error! keys count != values count")));
        return;
    }
    for (int i = 0; i < KeyArr.Num(); i++)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("CreateSession set key: %s, value: %s"), *KeyArr[i], *ValueArr[i]));
        SessionSettings->Set(FName(KeyArr[i]), ValueArr[i], EOnlineDataAdvertisementType::ViaOnlineService);
    }
    bool Result = GameInterface->CreateSession(/* Hosting Player Num*/ 0, TEXT("Game"), *SessionSettings);
    OnGameSessionStateChanged(FString::Printf(TEXT("CreateSession ExecuteResult: %d"), Result));
}

bool UPicoGameInstance::StartMatchmaking(const FString& Keys, const FString& Values, int NumPublicConnections) {
    if (!GameInterface)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("StartMatchmaking GameInterface is invalid")));
        return false;
    }
    TArray<TSharedRef<const FUniqueNetId>> GameLocalPlayers;
    SearchSettings = MakeShareable(new FOnlineSessionSearch());
    auto SessionSettings = new FOnlineSessionSettings();
    SessionSettings->NumPublicConnections = NumPublicConnections;

    if (!OnMatchmakingCompleteDelegate.IsBound())
    {
        OnMatchmakingCompleteDelegate = FOnMatchmakingCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnMatchmakingComplete);
        GameInterface->AddOnMatchmakingCompleteDelegate_Handle(OnMatchmakingCompleteDelegate);
    }

    TArray<FString> KeyArr, ValueArr;
    Keys.ParseIntoArray(KeyArr, TEXT(","), true);
    Values.ParseIntoArray(ValueArr, TEXT(","), true);
    if (ValueArr.Num() != KeyArr.Num())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("StartMatchmaking Input Error! keys count != values count")));
        return false;
    }
    for (int i = 0; i < KeyArr.Num(); i++)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("StartMatchmaking set key: %s, value: %s"), *KeyArr[i], *ValueArr[i]));
        SearchSettings->QuerySettings.Set(FName(KeyArr[i]), ValueArr[i], EOnlineComparisonOp::Equals);
    }
    TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SearchSettings.ToSharedRef();
    bool Result = GameInterface->StartMatchmaking(GameLocalPlayers, TEXT("Game"), *SessionSettings, SearchSettingsRef);
    OnGameSessionStateChanged(FString::Printf(TEXT("StartMatchmaking ExecuteResult: %d"), Result));
    return Result;
}

bool UPicoGameInstance::CancelMatchmaking(FName SessionName) {
    if (!GameInterface)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("CancelMatchmaking GameInterface is invalid")));
        return false;
    }
    if (!OnCancelMatchmakingCompleteDelegate.IsBound())
    {
        OnCancelMatchmakingCompleteDelegate = FOnCancelMatchmakingCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnCancelMatchmakingComplete);
        GameInterface->AddOnCancelMatchmakingCompleteDelegate_Handle(OnCancelMatchmakingCompleteDelegate);
    }
    bool Result = GameInterface->CancelMatchmaking(0, TEXT("Game"));
    OnGameSessionStateChanged(FString::Printf(TEXT("CancelMatchmaking ExecuteResult: %d"), Result));
    return Result;
}

void UPicoGameInstance::StartSession(FName SessionName) {
    if (!GameInterface)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("StartSession GameInterface is invalid")));
        return;
    }
    auto Session = GameInterface->GetNamedSession(SessionName);
    if (Session)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("StartSession Session OwningUserName: %s"), *Session->OwningUserName));
    }
    if (!OnStartSessionCompleteDelegate.IsBound())
    {
        OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnStartSessionComplete);
        GameInterface->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);
    }
    bool Result = GameInterface->StartSession(SessionName);
    OnGameSessionStateChanged(FString::Printf(TEXT("StartSession ExecuteResult: %d"), Result));
}

void UPicoGameInstance::DestroySession(FName SessionName) {
    if (!GameInterface)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("DestroySession GameInterface is invalid")));
        return;
    }
    auto Session = GameInterface->GetNamedSession(SessionName);
    if (Session)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("DestroySession owned by %s, state: %s"), *Session->OwningUserName, EOnlineSessionState::ToString(Session->SessionState)));
    }
    if (!OnDestroySessionCompleteDelegate.IsBound())
    {
        OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnDestroySessionComplete);
        GameInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
    }

    bool Result = GameInterface->DestroySession(SessionName);
    OnGameSessionStateChanged(FString::Printf(TEXT("DestroySession ExecuteResult: %d"), Result));
}

bool UPicoGameInstance::UpdateSession(const FString& Keys, const FString& Values, bool bShouldAdvertise) {
    if (!GameInterface)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("UpdateSession GameInterface is invalid")));
        return false;
    }
    TSharedPtr<class FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
    SessionSettings->NumPublicConnections = 2;
    SessionSettings->bShouldAdvertise = bShouldAdvertise;
    TArray<FString> KeyArr, ValueArr;
    Keys.ParseIntoArray(KeyArr, TEXT(","), true);
    Values.ParseIntoArray(ValueArr, TEXT(","), true);
    if (ValueArr.Num() != KeyArr.Num())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("UpdateSession Input Error! keys count != values count")));
        return false;
    }
    for (int i = 0; i < KeyArr.Num(); i++)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("UpdateSession set key: %s, value: %s"), *KeyArr[i], *ValueArr[i]));
        SessionSettings->Set(FName(KeyArr[i]), ValueArr[i], EOnlineDataAdvertisementType::ViaOnlineService);
    }

    bool Result = GameInterface->UpdateSession(TEXT("Game"), *SessionSettings);
    OnGameSessionStateChanged(FString::Printf(TEXT("UpdateSession ExecuteResult: %d"), Result));
    return Result;
}

void UPicoGameInstance::EndSession(FName SessionName) {
    if (!GameInterface)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("EndSession GameInterface is invalid")));
        return;
    }
    auto Session = GameInterface->GetNamedSession(SessionName);
    if (Session) 
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("EndSession OwningUserName: %s, SessionState: %s"), *Session->OwningUserName, EOnlineSessionState::ToString(Session->SessionState)));
    }
    if (!OnEndSessionCompleteDelegate.IsBound()) {
        OnEndSessionCompleteDelegate = FOnEndSessionCompleteDelegate::CreateUObject(
            this, &UPicoGameInstance::OnEndSessionComplete);
        GameInterface->AddOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
    }
    bool Result = GameInterface->EndSession(SessionName);
    OnGameSessionStateChanged(FString::Printf(TEXT("EndSession ExecuteResult: %d"), Result));
}

bool UPicoGameInstance::FindSessions(const FString& Keys, const FString& Values, int MaxSearchResults)
{
    if (!GameInterface)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("FindSessions GameInterface is invalid")));
        return false;
    }

    // Add the delegate
    if (!OnFindSessionsCompleteDelegate.IsBound()) {
        OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnFindSessionComplete);
        GameInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
    }

    SearchSettings = MakeShareable(new FOnlineSessionSearch());
    SearchSettings->MaxSearchResults = MaxSearchResults;
    TArray<FString> KeyArr, ValueArr;
    Keys.ParseIntoArray(KeyArr, TEXT(","), true);
    Values.ParseIntoArray(ValueArr, TEXT(","), true);
    if (ValueArr.Num() != KeyArr.Num())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("FindSessions Input Error! keys count != values count")));
        return false;
    }
    for (int i = 0; i < KeyArr.Num(); i++)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("FindSessions set key: %s, value: %s"), *KeyArr[i], *ValueArr[i]));
        if (KeyArr[i] == "PICOMODERATEDROOMSONLY")
        {
            SearchSettings->QuerySettings.Set(FName(KeyArr[i]), ValueArr[i].ToBool(), EOnlineComparisonOp::Equals);
        }
        else
        {
            SearchSettings->QuerySettings.Set(FName(KeyArr[i]), ValueArr[i], EOnlineComparisonOp::Equals);
        }
    }
    TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SearchSettings.ToSharedRef();
    auto Result = GameInterface->FindSessions(0, SearchSettingsRef);
    OnGameSessionStateChanged(FString::Printf(TEXT("FindSessions ExecuteResult: %d"), Result));
    return Result;
}

int32 UPicoGameInstance::RefreshPlayerCountInSession()
{
    if (!GameInterface.IsValid())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("RefreshPlayerCountInSession GameInterface is invalid")));
        return 0;
    }
    auto Session = GameInterface->GetNamedSession(TEXT("Game"));
    if (!Session)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("RefreshPlayerCountInSession: cannot find session: Game")));
        return 0;
    }
    OnGameSessionStateChanged(FString::Printf(TEXT("RefreshPlayerCountInSession: count: %d"), Session->RegisteredPlayers.Num()));
    return Session->RegisteredPlayers.Num();
}

void UPicoGameInstance::UpdateSessionState(FName SessionName)
{
    if (!GameInterface)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("UpdateSessionState GameInterface is invalid")));
        return;
    }
    auto Session = GameInterface->GetNamedSession(SessionName);
    if (Session)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("UpdateSessionState SessionState: %s"), EOnlineSessionState::ToString(Session->SessionState)));
    }
    else
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("UpdateSessionState NoSession")));
    }
}

bool UPicoGameInstance::InAValidSession()
{
    if (!GameInterface.IsValid())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("InAValidSession GameInterface is invalid")));
        return false;
    }
    auto Session = GameInterface->GetNamedSession(TEXT("Game"));
    if (!Session)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("InAValidSession: false ( cannot GetNamedSession(Game) )")));
        return false;
    }
    OnGameSessionStateChanged(FString::Printf(TEXT("InAValidSession: %s ( SessionState: NoSession->false, Other->true )"), Session->SessionState != EOnlineSessionState::NoSession?TEXT("true"):TEXT("false")));
    return (Session->SessionState != EOnlineSessionState::NoSession);
}

void UPicoGameInstance::DumpSessions()
{
    if (!GameInterface.IsValid())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("DumpSessions GameInterface is invalid")));
        return;
    }
    LogSessionData(TEXT("Game"));
    GameInterface->DumpSessionState();
}

bool UPicoGameInstance::IsPlayerInSession(const FString& UniqueNetIdString)
{
    if (!GameInterface.IsValid())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("IsPlayerInSession GameInterface is invalid")));
        return false;
    }
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get(TEXT("Pico"));
    auto NetId = Subsystem->GetIdentityInterface()->CreateUniquePlayerId(UniqueNetIdString).ToSharedRef();
    OnGameSessionStateChanged(FString::Printf(TEXT("IsPlayerInSession Input: %s"), *NetId->ToString()));
    auto Result = GameInterface->IsPlayerInSession(TEXT("Game"), NetId.Get());
    OnGameSessionStateChanged(FString::Printf(TEXT("IsPlayerInSession ExecuteResult: %s"), Result?TEXT("true"):TEXT("false")));
    return Result;
}

bool UPicoGameInstance::FindSessionById(const FString& SearchingUserId, const FString& RoomId)
{
    if (!OnFindSessionByIdCompleteDelegate.IsBound())
    {
        OnFindSessionByIdCompleteDelegate = FOnSingleSessionResultCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnFindSessionByIdComplete);
    }
    return FindSessionById(SearchingUserId, RoomId, OnFindSessionByIdCompleteDelegate);
}

bool UPicoGameInstance::FindSessionById(const FString& SearchingUserId, const FString& RoomId, const FOnSingleSessionResultCompleteDelegate& CompleteDelegate)
{
    if (!GameInterface.IsValid())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("FindSessionById GameInterface is invalid")));
        return false;
    }
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get(TEXT("Pico"));
    auto SearchingUserIdPtr = Subsystem->GetIdentityInterface()->CreateUniquePlayerId(SearchingUserId).ToSharedRef();
    auto RoomIdPtr = Subsystem->GetIdentityInterface()->CreateUniquePlayerId(RoomId).ToSharedRef();
    OnGameSessionStateChanged(FString::Printf(TEXT("FindSessionById SearchingUserId: %s, RoomId: %s"), *SearchingUserIdPtr->ToString(), *RoomIdPtr->ToString()));

#if ENGINE_MAJOR_VERSION > 4
    auto FriendId = FUniqueNetIdString::EmptyId();
    OnFindSessionByIdCompleteDelegate = FOnSingleSessionResultCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnFindSessionByIdComplete);
    auto Result = GameInterface->FindSessionById(SearchingUserIdPtr.Get(), RoomIdPtr.Get(), FriendId.Get(), CompleteDelegate);
#elif ENGINE_MINOR_VERSION > 24
    FUniqueNetIdString FriendId = FUniqueNetIdString();
    OnFindSessionByIdCompleteDelegate = FOnSingleSessionResultCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnFindSessionByIdComplete);
    auto Result = GameInterface->FindSessionById(SearchingUserIdPtr.Get(), RoomIdPtr.Get(), FriendId, CompleteDelegate);
#endif



    OnGameSessionStateChanged(FString::Printf(TEXT("FindSessionById ExecuteResult: %d"), Result));
    return Result;
}

void UPicoGameInstance::JoinSessionByRoomId(const FString& SearchingUserId, const FString& RoomId)
{
    if (!OnFindSessionByIdCompleteAndJoinDelegate.IsBound())
    {
        OnFindSessionByIdCompleteAndJoinDelegate = FOnSingleSessionResultCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnFindSessionByIdCompleteAndJoin);
    }
    auto Result = FindSessionById(SearchingUserId, RoomId, OnFindSessionByIdCompleteAndJoinDelegate);
}

bool UPicoGameInstance::JoinSearchResultSession(int SearchResultIndex)
{
    if (!GameInterface)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("JoinSearchResultSession GameInterface is invalid")));
        return false;
    }
    if (!SearchSettings.IsValid() || SearchResultIndex < 0 || SearchSettings->SearchResults.Num() <= SearchResultIndex)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("JoinSearchResultSession JoinSession SearchSettings is invalid or SearchResultIndex invalid")));
        return false;
    }
    FOnlineSessionSearchResult SearchResult = SearchSettings->SearchResults[SearchResultIndex];
    OnGameSessionStateChanged(FString::Printf(TEXT("JoinSearchResultSession Trying to join OwningUserId:%s's session"), *SearchResult.Session.OwningUserId->ToString()));
    if (!OnJoinSessionCompleteDelegate.IsBound()) {
        OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnJoinSessionComplete);
        GameInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
    }
    auto Result = GameInterface->JoinSession(0, TEXT("Game"), SearchResult);
    OnGameSessionStateChanged(FString::Printf(TEXT("JoinSearchResultSession JoinSession ExecuteResult: %d"), Result));
    return Result;
}

void UPicoGameInstance::JoinFriendSession(const FString& UserID) {
    if (!GameInterface)
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("JoinFriendSession GameInterface is invalid")));
        return;
    }
    if (!OnJoinSessionCompleteDelegate.IsBound()) {
        OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnJoinSessionComplete);
        GameInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
    }
    if (!FriendsSessions.Contains(UserID))
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("JoinFriendSession input UserID cannot find!")));
        return;
    }
    auto Result = GameInterface->JoinSession(0, TEXT("Game"), FriendsSessions[UserID]);
    OnGameSessionStateChanged(FString::Printf(TEXT("JoinFriendSession JoinSession ExecuteResult: %d"), Result));
}

void UPicoGameInstance::ReadFriends(const FString& FriendsType)
{
	if (!FriendInterface.IsValid())
	{
		OnGameSessionStateChanged(FString::Printf(TEXT("ReadFriendsList FriendInterface is invalid")));
		return;
	}
	OnGameSessionStateChanged(
		FString::Printf(TEXT("ReadFriendsList input FriendsType is %s"), *FriendsType));
	bool Result = FriendInterface->ReadFriendsList(0, *FriendsType, FOnReadFriendsListComplete::CreateUObject(
		                                 this, &UPicoGameInstance::OnReadFriendsListComplete));
    OnGameSessionStateChanged(FString::Printf(TEXT("ReadFriendsList ExecuteResult: %d"), Result));
}

void UPicoGameInstance::InviteFriendToSession(const FString& FriendUserID)
{
	if (!GameInterface)
	{
	    OnGameSessionStateChanged(FString::Printf(TEXT("InviteFriendToSession FriendInterface is invalid")));
	    return;
	}
    auto UniqueNetIdRef = Online::GetIdentityInterface()->CreateUniquePlayerId(FriendUserID).ToSharedRef();
    bool Result = GameInterface->SendSessionInviteToFriend(0, TEXT("Game"), UniqueNetIdRef.Get());
    OnGameSessionStateChanged(FString::Printf(TEXT("SendSessionInviteToFriend ExecuteResult: %d"), Result));
}


// leaderboard
void UPicoGameInstance::ReadLeaderboards(TArray<FString> Players, const FString& LeaderboardName)
{
	auto UserId = IdentityInterface->GetUniquePlayerId(0);
    OnGameSessionStateChanged(FString::Printf(TEXT("ReadLeaderboards input LeaderboardName: %s"), *LeaderboardName));
	TArray<TSharedRef<const FUniqueNetId>> LeaderboardPlayers;
    for (int i = 0; i < Players.Num(); i++)
    {
        auto UniqueNetIdRef = Online::GetIdentityInterface()->CreateUniquePlayerId(Players[i]).ToSharedRef();
        OnGameSessionStateChanged(FString::Printf(TEXT("ReadLeaderboards add LeaderboardName: %s"), *LeaderboardName));
        LeaderboardPlayers.Add(UniqueNetIdRef);
    }
    LeaderboardInterface->ClearOnLeaderboardReadCompleteDelegates(this);
    if (!OnLeaderboardReadCompleteDelegate.IsBound())
    {
        OnLeaderboardReadCompleteDelegate = FOnLeaderboardReadCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnLeaderboardReadComplete);
    }
    LeaderboardInterface->AddOnLeaderboardReadCompleteDelegate_Handle(OnLeaderboardReadCompleteDelegate);

    // use FOnlineLeaderboardRead
    LeaderboardReadPtr = new FOnlineLeaderboardRead(); 
    auto Ref = new FOnlineLeaderboardReadRef(LeaderboardReadPtr);
    LeaderboardReadPtr->LeaderboardName = FName(LeaderboardName);
    bool Result = LeaderboardInterface->ReadLeaderboards(LeaderboardPlayers, *Ref);
    OnGameSessionStateChanged(FString::Printf(TEXT("use FOnlineLeaderboardRead Result: %s"), Result ? TEXT("true") : TEXT("false")));
}

void UPicoGameInstance::ReadLeaderboardsWithPicoObject(TArray<FString> Players, const FString& LeaderboardName, int PageIndex, int PageSize)
{
    TArray<TSharedRef<const FUniqueNetId>> LeaderboardPlayers;
    for (int i = 0; i < Players.Num(); i++)
    {
        auto UniqueNetIdRef = Online::GetIdentityInterface()->CreateUniquePlayerId(Players[i]).ToSharedRef();
        LeaderboardPlayers.Add(UniqueNetIdRef);
    }
    LeaderboardInterface->ClearOnLeaderboardReadCompleteDelegates(this);
    if (!OnLeaderboardReadWithPicoObjectCompleteDelegate.IsBound())
    {
        OnLeaderboardReadWithPicoObjectCompleteDelegate = FOnLeaderboardReadCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnLeaderboardReadWithPicoObjectComplete);
    }
    LeaderboardInterface->AddOnLeaderboardReadCompleteDelegate_Handle(OnLeaderboardReadWithPicoObjectCompleteDelegate);
    
    PicoLeaderboardReadPtr = new Pico_OnlineLeaderboardRead(LeaderboardName, PageIndex, PageSize);
    const auto PicoRef = new FOnlineLeaderboardReadRef(PicoLeaderboardReadPtr);
    const auto Result = LeaderboardInterface->ReadLeaderboards(LeaderboardPlayers, *PicoRef);
    OnGameSessionStateChanged(FString::Printf(TEXT("ReadLeaderboardDataWithPicoObject Result: %s"), Result ? TEXT("true") : TEXT("false")));
}

void UPicoGameInstance::ReadLeaderboardsForFriendsWithPicoObject(const FString& LeaderboardName, int PageIndex, int PageSize)
{
    PicoLeaderboardReadPtr = new Pico_OnlineLeaderboardRead(LeaderboardName, PageIndex, PageSize);
    const auto PicoRef = new FOnlineLeaderboardReadRef(PicoLeaderboardReadPtr);
    
    LeaderboardInterface->ClearOnLeaderboardReadCompleteDelegates(this);
    if (!OnLeaderboardReadWithPicoObjectCompleteDelegate.IsBound())
    {
        OnLeaderboardReadWithPicoObjectCompleteDelegate = FOnLeaderboardReadCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnLeaderboardReadWithPicoObjectComplete);
    }
    LeaderboardInterface->AddOnLeaderboardReadCompleteDelegate_Handle(OnLeaderboardReadWithPicoObjectCompleteDelegate);
    
    const auto Result = LeaderboardInterface->ReadLeaderboardsForFriends(0, *PicoRef);
	OnGameSessionStateChanged(FString::Printf(TEXT("ReadLeaderboardsForFriendsWithPicoObject ExecuteResult: %d"), Result));
}

void UPicoGameInstance::OnLeaderboardReadComplete(bool bWasSuccessful)
{
    OnGameSessionStateChanged(FString::Printf(TEXT("OnLeaderboardReadComplete bWasSuccessful: %d"), bWasSuccessful));
    PrintLeaderboardData(LeaderboardReadPtr, LeaderboardReadPtr->LeaderboardName.ToString());
}

void UPicoGameInstance::OnLeaderboardReadWithPicoObjectComplete(bool bWasSuccessful)
{
    OnGameSessionStateChanged(FString::Printf(TEXT("OnLeaderboardReadWithPicoObjectComplete bWasSuccessful: %d"), bWasSuccessful));
    PrintLeaderboardData(PicoLeaderboardReadPtr, PicoLeaderboardReadPtr->PicoLeaderboardName);
}

void UPicoGameInstance::PrintLeaderboardData(FOnlineLeaderboardRead* ReadPtr, const FString LeaderboardName)
{
    FStatsColumnArray ColumnArray;
    OnGameSessionStateChanged(FString::Printf(TEXT("PrintLeaderboardData Name: %s, ReadState: %d"),
    *LeaderboardName, ReadPtr->ReadState));
    for (auto row : ReadPtr->Rows)
    {
        FString Log;
        if (row.PlayerId.IsValid())
        {
            Log = FString::Printf(TEXT("PrintLeaderboardData LeaderboardEntry: PlayerID: %s, Nickname: %s, Rank: %i\n"),
               *row.PlayerId->ToString(),
               *row.NickName,
               row.Rank);
        }
        else
        {
            Log = FString::Printf(TEXT("PrintLeaderboardData LeaderboardEntry: Nickname: %s, Rank: %i\n"),
               *row.NickName,
               row.Rank);
        }
        FString ColumnsLog;
        for (auto col : row.Columns)
        {
            ColumnsLog.Append(FString::Printf(TEXT("row.Columns[ Key: %s, Value: %s]\n"),
                   *col.Key.ToString(),
                   *col.Value.ToString()));
        }
        OnGameSessionStateChanged(FString::Printf(TEXT("%s, %s"), *Log, *ColumnsLog));
    }
}

void UPicoGameInstance::WriteLeaderboards(const FString& LeaderboardName, const FString& ValueToWrite, int UpdateMethod, FString RatedStat)
{
	if (LeaderboardName.IsEmpty())
	{
		OnGameSessionStateChanged(FString::Printf(TEXT("WriteLeaderboards input LeaderboardName is empty!")));
		return;
	}
 
    OnGameSessionStateChanged(FString::Printf(TEXT("WriteLeaderboards Trying to write to leaderboard: %s with data: %s"), *LeaderboardName, *ValueToWrite));
	auto UserId = IdentityInterface->GetUniquePlayerId(0);
    if (!UserId.IsValid())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("WriteLeaderboards Please login first!")));
        return;
    }

    // test FOnlineLeaderboardWrite
    FOnlineLeaderboardWrite WriteObj;
    OnGameSessionStateChanged(FString::Printf(TEXT("WriteLeaderboards, use FOnlineLeaderboardWrite")));
    WriteObj.LeaderboardNames.Add(FName(LeaderboardName));
    WriteObj.RatedStat = FName(*RatedStat);
    FVariantData VariantData;
    VariantData.SetValue(FCString::Strtoi64(*ValueToWrite, NULL, 10));
    WriteObj.Properties.Add(*RatedStat, VariantData);
    // WriteObj.SetIntStat(*RatedStat, ValueToWrite);
    WriteObj.UpdateMethod = (ELeaderboardUpdateMethod::Type)UpdateMethod;
    if (IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::IsLoaded() ? IOnlineSubsystem::Get() : nullptr)
    {
        IOnlineLeaderboardsPtr Leaderboards = OnlineSub->GetLeaderboardsInterface();
        if (Leaderboards.IsValid())
        {
            bool bResult = Leaderboards->WriteLeaderboards(TEXT("test"), *UserId, WriteObj);
            OnGameSessionStateChanged(FString::Printf(TEXT("WriteLeaderboards bResult: %d"), bResult));
        }
    }
}

void UPicoGameInstance::WriteLeaderboardsWithPicoObject(const FString& LeaderboardName, const FString& ValueToWrite, int UpdateMethod, FString RatedStat)
{
	if (LeaderboardName.IsEmpty())
	{
		OnGameSessionStateChanged(FString::Printf(TEXT("WriteLeaderboardDataWithPicoObject input LeaderboardName is empty!")));
		return;
	}
 
    OnGameSessionStateChanged(FString::Printf(TEXT("WriteLeaderboardDataWithPicoObject Trying to write to leaderboard: %s with data: %s"), *LeaderboardName, *ValueToWrite));
	auto UserId = IdentityInterface->GetUniquePlayerId(0);
    if (!UserId.IsValid())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("WriteLeaderboardDataWithPicoObject Please login first!")));
        return;
    }

    OnGameSessionStateChanged(FString::Printf(TEXT("WriteLeaderboardDataWithPicoObject, use Pico_OnlineLeaderboardWrite")));
    Pico_OnlineLeaderboardWrite LeaderboardWriteObj;
    TArray<FString> LBNames;
    LBNames.Add(LeaderboardName);
    LeaderboardWriteObj.PicoLeaderboardNames = LBNames;
    LeaderboardWriteObj.RatedStat = FName(*RatedStat);// TEXT("SCORE");
    FVariantData VariantData;
    VariantData.SetValue(FCString::Strtoi64(*ValueToWrite, NULL, 10));
    LeaderboardWriteObj.Properties.Add(*RatedStat, VariantData);
    // LeaderboardWriteObj.SetIntStat(*RatedStat, ValueToWrite);
    LeaderboardWriteObj.UpdateMethod = (ELeaderboardUpdateMethod::Type)UpdateMethod;
    auto Result = LeaderboardInterface->WriteLeaderboards(TEXT("test"), *UserId, LeaderboardWriteObj);
    OnGameSessionStateChanged(FString::Printf(TEXT("WriteLeaderboardDataWithPicoObject ExecuteResult: %d"), Result));
}

// achievements
void UPicoGameInstance::WriteAchievements(const FString& AchievementName, const FString& Value)
{
    auto UserId = IdentityInterface->GetUniquePlayerId(0);
    if (!UserId.IsValid())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("WriteAchievements Please login first!")));
        return;
    }
    FOnlineAchievement Achievement;
    Achievement.Id = TEXT(""); // empty
    OnGameSessionStateChanged(FString::Printf(TEXT("WriteAchievements Trying to get cached achievement: %s"), *AchievementName));
    AchievementInterface->GetCachedAchievement(*UserId.Get(), AchievementName, Achievement);
    if (!Achievement.Id.IsEmpty()) {
        OnGameSessionStateChanged(FString::Printf(TEXT("WriteAchievements Trying to update player achievements to server....")));
        const FOnlineAchievementsWritePicoPtr WriteObject = MakeShareable(new FOnlineAchievementsWritePico());
        WriteObject->SetIntStat(FName(TEXT("USE_PICO_ACHIEVEMENT_WRITE")), 1);
        WriteObject->SetPicoIntStat(*AchievementName, FCString::Atoi(*Value));
        FOnlineAchievementsWriteRef WriteObjectRef = WriteObject.ToSharedRef();
        AchievementInterface->WriteAchievements(
            *UserId,
            WriteObjectRef,
            FOnAchievementsWrittenDelegate::CreateUObject(this, &UPicoGameInstance::OnAchievementsWriteComplete));
    }
    else
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("WriteAchievements cannot find the input achievement")));
    }
}
void UPicoGameInstance::OnAchievementsWriteComplete(const FUniqueNetId& PlayerId, bool bSuccessful)
{
    OnGameSessionStateChanged(FString::Printf(TEXT("OnAchievementsWriteComplete PlayerId: %s, bSuccessful: %s"), *PlayerId.ToString(), bSuccessful?TEXT("true"):TEXT("false")));
}
void UPicoGameInstance::RefreshAllAchievementsProgress()
{
    auto UserId = IdentityInterface->GetUniquePlayerId(0);
    if (!UserId.IsValid())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("RefreshAllAchievementsProgress Please login first!")));
        return;
    }
    OnGameSessionStateChanged(FString::Printf(TEXT("RefreshAllAchievementsProgress UserId: %s"), *UserId->ToString()));
    AchievementInterface->QueryAchievements(
        *UserId,
        FOnQueryAchievementsCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnRefreshAllAchievementsProgressComplete));
}
void UPicoGameInstance::OnRefreshAllAchievementsProgressComplete(const FUniqueNetId& PlayerId, bool bSuccessful)
{
    OnGameSessionStateChanged(FString::Printf(TEXT("OnRefreshAllAchievementsProgressComplete PlayerId: %s, bSuccessful: %s"), *PlayerId.ToString(), bSuccessful?TEXT("true"):TEXT("false")));
}
void UPicoGameInstance::RefreshAllAchievementsDefinition()
{
    auto UserId = IdentityInterface->GetUniquePlayerId(0);
    if (!UserId.IsValid())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("RefreshAllAchievementsDefinition Please login first!")));
        return;
    }
    AchievementInterface->QueryAchievementDescriptions(
        *UserId,
        FOnQueryAchievementsCompleteDelegate::CreateUObject(this, &UPicoGameInstance::OnRefreshAllAchievementsDefinitionComplete));
}
void UPicoGameInstance::OnRefreshAllAchievementsDefinitionComplete(const FUniqueNetId& PlayerId, bool bSuccessful)
{
    OnGameSessionStateChanged(FString::Printf(TEXT("OnRefreshAllAchievementsDefinitionComplete PlayerId: %s, bSuccessful: %s"), *PlayerId.ToString(), bSuccessful?TEXT("true"):TEXT("false")));
}
void UPicoGameInstance::GetCachedAchievement(const FString& AchievementName)
{
    auto UserId = IdentityInterface->GetUniquePlayerId(0);
    if (!UserId.IsValid())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("GetCachedAchievement Please login first!")));
        return;
    }
    FOnlineAchievement Achievement;
    const EOnlineCachedResult::Type Type = AchievementInterface->GetCachedAchievement(*UserId, AchievementName, Achievement);
    OnGameSessionStateChanged(FString::Printf(TEXT("GetCachedAchievement Type: %s, Achievement DebugString: %s")
        , Type == 0?TEXT("Success"):TEXT("NotFound"), *Achievement.ToDebugString()));
}
void UPicoGameInstance::GetCachedAchievements()
{
    auto UserId = IdentityInterface->GetUniquePlayerId(0);
    if (!UserId.IsValid())
    {
        OnGameSessionStateChanged(FString::Printf(TEXT("GetCachedAchievements Please login first!")));
        return;
    }
    const EOnlineCachedResult::Type Type = AchievementInterface->GetCachedAchievements(*UserId, Achievements);
    FString Log = FString::Printf(TEXT("GetCachedAchievements Type: %s, TArray Num: %d\n"), Type == 0?TEXT("Success"):TEXT("NotFound"), Achievements.Num());
    for (int i = 0; i < Achievements.Num(); i++)
    {
        Log += FString::Printf(TEXT("[%d]%s\n"), i, *Achievements[i].ToDebugString());
    }
    OnGameSessionStateChanged(Log);
}
void UPicoGameInstance::GetCachedAchievementDescription(const FString& AchievementName)
{
    FOnlineAchievementDesc AchievementDesc;
    const EOnlineCachedResult::Type Type = AchievementInterface->GetCachedAchievementDescription(AchievementName, AchievementDesc);
    FDateTime Time = AchievementDesc.UnlockTime;
    int year = Time.GetYear();
    int month = Time.GetMonth();
    int day = Time.GetDay();
    int hour = Time.GetHour();
    int minute = Time.GetMinute();
    int second = Time.GetSecond();
    OnGameSessionStateChanged(FString::Printf(TEXT("GetCachedAchievementDescription Type: %s, AchievementDesc DebugString: Title='%s', LockedDesc='%s', UnlockedDesc='%s', bIsHidden=%s, UnlockTime=[%d.%d.%d %d:%d:%d]")
        , Type == 0?TEXT("Success"):TEXT("NotFound"),
        *AchievementDesc.Title.ToString(),
        *AchievementDesc.LockedDesc.ToString(),
        *AchievementDesc.UnlockedDesc.ToString(),
        AchievementDesc.bIsHidden ? TEXT("true") : TEXT("false"),
        year, month, day, hour, minute, second));
}


