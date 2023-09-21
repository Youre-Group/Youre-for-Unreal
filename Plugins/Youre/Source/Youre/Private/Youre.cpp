// Copyright Epic Games, Inc. All Rights Reserved.

#include "Youre.h"
#include "Modules/ModuleManager.h"
#include "YoureAuth.h"
#include "Blueprint/WidgetTree.h"
#include "WebViewWidget.h"
#include <GenericPlatform/GenericPlatformHttp.h>



#define LOCTEXT_NAMESPACE "FYoureModule"

DEFINE_LOG_CATEGORY(LogYoure);


void FYoureModule::Initialize(const FString clientId, const FString apiEndpointUrl, const FString t_redirectUrl)
{
	m_clientId = clientId;
	m_apiEndpointUrl = apiEndpointUrl;
	m_redirectUrl = FGenericPlatformHttp::UrlEncode(t_redirectUrl);
	UE_LOG(LogYoure, Log, TEXT("Youre module Initialized"));
	m_isInitialized = true;
}


void FYoureModule::ClearSession()
{
	YoureAuth* auth = new YoureAuth(m_clientId, m_apiEndpointUrl, m_redirectUrl);
	auth->clearTokenCache();
	delete auth;
}


void FYoureModule::Authenticate(UWorld* world, bool wasRetry)
{
	if (m_isInitialized) {

		YoureAuth* auth = new YoureAuth(m_clientId, m_apiEndpointUrl, m_redirectUrl);
	
		auto onValidAccessToken = [this, auth, world, wasRetry]() {
			auth->requestUserInfo([this](YoureUserInfo& user) {
				AuthSucceeded.Broadcast(user);
				}, [this, auth, world, wasRetry]() {
					auth->clearTokenCache();
					if (wasRetry) {
						AuthFailed.Broadcast("AuthError. Something went wrong.");
					}
					else {
						Authenticate(world, true);
					}
					});
					
			};

		auto onAuthError = [this](FString error) {
			AuthFailed.Broadcast(error);
			};

		if (!auth->isAuthenticated()) {

			FString WidgetClassName = TEXT("/Youre/WebView/WebView.WebView_C"); 
			TSubclassOf<UUserWidget> WidgetClass = LoadClass<UUserWidget>(nullptr, *WidgetClassName);
			UWebViewWidget* Widget = CreateWidget<UWebViewWidget>(world->GetGameInstance(), WidgetClass);
			Widget->AddToViewport();

			FVector2D Alignment;
			Alignment.X = 0.5;
			Alignment.Y = 0.5;
			Widget->SetAlignmentInViewport(Alignment);
		
			Widget->OnUrlChanged.AddLambda([auth, onValidAccessToken, onAuthError, Widget](FString str) {
				if (str.Contains("?code=")) {
					TArray<FString> resultArray;
					str.ParseIntoArray(resultArray, TEXT("?code="), true);
					if (resultArray.IsValidIndex(1)) {
						Widget->RemoveFromParent();
						auth->requestAccessToken(*resultArray[1], onValidAccessToken, onAuthError);
					}
				}
				});

			FString loginUrl = auth->getLoginUrl();
			Widget->WebBrowser->LoadURL(loginUrl);
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