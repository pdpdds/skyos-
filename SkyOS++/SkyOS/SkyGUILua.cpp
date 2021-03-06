#include "windef.h"
#include "stdint.h"
#include "PlatformAPI.h"
#include "SkyGUILua.h"
#include "memory.h"
#include "SkyRenderer32.h"
//#include "KeyboardController.h"
#include "SkyInputManager.h"
#include "SkyInputHandler.h"
#include "LuaKernel.h"
#include "SystemAPI.h"
#include "SkyGUISystem.h"
#include "ConsoleSystem\ConsoleManager.h"
#include "stl/stl_string.h"
#include "string.h"
#include "ctype.h"
#include "syscall.h"
#include "FontData.h"

extern char skyFontData[4096];
bool LoadFontFromMemory()
{
	unsigned char* buffer = (unsigned char*)skyFontData;
	int bufferIndex = 0;
	int charIndex = 0;

	memset(buffer, 0, 4096);

	int readIndex = 0;

	//fontData에 정의된 폰트 배열을 커널에서 사용할 수 있도록 변환해서 skyFontData에 저장한다.
	//폰트 개수는 255개, 한 폰트는 8 * 16 = 128바이트이므로 fontData 배열의 크기는 32768
	while (readIndex < 32768) {

		//한문자를 표현하는 폰트는 8 * 16 = 128바이트
		for (int i = 0; i < 128; i++)
		{
			//'*'는 픽셀을, '.'은 빈 공간을 의미한다.
			//fontdata에서 한문자의 가로 한 라인은 8바이트이므로
			//이 8바이트를 읽으면서 1바이트 공간에 담는다.
			//skyFontData는 4096 바이트 배열이다. => 32768 / 8 = 4096
			if (fontData[readIndex + i] == '*')
			{

				if (charIndex < 8)
				{
					char a = (char)(1 << (8 - 1 - charIndex));
					buffer[bufferIndex] |= a;
				}

			}

			if ((fontData[readIndex + i] == '*') || (fontData[readIndex + i] == '.'))
			{
				charIndex++;

				if (charIndex >= 8)
				{
					//8바이트를 체크했으므로 bufferIndex를 하나 증가시킨다.
					bufferIndex++;
					charIndex = 0;
				}
			}
		}

		readIndex += 128;

	}

	return true;
}

extern char skyFontData[4096];

ULONG* SkyGUILua::m_pVideoRamPtr = nullptr;

#define PIVOT_X 8
#define PIVOT_Y 16
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16
#define CHAR_COLOR 0xff

SkyGUILua::SkyGUILua()
{
	m_yPos = PIVOT_Y;
	m_xPos = PIVOT_X;
	m_pRenderer = nullptr;
}

SkyGUILua::~SkyGUILua()
{
}

bool SkyGUILua::Initialize(void* pVideoRamPtr, int width, int height, int bpp, uint8_t buffertype)
{	
	m_pSkyInputManager = new SkyInputManager();
	m_pSkyInputManager->Initialize();

	m_pRenderer = new SkyRenderer32();
	LoadFontFromMemory();
		
	m_pVideoRamPtr = (ULONG*)pVideoRamPtr;
	m_width = width;
	m_height = height;
	m_bpp = bpp;

	PrintCurrentTime();

	return true;
}

void SkyGUILua::GetCommandForGUI(char* commandBuffer, int bufSize)
{
	int i = 0;
	unsigned char c = 0;
	bool BufChar = false;
	I_HangulEngine* pIMEEngine = SkyGUISystem::GetInstance()->GetIMEEngine();

	std::string command;
	while (i < bufSize)
	{		
		BufChar = true;
		
		KEYDATA keyData;

		if (m_pSkyInputManager->GetKeyFromKeyQueue(&keyData) == false)
		{
			platformAPI._processInterface.sky_ksleep(0);
			continue;
		}

		if ((keyData.bFlags & KEY_FLAGS_DOWN) == false)
			continue;

		c = keyData.bASCIICode;

		if (c == '\n' || c == 0x0d)
		{
			int len = pIMEEngine->GetString(commandBuffer);
			break;
		}

		memset(commandBuffer, 0, bufSize);

		//backspace
		if (c == 0x08) {
			
			BufChar = false;

			if (i > 0) {

				pIMEEngine->InputAscii(c);
				int len = pIMEEngine->GetString(commandBuffer);

				
				if (len != 0)
				{
					command = commandBuffer;

					PrintCommand((char*)command.c_str(), true);
					i--;
				}
					
			}
		}

		if ((unsigned char)c == 0x85) {
			
			BufChar = false;
			pIMEEngine->InputAscii(0x85);
		}
		
		if (BufChar) 
		{			
			if (c != 0) 
			{ 
				pIMEEngine->InputAscii(c);
				i = pIMEEngine->GetString(commandBuffer);
				
				command = commandBuffer;

				PrintCommand((char*)command.c_str(), false);				
			}
		}

		platformAPI._processInterface.sky_ksleep(0);
	}	
}

