@echo off
powershell -NoLogo -NoProfile -Command ^
  "$root='rc_client_browser'; $relative='..\rc_client_browser';" ^
  "$base=(Get-Item $root).FullName;" ^
  "$files=Get-ChildItem -Path $root -Recurse | Where-Object { -not $_.PSIsContainer -and ($_.FullName -notmatch '\\\.vscode\\') -and ($_.Name -notmatch '^\.') };" ^
  "Write-Host '<ItemGroup>'; foreach ($f in $files) {" ^
  "  $full=$f.FullName;" ^
  "  $path=$full.Substring($base.Length+1);" ^
  "  $xmlPath=$path -replace '/', '\\';" ^
  "  if ($xmlPath.StartsWith('\')) { $xmlPath = $xmlPath.Substring(1) }" ^
  "  $link=$path -replace '\\', '/';" ^
  "  Write-Host ('  <EmbeddedResource Include=\"' + $relative + '\' + $xmlPath + '\">');" ^
  "  Write-Host ('    <Link>' + $path + '</Link>');" ^
  "  Write-Host '  </EmbeddedResource>';" ^
  "} Write-Host '</ItemGroup>'"
