Write-Host "Replacing vectors and app launch macros...";

# Get a list of the source files
$FileList = Get-ChildItem -Path .\ -Include *.cpp,*.h -Recurse;

# For each file, get the content, replace the content, and 
# write to new output location
foreach ($File in $FileList) {
    $OutputFile = $File.FullName;
	
    $Content = (Get-Content -Path $File.FullName -Raw).Replace("ci::Vec2f", "ci::vec2").
		Replace("ci::Vec2i", "ci::ivec2").
		Replace("ci::Vec3f", "ci::vec3").
		Replace("ci::Vec3i", "ci::ivec3").
		Replace("ci::Vec4f", "ci::vec4").
		Replace("ci::Vec4i", "ci::ivec4").
		Replace("ci::Matrix44f", "ci::mat4").
		Replace("ci::Matrix33f", "ci::mat3").
		Replace("<cinder/app/AppBasic.h>", "<cinder/app/App.h>").
		Replace("CINDER_APP_BASIC", "CINDER_APP").
		Replace('RendererGl(RendererGl::Options', 'ci::app::RendererGl(ci::app::RendererGl::Options');
	
    Set-Content -Path $OutputFile -Value $Content;
}

Write-Host "Updating solution and project files...";

# Get a list of the source files
$FileListSolution = Get-ChildItem -Path .\ -Include *.sln,*.vcxproj -Recurse;

# For each file, get the content, replace the content, and 
# write to new output location
foreach ($File in $FileListSolution) {
    $OutputFile = $File.FullName;
	
    $Content = (Get-Content -Path $File.FullName -Raw).Replace("DS_PLATFORM_086", "DS_PLATFORM_090").
		Replace("CINDER_086", "CINDER_090");
	
    Set-Content -Path $OutputFile -Value $Content;
}


Write-Host "Finished!";

pause