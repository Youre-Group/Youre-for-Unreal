// Copyright YOURE Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
DECLARE_LOG_CATEGORY_EXTERN(LogYoure, Log, All);


class YOURE_API YoureUserInfo {
public:
	FString userId;
	FString userName;
	FString email;
	FString accessToken;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FYoureAuthSuccessDelegate, YoureUserInfo&);
DECLARE_MULTICAST_DELEGATE_OneParam(FYoureAuthErrorDelegate, FString);





class YOURE_API FYoureModule : public IModuleInterface
{

private:
	FString m_clientId;
	FString m_apiEndpointUrl;
	FString m_redirectUrl;
	bool m_isInitialized;

public:
	static inline FYoureModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FYoureModule>("Youre");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("Youre");
	}

	FYoureAuthErrorDelegate AuthFailed;
	FYoureAuthSuccessDelegate AuthSucceeded;



	void Initialize(const FString t_clientId, const FString t_apiEndpointUrl, const FString t_redirectUrl);
	void ClearSession();
	void Authenticate(UWorld* world, bool wasRetry = false);


	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
