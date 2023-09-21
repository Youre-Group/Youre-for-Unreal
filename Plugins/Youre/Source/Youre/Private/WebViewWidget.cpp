// Copyright Youre Games, All Rights Reserved.


#include "WebViewWidget.h"
#include "Blueprint/UserWidget.h"
#include <WebBrowser.h>
DECLARE_EVENT(UWebViewWidget, InternalOnUrlChanged2)


void UWebViewWidget::InternalOnUrlChanged(const FText& text)
{
	FString st = text.ToString();
	OnUrlChanged.Broadcast(st);
}

void UWebViewWidget::NativeConstruct()
{
	Super::NativeConstruct(); 
	FScriptDelegate delegate;
	delegate.BindUFunction(this, TEXT("InternalOnUrlChanged"));
	WebBrowser->OnUrlChanged.Add(delegate);
}