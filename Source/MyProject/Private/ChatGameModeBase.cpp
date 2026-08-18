// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatGameModeBase.h"

#include "MyGameStateBase.h"
#include "ChatPlayerController.h"
#include "EngineUtils.h"
#include "MyPlayerState.h"
void AChatGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	AChatPlayerController* ChatPlayerController = Cast<AChatPlayerController>(NewPlayer);
	if (IsValid(ChatPlayerController) == true)
	{
		ChatPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));
		AllPlayerControllers.Add(ChatPlayerController);

		static int32 PlayerNumber = 0;
		AMyPlayerState* ChatPS = ChatPlayerController->GetPlayerState<AMyPlayerState>();
		if (IsValid(ChatPS) == true)
		{
			ChatPS->PlayerNameString = TEXT("Player") + FString::FromInt(++PlayerNumber);
		}

		AMyGameStateBase* ChatGameStateBase = GetGameState<AMyGameStateBase>();
		if (IsValid(ChatGameStateBase) == true)
		{
			ChatGameStateBase->MulticastRPCBroadcastLoginMessage(ChatPS->PlayerNameString);
		}
	}
}

FString AChatGameModeBase::GenerateSecretNumber()
{
	TArray<int32> Numbers;
	for (int32 i = 1; i <= 9; ++i)
	{
		Numbers.Add(i);
	}

	FString Result;
	for (int32 i = 0; i < 3; ++i)
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}

bool AChatGameModeBase::IsGuessNumberString(const FString& InNumberString)
{
	bool bCanPlay = false;

	do {

		if (InNumberString.Len() != 3)
		{
			
			break;
		}

		bool bIsUnique = true;
		TSet<TCHAR> UniqueDigits;
		for (TCHAR C : InNumberString)
		{
			if (FChar::IsDigit(C) == false || C == '0')
			{
				bIsUnique = false;
				break;
			}

			UniqueDigits.Add(C);
		}

		if (UniqueDigits.Num() <= 2)
		{
			bIsUnique = false;
		}

		if (bIsUnique == false)
		{
			break;
		}

		bCanPlay = true;

	} while (false);

	return bCanPlay;
}

FString AChatGameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString)
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (InSecretNumberString[i] == InGuessNumberString[i])
		{
			StrikeCount++;
		}
		else
		{
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (InSecretNumberString.Contains(PlayerGuessChar))
			{
				BallCount++;
			}
		}
	}

	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void AChatGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SecretNumberString = GenerateSecretNumber();
}

void AChatGameModeBase::PrintChatMessageString(AChatPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	AMyPlayerState* ChatPS = InChattingPlayerController->GetPlayerState<AMyPlayerState>();

	if (IsValid(ChatPS) == true)
	{
		if (ChatPS->CurrentGuessCount >= ChatPS->MaxGuessCount)
		{
			InChattingPlayerController->
				ClientRPCPrintChatMessageString
				(TEXT("You cannot chat anymore."));
			return;
		}
	}

	int32 ColonIndex = InChatMessageString.Find(TEXT(":"));

	FString GuessNumberString;

	if (ColonIndex != INDEX_NONE)
	{
		GuessNumberString =
			InChatMessageString.RightChop(ColonIndex + 2);
	}
	else
	{
		GuessNumberString =
			InChatMessageString;
	}

	if (IsGuessNumberString(GuessNumberString) == true)
	{
		FString JudgeResultString =
			JudgeResult(SecretNumberString, GuessNumberString);

		IncreaseGuessCount(InChattingPlayerController);

		for (int32 i = AllPlayerControllers.Num() - 1; i >= 0; --i)
		{
			AChatPlayerController* ChatPlayerController =
				AllPlayerControllers[i].Get();

			if (IsValid(ChatPlayerController) == false)
			{
				AllPlayerControllers.RemoveAt(i);
				continue;
			}

			if (IsValid(ChatPS) == true)
			{
				FString CombinedMessageString =
					InChatMessageString +
					TEXT(" -> ") +
					JudgeResultString +
					ChatPS->GetPlayerInfoString();

				ChatPlayerController->
					ClientRPCPrintChatMessageString
					(CombinedMessageString);
			}
		}

		int32 StrikeCount =
			FCString::Atoi(*JudgeResultString.Left(1));

		JudgeGame(
			InChattingPlayerController,
			StrikeCount);
	}
	else
	{
		InChattingPlayerController->
			ClientRPCPrintChatMessageString
			(TEXT("Please enter 3 unique digits."));
	}
}


void AChatGameModeBase::IncreaseGuessCount(AChatPlayerController* InChattingPlayerController)
{
	AMyPlayerState* ChatPS = InChattingPlayerController->GetPlayerState<AMyPlayerState>();
	if (IsValid(ChatPS) == true)
	{
		ChatPS->CurrentGuessCount++;
	}
}

void AChatGameModeBase::ResetGame()
{
	SecretNumberString = GenerateSecretNumber();

	for (int32 i = AllPlayerControllers.Num() - 1; i >= 0; --i)
	{
		AChatPlayerController* CXPlayerController = AllPlayerControllers[i].Get();
		if (IsValid(CXPlayerController) == true)
		{
			AMyPlayerState* ChatPS = CXPlayerController->GetPlayerState<AMyPlayerState>();
			if (IsValid(ChatPS) == true)
			{
				ChatPS->CurrentGuessCount = 0;
			}
		}
	}
}

void AChatGameModeBase::JudgeGame(AChatPlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (3 == InStrikeCount)
	{
		AMyPlayerState* ChatPS = InChattingPlayerController->GetPlayerState<AMyPlayerState>();

		for (int32 i = AllPlayerControllers.Num() - 1; i >= 0; --i)
		{
			AChatPlayerController* CXPlayerController = AllPlayerControllers[i].Get();
			if (IsValid(CXPlayerController) == true)
			{
				if (IsValid(ChatPS) == true)
				{
					FString CombinedMessageString = ChatPS->PlayerNameString + TEXT(" has won the game.");
					CXPlayerController->NotificationText = FText::FromString(CombinedMessageString);
				}
			}
		}

		ResetGame();
	}
	else
	{
		bool bIsDraw = true;
		for (int32 i = AllPlayerControllers.Num() - 1; i >= 0; --i)
		{
			AChatPlayerController* ChatPlayerController = AllPlayerControllers[i].Get();
			if (IsValid(ChatPlayerController) == true)
			{
				AMyPlayerState* ChatPS = ChatPlayerController->GetPlayerState<AMyPlayerState>();
				if (IsValid(ChatPS) == true)
				{
					if (ChatPS->CurrentGuessCount < ChatPS->MaxGuessCount)
					{
						bIsDraw = false;
						break;
					}
				}
			}
		}

		if (true == bIsDraw)
		{
			for (int32 i = AllPlayerControllers.Num() - 1; i >= 0; --i)
			{
				AChatPlayerController* ChatPlayerController = AllPlayerControllers[i].Get();
				if (IsValid(ChatPlayerController) == true)
				{
					ChatPlayerController->NotificationText = FText::FromString(TEXT("Draw..."));
				}
			}

			ResetGame();
		}
	}
}
