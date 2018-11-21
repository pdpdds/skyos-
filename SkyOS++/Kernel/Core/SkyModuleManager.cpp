#include "SkyOS.h"

SkyModuleManager* SkyModuleManager::m_pModuleManager = nullptr;
extern SKY_FILE_Interface g_FileInterface;
extern SKY_ALLOC_Interface g_allocInterface;
extern SKY_Print_Interface g_printInterface;
extern SKY_PROCESS_INTERFACE g_processInterface;

extern FILE* g_stdOut;
extern FILE* g_stdIn;
extern FILE* g_stdErr;

#ifdef SKY_EMULATOR
#include "SkyOSWin32Stub.h"
#endif // #ifdef SKY_EMULATOR

SkyModuleManager::SkyModuleManager()
{
}

SkyModuleManager::~SkyModuleManager()
{
}

bool SkyModuleManager::Initialize(multiboot_info* pBootInfo)
{
#ifndef SKY_EMULATOR
	g_printInterface.sky_stdin = g_stdIn;
	g_printInterface.sky_stdout = g_stdOut;
	g_printInterface.sky_stderr = g_stdErr;
#endif // !SKY_EMULATOR

	m_pMultibootInfo = pBootInfo;
	return true;
}

void SkyModuleManager::PrintMoudleList()
{
	uint32_t mb_flags = m_pMultibootInfo->flags;
	if (mb_flags & MULTIBOOT_INFO_MODS)
	{
		uint32_t mods_count = m_pMultibootInfo->mods_count;
		uint32_t mods_addr = (uint32_t)m_pMultibootInfo->Modules;

		for (uint32_t mod = 0; mod < mods_count; mod++)
		{
			Module* module = (Module*)(mods_addr + (mod * sizeof(Module)));

			const char* module_string = (const char*)module->Name;

			SkyConsole::Print(" %s\n", module_string);			
		}
	}
}

Module* SkyModuleManager::FindModule( const char* moduleName)
{
	uint32_t mb_flags = m_pMultibootInfo->flags;
	if (mb_flags & MULTIBOOT_INFO_MODS)
	{
		uint32_t mods_count = m_pMultibootInfo->mods_count;
		uint32_t mods_addr = (uint32_t)m_pMultibootInfo->Modules;

		for (uint32_t mod = 0; mod < mods_count; mod++)
		{
			Module* module = (Module*)(mods_addr + (mod * sizeof(Module)));

			const char* module_string = (const char*)module->Name;

			//SkyConsole::Print("Module Name : %s 0x%x 0x%x\n", module_string, module->ModuleStart, module->ModuleEnd);

			if (strcmp(module_string, moduleName) == 0)
			{
				return module;
			}
		}
	}

	return nullptr;
}

LOAD_DLL_INFO* SkyModuleManager::FindLoadedModule(const char* dll_path)
{
	auto iter = m_moduleList.begin();

	for (; iter != m_moduleList.end(); iter++)
	{
		if (strcmp((*iter)->moduleName, dll_path) == 0)
			return (*iter);
	}

	return nullptr;
}

void* SkyModuleManager::LoadModule(const char* moduleName, bool fromMemory)
{
#ifdef SKY_EMULATOR_DLL
	void* hwnd = (void*)g_processInterface.sky_kload_library(moduleName);
#else*/
	MODULE_HANDLE hwnd = SkyModuleManager::GetInstance()->LoadModuleFromMemory(moduleName);
#endif // SKY_EMULATOR_DLL	

	if (hwnd == nullptr)
	{
		HaltSystem("Memory Module Load Fail!!");
	}

#ifdef SKY_EMULATOR_DLL
	PSetSkyMockInterface SetSkyMockInterface = (PSetSkyMockInterface)g_processInterface.sky_kget_proc_address(hwnd, "SetSkyMockInterface");
	PSetSkyProcessInterface SetSkyProcessInterface = (PSetSkyProcessInterface)g_processInterface.sky_kget_proc_address(hwnd, "SetSkyProcessInterface");	
#else

	if (hwnd->refCount > 1)
		return hwnd;
	
	PSetSkyMockInterface SetSkyMockInterface = (PSetSkyMockInterface)SkyModuleManager::GetInstance()->GetModuleFunction(hwnd, "SetSkyMockInterface");
	PSetSkyProcessInterface SetSkyProcessInterface = (PSetSkyProcessInterface)SkyModuleManager::GetInstance()->GetModuleFunction(hwnd, "SetSkyProcessInterface");	
#endif

	//디버그 엔진에 플랫폼 종속적인 인터페이스를 넘긴다.
	if (SetSkyMockInterface != nullptr)
		SetSkyMockInterface(g_allocInterface, g_FileInterface, g_printInterface);

	if(SetSkyProcessInterface != nullptr)
		SetSkyProcessInterface(g_processInterface);

	return hwnd;
}

