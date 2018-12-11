Add-Type -AssemblyName System.IO.Compression.FileSystem



$FFmpegUrl = "https://ffmpeg.zeranoe.com/builds/win64/static/ffmpeg-20180905-ad9b4ec-win64-static.zip";
$ThisCommand = $MyInvocation.MyCommand.Path;
$ThisDir = (Get-Item $ThisCommand ).Directory.FullName;
$TempFile = $ThisDir + "/ffmpeg.zip";
$DebugDir = $ThisDir + "/vs2015/Debug/";
$ReleaseDir = $ThisDir + "/vs2015/Release/";
$SourceDir = $ThisDir + "/ffmpeg-20180905-ad9b4ec-win64-static/bin/ffmpeg.exe";
$FfmpegDir = $ThisDir + "/ffmpeg-20180905-ad9b4ec-win64-static/";
$OutputDir = $ThisDir;


Write-Host Downloading ffmpeg zip...;
Invoke-WebRequest -Uri $FFmpegUrl -OutFile $TempFile;

Write-Host Extracting ffmpeg zip...
[System.IO.Compression.ZipFile]::ExtractToDirectory($TempFile, $OutputDir);

Write-Host Copying to Debug and Release dirs...;
if(!(Test-Path -Path $DebugDir )){
    New-Item -ItemType directory -Path $DebugDir
}
if(!(Test-Path -Path $ReleaseDir )){
    New-Item -ItemType directory -Path $ReleaseDir
}
Copy-Item -Path $SourceDir -Destination $DebugDir
Copy-Item -Path $SourceDir -Destination $ReleaseDir

Write-Host Removing zip file...;
Remove-Item $TempFile;
Remove-Item -Recurse $FfmpegDir;



Write-Host Donezo!;

