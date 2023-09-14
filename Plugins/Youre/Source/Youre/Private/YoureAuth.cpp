#include "YoureAuth.h"
#include "CoreMinimal.h"
#include "PKCEHelper.h"
#include <stdlib.h>
#include "Misc/FileHelper.h"
#include "Json.h"
#include <functional>
#include "Http.h"
#include "Serialization/JsonTypes.h"
static const char* const HEX_DIGITS = "0123456789abcdef";
static const char* const BASE64_DIGITS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char BASE64_PADDING = '=';


YoureAuth::YoureAuth (FString apiEndpointUrl, FString clientId)
{
    m_clientId = clientId;
    m_apiEndpointUrl = apiEndpointUrl;

}
YoureAuth::~YoureAuth()
{
}



FString YoureAuth::getLoginUrl()
{

    PKCEHelper pkce;
    std::string code_verifier = pkce.randomString(128);
    std::string challenge64 = pkce.base64_encode2(pkce.getSHA256HashFromHex(code_verifier));

    m_lastGeneratedCodeVerifier = FString(code_verifier.c_str());

    FString url = "https://"+ m_apiEndpointUrl + "/authorize?";
    url += "client_id=" + m_clientId;
    url += "&redirect_uri=unity:oauth";
    url += "&response_type=code";
    url += "&token_endpoint_auth_method=none";
    url += "&scope=openid email profile";
    url += "&code_challenge_method=S256";
    url += "&code_challenge=" + FString(challenge64.c_str());
    
    return url;
}



void YoureAuth::writeTokenCache(FString accessToken)
{

    FString file = FPaths::ProjectUserDir();
    file.Append(TEXT("youre_token.cache"));

    IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

    IFileHandle* f = FileManager.OpenWrite(*file);
    delete(f);

    TArray<FStringFormatArg> args;
    args.Add(FStringFormatArg(accessToken));

    
    FString jsonString = FString::Format(TEXT("{  \"access_token\" : \"{0}\" }"), args);

    if (!FFileHelper::SaveStringToFile(jsonString, *file))
    {
        UE_LOG(LogYoure, Warning, TEXT("Failed to write token cache file"));
    }

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

                    FString sub = JsonObject->GetStringField(TEXT("sub"));
                    sub.RemoveFromStart("auth0|");
                    YoureUserInfo data;
                    data.userId = sub;
                    if (JsonObject->HasTypedField<EJson::String>(TEXT("username"))) {
                        data.userName = JsonObject->GetStringField(TEXT("username"));
                    }
                        
                    if (JsonObject->HasTypedField<EJson::String>(TEXT("email"))) {
                        data.email = JsonObject->GetStringField(TEXT("email"));
                    }
                  
                    callback(data);
                }
                else
                {
                    errorCallback();
                }

            }
            else
            {
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
    }

}

void YoureAuth::requestAccessToken(FString code, const std::function<void()>& callback, const std::function<void(FString)>& errorCallback)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
   
    Request->SetURL("https://" + m_apiEndpointUrl + "/oauth/token");
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
                if (JsonObject->HasField(TEXT("access_token"))) {
                    writeTokenCache(JsonObject->GetStringField(TEXT("access_token")));
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
                if (JsonObject->HasField(TEXT("access_token"))) {
                    m_accessToken = JsonObject->GetStringField(TEXT("access_token"));
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