MODULE_HANDLE SkyModuleManager::LoadModuleFromFile(const char* dll_path)
{
	LOAD_DLL_INFO *p = FindLoadedModule(dll_path);
	if (p)
	{
		p->refCount++;
		return p;
	}

	p = new LOAD_DLL_INFO;
	strcpy(p->moduleName, dll_path);
	p->refCount = 1;

	DWORD res = LoadDLLFromFileName(dll_path, 0, p);
	if (res != ELoadDLLResult_OK)
	{
		delete p;
		return NULL;
	}

	m_moduleList.push_back(p);
	return p;
}

bool SkyModuleManager::LoadImplictDLL(DWORD moduleAddress)
{
	if (false == ValidatePEImage((void*)moduleAddress))
	{
		SkyConsole::Print("LoadImplictDLL Fail\n");		
		return false;
	}

	FixIAT((void*)moduleAddress);



	return true;
}



bool SkyModuleManager::FixIAT(void* image)
{
	StorageManager::GetInstance();


	IMAGE_DOS_HEADER* dosHeader = 0;
	IMAGE_NT_HEADERS* ntHeaders = 0;

	dosHeader = (IMAGE_DOS_HEADER*)image;
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	if (dosHeader->e_lfanew == 0)
		return false;

	//NT Header 체크
	ntHeaders = (IMAGE_NT_HEADERS*)(dosHeader->e_lfanew + (uint32_t)image);
	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
		return false;

	/* only supporting for i386 archs */
	if (ntHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
		return false;

	/* only support 32 bit executable images */
	if (!(ntHeaders->FileHeader.Characteristics & (IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_32BIT_MACHINE)))
		return false;

	/*
	Note: 1st 4 MB remains idenitity mapped as kernel pages as it contains
	kernel stack and page directory. If you want to support loading below 1MB,
	make sure to move these into kernel land
	*/

	//로드되는 프로세스의 베이스 주소는 0x00400000다. 
	//비쥬얼 스튜디오에서 속성=> 링커 => 고급의 기준주소 항목에서 확인 가능하다
	if ((ntHeaders->OptionalHeader.ImageBase < 0x400000) || (ntHeaders->OptionalHeader.ImageBase > 0x80000000))
		return false;

	/* only support 32 bit optional header format */
	if (ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		return false;

	auto importDir = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

	if (!importDir.VirtualAddress || !importDir.Size)
		return false;

	auto importDescriptor = PIMAGE_IMPORT_DESCRIPTOR(ULONG_PTR(image) + importDir.VirtualAddress);
	auto fixIATCount = 0;

	for (; importDescriptor->FirstThunk; importDescriptor++)
	{
		SkyConsole::Print("OriginalFirstThunk: %x\n", importDescriptor->OriginalFirstThunk);
		SkyConsole::Print("     TimeDateStamp: %x\n", importDescriptor->TimeDateStamp);
		SkyConsole::Print("    ForwarderChain: %x\n", importDescriptor->ForwarderChain);
		//if (!IsBadReadPtr((char*)image + importDescriptor->Name, 2))
		SkyConsole::Print("              Name: %x %s\n", importDescriptor->Name, (char*)image + importDescriptor->Name);

		if (strcmp("SkyOSWin32Stub.dll", (char*)image + importDescriptor->Name) == 0)
			continue;

		char* dllName = (char*)image + importDescriptor->Name;
		void* hwnd = (void*)LoadModule(dllName);
		
#ifdef SKY_EMULATOR_DLL
		//if (hwnd != nullptr) SKY_EMULATOR_DLL;
		LoadImplictDLL((DWORD)hwnd);
		continue;
#else
		if (((MODULE_HANDLE)(hwnd))->refCount > 1)
			continue;
#endif

		auto thunkData = PIMAGE_THUNK_DATA32(ULONG_PTR(image) + importDescriptor->FirstThunk);

		PIMAGE_THUNK_DATA32 pthunk;
		if (importDescriptor->OriginalFirstThunk == 0)
			pthunk = PIMAGE_THUNK_DATA32(ULONG_PTR(image) + importDescriptor->FirstThunk);
		else
			pthunk = PIMAGE_THUNK_DATA32(ULONG_PTR(image) + importDescriptor->OriginalFirstThunk);
		PIMAGE_THUNK_DATA32 nextthunk;
		for (int i = 0; pthunk->u1.Function != 0; i++, pthunk++) 
		{
			nextthunk = PIMAGE_THUNK_DATA32(ULONG_PTR(image) + importDescriptor->FirstThunk);
			if ((pthunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) == 0) 
			{
				PIMAGE_IMPORT_BY_NAME pname = (PIMAGE_IMPORT_BY_NAME)((PCHAR)image + pthunk->u1.AddressOfData);
				
				void* p = GetModuleFunction(hwnd, (char*)pname->Name);

				if (p)
				{					
					nextthunk[i].u1.Function = reinterpret_cast<DWORD>(p);
					g_printInterface.sky_printf("Function: %x %s\n", nextthunk[i].u1.Function, (char*)pname->Name);
					fixIATCount++;
				}				
			}			
		}
	}
	
	TRACE("%d imports parsed!\n", fixIATCount);

	return true;
}

MODULE_HANDLE SkyModuleManager::LoadModuleFromMemory(const char* moduleName)
{
	LOAD_DLL_INFO *p = FindLoadedModule(moduleName);
	if (p)
	{
		p->refCount++;
		return p;
	}

	Module* pModule = FindModule(moduleName);

	if (pModule == nullptr)
		return nullptr;

	SkyConsole::Print("Found %s\n", pModule->Name);

	if (false == ValidatePEImage((void*)pModule->ModuleStart))
	{
		SkyConsole::Print("invalid %s\n", pModule->Name);
		return nullptr;
	}

	p = new LOAD_DLL_INFO;
	strcpy(p->moduleName, moduleName);
	p->refCount = 1;

	DWORD res = LoadDLLFromMemory((void*)pModule->ModuleStart, ((size_t)(pModule->ModuleEnd) - (size_t)pModule->ModuleStart), 0, p);
	if (res != ELoadDLLResult_OK)
	{
		delete p;
		SkyConsole::Print("LoadModuleFromMemory Fail. ModuleName : %s Error : %d\n", pModule->Name, res);
		p = nullptr;
		return nullptr;
	}

	SkyConsole::Print("%s Module Loaded\n", moduleName);
	m_moduleList.push_back(p);

	return p;
}

bool SkyModuleManager::UnloadModule(MODULE_HANDLE handle)
{
	bool findModule = false;
	auto iter = m_moduleList.begin();

	for (; iter != m_moduleList.end(); iter++)
	{
		if ((*iter) == handle)
		{
			findModule = true;
			break;
		}
	}

	if (findModule == false)
	{
		SkyConsole::Print("Module UnloadModule Fail!!\n");
		return false;
	}
	
	if (handle->refCount > 1)
	{
		handle->refCount--;
		return true;
	}

	bool res = FALSE != UnloadDLL(handle);
	delete handle;
	return res;
}

void* SkyModuleManager::GetModuleFunction(void* handle, const char* func_name)
{

#ifdef SKY_EMULATOR_DLL
	return (void*)g_processInterface.sky_kget_proc_address(handle, func_name);
#else
	return (void*)myGetProcAddress_LoadDLLInfo((MODULE_HANDLE)handle, func_name);
#endif
}


