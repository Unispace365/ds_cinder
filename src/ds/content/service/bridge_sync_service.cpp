#include "stdafx.h"

#include "bridge_sync_service.h"

#include <iostream>

#include <Poco/CountingStream.h>
#include <Poco/PipeStream.h>
#include <Poco/StreamCopier.h>

#include <cinder/CinderImGui.h>
#include <cinder/CinderImGuiConfig.h>

#include <ds/debug/logger.h>

#include <imgui_components/TextAnsi.h>


namespace ds::content {

BridgeSyncService::BridgeSyncService(ds::ui::SpriteEngine& eng)
  : AutoUpdate(eng)
  , mEngine(eng)
  , mProcessId(0) {

	mStdoutBuffer.clear();
}

BridgeSyncService::~BridgeSyncService() {
	// clean up if we are destroyed properly
	if (mStarted && Poco::Process::isRunning(mProcessId)) {
		mExit = true;
		Poco::Process::kill(mProcessId);
	}
	mThreadObj.join();
}

void BridgeSyncService::initialize(const BridgeSyncSettings& settings) {

	// create a job to hold the Sync Process (so it quits when we quit)
	// if the job is already made, it will return that one, so its okay to call this more than once.
	mJobObj = CreateJobObjectA(nullptr, "larry");
	if (mJobObj != 0) {
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION info;
		info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		SetInformationJobObject(mJobObj, JobObjectExtendedLimitInformation, &info, sizeof(info));
	} else {
		DS_LOG_ERROR("BridgeSyncService (bridgesync): Could not create job for sync process. Aborting");
		return;
	}

	// if we are already tracking a process, kill it for restart.
	if (mStarted && Poco::Process::isRunning(mProcessId)) {
		mExit = true;
		mThreadObj.join();
		Poco::Process::kill(mProcessId);
		mExit = false;
	}

	// mPath = path;
	mThreadObj = std::thread([this, settings]() {
		while (!mExit) {

			// set up the process
			Poco::Process::Args args;
			if (!settings.server.empty()) {
				args.push_back("-s");
				args.push_back(settings.server);
			}
			if (!settings.authServer.empty()) {
				args.push_back("--authServer");
				args.push_back(settings.authServer);
			}
			if (!settings.clientId.empty()) {
				args.push_back("--clientId");
				args.push_back(settings.clientId);
			}
			if (!settings.clientSecret.empty()) {
				args.push_back("--clientSecret");
				args.push_back(settings.clientSecret);
			}
			if (!settings.directory.empty()) {
				args.push_back("-d");
				args.push_back(settings.directory);
			}
			// optionals
			if (!settings.interval.empty()) {
				args.push_back("-i");
				args.push_back(settings.interval);
			}
			if (settings.verbose) {
				args.push_back("-v");
			}
			std::string sync_path;
			if (settings.syncPath.empty()) {
				//Default path on production
				sync_path = "%APP%/bridgesync/bridge_sync_console.exe";
			}
			else {
				sync_path = settings.syncPath;
			}

			if (std::filesystem::exists(sync_path)) {
				// mLock.lock();
				auto process = Poco::Process::launch(sync_path, args, nullptr, &mOutPipe, &mErrPipe);

				if (Poco::Process::isRunning(process)) {
					// get the win32 (as opposed to Poco) handle for the process we just started.
					HANDLE procHandle = OpenProcess(PROCESS_ALL_ACCESS, false, process.id());
					// add it to our job.
					AssignProcessToJobObject(mJobObj, procHandle);

					mProcessId = process.id();
					mStarted   = true;
					DS_LOG_INFO("BridgeSyncService (bridgesync): Started bridgesync");
				} else {
					DS_LOG_ERROR("BridgeSyncService (bridgesync): Failed to start bridgesync");
					mExit	 = true;
					mStarted = false;
				}

				// mLock.unlock();
			} else {
				DS_LOG_ERROR("BridgeSyncService (bridgesync): bridge_sync_console.exe not found at " << sync_path);
				mExit	 = true;
				mStarted = false;
			}

			// start collecting downsync's output
			auto pipe_stream = Poco::PipeInputStream(mOutPipe);


			std::string line;

			while (!mExit && Poco::Process::isRunning(mProcessId)) {
				if (pipe_stream.peek() != EOF) {
					std::getline(pipe_stream, line);
					if (line != "") {
						std::unique_lock<std::mutex> lock(mMutex);
						mStdoutBuffer.push_back(line);
						if (mFirstLine == "") {
							mFirstLine = mStdoutBuffer[0];
						}
						mTestScroll = true;
					}
				}

				while (mStdoutBuffer.size() > 5000) {
					std::unique_lock<std::mutex> lock(mMutex);
					mStdoutBuffer.pop_front();
				}
			}
		}
	});
}

void BridgeSyncService::update(const ds::UpdateParams&) {
	if (mStarted && mShowOutput) {
		std::unique_lock<std::mutex> lock(mMutex);
		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		ImGui::Begin("BridgeSync", &mShowOutput, ImGuiWindowFlags_NoFocusOnAppearing);

		ImGui::BeginChild("Scrolling");

		auto mx	 = ImGui::GetScrollMaxY();
		auto scy = ImGui::GetScrollY();

		auto offset = mStdoutBuffer.size() < mShowCount ? 0 : mStdoutBuffer.size() - mShowCount;
		// auto offset = mStdoutBuffer.size();
		auto start = mStdoutBuffer.begin() + offset;
		for (auto itr = start; itr != mStdoutBuffer.end(); ++itr)
			ImGui::TextAnsi(itr->c_str());


		if (mTestScroll) {
			mTestScroll = false;
			if (mx == scy) {
				ImGui::SetScrollHereY(1.0f);
			}
		}

		/*
		std::stringstream ss;
		ss << "max:" << mx << " ht:"<<ht;
		std::string out = ss.str();
		ImGui::Text(out.c_str());
		*/

		ImGui::EndChild();
		ImGui::End();
	}
}


} // namespace ds::content
