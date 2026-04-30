import json
import os


def generate_cpp_array(json_file="gpu_shellcode_keybook.json"):
    if not os.path.exists(json_file):
        print(f"[!] Error: Could not find {json_file}")
        return

    with open(json_file, 'r') as f:
        data = json.load(f)

    keys = data.get("keys", [])
    if not keys:
        print("[!] No keys found in the JSON file.")
        return

    print("[*] Extraction successful. Copy and paste the following into reconstructor.cpp:\n")
    print("    std::vector<KeybookEntry> keybook = {")

    for i, key in enumerate(keys):
        # Format: {nonce, offset, length}
        line = f"        {{{key['n']}, {key['o']}, {key['l']}}}"

        # Add a comma to all entries except the very last one
        if i < len(keys) - 1:
            line += ","

        print(line)

    print("    };")
    print(f"\n[*] Total chunks extracted: {len(keys)}")


if __name__ == '__main__':
    generate_cpp_array()