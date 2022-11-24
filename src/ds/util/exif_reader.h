#include "exif.h"
#include <ds/debug/logger.h>
#include <stdio.h>

namespace ds {

class ExifHelper {
  public:
	/// Returns true if exif was found and w/h is valid (above zero)
	static bool getImageSize(const std::string filePath, int& outWidth, int& outHeight) {
		FILE* fp = fopen(filePath.c_str(), "rb");
		if (!fp) {
			DS_LOG_WARNING("Can't open file for exif data. " << filePath);
			return false;
		}
		fseek(fp, 0, SEEK_END);
		unsigned long fsize = ftell(fp);
		rewind(fp);
		unsigned char* buf = new unsigned char[fsize];
		if (fread(buf, 1, fsize, fp) != fsize) {
			DS_LOG_WARNING("Can't read file for exif data. " << filePath);
			delete[] buf;
			return false;
		}
		fclose(fp);

		/// Parse EXIF
		easyexif::EXIFInfo result;
		int				   code = result.parseFrom(buf, fsize);
		delete[] buf;
		if (code) {
			if (code == 1983) {
				/// this means the file didn't have any exif. Bummer. ignore it, no reason to log it.
			} else {
				DS_LOG_WARNING("Error parsing EXIF: code " << code << " for filepath: " << filePath);
			}
			return false;
		}

		outWidth  = result.ImageWidth;
		outHeight = result.ImageHeight;

		if (outHeight < 1 || outWidth < 1) {
			return false;
		}

		return true;
	}

	/// Prints out all the exif data
	static int printExifData(std::string filePath) {

		/// Read the JPEG file into a buffer
		FILE* fp = fopen(filePath.c_str(), "rb");
		if (!fp) {
			printf("Can't open file.\n");
			return -1;
		}
		fseek(fp, 0, SEEK_END);
		unsigned long fsize = ftell(fp);
		rewind(fp);
		unsigned char* buf = new unsigned char[fsize];
		if (fread(buf, 1, fsize, fp) != fsize) {
			printf("Can't read file.\n");
			delete[] buf;
			return -2;
		}
		fclose(fp);

		/// Parse EXIF
		easyexif::EXIFInfo result;
		int				   code = result.parseFrom(buf, fsize);
		delete[] buf;
		if (code) {
			printf("Error parsing EXIF: code %d\n", code);
			return -3;
		}

		/// Dump EXIF information
		printf("Camera make          : %s\n", result.Make.c_str());
		printf("Camera model         : %s\n", result.Model.c_str());
		printf("Software             : %s\n", result.Software.c_str());
		printf("Bits per sample      : %d\n", result.BitsPerSample);
		printf("Image width          : %d\n", result.ImageWidth);
		printf("Image height         : %d\n", result.ImageHeight);
		printf("Image description    : %s\n", result.ImageDescription.c_str());
		printf("Image orientation    : %d\n", result.Orientation);
		printf("Image copyright      : %s\n", result.Copyright.c_str());
		printf("Image date/time      : %s\n", result.DateTime.c_str());
		printf("Original date/time   : %s\n", result.DateTimeOriginal.c_str());
		printf("Digitize date/time   : %s\n", result.DateTimeDigitized.c_str());
		printf("Subsecond time       : %s\n", result.SubSecTimeOriginal.c_str());
		printf("Exposure time        : 1/%d s\n", (unsigned)(1.0 / result.ExposureTime));
		printf("F-stop               : f/%.1f\n", result.FNumber);
		printf("ISO speed            : %d\n", result.ISOSpeedRatings);
		printf("Subject distance     : %f m\n", result.SubjectDistance);
		printf("Exposure bias        : %f EV\n", result.ExposureBiasValue);
		printf("Flash used?          : %d\n", result.Flash);
		printf("Metering mode        : %d\n", result.MeteringMode);
		printf("Lens focal length    : %f mm\n", result.FocalLength);
		printf("35mm focal length    : %u mm\n", result.FocalLengthIn35mm);
		printf("GPS Latitude         : %f deg (%f deg, %f min, %f sec %c)\n", result.GeoLocation.Latitude,
			   result.GeoLocation.LatComponents.degrees, result.GeoLocation.LatComponents.minutes,
			   result.GeoLocation.LatComponents.seconds, result.GeoLocation.LatComponents.direction);
		printf("GPS Longitude        : %f deg (%f deg, %f min, %f sec %c)\n", result.GeoLocation.Longitude,
			   result.GeoLocation.LonComponents.degrees, result.GeoLocation.LonComponents.minutes,
			   result.GeoLocation.LonComponents.seconds, result.GeoLocation.LonComponents.direction);
		printf("GPS Altitude         : %f m\n", result.GeoLocation.Altitude);
		printf("GPS Precision (DOP)  : %f\n", result.GeoLocation.DOP);
		printf("Lens min focal length: %f mm\n", result.LensInfo.FocalLengthMin);
		printf("Lens max focal length: %f mm\n", result.LensInfo.FocalLengthMax);
		printf("Lens f-stop min      : f/%.1f\n", result.LensInfo.FStopMin);
		printf("Lens f-stop max      : f/%.1f\n", result.LensInfo.FStopMax);
		printf("Lens make            : %s\n", result.LensInfo.Make.c_str());
		printf("Lens model           : %s\n", result.LensInfo.Model.c_str());
		printf("Focal plane XRes     : %f\n", result.LensInfo.FocalPlaneXResolution);
		printf("Focal plane YRes     : %f\n", result.LensInfo.FocalPlaneYResolution);

		return 0;
	}
};
} // namespace ds
