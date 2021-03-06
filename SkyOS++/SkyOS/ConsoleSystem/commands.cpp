#include "SkyConsole.h"
#include "SkyDebugger.h"
#include "string.h"
#include "ctype.h"
#include "stdio.h"

long CmdCls(char *szCommand)
{
	SkyConsole::Clear();
	return false;
}

long CmdKill(char *szCommand)
{
	/*int id = atoi(szCommand);

	kEnterCriticalSection();

	Process* pProcess = ProcessManager::GetInstance()->FindProcess(id);

	kLeaveCriticalSection();

	if (pProcess != nullptr)
	{
		SkyConsole::Print("kill process : %s, ID : %d\n", pProcess->m_processName, pProcess->GetProcessId());
		ProcessManager::GetInstance()->RemoveProcess(pProcess->GetProcessId());
	}
	else
		SkyConsole::Print("process don't exist(%d)\n", id);	*/

	return false;
}

long CmdProcessList(char *szCommand)
{
	/*kEnterCriticalSection();
	SkyConsole::Print(" ID : Process Name\n");

	ProcessManager::ProcessList* processlist = ProcessManager::GetInstance()->GetProcessList();
	map<int, Process*>::iterator iter = processlist->begin();

	for (; iter != processlist->end(); ++iter)
	{
		Process* pProcess = (*iter).second;
		SkyConsole::Print("  %d %s\n", pProcess->GetProcessId(), pProcess->m_processName);
	}

	kLeaveCriticalSection();*/

	return true;
}

long cmdMemState(char *szCommand)
{
	//SystemProfiler::GetInstance()->PrintMemoryState();
	return false;
}

long cmdCreateWatchdogTask(char* pName)
{
	/*kEnterCriticalSection();
	
	Process* pProcess = ProcessManager::GetInstance()->CreateProcessFromMemory("WatchDog", WatchDogProc, NULL, PROCESS_KERNEL);	
	kLeaveCriticalSection();
	
	if(pProcess == nullptr)
		SkyConsole::Print("Can't create process\n");	*/

	return false;
}

long cmdTaskCount(char *szCommand)
{
	/*kEnterCriticalSection();

	ProcessManager::TaskList* taskList = ProcessManager::GetInstance()->GetTaskList();
	SkyConsole::Print("current task count %d\n", taskList->size());

	kLeaveCriticalSection();*/
	return false;
}

long cmdGlobalState(char *szCommand)
{
	//SystemProfiler::GetInstance()->PrintGlobalState();
	return false;
}

long CmdExec(char *szCommand)
{

	/*Process* pProcess = ProcessManager::GetInstance()->CreateProcessFromFile(szCommand, nullptr, PROCESS_USER);

	if (pProcess == nullptr)
		SkyConsole::Print("Can't create process %s\n", szCommand);*/
	

	return false;
}

long cmdJpeg(char *szCommand)
{
	/*if (false == SkyGUISystem::GetInstance()->GUIEnable())
	{
		SkyConsole::Print("GUI console mode only support this command.\n", szCommand);
		return false;
	}*/


	return false;
}

long cmdGUI(char *szCommand)
{
	//RequesGUIResolution();

	return false;
}

long cmdCD(char *szCommand)
{
	int size = strlen(szCommand);
	if (size != 2 || szCommand[1] != ':')
	{
		SkyConsole::Print("Invalid Argument\n");
		return false;
	}
	char driveLetter = toupper(szCommand[0]);
	//StorageManager::GetInstance()->SetCurrentFileSystemByID(driveLetter);

	return false;
}

void FillRect8(int x, int y, int w, int h, char col, int actualX, int actualY)
{
	char* lfb = (char*)0xF0000000;

	for (int k = 0; k < h; k++)
		for (int j = 0; j < w; j++)
		{
			int index = ((j + x) + (k + y) * actualX);
			lfb[index] = col;
			index++;
		}

}

long cmdSwitchGUI(char *szCommand)
{
	//성공적으로 그래픽 모드가 전환되는지 확인
	/*if(true == SwitchGUIMode(1024, 768, 261))
	{
		//그래픽 버퍼 주소를 매핑해야 한다.
		VirtualMemoryManager::MapDMAAddress(VirtualMemoryManager::GetCurPageDirectory(), 0xE0000000, 0xE0000000, 0xE0FF0000);
		VirtualMemoryManager::MapDMAAddress(VirtualMemoryManager::GetCurPageDirectory(), 0xF0000000, 0xF0000000, 0xF0FF0000);
		
		//사각형을 그린다.
		FillRect8(100, 100, 100, 100, 8, 1024, 768);
		for (;;);
	}*/
	
	return false;
}


long cmdPCI(char *szCommand)
{
	//DeviceDriverManager::GetInstance()->RequestPCIList();

	return false;
}

long cmdCallStack(char *szCommand)
{
	if (szCommand == nullptr)
	{
		SkyDebugger::GetInstance()->TraceStackWithSymbol();
		return false;
	}

	int id = atoi(szCommand);
	SkyDebugger::GetInstance()->TraceStackWithProcessId(id);

	return false;
}

long cmdDir(char *szCommand)
{
	//StorageManager::GetInstance()->GetFileList();

	return false;
}


long cmdFreeType(char *szCommand)
{
	//TestSkySDL(1024, 768, 32);
	//return false;

	/*char* text = "i love you";
	
	//kEnterCriticalSection();
	bool error = TestFreeType("1.TTF", text);
	//kLeaveCriticalSection();

	if (error)
		printf("sdffsdsfd\n");

	*/

	return false;
}
