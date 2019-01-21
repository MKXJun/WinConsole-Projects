#pragma once
#include <string>
#include <list>
#include <fstream>
#include <sstream>
#include <functional>
#include <algorithm>
#include <cassert>

struct PCB
{
	std::string processName;	// ������
	uint32_t processID;			// ����ID
	uint32_t priority;			// �������ȼ�
	uint32_t arriveTime;		// ����ʱ��

	uint32_t runTime;			// ��Ҫ����ʱ��
	uint32_t currRunTime;		// ��ǰ����ʱ��
	
	uint32_t startBlockTime;	// ��ʼ����ʱ��
	uint32_t blockTime;			// ��Ҫ����ʱ��
	uint32_t currBlockTime;		// ��ǰ����ʱ��

	uint32_t finishTime;		// ���ʱ��

	PCB()
	{
		memset((char*)this + sizeof(std::string), 0, 
			sizeof(PCB) - sizeof(std::string));
	}

	bool operator==(const PCB& rhs) const
	{
		return processName == rhs.processName &&
			!memcmp((char*)this + sizeof(std::string),
				(char*)&rhs + sizeof(std::string),
				sizeof(PCB) - sizeof(std::string));
	}
};


class ProcessSimulator
{
public:
	using ProcessDispatch = std::function<const PCB*(
		uint32_t currTime, 
		const std::list<PCB>& runningList, 
		const std::list<PCB>& readyList)> ;
	using JobDispatch = std::function<const PCB*(
		uint32_t currTime,
		const std::list<PCB>& readyList,
		const std::list<PCB>& idleList)>;

	enum class Mode {
		Unknown,		// δ֪
		NonPreemption,	// ����ռʽ
		Preemption,		// ��ռʽ
		Concurrent		// ����
	};

	enum class Algorithm {
		Unknown,		// δ֪
		FCFS,			// �����ȷ���
		SJF,			// �̽���(��ҵ)����
		FPF,			// ������Ȩ����
		HRRN,			// ����Ӧ������
	};

	ProcessSimulator();


	// ���ı��ļ���ȡ���н���
	bool CreateProcessFromFile(std::string txtFileName);
	// ������ҵģʽ
	void SetJobMode(Mode mode);
	// ���ý���ģʽ
	void SetProcessMode(Mode mode);
	// ����CPU��Ŀ
	void SetNumCPU(uint32_t numCPU);
	// ���þ������г���
	void SetMaxReadyLength(uint32_t length);
	// ������ҵ�����㷨
	void SetJobDispatchFunc(Algorithm algo);
	// ���ý��̵����㷨
	void SetProcessDispatchFunc(Algorithm algo);
	// ����ģ���ٶ�(��λʱ��/s)
	void SetSimSpeed(float speed);
	// ��1ʱ�䵥λ���и���
	void Update();
	// ����
	void Reset();

	// ��ǰʱ��
	uint32_t GetCurrTime() const;
	// ��ǰģʽ
	Mode GetMode() const;
	// ��ǰCPU��Ŀ
	uint32_t GetNumCPU() const;
	// ��ǰ���������г���
	uint32_t GetMaxReadyLength() const;
	// ��ȡ��ǰ��ҵ�����㷨
	Algorithm GetJobAlgorithm() const;
	// ��ȡ��ǰ���̵����㷨
	Algorithm GetProcessAlgorithm() const;
	// ��ȡ˯�ߺ�����
	uint32_t GetSleepTime() const;

	// ��ȡ���ö���
	const std::list<PCB>& GetIdleProcess() const;
	// ��ȡ��������
	const std::list<PCB>& GetBlockProcess() const;
	// ��ȡ��������
	const std::list<PCB>& GetReadyProcess() const;
	// ��ȡ��ɶ���
	const std::list<PCB>& GetFinishProcess() const;
	// ��ȡ���ж���
	const std::list<PCB>& GetRunningProcess() const;
	
	// ģ���Ƿ��Ѿ�����
	bool IsEnd() const;

private:

	static const PCB* JobFCFS(uint32_t currTime, const std::list<PCB>& readyList, const std::list<PCB>& idleList);
	static const PCB* JobSJF(uint32_t currTime, const std::list<PCB>& readyList, const std::list<PCB>& idleList);
	static const PCB* JobFPF(uint32_t currTime, const std::list<PCB>& readyList, const std::list<PCB>& idleList);
	static const PCB* ProcessFCFS(uint32_t currTime, const std::list<PCB>& runningList, const std::list<PCB>& readyList);
	static const PCB* ProcessSJF(uint32_t currTime, const std::list<PCB>& runningList, const std::list<PCB>& readyList);
	static const PCB* ProcessFPF(uint32_t currTime, const std::list<PCB>& runningList, const std::list<PCB>& readyList);
	static const PCB* ProcessHRRN(uint32_t currTime, const std::list<PCB>& runningList, const std::list<PCB>& readyList);
private:
	std::list<PCB> m_IdleProcess;			// ���н��̶���
	std::list<PCB> m_BlockProcess;			// �������̶���
	std::list<PCB> m_ReadyProcess;			// �������̶���
	std::list<PCB> m_FinishProcess;			// ��ɽ��̶���
	std::list<PCB> m_RunningProcess;		// ��ǰ���н���
	JobDispatch		m_JobDispatch;			// ��ҵ�����㷨
	ProcessDispatch m_ProcessDispatch;		// ���̵����㷨
	uint32_t m_CurrTime;					// ��ǰʱ��(û�н���ʱ��ֹͣ)
	uint32_t m_NumCPU;						// CPU��Ŀ
	uint32_t m_MaxReadyLength;				// ����������󳤶�
	uint32_t m_SleepTime;					// s/��λʱ��
	Mode m_ProcessMode;						// ��ǰ����ģʽ
	Algorithm m_JobAlgorithm;				// ��ҵ�����㷨
	Algorithm m_ProcessAlgorithm;			// ���̵����㷨
	bool m_IsEnd;							// �Ƿ����
};