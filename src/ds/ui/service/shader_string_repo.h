#pragma once
#include <ds/util/ada/tools/list.h>
#include <stdafx.h>

namespace ds::ui {
class ShaderStringRepository {
  public:
	ShaderStringRepository();

	/// <summary>
	/// set the default repository while returning the previous default repository or an empty shared pointer
	/// </summary>
	/// <param name="repo">Shared Pointer to a ShaderStringRepository</param>
	/// <returns>Shared Pointer to the previous ShaderStringRepository</returns>
	static std::shared_ptr<ShaderStringRepository> setDefaultRepository(std::shared_ptr<ShaderStringRepository>& repo);

	/// <summary>
	/// Get the default shared repository that was set with setDefaultRepository or creates one if none was set.
	/// </summary>
	/// <returns>shared pointer to the default repository</returns>
	static std::shared_ptr<ShaderStringRepository> getDefaultRepository();

	void addIncludeDirectory(std::string includeDirectory);
	// void addIncludeDirectories(std::vector<std::string> includeDirectories);
	// void clearIncludeDirectories();


	std::string getShader(std::string file);
	// std::string fillIncludesInString(std::strng shader);
	void clearRepository();

  protected:
	void										   cacheFromFile(std::string file);
	static std::shared_ptr<ShaderStringRepository> mDefaultRepo;
	std::unordered_map<std::string, std::string>   mRepository;
	std::vector<std::string>					   mIncludeDirectories;
	// ds::Engine& mEngine;
};
} // namespace ds::ui
