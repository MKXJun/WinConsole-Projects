#include "ProcessSimulator.h"

ProcessSimulator::ProcessSimulator()
	: m_CurrTime(0), m_IsEnd(false),
	m_JobDispatch(JobFCFS), m_ProcessDispatch(ProcessFCFS),
	m_NumCPU(1), m_MaxReadyLength(UINT16_MAX), 
	m_ProcessMode(ProcessSimulator::Mode::NonPreemption),
	m_ProcessAlgorithm(ProcessSimulator::Algorithm::FCFS),
	m_JobAlgorithm(ProcessSimulator::Algorithm::FCFS),
	m_SleepTime(1000)
{
}

bool ProcessSimulator::CreateProcessFromFile(std::string txtFileName)
{
	std::ifstream fin(txtFileName);
	if (!fin.is_open())
		return false;

	PCB pcb;
	pcb.currRunTime = pcb.processID = 0;

	std::string line;
	
	while (!fin.eof())
	{
		std::getline(fin, line);
		// 排除注释
		if (line[0] != '#')
		{
			std::istringstream iss(line);
			if (iss >> pcb.processName >> pcb.priority
				>> pcb.arriveTime >> pcb.runTime >>
				pcb.startBlockTime >> pcb.blockTime)
			{
				assert(pcb.startBlockTime >= 0 && pcb.startBlockTime <= pcb.runTime);
				m_IdleProcess.push_back(pcb);
				++pcb.processID;	// 新进程ID
			}
			else
				continue;
		}
	}

	fin.close();

	// 按到达时间排序
	m_IdleProcess.sort([](const PCB& lhs, const PCB& rhs) {
		return lhs.arriveTime < rhs.arriveTime;
	});

	return true;

}

void ProcessSimulator::SetProcessMode(Mode mode)
{
	m_ProcessMode = mode;
}

void ProcessSimulator::SetNumCPU(uint32_t numCPU)
{
	m_NumCPU = numCPU;
}

void ProcessSimulator::SetMaxReadyLength(uint32_t length)
{
	m_MaxReadyLength = length;
}

void ProcessSimulator::SetJobDispatchFunc(Algorithm algo)
{
	m_JobAlgorithm = algo;
	switch (algo)
	{
	case Algorithm::FCFS: m_JobDispatch = JobFCFS; break;
	case Algorithm::SJF: m_JobDispatch = JobSJF; break;
	case Algorithm::FPF: m_JobDispatch = JobFPF; break;
	default: m_JobDispatch = nullptr; 
		m_JobAlgorithm = Algorithm::Unknown;
		break;
	}
}

void ProcessSimulator::SetProcessDispatchFunc(Algorithm algo)
{
	m_ProcessAlgorithm = algo;
	switch (algo)
	{
	case Algorithm::FCFS: m_ProcessDispatch = ProcessFCFS; break;
	case Algorithm::SJF: m_ProcessDispatch = ProcessSJF; break;
	case Algorithm::FPF: m_ProcessDispatch = ProcessFPF; break;
	case Algorithm::HRRN: m_ProcessDispatch = ProcessHRRN; break;
	default: m_ProcessDispatch = nullptr;
		m_ProcessAlgorithm = Algorithm::Unknown;
		break;
	}
}

void ProcessSimulator::SetSimSpeed(float speed)
{
	assert(speed > 0);
	m_SleepTime = static_cast<int>(1000 / speed);
}




