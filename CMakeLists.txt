# Step 1: Initialize the build system
cmake_minimum_required(VERSION 3.10)
project(GhostScanner C)

# Step 2: Set C99 standard for systems programming
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Step 3: Define the executable
add_executable(GhostScanner src/main.c)

# Step 4: Fix the 'WinMain' error
# This explicitly tells the linker to build a Console application, not a GUI one.
if(WIN32)
    set_target_properties(GhostScanner PROPERTIES LINK_FLAGS "-mconsole")
endif()

# Step 5: Include headers
target_include_directories(GhostScanner PRIVATE include)

# Step 6: Platform-specific library linking
if(WIN32)
    # Search for the Windows Sockets library with MSYS2 hints
    find_library(WS2_32_LIBRARY 
        NAMES ws2_32 libws2_32 
        HINTS "C:/msys64/mingw64/x86_64-w64-mingw32/lib" "C:/msys64/mingw64/lib"
        REQUIRED
    )
    target_link_libraries(GhostScanner PRIVATE ${WS2_32_LIBRARY})
    
    # Optional: Logic for Npcap if you install the SDK later
    find_path(NPCAP_INCLUDE_DIR pcap/pcap.h HINTS "C:/Program Files/Npcap/include")
    if(NPCAP_INCLUDE_DIR)
        target_include_directories(GhostScanner PRIVATE ${NPCAP_INCLUDE_DIR})
        find_library(NPCAP_LIBRARY pcap HINTS "C:/Program Files/Npcap/lib/x64")
        if(NPCAP_LIBRARY)
            target_link_libraries(GhostScanner PRIVATE ${NPCAP_LIBRARY})
        endif()
    endif()
else()
    # Unix-like systems linking
    find_package(Threads REQUIRED)
    target_link_libraries(GhostScanner PRIVATE Threads::Threads)
endif()