// Copyright 2016 Daniel Schukies. All Rights Reserved.


#include "StomtPluginPrivatePCH.h"
#include "StomtPluginWidget.h"
#include "StomtRestRequest.h"
#include "StomtLabel.h"
#include "Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h"



UStomtPluginWidget::~UStomtPluginWidget()
{
	this->LoginErrorCode = 0;
}

void UStomtPluginWidget::OnConstruction(FString TargetID, FString RestURL, FString AppID)
{
	// Create API Object
	if (api == NULL)
	{
		api = UStomtAPI::ConstructStomtAPI(TargetID, RestURL, AppID);
	}
	else
	{
		this->api->SetAppID(AppID);
		this->api->SetTargetID(TargetID);
		this->api->SetRestURL(RestURL);
	}

	this->Request = this->api->GetRequest();

	// Request Target Name
	UStomtRestRequest* request = this->api->RequestTarget(TargetID);
	request->OnRequestComplete.AddDynamic(this, &UStomtPluginWidget::OnTargetResponse);

	this->api->GetRequest()->OnRequestComplete.AddDynamic(this, &UStomtPluginWidget::OnLoginRequestResponse);

	//Lookup EMail
	this->IsEMailAlreadyKnown = this->api->Config->GetSubscribed();
	this->IsUserLoggedIn = this->api->Config->GetLoggedIn();
}

void UStomtPluginWidget::OnMessageChanged(FString text)
{

	if (!text.IsEmpty())
	{
		this->Message = text;
	}
	else
	{
		this->Message = FString(TEXT(""));
	}
}

void UStomtPluginWidget::OnSubmit()
{
	//TakeScreenshot();

	if (!this->Message.IsEmpty())
	{
		this->stomt = UStomt::ConstructStomt(this->api->GetTargetID(), !this->IsWish, this->Message);
		this->stomt->SetLabels(this->Labels);

		stomt->SetAnonym(false);

		// API Object should not be null ;)
		this->api->SetStomtToSend(stomt);

		FString LogFileName = FApp::GetGameName() + FString(TEXT(".log"));

		this->api->SendLogFile(this->api->ReadLogFile(LogFileName), LogFileName);

		// Check EMail
		this->IsEMailAlreadyKnown = this->api->Config->GetSubscribed();

		//UE_LOG(LogTemp, Warning, TEXT("email: %s"), this->IsEMailAlreadyKnown ? TEXT("true") : TEXT("false"));
	}
}

void UStomtPluginWidget::OnSubmitLastLayer()
{
	/*
	if (!this->EMail.IsEmpty() && !this->UserPassword.IsEmpty())
	{
		this->api->SendLoginRequest(this->EMail, this->UserPassword);
	}

	if (!this->EMail.IsEmpty())
	{
		this->api->SendEMail(this->EMail);
	}
	*/
}

bool UStomtPluginWidget::OnSubmitLogin()
{
	if (!this->UserName.IsEmpty() && !this->UserPassword.IsEmpty())
	{
		UStomtRestRequest* request = this->api->SendLoginRequest(this->UserName, this->UserPassword);
		request->OnRequestComplete.AddDynamic(this, &UStomtPluginWidget::OnLoginRequestResponse);
		this->LoginErrorCode = 0;
		return true;
	}
	else
	{
		return false;
	}

}

void UStomtPluginWidget::OnSubmitEMail()
{
	if (!this->EMail.IsEmpty())
	{
		this->api->SendEMail(this->EMail);
	}
}

void UStomtPluginWidget::OnLoginRequestResponse(UStomtRestRequest * LoginRequest)
{

	this->LoginErrorCode = LoginRequest->GetResponseCode();
	this->IsUserLoggedIn = this->api->Config->GetLoggedIn();
	this->api->OnLoginRequestComplete.Broadcast(LoginRequest);
}

void UStomtPluginWidget::OnTargetResponse(UStomtRestRequest * TargetRequest)
{
	this->TargetName = this->api->GetTargetName();
	this->ImageURL = this->api->GetImageURL();

	this->api->OnTargetRequestComplete.Broadcast(Request);
}

void UStomtPluginWidget::TakeScreenshot()
{
	USceneCaptureComponent2D* cap = NewObject<USceneCaptureComponent2D>();
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
	RenderTarget->InitAutoFormat(1024, 1024);
	//UMaterialInstanceDynamic* ScopeMat = UMaterialInstanceDynamic::Create(AssetScopeMat_Default, this);
	cap->TextureTarget = RenderTarget;
	if (!api->CaptureComponent2D_SaveImage(cap, FString("bild.png"), FLinearColor()))
	{
		UE_LOG(LogTemp, Warning, TEXT("bad!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("nice"));
	}
	/*
	FTexureRenderTarget2DResource *Texture = (FTextureRenderTarget2DResource *)SceneCapture->TextureTarget->Resource;
	TArray<FColor> ColorBuffer;
	Texture->ReadPixels(ColorBuffer);*/
}

