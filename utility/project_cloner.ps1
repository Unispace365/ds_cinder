
Function Get-FileName($initialDirectory)
{
    [System.Reflection.Assembly]::LoadWithPartialName("System.windows.forms") | Out-Null
    
    $OpenFileDialog = New-Object System.Windows.Forms.FolderBrowserDialog
    $Show = $OpenFileDialog.ShowDialog()
 
    if($Show -eq "OK"){
        return $OpenFileDialog.SelectedPath
    } else {
        return $initialDirectory;
    }
}


Function Execute-Clone($BaseDir, $DestDir, $NewName, $NameSpace){
    #$DestDir = $DsCinder + "\example\" + $NewName;
    $DestDir = $DestDir + "\" + $NewName;
    $SrcDir = $BaseDir + "src\";
    $SlnDir = $BaseDir + "vs2013\";
    $DebugDir = $BaseDir + "vs2013\Debug";
    $ReleaseDir = $BaseDir + "vs2013\Release";
    $IpchDir = $BaseDir + "vs2013\ipch";
    $x64Dir = $BaseDir + "vs2013\x64";
    $logsDir = $BaseDir + "logs";

    #Write-Host "Removing old stuff at the destination";
    #Remove-Item -Recurse -Force $DestDir;

    Write-Host "Duplicating base project " $BaseDir " to " $DestDir;

    robocopy $BaseDir $DestDir /E /XD $DebugDir $x64Dir $logsdir $ReleaseDir $IpchDir /xf *.sdf *.suo

    $FileList = Get-ChildItem -Path $DestDir -Include *.cpp,*.h -Recurse;

    foreach ($File in $FileList) {
        $OutputFile = $File.FullName;

        $Content = (Get-Content -Path $File.FullName -Raw).
            Replace("fullstarter", $NameSpace).
            Replace("FULLSTARTER", $NewName.ToUpper()).
            Replace("FullStarterApp", $NewName + "_app").
            Replace("FullStarter", $NewName).
            Replace("full_starter_app", $NewName + "_app").
            Replace("full_starter", $NewName);

        Set-Content -Path $OutputFile -Value $Content;

    
        $OutputFileName = $File.Name.Replace("full_starter", $NewName);
        Rename-Item $OutputFile $OutputFileName;

    
        Write-Host "     Wrote" $OutputFileName;
    }

    $FileList = Get-ChildItem -Path $DestDir -Include *.sln,*.vcxproj* -Recurse;

    foreach ($File in $FileList) {
        $OutputFile = $File.FullName;
    
        $Content = (Get-Content -Path $File.FullName -Raw).Replace("fullstarter", $NewName).
            Replace("FULLSTARTER", $NewName.ToUpper()).
            Replace("FullStarter", $NewName).
            Replace("full_starter", $NewName);

        Set-Content -Path $OutputFile -Value $Content;

    
        $OutputFileName = $File.Name.Replace("full_starter", $NewName);
        Rename-Item $OutputFile $OutputFileName;

    
        Write-Host "     Wrote" $OutputFileName;
    }

    Write-Host "Clone finished.";
}


$DsCinder = (Get-ChildItem Env:\DS_PLATFORM_090).Value;
$FullStarter = $DsCinder + "\example\full_starter\";
$ThisCommand = $MyInvocation.MyCommand.Path;
$ThisDir = (get-item $ThisCommand ).Directory.FullName;
$Source = $FullStarter;
$Dest = $ThisDir;
$TheName = "project_name";
$TheNameSpace = "downstream";

[void] [System.Reflection.Assembly]::LoadWithPartialName("System.Drawing") 
[void] [System.Reflection.Assembly]::LoadWithPartialName("System.Windows.Forms") 

$objForm = New-Object System.Windows.Forms.Form 
$objForm.Text = "DS Cinder | Project Cloner"
$objForm.Size = New-Object System.Drawing.Size(800,620) 
$objForm.StartPosition = "CenterScreen"

$objForm.KeyPreview = $True
$objForm.Add_KeyDown({if ($_.KeyCode -eq "Enter") 
    {$x=$objTextBox.Text; Execute-Clone $objTextBox.Text $destTextBox.Text $projText.Text $nameSpaceText.Text }})
$objForm.Add_KeyDown({if ($_.KeyCode -eq "Escape") 
    {$objForm.Close()}})

