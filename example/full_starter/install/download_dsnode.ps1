Add-Type -AssemblyName System.IO.Compression.FileSystem

$DsnodeUrl = "http://update.downstreamdev.com/ds/DsNode/dsnode.zip";
$ThisCommand = $MyInvocation.MyCommand.Path;
$ThisDir = (Get-Item $ThisCommand ).Directory.FullName;
$TempFile = $ThisDir + "/dsnoder.zip";
$DsnodeDir = $ThisDir + "/DSNode/";

if (Test-Path $DsnodeDir -PathType Container){
    Write-Host Removing old dsnode dir...;
    Remove-Item $DsnodeDir -Recurse;
}

Write-Host Downloading dsnode zip...;
Invoke-WebRequest -Uri $DsnodeUrl -OutFile $TempFile;

Write-Host Extracting dsnode zip...
[System.IO.Compression.ZipFile]::ExtractToDirectory($TempFile, $DsnodeDir);

Write-Host Removing zip file...;
Remove-Item $TempFile;

if (Test-Path $DsnodeDir -PathType Container){
    Write-Host Success!;
} else {
    Write-Host Uh oh, something went wrong!;
}
