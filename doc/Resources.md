Application Resources
=========================

The preferred method to add images or resources to your project is to include them in the data folder, and refer to them like so:

    mImage.setImageFile("%APP%/data/images/ImageImage.png");
    
The %APP% wildcard points to the root level of your app, generally where your exe lives. It's the folder that contains the 'data' and 'settings' folders. 

For PNG's, the width and height are read from the metadata of the file, so the width/height will be known immediately and quickly, while the full image will be loaded asyncronously. 

For JPEG's, the width and height cannot be reliably found without loading the full image, so you can use the directions below to specify the width and height manually to ensure the application doesn't stutter while loading images.

Adding Medata manually
-----------------------------
Apps can manage their own resource folders, and supply file names directly to sprites. Png's will load the metadata automatically, so this generally only applies to jpgs. You can optimize performance of images by including meta information in the file name like so:

    (your_image_name).w_(your_image_width).h_(your_image_height).(extention)
	
example:

    killer_bunny_glow.w_1920.h_1080.jpg


Loading images from the Resources database
-------------------------------------

If your app loads data from a sqlite database that includes a Resources table, you can load an image from the database without the metadata penalty, like so:

    	mToggleArrow.setImageResource(resourceId);
    	
This allows you to load cms-delivered content quickly, and applies to the other media types as well (such as PDF and Video)
