# Instance Director

**Instance Director** is a robust Unreal Engine plugin designed to manage application instances. It ensures single-instance enforcement, handles automatic window focusing, and provides seamless deep linking (app redirection) capabilities via custom URI schemes.

## Features

*   **Single Instance Enforcement**: Prevents multiple copies of your application from running simultaneously.
*   **Automatic Redirection**: Automatically brings the existing instance to the foreground when a user attempts to launch a second one.
*   **Deep Linking (URI Schemes)**: Register custom protocols (e.g., `mygame://`) to launch or redirect to your application from web browsers or other apps.
*   **Smart Argument Parsing**: Automatically extracts the relevant data from URI links (e.g., `mygame://join?id=123` -> `join?id=123`), simplifying Blueprint logic.
*   **Blueprint Support**: Easy-to-use Subsystem with event bindings for handling redirects.
*   **Configurable**: Toggle functionality and customize settings via Project Settings.

## Installation

1.  Copy the `InstanceDirector` folder to your project's `Plugins` directory.
2.  Open your project in the Unreal Editor.
3.  Go to **Edit > Plugins**.
4.  Search for **Instance Director** and enable it.
5.  Restart the editor.

## Usage

### 1. Configuration

Go to **Project Settings > Game > Instance Director**:

*   **Enable Single Instance Check**: Toggle the single-instance check on/off.
*   **Port Number**: Set the TCP port used for instance detection (Default: `64321`).
*   **Deep Linking Settings**:
    *   **URI Scheme**: Your custom protocol (e.g., `mygame`). Do not include `://`.
    *   **Register URI Scheme On Startup**: If checked, the app will automatically register the scheme in the Windows Registry on launch.

### 2. Handling Redirects in Blueprints

To respond to deep links or new instance launches:

1.  Open your **Game Instance** Blueprint.
2.  Get the **Instance Director Subsystem** node.
3.  Bind an event to **On App Redirected**.
4.  **Crucial**: Call **Check Startup Arguments** immediately after binding (e.g., in `Event Init`) to handle cold starts (launching via link when app is closed).

**Blueprint Graph Example:**
`Event Init` -> `Bind Event to OnAppRedirected` -> `Check Startup Arguments`

**Event Logic:**
The `OnAppRedirected` event provides an `Arguments` string.
*   If launched via `mygame://lobby/123`, the string will be `lobby/123`.
*   You can parse this string to open the specific lobby or menu.

### 3. Testing Deep Linking

1.  Configure your **URI Scheme** in Project Settings (e.g., `mygame`).
2.  Package your project (Windows).
3.  Run the packaged executable **once** to register the scheme in Windows.
4.  Open a web browser and type `mygame://test`.
5.  Your application should launch (or focus if running) and receive `test` as the argument.

## Technical Details

*   **Communication**: Uses a local TCP socket (localhost) to detect instances and pass data.
*   **Platform Support**: Windows (Primary).
*   **Registry**: Writes to `HKCU\Software\Classes\<Scheme>` for URI registration.
