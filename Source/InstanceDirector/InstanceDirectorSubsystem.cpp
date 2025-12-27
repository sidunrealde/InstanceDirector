#include "InstanceDirectorSubsystem.h"
#include "InstanceDirector.h"

void UInstanceDirectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Bind to the static delegate in the module
	FInstanceDirectorModule::GetOnInstanceRedirected().AddUObject(this, &UInstanceDirectorSubsystem::HandleRedirect);
}

void UInstanceDirectorSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UInstanceDirectorSubsystem::HandleRedirect(const FString& Arguments)
{
	OnAppRedirected.Broadcast(Arguments);
}

void UInstanceDirectorSubsystem::RegisterURIScheme(FString SchemeName)
{
	// Use the static function in the module
	// We use a default friendly name here if called from BP without one
	FInstanceDirectorModule::RegisterURIScheme(SchemeName, TEXT("Instance Director Application"));
}