void ProcessSimulator::Update()
{
	// 进程已经执行完成就直接退出
	if (m_IdleProcess.empty() && m_ReadyProcess.empty() && m_RunningProcess.empty())
	{
		m_IsEnd = true;
		return;
	}


	//
	// 阻塞进程-->就绪进程
	//

	uint32_t numIdleProcess = (uint32_t)m_IdleProcess.size();
	uint32_t numBlockProcess = (uint32_t)m_BlockProcess.size();
	uint32_t numReadyProcess = (uint32_t)m_ReadyProcess.size();
	uint32_t numRunningProcess = (uint32_t)m_RunningProcess.size();

	uint32_t numAllowProcess = std::min(m_MaxReadyLength, numReadyProcess + numBlockProcess);

	// 更新阻塞进程
	for (PCB& pcb : m_BlockProcess)
	{
		++pcb.currBlockTime;
		if (pcb.currBlockTime >= pcb.blockTime)
			pcb.currBlockTime = pcb.blockTime;
	}

	while (numReadyProcess < numAllowProcess)
	{
		// 寻找合适进程
		auto it = std::find_if(m_BlockProcess.begin(), m_BlockProcess.end(), [](const PCB& pcb) {
			return pcb.currBlockTime == pcb.blockTime;
		});

		if (it != m_BlockProcess.end())
		{
			m_ReadyProcess.push_back(*it);
			m_BlockProcess.remove(*it);
			numReadyProcess++;
		}
		else
			break;
	}

	// 移除所有事件到来的进程
	m_BlockProcess.remove_if([](const PCB& pcb) {
		return pcb.currBlockTime == pcb.blockTime;
	});

	//
	// 空闲进程-->就绪进程(作业调度)
	//

	
	// 当前允许调度作业数
	numAllowProcess = std::min(m_MaxReadyLength, numReadyProcess + numIdleProcess);

	while (numReadyProcess < numAllowProcess)
	{
		// 空闲进程寻找现在到达的进程放入就绪进程
		const PCB* pPCB = m_JobDispatch(m_CurrTime, m_ReadyProcess, m_IdleProcess);
		if (pPCB != nullptr)
		{
			m_ReadyProcess.push_back(*pPCB);
			m_IdleProcess.remove(*pPCB);
			numReadyProcess++;
		}
		else
			break;
	}

	//
	// 就绪进程-->运行进程(进程调度)
	//

	
	numAllowProcess = std::min(m_NumCPU, numRunningProcess + numReadyProcess);

	// 非抢占式
	if (m_ProcessMode == Mode::NonPreemption)
	{
		while (numRunningProcess < numAllowProcess)
		{
			// 就绪进程寻找现在到达的进程放入运行进程
			const PCB* pPCB = m_ProcessDispatch(m_CurrTime, m_RunningProcess, m_ReadyProcess);
			if (pPCB != nullptr)
			{
				m_RunningProcess.push_back(*pPCB);
				m_ReadyProcess.remove(*pPCB);
				numRunningProcess++;
			}
			else
				break;
		}
	}
	// 抢占式
	else if (m_ProcessMode == Mode::Preemption)
	{
		// 找出所有满足条件的进程作为运行的进程
		std::list<PCB> finalProcess;
		for (uint32_t i = 0; i < numAllowProcess; ++i)
		{
			const PCB* pPCB = m_ProcessDispatch(m_CurrTime, m_RunningProcess, m_ReadyProcess);
			if (pPCB != nullptr)
			{
				finalProcess.push_back(*pPCB);
				m_RunningProcess.remove(*pPCB);
				m_ReadyProcess.remove(*pPCB);
			}
			else
				break;
		}

		// 转移所有不满足条件的进程至就绪进程
		for (const PCB& pcb : m_RunningProcess)
		{
			m_ReadyProcess.push_back(pcb);
		}
		m_RunningProcess.clear();

		// 满足条件的进程转移至运行进程
		for (const PCB& pcb : finalProcess)
		{
			m_RunningProcess.push_back(pcb);
		}
	}
	// 并发
	else
	{
		// 转移所有运行的进程至就绪进程
		for (const PCB& pcb : m_RunningProcess)
		{
			m_ReadyProcess.push_back(pcb);
		}
		m_RunningProcess.clear();

		// 找出所有满足条件的进程作为运行的进程
		std::list<PCB> finalProcess;

		for (uint32_t i = 0; i < numAllowProcess; ++i)
		{
			// 寻找满足条件的进程
			const PCB* pPCB = m_ProcessDispatch(m_CurrTime, m_RunningProcess, m_ReadyProcess);
			if (pPCB != nullptr)
			{
				finalProcess.push_back(*pPCB);
				m_ReadyProcess.remove(*pPCB);
				m_RunningProcess.remove(*pPCB);
			}
			else
				break;
		}

		for (const PCB& pcb : finalProcess)
		{
			m_RunningProcess.push_back(pcb);
		}
	}

	//
	// 运行进程-->完成进程
	// 运行进程-->阻塞进程
	//

	for (PCB& pcb : m_RunningProcess)
	{
		++pcb.currRunTime;
		if (pcb.currRunTime == pcb.startBlockTime && pcb.blockTime != 0)
		{
			m_BlockProcess.push_back(pcb);
		}
		else if (pcb.currRunTime == pcb.runTime)
		{
			pcb.finishTime = m_CurrTime + 1;
			m_FinishProcess.push_back(pcb);
		}
	}

	// 移除所有已经完成或开始阻塞的进程
	m_RunningProcess.remove_if([](const PCB& pcb) {
		return pcb.currRunTime == pcb.runTime ||
			pcb.currRunTime == pcb.startBlockTime && pcb.blockTime != 0;
	});

	// 计时器+1
	++m_CurrTime;

}

