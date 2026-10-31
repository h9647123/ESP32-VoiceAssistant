[CmdletBinding()]
param(
  [ValidateSet('esp32s3')][string]$Target = 'esp32s3',
  [string]$PortName = 'COM5',
  [string]$Sketch = 'VoiceAssistant',
  [string]$BuildPath = 'validation\.build-release',
  [switch]$Upload,
  [switch]$SkipCompile
)

# Canonical build for this project.
#
# PSRAM=opi is REQUIRED and must not be dropped: this module carries 8 MB of
# embedded OPI PSRAM, and without it mbedTLS cannot complete a TLS handshake
# once a real chat session is loaded ("BIGNUM - Memory allocation failed").
# The visible symptom is every HTTPS/WSS call failing intermittently, which
# looks exactly like a broken network or an expired API key. Keep it here so a
# plain `compile` can never silently reproduce that failure mode.
#
# PartitionScheme=huge_app is also required; the USBMode=hwcdc/CDCOnBoot=cdc
# combination recorded in older notes silences the serial console completely.

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$fqbn = "esp32:esp32:${Target}:PartitionScheme=huge_app,PSRAM=opi,FlashSize=16M,USBMode=hwcdc,CDCOnBoot=default"

Write-Host "FQBN: $fqbn"

if (-not $SkipCompile) {
  & arduino-cli compile --fqbn $fqbn --build-path $BuildPath $Sketch
  if ($LASTEXITCODE -ne 0) { throw "compile failed ($LASTEXITCODE)" }
}

if ($Upload) {
  & arduino-cli upload -p $PortName --fqbn $fqbn --build-path $BuildPath $Sketch
  if ($LASTEXITCODE -ne 0) { throw "upload failed ($LASTEXITCODE)" }
  Write-Host "Flashed to $PortName."
}
