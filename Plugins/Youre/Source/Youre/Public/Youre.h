// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include <string>

DECLARE_LOG_CATEGORY_EXTERN(LogYoure, Log, All);


class YOURE_API YoureUserInfo {
public:
	FString userId;
	FString userName;
	FString email;
};

//DECLARE_MULTICAST_DELEGATE_OneParam(FYoureAuthErrorDelegate, FString);
DECLARE_MULTICAST_DELEGATE_OneParam(FYoureAuthSuccessDelegate, YoureUserInfo&);
DECLARE_MULTICAST_DELEGATE_OneParam(FYoureAuthErrorDelegate, std::string);




class YOURE_API FYoureModule : public IModuleInterface
{

private:
	std::string m_clientId;
	std::string m_apiEndpointUrl = "sso.prepro.youre.id";
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

	void Initialize(const std::string t_clientId, const std::string t_apiEndpointUrl = "");
	void ClearSession();
	void Authenticate(UWorld* world, bool wasRetry = false);

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
};
