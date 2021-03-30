# Get a list of the source files
$ExamplePath = Join-Path $env:DS_PLATFORM_093 -ChildPath "/example"
$DirectoryList = Get-ChildItem -Path $ExamplePath -Attributes D;

# Get source CMakeLists.txt
$SourceLists = Join-Path $env:DS_PLATFORM_093 -ChildPath "/example/full_starter/CMakeLists.txt"
$SourceExists = Test-Path -Path $SourceLists -PathType Leaf
if(!$SourceExists){
    Write-Host "Source CMakeLists.txt: " + $SourceLists + " doesn't exists!"
    exit
}
"p: "+$SourceLists
$RawContent = Get-Content -LiteralPath $SourceLists -Raw;
"RC: " + $RawContent.Length

#Get source CMakeSttings.json
$SourceJson = Join-Path $env:DS_PLATFORM_093 -ChildPath "/example/full_starter/CMakeSettings.json"


#copy

foreach ($File in $DirectoryList) {
    $File
    $FileFN = [REGEX]::Escape($File.FullName)
    $fileFN
    $File.FullName
    $SourceLists

    $OutputFile = Join-Path $File.FullName -ChildPath "/CMakeLists.txt"
    if($OutputFile -ne $SourceLists){
        Copy-Item -Path $SourceJson -Destination $File.FullName
        $reserved = 'pdf','viewers','web','video'
        $ReplaceName = "$File"
        if($reserved.Contains($ReplaceName)){
            $ReplaceName = "$ReplaceName-ish"
        }
    
        #get source files

        $FileList = (gci -Path "$FileFN\\src" -Recurse *.* -Include *.cpp,*.h,*.c,*.cc,*.hpp) -replace "^$FileFN\\","" -replace '$',"`r`n" -replace '\\','/'
        $FileList = " $FileList" -replace ' ','    '
        "FileList: $FileList"
        #if(!$exists){
            $Content = $RawContent -replace '(?ms)(add_executable\( .* WIN32).*?(\))',"`$1`r`n$FileList`r`n`$2" -replace 'full_starter',$ReplaceName
        
            Set-Content -Path $OutputFile -Value $Content
        #}
    }

}