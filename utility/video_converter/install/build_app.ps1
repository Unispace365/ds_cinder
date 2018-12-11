function buildVS
{
    param
    (
        [parameter(Mandatory=$true)]
        [String] $path,
        
        [parameter(Mandatory=$false)]
        [bool] $clean = $false
    )
    process
    {
        $msBuildExe = 'msbuild'


        if ($clean) {
            Write-Host "Cleaning $($path)" -foregroundcolor green
            & "$($msBuildExe)" "$($path)" /t:Clean /m
        }

        Write-Host "Building $($path)" -foregroundcolor green
        & "$($msBuildExe)" "$($path)" /t:Build /m /p:Configuration=Release
    }
}

pushd 'C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\amd64\'    
cmd /c "vcvars64.bat&set" |
foreach {
  if ($_ -match "=") {
    $v = $_.split("="); set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])"
  }
}
popd
write-host "`nVisual Studio 2015 Command Prompt variables set." -ForegroundColor Yellow


$ThisCommand = $MyInvocation.MyCommand.Path;
$ThisDir = (Get-Item $ThisCommand ).Directory.FullName;

buildVs $ThisDir/../vs2015/video_converter.vcxproj
