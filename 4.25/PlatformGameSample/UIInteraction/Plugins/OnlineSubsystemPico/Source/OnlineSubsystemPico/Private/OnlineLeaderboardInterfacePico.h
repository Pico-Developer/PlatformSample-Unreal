// Copyright 2022 Pico Technology Co., Ltd.All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc.In the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2022, Epic Games, Inc.All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemPico.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "OnlineIdentityPico.h"
#include "OnlineSubsystemPicoPackage.h"

#define WRITE_LEADERBOARDS_PAGEINDEX FName(TEXT("WRITELEADERBOARDSPAGEINDEX"))
#define WRITE_LEADERBOARDS_PAGESIZE FName(TEXT("WRITELEADERBOARDSPAGESIZE"))

/**
*	IOnlineLeaderboard - Interface class for Leaderboard
*/
class FOnlineLeaderboardPico : public IOnlineLeaderboards
{
private:
	
	/** Reference to the owning subsystem */
	FOnlineSubsystemPico& PicoSubsystem;

	bool ReadPicoLeaderboards(bool bOnlyFriends, bool bOnlyLoggedInUser, FOnlineLeaderboardReadRef& ReadObject);
	void OnReadLeaderboardsComplete(ppfMessageHandle Message, bool bIsError, const FOnlineLeaderboardReadRef& ReadObject);

	const char* FilterTypeNames[4] = { "None", "Friends", "Unknown", "UserIds" };
	const char* StartAtNames[4] = { "Top", "CenteredOnViewer", "CenteredOnViewerOrTop", "Unknown" };

	static void SaveLog(const ELogVerbosity::Type Verbosity, const FString& Log);

public:

	/**
	* Constructor
	*
	* @param InSubsystem - A reference to the owning subsystem
	*/
	FOnlineLeaderboardPico(FOnlineSubsystemPico& InSubsystem);

	/**
	* Default destructor
	*/
	virtual ~FOnlineLeaderboardPico() = default;

	// Begin IOnlineLeaderboard interface
	/// <summary>Get entries of a leaderboard
	/// <br><b><i>Note</i></b>
	/// </summary>
	///
	/// <param name="Players">If the num of Players is bigger than zero, and the only Player in it is the logged in Player, then the value of StartAt will be ppfLeaderboard_StartAtCenteredOnViewer</param>
	/// <param name="PicoReadObject">Set leaderboard name in it</param>
	/// <returns>Bool: 
	/// <ul>
	/// <li>`true`: success</li>
	/// <li>`false`: failure</li>
	/// </ul>
	/// </returns>
	virtual bool ReadLeaderboards(const TArray< TSharedRef<const FUniqueNetId> >& Players, FOnlineLeaderboardReadRef& ReadObject) override;

	/// <summary>Get entries of a leaderboard
	/// <br><b><i>Note</i></b>
	/// </summary>
	///
	/// <param name="LocalUserNum">not used</param>
	/// <param name="PicoReadObject">Set leaderboard name in it</param>
	/// <returns>Bool: 
	/// <ul>
	/// <li>`true`: success</li>
	/// <li>`false`: failure</li>
	/// </ul>
	/// </returns>
	virtual bool ReadLeaderboardsForFriends(int32 LocalUserNum, FOnlineLeaderboardReadRef& ReadObject) override;
	
	// Not supported. Always return false.
	virtual bool ReadLeaderboardsAroundRank(int32 Rank, uint32 Range, FOnlineLeaderboardReadRef& ReadObject) override;
	
	// Not supported. Always return false.
	virtual bool ReadLeaderboardsAroundUser(TSharedRef<const FUniqueNetId> Player, uint32 Range, FOnlineLeaderboardReadRef& ReadObject) override;

	// Not supported.
	virtual void FreeStats(FOnlineLeaderboardRead& ReadObject) override;

	/// <summary>Writes a entry of a leaderboard
	/// <br><b><i>Note</i></b>
	/// </summary>
	///
	/// <param name="SessionName">not used</param>
	/// <param name="Player">Need be the logged in player</param>
	/// <param name="PicoWriteObject">Set leaderboard name and the score in it</param>
	/// <returns>Bool: 
	/// <ul>
	/// <li>`true`: success</li>
	/// <li>`false`: failure</li>
	/// </ul>
	/// </returns>
	virtual bool WriteLeaderboards(const FName& SessionName, const FUniqueNetId& Player, FOnlineLeaderboardWrite& WriteObject) override;

	/// <summary>Execute custom LeaderboardFlushCompleteDelegates with SessionName and bWasSuccessful is always true
	/// <br><b><i>Note</i></b>
	/// </summary>
	///
	/// <param name="SessionName">The param of LeaderboardFlushCompleteDelegates</param>
	/// <returns>Bool: 
	/// <ul>
	/// <li>`true`: success</li>
	/// <li>`false`: failure</li>
	/// </ul>
	/// </returns>
	virtual bool FlushLeaderboards(const FName& SessionName) override;

	// Not supported. Always return false.
	virtual bool WriteOnlinePlayerRatings(const FName& SessionName, int32 LeaderboardId, const TArray<FOnlinePlayerScore>& PlayerScores) override;
	// End IOnlineLeaderboard interface
};
