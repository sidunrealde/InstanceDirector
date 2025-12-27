#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Common/TcpListener.h"

DECLARE_LOG_CATEGORY_EXTERN(LogInstanceDirector, Log, All);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnInstanceRedirected, const FString& /* Arguments */);

class FInstanceDirectorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Accessor for the redirect delegate */
	static FOnInstanceRedirected& GetOnInstanceRedirected() { return OnInstanceRedirected; }

	/** Registers the URI scheme in the Windows Registry */
	static void RegisterURIScheme(const FString& SchemeName, const FString& FriendlyName);

	/** Gets the raw command line from the OS */
	static FString GetRawCommandLine();

private:
	bool CheckSingleInstance();
	void NotifyExistingInstance(int32 Port);
	bool HandleConnectionAccepted(class FSocket* ClientSocket, const struct FIPv4Endpoint& ClientEndpoint);
	void FocusWindow();

	class FTcpListener* InstanceListener = nullptr;
	static FOnInstanceRedirected OnInstanceRedirected;
};
