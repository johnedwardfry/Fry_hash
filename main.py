import hashlib
import json
import os
from concurrent.futures import ProcessPoolExecutor


def get_local_entropy(file_path):
    """
    CONCEPT: ENTROPY MAPPING & STABILITY SAMPLING
    Targeting 'ntdll.dll' as a high-stability artifact. This file acts as a
    pre-distributed One-Time Pad that is cryptographically identical across
    all machines on this specific Windows build.
    """
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"Entropy source not found: {file_path}")
    with open(file_path, "rb") as f:
        return f.read(5120)


def chunk_payload(payload_string, chunk_size=5):
    """
    CONCEPT: COMPUTATIONAL PROOF-OF-WORK (PoW)
    Using a 5-character chunk size (10 hex digits) creates a search space
    of ~1 trillion combinations per fragment. This serves as a 'Slow-Burn'
    barrier, making the cost of reverse-engineering astronomical for analysts.
    """
    return [payload_string[i:i + chunk_size] for i in range(0, len(payload_string), chunk_size)]


def check_nonce_range(seed, target_hex, start_nonce, end_nonce):
    """
    CONCEPT: CRYPTOGRAPHIC DATA COMPRESSION
    We are not storing the payload; we are storing the 'coordinates' (Nonce/Offset)
    of where these instructions occur naturally within the OS noise.
    """
    for nonce in range(start_nonce, end_nonce):
        hasher = hashlib.sha512()
        hasher.update(seed + str(nonce).encode())
        res_hash = hasher.hexdigest()

        if target_hex in res_hash:
            return {"nonce": nonce, "offset": res_hash.find(target_hex)}
    return None


def mine_sequence(source_path, target_list, output_file="keybook.json"):
    """
    APPLICATION: ENVIRONMENTAL KEY GENERATION (EKG)
    The resulting 'Keybook' is a mathematical map that is only valid if the
    underlying OS entropy remains unchanged.
    """
    seed = get_local_entropy(source_path)
    keybook = []
    num_workers = os.cpu_count() or 4

    for target in target_list:
        target_hex = target.encode().hex()
        found, current_start, batch_size = False, 0, 1000000  # Optimized for i9-10850K
        with ProcessPoolExecutor(max_workers=num_workers) as executor:
            while not found:
                futures = [executor.submit(check_nonce_range, seed, target_hex,
                                           current_start + (i * batch_size),
                                           current_start + ((i + 1) * batch_size)) for i in range(num_workers)]
                for future in futures:
                    res = future.result()
                    if res:
                        keybook.append({"t": target, "n": res["nonce"], "o": res["offset"], "l": len(target_hex)})
                        found = True
                        break
                current_start += (num_workers * batch_size)

    with open(output_file, 'w') as f:
        json.dump({"s": source_path, "keys": keybook}, f, indent=4)