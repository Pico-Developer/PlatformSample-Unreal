// Copyright 2022 Pico Technology Co., Ltd.All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc.In the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2022, Epic Games, Inc.All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemPico.h"
#include "OnlineSubsystemPicoPackage.h"
#include "OnlineSubsystemPicoNames.h"

/// @file RTCPicoUserInterface.h

DECLARE_LOG_CATEGORY_EXTERN(RtcInterface, Log, All);

/// <summary>Rtc engine privilege enum.</summary>
UENUM(BlueprintType)
enum class ERtcPrivilege : uint8
{
    None,
    PublishStream,
    PublishAudioStream,
    PublishVideoStream,
    SubscribeStream
};

/// <summary>Rtc engine init result enum.</summary>
UENUM(BlueprintType)
enum class ERtcEngineInitResult : uint8
{
    None,
    Unknow,
    AlreadyInitialized,
    InvalidConfig,
    Success
};

/// <summary>Rtc engine room profile type enum.</summary>
UENUM(BlueprintType)
enum class ERtcRoomProfileType : uint8
{
    None,
    Communication,
    LiveBroadcasting,
    Game,
    CloundGame,
    LowLatency
};

/// <summary>Rtc mute state enum.</summary>
UENUM(BlueprintType)
enum class ERtcMuteState : uint8
{
    None,
    Off,
    On
};

/// <summary>Rtc pause or resume media type enum.</summary>
UENUM(BlueprintType)
enum class ERtcPauseResumeMediaType : uint8
{
    None,
    Audio,
    Video,
    AudioAndVideo
};

/// <summary>Rtc audio playback device enum.</summary>
UENUM(BlueprintType)
enum class ERtcAudioPlaybackDevice : uint8
{
    None,
    Headset,
    EarPiece,
    SpeakerPhone,
    HeadsetBlueTooth,
    HeadsetUsb
};

/// <summary>Rtc audio scenario type enum.</summary>
UENUM(BlueprintType)
enum class ERtcAudioScenarioType : uint8
{
    None,
    Music,
    HighQualityCommunication,
    Communication,
    Media,
    GameStreaming
};

/// <summary>Rtc audio stream index enum.</summary>
UENUM(BlueprintType)
enum class ERtcStreamIndex : uint8
{
    None,
    Main,
    Screen
};

/// <summary>Rtc ear monitor mode enum.</summary>
UENUM(BlueprintType)
enum class ERtcEarMonitorMode : uint8
{
    None,
    Off,
    On
};

/// <summary>Rtc join room type enum.</summary>
UENUM(BlueprintType)
enum class ERtcJoinRoomType : uint8
{
    None,
    First,
    Reconnected
};

/// <summary>Rtc user leave reason type.</summary>
UENUM(BlueprintType)
enum class ERtcUserLeaveReasonType : uint8
{
    None,
    Quit,
    Dropped
};

/// <summary>Rtc media device type.</summary>
UENUM(BlueprintType)
enum class ERtcMediaDeviceType : uint8
{
    None,
    AudioUnknown,
    AudioRenderDevice,
    AudioCaptureDevice
};

/// <summary>Rtc media device state.</summary>
UENUM(BlueprintType)
enum class ERtcMediaDeviceState : uint8
{
    None,
    Started,
    Stopped,
    RuntimeError,
    Added,
    Removed
};

/// <summary>Rtc media device error type.</summary>
UENUM(BlueprintType)
enum class ERtcMediaDeviceError : uint8
{
    None,
    Ok,
    NoPermission,
    DeviceBusy,
    DeviceFailure,
    DeviceNotFound,
    DeviceDisconnected,
    DeviceNoCallback,
    UnSupporttedFormat
};

// Request
DECLARE_DELEGATE_ThreeParams(FOnGetTokenComplete, const FString& /*String Token*/, bool /*IsSuccessed*/, const FString& /*Error Message*/);