LuaKernel* g_pKernel = nullptr;
SkyGUILua* g_guiLua = nullptr;

DWORD WINAPI LuaInputProc(LPVOID parameter)
{
	SkyGUILua* pConsole = g_guiLua;
	I_HangulEngine* pIMEEngine = SkyGUISystem::GetInstance()->GetIMEEngine();

	ConsoleManager manager;

	int bufferLen = (1024 / CHAR_WIDTH) - 15;
	char* commandBuffer = new char[bufferLen];

	while (1)
	{
		//std::string driveName = "Lua> ";
		//pConsole->PrintCommand((char*)driveName.c_str(), false);

		memset(commandBuffer, 0, bufferLen);
		pIMEEngine->Reset();

		pConsole->GetCommandForGUI(commandBuffer, bufferLen);
		pConsole->Print("\n");
		g_pKernel->ExecuteCommand(commandBuffer);		
	}

	return 0;
}
typedef I_GUIEngine* (*PGUIEngine)();
bool SkyGUILua::Run()
{	
	I_HangulEngine* pIMEEngine = SkyGUISystem::GetInstance()->GetIMEEngine();
	
	PrintUnicode("Welcome to SkyLua!!");
	PrintUnicode("Lua Version is 5.40");
	GetNewLine();


	/*void* hwnd = SkyModuleManager::GetInstance()->LoadModule("SkyImgui.dll");
#ifdef SKY_EMULATOR_DLL
	//if (hwnd != nullptr) SKY_EMULATOR_DLL;
	SkyModuleManager::GetInstance()->LoadImplictDLL((DWORD)hwnd);
#endif

	PGUIEngine GUIEngine = (PGUIEngine)SkyModuleManager::GetInstance()->GetModuleFunction(hwnd, "GetGUIEngine");
	GUIEngine();*/

	LuaKernel* pKernel  = new LuaKernel();
	g_pKernel = pKernel;
	g_guiLua = this;
	ExecThread("aaa", (thread_start_t)LuaInputProc, this, 1);
	//platformAPI._processInterface.sky_kcreate_thread_from_memory(0, LuaInputProc, this);

	pKernel->Initialize((uint8*)m_pVideoRamPtr);	
	
	return true;
}

VOID SkyGUILua::GetNewLine()
{
	int x, y;
	ULONG *buf = m_pVideoRamPtr;	
	int bxsize = m_width;
	if ((m_yPos + PIVOT_Y + CHAR_HEIGHT) < m_height) 
	{
		m_yPos += CHAR_HEIGHT; //커서를 다음행으로 옮긴다.
	}
	else
	{
		//화면을 스크롤한다.
		for (y = PIVOT_Y; y < (m_height - CHAR_HEIGHT - PIVOT_Y); y++)
		{
			for (x = PIVOT_X; x < m_width/2; x++) 
			{
				buf[x + y * bxsize] = buf[x + (y + CHAR_HEIGHT) * bxsize];
			}
		}
		for (y = m_height - CHAR_HEIGHT - PIVOT_Y; y < (m_height - PIVOT_Y); y++) 
		{
			for (x = PIVOT_X; x < m_width; x++) 
			{
				buf[x + y * bxsize] = 0x00000000;
			}
		}

	}

	m_xPos = PIVOT_X;
}
bool SkyGUILua::Clear()
{
	m_xPos = PIVOT_X;
	m_yPos = PIVOT_Y;

	memset(m_pVideoRamPtr, 0, (m_width * m_height) * sizeof(ULONG));

	return true;
}

void SkyGUILua::Update(unsigned long *buf)
{
	unsigned long *p = m_pVideoRamPtr, *p2 = buf;

	for (int c = 0; c<m_width * m_height; c++) 
	{
		*p = *p2;
		p++;
		p2++;
	}
}

