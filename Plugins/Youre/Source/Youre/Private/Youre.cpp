// Copyright Youre Games, All Rights Reserved.

#include "Youre.h"
#include "Modules/ModuleManager.h"
#include "YoureAuth.h"

#include <UMG/Public/Blueprint/WidgetTree.h>
#include <UMG/Public/Components/TextBlock.h>
#include <WebBrowser.h>
#include <GameFramework/Actor.h>
#include <vector>
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "WebViewUserWidget.h"

#define LOCTEXT_NAMESPACE "FYoureModule"

DEFINE_LOG_CATEGORY(LogYoure);


void FYoureModule::Initialize(const std::string clientId, const std::string apiEndpointUrl)
{
	m_clientId = clientId;
	if (!apiEndpointUrl.empty()) {
		m_apiEndpointUrl = apiEndpointUrl;
	}
	FString lclid(clientId.c_str());
	m_isInitialized = true;
}

void FYoureModule::ClearSession()
{
	YoureAuth* auth = new YoureAuth(m_apiEndpointUrl, m_clientId);
	auth->clearTokenCache();
	delete auth;
}


void FYoureModule::Authenticate(UWorld* world, bool wasRetry)
{
	if (m_isInitialized) {
		
		YoureAuth* auth = new YoureAuth(m_apiEndpointUrl, m_clientId);
		auto onValidAccessToken = [this, auth, world, wasRetry]() {
			auth->requestUserInfo([this](YoureUserInfo& user) {
				AuthSucceeded.Broadcast(user);
				},[this,auth, world, wasRetry]() {
					auth->clearTokenCache(); 
					if (wasRetry) {
						AuthFailed.Broadcast("AuthError. Something went wrong.");
					}
					else {
						Authenticate(world, true);
					}
				});
		};

		auto onAuthError = [this](std::string error) {
			AuthFailed.Broadcast(error);
		};

		if (!auth->isAuthenticated()) {

			FSoftClassPath WidgetClassRef(TEXT("/Script/UMGEditor.WidgetBlueprint'/Youre/WebViewWidgetBlueprint.WebViewWidgetBlueprint_C'"));
			if (UClass* WidgetClass = WidgetClassRef.TryLoadClass<UWebViewUserWidget>())
			{
				UWebViewUserWidget* Widget = CreateWidget<UWebViewUserWidget>(world->GetGameInstance(), WidgetClass);
				Widget->AddToViewport();
				
				FVector2D Alignment;
				Alignment.X = 0.5;
				Alignment.Y = 0.5;
				Widget->SetAlignmentInViewport(Alignment);
				/*FVector2D Position;
				Position.X =-2000;
				Position.Y = 0;
				Widget->SetPositionInViewport(Position);
				*/
				Widget->OnUrlChanged.AddLambda([auth, onValidAccessToken, onAuthError, Widget](FString str) {
					if (str.StartsWith("unity:oauth")) {
						TArray<FString> resultArray;
						str.ParseIntoArray(resultArray, TEXT("code="), true);
						if (resultArray.IsValidIndex(1)) {
							Widget->RemoveFromParent();
							auth->requestAccessToken(*resultArray[1], onValidAccessToken, onAuthError);
						}
					}
				});
				
				FString loginUrl = auth->getLoginUrl();
				Widget->WebBrowser->LoadURL(loginUrl);
			}
		}
		else {
			onValidAccessToken();
		}
		
	}
	else {
		UE_LOG(LogYoure, Error, TEXT("Youre is not initialized. Call Initialize before using any other service."));
	}
}

void FYoureModule::StartupModule()
{
	UE_LOG(LogYoure, Log, TEXT("Youre module started"));
}

void FYoureModule::ShutdownModule()
{
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FYoureModule, Youre)

