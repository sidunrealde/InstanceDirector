# Instance Director - Single Instance & Deep Linking Solution

**Instance Director** is the ultimate solution for managing application instances in Unreal Engine. Whether you are building a game, an enterprise application, or a tool, ensuring a single running instance and handling external links (Deep Linking) is crucial for a professional user experience.

## Why Instance Director?

Have you ever clicked a "Join Game" link in a browser, only to have it launch a *second* copy of the game instead of joining the running one? **Instance Director solves this.**

It detects if your application is already running. If it is, it brings the existing window to the front and passes the link data directly to it. If not, it launches the app and handles the link seamlessly.

## Key Features

*   **🛡️ Single Instance Enforcement**: robustly prevents multiple instances using TCP socket binding.
*   **🔗 Deep Linking / URI Schemes**: Register custom protocols like `mygame://` directly from Project Settings.
*   **⚡ Automatic Redirection**: Instantly focuses the running application window when a user tries to launch it again.
*   **🧠 Smart Argument Parsing**: Automatically strips the protocol (e.g., `mygame://`) and delivers clean data (e.g., `lobby/123`) to your Blueprints.
*   **🔌 Blueprint Ready**: Zero C++ required. Use the `Instance Director Subsystem` to handle events in your Game Instance.
*   **🛠️ Easy Configuration**: Set up ports, URI schemes, and toggles directly in Project Settings.

## How It Works

1.  **Detection**: On startup, the plugin checks for an existing instance.
2.  **Communication**: If found, it sends the command-line arguments (like your deep link) to the running instance via a local socket.
3.  **Focus**: It forces the running instance to the foreground (handling Windows focus restrictions).
4.  **Event**: It fires a Blueprint event with the data, allowing you to trigger logic like "Join Lobby" or "Open Store".

## Perfect For
*   Multiplayer games with web-based server browsers.
*   Applications requiring "Open in App" functionality.
*   Tools and utilities that must run as a singleton.

## Platform Support
*   **Target Platforms**: Windows (64-bit)
*   **Engine Versions**: 5.0 - 5.5+