void ProcessSimulator::Reset()
{
	m_RunningProcess.clear();
	m_FinishProcess.clear();
	m_IdleProcess.clear();
	m_ReadyProcess.clear();
	m_CurrTime = 0;
}

uint32_t ProcessSimulator::GetCurrTime() const
{
	return m_CurrTime;
}

ProcessSimulator::Mode ProcessSimulator::GetMode() const
{
	return m_ProcessMode;
}

uint32_t ProcessSimulator::GetNumCPU() const
{
	return m_NumCPU;
}

uint32_t ProcessSimulator::GetMaxReadyLength() const
{
	return m_MaxReadyLength;
}

ProcessSimulator::Algorithm ProcessSimulator::GetJobAlgorithm() const
{
	return m_JobAlgorithm;
}

ProcessSimulator::Algorithm ProcessSimulator::GetProcessAlgorithm() const
{
	return m_ProcessAlgorithm;
}

uint32_t ProcessSimulator::GetSleepTime() const
{
	return m_SleepTime;
}

const std::list<PCB>& ProcessSimulator::GetIdleProcess() const
{
	return m_IdleProcess;
}

const std::list<PCB>& ProcessSimulator::GetBlockProcess() const
{
	return m_BlockProcess;
}

const std::list<PCB>& ProcessSimulator::GetReadyProcess() const
{
	return m_ReadyProcess;
}

const std::list<PCB>& ProcessSimulator::GetFinishProcess() const
{
	return m_FinishProcess;
}

const std::list<PCB>& ProcessSimulator::GetRunningProcess() const
{
	return m_RunningProcess;
}

bool ProcessSimulator::IsEnd() const
{
	return m_IsEnd;
}


const PCB * ProcessSimulator::JobFCFS(uint32_t currTime, const std::list<PCB>& readyList, const std::list<PCB>& idleList)
{
	auto it = idleList.cbegin();
	if (it != idleList.cend() && it->arriveTime <= currTime)
		return &*it;
	return nullptr;
}

const PCB * ProcessSimulator::JobSJF(uint32_t currTime, const std::list<PCB>& readyList, const std::list<PCB>& idleList)
{
	auto target = idleList.cend();
	uint32_t shortestTime = UINT32_MAX;
	for (auto it = idleList.cbegin(); it != idleList.cend(); ++it)
	{
		if (it->arriveTime > currTime)
			break;
		if (it->runTime < shortestTime)
		{
			shortestTime = it->runTime;
			target = it;
		}
	}

	if (target != idleList.cend())
		return &*target;

	return nullptr;
}

const PCB * ProcessSimulator::JobFPF(uint32_t currTime, const std::list<PCB>& readyList, const std::list<PCB>& idleList)
{
	auto target1 = idleList.cend();
	uint32_t highestPriority1 = 0;
	// 先检查闲置队列
	for (auto it = idleList.cbegin(); it != idleList.cend(); ++it)
	{
		if (it->arriveTime > currTime)
			break;
		if (it->priority > highestPriority1)
		{
			highestPriority1 = it->priority;
			target1 = it;
		}
	}

	auto target2 = readyList.cend();
	uint32_t highestPriority2 = 0;
	// 再检查就绪队列
	for (auto it = readyList.cbegin(); it != readyList.cend(); ++it)
	{
		if (it->arriveTime > currTime)
			break;
		if (it->priority > highestPriority2)
		{
			highestPriority2 = it->priority;
			target2 = it;
		}
	}

	if (target1 != idleList.cend() && target2 != readyList.cend())
	{
		return (highestPriority1 > highestPriority2 ? &*target1 : &*target2);
	}
	else if (target1 != idleList.cend())
	{
		return &*target1;
	}
	else if (target2 != readyList.cend())
	{
		return &*target2;
	}

	return nullptr;
}

