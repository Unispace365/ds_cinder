/************************************************************************/
/* This DS Cinder project generates other DS Cinder projects by         */
/* by scanning a reference DS Cinder project (DS Cinder project-ception)*/
/* It scans the full_starter project located in %ds_platform_086%       */
/************************************************************************/

#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <cinder/params/Params.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/LocalDateTime.h>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

class ProjectGeneratorApp : public ds::App {
  public:
    ProjectGeneratorApp();

    void				setupServer();

  private:
    typedef ds::App   inherited;
};

ProjectGeneratorApp::ProjectGeneratorApp()
{

};

// trim from start
static inline std::string &ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
	return ltrim(rtrim(s));
}

namespace {
static const std::string ELIPSIS("...");
static const std::string DOT(".");
}

class SettingsSprite : public ds::ui::Sprite
{
public:
	explicit SettingsSprite(ds::ui::SpriteEngine& e)
		: inherited(e)
		, mAppName("OracleMediaWallApp")
		, mSolutionName("oracle_media_wall")
		, mProjectName("oracle_media_wall")
		, mProjectPath("oracle/media_wall")
		, mRootNamespace("oracle")
		, mWindowTitle("Oracle Media Wall App")
		, mProjectFolder(DOT)
		, mWorkingDirectory(ELIPSIS)
		, mWarningCount(0)
		, mTemplatePath("%ds_platform_086%\\example\\full_starter")
		, mTemplateName("full_starter")
		, mTemplateClassName("FullStarterApp")
		, mTemplateNamespace("fullstarter")
		, mTemplateHeadguard("FULLSTARTER")
	{
		mParams = ci::params::InterfaceGl::create(ci::app::getWindow(), "App parameters", ci::app::toPixels(ci::Vec2i(600, 250)));
		setTransparent(false);

		mParams->addText("status", "label=`Please modify the following values and hit Generate.`");
		
		mParams->addSeparator();
		mParams->addParam("App name", &mAppName, "help=`e.g: OracleMediaWallApp`");
		mParams->addParam("Solution name", &mSolutionName, "help=`e.g: oracle_media_wall`");
		mParams->addParam("Project name", &mProjectName, "help=`e.g: oracle_media_wall`");
		mParams->addParam("Root namespace", &mRootNamespace, "help=`oracle`");
		mParams->addParam("Project path", &mProjectPath, "help=`e.g: oracle/media_wall`");
		mParams->addParam("Window title", &mWindowTitle, "help=`e.g: Oracle Media Wall App`");
		mParams->addParam("Project folder", &mProjectFolder, "help=`e.g: c:/users/you/documents/projects/whatever/path/`");
		mParams->addSeparator();
		
		mParams->addButton("Generate", [this](){ generate(); }, "");
		mParams->setOptions("", "valueswidth=450");

		if (mEngine.getDebugSettings().getTextSize("template:path") > 0) {
			mTemplatePath = mEngine.getDebugSettings().getText("template:path");
		}
		if (mEngine.getDebugSettings().getTextSize("template:name") > 0) {
			mTemplateName = mEngine.getDebugSettings().getText("template:name");
		}
		if (mEngine.getDebugSettings().getTextSize("template:classname") > 0) {
			mTemplateClassName = mEngine.getDebugSettings().getText("template:classname");
		}
		if (mEngine.getDebugSettings().getTextSize("template:namespace") > 0) {
			mTemplateNamespace = mEngine.getDebugSettings().getText("template:namespace");
		}
		if (mEngine.getDebugSettings().getTextSize("template:headguard") > 0) {
			mTemplateHeadguard = mEngine.getDebugSettings().getText("template:headguard");
		}
	}

	void SettingsSprite::generate()
	{
		Poco::LocalDateTime now;
		std::string msg = "label=`Generated " + mAppName + " (" + Poco::DateTimeFormatter::format(now, "%H:%M:%S.%i") + ").`";
		mParams->setOptions("status", msg);

		analyzeWorkingDirectory();
	}

	virtual void drawLocalClient() {
		// Draw the interface
		mParams->draw();
		mParams->setPosition(ci::Vec2i(20, 25));
	}

	void copyTemplate() {
		const auto cmd = "xcopy /y /e \"" + mTemplatePath + "\" \"" + mProjectFolder + "\\" + mAppName + "\\\"";
		if (system(cmd.c_str()) != 0) {
			die("Project Generation failed. Failed to xcopy template project: " + mTemplatePath);
		} else {
			mWorkingDirectory.assign(mProjectFolder + "\\" + mAppName + "\\");
		}
	}

