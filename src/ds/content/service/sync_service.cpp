#include "stdafx.h"

#include "sync_service.h"

#include <iostream>
#include <Poco/PipeStream.h>
#include <cinder/CinderImGui.h>
#include <cinder/CinderImGuiConfig.h>
#include <imgui_components/TextAnsi.h>
#include <Poco/CountingStream.h>
#include <Poco/StreamCopier.h>
#include <ds/debug/logger.h>


namespace ds::content {

	SyncService::SyncService(ds::ui::SpriteEngine& eng)
		: AutoUpdate(eng), mEngine(eng), mProcessId(0)
	{
		
		mStdoutBuffer.clear();
	}

	SyncService::~SyncService()
	{
		//clean up if we are destroyed properly
		if (mStarted && Poco::Process::isRunning(mProcessId)){
			mExit = true;
			Poco::Process::kill(mProcessId);
		}
		mThreadObj.join();
	}

	void SyncService::initialize(const SyncSettings& settings)
	{

		//create a job to hold the Sync Process (so it quits when we quit)
		//if the job is already made, it will return that one, so its okay to call this more than once.
		mJobObj = CreateJobObjectA(nullptr, "larry");
		if (mJobObj != 0) {
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION info;
			info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
			SetInformationJobObject(mJobObj, JobObjectExtendedLimitInformation, &info, sizeof(info));
		}
		else {
			DS_LOG_ERROR("SyncService (downsync): Could not create job for sync process. Aborting");
			return;
		}

		//if we are already tracking a process, kill it for restart.
		if (mStarted && Poco::Process::isRunning(mProcessId)) {
			mExit = true;
			mThreadObj.join();
			Poco::Process::kill(mProcessId);
			mExit = false;
		}

		//mPath = path;
		mThreadObj = std::thread([this,settings]() {
			while (!mExit) {

				//set up the process
				Poco::Process::Args args;
				if (!settings.name.empty()) {
					args.push_back("-n");
					args.push_back(settings.name);
				}
				if (!settings.server.empty()) {
					args.push_back("-s");
					args.push_back(settings.server);
				}
				if (!settings.token.empty()) {
					args.push_back("-t");
					args.push_back(settings.token);
				}
				if (!settings.directory.empty()) {
					args.push_back("-d");
					args.push_back(settings.directory);
				}
				//optionals
				if (!settings.interval.empty()) {
					args.push_back("--interval");
					args.push_back(settings.interval);
				}
				if (!settings.rate_decay.empty()) {
					args.push_back("--rate-decay");
					args.push_back(settings.rate_decay);
				}
				if (!settings.rate_qty.empty()) {
					args.push_back("--rate-qty");
					args.push_back(settings.rate_qty);
				}
				if (!settings.udp_port.empty()) {
					args.push_back("--udp-port");
					args.push_back(settings.udp_port);
				}
				if (!settings.verbosity.empty()) {
					if(settings.verbosity=="v") args.push_back("-v");
					if (settings.verbosity== "vv") args.push_back("-vv");
					if (settings.verbosity== "vvv") args.push_back("-vvv");
				}
				auto sync_path = ds::Environment::expand("%APP%/downsync/downsync.exe");
				
				if (std::filesystem::exists(sync_path)) {
					//mLock.lock();
					auto process = Poco::Process::launch(sync_path, args, nullptr, &mOutPipe, &mErrPipe);

					if (Poco::Process::isRunning(process)) {
						//get the win32 (as opposed to Poco) handle for the process we just started.
						HANDLE procHandle = OpenProcess(PROCESS_ALL_ACCESS, false, process.id());
						//add it to our job.
						AssignProcessToJobObject(mJobObj, procHandle);

						mProcessId = process.id();
						mStarted = true;
						DS_LOG_INFO("SyncService (downsync): Started Downsync");
					}
					else {
						DS_LOG_ERROR("SyncService (downsync): Failed to start downsync");
						mExit = true;
						mStarted = false;
					}

					//mLock.unlock();
				}
				else {
					DS_LOG_ERROR("SyncService (downsync): downsync.exe not found at " << sync_path);
					mExit = true;
					mStarted = false;
				}

				//start collecting downsync's output
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

	void SyncService::update(const ds::UpdateParams&)
	{
		if (mStarted && mShowOutput) {
			std::unique_lock<std::mutex> lock(mMutex);
			ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
			ImGui::Begin("DownSync", &mShowOutput);
		
			ImGui::BeginChild("Scrolling");
			
			auto mx = ImGui::GetScrollMaxY();
			auto scy = ImGui::GetScrollY();

			auto offset = mStdoutBuffer.size() < mShowCount ? 0 : mStdoutBuffer.size() - mShowCount;
			//auto offset = mStdoutBuffer.size();
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


}  // namespace ds
