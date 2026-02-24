# Advanced Port Scanner Design Document

## 1. Architectural Overview
### Platform Abstraction Layer
- Header Shroud pattern using #ifdef directives
- Winsock2 (Windows) / BSD Sockets (Unix)
- Cross-platform socket wrapper API

### Main Components
1. Socket Manager
2. Scan Engine
3. ThreadPool Banner Grabber
4. Output/Logging Module
5. Stealth Modules
6. Advanced OS Fingerprinting
7. Decoy System
8. Adaptive Rate Limiting
9. Npcap Integration (Windows)

## 2. Socket Management
### Non-blocking Sockets
- setsockopt(SO_NONBLOCK)
- select()/poll() for multiplexing

### Platform-specific Socket Initialization
```c
#ifdef _WIN32
    // Winsock initialization with Npcap
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#else
    // BSD socket initialization
#endif
```

## 3. Scan Engine Design

### TCP SYN (Stealth) Scan
- Raw sockets for sending custom TCP SYN packets
- Listening for SYN-ACK responses
- Requires raw socket privileges on Unix

### Connect Scan
- Standard connect() approach
- Faster than SYN scan but more detectable
- Simpler implementation

## 4. Stealth Modules
### Packet Fragmentation and Source Port Manipulation
```c
struct StealthConfig {
    int fragment_size;
    uint16_t source_port;
};
```
- Spoofing port 53 for evasion
- Fragment packets to bypass firewall rules

## 5. Advanced OS Fingerprinting
### Implementation Details
```c
struct OSFingerprint {
    int window_size;
    int ttl_value;
};
```
- Database integration (e.g., nmap-os-db)
- Analyze TCP window sizes and TTL values

## 6. Decoy System Design
### Strategy
- Spoof packets from multiple decoy IPs
- Rotate source IP addresses during scans
- Randomize timing between decoy responses

## 7. Adaptive Rate Limiting
### Algorithm
- Monitor RTT (Round Trip Time) for each port
- Adjust sending rate dynamically
- Prevent network congestion

## 8. ThreadPool Design
### Cross-platform Threading API
```c
struct ThreadPool {
   #ifdef _WIN32
        HANDLE* threads;
    #else
        pthread_t* threads;
    #endif
    int thread_count;
    Queue* task_queue;
};
```

### Task Types
1. Banner Grabbing
2. Service Fingerprinting
3. Additional port info collection

## 9. Npcap Integration (Windows)
- Required for raw packet injection
- Configure build system to link with npcap_dll
- Cross-platform wrapper functions for raw socket operations

## 10. External Database Requirements
### OS Signature Database
```json
{
    "os": {
        "windows_2019": {
            "ttl": 128,
            "window_size": 65535
        },
        "linux_debian_11": {
            "ttl": 64,
            "window_size": 4128
        }
    }
}
```
- Database location: ./data/os_signatures.json
- Regular updates via script

## 11. Configuration Options
```json
{
    "scan_type": "syn|connect",
    "threads": 10,
    "timeout": 3,
    "ports": "22,443,80",
    "stealth": {
        "fragment_size": 64,
        "source_port": 53
    },
    "decoy_ips": [
        "1.2.3.4",
        "5.6.7.8"
    ],
    "adaptive_rate": {
        "base": 100,
        "max": 500
    }
}
```

## 12. Cross-platform Compatibility
### Windows (Winsock2)
- Npcap integration for raw socket support
- Specific initialization/shutdown steps

### Unix (BSD Sockets)
- Raw socket permissions
- Signal handling

## 13. Testing Strategy
1. Unit Tests per module
2. Integration testing
3. Performance benchmarking
4. Cross-platform verification
5. Stealth feature validation
6. OS fingerprint accuracy tests

## 14. Development Roadmap
Phase 1: Platform Abstraction Layer  
Phase 2: Scan Engine Implementation  
Phase 3: ThreadPool Banner Grabber  
Phase 4: Stealth & Decoy Systems Integration  
Phase 5: OS Fingerprinting Database Integration  
Phase 6: Adaptive Rate Limiting Algorithm  
Phase 7: Final Testing & Optimization  
Phase 8: Documentation & Release

End of Design Document
```