// Copyright Epic Games, Inc. All Rights Reserved.

#include "OnlineMessageMultiTaskPico.h"
#include "OnlineSubsystemPicoPrivate.h"


void FOnlineMessageMultiTaskPico::AddNewRequest(ppfRequest RequestId)
{
	InProgressRequests.Add(RequestId);
	PicoSubsystem.AddAsyncTask(
		RequestId,
		FPicoMessageOnCompleteDelegate::CreateLambda([this, RequestId](ppfMessageHandle Message, bool bIsError)
	{
		InProgressRequests.Remove(RequestId);
		if (bIsError)
		{
			bDidAllRequestsFinishedSuccessfully = false;
		}

		if (InProgressRequests.Num() == 0)
		{
			Delegate.ExecuteIfBound();
		}
	}));
}
