// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActor.h"
#include "Youre.h"
#include "Engine/World.h"

// Sets default values
AMyActor::AMyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
	Super::BeginPlay();
	

	if (FYoureModule::IsAvailable())
	{
		FYoureModule& youre = FYoureModule::Get();
		youre.AuthSucceeded.Clear();
		youre.AuthSucceeded.AddLambda([this](YoureUserInfo user) {
			UE_LOG(LogTemp, Warning, TEXT("Success! YOURE ID: %s"), *user.userId);
			});

		youre.AuthFailed.Clear();
		youre.AuthFailed.AddLambda([this](std::string error) {
			UE_LOG(LogTemp, Warning, TEXT("Failed! Error: %s"), *FString(error.c_str()));
			});

		youre.Initialize("{ENTER YOUR CLIENT ID}", "sso.prepro.youre.id");
		youre.Authenticate(GetWorld());
	}

}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

