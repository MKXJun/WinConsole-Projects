#include "GUI.h"

static Console console = Console();



void ShowProcess(const PCB& pcb, uint16_t X, uint16_t Y, ShowMode mode)
{
	std::string str;

	// ������
	Console::SetTextColor(Console::Green);
	Console::SetCursorPos(X, Y);
	if (pcb.processName.size() > 12)
		str = pcb.processName.substr(0, 12) + "...";
	else
		str = pcb.processName;
	Console::WriteFormat("%-20s", str.c_str());
	++Y;

	// ����ID
	str = "pID: " + std::to_string(pcb.processID);
	Console::SetCursorPos(X, Y);
	Console::WriteFormat("%-20s", str.c_str());
	++Y;

	// ���ȼ�
	if (mode == ShowMode::Ready || mode == ShowMode::Running)
	{
		str = "���ȼ�: " + std::to_string(pcb.priority);
		Console::SetTextColor(Console::Red);
		Console::SetCursorPos(X, Y);
		Console::WriteFormat("%-20s", str.c_str());
		++Y;
	}

	// ����ʱ��
	if (mode != ShowMode::Block)
	{
		str = "����ʱ��: " + std::to_string(pcb.arriveTime);
		Console::SetTextColor(Console::Cyan);
		Console::SetCursorPos(X, Y);
		Console::WriteFormat("%-20s", str.c_str());
		++Y;
	}

	// ��ǰ
	if (mode == ShowMode::Ready || mode == ShowMode::Running)
	{
		str = "��ǰ: " + std::to_string(pcb.currRunTime) + "/" + std::to_string(pcb.runTime);
		Console::SetTextColor(Console::Yellow);
		Console::SetCursorPos(X, Y);
		Console::WriteFormat("%-20s", str.c_str());
	}
	else if (mode == ShowMode::Block)
	{
		str = "�ȴ�: " + std::to_string(pcb.currBlockTime) + "/" + std::to_string(pcb.blockTime);
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
	Console::Write("����Ҫ��ȡ�Ľ����ļ�����");
	Console::ReadLine(str, 260);
	assert(sim.CreateProcessFromFile(str));

	Console::Write("�����������������(1-65535)��");
	Console::ReadFormat("%u", &num);
	assert(num > 0 && num < 65536);
	sim.SetMaxReadyLength(num);

	Console::Write("����CPU��Ŀ(1-4)��");
	Console::ReadFormat("%u", &num);
	assert(num > 0 && num < 5);
	sim.SetNumCPU(num);

	Console::WriteLine("������ѡ����ҵ�����㷨��");
	Console::WriteLine("(1)FCFS (2)SJF");
	Console::ReadFormat("%u", &num);
	assert(num > 0 && num < 3);
	sim.SetJobDispatchFunc(static_cast<ProcessSimulator::Algorithm>(num));
	
	Console::WriteLine("������ѡ���������ģʽ��");
	Console::WriteLine("(1)����ռʽ (2)��ռʽ (3)ʱ��Ƭ��ת(��FCFS)");
	Console::ReadFormat("%u", &num);
	assert(num > 0 && num < 4);
	sim.SetProcessMode(static_cast<ProcessSimulator::Mode>(num));

	// ����ռʽ
	if (num == 1)
	{
		Console::WriteLine("������ѡ����̵����㷨��");
		Console::WriteLine("(1)FCFS (2)SJF");
		Console::ReadFormat("%u", &num);
		assert(num > 0 && num < 3);
		sim.SetProcessDispatchFunc(static_cast<ProcessSimulator::Algorithm>(num));
	}
	// ��ռʽ
	else if (num == 2)
	{
		Console::WriteLine("������ѡ����̵����㷨��");
		Console::WriteLine("(1)FPF (2)HRRN");
		Console::ReadFormat("%u", &num);
		assert(num > 0 && num < 3);
		sim.SetProcessDispatchFunc(static_cast<ProcessSimulator::Algorithm>(num + 2));
	}
	// ʱ��Ƭ��ת
	else
	{
		sim.SetProcessDispatchFunc(ProcessSimulator::Algorithm::FCFS);
	}

	Console::Write("����ÿ��֡��(0.1-20)��");
	Console::ReadFormat("%f", &fnum);
	assert(fnum >= 0.1f && fnum <= 20.0f);
	sim.SetSimSpeed(fnum);
	while (Console::ReadChar() != '\n')
		continue;
}

void InitGUI()
{
	// ����
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

	// ��ҵ�����㷨
	ProcessSimulator::Algorithm algo = sim.GetJobAlgorithm();
	switch (algo)
	{
	case ProcessSimulator::Algorithm::Unknown: str = "δ֪"; break;
	case ProcessSimulator::Algorithm::FCFS: str = "FCFS"; break;
	case ProcessSimulator::Algorithm::SJF: str = "SJF"; break;
	case ProcessSimulator::Algorithm::FPF: str = "FPF"; break;
	}
	Console::SetTextColor();
	Console::SetCursorPos(14, 0);
	Console::WriteFormat("%-20s", str.c_str());

	// ���̵����㷨
	algo = sim.GetProcessAlgorithm();
	switch (algo)
	{
	case ProcessSimulator::Algorithm::Unknown: str = "δ֪"; break;
	case ProcessSimulator::Algorithm::FCFS: str = "FCFS"; break;
	case ProcessSimulator::Algorithm::SJF: str = "SJF"; break;
	case ProcessSimulator::Algorithm::FPF: str = "FPF"; break;
	case ProcessSimulator::Algorithm::HRRN: str = "HRRN"; break;
	}
	Console::SetCursorPos(54, 0);
	Console::WriteFormat("%-20s", str.c_str());

	// ��ǰģʽ
	ProcessSimulator::Mode mode = sim.GetMode();
	switch (mode)
	{
	case ProcessSimulator::Mode::NonPreemption: str = "����ռʽ"; break;
	case ProcessSimulator::Mode::Preemption: str = "��ռʽ"; break;
	case ProcessSimulator::Mode::Concurrent: str = "����"; break;
	}
	Console::SetCursorPos(10, 1);
	Console::WriteFormat("%-20s", str.c_str());

	// ��ǰʱ��
	str = std::to_string(sim.GetCurrTime());
	Console::SetCursorPos(50, 1);
	Console::WriteFormat("%-20s", str.c_str());

	//
	// ���ö���
	//
	uint32_t row = 0, col = 0;
	const std::list<PCB>& idleList = sim.GetIdleProcess();
	// ���ö��г���
	str = std::to_string(idleList.size());
	Console::SetCursorPos(14, 3);
	Console::WriteFormat("%-20s", str.c_str());

	// ���
	Console::SetCursorPos(0, 4);
	Console::WriteFormat("%79s\n%79s\n%79s\n", "", "", "");

	// ���ö���
	for (const PCB& pcb : idleList)
	{
		if (col >= 4)
			break;
		ShowProcess(pcb, col * 20, 4, ShowMode::Idle);
		++col;
	}

	//
	// ��������
	//
	col = 0;
	const std::list<PCB>& blockList = sim.GetBlockProcess();
	// 
	// �������г���
	str = std::to_string(blockList.size());
	Console::SetCursorPos(14, 8);
	Console::WriteFormat("%-20s", str.c_str());

	// ���
	Console::SetCursorPos(0, 9);
	Console::WriteFormat("%79s\n%79s\n%79s\n", "", "", "");

	// ��������
	for (const PCB& pcb : blockList)
	{
		if (col >= 4)
			break;
		ShowProcess(pcb, col * 20, 9, ShowMode::Block);
		++col;
	}

	//
	// ��������
	//
	row = col = 0;
	const std::list<PCB>& readyList = sim.GetReadyProcess();
	// �������г���
	str = std::to_string(readyList.size()) + "/" + std::to_string(sim.GetMaxReadyLength());
	Console::SetCursorPos(14, 13);
	Console::WriteFormat("%-20s", str.c_str());

	// ���
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
	// ���ж���
	//
	row = col = 0;
	const std::list<PCB>& runningList = sim.GetRunningProcess();
	// ���ж��г���
	str = std::to_string(runningList.size()) + "/" + std::to_string(sim.GetNumCPU());
	Console::SetCursorPos(14, 26);
	Console::WriteFormat("%-20s", str.c_str());

	// ���
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
	// ����ɽ�����
	//
	const std::list<PCB>& finishList = sim.GetFinishProcess();
	str = std::to_string(finishList.size());
	Console::SetCursorPos(14, 34);
	Console::WriteFormat("%-20s", str.c_str());
}

void ShowResultTable(const ProcessSimulator& sim)
{
	// ����
	std::string str;
	system("cls");

	Console::SetBufferSize(105, 999);
	Console::SetWindowSize(105, 40);

	Console::SetCursorPos(0, 0);
	Console::WriteFormat("%-24s%-10s%-10s%-12s%-12s%-10s%-10s%-12s\n",
		"����", "���ȼ�", "����ʱ��", "������ʱ��", "I/O�¼���ʱ", "���ʱ��", "��תʱ��", "��Ȩ��תʱ��");

	const std::list<PCB>& finishList = sim.GetFinishProcess();
	float averageCycleTime = 0.0f, averageWeightedCycleTime = 0.0f;


	for (const PCB& pcb : finishList)
	{
		// ��תʱ��ʹ�Ȩ��תʱ��
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

	Console::WriteFormat("ƽ����תʱ�䣺%f\nƽ����Ȩ��תʱ�䣺%f\n", averageCycleTime, averageWeightedCycleTime);

}