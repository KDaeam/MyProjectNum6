// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatPlayerController.h"
#include "ChatUserWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MyProject/MyProject.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "ChatGameModeBase.h"
#include "MyPlayerState.h"
#include "Net/UnrealNetwork.h"
AChatPlayerController::AChatPlayerController()
{
	bReplicates = true;
}
void AChatPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return; //일종의 방어코드
	}

	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);
	

	if (IsValid(ChatInputWidgetClass) == true)
	{
		ChatInputWidgetInstance = CreateWidget<UChatUserWidget>(this, ChatInputWidgetClass);
		if (IsValid(ChatInputWidgetInstance) == true)
		{
			ChatInputWidgetInstance->AddToViewport();
		}
	}
	if (IsValid(NotificationTextWidgetClass) == true)
	{
		NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
		if (IsValid(NotificationTextWidgetInstance) == true)
		{
			NotificationTextWidgetInstance->AddToViewport();
		}
	}
}

void AChatPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	ChatMessageString = InChatMessageString;

	if (IsLocalController() == true)
	{
		// ServerRPCPrintChatMessageString(InChatMessageString);

		AMyPlayerState* ChatPS = GetPlayerState<AMyPlayerState>();
		if (IsValid(ChatPS) == true)
		{
			FString CombinedMessageString = ChatPS->PlayerNameString + TEXT(": ") + InChatMessageString;

			ServerRPCPrintChatMessageString(CombinedMessageString);
		}
	}
}

void AChatPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	MyFunctionLibrary::MyPrintString(this, InChatMessageString, 10.f);
}

void AChatPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);
}

void AChatPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM) == true)
	{
		AChatGameModeBase* ChatGM = Cast<AChatGameModeBase>(GM);
		if (IsValid(ChatGM) == true)
		{
			ChatGM->PrintChatMessageString(this, InChatMessageString);
		}
	}
}

void AChatPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NotificationText);
}