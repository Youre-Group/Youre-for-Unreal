// Copyright Youre Games, All Rights Reserved.


#include "WebViewUserWidget.h"
#include "Blueprint/UserWidget.h"
#include <WebBrowser.h>

void UWebViewUserWidget::InternalOnUrlChanged(const FText& text)
{
	FString st = text.ToString();
	OnUrlChanged.Broadcast(st);
}

void UWebViewUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	TScriptDelegate delegate;
	delegate.BindUFunction(this, TEXT("InternalOnUrlChanged"));
	WebBrowser->OnUrlChanged.Add(delegate);
}