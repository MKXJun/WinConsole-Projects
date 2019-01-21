#pragma once

#define NOMINMAX
#include "ProcessSimulator.h"
#include "Console.h"

enum class ShowMode { Idle, Block, Ready, Running };

void InitProcessSimulator(ProcessSimulator& sim);

void InitGUI();

void Show(const ProcessSimulator& sim);

void ShowResultTable(const ProcessSimulator& sim);

