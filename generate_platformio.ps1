
$ErrorActionPreference = "Stop"

$reportPath = Join-Path $PWD "esp32_report.txt"
$outputPath = Join-Path $PWD "platformio.generated.ini"

try {
    if (!(Test-Path $reportPath)) {
        throw "Report not found: $reportPath"
    }

    $report = Get-Content $reportPath -Raw

    # Identify the chip family.
    if ($report -match 'Chip is ESP32-S3') {
        $board = "esp32-s3-devkitm-1"
    }
    elseif ($report -match 'Chip is ESP32-C3') {
        $board = "esp32-c3-devkitm-1"
    }
    elseif ($report -match 'Chip is ESP32-D0WD|Chip is ESP32-D0WD-V3|Chip is ESP32 \(revision') {
        $board = "esp32dev"
    }
    else {
        throw "Unknown chip. Cannot safely select a PlatformIO board."
    }

    # Read flash size.
    if ($report -match 'Detected flash size:\s*(\d+)\s*(MB|KB)') {
        $flashSize = "$($Matches[1])$($Matches[2])"
    }
    else {
        throw "Flash size not found in report."
    }

    $ini = @"
[env:RONGO_node]
platform = espressif32
board = $board
framework = arduino

board_upload.flash_size = $flashSize
board_build.partitions = default.csv

monitor_speed = 115000
"@

    Set-Content -Path $outputPath -Value $ini -Encoding utf8

    Write-Host ""
    Write-Host "PlatformIO configuration generated!" -ForegroundColor Green
    Write-Host "Chip board: $board"
    Write-Host "Flash size: $flashSize"
    Write-Host "Output: $outputPath"
}
catch {
    Write-Host ""
    Write-Host "ERROR: $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""
Read-Host "Press Enter to close"
