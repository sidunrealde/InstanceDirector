// Copyright SiddarthaG 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InstanceDirectorSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAppRedirectedMCDelegate, FString, Arguments);

/**
 * Subsystem to handle Instance Director events.
 */
UCLASS()
class INSTANCEDIRECTOR_API UInstanceDirectorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Called when a new instance attempts to start and redirects to this one. */
	UPROPERTY(BlueprintAssignable, Category = "Instance Director")
	FOnAppRedirectedMCDelegate OnAppRedirected;

	/** 
	 * Registers a custom URI scheme (e.g., "myapp") for the current application.
	 * This allows the app to be opened via web links like myapp://...
	 * @param SchemeName The protocol name (without ://), e.g., "myapp".
	 */
	UFUNCTION(BlueprintCallable, Category = "Instance Director")
	void RegisterURIScheme(FString SchemeName);

	/**
	 * Checks the command line arguments used to launch this instance.
	 * If arguments are found, it broadcasts the OnAppRedirected event.
	 * Call this in your GameInstance Init after binding to the event to handle cold starts (e.g. URI links).
	 */
	UFUNCTION(BlueprintCallable, Category = "Instance Director")
	void CheckStartupArguments();

private:
	void HandleRedirect(const FString& Arguments);
	
	/** 
	 * Parses the raw command line to extract relevant arguments.
	 * - If a Deep Link (://) is found, returns the suffix (e.g. "mygame://foo" -> "foo").
	 * - If no Deep Link, returns the arguments excluding the executable path.
	 * - If only executable path is present, returns empty string.
	 */
	FString ParseArguments(const FString& CommandLine);
};