bool SkyGUILua::Print(char* pMsg)
{
	if (m_pRenderer == nullptr)
		return false;

	//백스페이스
	if (strlen(pMsg) == 1 && pMsg[0] == 0x08)
	{
		if (m_xPos > 9 * 8)
		{
			FillRect(m_xPos, m_yPos, CHAR_WIDTH, CHAR_HEIGHT, 0x00);
			m_xPos -= 1 * 8;
			FillRect(m_xPos, m_yPos, CHAR_WIDTH, CHAR_HEIGHT, 0x00);

			PutCursor();
		}

		return true;
	}

	FillRect(m_xPos, m_yPos, CHAR_WIDTH, CHAR_HEIGHT, 0x00);

	unsigned char *s = (unsigned char*)pMsg;
	for (; *s != 0x00; s++)
	{
		if (*s == '\n')
		{
			GetNewLine();
			continue;
		}

		if (*s == '\r')
		{
			GetNewLine();
			continue;
		}

		if (!isascii(*s) || (*s) < 0x20)
			continue;

		m_pRenderer->PutFont((char*)m_pVideoRamPtr, m_width, m_xPos, m_yPos, CHAR_COLOR, skyFontData + *s * 16);
		m_xPos += CHAR_WIDTH;
	}

	PutCursor();

	return true;
}

bool SkyGUILua::PrintUnicode(char* pMsg)
{
	FillRect(m_xPos, m_yPos, CHAR_WIDTH, CHAR_HEIGHT, 0x00);
	I_Hangul* pEngine = SkyGUISystem::GetInstance()->GetUnicodeEngine();
	pEngine->PutFonts((char*)SkyGUISystem::GetInstance()->GetVideoRamInfo()._pVideoRamPtr, m_width, m_xPos, m_yPos, 0xffffffff, (unsigned char*)pMsg);
	GetNewLine();

	return true;
}

bool SkyGUILua::PrintCommand(char* pMsg, bool backspace)
{
	I_HangulEngine* pIMEEngine = SkyGUISystem::GetInstance()->GetIMEEngine();
	if (m_pRenderer == nullptr)
		return false;

	m_xPos = PIVOT_X;

	if (backspace == true)
	{		
		FillRect(m_lastCommandLength, m_yPos, CHAR_WIDTH, CHAR_HEIGHT, 0x00);
	}

	m_lastCommandLength = pIMEEngine->DrawText(0, 0, m_width, m_height, (DWORD*)m_pVideoRamPtr, m_xPos, m_yPos, 0xffffffff, 0x00, pMsg, strlen(pMsg));

	m_lastCommandLength += PIVOT_X;
	m_xPos = m_lastCommandLength;
	PutCursor();

	return true;
}

bool SkyGUILua::PutKeyboardQueue(KEYDATA* pData)
{
	return m_pSkyInputManager->PutKeyQueue(pData);
}

bool SkyGUILua::PutMouseQueue(MOUSEDATA* pData)
{
	return true;
}

//////////////////////////////////////////////
//그래픽 출력 관련
void SkyGUILua::PutCursor()
{
	FillRect(m_xPos, m_yPos, CHAR_WIDTH * 2, CHAR_HEIGHT, 0x00);
	FillRect(m_xPos, m_yPos + (CHAR_HEIGHT - 4), CHAR_WIDTH, 4, 0xffffffff);
}

void SkyGUILua::PutPixel(ULONG i, unsigned char r, unsigned char g, unsigned char b) {
	m_pVideoRamPtr[i] = (r << 16) | (g << 8) | b;
}


ULONG SkyGUILua::GetBPP()
{
	return m_bpp;
}

void SkyGUILua::PutPixel(ULONG x, ULONG y, ULONG col)
{
	m_pVideoRamPtr[(y * m_width) + x] = col;
}

ULONG SkyGUILua::GetPixel(ULONG i) {
	return m_pVideoRamPtr[i];
}

void SkyGUILua::PutPixel(ULONG i, ULONG col) {
	m_pVideoRamPtr[i] = col;
}

void SkyGUILua::FillRect(int x, int y, int w, int h, int col)
{
	unsigned* lfb = (unsigned*)m_pVideoRamPtr;
	for (int k = 0; k < h; k++)
		for (int j = 0; j < w; j++)
		{
			int index = ((j + x) + (k + y) * m_width);
			lfb[index] = col;
		}
}