// Notification
DECLARE_MULTICAST_DELEGATE_FiveParams(FRtcJoinRoomResult, const FString& /*RoomId*/, const FString& /*UserId*/, int /*ErrorCode*/, int /*Elapsed*/, ERtcJoinRoomType /*JoinRoomType*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FRtcLeaveRoomReault, const FString& /*RoomId*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FRtcRoomState, int /*TotalDuration*/, int /*UserCount*/, const FString& /*RoomId*/);
DECLARE_MULTICAST_DELEGATE_FourParams(FRtcUserJoinInfo, const FString& /*UserId*/, const FString& /*UserExtra*/, int /*Elapsed*/, const FString& /*RoomId*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FRtcUserLeaveInfo, const FString& /*UserId*/, ERtcUserLeaveReasonType /*RtcUserLeaveReasonType*/, const FString& /*RoomId*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FRtcRoomWarn, int /*Code*/, const FString& /*RoomId*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FRtcRoomError, int /*Code*/, const FString& /*RoomId*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FRtcUserMuteAudioInfo, const FString& /*UserId*/, ERtcMuteState /*RtcMuteState*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FRtcAudioChangePlaybackDevice, ERtcAudioPlaybackDevice /*RtcAudioPlaybackDevice*/);
DECLARE_MULTICAST_DELEGATE_FourParams(FRtcMediaDeviceChangeInfo, const FString& /*DeviceId*/, ERtcMediaDeviceType /*MediaDeciveType*/, ERtcMediaDeviceState /*MediaDeviceState*/, ERtcMediaDeviceError /*MediaDeviceError*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FRtcLocalAudioPropertiesReport, TArray<ERtcStreamIndex>  /*StreamIndex*/, TArray<int> /*Volume Array*/);
DECLARE_MULTICAST_DELEGATE_FiveParams(FRtcRemoteAudioPropertiesReport, int /*TotalRemoteVolume*/, TArray<int> /*Volume Array*/, const TArray<FString>& /*RoomId Array*/, const TArray<FString>&  /*UserId Array*/, TArray<ERtcStreamIndex>  /*StreamIndex Array*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FRtcStringResult, const FString& /*MessageString*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FRtcIntResult, int /*MessageCode*/);


/** @addtogroup Function Function
 *  This is the Function group
 *  @{
 */

/** @defgroup RTC RTC
 *  This is the RTC group
 *  @{
 */

/// <summary>Pico RTC interface class.</summary>
class ONLINESUBSYSTEMPICO_API FRTCPicoUserInterface
{
private:

	FOnlineSubsystemPico& PicoSubsystem;

	
public:
	FRTCPicoUserInterface(FOnlineSubsystemPico& InSubsystem);
	~FRTCPicoUserInterface();

	// Rtc Function


    /// <summary>Initializes the RTC engine.</summary>
    /// <returns>The initialization result.</returns>
	ERtcEngineInitResult RtcEngineInit();


    /// <summary>Joins a user to a room.</summary>
    /// <param name="RoomId">Room ID.</param>
    /// <param name="UserId">User ID.</param>
    /// <param name="Token">Room token.</param>
    /// <param name="UserExtra">Extra information added by the user.</param>
    /// <param name="InRoomProfileType">Room type: 
    /// <ul>
    /// <li>`0`: communication room</li>
    /// <li>`1`: live broadcasting room</li> 
    /// <li>`2`: game room</li>
    /// <li>`3`: cloud game room</li>
    /// <li>`4`: low-latency room</li>
    /// </ul>
    /// </param>
    /// <param name="bIsAutoSubscribeAudio">Whether to automatically subscribe to the audio of the room: 
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
	int RtcJoinRoom(const FString& RoomId, const FString& UserId, const FString& Token, const FString& UserExtra, ERtcRoomProfileType InRoomProfileType, bool bIsAutoSubscribeAudio);
	

