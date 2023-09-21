// Copyright Youre Games, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Youre.h"
#include <functional>

#ifdef _MSC_VER
// Windows
typedef unsigned __int8  uint8_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
// GCC
#include <stdint.h>
#endif

class YOURE_API YoureAuth
{

public:
	YoureAuth(FString clientId, FString apiEndpointUrl, FString redirectUrl);

	~YoureAuth();
	void requestUserInfo(const std::function<void(YoureUserInfo&)>& callback, const std::function<void()>& errorCallback);

	bool isAuthenticated();
	void authenticate();
	void clearTokenCache();

	void requestAccessToken(FString code, const std::function<void()>& callback, const std::function<void(FString)>& errorCallback);
	FString getLoginUrl();
	

private:


	FString m_accessToken;
	FString m_redirectUrl;
	FString m_apiEndpointUrl;
	FString m_clientId;
	FString m_lastGeneratedCodeVerifier;

	void writeTokenCache(FString accessToken);
	bool readTokenCacheChecked();

};

