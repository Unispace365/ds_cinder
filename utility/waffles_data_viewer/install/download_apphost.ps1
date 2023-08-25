Add-Type -AssemblyName System.IO.Compression.FileSystem

Add-Type -Language CSharp "
namespace System.Net {
public static class Util {
public static void Init() {
ServicePointManager.ServerCertificateValidationCallback = null;
ServicePointManager.ServerCertificateValidationCallback += (sender, cert, chain, errs) => true;
ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls | SecurityProtocolType.Tls11 | SecurityProtocolType.Tls12;
}}}"
[System.Net.Util]::Init()

$DsapphostVer = (Invoke-WebRequest -UseBasicParsing -Uri 'http://update.downstreamdev.com/ds/dsapphost/version.txt').Content;
Write-Host $DsapphostVer;


$DsapphostUrl = "http://update.downstreamdev.com/ds/dsapphost/" + $DsapphostVer;
$ThisCommand = $MyInvocation.MyCommand.Path;
$ThisDir = (Get-Item $ThisCommand ).Directory.FullName;
$TempFile = $ThisDir + "/Dsapphostr.zip";
$DsapphostDir = $ThisDir + "/DSAppHost/";
$OutputDir = $ThisDir;

if (Test-Path $DsapphostDir -PathType Container){
    Write-Host Removing old Dsapphost dir...;
    Remove-Item $DsapphostDir -Recurse;
}

Write-Host Downloading Dsapphost zip...;
Invoke-WebRequest -Uri $DsapphostUrl -OutFile $TempFile;

Write-Host Extracting Dsapphost zip...
[System.IO.Compression.ZipFile]::ExtractToDirectory($TempFile, $OutputDir);

Write-Host Removing zip file...;
Remove-Item $TempFile;

if (Test-Path $DsapphostDir -PathType Container){
    Write-Host Success!;
} else {
    Write-Host Uh oh, something went wrong!;
}


