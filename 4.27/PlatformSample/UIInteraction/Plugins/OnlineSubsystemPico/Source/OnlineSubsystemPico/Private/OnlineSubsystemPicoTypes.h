// Copyright 2022 Pico Technology Co., Ltd.All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc.In the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2022, Epic Games, Inc.All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemPicoPackage.h"
#include "PPF_Platform.h"
#include "OnlineSubsystemPicoNames.h"

using FUniqueNetIdPicoPtr = TSharedPtr<const class FUniqueNetIdPico, UNIQUENETID_ESPMODE>;
using FUniqueNetIdPicoRef = TSharedRef<const class FUniqueNetIdPico, UNIQUENETID_ESPMODE>;




class FUniqueNetIdPico : public FUniqueNetId
{
private:
    // Pass by string
    FString StrID;
    ppfID ID;
    bool bInitWithString;

protected:
    bool Compare(const FUniqueNetId& Other) const override
    {
        if (Other.GetType() != GetType())
        {
            return false;
        }
        return StrID == static_cast<const FUniqueNetIdPico&>(Other).StrID;
    }

public:
    template<typename... TArgs>
    static FUniqueNetIdPicoRef Create(TArgs&&... Args)
    {
        return MakeShared<FUniqueNetIdPico, UNIQUENETID_ESPMODE>(Forward<TArgs>(Args)...);
    }

    /** Allow MakeShared to see private constructors */
    friend class SharedPointerInternals::TIntrusiveReferenceController<FUniqueNetIdPico>;

    static const FUniqueNetIdPico& Cast(const FUniqueNetId& NetId)
    {
        check(NetId.GetType() == PICO_SUBSYSTEM);
        return *static_cast<const FUniqueNetIdPico*>(&NetId);
    }

    FUniqueNetIdPicoRef AsShared() const
    {
        return StaticCastSharedRef<const FUniqueNetIdPico>(FUniqueNetId::AsShared());
    }

    virtual FName GetType() const override
    {
        return PICO_SUBSYSTEM;
    }

    // IOnlinePlatformData

    virtual const uint8* GetBytes() const override
    {
        return reinterpret_cast<const uint8*>(&ID);
    }

    virtual int32 GetSize() const override
    {
        return sizeof(ID);
    }

    virtual bool IsValid() const override
    {
        return !GetStringID().IsEmpty();
    }

    ppfID GetID() const
    {
        if (bInitWithString)
        {
            // Conversion may failed
            return FCString::Strtoui64(*StrID, NULL, 10);
        }
        else
        {
            return ID;
        }

    }
    
    FString GetStringID() const
    {
        if (bInitWithString)
        {
            return StrID;
        }
        else
        {
            // Conversion may failed
            return FString::Printf(TEXT("%llu"), ID);
        }

    }

    virtual FString ToString() const override
    {
        return GetStringID();
    }

    virtual FString ToDebugString() const override
    {
        const FString UniqueNetIdStr = FString::Printf(TEXT("%llu"), ID);
        return TEXT("ppfID:") + OSS_UNIQUEID_REDACT(*this, UniqueNetIdStr);
    }

    /** Needed for TMap::GetTypeHash() */
    friend uint32 GetTypeHash(const FUniqueNetIdPico& A)
    {
        return GetTypeHash((uint64)A.ID);
    }

    /** global static instance of invalid (zero) id */
    static const FUniqueNetIdPicoRef& EmptyId()
    {
        static const FUniqueNetIdPicoRef EmptyId(Create());
        return EmptyId;
    }

    FUniqueNetIdPico()
    {
        ID = 0;
        StrID = FString();
    }
    FUniqueNetIdPico(const ppfID& id)
    {
        ID = id;
        bInitWithString = false;
       // StrID = FString::Printf(TEXT("%llu"), ID);
    }

    FUniqueNetIdPico(const FString& id)
    {
        StrID = id;
        bInitWithString = true;
      //  ID = FCString::Strtoui64(*StrID, NULL, 10);
    }
    /**
    * Copy Constructor
    *
    * @param Src the id to copy
    */
    explicit FUniqueNetIdPico(const FUniqueNetId& Src)
    {
        if (Src.GetType() == PICO_SUBSYSTEM)
        {
            ID = FUniqueNetIdPico::Cast(Src).ID;
        }
    }
};

/**
* Implementation of session information
*/
class FOnlineSessionInfoPico : public FOnlineSessionInfo
{
protected:

    /** Hidden on purpose */
    FOnlineSessionInfoPico(const FOnlineSessionInfoPico& Src) = delete;
    FOnlineSessionInfoPico& operator=(const FOnlineSessionInfoPico& Src) = delete;

PACKAGE_SCOPE:

    FOnlineSessionInfoPico(ppfID RoomId);

    /** Unique Id for this session */
    FUniqueNetIdPicoRef SessionId;

public:

    virtual ~FOnlineSessionInfoPico() {}

    bool operator==(const FOnlineSessionInfoPico& Other) const
    {
        return *Other.SessionId == *SessionId;
    }

    virtual const uint8* GetBytes() const override
    {
        return nullptr;
    }

    virtual int32 GetSize() const override
    {
        return 0;
    }

    virtual bool IsValid() const override
    {
        return true;
    }

    virtual FString ToString() const override
    {
        return SessionId->ToString();
    }

    virtual FString ToDebugString() const override
    {
        return FString::Printf(TEXT("SessionId: %s"), *SessionId->ToDebugString());
    }

    virtual const FUniqueNetId& GetSessionId() const override
    {
        return *SessionId;
    }
};


/**
 *	Interface for reading data from a leaderboard service
 */
class FOnlineLeaderboardReadPico : FOnlineLeaderboardRead
{
public:
    int PageSize = 0;
    int PageIndex = 0;
};

typedef TSharedRef<FOnlineLeaderboardReadPico, ESPMode::ThreadSafe> FOnlineLeaderboardReadPicoRef;
typedef TSharedPtr<FOnlineLeaderboardReadPico, ESPMode::ThreadSafe> FOnlineLeaderboardReadPicoPtr;