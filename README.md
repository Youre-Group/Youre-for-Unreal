# Youre-for-Unreal

> The YOURE Sign In Plugin for Unreal Engine provides a simple and convenient way to integrate YOURE sign-in functionality into their applications. With this plugin code, users can quickly and easily sign in to YOURE and access their accounts without leaving the Game.


### Supported Platforms: 
Desktop



## Usage

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
    });

  youre.AuthFailed.Clear();
  youre.AuthFailed.AddLambda([this](std::string error) {
    UE_LOG(LogTemp, Warning, TEXT("Failed! Error: %s"), *FString(error.c_str()));
    });

  youre.Initialize("{ENTER YOUR CLIENT ID}", "sso.prepro.youre.id");
  youre.Authenticate(GetWorld());
}
```
