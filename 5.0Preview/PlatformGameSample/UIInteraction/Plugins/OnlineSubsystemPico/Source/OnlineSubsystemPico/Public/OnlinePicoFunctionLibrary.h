// Copyright 2022 Pico Technology Co., Ltd.All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc.In the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2022, Epic Games, Inc.All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OnlineSubsystemPicoManager.h"
#include "RTCPicoUserInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Net/OnlineBlueprintCallProxyBase.h"
#include "OnlinePicoFunctionLibrary.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnlinePicoVerifyAppDelegate, const int32&, code);
/// @file OnlinePicoFunctionLibrary.h
 

/// @brief Pico blueprint function library.
UCLASS()
class ONLINESUBSYSTEMPICO_API UOnlinePicoFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UOnlinePicoFunctionLibrary();
	static UOnlineSubsystemPicoManager* PicoSubsystemManager;

    /** @defgroup BlueprintFunction BlueprintFunction
     *  This is the BlueprintFunction group
     *  @{
     */

    /** @defgroup BP_Identity BP_Identity
     *  This is the BP_Identity group
     *  @{
     */

    // Pico Identity 
    
    /// <summary>Gets the account login information for the current device.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="LocalUserNum">The controller number of the user to get login information for.</param>   
    /// <param name ="UserId">User ID.</param>
    /// <param name ="InToken">User token.</param>
    /// <param name ="InType">User type.</param>
    /// <param name ="InLoginComleteDelegate">Callback function proxy.</param>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Identity")
    static void PicoLogin(UObject* WorldContextObject, int32 LocalUserNum, const FString& UserId, const FString& InToken, const FString& InType, FOnlineManagerLoginCompleteDelegate InLoginComleteDelegate);

    /// <summary>Gets a user's nickname.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="LocalUserNum">The controller number of the user to get nickname for.</param>   
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Identity")
    static FString PicoGetNickName(UObject* WorldContextObject, int32 LocalUserNum);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Identity")
    static UPico_User* GetLoginPicoUser(UObject* WorldContextObject, int32 LocalUserNum);


    /** @} */ // end of BP_Identity

    /** @defgroup BP_Friends BP_Friends
     *  This is the BP_Friends group
     *  @{
     */

	// Pico FriendInterface
    
    
    /// <summary>Reads user's friend list by account number.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="LocalUserNum">User's account number.</param>
    /// <param name ="ListName">The name of the list. Valid value is `Default` or `OnlinePlayers`.</param>
    /// <param name ="InReadFriendListDelegate">Callback function proxy.</param> 
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Friend")
	static void PicoReadFriendList(UObject* WorldContextObject, int32 LocalUserNum, const FString& ListName, FOnlineManagerReadFriendListDelegate InReadFriendListDelegate);

    
    /// <summary>Gets an arrary of friends for a specified user by friend list name.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="LocalUserNum">User's account number.</param>
    /// <param name ="ListName">The name of the list. Valid value is `Default` or `OnlinePlayers`.</param>
    /// <param name ="OutFriends">Returns an array of friends.</param>   
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Friend")
	static void PicoGetFriendList(UObject* WorldContextObject, int32 LocalUserNum, const FString& ListName, TArray<FPicoUserInfo> &OutFriends);


    
    /// <summary>Gets a user's friends by friend list name and friend ID.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="LocalUserNum">User's account number.</param>
    /// <param name ="FriendId">Friend ID.</param>
    /// <param name ="ListName">The name of the list. Valid value is `Default` or `OnlinePlayers`.</param>
    /// <returns>FPicoFriend Pico friend struct.</returns>  
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Friend")
	static FPicoUserInfo PicoGetFriend(UObject* WorldContextObject, int32 LocalUserNum, const FString& FriendId, const FString& ListName);


    /** @} */ // end of BP_Friends


    /** @defgroup BP_RTC BP_RTC
     *  This is the BP_RTC group
     *  @{
     */

	// Pico RtcInterface
    
    /// <summary>Gets the rtc token.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="UserId">User ID.</param>
    /// <param name ="RoomId">Room ID.</param>
    /// <param name ="Ttl">The effective duration of the room (in seconds).</param>
    /// <param name ="InValue">Channel effective time (in seconds).</param>
    /// <param name ="InRtcGetTokenDelegate">Callback function proxy.</param>  
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
	static void PicoRtcGetToken(UObject* WorldContextObject, const FString& UserId, const FString& RoomId, int Ttl, int InValue, FOnlineManagerRtcGetTokenDelegate InRtcGetTokenDelegate);

    // Function Call

    
    /// <summary>Initializes the RTC engine.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <returns>Voice The initialization result.</returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static ERtcEngineInitResult PicoGetRtcEngineInit(UObject* WorldContextObject);

    
    /// <summary>Joins a user to a rtc room.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="RoomId">Room ID.</param>
    /// <param name ="UserId">User ID.</param>
    /// <param name ="Token">Room token.</param>
    /// <param name ="UserExtra">Extra information added by the user.</param>
    /// <param name ="InRoomProfileType">Room type: 
    /// <ul>
    /// <li>`0`: communication room</li>
    /// <li>`1`: live broadcasting room</li> 
    /// <li>`2`: game room</li> 
    /// <li>`3`: cloud game room</li>
    /// <li>`4`: low-latency room</li>
    /// </ul>
    /// </param>
    /// <param name ="bIsAutoSubscribeAudio">Whether to automatically subscribe to the audio of the room: 
    /// <ul>
    /// <li>`true`: yes</li>
    /// <li>`false`: no</li>
    /// </ul>
    /// </param>
    /// <returns>Int:
    /// <ul>
    /// <li>`0`: success</li>
    /// <li>Other values: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
	static int PicoRtcJoinRoom(UObject* WorldContextObject, const FString& RoomId, const FString& UserId, const FString& Token, const FString& UserExtra, ERtcRoomProfileType InRoomProfileType, bool bIsAutoSubscribeAudio);

    
    /// <summary>Destroys a room.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="RoomId">Room ID.</param>
    /// <returns>Int: 
    /// <ul>
    /// <li>`0`: success</li>
    /// <li>Other values: failure</li>
    /// </ul>
    /// </returns>    
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static int RtcDestroyRoom(UObject* WorldContextObject, const FString& RoomId);

    
    /// <summary>Enables audio properties report. When this switch is turned on, you will regularly receive a statistical report of audio data.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="Interval">The interval (in milliseconds) between one report and the next. 
    /// You can set this parameter to `0` or any negative integer to stop receiving audio properties report. 
    /// For any integer between (0, 100), the SDK will regard it as invalid and automatically set this parameter to `100`; 
    /// Any integer equal to or greater than `100` is valid. 
    /// </param>   
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static void RtcEnableAudioPropertiesReport(UObject* WorldContextObject, int Interval);

    /// <summary>Leaves a rtc room.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="RoomId">Room ID.</param>
    /// <returns>
    /// <ul>
    /// <li>`0`: success</li>
    /// <li>Other values: failure</li>
    /// </ul>
    /// </returns> 
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static int RtcLeaveRoom(UObject* WorldContextObject, const FString& RoomId);


    /// <summary>Mutes local audio to make one's voice unable to be heard be others in the same room.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="InRtcMuteState">The state of local audio:
    /// <ul>
    /// <li> `0`: off</li>
    /// <li> `1`: on</li>
    /// </ul>
    /// </param>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static void RtcMuteLocalAudio(UObject* WorldContextObject, ERtcMuteState InRtcMuteState);

    
    /// <summary>Publishes local audio stream to a room, thereby making the voice heard be others in the same room.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="RoomId">Room ID.</param>     
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static void RtcPublishRoom(UObject* WorldContextObject, const FString& RoomId);

    
    /// <summary>Stops publishing local audio stream to a room, so others in the same room cannot hear the voice.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="RoomId">Room ID.</param>        
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static void RtcUnPublishRoom(UObject* WorldContextObject, const FString& RoomId);

    
    /// <summary>Pauses all subscribed streams of a room. Once paused, the voice of users in the room is blocked so nothing can be heard from this room.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param> 
    /// <param name ="RoomId">Room ID.</param> 
    /// <param name ="InPauseResumeMediaType">Media type.</param>  
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static void RtcRoomPauseAllSubscribedStream(UObject* WorldContextObject, const FString& RoomId, ERtcPauseResumeMediaType InPauseResumeMediaType);

    
    /// <summary>Resumes all subscribed streams of a room. Once resumed, the voice of users in the room can be heard again.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="RoomId">The ID of the room to resume subscribed streams for.</param>
    /// <param name ="InPauseResumeMediaType">Media type.</param>     
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static void RtcRoomResumeAllSubscribedStream(UObject* WorldContextObject, const FString& RoomId, ERtcPauseResumeMediaType InPauseResumeMediaType);

    
    /// <summary>Sets the type of audio playback device.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="InRtcAudioPlaybackDevice">Device type.</param>      
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static void RtcSetAudioPlaybackDevice(UObject* WorldContextObject, ERtcAudioPlaybackDevice InRtcAudioPlaybackDevice);

    
    /// <summary>Sets audio scenario type.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="InRtcAudioScenarioType">Audio scenario type: 
    /// <ul>
    /// <li>`0`: Music</li> 
    /// <li>`1`: HighQualityCommunication</li>  
    /// <li>`2`: Communication</li>  
    /// <li>`3`: Media</li> 
    /// <li>`4`: GameStreaming</li>  
    /// </ul>
    /// </param>     
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static void RtcSetAudioScenario(UObject* WorldContextObject, ERtcAudioScenarioType InRtcAudioScenarioType);

    
    /// <summary>Sets volume for audio capture.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="InRtcStreamIndex">Stream index main/screen.</param>
    /// <param name ="InVolume">The volume. 
    /// The valid value ranges from `0` to `400`. 
    /// `100` indicates keeping the original volume.
    /// </param>       
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static void RtcSetCaptureVolume(UObject* WorldContextObject, ERtcStreamIndex InRtcStreamIndex, int InVolume);

    
    /// <summary>Sets the in-ear monitoring mode.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="InRtcEarMonitorMode">The state of in-ear monitoring mode: 
    /// <ul>
    /// <li>`0`: off</li>
    /// <li>`1`: on</li>
    /// </ul>
    /// </param>      
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static void RtcSetEarMonitorMode(UObject* WorldContextObject, ERtcEarMonitorMode InRtcEarMonitorMode);

    
    /// <summary>Sets the volume for in-ear monitoring.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="InVolume">The volume. 
    /// The valid value ranges from `0` to `400`. 
    /// `100` indicates keeping the original volume.
    /// </param>       
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static void RtcSetEarMonitorVolume(UObject* WorldContextObject, int InVolume);

    
    /// <summary>Sets the playback volume.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="InVolume">The volume. 
    /// The valid value ranges from `0` to `400`. 
    /// `100` indicates keeping the original volume.
    /// </param>    
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static void RtcSetPlaybackVolume(UObject* WorldContextObject, int InVolume);

    
    /// <summary>Starts audio capture.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>    
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static void RtcStartAudioCapture(UObject* WorldContextObject);

    
    /// <summary>Stops audio capture.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>        
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static void RtcStopAudioCapture(UObject* WorldContextObject);

    
    /// <summary>Updates room token.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="RoomId">Room ID.</param>
    /// <param name ="Token">The new token.</param>    
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Rtc")
    static void RtcUpdateToken(UObject* WorldContextObject, const FString& RoomId, const FString& Token);
    /** @} */ // end of BP_RTC

    /** @defgroup BP_Session BP_Session
     *  This is the BP_Session group
     *  @{
     */

    // Game

    /// <summary>Creates a private session or a matchmaking session.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param> 
    /// <param name ="HostingPlayerNum">The index in the current userId array.</param> 
    /// <param name ="SessionName">The session name.</param> 
    /// <param name ="NewSessionSettings">The session settings.</param> 
    /// <param name ="OnCreateSessionCompleteDelegate">Will be executed when the request has been complete.</param> 
    /// <returns>Bool: 
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
    static bool CreateSession(UObject* WorldContextObject, int HostingPlayerNum, FName SessionName, const FPicoOnlineSessionSettings& NewSessionSettings, FPicoManagerOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate);

    /// <summary>Changes the session state to `InProgress`.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param> 
    /// <param name ="SessionName">The session name.</param> 
    /// <param name ="OnStartSessionCompleteDelegate">Will be executed when the request has been complete.</param> 
    /// <returns>Bool: 
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
    static bool StartSession(UObject* WorldContextObject, FName SessionName, FPicoManagerOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate);

    /// <summary>Updates the datastore of a session.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param> 
    /// <param name ="SessionName">The session name.</param> 
    /// <param name ="UpdatedSessionSettings">The settings with new datastore.</param> 
    /// <param name ="bShouldRefreshOnlineData">(not used)</param> 
    /// <param name ="OnUpdateSessionCompleteDelegate">Will be executed when the request has been complete.</param> 
    /// <returns>Bool: 
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
    static bool UpdateSession(UObject* WorldContextObject, FName SessionName, const FPicoOnlineSessionSettings& UpdatedSessionSettings, FPicoManagerOnUpdateSessionCompleteDelegate OnUpdateSessionCompleteDelegate, bool bShouldRefreshOnlineData = true);

    /// <summary>Changes the session state to `Ended`.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param> 
    /// <param name ="SessionName">The session name.</param> 
    /// <param name ="OnEndSessionCompleteDelegate">Will be executed when the request has been complete.</param> 
    /// <returns>Bool: 
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
    static bool EndSession(UObject* WorldContextObject, FName SessionName, FPicoManagerOnEndSessionCompleteDelegate OnEndSessionCompleteDelegate);

	// todo CompletionDelegate
    /// <summary>Destroys the current session.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param> 
    /// <param name ="SessionName">The session name.</param> 
    /// <param name ="OnDestroySessionCompleteDelegate">Will be executed when the request has been complete.</param> 
    /// <returns>Bool: 
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
    static bool DestroySession(UObject* WorldContextObject, FName SessionName, FPicoManagerOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate);

    /// <summary>Checks whether a player is in a session.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param> 
    /// <param name ="SessionName">The session name.</param> 
    /// <param name ="UniqueId">The unique ID of the player.</param> 
    /// <returns>Bool: 
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
    static bool IsPlayerInSession(UObject* WorldContextObject, FName SessionName, const FString& UniqueId);

    /// <summary>Starts matchmaking for a session.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="LocalPlayers">The logged-in users in the session.</param>
    /// <param name ="SessionName">The session name.</param>
    /// <param name ="NewSessionSettings">Set `NumPrivateConnections` to `0`.</param>
    /// <param name ="NewSessionSearch">Used to modify the search state.</param>
    /// <param name ="OnMatchmakingCompleteDelegate">Will be executed when the request has been complete.</param>
    /// <returns>Bool: 
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
    static bool StartMatchmaking(UObject* WorldContextObject, const TArray<FString>& LocalPlayers, FName SessionName, const FPicoOnlineSessionSettings& NewSessionSettings, UPARAM(ref)FPicoOnlineSessionSearch& NewSessionSearch, FPicoManagerOnMatchmakingCompleteDelegate OnMatchmakingCompleteDelegate);

    /// <summary>Cancels matchmaking for a session.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="SearchingPlayerNum">(not used)</param>
    /// <param name ="SessionName">The session name.</param>
    /// <param name ="OnCancelMatchmakingCompleteDelegate">Will be executed when the request has been complete.</param>
    /// <returns>Bool: 
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
    static bool CancelMatchmaking(UObject* WorldContextObject, int SearchingPlayerNum, FName SessionName, FPicoManagerOnCancelMatchmakingCompleteDelegate OnCancelMatchmakingCompleteDelegate);

    /// <summary>Finds matchmaking sessions or moderated sessions.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="SearchingPlayerNum">(not used)</param>
    /// <param name ="NewSessionSearch">The search settings.</param>
    /// <param name ="OnFindSessionCompleteDelegate">Will be executed when the request has been complete.</param>
    /// <returns>Bool: 
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
    static bool FindSessions(UObject* WorldContextObject, int32 SearchingPlayerNum, UPARAM(ref)FPicoOnlineSessionSearch& NewSessionSearch, FPicoManagerOnFindSessionCompleteDelegate OnFindSessionCompleteDelegate);

	// todo FriendId /// CompletionDelegate
    /// <summary>Gets session data by session ID.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="SearchingUserId">The ID of the logged-in player. If the played has not logged in, the session data will be unable to get.</param>
    /// <param name ="SessionId">The session ID.</param>
    /// <param name ="OnSingleSessionResultCompleteDelegate">Will be executed when the request has been complete.</param>
    /// <returns>Bool: 
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
    static bool FindSessionById(UObject* WorldContextObject, const FString& SearchingUserId, const FString& SessionId, FPicoManagerOnSingleSessionResultCompleteDelegate OnSingleSessionResultCompleteDelegate);

    /// <summary>Joins a session.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="PlayerNum">The `LocalOwnerId` of the session.</param>
    /// <param name ="SessionName">The name of the session to join.</param>
    /// <param name ="SearchResult">The search session result settings.</param>
    /// <param name ="OnJoinSessionCompleteDelegate">Will be executed when the request has been complete.</param>
    /// <returns>Bool: 
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
	static bool JoinSession(UObject* WorldContextObject, int PlayerNum, FName SessionName, const FPicoOnlineSessionSearchResult& SearchResult, FPicoManagerOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate);

    /// <summary>Dumps a session.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
	static void DumpSessionState(UObject* WorldContextObject);

    /// <summary>Gets the state of a session.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="SessionName">The name of the session to get state for.</param>
    /// <returns>The state of the session.</returns>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
	static EOnlineSessionStatePicoType GetSessionState(UObject* WorldContextObject, FName SessionName);

    /// <summary>Gets the data about a session.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="SessionName">The session name.</param>
    /// <returns>The data about the session.</returns>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
	static FPicoNamedOnlineSession GetNamedSession(UObject* WorldContextObject, FName SessionName);

    /// <summary>Adds session by session settings.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="SessionName">The session name.</param>
    /// <param name ="SessionSettings">The settings of the session.</param>
    /// <returns>The session added.</returns>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
	static FPicoNamedOnlineSession AddNamedSessionBySettings(UObject* WorldContextObject, FName SessionName, const FPicoOnlineSessionSettings& SessionSettings);

    /// <summary>Adds a session.</summary>
    /// <param name ="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name ="SessionName">The session name.</param>
    /// <param name ="Session">The session will be added.</param>
    /// <returns>The session added.</returns>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
	static FPicoNamedOnlineSession AddNamedSession(UObject* WorldContextObject, FName SessionName, const FPicoOnlineSession& Session);

	/// <summary>Gets the settings of a session.</summary>
	/// <param name ="WorldContextObject">Used to get the information about the current world.</param>
	/// <param name ="SessionName">The session name.</param>
	/// <returns>The settings of the session.</returns>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
	static FPicoOnlineSessionSettings GetSessionSettings(UObject* WorldContextObject, FName SessionName);

	
	/// <summary>Invites a friend to the session.</summary>
	/// <param name ="WorldContextObject">Used to get the information about the current world.</param>
	/// <param name ="LocalUserNum">The controller number of the friend to invite.</param>
	/// <param name ="SessionName">The session name.</param>
	/// <param name ="Friend">The name of the friend to invite.</param>
	/// <returns>The settings of the session.</returns>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Game")
	static bool SendSessionInviteToFriend(UObject* WorldContextObject, int32 LocalUserNum, FName SessionName, const FString& Friend);

    /** @} */ // end of BP_Session
	
