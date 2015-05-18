#ifndef DS_PLATFORM_VIDEO_GSTREAMER_ENV_CHECK_H_
#define DS_PLATFORM_VIDEO_GSTREAMER_ENV_CHECK_H_

#include <string>

namespace ds {
namespace gstreamer {

/*!
 * \class EnvCheck
 * \namespace ds::gstreamer
 * \brief checks and verifies integrity of all GStreamer env vars.
 */
class EnvCheck
{
public:
    EnvCheck();

    void setEnvVarName(const std::string&);
    void setDefaultLoc(const std::string&);
    void verifyPathVar();

private:
    std::string mGstreamerEnvVarName;
    std::string mGstreamerDefaultLoc;
    std::string mGstreamerPluginPath;
};

}} //!ds

#endif //!DS_PLATFORM_VIDEO_GSTREAMER_ENV_CHECK_H_
