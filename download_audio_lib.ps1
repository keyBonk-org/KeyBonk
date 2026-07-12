param(
    [string]$Repo,
    [string]$Arch,
    [string]$DownloadDir,
    [string]$TargetFile
)

$ErrorActionPreference = "Stop"

# 1. 创建下载目录
if (-not (Test-Path $DownloadDir)) {
    New-Item -ItemType Directory -Path $DownloadDir -Force | Out-Null
}

$zipPath = Join-Path $DownloadDir "release.zip"

# 2. 获取最新 Release 下载 URL
Write-Host "获取最新 Release 信息..." -ForegroundColor Cyan
$apiUrl = "https://api.github.com/repos/$Repo/releases/latest"
try {
    $response = Invoke-RestMethod -Uri $apiUrl -UseBasicParsing -ErrorAction Stop
} catch {
    Write-Error "无法获取 Release 信息: $_"
    exit 1
}

$pattern = if ($Arch -eq "x64") { "x64" } else { "x86" }
$asset = $response.assets | Where-Object { $_.name -match $pattern } | Select-Object -First 1
if (-not $asset) {
    Write-Error "未找到匹配 '$pattern' 的压缩包"
    exit 1
}

$downloadUrl = $asset.browser_download_url
Write-Host "下载链接: $downloadUrl" -ForegroundColor Gray

# 3. 下载（带重试）
$maxRetries = 3
$attempt = 0
$downloaded = $false
while (-not $downloaded -and $attempt -lt $maxRetries) {
    $attempt++
    Write-Host "下载尝试 $attempt/$maxRetries ..." -ForegroundColor Yellow
    try {
        Invoke-WebRequest -Uri $downloadUrl -OutFile $zipPath -UseBasicParsing -ErrorAction Stop
        $downloaded = $true
        Write-Host "下载成功" -ForegroundColor Green
    } catch {
        Write-Warning "下载失败 (尝试 $attempt): $_"
        if ($attempt -lt $maxRetries) {
            Write-Host "等待 2 秒后重试..." -ForegroundColor Gray
            Start-Sleep -Seconds 2
        }
    }
}

if (-not $downloaded) {
    Write-Error "下载失败，已达最大重试次数"
    exit 1
}

# 4. 验证 ZIP 完整性
Write-Host "验证 ZIP 文件完整性..." -ForegroundColor Cyan
Add-Type -AssemblyName System.IO.Compression.FileSystem -ErrorAction Stop
try {
    $zip = [System.IO.Compression.ZipFile]::OpenRead($zipPath)
    $zip.Dispose()
    Write-Host "ZIP 文件有效" -ForegroundColor Green
} catch {
    Write-Error "ZIP 文件损坏或无法读取: $_"
    exit 1
}

# 5. 解压
Write-Host "解压到 $DownloadDir ..." -ForegroundColor Cyan
try {
    Expand-Archive -Path $zipPath -DestinationPath $DownloadDir -Force -ErrorAction Stop
    Write-Host "解压完成" -ForegroundColor Green
} catch {
    Write-Error "解压失败: $_"
    exit 1
}

# 6. 删除 ZIP 文件
Remove-Item $zipPath -Force -ErrorAction SilentlyContinue

# 7. 验证目标库文件
Write-Host "验证库文件 $TargetFile ..." -ForegroundColor Cyan
if (-not (Test-Path $TargetFile)) {
    Write-Error "库文件不存在: $TargetFile"
    exit 1
}

# 使用 ar 检查格式（需在 PATH 中）
$ar = "ar.exe"
$test = & $ar t $TargetFile 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Error "库文件格式无效 (ar t 失败): $($test -join "`n")"
    exit 1
}

Write-Host "库文件验证通过" -ForegroundColor Green
Write-Host "所有操作完成！" -ForegroundColor Green
exit 0