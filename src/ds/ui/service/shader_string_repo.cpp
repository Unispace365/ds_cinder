#include "stdafx.h"

#include "shader_string_repo.h"

#include <filesystem>

#include <ds/util/ada/tools/fs.h>
namespace ds::ui {
std::shared_ptr<ShaderStringRepository> ShaderStringRepository::mDefaultRepo =
	std::shared_ptr<ShaderStringRepository>();

ShaderStringRepository::ShaderStringRepository() {}

std::shared_ptr<ShaderStringRepository>
ShaderStringRepository::setDefaultRepository(std::shared_ptr<ShaderStringRepository>& repo) {
	if (repo != mDefaultRepo) {
		std::shared_ptr<ShaderStringRepository> retval = mDefaultRepo;
		mDefaultRepo								   = repo;
		return retval;
	} else {
		return mDefaultRepo;
	}
}

std::shared_ptr<ShaderStringRepository> ShaderStringRepository::getDefaultRepository() {
	if (!mDefaultRepo) {
		mDefaultRepo = std::shared_ptr<ShaderStringRepository>(new ShaderStringRepository());
		mDefaultRepo->addIncludeDirectory(ds::Environment::expand("%APP%/data/shaders/include"));
	}
	return mDefaultRepo;
}

void ShaderStringRepository::addIncludeDirectory(std::string includeDirectory) {
	if (std::find(mIncludeDirectories.begin(), mIncludeDirectories.end(), includeDirectory) ==
		mIncludeDirectories.end()) {

		if (std::filesystem::exists(includeDirectory)) {
			mIncludeDirectories.push_back(includeDirectory);
		} else {
			DS_LOG_ERROR("Attempted to add directory: " << includeDirectory << " but it does not exist");
		}
	}
}

std::string ShaderStringRepository::getShader(std::string infile) {
	auto file = ds::Environment::expand(infile);
	if (mRepository.find(file) == mRepository.end()) {
		std::string result = "";
		ada::List	mdeps  = ada::List();
		ada::loadFromPath(file, &result, mIncludeDirectories, &mdeps);
		if (!result.empty()) {
			mRepository[file] = result;
			return mRepository[file];
		} else {
			return "";
		}
	} else {
		return mRepository[file];
	}
}

void ShaderStringRepository::clearRepository() {
	mRepository.clear();
}
} // namespace ds::ui