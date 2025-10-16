#include "stdafx.h"

#include "bridge_sync_service.h"

#include "ds/util/float_util.h"

#include <iostream>

#include <Poco/CountingStream.h>
#include <Poco/PipeStream.h>
#include <Poco/StreamCopier.h>

#include <cinder/CinderImGui.h>

#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include <imgui_components/TextAnsi.h>

namespace {

std::string errorToString(DWORD errorMessageId) {
	if (errorMessageId == 0) return {};

	// Ask Win32 to give us the string version of that message ID.
	// The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet
	// know how long the message string will be).
	LPSTR  messageBuffer = nullptr;
	size_t size			 = FormatMessageA(
		 FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
		 errorMessageId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, nullptr);

	// Copy the error message into a std::string.
	std::string message(messageBuffer, size - 2); // remove the \r\n

	// Free the Win32's string's buffer.
	LocalFree(messageBuffer);

	return message;
}

} // namespace

namespace ds::content {

BridgeSyncService::BridgeSyncService(ds::ui::SpriteEngine& engine)
  : AutoUpdate(engine) {
	mThread.setName("BridgeSyncService");
}

BridgeSyncService::~BridgeSyncService() {
	stop();

	try {
		mThread.join();
	} catch (std::exception&) {}
}

void BridgeSyncService::update(const ds::UpdateParams&) {
	if (mThread.isRunning() && mShowOutput) {
		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		ImGui::Begin("BridgeSync", &mShowOutput, ImGuiWindowFlags_NoFocusOnAppearing);

		ImGui::BeginChild("Scrolling");

		auto mx			= ImGui::GetScrollMaxY();
		auto scy		= ImGui::GetScrollY();
		bool autoScroll = ds::approxEqual(mx, scy);

		bool hasNewContent = mLoop.readBuffer(mStdoutBuffer);
		while (mStdoutBuffer.size() > 5000) {
			mStdoutBuffer.pop_front();
		}

		auto offset = mStdoutBuffer.size() < mShowCount ? 0 : mStdoutBuffer.size() - mShowCount;
		// auto offset = mStdoutBuffer.size();
		auto start = mStdoutBuffer.begin() + offset;
		for (auto itr = start; itr != mStdoutBuffer.end(); ++itr)
			ImGui::TextAnsi(itr->c_str());

		if (autoScroll && hasNewContent) {
			ImGui::SetScrollHereY(1.0f);
		}

		ImGui::EndChild();
		ImGui::End();
	}
}

void BridgeSyncService::start(const BridgeSyncSettings& settings) {
	stop();

	if (!mThread.isRunning()) {
		mLoop.setSettings(settings);

		try {
			mThread.start(mLoop);
		} catch (std::exception& ex) {
			DS_LOG_WARNING("BridgeSyncService::start() threw an exception: " << ex.what())
		}
	}
}

void BridgeSyncService::stop() {
	if (mThread.isRunning()) {
		try {
			mLoop.abort();
			mThread.wakeUp();
		} catch (std::exception& ex) {
			DS_LOG_WARNING("BridgeSyncService::stop() threw an exception: " << ex.what())
		}
	}
}

void BridgeSyncService::Loop::run() {
	mExit = false;

	clearBuffer();

	// create a job to hold the Sync Process (so it quits when we quit)
	// if the job is already made, it will return that one, so it's okay to call this more than once.
	HANDLE jobObject = CreateJobObjectA(nullptr, "larry");
	if (jobObject != nullptr) {
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION info;
		info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		SetInformationJobObject(jobObject, JobObjectExtendedLimitInformation, &info, sizeof(info));
	} else {
		DS_LOG_ERROR("BridgeSyncService (BridgeSync): Could not create job for sync process. Aborting");
		return;
	}

	//
	Poco::Process::PID processId = 0;

	while (!mExit) {
		Poco::Thread::trySleep(1000);

		Poco::Process::Args args;
		if (!mSettings.server.empty()) {
			args.emplace_back("-s");
			args.push_back(mSettings.server);
		}
		if (!mSettings.authServer.empty()) {
			args.emplace_back("--authServer");
			args.push_back(mSettings.authServer);
		}
		if (!mSettings.clientId.empty()) {
			args.emplace_back("--clientId");
			args.push_back(mSettings.clientId);
		}
		if (!mSettings.clientSecret.empty()) {
			args.emplace_back("--clientSecret");
			args.push_back(mSettings.clientSecret);
		}
		if (!mSettings.directory.empty()) {
			args.emplace_back("-d");
			args.push_back(mSettings.directory);
		}
		// optionals
		if (!mSettings.interval.empty()) {
			args.emplace_back("-i");
			args.push_back(mSettings.interval);
		}
		if (mSettings.verbose) {
			args.emplace_back("-v");
		}

		// Handle additional args in format "--singleArg;--Another"
		// and/or in format "-s: server; --singleArg"
		if (!mSettings.additionalArgs.empty()) {
			auto splitAdditional = ds::split(mSettings.additionalArgs, ";");
			for (const auto& kv : splitAdditional) {
				auto pair = ds::split(kv, ":");
				if (!pair.empty()) {
					args.push_back(pair[0]);
				}
				if (pair.size() >= 2) {
					args.push_back(pair[1]);
				}
			}
		}

		// Default path on production
		std::string sync_path = ds::Environment::expand("%APP%/bridgesync/bridge_sync_console.exe");
		if (!mSettings.syncPath.empty()) {
			sync_path = ds::Environment::expand(mSettings.syncPath);
		}

		if (std::filesystem::exists(sync_path)) {

			try {
				auto process = Poco::Process::launch(sync_path, args, nullptr, &mOutPipe, &mErrPipe);
				Sleep(100);

				if (Poco::Process::isRunning(process)) {
					// get the win32 (as opposed to Poco) handle for the process we just started.
					HANDLE procHandle = OpenProcess(PROCESS_ALL_ACCESS, false, process.id());
					// add it to our job.
					AssignProcessToJobObject(jobObject, procHandle);

					processId = process.id();
					DS_LOG_INFO("BridgeSyncService (BridgeSync): Started BridgeSync");
				} else {
					DS_LOG_ERROR("BridgeSyncService (BridgeSync): Failed to start BridgeSync: "
								 << errorToString(process.wait()));
					Poco::PipeInputStream inErr(mErrPipe);
					Poco::PipeInputStream inStd(mOutPipe);
					for (std::string line; std::getline(inErr, line);) {
						DS_LOG_ERROR("BridgeSync Error: " << line);
					}

					for (std::string line; std::getline(inStd, line);) {
						DS_LOG_ERROR("BridgeSync Std: " << line);
					}

					DS_LOG_ERROR("BridgeSyncService (BridgeSync): Failed to start BridgeSync")

					mExit = true;
				}
			} catch (const std::exception& e) {
				DS_LOG_ERROR("BridgeSyncService (BridgeSync): Failed to start BridgeSync: " << e.what());
				mExit = true;
			}

		} else {
			DS_LOG_ERROR("BridgeSyncService (BridgeSync): bridge_sync_console.exe not found at " << sync_path
																								 << std::endl
																								 << std::flush);
			mExit = true;
		}

		if (!mExit) {
			auto pipe_stream = Poco::PipeInputStream(mOutPipe);

			std::string line;
			while (!mExit && Poco::Process::isRunning(processId)) {
				if (pipe_stream.peek() != EOF) {
					std::getline(pipe_stream, line);
					if (!line.empty()) {
						Poco::Mutex::ScopedLock lock(mMutex);
						mStdoutBuffer.push_back(line);

						while (mStdoutBuffer.size() > 5000) {
							mStdoutBuffer.pop_front();
						}
					}
				}

				Poco::Thread::yield();
			}
		}
	}

	if (Poco::Process::isRunning(processId)) Poco::Process::kill(processId);

	CloseHandle(jobObject);
}

void BridgeSyncService::Loop::clearBuffer() {
	Poco::Mutex::ScopedLock lock(mMutex);
	mStdoutBuffer.clear();
}

bool BridgeSyncService::Loop::readBuffer(std::deque<std::string>& buffer) {
	Poco::Mutex::ScopedLock lock(mMutex);
	if (mStdoutBuffer.empty()) return false;
	for (const auto& line : mStdoutBuffer)
		buffer.push_back(line);
	mStdoutBuffer.clear();
	return true;
}


} // namespace ds::content
