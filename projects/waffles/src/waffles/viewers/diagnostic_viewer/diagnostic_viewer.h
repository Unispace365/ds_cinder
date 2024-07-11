#pragma once

#include "waffles/viewers/base_element.h"

#include <ds/ui/button/sprite_button.h>
#include <ds/ui/layout/smart_layout.h>

namespace waffles {
class DiagnosticTest;

/**
 * \class ds::DiagnosticViewer
 *			A viewer panel that runs some tests for the environment (network connectivity, local access, etc)
 */
class DiagnosticViewer : public BaseElement {
  public:
	DiagnosticViewer(ds::ui::SpriteEngine& g);

  protected:
	virtual void onLayout();
	void		 startDiagnostics();
	void		 showLogs();
	void		 addTest(const int type, const std::string& url = "");
	void		 testsComplete();

	std::vector<DiagnosticTest*> mTests;
	ds::ui::SmartLayout*		 mPrimaryLayout;
	int							 mNumCompletedTests;
};

} // namespace waffles