	void analyzeWorkingDirectory() {
		// Copy the template
		copyTemplate();
		// Find all files to be configured by RegExp
		std::string f = getTargetFiles();
		// Process output of "dir" command
		boost::char_separator<char> sep("\n");
		tokenizer sources(f, sep);
		std::vector<std::string> files;
		for (auto& file : sources) {
			files.push_back(file);
		}

		std::cout << "Found " << files.size() << " project files to be configured in the copied template project" << std::endl;

		doIt(files);
	}

	void doIt(const std::vector<std::string>& files) {
		Poco::Timestamp now;

		replace_namespaces(files);
		replace_guards(files);
		rename_references(files);
		fix_settings_files(files);
		rename_classes(files);
		rename_sources(files);
		
		std::cout << "Done in " << now.elapsed() << "ms. with [" << mWarningCount << "] warnings generated.";
		if (mWarningCount > 0) {
			std::cout << "************************ WARNING  *********************** " << std::endl;
			std::cout << "Project files might be invalid. Try again until you get 0 warnings!" << std::endl;
			std::cout << "********************************************************* " << std::endl;
		}
	}

	std::string getTargetFiles() {
		std::string cmd = "dir /s /b";
		std::vector<std::string> extensions{ "h", "cpp", "vcxproj", "filters", "sln", "xml" };
		for (auto& ext : extensions)
		{
			cmd.append(" \"" + mWorkingDirectory + "\\*." + ext + "\"");
		}
		auto response = exec(cmd.c_str());
		return response;
		if (!response.empty() && !boost::iequals(trim(response), "File Not Found"))
			return response;
		else
			return "";
	}
private:
	typedef ds::ui::Sprite inherited;
	typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
	ci::params::InterfaceGlRef mParams;

	void die(const std::string& msg) {
		MessageBoxA(NULL, msg.c_str(), "project generation failed", MB_OK | MB_ICONERROR);
		exit(1);
	}

	std::string exec(const char* cmd) {
		FILE* pipe = _popen(cmd, "rt");
		if (!pipe) return "File Not Found";
		char buffer[128];
		std::string result = "";
		while (!feof(pipe)) {
			if (fgets(buffer, 128, pipe) != NULL)
				result += buffer;
		}
		_pclose(pipe);
		return result;
	}

	void str_replace(const std::string& inFilePath, const std::string& searchStr, const std::string& replacement)
	{
		boost::iostreams::mapped_file istm(inFilePath.c_str());
		if (!istm.is_open())
			return;

		std::string tempFilePath = inFilePath + ".tmp";
		std::ofstream ostm(tempFilePath.c_str(), std::ios::out | std::ios::binary);
		if (!ostm.is_open()) {
			istm.close();
			return;
		}

		boost::regex regexp(searchStr, boost::regex_constants::match_not_dot_newline);
		std::ostreambuf_iterator<char> it(ostm);
		boost::regex_replace(it, istm.begin(), istm.end(), regexp, replacement, boost::match_default | boost::format_all);

		istm.close();
		ostm.close();

		try {
			boost::filesystem::rename(tempFilePath, inFilePath);
		}
		catch (...) {
			Sleep(250);
			try {
				boost::filesystem::rename(tempFilePath, inFilePath);
			}
			catch (const std::exception&) {
				std::cout << "***************** WARNING : ELINK BUSY ****************** " << std::endl;
				std::cout << "Tried hard to rename " << tempFilePath << " but OS has still a lock on it." << std::endl;
				std::cout << "********************************************************* " << std::endl;
				mWarningCount++;
			}
		}
	}

	void rename_references(const std::vector<std::string>& files) {
		for (auto& file : files) {
			if (extension(file) == ".vcxproj" || extension(file) == ".filters" || extension(file) == ".h" || extension(file) == ".cpp") {
				std::cout << "Touching " << file << " for references..." << std::endl;
				str_replace(file, mTemplateName + "_app.cpp", boost::to_lower_copy(mAppName) + "_app.cpp");
				str_replace(file, mTemplateName + "_app.h", boost::to_lower_copy(mAppName) + "_app.h");
			}
			else if (extension(file) == ".sln") {
				std::cout << "Touching " << file << " for references..." << std::endl;
				str_replace(file, mTemplateName + ".vcxproj", boost::to_lower_copy(mProjectName) + ".vcxproj");
				str_replace(file, mTemplateName, mProjectName);
			}
		}
	}

