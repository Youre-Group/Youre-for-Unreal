// Copyright Youre Games, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include <string>
#include "Youre.h"


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
	YoureAuth(std::string apiEndpointUrl, std::string clientId);

	~YoureAuth();

	
	void requestAccessToken(FString code, const std::function<void()>& callback, const std::function<void(std::string)>& errorCallback);
	void requestUserInfo(const std::function<void(YoureUserInfo&)>& callback, const std::function<void()>& errorCallback);
	bool isAuthenticated();
	void authenticate();
	FString getLoginUrl();
	void clearTokenCache();

private:


	FString m_idToken;
	FString m_accessToken;
	FString m_refreshToken;

	FString m_apiEndpointUrl;
	FString m_clientId;
	FString m_lastGeneratedCodeVerifier;

	void requestTokenRefresh(const std::function<void()>& callback, const std::function<void()>& errorCallback);
	void writeTokenCache(FString idToken, FString accessToken, FString refreshToken);
	bool readTokenCacheChecked();

};

