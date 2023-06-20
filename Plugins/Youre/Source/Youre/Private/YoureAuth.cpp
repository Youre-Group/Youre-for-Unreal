#include "YoureAuth.h"
#ifndef _MSC_VER
#include <endian.h>
#endif
#include <string>
#include "PKCEHelper.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex>
#include <bitset>
#include <array>
#include "Misc/FileHelper.h"
#include "Json.h"
#include "Runtime/Online/HTTP/Public/Http.h"

static const char* const HEX_DIGITS = "0123456789abcdef";
static const char* const BASE64_DIGITS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char BASE64_PADDING = '=';


YoureAuth::YoureAuth (std::string apiEndpointUrl, std::string clientId)
{
    m_clientId = FString(clientId.c_str());
    m_apiEndpointUrl = FString(apiEndpointUrl.c_str());

}
YoureAuth::~YoureAuth()
{
}



FString YoureAuth::getLoginUrl()
{

    PKCEHelper pkce;
    std::string code_verifier = pkce.randomString(128);
    std::string code_verifier_hex = pkce.convertStringToHex(code_verifier);
    std::string challenge64 = pkce.base64_encode2(pkce.getSHA256HashFromHex(code_verifier_hex));

    m_lastGeneratedCodeVerifier = FString(code_verifier_hex.c_str());

    FString url = "https://"+ m_apiEndpointUrl + "/oauth2/authorize?";
    url += "client_id=" + m_clientId;
    url += "&redirect_uri=unity:oauth";
    url += "&response_type=code";
    url += "&token_endpoint_auth_method=none";
    url += "&scope=openid email profile";
    url += "&code_challenge_method=S256";
    url += "&code_challenge=" + FString(challenge64.c_str());
    
    return url;
}



void YoureAuth::writeTokenCache(FString idToken, FString accessToken, FString refreshToken)
{

    FString file = FPaths::ProjectUserDir();
    file.Append(TEXT("youre_token.cache"));

    IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

    IFileHandle* f = FileManager.OpenWrite(*file);
    delete(f);

    TArray<FStringFormatArg> args;
    args.Add(FStringFormatArg(idToken));
    args.Add(FStringFormatArg(accessToken));
    args.Add(FStringFormatArg(refreshToken));

    
    FString jsonString = FString::Format(TEXT("{ \"id_token\": \"{0}\", \"access_token\" : \"{1}\", \"refresh_token\" : \"{2}\" }"), args);

    if (!FFileHelper::SaveStringToFile(jsonString, *file))
    {
        UE_LOG(LogYoure, Warning, TEXT("Failed to write token cache file"));
    }

}

bool YoureAuth::readTokenCacheChecked()
{
  
    FString file = FPaths::ProjectUserDir();
    file.Append(TEXT("youre_token.cache"));

    IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

    FString FileContent;
    if (FileManager.FileExists(*file))
    {
        if (FFileHelper::LoadFileToString(FileContent, *file, FFileHelper::EHashOptions::None))
        {
            TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*FileContent);
            if (FJsonSerializer::Deserialize(Reader, JsonObject))
            {
                if (JsonObject->HasField(TEXT("id_token"))) {
                    m_idToken = JsonObject->GetStringField(TEXT("id_token"));
                    m_accessToken = JsonObject->GetStringField(TEXT("access_token"));
                    m_refreshToken = JsonObject->GetStringField(TEXT("refresh_token"));
                    return true;
                }
                else {
                    UE_LOG(LogYoure, Warning, TEXT("Error while parsing token data. Token seems not valid."));
                }
            }
            else
            {
                UE_LOG(LogYoure, Warning, TEXT("Error while parsing token data"));
            }
        }
        else
        {
            UE_LOG(LogYoure, Warning, TEXT("Cached tokens could not be loaded"));
        }
    }

    return false;
}


void YoureAuth::requestTokenRefresh(const std::function<void()>& callback, const std::function<void()>& errorCallback)
{

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL("https://" + m_apiEndpointUrl + "/oauth2/token");
    Request->SetVerb("POST");
    Request->SetHeader("Content-Type", "application/x-www-form-urlencoded");
    FString PostData = FString::Printf(TEXT("client_id=%s&grant_type=refresh_token&refresh_token=%s"),*m_clientId,*m_refreshToken);
    Request->SetContentAsString(PostData);

    auto CreateDelegate = [this, callback, errorCallback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {

        if (bWasSuccessful && Response.IsValid())
        {
            FString ResponseBody = Response->GetContentAsString();
           
            TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*ResponseBody);
            if (FJsonSerializer::Deserialize(Reader, JsonObject))
            {
                if (JsonObject->HasField(TEXT("id_token"))) {
                    writeTokenCache(JsonObject->GetStringField(TEXT("id_token")),
                        JsonObject->GetStringField(TEXT("access_token")),
                        *m_refreshToken);
                    callback();
                }
                else
                {
                    UE_LOG(LogYoure, Error, TEXT("Refresh token request failed!"));
                    errorCallback();
                }
            }
            else
            {
                UE_LOG(LogYoure, Error, TEXT("Refresh token request failed!"));
                errorCallback();
            }
        }
        else
        {
            UE_LOG(LogYoure, Error, TEXT("Refresh token request failed!"));
            errorCallback();
        }

    };
    Request->OnProcessRequestComplete().BindLambda(CreateDelegate);
    Request->ProcessRequest();
}