private:
	static FOnlineSessionSettings GetOnlineSessionSettings(const FPicoOnlineSessionSettings& UpdatedSessionSettings);
	static FPicoOnlineSessionSettings GetPicoOnlineSessionSettings(const FOnlineSessionSettings& UpdatedSessionSettings);
	static FPicoNamedOnlineSession GetPicoOnlineSession(const FNamedOnlineSession& Session);
	static FOnlineSession GetOnlineSession(const FPicoOnlineSession& PicoSession);
	static bool IsInputSessionSettingsDataStoreValid(const FPicoOnlineSessionSettings& UpdatedSessionSettings);
	static bool IsInputSessionSearchQueryDataValid(const FPicoOnlineSessionSearch& SessionSearch);
	
public:

    /** @defgroup BP_Presence BP_Presence
     *  This is the BP_Presence group
     *  @{
     */

    //  Presence

    /// <summary>Clears a user's presence data in the current app.</summary>
    /// <param name="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name="InPresenceClearDelegate">Will be executed when the request has been complete.</param>
    /// <returns>Bool:
    /// * `true`: success
    /// * `false`: failure
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Presence")
    static bool PresenceClear(UObject* WorldContextObject, FOnlineManagerPresenceClearDelegate InPresenceClearDelegate);

    /// <summary>Reads a list of invitable users for a user. 
    /// @note Call `GetInvitableFriendList` after the Delegate has been executed.
    /// <param name="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name="SuggestedUserList">The ID list of the users suggested being invited. </param>
    /// <param name="InReadInvitableUserDelegate">Will be executed when the request has been complete.</param>
    /// <returns>Bool:
    /// * `true`: success
    /// * `false`: failure
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Presence")
    static void ReadInvitableUser(UObject* WorldContextObject, TArray<FString> SuggestedUserList, FOnlineManagerPresenceReadInvitableUserDelegate InReadInvitableUserDelegate);

    /// <summary>Gets a list of invitable friends for a user. These friends are previously retrieved from the online service when `PresenceGetDestinations` is complete.</summary>
    /// <param name="WorldContextObject">Used to get the information about the current world.</param>    
    /// <param name="OutFriendsList">The [out] array that receives the copied data.</param>
    /// <returns>Bool:
    /// * `true`: the friend list has been found
    /// * `false`: otherwise
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Presence")
    static bool GetInvitableFriendList(UObject* WorldContextObject, TArray<FPicoUserInfo>& OutFriendsList);

    /// <summary>Sets presence data for a user in the current app.</summary>
    /// <param name="WorldContextObject">Used to get the information about the current world.</param>    
    /// <param name="ApiName">The API name of the destination.</param> 
    /// <param name="LobbySessionId">Lobby session ID is used to identify a user group or team. Users with the same lobby session ID can play together or form a team in a game.</param> 
    /// <param name="MatchSessionId">Match session ID is used to identify all users within a same destination, such as maps and levels. Users with different lobby session IDs will have the same match session ID when playing the same match.</param> 
    /// <param name="bIsJoinable">Defines whether the user is joinable:
    /// * `true`: joinable
    /// * `false`: not joinable
    /// </param> 
    /// <param name="Extra">Extra presence data defined by the developer.</param> 
    /// <param name="InPresenceSetDelegate">Will be executed when the request has been complete.</param> 
    /// <returns>Bool:
    /// * `true`: success
    /// * `false`: failure
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Presence")
    static bool PresenceSet(UObject* WorldContextObject, const FString& ApiName, const FString& LobbySessionId, const FString& MatchSessionId, bool bIsJoinable, const FString& Extra, FOnlineManagerPresenceSetDelegate InPresenceSetDelegate);

    /// <summary>Replaces a user's current destination with the provided one.
    /// @note Other presence parameter settings will remain the same.
    /// </summary>
    /// <param name="WorldContextObject">Used to get the information about the current world.</param>   
    /// <param name="ApiName">The API name of the new destination.</param>
    /// <param name="InPresenceSetDestinationDelegate">Will be executed when the request has been complete.</param>
    /// <returns>Bool:
    /// * `true`: success
    /// * `false`: failure
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Presence")
    static bool PresenceSetDestination(UObject* WorldContextObject, const FString& ApiName, FOnlineManagerPresenceSetDestinationDelegate InPresenceSetDestinationDelegate);

    /// <summary>Sets whether a user is joinable.
    /// @note Other presence parameter settings will remain the same. If the destination or session
    /// ID has not been set up for the user, the user cannot be set as "joinable".
    /// </summary>
    /// <param name="WorldContextObject">Used to get the information about the current world.</param>  
    /// <param name="bIsJoinable">Defiens whether the user is joinable:
    /// * `true`: joinable
    /// * `false`: not joinable
    /// </param>
    /// <param name="InPresenceSetIsJoinableDelegate">Will be executed when the request has been complete.</param>
    /// <returns>Bool:
    /// * `true`: success
    /// * `false`: failure
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Presence")
    static bool PresenceSetIsJoinable(UObject* WorldContextObject, bool bIsJoinable, FOnlineManagerPresenceSetIsJoinableDelegate InPresenceSetIsJoinableDelegate);

    /// <summary>Replaces a user's current lobby session ID with the provided one.
    /// @note Other presence parameter settings will remain the same.
    /// </summary>
    /// <param name="WorldContextObject">Used to get the information about the current world.</param>  
    /// <param name="LobbySessionId">The new lobby session ID.</param>
    /// <param name="InPresenceSetLobbySessionDelegate">Will be executed when the request has been complete.</param>
    /// <returns>Bool:
    /// * `true`: success
    /// * `false`: failure
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Presence")
    static bool PresenceSetLobbySession(UObject* WorldContextObject, const FString& LobbySession, FOnlineManagerPresenceSetLobbySessionDelegate InPresenceSetLobbySessionDelegate);

    /// <summary>Replaces a user's current match session ID with the provided one.
    /// @note Other presence-realated parameters will remain the same.
    /// </summary>
    /// <param name="WorldContextObject">Used to get the information about the current world.</param> 
    /// <param name="MatchSessionId">The new match session ID.</param>
    /// <param name="InPresenceSetMatchSessionDelegate">Will be executed when the request has been complete.</param>
    /// <returns>Bool:
    /// * `true`: success
    /// * `false`: failure
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Presence")
    static bool PresenceSetMatchSession(UObject* WorldContextObject, const FString& MatchSession, FOnlineManagerPresenceSetMatchSessionDelegate InPresenceSetMatchSessionDelegate);

    /// <summary>Sets extra presence data for a user.</summary>
    /// <param name="WorldContextObject">Used to get the information about the current world.</param> 
    /// <param name="Extra">The extra presence data defined by the developer.</param> 
    /// <param name="InPresenceSetExtraDelegate">Will be executed when the request has been complete.</param> 
    /// <returns>Bool:
    /// * `true`: success
    /// * `false`: failure
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Presence")
    static bool PresenceSetExtra(UObject* WorldContextObject, const FString& Extra, FOnlineManagerPresenceSetExtraDelegate InPresenceSetExtraDelegate);

    /// <summary>Reads a list of sent invitations. 
    /// @note Call `GetSendInvitesList` after the Delegate has been executed.
    /// </summary>
    /// <param name="WorldContextObject">Used to get the information about the current world.</param>
    /// <param name="InPresenceReadSendInvitesDelegate">Will be executed when the request has been complete.</param>
    /// <returns>Bool:
    /// * `true`: success
    /// * `false`: failure
    /// <returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Presence")
    static bool PresenceReadSendInvites(UObject* WorldContextObject, FOnlineManagerPresenceReadSentInvitesDelegate InPresenceReadSendInvitesDelegate);

    /// <summary>Sends invitations to users.
    /// @note Call `GetSendInvitesList` after the Delegate has been executed.
    /// </summary>
    /// <param name="WorldContextObject">Used to get the information about the current world.</param> 
    /// <param name="UserIdArray">The ID array of the users to invite.</param>
    /// <param name="InPresenceSentInvitesDelegate">Will be executed when the request has been complete.</param>
    /// <returns>Bool:
    /// * `true`: success
    /// * `false`: failure
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Presence")
    static bool PresenceSendInvites(UObject* WorldContextObject, TArray<FString> UserIdArray, FOnlineManagerPresenceSentInvitesDelegate InPresenceSentInvitesDelegate);

    /// <summary>Gets a list of sent invitations when `PresenceSendInvites` or `PresenceReadSendInvites` is complete.</summary>
    /// <param name="WorldContextObject">Used to get the information about the current world.</param> 
    /// <param name="OutList">The [out] array that receives the copied data.</param>
    /// <returns>Bool:
    /// * `true`: the invitation list has been found
    /// * `false`: otherwise
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Presence")
    static bool GetSendInvitesList(UObject* WorldContextObject, TArray<FPicoApplicationInvite>& OutList);

    /// <summary>Gets all the destinations that can be set for a user.
    /// @note Call `PresenceGetDescriptionList` after the Delegate has been executed.
    /// </summary>
    /// <param name="WorldContextObject">Used to get the information about the current world.</param> 
    /// <param name="InPresenceGetDestinationsDelegate">Will be executed when the request has been complete.</param>
    /// <returns>Bool:
    /// * `true`: success
    /// * `false`: failure
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Presence")
    static bool PresenceGetDestinations(UObject* WorldContextObject, FOnlineManagerPresenceGetDestinationsDelegate InPresenceGetDestinationsDelegate);

    /// <summary>Gets a list of destinations when `PresenceGetDestinations` is complete.</summary>
    /// <param name="WorldContextObject">Used to get the information about the current world.</param> 
    /// <param name="OutList">The [out] array that receives the copied data.</param>
    /// <returns>Bool:
    /// * `true`: success
    /// * `false`: failure
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Presence")
    static bool PresenceGetDestinationsList(UObject* WorldContextObject, TArray<FPicoDestination>& OutList);

    /** @} */ // end of BP_Presence

    /** @defgroup BP_Application BP_Application
     *  This is the BP_Application group
     *  @{
     */

    // Application

    /// <summary>Launches a different app in a user's library.
    /// @note If the user does not have that app installed, the user will be directed to that app's page on the Pico Store.
    /// </summary>
    /// <param name="WorldContextObject">Used to get the information about the current world.</param> 
    /// <param name="AppID">The ID of the app to launch.</param> 
    /// <param name="Message">A message to be passed to the launched app.</param> 
    /// <param name="InLaunchOtherAppDelegate">Will be executed when the request has been complete.</param> 
    /// <returns>Bool:
    /// * `true`: success
    /// * `false`: failure
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Application")
    static bool LaunchOtherApp(UObject* WorldContextObject, const FString& AppID, const FString& Message, FOnlineManagerLaunchOtherAppDelegate InLaunchOtherAppDelegate);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Application")
    static bool GetVersion(UObject* WorldContextObject, FOnlineManagerGetVersionDelegate InGetVersionDelegate);

    /// <summary>Launches a different app in a user's library.
    /// @note If the user does not have that app installed, the user will be directed to that app's page on the Pico Store.
    /// </summary>
    /// <param name="WorldContextObject">Used to get the information about the current world.</param> 
    /// <param name="AppID">The ID of the app to launch.</param>
    /// <param name="PackageName">The package name of the app to launch.</param>
    /// <param name="Message">A message to be passed to the launched app.</param>
    /// <param name="ApiName">The API name of the destination in the app.</param>
    /// <param name="LobbySessionId">The lobby session ID of the user's presence, which identifies a user group or team. Users with the same lobby session ID can play together or form a team in a game.</param>
    /// <param name="MatchSessionId">The match session ID of the user's presence, which identifies all users within the same destination, such as maps and levels. Users with different lobby session IDs will have the same match session ID when playing the same match.</param>
    /// <param name="TrackId">The tracking ID of the app launch event.</param>
    /// <param name="Extra">Extra data defined by the developer.</param>
    /// <param name="InLaunchOtherAppByPresenceDelegate">Will be executed when the request has been complete.</param>
    /// <returns>Bool:
    /// * `true`: success
    /// * `false`: failure
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Application")
    static bool LaunchOtherAppByPresence(UObject* WorldContextObject, const FString& AppID, const FString& PackageName, const FString& Message, const FString& ApiName, const FString& LobbySessionId, const FString& MatchSessionId, const FString& TrackId, const FString& Extra, FOnlineManagerLaunchOtherAppByPresenceDelegate InLaunchOtherAppByPresenceDelegate);


    /** @} */ // end of BP_Application

    /** @defgroup BP_ApplicationLifecycle BP_ApplicationLifecycle
     *  This is the BP_ApplicationLifecycle group
     *  @{
     */

    // ApplicationLifecycle

    /// <summary>Gets information about how the app was launched.</summary>
    /// <param name="WorldContextObject">Used to get the information about the current world.</param> 
    /// <param name="OutLaunchDetails">The [out] struct of launch details.</param> 
    /// <returns>Bool:
    /// * `true`: success
    /// * `false`: failure
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|ApplicationLifecycle")
    static bool GetLaunchDetails(UObject* WorldContextObject, FLaunchDetails& OutLaunchDetails);

    /// <summary>Logs if the user has been successfully directed to the desired destination via a deep link.</summary>
    /// <param name="WorldContextObject">Used to get the information about the current world.</param> 
    /// <param name="TrackingID">The tracking ID of the app launch event.</param>
    /// <param name="LaunchResult">Enumerations of the launch result:
    /// * `Unknown`
    /// * `Success`
    /// * `FailedRoomFull`
    /// * `FailedGameAlreadyStarted`
    /// * `FailedRoomNotFound`
    /// * `FailedUserDeclined`
    /// * `FailedOtherReason`
    /// </param>
    /// <returns>Bool:
    /// * `true`: the result has been logged
    /// * `false`: failed to log the result
    /// </returns>
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|ApplicationLifecycle")
    static bool LogDeeplinkResult(UObject* WorldContextObject, const FString& TrackingID, ELaunchResult LaunchResult);

    /** @} */ // end of BP_ApplicationLifecycle

	// Leaderboard
	
    /// <summary>Get entries of a leaderboard
    /// <br><b><i>Note</i></b>
    /// </summary>
    ///
    /// <param name="WorldContextObject">Used to get the information about the current world. </param>
    /// <param name="Players">If the num of Players is bigger than zero, and the only Player in it is the logged in Player, then the value of StartAt will be ppfLeaderboard_StartAtCenteredOnViewer</param>
    /// <param name="PicoReadObject">Set leaderboard name in it</param>
    /// <param name ="OnReadLeaderboardsCompleteDelegate">Executes this parameter after the request has been done.</param> 
    /// <returns>Bool: 
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Leaderboard")
	static bool ReadLeaderboards(UObject* WorldContextObject, const TArray<FString>& Players, UPARAM(ref)FPicoOnlineLeaderboardRead& PicoReadObject, FPicoManagerOnReadLeaderboardsCompleteDelegate OnReadLeaderboardsCompleteDelegate);
	
	/// <summary>Get entries of a leaderboard
    /// <br><b><i>Note</i></b>
    /// </summary>
    ///
    /// <param name="WorldContextObject">Used to get the information about the current world. </param>
    /// <param name="LocalUserNum">not used</param>
    /// <param name="PicoReadObject">Set leaderboard name in it</param>
    /// <param name ="OnReadLeaderboardsCompleteDelegate">Executes this parameter after the request has been done.</param> 
    /// <returns>Bool: 
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Leaderboard")
	static bool ReadLeaderboardsForFriends(UObject* WorldContextObject, int32 LocalUserNum, UPARAM(ref)FPicoOnlineLeaderboardRead& PicoReadObject, FPicoManagerOnReadLeaderboardsCompleteDelegate OnReadLeaderboardsCompleteDelegate);
	
	/// <summary>Writes a entry of a leaderboard
    /// <br><b><i>Note</i></b>
    /// </summary>
    ///
    /// <param name="WorldContextObject">Used to get the information about the current world. </param>
    /// <param name="SessionName">not used</param>
    /// <param name="Player">Need be the logged in player</param>
    /// <param name="PicoWriteObject">Set leaderboard name and the score in it</param>
    /// <returns>Bool: 
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OnlinePico|Leaderboard")
	static bool WriteLeaderboards(UObject* WorldContextObject, const FString& SessionName, const FString& Player, UPARAM(ref)FPicoOnlineLeaderboardWrite& PicoWriteObject);

	
    // Old Online Pico
public:
    static FOnlinePicoVerifyAppDelegate VerifyAppDelegate;
    static int32 VerifyAppCode;

    /** @defgroup BP_Common BP_Common
     *  This is the BP_Common group
     *  @{
     */

    /// <summary>Gets the class of online subsystem Pico manager for binding notifications.</summary>
    /// <returns>The UOnlineSubsystemPicoManager class.</returns>
    UFUNCTION(BlueprintPure, Category = "OnlinePico")
    static UOnlineSubsystemPicoManager* GetOnlineSubsystemPicoManager();

    /** @} */ // end of BP_Common
    /** @} */ // end of BlueprintFunction
    // Old Online Pico
    UFUNCTION(BlueprintCallable, Category = "OnlinePico|PicoEntitlement")
    static void PicoEntitlementVerifyAppDelegate(FOnlinePicoVerifyAppDelegate OnVerifyAppCallback);

    UFUNCTION(BlueprintCallable, Category = "OnlinePico|PicoEntitlement")
    static void PicoEntitlementVerifyCheck();

    UFUNCTION(BlueprintCallable, Category = "OnlinePico|PicoEntitlement")
    static FString PicoGetDeviceSN();
};

