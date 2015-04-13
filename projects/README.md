Take a look at [boilerplate](/boilerplate) if you are looking into developing a plug-in.


----------


This folder contains all the optional projects that can be added to a DsCinder project.

The structure is intended to allow for mutually exclusive objects. There should always be a top-level item (i.e. "video"), then any folders underneath that contain the list of mutually exclusive projects. For example, video/ has gstreamer/ and quicktime; a solution can't mix both, since they both will add a "ds::ui::Video" class to the solution.

In order to add an optional project, do these things:


1. Add the desired project to your solution.


2. Reorder the dependencies.
2.1. Right-click the Solution, select Properties, navigate to Common Properties->Project Dependencies, and make sure your app project relies on the new project).


3. Add the correct property sheets. Each project has release and debug property sheets in its PropertySheets folder.
3.1. Navigate to the Property Manager by selecting View->Property Manager.
3.2. In your app project, add the correct debug and release property sheets to the correct configuration (i.e. the *_d.props file is added to "Debug | Win32".
3.3. Make sure the order is correct -- the new property sheets should have been added at the top. In any case, they need to be above the Platform property sheet.


4. Make sure all paths are environment agnostic. By default, Visual Studio will use relative paths to the added project and property sheets. However, different people create application folders in different relative locations to the main platform folder, so you need to manually edit two files to correct this.

4.1. The sln
If the project is (for example) MuPDF, look for something like

Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "pdf", "..\..\..\ds_cinder\projects\pdf\mupdf\pdf.vcxproj", 

and replace it with something like

Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "pdf", "%DS_PLATFORM%\projects\pdf\mupdf\pdf.vcxproj", "{BA6D6227-5B3F-4967-B005-1A9573FF6A69}"

4.2. The .vcxproj
If the project is (for example) MuPDF, look for something like

<Import Project="..\..\..\ds_cinder\projects\pdf\mupdf\PropertySheets\Pdf_MuPdf.props" /> 

and replace it with something like

<Import Project="$(DS_PLATFORM)\projects\pdf\mupdf\PropertySheets\Pdf_MuPdf.props" />


5. Set the dependencies correctly (right click on the startup project, select Project Dependencies...). The newly added project needs to depend on platform, and the startup project needs to depend on the newly added project.