$OKButton = New-Object System.Windows.Forms.Button
$OKButton.Location = New-Object System.Drawing.Size(700,550)
$OKButton.Size = New-Object System.Drawing.Size(75,23)
$OKButton.Text = "Clone"
$OKButton.Add_Click({Execute-Clone $objTextBox.Text $destTextBox.Text $projText.Text $nameSpaceText.Text })
$objForm.Controls.Add($OKButton)

$CancelButton = New-Object System.Windows.Forms.Button
$CancelButton.Location = New-Object System.Drawing.Size(625,550)
$CancelButton.Size = New-Object System.Drawing.Size(75,23)
$CancelButton.Text = "Exit"
$CancelButton.Add_Click({$objForm.Close()})
$objForm.Controls.Add($CancelButton)

#TODO: uncomment the add lines, and add the ability to specify the strings to replace (namespace, app name, etc)
# Source Folder
$objLabel = New-Object System.Windows.Forms.Label
$objLabel.Location = New-Object System.Drawing.Size(10,20) 
$objLabel.Size = New-Object System.Drawing.Size(280,20) 
$objLabel.Text = "Folder directory of the project to clone from"
# $objForm.Controls.Add($objLabel) 

$objTextBox = New-Object System.Windows.Forms.TextBox 
$objTextBox.Location = New-Object System.Drawing.Size(10,40) 
$objTextBox.Size = New-Object System.Drawing.Size(690,20) 
$objTextBox.Text = $Source;
# $objForm.Controls.Add($objTextBox) 

$BrowseButton = New-Object System.Windows.Forms.Button
$BrowseButton.Location = New-Object System.Drawing.Size(700,40)
$BrowseButton.Size = New-Object System.Drawing.Size(75,20)
$BrowseButton.Text = "Browse"
$BrowseButton.Add_Click({$Source = Get-FileName $objTextBox.Text; $objTextBox.Text = $Source;})
# $objForm.Controls.Add($BrowseButton)


# Destination folder
$destLabel = New-Object System.Windows.Forms.Label
$destLabel.Location = New-Object System.Drawing.Size(10,80) 
$destLabel.Size = New-Object System.Drawing.Size(280,20) 
$destLabel.Text = "Destination folder (NO trailing slash)"
$objForm.Controls.Add($destLabel) 

$destTextBox = New-Object System.Windows.Forms.TextBox 
$destTextBox.Location = New-Object System.Drawing.Size(10,100) 
$destTextBox.Size = New-Object System.Drawing.Size(690,20) 
$destTextBox.Text = $Dest;
$objForm.Controls.Add($destTextBox) 

$DestBrowse = New-Object System.Windows.Forms.Button
$DestBrowse.Location = New-Object System.Drawing.Size(700,100)
$DestBrowse.Size = New-Object System.Drawing.Size(75,20)
$DestBrowse.Text = "Browse"
$DestBrowse.Add_Click({$Dest= Get-FileName $destTextBox.Text; $destTextBox.Text = $Dest;})
$objForm.Controls.Add($DestBrowse)


# Project name
$projName = New-Object System.Windows.Forms.Label
$projName.Location = New-Object System.Drawing.Size(10,140) 
$projName.Size = New-Object System.Drawing.Size(480,20) 
$projName.Text = "Project Name (spaces and special characters not allowed)"
$objForm.Controls.Add($projName) 

$projText = New-Object System.Windows.Forms.TextBox 
$projText.Location = New-Object System.Drawing.Size(10,160) 
$projText.Size = New-Object System.Drawing.Size(690,20) 
$projText.Text = $TheName;
$objForm.Controls.Add($projText) 

# Namespace
$nameSpace = New-Object System.Windows.Forms.Label
$nameSpace.Location = New-Object System.Drawing.Size(10,200) 
$nameSpace.Size = New-Object System.Drawing.Size(480,20) 
$nameSpace.Text = "Namespace"
$objForm.Controls.Add($nameSpace) 

$nameSpaceText = New-Object System.Windows.Forms.TextBox 
$nameSpaceText.Location = New-Object System.Drawing.Size(10,220) 
$nameSpaceText.Size = New-Object System.Drawing.Size(690,20) 
$nameSpaceText.Text = $TheNameSpace;
$objForm.Controls.Add($nameSpaceText) 


$objForm.Topmost = $True

$objForm.Add_Shown({$objForm.Activate()})
[void] $objForm.ShowDialog()



