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

	/// Which channels to use for output. Note that using anything other than 2 channels may cause the playback to stall
	/// Channels are powers of two, so 0x1 is the first channel, 0x2 is the second, 0x3 is the first two channels (front left and front right)
	/// 0x12 is channels 3 and 4 (I think back left and back right in 5.1 setups)
	/// 0x48 is channels 5 and 6 (I think center and subwoofer channels)
	std::string mChannelMask;

	/// How loud to make this output from 0.0 (mute) to 10.0 (10x boosted volume). default is 1.0 (100%)
	double		mVolume; 

	GstAudioDevice() : mVolume(1.0){};
	GstAudioDevice(const std::string& deviceName,
				   const std::string& channelMask = "0x3",
				   const std::string& deviceGuid = "", 
				   const double theVolume = 1.0);

	void			initialize();
};

} // namespace ds

#endif