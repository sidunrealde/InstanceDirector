# Instance Director - Developer Guide

This guide is intended for C++ developers who want to understand, modify, or extend the **Instance Director** plugin.

## Architecture Overview

The plugin consists of three main components:

1.  **FInstanceDirectorModule (`InstanceDirector.cpp`)**: The core logic.
    *   **Startup**: Checks for existing instances using `FTcpListener`.
    *   **Server**: Listens on a localhost port (default 64321) for incoming connections.
    *   **Client**: If an instance exists, connects to it, sends command-line arguments, and requests focus.
    *   **Registry**: Handles Windows Registry writes for URI scheme registration.

2.  **UInstanceDirectorSubsystem (`InstanceDirectorSubsystem.cpp`)**: The Blueprint interface.
    *   **Bridge**: Listens to the Module's C++ delegate and broadcasts a dynamic multicast delegate (`OnAppRedirected`) to Blueprints.
    *   **Helpers**: Provides `CheckStartupArguments` to handle cold starts.

3.  **UInstanceDirectorSettings (`InstanceDirectorSettings.cpp`)**: Configuration.
    *   Exposes settings to `Project Settings > Game`.

## Key Flows

### 1. Single Instance Check
*   **Location**: `FInstanceDirectorModule::StartupModule` -> `CheckSingleInstance`
*   **Mechanism**: Attempts to bind `FTcpListener` to `127.0.0.1:Port`.
    *   **Success**: We are the first instance. Keep listening.
    *   **Failure**: Port is in use. We are a duplicate. Call `NotifyExistingInstance`.

### 2. Inter-Process Communication (IPC)
*   **Protocol**: Simple TCP stream.
    *   [4 bytes] Length of string (int32).
    *   [N bytes] UTF-8 string data.
*   **Handling**:
    *   `NotifyExistingInstance` (Client): Connects, sends length, sends string, sleeps briefly, closes.
    *   `HandleConnectionAccepted` (Server): Accepts, sets blocking, reads length, reads string, broadcasts event.

### 3. Window Focus (Windows)
*   **Challenge**: Windows prevents background processes from stealing focus.
*   **Solution**:
    *   **Client**: Calls `AllowSetForegroundWindow(ASFW_ANY)` to grant permission.
    *   **Server**: Calls `SetForegroundWindow`, `BringWindowToTop`, and `SwitchToThisWindow` to force the window to the front.

### 4. URI Scheme Registration
*   **Location**: `FInstanceDirectorModule::RegisterURIScheme`
*   **Method**: Writes to `HKCU\Software\Classes\<Scheme>`.
*   **Command**: `"Path\To\Exe" "%1"`
*   **Normalization**: Uses `FPaths::MakePlatformFilename` to ensure backslashes are used, which is critical for Windows Registry compatibility.

## Extension Points

*   **Custom Protocol**: You can modify the IPC protocol in `NotifyExistingInstance` and `HandleConnectionAccepted` to send more structured data (e.g., JSON) instead of a raw string.
*   **Platform Support**: Currently heavily optimized for Windows (Registry, Focus). To support Mac/Linux:
    *   Implement `RegisterURIScheme` for macOS (`Info.plist` modification or LaunchServices).
    *   Implement `FocusWindow` using platform-specific APIs.

## Debugging

*   **Log Category**: `LogInstanceDirector`
*   **Logs**: Check `Saved/Logs/YourProject.log`.
*   **Registry Verification**: The plugin logs verification steps when registering the URI scheme. Look for `VERIFICATION: Match confirmed`.