	void fix_settings_files(const std::vector<std::string>& files) {
		for (auto& file : files) {
			if (extension(file) == ".xml") {
				std::cout << "Touching settings " << file << std::endl;
				str_replace(file, "<text\\s+name=\"project_path\"\\s+value=\"client/project\"\\s*/>", "<text name=\"project_path\" value=\"" + mProjectPath + "\" />");
				str_replace(file, "<text\\s+name=\"screen:title\"\\s+value=\"Downstream Application\"\\s*/>", "<text name=\"screen:title\" value=\"" + mWindowTitle + "\" />");
			}
		}
	}

	void rename_classes(const std::vector<std::string>& files) {
		for (auto& file : files) {
			if (extension(file) == ".h" || extension(file) == ".cpp") {
				std::cout << "Touching " << file << " for class names..." << std::endl;
				str_replace(file, "class\\s+" + mTemplateClassName, "class " + mAppName);
				str_replace(file, mTemplateClassName + "::", mAppName+"::");
				str_replace(file, "::" + mTemplateClassName, "::"+mAppName);
				str_replace(file, mTemplateClassName + "\\s*\\(", mAppName+"\\(");
			}
		}
	}

	void rename_sources(const std::vector<std::string>& files) {
		for (auto& file : files) {
			if (extension(file) == ".vcxproj") {
				std::cout << "Renaming " << file << std::endl;
				boost::filesystem::path p(file);
				boost::filesystem::rename(file, p.parent_path().generic_string() + "\\" + boost::to_lower_copy(mProjectName) + ".vcxproj");
			} else if (extension(file) == ".filters") {
				std::cout << "Renaming " << file << std::endl;
				boost::filesystem::path p(file);
				boost::filesystem::rename(file, p.parent_path().generic_string() + "\\" + boost::to_lower_copy(mProjectName) + ".vcxproj.filters");
			} else if (extension(file) == ".sln") {
				std::cout << "Renaming " << file << std::endl;
				boost::filesystem::path p(file);
				boost::filesystem::rename(file, p.parent_path().generic_string() + "\\" + boost::to_lower_copy(mSolutionName) + ".sln");
			} else {
				boost::filesystem::path p(file);
				if (p.filename() == mTemplateName + "_app.cpp") {
					std::cout << "Renaming " << file << std::endl;
					boost::filesystem::rename(file, p.parent_path().generic_string() + "\\" + boost::to_lower_copy(mAppName) + "_app.cpp");
				} else if (p.filename() == mTemplateName + "_app.h") {
					std::cout << "Renaming " << file << std::endl;
					boost::filesystem::rename(file, p.parent_path().generic_string() + "\\" + boost::to_lower_copy(mAppName) + "_app.h");
				}
			}
		}
	}

	void replace_namespaces(const std::vector<std::string>& files) {
		for (auto& file : files) {
			if (extension(file) == ".cpp" || extension(file) == ".h") {
				std::cout << "Touching " << file << " for namespace..." << std::endl;
				str_replace(file, "namespace\\s+" + mTemplateNamespace, "namespace " + mRootNamespace);
				str_replace(file, mTemplateNamespace + "::", mRootNamespace+"::");
			}
		}
	}

	void replace_guards(const std::vector<std::string>& files) {
		for (auto& file : files) {
			if (extension(file) == ".cpp" || extension(file) == ".h") {
				std::cout << "Touching " << file << " for header guard..." << std::endl;
				str_replace(file, "_" + mTemplateHeadguard + "_APP", "_"+boost::to_upper_copy(mAppName)+"_APP");
			}
		}
	}

	std::string extension(const std::string& filePath) {
		boost::filesystem::path p(filePath);
		const auto ext = p.extension().string();
		return p.extension().string();
	}

	std::string				mAppName; // OracleMediaWallApp
	std::string				mSolutionName; // oracle_media_wall.sln
	std::string				mProjectName; // oracle_media_wall.vcxproj
	std::string				mRootNamespace; // oracle
	std::string				mProjectPath; // oracle/media_wall
	std::string				mWindowTitle; // Oracle Media Wall App 
	std::string				mProjectFolder; // c:/users/you/documents/projects/whatever/path/
	std::string				mWorkingDirectory;
	std::string				mTemplatePath;
	std::string				mTemplateName;
	std::string				mTemplateClassName;
	std::string				mTemplateNamespace;
	std::string				mTemplateHeadguard;
	size_t					mWarningCount;
};

void ProjectGeneratorApp::setupServer()
{
  auto& rootSprite = mEngine.getRootSprite();
  auto& settingsSprite = ds::ui::Sprite::make<SettingsSprite>(mEngine, &rootSprite);
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC(ProjectGeneratorApp, ci::app::RendererGl(ci::app::RendererGl::AA_MSAA_4))
