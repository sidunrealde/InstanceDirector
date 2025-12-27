#include "InstanceDirector.h"
#include "InstanceDirectorSettings.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Misc/MessageDialog.h"
#include "Async/Async.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/CommandLine.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/Paths.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#define LOCTEXT_NAMESPACE "FInstanceDirectorModule"

FOnInstanceRedirected FInstanceDirectorModule::OnInstanceRedirected;

void FInstanceDirectorModule::StartupModule()
{
	// Do not run single instance check in Editor or Commandlets (Cooking, etc.)
	if (GIsEditor || IsRunningCommandlet())
	{
		return;
	}

	const UInstanceDirectorSettings* Settings = GetDefault<UInstanceDirectorSettings>();

	// Check if we should register the URI scheme on startup
	if (Settings->bRegisterURISchemeOnStartup && !Settings->URIScheme.IsEmpty())
	{
		RegisterURIScheme(Settings->URIScheme, Settings->URISchemeFriendlyName);
	}

	if (!CheckSingleInstance())
	{
		// We are a duplicate instance. We have already notified the existing one.
		// Request immediate exit
		FPlatformMisc::RequestExit(false);
	}
}

void FInstanceDirectorModule::ShutdownModule()
{
	if (InstanceListener)
	{
		delete InstanceListener;
		InstanceListener = nullptr;
	}
}

void FInstanceDirectorModule::RegisterURIScheme(const FString& SchemeName, const FString& FriendlyName)
{
#if PLATFORM_WINDOWS
	if (SchemeName.IsEmpty())
	{
		return;
	}

	FString ExePath = FPaths::ConvertRelativePathToFull(FPlatformProcess::ExecutablePath());
	// Ensure path is quoted
	FString Command = FString::Printf(TEXT("\"%s\" \"%%1\""), *ExePath);

	HKEY Key;
	LONG Result;

	// 1. Create the root key for the scheme
	// HKCU\Software\Classes\<SchemeName>
	FString RootKeyPath = TEXT("Software\\Classes\\") + SchemeName;
	Result = RegCreateKeyEx(HKEY_CURRENT_USER, *RootKeyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &Key, NULL);
	if (Result == ERROR_SUCCESS)
	{
		FString DefaultValue = TEXT("URL:") + FriendlyName;
		RegSetValueEx(Key, NULL, 0, REG_SZ, (const BYTE*)*DefaultValue, (DefaultValue.Len() + 1) * sizeof(TCHAR));
		
		FString UrlProtocol = TEXT("");
		RegSetValueEx(Key, TEXT("URL Protocol"), 0, REG_SZ, (const BYTE*)*UrlProtocol, (UrlProtocol.Len() + 1) * sizeof(TCHAR));
		
		RegCloseKey(Key);
	}

	// 2. Create the command key
	// HKCU\Software\Classes\<SchemeName>\shell\open\command
	FString CommandKeyPath = RootKeyPath + TEXT("\\shell\\open\\command");
	Result = RegCreateKeyEx(HKEY_CURRENT_USER, *CommandKeyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &Key, NULL);
	if (Result == ERROR_SUCCESS)
	{
		RegSetValueEx(Key, NULL, 0, REG_SZ, (const BYTE*)*Command, (Command.Len() + 1) * sizeof(TCHAR));
		RegCloseKey(Key);
	}
#endif
}

bool FInstanceDirectorModule::CheckSingleInstance()
{
	const UInstanceDirectorSettings* Settings = GetDefault<UInstanceDirectorSettings>();
	
	if (!Settings->bEnableSingleInstanceCheck)
	{
		return true;
	}

	const int32 Port = Settings->PortNumber;
	FIPv4Endpoint Endpoint(FIPv4Address::InternalLoopback, Port);

	// Try to start a listener on the port
	// IMPORTANT: Set bReusable to false to prevent multiple instances from binding to the same port!
	InstanceListener = new FTcpListener(Endpoint, FTimespan::FromSeconds(1), false);
	
	// Bind the connection handler
	InstanceListener->OnConnectionAccepted().BindRaw(this, &FInstanceDirectorModule::HandleConnectionAccepted);

	if (InstanceListener->IsActive())
	{
		// We successfully bound to the port, so we are the first instance.
		return true;
	}
	else
	{
		// Failed to bind, likely because another instance is running.
		// Notify the existing instance to bring it to front.
		NotifyExistingInstance(Port);
		
		// Clean up the failed listener
		delete InstanceListener;
		InstanceListener = nullptr;
		
		return false;
	}
}

