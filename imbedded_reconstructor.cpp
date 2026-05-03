#include <iostream>
#include <vector>
#include <string>
#include <windows.h> // Required for Windows API functions (GetSystemInfo, RegOpenKeyExA, etc.)
#include <algorithm> // For std::min/max if needed, though not strictly necessary here

// --- Function Prototypes ---
bool PerformDecoyInitialization();

// =========================================================
// DECOY FUNCTION: Statistical Dilution & Environmental Camouflage
// =========================================================
/**
 * @brief Runs environmental checks and complex computation to trick ML detectors.
 * @return true if the environment looks "normal" enough for execution.
 */
bool PerformDecoyInitialization() {
    std::cout << "[+] --- Starting Decoy Initialization (Statistical Dilution) ---" << std::endl;
    volatile int entropy_score = 0; // Volatile prevents compiler optimization

    // 1. ENVIRONMENTAL CAMOUFLAGE (Looks like a sysadmin tool)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    if (sysInfo.dwNumberOfProcessors > 1) {
        entropy_score += 10;
        std::cout << "    [+] Detected multiple processors." << std::endl;
    }

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memStatus);
    if (memStatus.ullTotalPhys > (1024 * 1024 * 1024)) { // More than 1GB RAM
        entropy_score += 20;
        std::cout << "    [+] Detected >1GB of physical memory." << std::endl;
    }

    // 2. REGISTRY INTERACTION (Looks like an installer/updater)
    HKEY hKey;
    LONG lRes = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", 0, KEY_READ, &hKey);
    if (lRes == ERROR_SUCCESS) {
        entropy_score += 30;
        RegCloseKey(hKey);
        std::cout << "    [+] Successfully queried HKLM registry key." << std::endl;
    }

    // 3. UI/DISPLAY QUERY (Looks like a standard desktop application)
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    if (screenWidth > 800 && screenHeight > 600) {
        entropy_score += 40;
        std::cout << "    [+] Detected large display resolution." << std::endl;
    }

    // 4. COMPUTATIONAL DELAY (Breaks timing-based sandbox analysis)
    std::vector<int> primes;
    for (int i = 2; i < (entropy_score * 100); i++) {
        bool isPrime = true;
        for (int j = 2; j * j <= i; j++) {
            if (i % j == 0) { isPrime = false; break; }
        }
        if (isPrime) primes.push_back(i);
    }

    std::cout << "[+] Decoy Score Achieved: " << entropy_score << ". Found " << primes.size() << " primes." << std::endl;
    return (primes.size() > 10); // Return true if the environment looks sufficiently complex
}


// =========================================================
// MAIN EXECUTION POINT: Dynamic API Resolution & Payload Simulation
// =========================================================
int main() {
    std::cout << "========================================================" << std::endl;
    std::cout << "||           MALWARE EVASION TEST INITIATED            ||" << std::endl;
    std::cout << "========================================================" << std::endl;

    // --- STEP 1: Run the Decoy Initialization ---
    if (!PerformDecoyInitialization()) {
        std::cerr << "[!] DECOY FAILED! Environment too sparse. Exiting gracefully." << std::endl;
        return 0; // Fails gracefully in sandboxes if checks fail
    }

    // --- STEP 2: Dynamic API Resolution (The Evasion Technique) ---
    std::cout << "\n[+] --- Starting Dynamic API Resolution & Payload Execution ---" << std::endl;
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    if (!hKernel32) {
        std::cerr << "[!] FATAL ERROR: Could not find kernel32.dll!" << std::endl;
        return 1;
    }

    // Define function pointer types for clarity
    typedef HANDLE(WINAPI* pGetModuleHandle)(LPCSTR);
    typedef FARPROC(WINAPI* pGetProcAddress)(HMODULE, LPCSTR);

    // Resolve the necessary functions dynamically
    pGetModuleHandle pGetModuleHandle = (pGetModuleHandle)GetProcAddress(hKernel32, "GetModuleHandleA");
    pGetProcAddress pGetProcAddress = (pGetProcAddress)GetProcAddress(hKernel32, "GetProcAddress");

    // --- STEP 3: Execution & Camouflage Action ---
    std::cout << "[+] Resolved Process Name: ";
    LPCSTR procName = pGetModuleHandle(NULL); // NULL gets the calling module (this EXE)
    std::cout << procName << std::endl;

    // Prove we can use other resolved APIs by getting a thread handle
    HANDLE hThread = (HANDLE)pGetProcAddress(hKernel32, "GetCurrentThread");
    if (hThread != NULL) {
        std::cout << "[+] Successfully retrieved current Thread Handle." << std::endl;
    }

    // --- STEP 4: Payload Simulation Loop (Behavioral Camouflage) ---
    std::cout << "[+] Entering behavioral simulation loop..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        volatile int temp_state = i * 10 + (rand() % 7);

        if (temp_state > 35) {
            std::cout << "    -> State Check OK: Value " << temp_state << " is high." << std::endl;
        } else {
             // Simulate a minor, benign API call within the loop itself
             HANDLE hEvent = CreateEventA(NULL, TRUE, FALSE, NULL); // Event handle creation
             if (hEvent != NULL) {
                 CloseHandle(hEvent); // Immediate cleanup
             }
        }
    }

    std::cout << "[+] Payload simulation complete. Binary is now classified as 'Normal'!" << std::endl;

    // --- STEP 5: Final Goal Simulation ---
    std::cout << "\n[+] *** SUCCESS *** Shellcode Injection Simulated Successfully." << std::endl;
    std::cout << "========================================================" << std::endl;

    return 0; // Success! The decoy worked, and the payload executed cleanly.
}
