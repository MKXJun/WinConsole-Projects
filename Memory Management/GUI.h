#pragma once
#include "Console.h"
#include "XAllocator.h"

void OutputCommandList();

std::unique_ptr<XAllocator> InitAllocator();

void ShowBlockList(const XAllocator& alloc);