#include "GUI.h"

static Console console = Console();



void ShowProcess(const PCB& pcb, uint16_t X, uint16_t Y, ShowMode mode)
{
	std::string str;

	// 进程名
	Console::SetTextColor(Console::Green);
	Console::SetCursorPos(X, Y);
	if (pcb.processName.size() > 12)
		str = pcb.processName.substr(0, 12) + "...";
	else
		str = pcb.processName;
	Console::WriteFormat("%-20s", str.c_str());
	++Y;

	// 进程ID
	str = "pID: " + std::to_string(pcb.processID);
	Console::SetCursorPos(X, Y);
	Console::WriteFormat("%-20s", str.c_str());
	++Y;

	// 优先级
	if (mode == ShowMode::Ready || mode == ShowMode::Running)
	{
		str = "优先级: " + std::to_string(pcb.priority);
		Console::SetTextColor(Console::Red);
		Console::SetCursorPos(X, Y);
		Console::WriteFormat("%-20s", str.c_str());
		++Y;
	}

	// 到达时间
	if (mode != ShowMode::Block)
	{
		str = "到达时间: " + std::to_string(pcb.arriveTime);
		Console::SetTextColor(Console::Cyan);
		Console::SetCursorPos(X, Y);
		Console::WriteFormat("%-20s", str.c_str());
		++Y;
	}

	// 当前
	if (mode == ShowMode::Ready || mode == ShowMode::Running)
	{
		str = "当前: " + std::to_string(pcb.currRunTime) + "/" + std::to_string(pcb.runTime);
		Console::SetTextColor(Console::Yellow);
		Console::SetCursorPos(X, Y);
		Console::WriteFormat("%-20s", str.c_str());
	}
	else if (mode == ShowMode::Block)
	{
		str = "等待: " + std::to_string(pcb.currBlockTime) + "/" + std::to_string(pcb.blockTime);
		Console::SetTextColor(Console::Yellow);
		Console::SetCursorPos(X, Y);
		Console::WriteFormat("%-20s", str.c_str());
	}
	Console::SetTextColor();
}



void InitProcessSimulator(ProcessSimulator & sim)
{
	char str[260];
	uint32_t num;
	float fnum;
	Console::Write("输入要读取的进程文件名：");
	Console::ReadLine(str, 260);
	assert(sim.CreateProcessFromFile(str));

	Console::Write("输入就绪队列允许长度(1-65535)：");
	Console::ReadFormat("%u", &num);
	assert(num > 0 && num < 65536);
	sim.SetMaxReadyLength(num);

	Console::Write("输入CPU数目(1-4)：");
	Console::ReadFormat("%u", &num);
	assert(num > 0 && num < 5);
	sim.SetNumCPU(num);

	Console::WriteLine("用数字选择作业调度算法：");
	Console::WriteLine("(1)FCFS (2)SJF");
	Console::ReadFormat("%u", &num);
	assert(num > 0 && num < 3);
	sim.SetJobDispatchFunc(static_cast<ProcessSimulator::Algorithm>(num));
	
	Console::WriteLine("用数字选择进程运行模式：");
	Console::WriteLine("(1)非抢占式 (2)抢占式 (3)时间片轮转(仅FCFS)");
	Console::ReadFormat("%u", &num);
	assert(num > 0 && num < 4);
	sim.SetProcessMode(static_cast<ProcessSimulator::Mode>(num));

	// 非抢占式
	if (num == 1)
	{
		Console::WriteLine("用数字选择进程调度算法：");
		Console::WriteLine("(1)FCFS (2)SJF");
		Console::ReadFormat("%u", &num);
		assert(num > 0 && num < 3);
		sim.SetProcessDispatchFunc(static_cast<ProcessSimulator::Algorithm>(num));
	}
	// 抢占式
	else if (num == 2)
	{
		Console::WriteLine("用数字选择进程调度算法：");
		Console::WriteLine("(1)FPF (2)HRRN");
		Console::ReadFormat("%u", &num);
		assert(num > 0 && num < 3);
		sim.SetProcessDispatchFunc(static_cast<ProcessSimulator::Algorithm>(num + 2));
	}
	// 时间片轮转
	else
	{
		sim.SetProcessDispatchFunc(ProcessSimulator::Algorithm::FCFS);
	}

	Console::Write("设置每秒帧数(0.1-20)：");
	Console::ReadFormat("%f", &fnum);
	assert(fnum >= 0.1f && fnum <= 20.0f);
	sim.SetSimSpeed(fnum);
	while (Console::ReadChar() != '\n')
		continue;
}

