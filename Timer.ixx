// Copyright AceBilly
module;
#include <Windows.h>
export module Timer;

export class Timer {
public:
	Timer();
	~Timer() = default;
public:
	// 两帧之间时间间隔
	double getDeltaTime()const;
	// 这里获取的是对象运行的时间，如果暂停的话不计入时间中。
	double getTotalTime();
public:
	// 重置状态
	void reset();
	void stop();
	void resume();
	// 每帧调用
	void tick();
	// 将计数转换为秒
	double convertToSecond(__int64 counter) const;
private:
	// 
	double m_secondsPerCount;
	// 停下来的时间；
	bool m_stoppedFlag;
	__int64 m_stoppedTime;
	// 用于计算每帧需要的时间是多少
	double m_deltaTime;
	__int64 m_currentTime, m_prevTime;
	// 暂停的总时常 
	double m_pauseTime;

	// 从对象创建到目前的总时间；
	__int64 m_originTime;  // 起始的时间
	double m_totalTime;
private:
	// 查询系统时间
	__int64 querySystemTime();
};

module : private;
void Timer::stop() {
	if (!m_stoppedFlag) {
		m_stoppedFlag = true;
		m_stoppedTime = querySystemTime();
	}
}
double Timer::convertToSecond(__int64 count) const { return m_secondsPerCount * count; }
void Timer::resume() {
	if (m_stoppedFlag) {
		__int64 _startTime = querySystemTime();
		m_stoppedFlag = false;
		m_pauseTime += convertToSecond(_startTime - m_stoppedTime);
		m_prevTime = _startTime;
		m_stoppedTime = 0;
	}

}
//如果是暂停状态那么deltaT 等于0 其他的什么都不做
void Timer::tick() {
	if (m_stoppedFlag) {
		m_deltaTime = 0.f;
		return;
	}
	m_currentTime = querySystemTime();
	m_deltaTime = convertToSecond(m_currentTime - m_prevTime);
	m_prevTime = m_currentTime;
	if (m_deltaTime < 0.f) { m_deltaTime = 0.f; }

}
Timer::Timer():m_stoppedFlag(false),m_stoppedTime(0), 
m_originTime(querySystemTime()),m_pauseTime(0), m_deltaTime(-1.0), m_totalTime(0) {
	m_currentTime = m_prevTime = m_originTime;
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_secondsPerCount = 1.f / static_cast<double>(countsPerSec);
}
__int64 Timer::querySystemTime() {
	__int64 result;
	QueryPerformanceCounter((LARGE_INTEGER*)(&result));
	return result;
}

void Timer::reset() {
	m_currentTime = m_prevTime = querySystemTime();
	m_stoppedFlag = false;
	m_stoppedTime = 0;
}
double Timer::getDeltaTime()const {
	return m_deltaTime;
}
double Timer::getTotalTime() {
	m_totalTime = convertToSecond(m_currentTime - m_originTime);
	return m_totalTime - m_pauseTime;
}