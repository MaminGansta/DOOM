#pragma once

#include <chrono>
#include <thread>
using namespace std::chrono;

namespace m
{
	namespace Timer
	{
		const bool FRAMELOCK60 = true;


		int nFrameTime = 0;
		int nFrameTimeSum = 0; // measure one second
		int nFPS = 0;
		float fFrameTimeAvg = 0;
		int nFrames = 0;
		bool bFrameLock = false;

		high_resolution_clock::time_point last_time;
		high_resolution_clock::time_point init_time;

		void timer_init(bool frameLock = false)
		{				
			init_time = last_time = high_resolution_clock::now();
			bFrameLock = frameLock;
		}

		void timer_update()
		{
			high_resolution_clock::time_point now = high_resolution_clock::now();
			int time_dif = (int)(duration_cast<microseconds>(now - last_time).count());
			last_time = now;

			nFrameTime = time_dif;
			nFrameTimeSum += nFrameTime;
			nFrames++;

			if (bFrameLock && nFrameTime < 16700)
			{
				int dif = 16700 - nFrameTime;
				nFrameTime = 16700;
				nFrameTimeSum += dif;
				std::this_thread::sleep_for(microseconds(dif));
			}

			if (nFrameTimeSum > 1e6)
			{
				fFrameTimeAvg = (float)nFrameTimeSum / (float)nFrames / 1000;
				nFPS = nFrames;
				nFrameTimeSum = 0;
				nFrames = 0;
			}
		}

		int time_from_init()
		{
			high_resolution_clock::time_point now = high_resolution_clock::now();
			return (int)(duration_cast<microseconds>(now - init_time).count());
		}
	}
}