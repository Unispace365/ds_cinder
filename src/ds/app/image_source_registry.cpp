#include "ds/app/image_source_registry.h"

#include <assert.h>
#include <iostream>
#include "ds/debug/debug_defines.h"

namespace ds {

ImageSourceRegistry::ImageSourceRegistry()
{
  mFactory.reserve(32);
  // Install an empty handler at index 0, because we don't want anyone
  // assigned byte '0' for read/writing.
  mFactory.push_back(nullptr);
}

char ImageSourceRegistry::add(const std::function<ds::ui::ImageSource*(ds::ui::SpriteEngine&)>& factoryFn)
{
  assert(mFactory.size() < 120);
  const char        index = static_cast<char>(mFactory.size());
  mFactory.push_back(factoryFn);
  return index;
}

ds::ui::ImageSource* ImageSourceRegistry::make(const char n, ds::ui::SpriteEngine& se)
{
	const size_t				idx = static_cast<int>(n);
	if (idx < 0 || idx >= mFactory.size()) return nullptr;
	if (mFactory[idx] == nullptr) return nullptr;
	return mFactory[idx](se);
}

} // namespace ds