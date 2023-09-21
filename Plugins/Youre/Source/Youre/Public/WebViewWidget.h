// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WebViewWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnUrlChangedDelegate, FString);


UCLASS()
class YOURE_API UWebViewWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:

	FOnUrlChangedDelegate OnUrlChanged;
		
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UWebBrowser* WebBrowser;
		
protected:

	UFUNCTION()
		void InternalOnUrlChanged(const FText& text);

	// Doing setup in the C++ constructor is not as
	// useful as using NativeConstruct.
	virtual void NativeConstruct() override;
};
