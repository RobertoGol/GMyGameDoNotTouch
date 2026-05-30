# ==============================================================================
# 🚨 ЖЕСТКАЯ ПОМЕТКА: ДАННЫЙ СКРИПТ ДОЛЖЕН НАХОДИТЬСЯ И ВЫПОЛНЯТЬСЯ СТРОГО 
# В КОРНЕВОЙ ПАПКЕ ПРОЕКТА: D:\Projects\Game_Project
# ==============================================================================

# Проверка физического местоположения скрипта перед запуском
$REQUIRED_PATH = "D:\Projects\Game_Project"
if ($PWD.Path -ne $REQUIRED_PATH) {
    Write-Host "❌ КРИТИЧЕСКАЯ ОШИБКА: Скрипт запущен вне целевой папки проекта!" -ForegroundColor Red
    Write-Host "Текущий путь: $($PWD.Path)" -ForegroundColor Yellow
    Write-Host "Скрипт должен лежать строго в: $REQUIRED_PATH" -ForegroundColor Cyan
    Exit
}

# 1. Задаем пути итоговых файлов
$TXT_FILE = "all_project_source.txt"
$PDF_FILE = "all_project_source.pdf"

if (Test-Path $TXT_FILE) { Remove-Item $TXT_FILE }
if (Test-Path $PDF_FILE) { Remove-Item $PDF_FILE }

Write-Host "[1/2] Сканирование репозитория Game_Project и сборка текстового буфера..." -ForegroundColor Cyan

# Массив тяжелых папок ассетов, внутренности которых мы НЕ открываем как текст
$ExcludedAssetPaths = @(
    "Game\assets\bethesda_fallout4",
    "Game\assets\bethesda_fallout76",
    "Game\assets\licensed",
    "Game\assets\local_override",
    "Game\assets\placeholder"
)

# 2. Итерируем по всем элементам проекта, исключая тяжелый кэш сборщика
Get-ChildItem -Recurse | Where-Object { $_.FullName -notmatch '\\(build|\.git|\.vscode)($|\\)' } | ForEach-Object {
    $item = $_
    $isAssetFile = $false
    
    # Проверяем, принадлежит ли текущий файл к папкам исключений
    foreach ($path in $ExcludedAssetPaths) {
        if ($item.FullName -like "*$path*") {
            $isAssetFile = $true
            break
        }
    }

    # Вычисляем чистый путь от корня проекта Game_Project (заменяем корень на пустую строку)
    $relativePath = $item.FullName.Replace("$REQUIRED_PATH\", "")

    # ФОРМАТ ШАПКИ СТРОГО ПО ТВОЕМУ ШАБЛОНУ С УКАЗАНИЕМ НАЗВАНИЯ И ОТНОСИТЕЛЬНОГО ПУТИ
    $header = "`r`n| Название файла: $($item.Name) |`r`n| Путь в проекте: $relativePath |"
    $line   = "________________________________________________________________________________"
    
    if ($item.PsIsContainer) {
        # Пропускаем папки, работаем только с файлами
        return
    }

    if ($isAssetFile) {
        # Если это папка ассетов - записываем ТОЛЬКО название .ba2 файла без его внутренностей
        if ($item.Extension -eq ".ba2" -or $item.Extension -eq ".txt") {
            $header | Out-File -FilePath $TXT_FILE -Append -Encoding utf8
            $line   | Out-File -FilePath $TXT_FILE -Append -Encoding utf8
            "| [РЕСУРСНЫЙ АРХИВ]: Название: $($item.Name) |" | Out-File -FilePath $TXT_FILE -Append -Encoding utf8
            "`r`n--следующий файл--" | Out-File -FilePath $TXT_FILE -Append -Encoding utf8
        }
    }
    else {
        # Если это исходный код (C++, .md, CMake), фильтруем по расширениям и читаем содержимое полностью
        if ($item.Extension -in @(".cpp", ".h", ".hpp", ".inl", ".md") -or $item.Name -eq "CMakeLists.txt") {
            $header | Out-File -FilePath $TXT_FILE -Append -Encoding utf8
            $line   | Out-File -FilePath $TXT_FILE -Append -Encoding utf8
            
            # Стримим содержимое кода долями построчно (ОЗУ ноутбука не грузится)
            Get-Content $item.FullName | Out-File -FilePath $TXT_FILE -Append -Encoding utf8
            
            "`r`n--следующий файл--" | Out-File -FilePath $TXT_FILE -Append -Encoding utf8
            Write-Host " Текст считан: $relativePath" -ForegroundColor Green
        }
    }
}

Write-Host "[2/2] Экспорт текстового буфера в PDF через системный движок..." -ForegroundColor Cyan

# 3. Отправляем готовый текстовый буфер на системную печать Windows в PDF
if (Get-Process -Name "wordpad" -ErrorAction SilentlyContinue) { Stop-Process -Name "wordpad" }
Start-Process -FilePath "wordpad.exe" -ArgumentList "/p `"$PWD\$TXT_FILE`"" -NoNewWindow -Wait

Write-Host "Успешно! Все файлы структуры DX11_V1.32 и списки .ba2 упакованы в: $TXT_FILE" -ForegroundColor Gold
