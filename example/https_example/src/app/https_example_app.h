#ifndef _HTTPS_EXAMPLE_APP_H_
#define _HTTPS_EXAMPLE_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>

#include "ds/network/https_client.h"

namespace example {
class AllData;

class https_example : public ds::App {
  public:
	https_example();

	void setupServer();

  private:
	ds::net::HttpsRequest mHttpsRequest;
};

} // namespace example

#endif // !_HTTPS_EXAMPLE_APP_H_