param($secretFile,[switch]$unzip=$true,[switch]$keepZip=$false)
$ErrorActionPreference = "Stop"

Add-Type -AssemblyName System.IO.Compression.FileSystem
write-host " "

function get-latest-repo-release( [string]$gitPersonalAccessToken, [string]$owner, [string]$repo, [string]$dropFolder, $gitApiBase){
  if ('' -eq "$gitApiBase"){
    $gitApiBase = 'api.github.com'}
  $parms = @{uri = "https://$gitApiBase/repos/$owner/$repo/releases"}
  $headers = @{Accept = 'application/vnd.github.v3+json'}
  if ('' -ne "$gitPersonalAccessToken"){
    $headers['Authorization'] = "token $gitPersonalAccessToken"}
  $parms['headers'] = $headers
  $releases = iwr -UseBasicParsing @parms | ConvertFrom-Json 
  $latestTag = $releases | sort -Property published_at -Descending | select -first 1 | select -expandProperty tag_name
  $assets = $releases | ?{ $_.tag_name -eq $latestTag}  | select -first 1 | select -expandProperty assets
  $headers['Accept'] = 'application/octet-stream'
  $assets | ?{ $_.name -like "*windows*.zip"} | %{
    $parms['uri'] = $_.url
    $parms['outfile'] = "$dropFolder\$($_.name)"
    $last = "$dropFolder\$($_.name)"
    iwr @parms}
  $out = New-Object psobject @{
    path = "$last"
    version = $latestTag
  }
  $out
}


$documents = [environment]::getfolderpath("mydocuments")

#control properties
$defaultSecretFile = "$documents\secret"
$zipDropFolder = '.'
$unzipPath = '..\downsync\'
$owner = 'Downstream'
$repo = 'downsync'


#get the token
## if we have a value on the command line use that
## otherwise use the file "<Documents>\secret"
if([string]::IsNullOrEmpty($secretFile)){
    $secretFile = $defaultSecretFile
}



if(-not (test-path -path $secretFile -pathType leaf)){
    write-host "secret file ($secretFile) doesn't exist. Please provide a valid secret file in this location or use -secretFile <file> option to provide one. Exiting."
    exit
}

$secretIn = Get-Content $secretFile | Select-String -pattern "#" -notMatch | ?{ $TRUE -ne [string]::IsNullOrWhiteSpace($_.Line) }| Select -first 1 

$secret = $secretIn.Line

if([string]::IsNullOrEmpty($secret)){
    write-host "secret file ($secretFile) doesn't seem to have a token in it. Exiting."
    exit
}

$outputDir = get-latest-repo-release -gitPersonalAccessToken $secret -owner $owner -repo $repo -dropFolder $zipDropFolder
if (($keepZip -eq $TRUE) -or ($unzip -eq $FALSE)){
    write-host "The latest releases can be found here: $($outputDir.path)"
}

write-host "Pulled $owner/$repo release $($outputDir.version)"

if($unzip -eq $TRUE){
    
    Write-Host Extracting Downsync zip...
    $fullZipPath = $outputDir.path | Resolve-Path
    $fullUnzipPath = $unzipPath | Resolve-Path
    Expand-Archive -LiteralPath $fullZipPath -DestinationPath $fullUnzipPath -force;
    
    if (Test-Path "$unzipPath\tag.txt" -pathType leaf) {
        Remove-Item "$unzipPath\tag.txt"
    }
    Add-Content -path "$unzipPath\tag.txt" $outputDir.version
    Write-Host DONE Extracting Downsync zip.
}

if(($keepZip -eq $FALSE) -and ($unzip -eq $TRUE)){
    Write-Host Removing zip file...;
    Remove-Item $outputDir.path;
}

write-host " "
write-host "********"
write-host "** Completed."
write-host "** Downsync $($outputDir.version) is in folder $unzipPath"
write-host "** Besure to update any configs or scripts to be compatible with this version" 
write-host "********"
write-host " "