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
	
	FString ParsedArgs = ParseArguments(Arguments);
	
	// Only broadcast if we have something meaningful
	if (!ParsedArgs.IsEmpty())
	{
		UE_LOG(LogInstanceDirector, Log, TEXT("Parsed Arguments: %s"), *ParsedArgs);
		OnAppRedirected.Broadcast(ParsedArgs);
	}
	else
	{
		UE_LOG(LogInstanceDirector, Log, TEXT("Parsed arguments are empty. Ignoring."));
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
		FString ParsedArgs = ParseArguments(CmdLine);
		
		if (!ParsedArgs.IsEmpty())
		{
			UE_LOG(LogInstanceDirector, Log, TEXT("Parsed Startup Arguments: %s"), *ParsedArgs);
			OnAppRedirected.Broadcast(ParsedArgs);
		}
		else
		{
			UE_LOG(LogInstanceDirector, Log, TEXT("Parsed startup arguments are empty. Ignoring."));
		}
	}
}

FString UInstanceDirectorSubsystem::ParseArguments(const FString& CommandLine)
{
	const TCHAR* Stream = *CommandLine;
	FString Token;
	FString Result;
	bool bFirstToken = true;

	while (FParse::Token(Stream, Token, false))
	{
		// Skip the first token (Executable path)
		if (bFirstToken)
		{
			bFirstToken = false;
			continue;
		}

		// Check for Deep Link
		int32 Index = Token.Find(TEXT("://"));
		if (Index != INDEX_NONE)
		{
			// Found a deep link! Return everything after "://"
			FString DeepLink = Token.Mid(Index + 3);
			
			// Remove trailing slash if present
			if (DeepLink.EndsWith(TEXT("/")))
			{
				DeepLink.LeftChopInline(1);
			}
			
			// If we find a deep link, we usually prioritize it and return just that
			// (Assuming one deep link per launch)
			return DeepLink;
		}

		// Accumulate other arguments
		if (!Result.IsEmpty())
		{
			Result += TEXT(" ");
		}
		Result += Token;
	}
	
	return Result;
}
