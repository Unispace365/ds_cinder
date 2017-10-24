### Updateing to 0.9.1 and vs2015

* Edit SLN
	1. Replace all 2013 with 2015
	2. Make Visual Studio Version 14 anywhere it says 12
	? 3. Delete global win32 configurations

* Edit vcxproj
	1. Replace all 2013 with 2015
	2. Delete any references to 32bit property sheets

* Open SLN (should auto-launch vs 2015)
	1. Go to configuration manager
	2. Next to the project's platform (should say x64 or Win32), select Edit... and Remove Win32
	3. Right-click the project and upgrade compiler version
	4. If you encounter issues, restart visual studio
	5. Clean and build