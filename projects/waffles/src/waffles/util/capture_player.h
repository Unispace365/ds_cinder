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
	void setCaptureSource(const std::string& sourceIdName);

	/// Set the capture source by ID and Name
	void setCaptureSource(int id, const std::string& sourceName);

	void saveImage();

  protected:
	virtual void onUpdateServer(const ds::UpdateParams& up) override;
	virtual void drawLocalClient() override;

	int			mCaptureId = -1;
	std::string mSourceName;
};

} // namespace waffles