void FInstanceDirectorModule::NotifyExistingInstance(int32 Port)
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (SocketSubsystem)
	{
		TSharedRef<FInternetAddr> Addr = SocketSubsystem->CreateInternetAddr();
		Addr->SetIp(FIPv4Address::InternalLoopback.Value);
		Addr->SetPort(Port);

		FSocket* Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("InstanceDirectorNotifier"), false);
		if (Socket)
		{
			// Connect to the existing instance
			if (Socket->Connect(*Addr))
			{
				// Send command line arguments
				FString CmdLine = FCommandLine::Get();
				FTCHARToUTF8 Convert(*CmdLine);
				int32 Len = Convert.Length();
				int32 BytesSent = 0;

				// Send length
				Socket->Send((uint8*)&Len, sizeof(int32), BytesSent);
				
				// Send data
				if (Len > 0)
				{
					Socket->Send((uint8*)Convert.Get(), Len, BytesSent);
				}
			}
			
			// Graceful shutdown
			Socket->Shutdown(ESocketShutdownMode::ReadWrite);
			Socket->Close();
			SocketSubsystem->DestroySocket(Socket);
		}
	}
}

bool FInstanceDirectorModule::HandleConnectionAccepted(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	// We received a connection, which means a duplicate instance tried to start.
	
	// Read command line arguments
	int32 Len = 0;
	int32 BytesRead = 0;
	FString ReceivedArguments;

	// Read length
	if (ClientSocket->Recv((uint8*)&Len, sizeof(int32), BytesRead) && BytesRead == sizeof(int32))
	{
		if (Len > 0)
		{
			TArray<uint8> Buffer;
			Buffer.SetNumUninitialized(Len + 1); // +1 for null terminator safety
			
			// Loop to ensure we read all bytes
			int32 TotalBytesRead = 0;
			while (TotalBytesRead < Len)
			{
				int32 ChunkRead = 0;
				if (ClientSocket->Recv(Buffer.GetData() + TotalBytesRead, Len - TotalBytesRead, ChunkRead))
				{
					TotalBytesRead += ChunkRead;
					if (ChunkRead == 0)
					{
						// Connection closed prematurely
						break;
					}
				}
				else
				{
					// Error reading
					break;
				}
			}

			if (TotalBytesRead == Len)
			{
				Buffer[Len] = 0; // Null terminate
				ReceivedArguments = FUTF8ToTCHAR((const char*)Buffer.GetData()).Get();
			}
		}
	}

	// Clean up the socket manually since we are returning true
	ClientSocket->Close();
	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket);
	
	// We want to run this on the game thread
	AsyncTask(ENamedThreads::GameThread, [this, ReceivedArguments]()
	{
		FocusWindow();
		OnInstanceRedirected.Broadcast(ReceivedArguments);
	});

	return true; // We took ownership and destroyed the socket
}

void FInstanceDirectorModule::FocusWindow()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication& SlateApp = FSlateApplication::Get();
		
		// Try to find a window to activate
		TSharedPtr<SWindow> ActiveWindow = SlateApp.GetActiveTopLevelWindow();
		
		// If no active window, try to get any window
		if (!ActiveWindow.IsValid())
		{
			TArray<TSharedRef<SWindow>> Windows;
			SlateApp.GetAllVisibleWindowsOrdered(Windows);
			if (Windows.Num() > 0)
			{
				ActiveWindow = Windows[0];
			}
		}

		if (ActiveWindow.IsValid())
		{
			// Standard Slate bring to front
			ActiveWindow->BringToFront();
			SlateApp.SetUserFocus(0, ActiveWindow);

#if PLATFORM_WINDOWS
			if (ActiveWindow->GetNativeWindow().IsValid())
			{
				HWND Hwnd = (HWND)ActiveWindow->GetNativeWindow()->GetOSWindowHandle();
				if (Hwnd)
				{
					// Restore if minimized
					if (IsIconic(Hwnd))
					{
						ShowWindow(Hwnd, SW_RESTORE);
					}
					
					// Force foreground
					SetForegroundWindow(Hwnd);
				}
			}
#endif
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInstanceDirectorModule, InstanceDirector)
