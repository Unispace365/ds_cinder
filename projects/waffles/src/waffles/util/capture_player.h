#pragma once

#include <ds/ui/sprite/sprite.h>

namespace waffles {

/// Displays a webcam/capture card input as a sprite
/// Caches any active connections so addtional sprites with the same capture have minimal performance hit
class CapturePlayer : public ds::ui::Sprite {
  public:
	CapturePlayer(ds::ui::SpriteEngine& g);
	~CapturePlayer();

	/// Set the capture source by string.
	/// sourceIdName in the format "ID;NAME" as reported by the OS
	/// or "NAME" if capture cards have unique names
	bool setCaptureSource(const std::string& sourceIdName);

	/// Set the capture source by ID and Name
	bool setCaptureSource(int id, const std::string& sourceName);

  protected:
	virtual void drawLocalClient() override;

	uint64_t	mCaptureId = 0;
	std::string mSourceName;

  private:
	bool setCaptureSourceWithUniqueName(const std::string& uniqueName);
	void initDeviceResolutionMap();

	std::thread mUpdateTextureThread;
	std::unordered_map<std::string, ci::vec2> mDeviceResolutionMap;
};

} // namespace waffles
