#include "ds/arc/arc_io.h"

#include <cinder/app/App.h>
#include <cinder/DataSource.h>
#include "ds/app/FrameworkResources.h"
// arc classes
#include "ds/arc/arc_chain.h"
#include "ds/arc/arc_layer.h"
#include "ds/arc/arc_map.h"
#include "ds/arc/arc_pow.h"

namespace ds {
namespace arc {

namespace {
const std::string			RESOURCE_("resource:");

std::unique_ptr<Arc>		load_xml(const std::string& xmlstr) {
	try {
		ci::XmlTree					xml(xmlstr);
		for (auto it=xml.begin(), end=xml.end(); it != end; ++it) {
			std::unique_ptr<Arc>	ans(create(*it));
			if (ans) return ans;
		}
	} catch (std::exception const&) {
	}
	return nullptr;
}

}

std::unique_ptr<Arc>		load(const std::string& filename) {
	// Determine if this is a resource
	if (filename.compare(0, RESOURCE_.length(), RESOURCE_) == 0) {
		ci::DataSourceRef		ds;
		const std::string		sub(filename.substr(RESOURCE_.length(), filename.length()));
		if (sub == "drop_shadow") {
			ds = ci::app::App::loadResource(RES_ARC_DROPSHADOW);
		}
		if (ds) {
			ci::Buffer&			buf = ds->getBuffer();
			if (buf.getSize() > 0 && buf.getSize() < 100000) {
				std::string	str(static_cast<const char*>(buf.getData()), buf.getDataSize());
				return load_xml(str);
			}
		}
	}
	return nullptr;
}

std::unique_ptr<Arc>		create(const std::string& classname)
{
	if (classname == "chain") return std::unique_ptr<Arc>(new ds::arc::Chain());
	if (classname == "map") return std::unique_ptr<Arc>(new ds::arc::Map());
	if (classname == "pow") return std::unique_ptr<Arc>(new ds::arc::Pow());
	return nullptr;
}

std::unique_ptr<Arc>		create(const ci::XmlTree& xml)
{
	std::unique_ptr<Arc>	ans;
	if (xml.getTag() == "arc") ans = create(xml.getAttribute("type").getValue());
	if (xml.getTag() == "layer") ans = std::unique_ptr<Arc>(new ds::arc::Layer());
	if (ans) ans->readXml(xml);
	return ans;
}

} // namespace arc
} // namespace ds
