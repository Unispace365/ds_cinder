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
		Replace("<cinder/app/AppBasic.h>", "<cinder/app/App.h>").
		Replace("CINDER_APP_BASIC", "CINDER_APP");
		Replace("RendererGl(RendererGl::Options", "ci::app::RendererGl(ci::app::RendererGl::Options");
	
    Set-Content -Path $OutputFile -Value $Content;
}

pause