#include "stdafx.h"
#include <iostream>
#include <time.h>
#include <cassert>
#include "TimerThread.h"

TimerThread::TimerThread()
: thead_handle(INVALID_HANDLE_VALUE),
stop_event(INVALID_HANDLE_VALUE), interval(0)
{
}
TimerThread::~TimerThread()
{

#ifdef _DEBUG

	std::deque<long>::iterator t = clock_log.begin();
	double dur_avg = 0.0;
	clock_t prevT = -1;
	std::cerr
		<< "---- " << std::endl
		<< "■直近" << clock_log.size()
		<< "回の実行間隔 : " << std::endl;
	for (; t != clock_log.end(); t++) {
		clock_t T = *t;
		if (prevT >= 0) {
			std::cerr << "+" << (T - prevT) << "/";
			dur_avg += (T - prevT);
		}
		prevT = T;
	}
	std::cerr << std::endl;
	if (clock_log.size() > 1) {
		dur_avg /= (clock_log.size() - 1);
		std::cerr
			<< "■平均: " << dur_avg << "[ms] "
			<< "■" << (1000.0 / dur_avg) << "回/秒"
			<< std::endl;
	}

#endif

}
void TimerThread::setInterval(int interval)
{
	assert(interval > 0);
	this->interval = interval;
}
void TimerThread::start()
{
	assert(stop_event == INVALID_HANDLE_VALUE);
	assert(thead_handle == INVALID_HANDLE_VALUE);

	//終了要求イベント
	stop_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(stop_event != NULL);

	this->interval = interval;
	thead_handle = ::CreateThread(0, 0,
		TimerThread::ThreadMain, (LPVOID)this,
		0, NULL);
	assert(thead_handle != NULL);
}
void TimerThread::stop()
{
	assert(stop_event != NULL);
	assert(thead_handle != NULL);

	::SetEvent(stop_event);
	
	WaitForSingleObject(thead_handle, INFINITE);
	::CloseHandle(thead_handle);
	thead_handle = INVALID_HANDLE_VALUE;

	::CloseHandle(stop_event);
	stop_event = INVALID_HANDLE_VALUE;
}

DWORD WINAPI TimerThread::ThreadMain(LPVOID data)
{
	TimerThread* thread = (TimerThread*)data;
	clock_t sleep_t = 0;			//累積待ち時間
	long run_counter = 0x7fffffff;	//実行カウンター
	clock_t t0 = 0;					//初回実行時刻
	
	while (::WaitForSingleObject(thread->stop_event, 0) != WAIT_OBJECT_0) {
		
		//カウンターリセット
		if (run_counter > 0x7ffffff0) {
			run_counter = 0;
			t0 = clock();
		}

		//タイマー処理実行
		thread->run();
		
		run_counter++;
		
		//時間待ち
		sleep_t += ((t0 + run_counter * thread->interval) - clock());
		if (sleep_t > 0) {
			Sleep(sleep_t);
			sleep_t = 0;
		}
		else {
			Sleep(1);
			sleep_t -= 1;
		}

#ifdef _DEBUG

		//実行間隔計測
		clock_t t = clock();
		thread->clock_log.push_back(t);
		if (thread->clock_log.size() > 10000) {
			thread->clock_log.pop_front();
		}
		
#endif

	}
	::ExitThread(0);
	return 0;
}
