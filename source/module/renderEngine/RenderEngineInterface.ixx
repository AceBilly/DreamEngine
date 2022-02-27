module;
#include <Windows.h>
export module RenderEngineInterface;

export class RenderEngineInterface {
public:
	RenderEngineInterface(UINT height, UINT width, HWND windowHwnd, HINSTANCE appHinstance)
		:m_height(height), m_width(width), m_windowHandle(windowHwnd), m_appHinstance(appHinstance) { 

	}
	virtual ~RenderEngineInterface() {};
public:
	virtual void init() = 0;
	virtual void onUpdate() = 0;
	virtual void onRender() = 0;
	virtual void onDestory() = 0;

	
protected:
	HWND m_windowHandle;  // Win32 窗口
	HINSTANCE m_appHinstance;  // win32 程序 instance
	UINT m_height, m_width;  // 视口/ 窗口大小
};