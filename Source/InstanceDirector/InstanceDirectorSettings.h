#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InstanceDirectorSettings.generated.h"

/**
 * Settings for the Instance Director plugin.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Instance Director"))
class INSTANCEDIRECTOR_API UInstanceDirectorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UInstanceDirectorSettings();

	// --- General Settings ---

	/** If true, the application will check if another instance is running on startup. */
	UPROPERTY(Config, EditAnywhere, Category = "General")
	bool bEnableSingleInstanceCheck;

	/** The port number used to check for a running instance. Must be unique to this application. */
	UPROPERTY(Config, EditAnywhere, Category = "General", meta = (ClampMin = "1024", ClampMax = "65535"))
	int32 PortNumber;

	// --- Deep Linking Settings ---

	/** 
	 * Custom URI Scheme to register (e.g. "myapp"). 
	 * Do not include "://". 
	 * This allows opening the app via links like myapp://...
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Deep Linking")
	FString URIScheme;

	/** 
	 * Friendly name for the URI protocol (e.g. "My Game Protocol"). 
	 * Used in the Windows Registry description.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Deep Linking")
	FString URISchemeFriendlyName;

	/** 
	 * If true, the plugin will automatically attempt to register the URI scheme in the Windows Registry on application startup. 
	 * Note: This operation writes to the registry (HKCU).
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Deep Linking")
	bool bRegisterURISchemeOnStartup;
};