void InitGUI()
{
	// 清屏
	system("cls");

	Console::SetCursorInfo(20, false);
	Console::SetBufferSize(80, 40);
	Console::SetWindowSize(80, 40);

	std::ifstream fin("gui.txt");
	std::string str;
	while (!fin.eof())
	{
		std::getline(fin, str);
		Console::WriteLine(str.c_str());
	}
}

void Show(const ProcessSimulator& sim)
{
	std::string str;

	// 作业调度算法
	ProcessSimulator::Algorithm algo = sim.GetJobAlgorithm();
	switch (algo)
	{
	case ProcessSimulator::Algorithm::Unknown: str = "未知"; break;
	case ProcessSimulator::Algorithm::FCFS: str = "FCFS"; break;
	case ProcessSimulator::Algorithm::SJF: str = "SJF"; break;
	case ProcessSimulator::Algorithm::FPF: str = "FPF"; break;
	}
	Console::SetTextColor();
	Console::SetCursorPos(14, 0);
	Console::WriteFormat("%-20s", str.c_str());

	// 进程调度算法
	algo = sim.GetProcessAlgorithm();
	switch (algo)
	{
	case ProcessSimulator::Algorithm::Unknown: str = "未知"; break;
	case ProcessSimulator::Algorithm::FCFS: str = "FCFS"; break;
	case ProcessSimulator::Algorithm::SJF: str = "SJF"; break;
	case ProcessSimulator::Algorithm::FPF: str = "FPF"; break;
	case ProcessSimulator::Algorithm::HRRN: str = "HRRN"; break;
	}
	Console::SetCursorPos(54, 0);
	Console::WriteFormat("%-20s", str.c_str());

	// 当前模式
	ProcessSimulator::Mode mode = sim.GetMode();
	switch (mode)
	{
	case ProcessSimulator::Mode::NonPreemption: str = "非抢占式"; break;
	case ProcessSimulator::Mode::Preemption: str = "抢占式"; break;
	case ProcessSimulator::Mode::Concurrent: str = "并发"; break;
	}
	Console::SetCursorPos(10, 1);
	Console::WriteFormat("%-20s", str.c_str());

	// 当前时间
	str = std::to_string(sim.GetCurrTime());
	Console::SetCursorPos(50, 1);
	Console::WriteFormat("%-20s", str.c_str());

	//
	// 闲置队列
	//
	uint32_t row = 0, col = 0;
	const std::list<PCB>& idleList = sim.GetIdleProcess();
	// 闲置队列长度
	str = std::to_string(idleList.size());
	Console::SetCursorPos(14, 3);
	Console::WriteFormat("%-20s", str.c_str());

	// 清空
	Console::SetCursorPos(0, 4);
	Console::WriteFormat("%79s\n%79s\n%79s\n", "", "", "");

	// 闲置队列
	for (const PCB& pcb : idleList)
	{
		if (col >= 4)
			break;
		ShowProcess(pcb, col * 20, 4, ShowMode::Idle);
		++col;
	}

	//
	// 阻塞队列
	//
	col = 0;
	const std::list<PCB>& blockList = sim.GetBlockProcess();
	// 
	// 阻塞队列长度
	str = std::to_string(blockList.size());
	Console::SetCursorPos(14, 8);
	Console::WriteFormat("%-20s", str.c_str());

	// 清空
	Console::SetCursorPos(0, 9);
	Console::WriteFormat("%79s\n%79s\n%79s\n", "", "", "");

	// 阻塞队列
	for (const PCB& pcb : blockList)
	{
		if (col >= 4)
			break;
		ShowProcess(pcb, col * 20, 9, ShowMode::Block);
		++col;
	}

	//
	// 就绪队列
	//
	row = col = 0;
	const std::list<PCB>& readyList = sim.GetReadyProcess();
	// 就绪队列长度
	str = std::to_string(readyList.size()) + "/" + std::to_string(sim.GetMaxReadyLength());
	Console::SetCursorPos(14, 13);
	Console::WriteFormat("%-20s", str.c_str());

	// 清空
	Console::SetCursorPos(0, 14);
	Console::WriteFormat("%79s\n%79s\n%79s\n%79s\n%79s\n%79s\n%79s\n%79s\n%79s\n%79s\n%79s\n"
		, "", "", "", "", "", "", "", "", "", "", "");

	for (const PCB& pcb : readyList)
	{
		if (row >= 2)
			break;
		ShowProcess(pcb, col * 20, 14 + 6 * row, ShowMode::Ready);
		++col;
		if (col == 4)
		{
			++row;
			col = 0;
		}
	}

	//
	// 运行队列
	//
	row = col = 0;
	const std::list<PCB>& runningList = sim.GetRunningProcess();
	// 运行队列长度
	str = std::to_string(runningList.size()) + "/" + std::to_string(sim.GetNumCPU());
	Console::SetCursorPos(14, 26);
	Console::WriteFormat("%-20s", str.c_str());

	// 清空
	Console::SetCursorPos(0, 27);
	Console::WriteFormat("%79s\n%79s\n%79s\n%79s\n%79s\n", "", "", "", "", "");

	for (const PCB& pcb : runningList)
	{
		if (row >= 2)
			break;
		ShowProcess(pcb, col * 20, 27 + 6 * row, ShowMode::Running);
		++col;
		if (col == 4)
		{
			++row;
			col = 0;
		}
	}

	//
	// 已完成进程数
	//
	const std::list<PCB>& finishList = sim.GetFinishProcess();
	str = std::to_string(finishList.size());
	Console::SetCursorPos(14, 34);
	Console::WriteFormat("%-20s", str.c_str());
}

