This folder contains all the optional projects that can be added to a DsCinder project.

The structure is intended to allow for mutually exclusive objects. There should always be a top-level item (i.e. "video"), then any folders underneath that contain the list of mutually exclusive projects. For example, video/ has gstreamer/ and quicktime; a solution can't mix both, since they both will add a "ds::ui::Video" class to the solution.

In order to add an optional project, do three things:

1. Add the desired project to your solution.

2. Reorder the dependencies.
2.1. Right-click the Solution, select Properties, navigate to Common Properties->Project Dependencies, and make sure your app project relies on the new project).

3. Add the correct property sheets. Each project has release and debug property sheets in its PropertySheets folder.
3.1. Navigate to the Property Manager by selecting View->Property Manager.
3.2. In your app project, add the correct debug and release property sheets to the correct configuration (i.e. the *_d.props file is added to "Debug | Win32".
3.3. Make sure the order is correct -- the new property sheets should have been added at the top. In any case, they need to be above the Platform property sheet.