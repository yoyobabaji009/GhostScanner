GhostScanner 👻
High-Performance, Cross-Platform TCP Port Scanner & Service Fingerprinter

🎯 Purpose: Why Created?
GhostScanner was developed to bridge the gap between slow, synchronous port scanners and complex industry tools like Nmap. In the context of Malware Analysis and Digital Forensics (DFIR), speed and accuracy are critical for incident response and attack surface mapping. This tool provides:

Efficiency: Identifying open attack surfaces in seconds by bypassing standard blocking I/O.

Intelligence: Moving beyond simple "Open/Closed" status to identify specific service versions (banners).

Portability: A single C codebase that compiles natively on both Windows and Linux without external libraries.

💻 The Tech Behind It: How it Works
The core engine relies on Socket Multiplexing rather than traditional multi-threading for its primary speed.

1. Non-Blocking I/O
Standard network sockets are "blocking," meaning the program pauses during the "Three-Way Handshake". GhostScanner sets sockets to Non-Blocking mode (FIONBIO on Windows / O_NONBLOCK on Linux), allowing the connect() call to return immediately so the scanner can initiate the next connection without waiting.

2. Multiplexing with select()
Instead of checking each socket individually, the tool uses the select() system call. This allows the Operating System to monitor an entire batch of sockets (up to 100 at a time) and notify the program only when a connection has been established or failed.

3. Service Fingerprinting
When a connection is successful, the tool performs a Banner Grab, reading the first few bytes of data sent by the service to identify the specific software (e.g., OpenSSH version, VMware headers, etc.).

🛠 Engineering Complications & Solutions
To ensure full transparency and technical integrity, the following hurdles were solved during development:

A. The "Ghost" Linker Error: WinMain vs. main
The Issue: Compiler errors regarding undefined reference to WinMain.

The Cause: Windows distinguishes between Console and GUI subsystems; if it defaults to GUI, it seeks WinMain.

The Fix: Forcing the MinGW Makefiles generator in CMake to ensure the Console subsystem is used.

B. Socket "Blocking" Bottleneck
The Issue: Scanning 1,000 ports would take minutes due to firewalled port timeouts (20+ seconds).

The Cause: Synchronous connect() calls wait for the full TCP handshake.

The Fix: Implementing non-blocking modes to initiate connections and move on immediately.

C. The select() Batching Limit
The Issue: Attempting to scan 65,535 ports at once causes OS-level process crashes.

The Cause: OS limits on "File Descriptors" and the hard-coded FD_SETSIZE limit (usually 64–512) in select().

The Fix: Implementing Batching (processing ports in groups of 100) to keep resource usage within OS limits.

🏗 Build & Installation
Prerequisites
CMake 3.10+

C Compiler (GCC/MinGW-w64)

Compilation
Generate the build environment:
cmake -G "MinGW Makefiles" -B build -S .

Compile the binary:
cmake --build build

Usage
Execute from the terminal:
./GhostScanner.exe <Target_IP> <Start_Port> <End_Port> [Optional_Log_File.txt]

🚀 Future Roadmap
Hybrid Concurrency: Combining select() with Multi-threading (pthreads) for parallel IP scanning.

Adaptive Timing: Implementing "Smart Timeouts" based on network latency (RTT).

Automation: Connecting output to n8n workflows for automated security reporting.
