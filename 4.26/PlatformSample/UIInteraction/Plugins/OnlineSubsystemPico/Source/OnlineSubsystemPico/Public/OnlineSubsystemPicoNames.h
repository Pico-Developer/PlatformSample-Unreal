// Copyright 2022 Pico Technology Co., Ltd.All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc.In the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2022, Epic Games, Inc.All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineStats.h"
#include "OnlineSubsystemPicoNames.generated.h"


#ifndef PICO_SUBSYSTEM
#define PICO_SUBSYSTEM FName(TEXT("Pico"))
#endif
class UPico_UserArray;

UENUM(BlueprintType)
enum class EUserPresenceStatus : uint8
{
    Unknow,
    OnLine,
    OffLine
};


UENUM(BlueprintType)
enum class EUserGender : uint8
{
    Unknow,
    Male,
    Female
};

USTRUCT(BlueprintType, meta = (DisplayName = "OnlinePicoUserInfo"))
struct FPicoUserInfo
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "OnlinePicoUserInfo")
    FString UserId;

    UPROPERTY(BlueprintReadWrite, Category = "OnlinePicoUserInfo")
    FString DisplayName;

    UPROPERTY(BlueprintReadWrite, Category = "OnlinePicoUserInfo")
    EUserPresenceStatus UserPresenceStatus;

    UPROPERTY(BlueprintReadWrite, Category = "OnlinePicoUserInfo")
    FString InviteToken;

    UPROPERTY(BlueprintReadWrite, Category = "OnlinePicoUserInfo")
    FString ImageUrl;

    UPROPERTY(BlueprintReadWrite, Category = "OnlinePicoUserInfo")
    FString SmallImageUrl;

    UPROPERTY(BlueprintReadWrite, Category = "OnlinePicoUserInfo")
    FString PresencePackage;

    UPROPERTY(BlueprintReadWrite, Category = "OnlinePicoUserInfo")
    FString PresenceStr;

    UPROPERTY(BlueprintReadWrite, Category = "OnlinePicoUserInfo")
    FString PresenceDeeplinkMessage;

    UPROPERTY(BlueprintReadWrite, Category = "OnlinePicoUserInfo")
    FString PresenceDestinationApiName;

    UPROPERTY(BlueprintReadWrite, Category = "OnlinePicoUserInfo")
    FString PresenceLobbySessionId;

    UPROPERTY(BlueprintReadWrite, Category = "OnlinePicoUserInfo")
    FString PresenceMatchSessionId;

    UPROPERTY(BlueprintReadWrite, Category = "OnlinePicoUserInfo")
    FString PresenceExtra;

    UPROPERTY(BlueprintReadWrite, Category = "OnlinePicoUserInfo")
    EUserGender Gender;
};

/* ppf_LaunchType.h */
UENUM(BlueprintType, meta = (DisplayName = "LaunchType"))
enum class ELaunchType : uint8
{
    Unknown,
    Normal,
    Invite,
    Coordinated,
    Deeplink
};

/* ppf_LaunchResult.h */
UENUM(BlueprintType, meta = (DisplayName = "LaunchResult"))
enum class ELaunchResult : uint8
{
    Unknown,
    Success,
    FailedRoomFull,
    FailedGameAlreadyStarted,
    FailedRoomNotFound,
    FailedUserDeclined,
    FailedOtherReason
};

