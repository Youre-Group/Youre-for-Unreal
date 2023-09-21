# Youre-for-Unreal

> The YOURE Sign In Plugin for Unreal Engine provides a simple and convenient way to integrate YOURE sign-in functionality into their applications. With this plugin code, users can quickly and easily sign in to YOURE and access their accounts without leaving the Game.


### Supported Platforms: 
Windows Desktop, Android

## Install:
1. Download Release package and unpack
2. Copy the unpacked _Youre_ directory to _YOUR_PROJECT_PATH/Plugins/_
3. Add **/Youre/Webview** to **Platforms -> Packaging -> Additional Assset Directories to Cook**
3. Restart Unreal Project



## Usage 
You can modify the Login Layer Widget to your needs, you will find it the Content Drawer: _Plugins/Youre/Content/WebView/WebView_

```c++

#include "Youre.h"
#include "Engine/World.h"
.........
  

if (FYoureModule::IsAvailable())
{
  FYoureModule& youre = FYoureModule::Get();
  youre.AuthSucceeded.Clear();
  youre.AuthSucceeded.AddLambda([this](YoureUserInfo user) {
    UE_LOG(LogTemp, Warning, TEXT("Success! YOURE ID: %s"), *user.userId);
    UE_LOG(LogTemp, Warning, TEXT("Success! YOURE AccessToken: %s"), *user.accessToken);
    });

  youre.AuthFailed.Clear();
  youre.AuthFailed.AddLambda([this](FString error) {
    UE_LOG(LogTemp, Warning, TEXT("Failed! Error: %s"), *FString(error));
    });

  // Client Id, Endpoint URL, Endpoint URL will provided from YOURE Games
  youre.Initialize("{ENTER YOUR CLIENT ID}", "https://{ENTER ENDPOINT URL}", "https://{ENTER REDIRECT URL}");
  youre.Authenticate(GetWorld());
}
```
