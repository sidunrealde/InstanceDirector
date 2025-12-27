#include "InstanceDirectorSubsystem.h"
#include "InstanceDirector.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

void UInstanceDirectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogInstanceDirector, Log, TEXT("InstanceDirectorSubsystem Initialized."));

	// Bind to the static delegate in the module
	FInstanceDirectorModule::GetOnInstanceRedirected().AddUObject(this, &UInstanceDirectorSubsystem::HandleRedirect);
}

void UInstanceDirectorSubsystem::Deinitialize()
{
	UE_LOG(LogInstanceDirector, Log, TEXT("InstanceDirectorSubsystem Deinitialized."));
	Super::Deinitialize();
}

void UInstanceDirectorSubsystem::HandleRedirect(const FString& Arguments)
{
	UE_LOG(LogInstanceDirector, Log, TEXT("Subsystem received redirect arguments: %s"), *Arguments);
	
	FString DeepLink = ExtractDeepLink(Arguments);
	if (!DeepLink.IsEmpty())
	{
		UE_LOG(LogInstanceDirector, Log, TEXT("Extracted Deep Link: %s"), *DeepLink);
		OnAppRedirected.Broadcast(DeepLink);
	}
	else
	{
		// If no deep link found, broadcast original arguments (or maybe nothing?)
		// User requested "Just send the string after ://".
		// If we can't find ://, maybe we shouldn't broadcast?
		// But for normal args like "-OpenMenu", we might want to keep them.
		// Let's assume if no URI is found, we broadcast the full args as fallback, 
		// OR we try to be smart.
		// Given the request "Just send the string after ://", I will prioritize that.
		// But if I launch with "-OpenMenu", there is no ://.
		// I'll stick to: If URI found, send suffix. Else send full.
		
		UE_LOG(LogInstanceDirector, Log, TEXT("No URI scheme found. Broadcasting full arguments."));
		OnAppRedirected.Broadcast(Arguments);
	}
}

void UInstanceDirectorSubsystem::RegisterURIScheme(FString SchemeName)
{
	UE_LOG(LogInstanceDirector, Log, TEXT("Subsystem RegisterURIScheme called for: %s"), *SchemeName);
	// Use the static function in the module
	// We use a default friendly name here if called from BP without one
	FInstanceDirectorModule::RegisterURIScheme(SchemeName, TEXT("Instance Director Application"));
}

void UInstanceDirectorSubsystem::CheckStartupArguments()
{
	// Use GetRawCommandLine to ensure we get the full arguments including URI
	FString CmdLine = FInstanceDirectorModule::GetRawCommandLine();
	UE_LOG(LogInstanceDirector, Log, TEXT("CheckStartupArguments called. Command Line: %s"), *CmdLine);
	
	if (!CmdLine.IsEmpty())
	{
		FString DeepLink = ExtractDeepLink(CmdLine);
		if (!DeepLink.IsEmpty())
		{
			UE_LOG(LogInstanceDirector, Log, TEXT("Extracted Deep Link: %s"), *DeepLink);
			OnAppRedirected.Broadcast(DeepLink);
		}
		else
		{
			// Fallback to full command line if no URI found
			OnAppRedirected.Broadcast(CmdLine);
		}
	}
}

FString UInstanceDirectorSubsystem::ExtractDeepLink(const FString& CommandLine)
{
	// Simple parsing to find a token containing "://"
	// We iterate through tokens to handle quotes properly
	
	const TCHAR* Stream = *CommandLine;
	FString Token;
	
	while (FParse::Token(Stream, Token, false))
	{
		int32 Index = Token.Find(TEXT("://"));
		if (Index != INDEX_NONE)
		{
			// Found it! Return everything after "://"
			return Token.Mid(Index + 3);
		}
	}
	
	return FString();
}