void ShowResultTable(const ProcessSimulator& sim)
{
	// 清屏
	std::string str;
	system("cls");

	Console::SetBufferSize(105, 999);
	Console::SetWindowSize(105, 40);

	Console::SetCursorPos(0, 0);
	Console::WriteFormat("%-24s%-10s%-10s%-12s%-12s%-10s%-10s%-12s\n",
		"进程", "优先级", "到达时间", "需运行时间", "I/O事件需时", "完成时间", "周转时间", "带权周转时间");

	const std::list<PCB>& finishList = sim.GetFinishProcess();
	float averageCycleTime = 0.0f, averageWeightedCycleTime = 0.0f;


	for (const PCB& pcb : finishList)
	{
		// 周转时间和带权周转时间
		uint32_t cycleTime = pcb.finishTime - pcb.arriveTime;
		float weightedCycleTime = float(pcb.finishTime - pcb.arriveTime) / (pcb.runTime + pcb.blockTime);
		str = pcb.processName + "(" + std::to_string(pcb.processID) + ")";
		Console::WriteFormat("%-24s%-10u%-10u%-12u%-12u%-10u%-10u%-12f\n",
			str.c_str(), pcb.priority, pcb.arriveTime, pcb.runTime, pcb.blockTime, pcb.finishTime,
			cycleTime, weightedCycleTime);
		averageCycleTime += pcb.finishTime - pcb.arriveTime;
		averageWeightedCycleTime += float(pcb.finishTime - pcb.arriveTime) / (pcb.runTime + pcb.blockTime);
	}

	averageCycleTime /= finishList.size();
	averageWeightedCycleTime /= finishList.size();

	Console::WriteFormat("平均周转时间：%f\n平均带权周转时间：%f\n", averageCycleTime, averageWeightedCycleTime);

}