# ==============================================================================
# 🚨 ЖЕСТКАЯ ПОМЕТКА: ДАННЫЙ СКРИПТ ДОЛЖЕН НАХОДИТЬСЯ И ВЫПОЛНЯТЬСЯ СТРОГО 
# В КОРНЕВОЙ ПАПКЕ ПРОЕКТА: D:\Projects\Game_Project
# ==============================================================================

$REQUIRED_PATH = "D:\Projects\Game_Project"
if ($PWD.Path.ToLower().TrimEnd('\') -ne $REQUIRED_PATH.ToLower().TrimEnd('\')) {
    Write-Host "❌ КРИТИЧЕСКАЯ ОШИБКА: Скрипт запущен вне целевой папки проекта!" -ForegroundColor Red
    Write-Host "Текущий путь: $($PWD.Path)" -ForegroundColor Yellow
    Exit
}

$TXT_FILE = "all_project_source.txt"
$PDF_FILE = "all_project_source.pdf"

if (Test-Path $TXT_FILE) { Remove-Item $TXT_FILE }
if (Test-Path $PDF_FILE) { Remove-Item $PDF_FILE }

Write-Host "[1/2] Сканирование репозитория Game_Project и сборка текстового буфера..." -ForegroundColor Cyan

$ExcludedAssetPaths = @(
    "Game\assets\bethesda_fallout4",
    "Game\assets\bethesda_fallout76",
    "Game\assets\licensed",
    "Game\assets\local_override",
    "Game\assets\placeholder"
)

Get-ChildItem -Recurse | Where-Object { $_.FullName -notmatch '\\(build|\.git|\.vscode)($|\\)' } | ForEach-Object {
    $item = $_
    $isAssetFile = $false
    
    foreach ($path in $ExcludedAssetPaths) {
        if ($item.FullName -like "*$path*") {
            $isAssetFile = $true
            break
        }
    }

    $relativePath = $item.FullName.Replace("$REQUIRED_PATH\", "")
    $header = "`r`n| Название файла: $($item.Name) |`r`n| Путь в проекте: $relativePath |"
    $line   = "________________________________________________________________________________"
    
    if ($item.PsIsContainer) { return }

    if ($isAssetFile) {
        if ($item.Extension -eq ".ba2" -or $item.Extension -eq ".txt") {
            $header | Out-File -FilePath $TXT_FILE -Append -Encoding utf8
            $line   | Out-File -FilePath $TXT_FILE -Append -Encoding utf8
            "| [РЕСУРСНЫЙ АРХИВ]: Название: $($item.Name) |" | Out-File -FilePath $TXT_FILE -Append -Encoding utf8
            "`r`n--следующий файл--" | Out-File -FilePath $TXT_FILE -Append -Encoding utf8
        }
    }
    else {
        if ($item.Extension -in @(".cpp", ".h", ".hpp", ".inl", ".md") -or $item.Name -eq "CMakeLists.txt") {
            $header | Out-File -FilePath $TXT_FILE -Append -Encoding utf8
            $line   | Out-File -FilePath $TXT_FILE -Append -Encoding utf8
            Get-Content $item.FullName | Out-File -FilePath $TXT_FILE -Append -Encoding utf8
            "`r`n--следующий файл--" | Out-File -FilePath $TXT_FILE -Append -Encoding utf8
            Write-Host " Текст считан: $relativePath" -ForegroundColor Green
        }
    }
}

Write-Host "[2/2] Экспорт текстового буфера в PDF через headless-печать Edge..." -ForegroundColor Cyan

# Используем Microsoft Edge для гарантированной тихой печати TXT в PDF на любой Windows 10/11
$EdgePath = "${env:ProgramFiles(x86)}\Microsoft\Edge\Application\msedge.exe"
if (-not (Test-Path $EdgePath)) {
    $EdgePath = "${env:ProgramFiles}\Microsoft\Edge\Application\msedge.exe"
}

if (Test-Path $EdgePath) {
    # Запуск Microsoft Edge в фоновом режиме для сохранения текстового буфера в PDF
    $AbsoluteTxt = Resolve-Path $TXT_FILE
    $AbsolutePdf = "$PWD\$PDF_FILE"
    Start-Process -FilePath $EdgePath -ArgumentList "--headless", "--print-to-pdf=`"$AbsolutePdf`"", "`"$AbsoluteTxt`"" -Wait
    Write-Host "Успешно! PDF-архив структуры сформирован: $PDF_FILE" -ForegroundColor Yellow
} else {
    Write-Host "⚠️ Предупреждение: Браузер Edge не найден. Текстовый буфер сохранен в: $TXT_FILE" -ForegroundColor Yellow
}
