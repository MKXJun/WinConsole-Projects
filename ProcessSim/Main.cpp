#include "GUI.h"

int main()
{
	ProcessSimulator sim;
	InitProcessSimulator(sim);
	InitGUI();
	Show(sim);
	while (!sim.IsEnd())
	{
		Sleep(sim.GetSleepTime());
		sim.Update();
		Show(sim);
	}
	Show(sim);
	Console::WriteLine("按回车键继续");
	Console::ReadChar();
	ShowResultTable(sim);
	Console::WriteLine("按回车键结束");
	Console::ReadChar();
	return 0;
}