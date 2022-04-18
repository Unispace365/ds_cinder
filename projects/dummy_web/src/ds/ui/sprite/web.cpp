#include "stdafx.h"

#include "web.h"

namespace {

class Init {
public:
	Init() {
		ds::App::AddStartup([](ds::Engine& e) {
			e.installSprite([](ds::BlobRegistry& r){ds::ui::Web::installAsServer(r);},
							[](ds::BlobRegistry& r){ds::ui::Web::installAsClient(r);});
		});
	}
	void					doNothing() { }
};
Init						INIT;


char						BLOB_TYPE			= 0;
}

namespace ds {
namespace ui {

void Web::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r);});
}

void Web::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<Web>(r);});
}

} // namespace ui
} // namespace ds
