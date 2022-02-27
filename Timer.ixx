// Copyright AceBilly
module;
#include <Windows.h>
export module Timer;

export class Timer {
public:
	Timer();
	~Timer() = default;
public:
	// ��֮֡��ʱ����
	double getDeltaTime()const;
	// �����ȡ���Ƕ������е�ʱ�䣬�����ͣ�Ļ�������ʱ���С�
	double getTotalTime();
public:
	// ����״̬
	void reset();
	void stop();
	void resume();
	// ÿ֡����
	void tick();
	// ������ת��Ϊ��
	double convertToSecond(__int64 counter) const;
private:
	// 
	double m_secondsPerCount;
	// ͣ������ʱ�䣻
	bool m_stoppedFlag;
	__int64 m_stoppedTime;
	// ���ڼ���ÿ֡��Ҫ��ʱ���Ƕ���
	double m_deltaTime;
	__int64 m_currentTime, m_prevTime;
	// ��ͣ����ʱ�� 
	double m_pauseTime;

	// �Ӷ��󴴽���Ŀǰ����ʱ�䣻
	__int64 m_originTime;  // ��ʼ��ʱ��
	double m_totalTime;
private:
	// ��ѯϵͳʱ��
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
//�������ͣ״̬��ôdeltaT ����0 ������ʲô������
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