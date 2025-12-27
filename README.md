# Instance Director

**Instance Director** is an Unreal Engine plugin that ensures only one instance of your application runs at a time. It handles single-instance enforcement, automatic window focusing, and app redirection (deep linking) by passing command-line arguments from new instances to the running one.

## Features

*   **Single Instance Enforcement**: Prevents multiple copies of the application from running simultaneously.
*   **Automatic Redirection**: Automatically brings the existing instance to the foreground when a user attempts to launch a second one.
*   **Deep Linking Support**: Passes command-line arguments from the new instance to the running instance, allowing you to trigger specific actions (e.g., joining a lobby, opening a menu).
*   **URI Scheme Registration**: Easily register custom URI schemes (e.g., `myapp://`) to launch your app from web browsers.
*   **Blueprint Support**: Easy-to-use Blueprint Subsystem to handle redirect events.
*   **Configurable**: Toggle functionality and customize the listening port via Project Settings.

## Installation

1.  Copy the `InstanceDirector` folder to your project's `Plugins` directory.
2.  Open your project in the Unreal Editor.
3.  Go to **Edit > Plugins**.
4.  Search for **Instance Director** and enable it.
5.  Restart the editor.

## Usage

### Configuration

Go to **Project Settings > Game > Instance Director** to configure the plugin:

*   **Enable Single Instance Check**: Turn the single-instance check on or off.
*   **Port Number**: Set the TCP port used for instance detection (Default: `64321`).

### Handling Redirects in Blueprints

To respond to a new instance being launched (e.g., for deep linking):

1.  Open your **Game Instance** Blueprint.
2.  Get the **Instance Director Subsystem** node (from `Get Game Instance`).
3.  Bind an event to **On App Redirected**.
4.  The event provides an `Arguments` string containing the command-line arguments passed to the new instance.

**Example:**
If you launch your app with `MyApp.exe -OpenStore`, the `Arguments` string will contain `-OpenStore`. You can parse this string to open the store menu in your running instance.

### Setting up Web Links (URI Scheme)

To allow your app to be opened via web links (e.g., `myapp://lobby/123`):

1.  In your **Game Instance** (Event Init), call **Register URI Scheme** from the Instance Director Subsystem.
2.  Pass your desired scheme name (e.g., `myapp`).
3.  Package your project and run it once to register the scheme in Windows.

**Testing:**
1.  Open a web browser.
2.  Type `myapp://test` in the address bar and press Enter.
3.  Your application should launch (or focus if running).
4.  The `On App Redirected` event will fire with the argument `myapp://test`.

## Technical Details

*   **Communication**: Uses a local TCP socket (localhost) to detect instances and pass data.
*   **Platform Support**: Windows (Primary), Mac/Linux (Should work but untested).
*   **Engine Version**: Compatible with Unreal Engine 4.27+ and 5.x.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
