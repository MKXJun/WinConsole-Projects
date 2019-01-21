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
	Console::WriteLine("���س�������");
	Console::ReadChar();
	ShowResultTable(sim);
	Console::WriteLine("���س�������");
	Console::ReadChar();
	return 0;
}