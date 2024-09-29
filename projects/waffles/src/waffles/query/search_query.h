#pragma once

#include <Poco/Runnable.h>

#include <ds/ui/sprite/sprite_engine.h>

namespace waffles {

bool alphaSort(ds::model::ContentModelRef& a, ds::model::ContentModelRef& b);

class SearchQuery : public Poco::Runnable {
  public:
	SearchQuery();

	// SearchMode: 0 = general, 1 = ?
	virtual void		 setInput(ds::ui::SpriteEngine& eng, const std::string& leQuery, const std::string& filterType,
						  const int& resourceFilter);
	virtual void run();

	std::vector<ds::model::ContentModelRef> mOutput;
	std::vector<std::string> mSeenFiles;

  protected:
	ds::ui::SpriteEngine* mEngine=nullptr;
	std::string			  mInput;
	std::string			  mFilterType;
	int					  mResourceFilter=0; // video, pdf, image, link, (presentation?)


	virtual void queryGeneral();
	
	virtual void recursiveMatch(ds::model::ContentModelRef item);
};

} // namespace waffles
