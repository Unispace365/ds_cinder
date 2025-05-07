#pragma once

#include <ds/ui/media/media_interface.h>
#include <waffles/util/waffles_helper.h>

namespace waffles {

using namespace ds::model;

class BaseWafflesHelper : public WafflesHelper {
  public:
	BaseWafflesHelper(ds::ui::SpriteEngine& eng);

	BaseWafflesHelper(const BaseWafflesHelper&)			   = delete;
	BaseWafflesHelper(BaseWafflesHelper&&)				   = delete;
	BaseWafflesHelper& operator=(const BaseWafflesHelper&) = delete;
	BaseWafflesHelper& operator=(BaseWafflesHelper&&)	   = delete;

	// Inherited via WafflesHelper

	bool						 getApplyParticles() override;
	ContentModelRef				 getPinboard() override;
	std::vector<ContentModelRef> getValidPinboards() override;
	ContentModelRef				 getAnnotationFolder() override;
	void						 setKeyboardStyle(ds::ui::SoftKeyboard* keeb) override;
	void						 setMediaInterfaceStyle(ds::ui::MediaInterface* interfacey) override;
	ds::Resource				 getBackgroundForPlatform() override;
	int							 getBackgroundPdfPage() override;
	std::vector<ContentModelRef> getContentForPlatform() override;

	bool				 isValidForFilter(const std::string& filter, ContentModelRef model) override;
	void				 setLauncherCustomFilters(CustomFilters cf) override;
	const CustomFilters& getLauncherCustomFilters() override;
	void				 setLauncherCustomContent(CustomContent cc) override;
	const CustomContent& getLauncherCustomContent() override;

  protected:
	void loadIntegration() const;

  private:
	mutable std::string				 mEventFieldKey;
	mutable std::string				 mPlatformFieldKey;
	mutable std::vector<std::string> mAnnotationFolderKeys;
	mutable bool					 mUseRoot{false};
	CustomFilters					 mLauncherCustomFilters;
	CustomContent					 mLauncherCustomContent;
};
} // namespace waffles
