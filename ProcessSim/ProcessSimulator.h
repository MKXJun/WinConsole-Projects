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
	std::string processName;	// 进程名
	uint32_t processID;			// 进程ID
	uint32_t priority;			// 进程优先级
	uint32_t arriveTime;		// 到达时间

	uint32_t runTime;			// 需要运行时间
	uint32_t currRunTime;		// 当前运行时间
	
	uint32_t startBlockTime;	// 开始阻塞时间
	uint32_t blockTime;			// 需要阻塞时间
	uint32_t currBlockTime;		// 当前阻塞时间

	uint32_t finishTime;		// 完成时间

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
		Unknown,		// 未知
		NonPreemption,	// 非抢占式
		Preemption,		// 抢占式
		Concurrent		// 并发
	};

	enum class Algorithm {
		Unknown,		// 未知
		FCFS,			// 先来先服务
		SJF,			// 短进程(作业)优先
		FPF,			// 高优先权优先
		HRRN,			// 高响应比优先
	};

	ProcessSimulator();


	// 从文本文件读取所有进程
	bool CreateProcessFromFile(std::string txtFileName);
	// 设置作业模式
	void SetJobMode(Mode mode);
	// 设置进程模式
	void SetProcessMode(Mode mode);
	// 设置CPU数目
	void SetNumCPU(uint32_t numCPU);
	// 设置就绪队列长度
	void SetMaxReadyLength(uint32_t length);
	// 设置作业调度算法
	void SetJobDispatchFunc(Algorithm algo);
	// 设置进程调度算法
	void SetProcessDispatchFunc(Algorithm algo);
	// 设置模拟速度(单位时间/s)
	void SetSimSpeed(float speed);
	// 按1时间单位进行更新
	void Update();
	// 重置
	void Reset();

	// 当前时间
	uint32_t GetCurrTime() const;
	// 当前模式
	Mode GetMode() const;
	// 当前CPU数目
	uint32_t GetNumCPU() const;
	// 当前最大就绪队列长度
	uint32_t GetMaxReadyLength() const;
	// 获取当前作业调度算法
	Algorithm GetJobAlgorithm() const;
	// 获取当前进程调度算法
	Algorithm GetProcessAlgorithm() const;
	// 获取睡眠毫秒数
	uint32_t GetSleepTime() const;

	// 获取闲置队列
	const std::list<PCB>& GetIdleProcess() const;
	// 获取阻塞队列
	const std::list<PCB>& GetBlockProcess() const;
	// 获取就绪队列
	const std::list<PCB>& GetReadyProcess() const;
	// 获取完成队列
	const std::list<PCB>& GetFinishProcess() const;
	// 获取运行队列
	const std::list<PCB>& GetRunningProcess() const;
	
	// 模拟是否已经结束
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
	std::list<PCB> m_IdleProcess;			// 空闲进程队列
	std::list<PCB> m_BlockProcess;			// 阻塞进程队列
	std::list<PCB> m_ReadyProcess;			// 就绪进程队列
	std::list<PCB> m_FinishProcess;			// 完成进程队列
	std::list<PCB> m_RunningProcess;		// 当前运行进程
	JobDispatch		m_JobDispatch;			// 作业调度算法
	ProcessDispatch m_ProcessDispatch;		// 进程调度算法
	uint32_t m_CurrTime;					// 当前时间(没有进程时将停止)
	uint32_t m_NumCPU;						// CPU数目
	uint32_t m_MaxReadyLength;				// 就绪队列最大长度
	uint32_t m_SleepTime;					// s/单位时间
	Mode m_ProcessMode;						// 当前进程模式
	Algorithm m_JobAlgorithm;				// 作业调度算法
	Algorithm m_ProcessAlgorithm;			// 进程调度算法
	bool m_IsEnd;							// 是否结束
};