const PCB * ProcessSimulator::ProcessFCFS(uint32_t currTime, const std::list<PCB>& runningList, const std::list<PCB>& readyList)
{
	auto it = readyList.cbegin();
	if (it != readyList.cend() && it->arriveTime <= currTime)
		return &*it;
	return nullptr;
}

const PCB * ProcessSimulator::ProcessSJF(uint32_t currTime, const std::list<PCB>& runningList, const std::list<PCB>& readyList)
{
	auto target = readyList.cend();
	uint32_t shortestTime = UINT32_MAX;
	for (auto it = readyList.cbegin(); it != readyList.cend(); ++it)
	{
		if (it->arriveTime > currTime)
			break;
		if (it->runTime < shortestTime)
		{
			shortestTime = it->runTime;
			target = it;
		}
	}

	if (target != readyList.cend())
		return &*target;

	return nullptr;
}

const PCB * ProcessSimulator::ProcessFPF(uint32_t currTime, const std::list<PCB>& runningList, const std::list<PCB>& readyList)
{
	auto target1 = readyList.cend();
	uint32_t highestPriority1 = 0;
	// 先检查就绪队列
	for (auto it = readyList.cbegin(); it != readyList.cend(); ++it)
	{
		if (it->arriveTime > currTime)
			break;
		if (it->priority > highestPriority1)
		{
			highestPriority1 = it->priority;
			target1 = it;
		}
	}

	auto target2 = runningList.cend();
	uint32_t highestPriority2 = 0;
	// 再检查运行队列
	for (auto it = runningList.cbegin(); it != runningList.cend(); ++it)
	{
		if (it->arriveTime > currTime)
			break;
		if (it->priority > highestPriority2)
		{
			highestPriority2 = it->priority;
			target2 = it;
		}
	}

	if (target1 != readyList.cend() && target2 != runningList.cend())
	{
		return (highestPriority1 > highestPriority2 ? &*target1 : &*target2);
	}
	else if (target1 != readyList.cend())
	{
		return &*target1;
	}
	else if (target2 != runningList.cend())
	{
		return &*target2;
	}

	return nullptr;
}

const PCB * ProcessSimulator::ProcessHRRN(uint32_t currTime, const std::list<PCB>& runningList, const std::list<PCB>& readyList)
{
	auto target1 = readyList.cend();
	float highestPriority1 = 0;
	// 先检查就绪队列
	for (auto it = readyList.cbegin(); it != readyList.cend(); ++it)
	{
		if (it->arriveTime > currTime)
			break;
		// 优先权 = (等待时间 + 要求服务时间) / 要求服务时间
		float priority = 1.0f + (float)(currTime - it->arriveTime) / it->runTime;
		if (priority > highestPriority1)
		{
			highestPriority1 = priority;
			target1 = it;
		}
	}

	auto target2 = runningList.cend();
	float highestPriority2 = 0;
	// 再检查运行队列
	for (auto it = runningList.cbegin(); it != runningList.cend(); ++it)
	{
		if (it->arriveTime > currTime)
			break;
		// 优先权 = (等待时间 + 要求服务时间) / 要求服务时间
		float priority = 1.0f + (float)(currTime - it->arriveTime) / it->runTime;
		if (priority > highestPriority2)
		{
			highestPriority2 = priority;
			target2 = it;
		}
	}

	if (target1 != readyList.cend() && target2 != runningList.cend())
	{
		return (highestPriority1 > highestPriority2 ? &*target1 : &*target2);
	}
	else if (target1 != readyList.cend())
	{
		return &*target1;
	}
	else if (target2 != runningList.cend())
	{
		return &*target2;
	}

	return nullptr;
}
