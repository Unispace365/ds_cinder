# Versioning Ds Cinder Apps

## Adding version info to an exisiting app
1. Open the solution in visual studio
2. Under the app project, delete Resouces.rc from the Resources folder
3. Right click the resources folder and select Add -> Resource
	- This might trigger an error, just repeat the step and it should resolve
4. Selct 'version' and click 'add'
5. In the newly opened project.rc file set the following values
	1. CompanyName: "Downstream"
	2. FileDescription: Short description of the app
	3. FILEVERSION (not the FileVersion string): set to major.minor.bugfix.build_number
	4. PRODUCTVERSION (not the ProductVersoin string): Same as FILEVERSION
	5. ProductName: The human readable name of the product/project
	6. OriginalFilename + InternalName: Should be the name of the final application exe
6. Right click Resources folder again and Add -> Resource -> Icon -> Import
7. Navigate to the vs2019 folder, change file type selector to `All Files (*.*)` and select cinder_app_icon.ico
8. Move the new .rc, .h and cinder_app_icon.ico into the Resource folder
9. Build the app and make sure everything is working as expected
10. To ensure the version is in the exe
	1. Right click the final exe -> Properties -> Details
	2. The file description / version / etc should all be listed

## Bumping versions
1. Open the `.rc` file for your project
2. Update FILEVERSION and PRODUCTVERSION
	- For settings changes and other non-code changes the app version can stay the same. The installer version should have its build_number incremented to differentiate with older installers
	- For minor bugfixes and small (non-breaking) code changes: increment the bugfix revision
	- For larger bugfixes or small (breaking) feature changes: increment the minor revision
	- For new versions of projects, major ASR changes or breaking changes: Increment the major Revision
3. Update all the versions in `install/*.iss` files for the project to match the major.minor.bugfix.build_number
	* build_number of the installer should be >= the build_number of the application
4. Add the new version and changelog to the release notes: `install/readme.txt`
	- For older projects you may need to create this file. See ds_cinder/example/full_starter/install/readme.txt for the format
6. Build and distribute installers as necessary
7. Commit your changes including the version number in the commit
8. Push (To the main branch for solo projects. For larger projects push to a feature branch and create a pull request)