    /// <summary>Destroys a room.</summary>
    /// <param name="RoomId">Room ID</param>
    /// <returns>Int:
    /// <ul>
    /// <li>`0`: success</li> 
    /// <li>Other values: failure</li>
    /// </ul>
    /// </returns>
    int RtcDestroyRoom(const FString& RoomId);


    /// <summary>Enables audio properties report. When this switch is turned on, you will regularly receive a statistical report of audio data.</summary>
    /// <param name="Interval">The interval (in milliseconds) between one report and the next. 
    /// You can set this parameter to `0` or any negative integer to stop receiving audio properties report. 
    /// For any integer between (0, 100), the SDK will regard it as invalid and automatically set this parameter to `100`; 
    /// Any integer equal to or greater than `100` is valid.
    /// </param>
	void RtcEnableAudioPropertiesReport(int Interval);


    /// <summary>Leaves a room.</summary>
    /// <param name="RoomId">Room ID.</param>
    /// <returns>Int:
    /// <ul>
    /// <li>`0`: success</li> 
    /// <li>Other values: failure</li>
    /// </ul>
    /// </returns>
	int RtcLeaveRoom(const FString& RoomId);


    /// <summary>Mutes local audio to make one's voice unable to be heard be others in the same room.</summary>
    /// <param name="InRtcMuteState">The state of local audio: 
    /// <ul>
    /// <li>`0`: off</li>
    /// <li>`1`: on</li>
    /// </ul>
    /// </param>
	void RtcMuteLocalAudio(ERtcMuteState InRtcMuteState);


    /// <summary>Publishes local audio stream to a room, thereby making the voice heard be others in the same room.</summary>
    /// <param name="RoomId">Room ID.</param>
	void RtcPublishRoom(const FString& RoomId);


    /// <summary>Pauses all subscribed streams of a room. Once paused, the voice of users in the room is blocked so nothing can be heard from this room.</summary>
    /// <param name="RoomId">Room id.</param>
    /// <param name="InPauseResumeMediaType">Media type.</param>
	void RtcRoomPauseAllSubscribedStream(const FString& RoomId, ERtcPauseResumeMediaType InPauseResumeMediaType);


    /// <summary>Resumes all subscribed streams of a room. Once resumed, the voice of users in the room can be heard again.</summary>
    /// <param name="RoomId">Room ID.</param>
    /// <param name="InPauseResumeMediaType">Media type.</param>
	void RtcRoomResumeAllSubscribedStream(const FString& RoomId, ERtcPauseResumeMediaType InPauseResumeMediaType);


    /// <summary>Sets the audio playback device.</summary>
	/// <param name="InRtcAudioPlaybackDevice">Device type:
    /// <ul>
    /// <li>`1`: Headset</li>
    /// <li>`2`: EarPiece</li> 
    /// <li>`3`: SpeakerPhone</li>
    /// <li>`4`: HeadsetBlueTooth</li>
    /// <li>`5`: HeadsetUsb</li>
    /// </ul>
    /// </param>
	void RtcSetAudioPlaybackDevice(ERtcAudioPlaybackDevice InRtcAudioPlaybackDevice);


    /// <summary>Sets audio scenario type.</summary>
    /// <param name="InRtcAudioScenarioType">Audio scenario type: 
    /// <ul>
    /// <li>`0`: Music</li> 
    /// <li>`1`: HighQualityCommunication</li> 
    /// <li>`2`: Communication</li> 
    /// <li>`3`: Media</li>
    /// <li>`4`: GameStreaming</li>
    /// </ul>
    /// </param>
	void RtcSetAudioScenario(ERtcAudioScenarioType InRtcAudioScenarioType);


    /// <summary> Sets volume for audio capture.</summary>
    /// <param name="InRtcStreamIndex">Stream index main/screen.</param>
    /// <param name="InVolume">The volume. 
    /// The valid value ranges from `0` to `400`. 
    /// `100` indicates keeping the original volume.
    /// </param>
	void RtcSetCaptureVolume(ERtcStreamIndex InRtcStreamIndex, int InVolume);


