Apps can include resources in several ways.  The traditional way is to include a resource database, generally constructed using the downstream app_resources utility.  When using the database (along with the resource writer), the app will have an auto-generated include file that lists all possible resources in the form of Resource::Id objects, when can then be supplied to sprites.

Alternatively, apps can manage their own resource folders, and supply file names directly to sprites.  When doing this, you can optimize performance of images by including meta information int he file name like so:
	(your_image_name).w_(your_image_width).h_(your_image_height).(extention)
example:
	killer_bunny_glow.w_1920.h_1080.png
