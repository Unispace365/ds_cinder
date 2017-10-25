#ifndef DS_GST_AUDIO_DEVICE
#define DS_GST_AUDIO_DEVICE

#include <string>

namespace ds {

/// Defines a device to target for audio playback.
/// Note that targetting multiple USB devices from the same video may cause playback to stall
/// Currently only working on Windows as this uses the directsoundsink element in gstreamer which is windows-only
struct GstAudioDevice {
	/// The name of the device as listed in the Sound control panel
	std::string mDeviceName;

	/// If this is blank, it will be auto-detected from the current sound card driver list, assuming the device name can be found
	std::string mDeviceGuid;

	/// Internally-used name of the panorama and volume elements. Clients don't need to set these
	std::string mPanoramaName;
	std::string mVolumeName;


	/// How loud to make this output from 0.0 (mute) to 10.0 (10x boosted volume). default is 1.0 (100%)
	/// Set this when sending into the GstVideo sprite instance to change the volume after starting
	double		mVolume; 

	/// Left / right pan for this device, -1.0 (left) to 1.0 (right). Default is 0.0 which is balanced
	/// Set this when sending into the GstVideo sprite instance to change the panb after starting
	float		mPan;


	GstAudioDevice() : mVolume(1.0){};
	GstAudioDevice(const std::string& deviceName,
				   const std::string& deviceGuid = "", 
				   const double theVolume = 1.0,
				   const float thePan = 0.0
				   );

	void			initialize();
};

} // namespace ds

#endif