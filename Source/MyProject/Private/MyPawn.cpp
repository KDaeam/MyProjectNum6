// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPawn.h"
#include "MyProject/MyProject.h"

void AMyPawn::BeginPlay()
{
	Super::BeginPlay();

	FString NetRoleString = MyFunctionLibrary::GetRoleString(this);
	FString CombinedString = 
		FString::Printf(TEXT("CXPawn::BeginPlay() %s [%s]"),
		*MyFunctionLibrary::GetNetModeString(this), *NetRoleString);
	MyFunctionLibrary::MyPrintString(this, CombinedString, 10.f);
}

void AMyPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	FString NetRoleString = MyFunctionLibrary::GetRoleString(this);
	FString CombinedString = 
		FString::Printf(TEXT("CXPawn::PossessedBy() %s [%s]"),
			*MyFunctionLibrary::GetNetModeString(this), *NetRoleString);
	MyFunctionLibrary::MyPrintString(this, CombinedString, 10.f);
}