void YoureAuth::requestUserInfo(const std::function<void(YoureUserInfo&)>& callback, const std::function<void()>& errorCallback)
{
    if (!isAuthenticated()) {
    
        UE_LOG(LogYoure, Warning, TEXT("Your service not authenticated"));
        return;
    }

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetVerb("GET");
    Request->SetURL("https://" + m_apiEndpointUrl + "/oauth2/userInfo");
    Request->SetHeader("Authorization", "Bearer "+m_accessToken);

    auto CreateDelegate = [this, callback, errorCallback] (FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {

        if (bWasSuccessful && Response.IsValid())
        {
            FString ResponseBody = Response->GetContentAsString();

            TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*ResponseBody);
            if (FJsonSerializer::Deserialize(Reader, JsonObject))
            {
                if (JsonObject->HasField(TEXT("sub"))) {

                    YoureUserInfo data;
                    data.userId = JsonObject->GetStringField(TEXT("sub"));
                    data.userName = JsonObject->GetStringField(TEXT("username"));
                    data.email = JsonObject->GetStringField(TEXT("email"));
                    callback(data);
                }
                else
                {
                    requestTokenRefresh([this, callback, errorCallback]() {
                        requestUserInfo(callback, errorCallback);
                        }, errorCallback);
                        
                    UE_LOG(LogYoure, Error, TEXT("get userinfo request failed: %s"),*ResponseBody);
                    errorCallback();
                }

            }
            else
            {
                UE_LOG(LogYoure, Error, TEXT("get userinfo request failed: %s"), *ResponseBody);
                errorCallback();
            }
        }
        else
        {
            UE_LOG(LogYoure, Error, TEXT("get userinfo request failed"));
            errorCallback();
        }
      
    };


    Request->OnProcessRequestComplete().BindLambda(CreateDelegate); 
    Request->ProcessRequest();
}

void YoureAuth::clearTokenCache() 
{
    FString file = FPaths::ProjectUserDir();
    file.Append(TEXT("youre_token.cache"));

    IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

    if (FileManager.FileExists(*file))
    {
        FileManager.DeleteFile(*file);
        m_accessToken = nullptr;
        m_idToken = nullptr;
        m_refreshToken = nullptr;
    }

}


void YoureAuth::requestAccessToken(FString code, const std::function<void()>& callback, const std::function<void(std::string)>& errorCallback)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
   
    Request->SetURL("https://" + m_apiEndpointUrl + "/oauth2/token");
    Request->SetVerb("POST");
    Request->SetHeader("Content-Type", "application/x-www-form-urlencoded");

    FString PostData = FString::Printf(TEXT("client_id=%s&grant_type=authorization_code&code_verifier=%s&code=%s&redirect_uri=unity:oauth&token_endpoint_auth_method=none"),
        *m_clientId,
        *m_lastGeneratedCodeVerifier,
        *code);

    Request->SetContentAsString(PostData);

    auto CreateDelegate = [this,callback, errorCallback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {

        if (bWasSuccessful && Response.IsValid())
        {
            FString ResponseBody = Response->GetContentAsString();

            TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*ResponseBody);
            if (FJsonSerializer::Deserialize(Reader, JsonObject))
            {
                if (JsonObject->HasField(TEXT("id_token"))) {
                    writeTokenCache(JsonObject->GetStringField(TEXT("id_token")),
                        JsonObject->GetStringField(TEXT("access_token")),
                        JsonObject->GetStringField(TEXT("refresh_token")));
                    callback();
                }
                else
                {
                    errorCallback("Access token json has no valid id_token");
                }
            }
            else
            {
                errorCallback("Access token json parsing failed");
            }
        }
        else
        {
            errorCallback("Access token request failed!");
        }
    };
    Request->OnProcessRequestComplete().BindLambda(CreateDelegate);
    Request->ProcessRequest();
}

bool YoureAuth::isAuthenticated()
{
    if (readTokenCacheChecked()) {

        return true;
    }
    return false;
}

void YoureAuth::authenticate()
{
    

    if (isAuthenticated()) {
        
    }
    else {
        
    }
}




