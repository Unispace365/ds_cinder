#pragma once

#include <Poco/Runnable.h>

#include <ds/ui/sprite/sprite_engine.h>

namespace waffles {

bool alphaSort(ds::model::ContentModelRef& a, ds::model::ContentModelRef& b);

class SearchQuery : public Poco::Runnable {
  public:
	SearchQuery();

	// SearchMode: 0 = general, 1 = ?
	void		 setInput(ds::ui::SpriteEngine& eng, const std::string& leQuery, const std::string& filterType,
						  const int& resourceFilter);
	virtual void run();

	std::vector<ds::model::ContentModelRef> mOutput;

  private:
	ds::ui::SpriteEngine* mEngine;
	std::string			  mInput;
	std::string			  mFilterType;
	int					  mResourceFilter; // video, pdf, image, link

	void queryGeneral();
	void recursiveMatch(ds::model::ContentModelRef item);
};

} // namespace waffles