    /// <summary>Sets the in-ear monitoring mode.</summary>
    /// <param name="InRtcEarMonitorMode">The state of in-ear monitoring mode:
    /// <ul>
    /// <li>`0`: off</li>
    /// <li>`1`: on</li>
    /// </ul>
    /// </param>
	void RtcSetEarMonitorMode(ERtcEarMonitorMode InRtcEarMonitorMode);


    /// <summary>Sets volume for in-ear monitoring.</summary>
    /// <param name="InVolume">The volume. 
    /// The valid value ranges from `0` to `400`. 
    /// `100` indicates keeping the original volume.
    /// </param>
	void RtcSetEarMonitorVolume(int InVolume);


    /// <summary>Sets the playback volume.</summary>
    /// <param name="InVolume">The volume. 
    /// The valid value ranges from `0` to `400`. 
    /// `100` indicates keeping the original volume.
    /// </param>
	void RtcSetPlaybackVolume(int InVolume);


    /// <summary>Starts audio capture.</summary>
	void RtcStartAudioCapture();


    /// <summary>Stops audio capture.</summary>
	void RtcStopAudioCatpure();


    /// <summary>Stops publishing local audio stream to a room, so others in the same room cannot hear the voice.</summary>
    /// <param name="RoomId">Room ID.</param>
	void RtcUnPublishRoom(const FString& RoomId);


    /// <summary>Updates room token.</summary>
    /// <param name="RoomId">Room ID.</param>
    /// <param name="Token">The new token.</param>
	void RtcUpdateToken(const FString& RoomId, const FString& Token);

    //Request

    /// <summary>Gets the token.</summary>
    /// <param name="UserId">User ID.</param>
    /// <param name="RoomId">Room ID.</param>
    /// <param name="Ttl">The effective duration (in seconds) of room.</param>
    /// <param name="InValue">Channel effective time (in seconds).</param>
    /// <param name="InRtcGetTokenDelegate">Callback function proxy.</param>
    void GetToken(const FString& UserId, const FString& RoomId, int Ttl, int InValue, const FOnGetTokenComplete& Delegate = FOnGetTokenComplete());

PACKAGE_SCOPE:


	void OnQueryGetTokenComplete(ppfMessageHandle Message, bool bIsError, const FOnGetTokenComplete& Delegate);


	// Notification

	FDelegateHandle OnJoinRoomNotificationResultHandle;
	void OnJoinRoomResult(ppfMessageHandle Message, bool bIsError);

	FDelegateHandle OnLeaveRoomNotificationResultHandle;
	void OnLeaveRoomResult(ppfMessageHandle Message, bool bIsError);

	FDelegateHandle OnUserJoinRoomNotificationResultHandle;
	void OnUserJoinRoomResult(ppfMessageHandle Message, bool bIsError);

	FDelegateHandle OnUserLeaveRoomNotificationResultHandle;
	void OnUserLeaveRoomResult(ppfMessageHandle Message, bool bIsError);

	FDelegateHandle OnRoomStatsNotificationHandle;
	void OnRoomStatsNotification(ppfMessageHandle Message, bool bIsError);

	FDelegateHandle OnWarnNotificationHandle;
	void OnWarnNotification(ppfMessageHandle Message, bool bIsError);

	FDelegateHandle OnErrorNotificationHandle;
	void OnErrorNotification(ppfMessageHandle Message, bool bIsError);

	FDelegateHandle OnRoomWarnNotificationHandle;
	void OnRoomWarnNotification(ppfMessageHandle Message, bool bIsError);

	FDelegateHandle OnRoomErrorNotificationHandle;
	void OnRoomErrorNotification(ppfMessageHandle Message, bool bIsError);

