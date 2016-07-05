// Copyright 2016 Daniel Schukies. All Rights Reserved.

#pragma once
#include "StomtPluginPrivatePCH.h"
#include "StomtJsonObject.h"
#include "StomtRestRequest.generated.h"


/**
* @author Original latent action class by https://github.com/unktomi
*/
template <class T> class FVaRestLatentAction : public FPendingLatentAction
{
public:
	virtual void Call(const T &Value)
	{
		Result = Value;
		Called = true;
	}

	void operator()(const T &Value)
	{
		Call(Value);
	}

	void Cancel();

	FVaRestLatentAction(FWeakObjectPtr RequestObj, T& ResultParam, const FLatentActionInfo& LatentInfo) :
		Called(false),
		Request(RequestObj),
		ExecutionFunction(LatentInfo.ExecutionFunction),
		OutputLink(LatentInfo.Linkage),
		CallbackTarget(LatentInfo.CallbackTarget),
		Result(ResultParam)
	{
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		Response.FinishAndTriggerIf(Called, ExecutionFunction, OutputLink, CallbackTarget);
	}

	virtual void NotifyObjectDestroyed()
	{
		Cancel();
	}

	virtual void NotifyActionAborted()
	{
		Cancel();
	}

private:
	bool Called;
	FWeakObjectPtr Request;

public:
	const FName ExecutionFunction;
	const int32 OutputLink;
	const FWeakObjectPtr CallbackTarget;
	T &Result;

};

 /** Verb (GET, PUT, POST) */
UENUM(BlueprintType)
namespace ERequestVerb
{
	enum Type
	{
		GET,
		POST,
		PUT,
		DEL UMETA(DisplayName = "DELETE"),
	};
}

/** Generate a delegates for callback events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRequestComplete, class UStomtRestRequest*, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRequestFail, class UStomtRestRequest*, Request);


UCLASS()
class UStomtRestRequest : public UObject
{
	GENERATED_BODY()

public:
	//////////////////////////////////////////////////////////////////////////
	// Construction

	UStomtRestRequest();
	~UStomtRestRequest();

	void MyHttpCall();

	static UStomtRestRequest* ConstructRequest();

	/** Set verb to the request */
	void SetVerb(ERequestVerb::Type Verb);

	/** Sets header info */
	void SetHeader(const FString &HeaderName, const FString &HeaderValue);

	void OnResponseReceived(
		FHttpRequestPtr Request, 
		FHttpResponsePtr Response, 
		bool bWasSuccessful);

	//////////////////////////////////////////////////////////////////////////
	// Destruction and reset

	/** Reset all internal saved data */
	void ResetData();

	/** Reset saved request data */
	void ResetRequestData();

	/** Reset saved response data */
	void ResetResponseData();

	/** Cancel latent response waiting  */
	void Cancel();


	//////////////////////////////////////////////////////////////////////////
	// JSON data accessors
	
	/** Get the Request Json object */
	UStomtRestJsonObject* GetRequestObject();

	/** Set the Request Json object */
	void SetRequestObject(UStomtRestJsonObject* JsonObject);

	/** Get the Response Json object */
	UStomtRestJsonObject* GetResponseObject();

	/** Set the Response Json object */
	void SetResponseObject(UStomtRestJsonObject* JsonObject);


	///////////////////////////////////////////////////////////////////////////
	// Response data access

	/** Get the responce code of the last query */
	int32 GetResponseCode();

	/** Get value of desired response header */
	FString GetResponseHeader(const FString HeaderName);

	/** Get list of all response headers */
	TArray<FString> GetAllResponseHeaders();


	//////////////////////////////////////////////////////////////////////////
	// URL processing

	/** Open URL with current setup */
	virtual void ProcessURL(const FString& Url = TEXT("http://alyamkin.com"));

	/** Apply current internal setup to request and process it */
	void ProcessRequest(TSharedRef<IHttpRequest> HttpRequest);


	//////////////////////////////////////////////////////////////////////////
	// Request callbacks

private:
	/** Internal bind function for the IHTTPRequest::OnProcessRequestCompleted() event */
	void OnProcessRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

public:
	/** Event occured when the request has been completed */
	UPROPERTY(BlueprintAssignable, Category = "Stomt|Event")
	FOnRequestComplete OnRequestComplete;

	/** Event occured when the request wasn't successfull */
	UPROPERTY(BlueprintAssignable, Category = "Stomt|Event")
	FOnRequestFail OnRequestFail;
	
	//////////////////////////////////////////////////////////////////////////
	// Data
public:

	/** Request response stored as a string */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StomtRest|Response")
	FString ResponseContent;

	/** Is the response valid JSON? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StomtRest|Response")
	bool bIsValidJsonResponse;

protected:
	/** Latent action helper */
	FVaRestLatentAction <UStomtRestJsonObject*> *ContinueAction;

	/** Internal request data stored as JSON */
	UStomtRestJsonObject* RequestJsonObj;

	/** Response data stored as JSON */
	UStomtRestJsonObject* ResponseJsonObj;

	/** Verb for making request (GET,POST,etc) */
	ERequestVerb::Type RequestVerb;

	/** Mapping of header section to values. Used to generate final header string for request */
	TMap<FString, FString> RequestHeaders;

	/** Cached key/value header pairs. Parsed once request completes */
	TMap<FString, FString> ResponseHeaders;

	/** Http Response code */
	int32 ResponseCode;

};




