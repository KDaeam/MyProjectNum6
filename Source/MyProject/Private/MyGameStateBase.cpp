// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameStateBase.h"

#include "Kismet/GameplayStatics.h"
#include "ChatPlayerController.h"

void AMyGameStateBase::MulticastRPCBroadcastLoginMessage_Implementation(const FString& InNameString)
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PC) == true)
		{
			AChatPlayerController* ChatPC = Cast<AChatPlayerController>(PC);
			if (IsValid(ChatPC) == true)
			{
				FString NotificationString = InNameString + TEXT(" has joined the game.");
				ChatPC->PrintChatMessageString(NotificationString);
			}
		}
	}
}