	FDelegateHandle OnConnectionStateChangeNotificationHandle;
	void OnConnectionStateChangeNotification(ppfMessageHandle Message, bool bIsError);

	FDelegateHandle OnUserStartAudioCaptureNotificationHandle;
	void OnUserStartAudioCaptureNotification(ppfMessageHandle Message, bool bIsError);

	FDelegateHandle OnUserStopAudioCaptureNotificationHandle;
	void OnUserStopAudioCaptureNotification(ppfMessageHandle Message, bool bIsError);

	FDelegateHandle OnAudioPlaybackDeviceChangedNotificationHandle;
	void OnAudioPlaybackDeviceChangeNotification(ppfMessageHandle Message, bool bIsError);

	FDelegateHandle OnRemoteAudioPropertiesReportNotificationHandle;
	void OnRemoteAudioPropertiesReportNotification(ppfMessageHandle Message, bool bIsError);

	FDelegateHandle OnLocalAudioPropertiesReportNotificationHandle;
	void OnLocalAudioPropertiesReportNotification(ppfMessageHandle Message, bool bIsError);

	FDelegateHandle OnUserMuteAudioNotificationHandle;
	void OnUserMuteAudioNotification(ppfMessageHandle Message, bool bIsError);

	FDelegateHandle OnMediaDeviceStateChangedNotificationHandle;
	void OnMediaDeviceStateChangedNotification(ppfMessageHandle Message, bool bIsError);


public:

    /// <summary>Gets notified after the user has joined the room.</summary>
	FRtcJoinRoomResult RtcJoinRoomCallback;

    /// <summary>Gets notified after the user has left the room.</summary>
	FRtcLeaveRoomReault RtcLeaveRoomCallback;

    /// <summary>Gets notified when other users have joined the room.</summary>
	FRtcUserJoinInfo RtcUserJoinInfoCallback;

    /// <summary>Gets notified when other users have left the room.</summary>
	FRtcUserLeaveInfo RtcUserLeaveInfoCallback;

    /// <summary>Gets notified of room information.</summary>
	FRtcRoomState RtcRoomStateCallback;

    /// <summary>Gets the warning from the room.</summary>
	FRtcRoomWarn RtcRoomWarnCallback;

    /// <summary>Gets the error occurred in the room.</summary>
	FRtcRoomError RtcRoomErrorCallback;

    /// <summary>Gets notified when the audio playback device has been changed.</summary>
	FRtcAudioChangePlaybackDevice RtcAudioPlaybackDeviceChangeCallback;

    /// <summary>Gets the volume of each user's voice.</summary>
	FRtcRemoteAudioPropertiesReport RtcRemoteAudioPropertiesReportCallback;

    /// <summary>Gets the volume of the current user's voice.</summary>
	FRtcLocalAudioPropertiesReport RtcLocalAudioPropertiesReportCallback;

    /// <summary>Gets notified when the user has been muted.</summary>
	FRtcUserMuteAudioInfo RtcUserMuteAudioInfoCallback;

    /// <summary>Gets notified by the media device has been changed.</summary>
	FRtcMediaDeviceChangeInfo RtcMediaDeviceChangeInfoCallback;

    /// <summary>Gets the warning from the RTC engine.</summary>
    FRtcIntResult RtcWarnCallback;

    /// <summary>Gets the error from the RTC engine.</summary>
    FRtcIntResult RtcErrorCallback;

    /// <summary>Gets notified when the network connection status has changed.</summary>
    FRtcStringResult RtcConnectStateChangedCallback;

    /// <summary>Gets notified when the user has turned on audio capture.</summary>
    FRtcStringResult RtcUserStartAudioCaptureCallback;

    /// <summary>Gets notified when the user has turned off audio capture.</summary>
    FRtcStringResult RtcUserStopAudioCaptureCallback;

};
/** @} */ // end of RTC
/** @} */ // end of Function