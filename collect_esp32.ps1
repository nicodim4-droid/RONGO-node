
$ErrorActionPreference = "Stop"

$python = Join-Path $env:USERPROFILE ".platformio\penv\Scripts\python.exe"
$esptool = Join-Path $env:USERPROFILE ".platformio\packages\tool-esptoolpy\esptool.py"

$port = Read-Host "COM port (default COM4)"
if ([string]::IsNullOrWhiteSpace($port)) { $port = "COM4" }

$reportPath = Join-Path $PWD "esp32_report.txt"

try {
    if (!(Test-Path $python)) { throw "PlatformIO Python not found" }
    if (!(Test-Path $esptool)) { throw "esptool.py not found" }

    $chipOutput = @(& $python $esptool --port $port chip_id 2>&1)
    if ($LASTEXITCODE -ne 0) { throw "Could not read chip information" }

    $flashOutput = @(& $python $esptool --port $port flash_id 2>&1)
    if ($LASTEXITCODE -ne 0) { throw "Could not read flash information" }

    $chipText = $chipOutput -join "`n"
    $flashText = $flashOutput -join "`n"

    $report = [System.Collections.Generic.List[string]]::new()

    $report.Add("===== CHIP INFO AND MAC =====")

    foreach ($pattern in @(
        'Chip is .+',
        'Features: .+',
        'Crystal is .+',
        'MAC: [0-9a-fA-F:]+'
    )) {
        $match = [regex]::Match($chipText, $pattern)
        if ($match.Success) { $report.Add($match.Value) }
    }

    $report.Add("===== FLASH IDENTIFICATION =====")

    foreach ($pattern in @(
        'Manufacturer: .+',
        'Device: .+',
        'Detected flash size: .+',
        'Flash voltage set by a strapping pin to .+'
    )) {
        $match = [regex]::Match($flashText, $pattern)
        if ($match.Success) { $report.Add($match.Value) }
    }

    $report | Out-File -FilePath $reportPath -Encoding utf8

    Write-Host ""
    Write-Host "Done. Report saved to: $reportPath" -ForegroundColor Green
}
catch {
    Write-Host "ERROR: $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""
Read-Host "Press Enter to close"
