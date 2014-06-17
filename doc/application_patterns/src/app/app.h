#pragma once
#ifndef NA_APP_APP_H_
#define NA_APP_APP_H_

namespace na {

/**
 * \class na::App
 * 3x3 wall, each screen 1366 x 768, 8 pixels between columns.
 * Total resolution (raw):                  4098 x 2304
 * Final, on site (including extra pixels): 4114 x 2304
 * Half scaled:                             2057 x 1052
 * Current "docs" \\192.168.0.77\project2\CORPORATE\NETAPP\AMSTERDAM\DEV
 */
class App : public ofBaseApp
{
public:
	explicit App(const ds::AppSettings&);

private:
  void                            onAppEvent(const na::Event&);
  void                            onStoryTreeResult(StoryTreeQuery&);
  void                            onStoryPanelRequestQuery(na::AnnotatedStoryQuery<na::StoryPanelRequestQuery>&);
  void                            onStoryPageRequestQuery(na::AnnotatedStoryQuery<na::StoryPageRequestQuery>&);
//  void                            onOpenRtnEventQuery(na::AnnotatedRtnQuery<na::OpenRtnEvent>&);
  void                            onOpenRtnEventQuery(const na::OpenRtnEvent&);

  friend class Globals;
	ofxWorldEngine						      mWorld;
	const ds::AppSettings				    mAppSettings;
	ds::AppMouseHandler					    mMouseHandler;

	Globals				    			        mGlobals;
  WindowSprite&                   mIdler;
  ds::SerialRunnable<StoryTreeQuery>
                                  mStoryTreeQuery;
  // DATA
  // The app houses this data but it's primarily handled by the Globals;
  // it only lives here to avoid dragging dependencies into the Globals,
  // which *everyone* relies on.
  na::StoryTree                   mStoryTree;
  std::unordered_map<int, na::Virtue>
                                  mVirtue;
  na::RtnList                     mRtnList;

  // Intercept story panel request queries and convert them into
  // normal requests, with the story data.
  ds::ParallelRunnable<na::AnnotatedStoryQuery<na::StoryPanelRequestQuery>>
                                  mStoryPanelRequestQuery;
  ds::ParallelRunnable<na::AnnotatedStoryQuery<na::StoryPageRequestQuery>>
                                  mStoryPageRequestQuery;
//  ds::ParallelRunnable<na::AnnotatedRtnQuery<na::OpenRtnEvent>>
//                                  mOpenRtnEventQuery;

  ds::ParallelRunnable<na::VirtueQuery>
                                  mVirtueQuery;

  na::math::PolygonManager        mPanelHotspotHitmap;

  ds::PreloadImage                mPreloader;
  na::AppDebug                    mDebug;
};

} // namespace na

#endif // NA_APP_APP_H_
