module;
#include <memory>
#include <Windows.h>
#include <iostream>

export module Win32Application;
import Timer;
import RenderEngineInterface;
import RenderEngine;
using ProcessMessageCallback = LRESULT (*)(HWND, UINT, WPARAM, LPARAM);


export class Win32Application {
public:
	Win32Application();
	~Win32Application() = default;

	using pTimer = std::shared_ptr<Timer>;
	pTimer& getpTimer();
public:
	static HWND getHwnd() { return m_hwnd; }
public:
	// ��ʼ������
	// @param hInstance Ӧ�ó���ʵ��
	// @param nCmdShow ���ڵ���ʾ��ʽ
	// @return �����ʼ��ʧ�ܷ���flase �ɹ�����true
	static bool initWin32Application(HINSTANCE hInstance, int nCmdShow, LPCWSTR windowTitle, UINT height, UINT width);
	// ���г���
	static int run();
	// �����Ⱦ����
	static void addRenderEngine();
private:
	// Ĭ����Ϣ������
	static LRESULT wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	static ProcessMessageCallback pm_wndProc;
	static HWND m_hwnd;
	static HINSTANCE m_instance;
private:
	std::shared_ptr<Timer> pm_timer;
	static std::unique_ptr<RenderEngineInterface> pm_renderEngine;
	static UINT m_height, m_width;

};

module: private;


Win32Application::Win32Application():pm_timer(std::make_shared<Timer>()) {

}
Win32Application::pTimer& Win32Application::getpTimer() {
	return pm_timer;
}

HWND Win32Application::m_hwnd = 0;
std::unique_ptr<RenderEngineInterface> Win32Application::pm_renderEngine = nullptr;
ProcessMessageCallback Win32Application::pm_wndProc = &Win32Application::wndProc;
UINT Win32Application::m_height = 0;
UINT Win32Application::m_width = 0;
HINSTANCE Win32Application::m_instance = 0;

bool Win32Application::initWin32Application(HINSTANCE hInstance, int nCmdShow, LPCWSTR title, UINT height, UINT width) {
	m_height = height;
	m_width = width;
	m_instance = hInstance;
	WNDCLASS wc = {
		.style = CS_HREDRAW | CS_VREDRAW, // ���ڸ߶Ȼ��ȸı�ʱ���»��ƴ���
		.lpfnWndProc = pm_wndProc, // ��Ϣ����������ʱnullptr
		.cbClsExtra = 0, .cbWndExtra = 0, // ������ڴ�ռ䣬��ʱ����Ҫ
		.hInstance = hInstance, // Ӧ�ó���ʵ��
		.hIcon = LoadIcon(0, IDC_ICON), .hCursor = LoadCursorW(0, IDC_ARROW), // TODO(AceBIlly): ָ��ͳ���ͼ����ڸ�������Ĭ��ͼ��
		.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)),
		.lpszMenuName = 0, // �˵�����
		.lpszClassName = L"BaseWndCLASS"
		};
	// ע�ᴰ��
	if (!RegisterClass(&wc)) {
		MessageBox(0, L"register class wc failed", 0, 0);
		return false;
	}
	constexpr UINT32 style = WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZE; // Default ��ʱ��֧������
	m_hwnd = CreateWindow(L"BaseWndCLASS", title, style, 
		                  CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, hInstance, 0);
	if (!m_hwnd) {
		MessageBox(0, L"Create window failed", 0, 0);
		return false;
	}
	ShowWindow(m_hwnd, nCmdShow);
	UpdateWindow(m_hwnd);
	return true;
}

int Win32Application::run() {
	MSG msg = { 0 };
	if (pm_renderEngine) pm_renderEngine->init();
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			// game logic
			continue;
		}
	}
	pm_renderEngine->onDestory();
	return static_cast<int>(msg.wParam);
}

void Win32Application::addRenderEngine()
{
	pm_renderEngine = std::make_unique<Directx12RenderEngine>(m_width, m_height, m_hwnd, m_instance);
	if (pm_renderEngine == nullptr) throw std::runtime_error("failed load render engine");
}

LRESULT Win32Application::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_PAINT:
		if (pm_renderEngine == nullptr) return 0;
		pm_renderEngine->onUpdate();
		pm_renderEngine->onRender();
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return  DefWindowProc(hWnd, msg, wParam, lParam);;
}
