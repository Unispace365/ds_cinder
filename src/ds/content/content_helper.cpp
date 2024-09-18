#include "stdafx.h"
#include "ds/content/content_helper.h"

namespace ds::model {

ContentHelperPtr ContentHelperFactory::mDefault = nullptr;

const std::string ContentHelper::DEFAULTCATEGORY = "default!";
const std::string ContentHelper::WAFFLESCATEGORY = "waffles!";
const std::string ContentHelper::PRESENTATIONCATEGORY = "presentation";
const std::string ContentHelper::AMBIENTCATEGORY = "ambient";

}