/* ppf_LaunchDetails.h */
USTRUCT(BlueprintType, meta = (DisplayName = "LaunchDetails"))
struct FLaunchDetails
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "ApplicationLifecycle")
    FString DeeplinkMessage;

    UPROPERTY(BlueprintReadWrite, Category = "ApplicationLifecycle")
    FString DestinationApiName;

    UPROPERTY(BlueprintReadWrite, Category = "ApplicationLifecycle")
    FString LaunchSource;

    UPROPERTY(BlueprintReadWrite, Category = "ApplicationLifecycle")
    FString LobbySessionID;

    UPROPERTY(BlueprintReadWrite, Category = "ApplicationLifecycle")
    FString MatchSessionID;

    UPROPERTY(BlueprintReadWrite, Category = "ApplicationLifecycle")
    FString Extra;

    UPROPERTY(BlueprintReadWrite, Category = "ApplicationLifecycle")
    FString RoomID;

    UPROPERTY(BlueprintReadWrite, Category = "ApplicationLifecycle")
    FString TrackingID;

    UPROPERTY(BlueprintReadWrite, Category = "ApplicationLifecycle")
    ELaunchType LaunchType;

    UPROPERTY(BlueprintReadWrite, Category = "ApplicationLifecycle")
    UPico_UserArray* UserArray = nullptr;

};

UENUM(BlueprintType, meta = (DisplayName = "RoomType"))
enum class ERoomType : uint8
{
    TypeUnknown,
    TypeMatchmaking,
    TypeModerated,
    TypePrivate
};

UENUM(BlueprintType, meta = (DisplayName = "RoomJoinPolicy"))
enum class ERoomJoinPolicy : uint8
{
    JoinPolicyNone,
    JoinPolicyEveryone,
    JoinPolicyFriendsOfMembers,
    JoinPolicyFriendsOfOwner,
    JoinPolicyInvitedUsers,
    JoinPolicyUnknown
};

UENUM(BlueprintType, meta = (DisplayName = "RoomJoinabilit"))
enum class ERoomJoinabilit : uint8
{
    JoinabilityUnknown,
    JoinabilityAreIn,
    JoinabilityAreKicked,
    JoinabilityCanJoin,
    JoinabilityIsFull,
    JoinabilityNoViewer,
    JoinabilityPolicyPrevents
};

USTRUCT(BlueprintType, meta = (DisplayName = "PicoDestination"))
struct FPicoDestination
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Destination")
    FString ApiName;

    UPROPERTY(BlueprintReadWrite, Category = "Destination")
    FString DeepLinkMessage;

    UPROPERTY(BlueprintReadWrite, Category = "Destination")
    FString DisplayName;
};


USTRUCT(BlueprintType, meta = (DisplayName = "PicoApplicationInvite"))
struct FPicoApplicationInvite
{
    GENERATED_USTRUCT_BODY()
    //	FPicoRecipent Recipent;
    UPROPERTY(BlueprintReadWrite, Category = "Destination")
    FPicoUserInfo Recipent;
//    TSharedRef<FOnlinePicoFriend> Recipent;
    UPROPERTY(BlueprintReadWrite, Category = "Destination")
    FPicoDestination Destination;

    UPROPERTY(BlueprintReadWrite, Category = "Destination")
    FString ID;

//    ppfID Id;

    UPROPERTY(BlueprintReadWrite, Category = "Destination")
    bool bIsActive;

    UPROPERTY(BlueprintReadWrite, Category = "Destination")
    FString LobbySessionId;

    UPROPERTY(BlueprintReadWrite, Category = "Destination")
    FString MatchSessionId;

//    FPicoApplicationInvite(TSharedRef<FOnlinePicoFriend> InRecipent) : Recipent(InRecipent) {}
};

UENUM(BlueprintType)
enum class EOnlineAsyncTaskStatePicoType : uint8
{
    NotStarted,
    InProgress,
    Done,
    Failed
};
UENUM(BlueprintType)
enum class EOnJoinSessionCompleteResultPicoType : uint8
{
    Success,
    SessionIsFull,
    SessionDoesNotExist,
    CouldNotRetrieveAddress,
    AlreadyInSession,
    UnknownError
};
UENUM(BlueprintType)
enum class EOnlineSessionStatePicoType : uint8
{
    NoSession,
    Creating,
    Pending,
    Starting,
    InProgress,
    Ending,
    Ended,
    Destroying
};

USTRUCT(BlueprintType, meta = (DisplayName = "PicoOnlineSessionSettings"))
struct FPicoOnlineSessionSettings
{
    GENERATED_USTRUCT_BODY()

        UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSettings")
        int NumPublicConnections;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSettings")
        int NumPrivateConnections;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSettings")
        bool bShouldAdvertise;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSettings")
        bool bAllowInvites;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSettings")
        bool bUsesPresence;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSettings")
        bool bAllowJoinViaPresence;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSettings")
        bool bAllowJoinViaPresenceFriendsOnly;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSettings")
        int BuildUniqueId;

    // Settings
    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSettings")
        TArray<FString> KeyArray;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSettings")
        TArray<FString> ValueArray;
};
USTRUCT(BlueprintType, meta = (DisplayName = "PicoOnlineSession"))
struct FPicoOnlineSession
{
    GENERATED_USTRUCT_BODY()

        UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSession")
        FString OwningUserId;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSession")
        FString OwningUserName;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSession")
        FPicoOnlineSessionSettings SessionSettings;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSession")
        FString SessionInfoRoomID;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSession")
        int32 NumOpenPrivateConnections;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSession")
        int32 NumOpenPublicConnections;
};
USTRUCT(BlueprintType, meta = (DisplayName = "PicoNamedOnlineSession"))
struct FPicoNamedOnlineSession
{
    GENERATED_USTRUCT_BODY()

        UPROPERTY(BlueprintReadWrite, Category = "PicoNamedOnlineSession")
        FString SessionName;

    UPROPERTY(BlueprintReadWrite, Category = "PicoNamedOnlineSession")
        int32 HostingPlayerNum;

    UPROPERTY(BlueprintReadWrite, Category = "PicoNamedOnlineSession")
        bool bHosting;

    UPROPERTY(BlueprintReadWrite, Category = "PicoNamedOnlineSession")
        FString LocalOwnerId;

    UPROPERTY(BlueprintReadWrite, Category = "PicoNamedOnlineSession")
        TArray< FString > RegisteredPlayers;

    UPROPERTY(BlueprintReadWrite, Category = "PicoNamedOnlineSession")
        EOnlineSessionStatePicoType SessionState;

    UPROPERTY(BlueprintReadWrite, Category = "PicoNamedOnlineSession")
        FString OwningUserId;

    UPROPERTY(BlueprintReadWrite, Category = "PicoNamedOnlineSession")
        FString OwningUserName;

    UPROPERTY(BlueprintReadWrite, Category = "PicoNamedOnlineSession")
        FPicoOnlineSessionSettings SessionSettings;

    UPROPERTY(BlueprintReadWrite, Category = "PicoNamedOnlineSession")
        FString SessionInfoRoomID;

    UPROPERTY(BlueprintReadWrite, Category = "PicoNamedOnlineSession")
        int32 NumOpenPrivateConnections;

    UPROPERTY(BlueprintReadWrite, Category = "PicoNamedOnlineSession")
        int32 NumOpenPublicConnections;
};
USTRUCT(BlueprintType, meta = (DisplayName = "PicoOnlineSessionSearchResult"))
struct FPicoOnlineSessionSearchResult
{
    GENERATED_USTRUCT_BODY()

        UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSearchResult")
        FPicoOnlineSession Session;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSearchResult")
        int32 PingInMs;
};
USTRUCT(BlueprintType, meta = (DisplayName = "PicoOnlineSessionSearch"))
struct FPicoOnlineSessionSearch
{
    GENERATED_USTRUCT_BODY()

        UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSearch")
        TArray<FPicoOnlineSessionSearchResult> SearchResults;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSearch")
        EOnlineAsyncTaskStatePicoType SearchState;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSearch")
        int32 MaxSearchResults;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSearch")
        TArray<FString> QuerySettingsKeyArray;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSearch")
        TArray<FString> QuerySettingsValueArray;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSearch")
        bool bIsLanQuery;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSearch")
        int32 PingBucketSize;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSearch")
        int32 PlatformHash;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineSessionSearch")
        float TimeoutInSeconds;
};
// Leaderboard
UENUM(BlueprintType)
enum class EPicoOnlineKeyValuePairDataType : uint8
{
    Empty,
    Int32,
    UInt32,
    Int64,
    UInt64,
    Double,
    String,
    Float,
    Blob,
    Bool,
    Json,
    MAX
};
USTRUCT(BlueprintType, meta = (DisplayName = "FVariantDataPico"))
struct  FVariantDataPico
{
    GENERATED_USTRUCT_BODY()
    
    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineStatsRow")
    EPicoOnlineKeyValuePairDataType Type;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineStatsRow")
    FString Value;
};
USTRUCT(BlueprintType, meta = (DisplayName = "FPicoColumnMetaData"))
struct FPicoColumnMetaData
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "FPicoColumnMetaData")
    FString ColumnName;

    UPROPERTY(BlueprintReadWrite, Category = "FPicoColumnMetaData")
    EPicoOnlineKeyValuePairDataType DataType;
};
USTRUCT(BlueprintType, meta = (DisplayName = "FPicoOnlineStatsRow"))
struct FPicoOnlineStatsRow
{
    GENERATED_USTRUCT_BODY()
    
    UPROPERTY(BlueprintReadWrite, Category = "FPicoOnlineStatsRow")
    FString NickName;

    UPROPERTY(BlueprintReadWrite, Category = "FPicoOnlineStatsRow")
    FString PlayerId;

    UPROPERTY(BlueprintReadWrite, Category = "FPicoOnlineStatsRow")
    int32 Rank;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineStatsRow")
    TMap<FName, FVariantDataPico> Columns;
};
USTRUCT(BlueprintType, meta = (DisplayName = "PicoOnlineLeaderboardRead"))
struct FPicoOnlineLeaderboardRead
{
    GENERATED_USTRUCT_BODY()
    
    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineLeaderboardRead")
    FString LeaderboardName;
    
    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineLeaderboardRead")
    FString SortedColumn;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineLeaderboardRead")
    TArray<FPicoOnlineStatsRow> Rows;
    
    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineLeaderboardRead")
    EOnlineAsyncTaskStatePicoType ReadState;
    
    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineLeaderboardRead")
    TArray<FPicoColumnMetaData> ColumnMetadata;
    
    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineLeaderboardRead")
    int PageIndex;
    
    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineLeaderboardRead")
    int PageSize;

};
UENUM(BlueprintType)
enum class ELeaderboardUpdateMethodPicoType : uint8
{
    KeepBest,
    Force
};
USTRUCT(BlueprintType, meta = (DisplayName = "PicoOnlineLeaderboardWrite"))
struct FPicoOnlineLeaderboardWrite
{
    GENERATED_USTRUCT_BODY()
    
    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineLeaderboardWrite")
    ELeaderboardUpdateMethodPicoType UpdateMethod;
    
    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineLeaderboardWrite")
    TArray<FString> LeaderboardNames;
    
    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineLeaderboardWrite")
    FString RatedStat;

    UPROPERTY(BlueprintReadWrite, Category = "PicoOnlineLeaderboardWrite")
    int32 ValueToWrite;
};

UENUM(BlueprintType)
enum class ESportTarget : uint8
{
    None,
    LoseFat,
    KeepFit
};

UENUM(BlueprintType)
enum class EAchievementType : uint8
{
    Unknown,
    Simple,
    Count,
    Bitfield,
};
UENUM(BlueprintType)
enum class EAchievementWritePolicy : uint8
{
    Unknown,
    Client,
    Server,
};


UENUM(BlueprintType)
enum class EAssetFileDownloadCompleteStatus : uint8
{
    Downloading,
    Succeed,
    Failed,
    Unkonw
};

inline uint64 FStringTouint64(FString InStr)
{
    return FCString::Strtoui64(*InStr, NULL, 10);
};

inline FString uint64ToFString(uint64 Inuint64)
{
    return FString::Printf(TEXT("%llu"), Inuint64);
};