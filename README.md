# \*\*Hash Frying\*\* is an environment-keyed execution technique that uses GPU-mined hash collisions against native OS binaries to reconstruct zero-signature payloads at runtime.

# 

# This repository serves as a proof-of-concept for evading modern machine-learning EDRs and static analysis engines by completely destroying payload signatures and obfuscating the execution pipeline through statistical dilution.

# 

# \## Architecture

# 

# This technique relies on two distinct phases:

# 

# \### 1. The Chisel (GPU Mining)

# Rather than embedding encrypted shellcode, the payload is "mined" from the host environment.

# \* \*\*Target:\*\* `C:\\\\Windows\\\\System32\\\\ntdll.dll` (The entropy source).

# \* \*\*Mechanism:\*\* A custom CUDA kernel utilizes SHA-512 midstate caching on high-end GPUs to brute-force integer nonces. When hashed alongside the `ntdll.dll` bytes, the resulting hash contains a fragment of the desired shellcode (e.g., MSFvenom `calc.exe`).

# \* \*\*Output:\*\* A "Keybook" array of `{nonce, offset, length}` chunks.

# 

# \### 2. The Ghost (Runtime Reconstruction)

# The C++ loader contains absolutely zero malicious shellcode. It contains only the Keybook and the logic to rebuild the payload at runtime.

# \* \*\*Dynamic API Resolution:\*\* Memory management and execution APIs (`VirtualAlloc`, `VirtualProtect`, `CreateThread`, `WaitForSingleObject`) are dynamically resolved via `GetProcAddress`. This completely strips the Import Address Table (IAT) and blinds static heuristic models.

# \* \*\*Statistical Dilution (API Stuffing):\*\* The loader begins with a "Decoy Initialization" phase—executing interdependent, benign system queries (e.g., UI checks, Registry reads, RAM validation). This forces the ML engine to classify the binary as a standard enterprise administrative tool, shifting the mathematical weight of the executable away from the "malicious loader" cluster.

# \* \*\*Execution:\*\* A clean thread is dynamically created to detonate the reconstructed memory block, bypassing stack misalignment issues common with OS callback execution (e.g., `EnumSystemLocalesA`).

# 

# \## Evasion Results

# Against 71 enterprise security engines on VirusTotal:

# \* \*\*Static Signatures:\*\* 0 detections. The MSFvenom payload signature is mathematically destroyed.

# \* \*\*Without API Stuffing:\*\* 2/71 (Flagged only by hyper-aggressive ML heuristics looking at IAT anomalies and high basic-block density).

# \* \*\*With API Stuffing:\*\* Functional FUD (Fully Undetected).

# 

# 

# \### Prerequisites

# \* Windows 10/11 Target Environment

# \* Visual Studio 2022 (x64 Native Tools Command Prompt)

# \* NVIDIA RTX 3090 (or equivalent) for reasonable mining times

# 

# 

