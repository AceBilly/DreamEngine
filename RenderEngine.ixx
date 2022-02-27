module;
#include "helper.h"
#include "d3dx12.h"
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <filesystem>
#pragma comment(lib, "dxgi")
export module RenderEngine;
import utility;
import RenderEngineInterface;
namespace fs = std::filesystem;
using namespace Microsoft::WRL;


export class Directx12RenderEngine 
	: public RenderEngineInterface {
public:
	Directx12RenderEngine(UINT _width, UINT _height, HWND _handle, HINSTANCE instance);
	~Directx12RenderEngine() = default;
public:
	void init() override;
	void setViewport(D3D12_VIEWPORT viewport);
	void setScissor(D3D12_RECT scissor);
	void onUpdate() override;
	void onRender() override;
	void onDestory()override;
protected:

private:
	void initPipeLine();  //初始化图形管线所需要的对象；
	void loadAssets(); // 加载资产
	void populateCommandList(); // 意如其名
	void waitPrevFrame();
private:
	// pipeLine Object
	ComPtr<ID3D12Device> m_device;
	ComPtr<IDXGISwapChain3> m_swapChain; // 交换链
	ComPtr<ID3D12CommandQueue> m_commandQueue;  // 命令队列
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;  // 渲染目标视图描述符堆
	UINT m_rtvDescriptorSize;  // 目标渲染视图描述符大小
	static const UINT m_frameCount = 2;  // 交换链所使用的缓冲区数量
	ComPtr<ID3D12Resource> m_renderTargets[m_frameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipeLineState;
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;
	HANDLE m_fenceEvent;
private:
	// custom object
	bool m_useWarpAdapter = false;  // 是否使用base rendering warp render（software adapter)
	// UINT m_width, m_height;  // 视口高度与宽度(缓冲区大小width * height）
	UINT m_frameIndex = 0; // 当前缓冲区索引

	D3D12_VIEWPORT m_viewport;  // 视口大小
	D3D12_RECT m_scissor;  // 裁剪矩形
};

module: private;
void Directx12RenderEngine::loadAssets() {
	// create empty root signature
	{
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		ComPtr<ID3DBlob> rootSignature;
		ComPtr<ID3DBlob> err;
		throwIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignature, &err), "SerializeRootSignature failed");
		throwIfFailed(m_device->CreateRootSignature(0, rootSignature->GetBufferPointer(), rootSignature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)), "Create root signature failed");
	}
	// create pipeline state
	{
		// create compiler shader
		ComPtr<ID3D10Blob> vertexShader, fragmentShader;
		UINT compileFlags = 0;
		if constexpr (_DEBUG) {
			// TODO： 启用着色器调试工具。
			compileFlags = 0;
		} 
		else {
			compileFlags = 0;
		}
		D3DCompileFromFile(getAssetFullPath("VertexShader.hlsl").c_str(), nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
		D3DCompileFromFile(getAssetFullPath("PixelShader.hlsl").c_str(), nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &fragmentShader, nullptr);
		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
		psoDesc.PS = { reinterpret_cast<UINT8*>(fragmentShader->GetBufferPointer()), fragmentShader->GetBufferSize() };
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		throwIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipeLineState)), "Failed to create graphicsPipeline state");
	}
// Create the command list.
throwIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipeLineState.Get(), IID_PPV_ARGS(&m_commandList)), "Failed to create command list");
// 创建视口
{
	m_commandList->RSSetViewports(1, &m_viewport);
	D3D12_RECT scissor_rectangle = { 0, 0, m_width / 2, m_height / 2 };
	m_commandList->RSSetScissorRects(1, &scissor_rectangle);
}
// Command lists are created in the recording state, but there is nothing
// to record yet. The main loop expects it to be closed, so close it now.
throwIfFailed(m_commandList->Close(), "Failed to close command list");

// Create the vertex buffer.
{
	// Define the geometry for a triangle.
	Vertex0 triangleVertices[] =
	{
		{ { 0.0f, 0.25f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.25f, -0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
	};

	const UINT vertexBufferSize = sizeof(triangleVertices);

	// Note: using upload heaps to transfer static data like vert buffers is not 
	// recommended. Every time the GPU needs it, the upload heap will be marshalled 
	// over. Please read up on Default Heap usage. An upload heap is used here for 
	// code simplicity and because there are very few verts to actually transfer.
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	throwIfFailed(m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)), "Failed to create committed resourse");

	// Copy the triangle data to the vertex buffer.
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
	throwIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)), "Unkonw error");
	memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
	m_vertexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(Vertex0);
	m_vertexBufferView.SizeInBytes = vertexBufferSize;
}

// Create synchronization objects and wait until assets have been uploaded to the GPU.
{
	throwIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "Failed to create fence");
	m_fenceValue = 1;

	// Create an event handle to use for frame synchronization.
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		throwIfFailed(HRESULT_FROM_WIN32(GetLastError()), "placeholder");
	}

	// Wait for the command list to execute; we are reusing the same command 
	// list in our main loop but for now, we just want to wait for setup to 
	// complete before continuing.
	waitPrevFrame();
}
}

void Directx12RenderEngine::populateCommandList() {
	// Command list allocators can only be reset when the associated 
   // command lists have finished execution on the GPU; apps should use 
   // fences to determine GPU execution progress.
	throwIfFailed(m_commandAllocator->Reset(), "failed to reset command allocator");

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	throwIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipeLineState.Get()), "failed to reset command list");

	// Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissor);

	// Indicate that the back buffer will be used as a render target.
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &barrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->DrawInstanced(3, 1, 0, 0);

	// Indicate that the back buffer will now be used to present.
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &barrier);

	throwIfFailed(m_commandList->Close(), "failed to closed command list");
}
void Directx12RenderEngine::waitPrevFrame() {
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. More advanced samples 
	// illustrate how to use fences for efficient resource usage.

	// Signal and increment the fence value.
	const UINT64 fence = m_fenceValue;
	throwIfFailed(m_commandQueue->Signal(m_fence.Get(), fence), "failed to signal");
	m_fenceValue++;

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < fence)
	{
		throwIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent), "failed to set event on completion");
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}
Directx12RenderEngine::Directx12RenderEngine(UINT _width, UINT _height, HWND _handle, HINSTANCE instance)
	:RenderEngineInterface(_height, _width, _handle, instance) {
	// 设置视口和裁剪矩阵的大小
	setViewport({
			.TopLeftX = 0.f, .TopLeftY = 0.f,
			.Width = static_cast<float>(m_width),
			.Height = static_cast<float>(m_height),
			.MinDepth = 0.f, .MaxDepth = 1.f
		});
	setScissor({ 0, 0, static_cast<long>(m_width / 2), static_cast<long>(m_height / 2)});
}

void Directx12RenderEngine::setViewport(D3D12_VIEWPORT viewport) {
	m_viewport = viewport;
}
void Directx12RenderEngine::setScissor(D3D12_RECT scissor) {
	m_scissor = scissor;
}

void Directx12RenderEngine::onUpdate()
{
}

void Directx12RenderEngine::onRender() {
	// Record all the commands we need to render the scene into the command list.
	populateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	throwIfFailed(m_swapChain->Present(1, 0),"failed to present");

	waitPrevFrame();
}

void Directx12RenderEngine::onDestory()
{
	waitPrevFrame();
	CloseHandle(m_fenceEvent);
}

void Directx12RenderEngine::initPipeLine() {
	//  若处于debug那么启用调试层；
	if constexpr (_DEBUG) {
		ComPtr<ID3D12Debug> pDebugLayers;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugLayers)))) {
			pDebugLayers->EnableDebugLayer();
		}
	}
	ComPtr<IDXGIFactory4> pFactory;
	throwIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&pFactory)), "Factory create Failed");
	//  创建device
	{
		if (m_useWarpAdapter) {
			ComPtr<IDXGIAdapter> pWarpAdapter;
			throwIfFailed(pFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)), "enumAdapterFailed");
			throwIfFailed(D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&m_device)), "warpAdapterCreateFailed");
		}
		else {
			ComPtr<IDXGIAdapter1> pHardwareAdapter;
			getHardwareAdapter(pFactory.Get(), pHardwareAdapter.Get());
			throwIfFailed(D3D12CreateDevice(pHardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&m_device)), "HardwareAdapterCreateFailed");
		}
	}
	// 创建命令队列(command queue)
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		throwIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)), "Command queue created failed!");
	}
	//  创建交换链
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = m_frameCount;
		swapChainDesc.BufferDesc.Width = m_width;
		swapChainDesc.BufferDesc.Height = m_height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.OutputWindow = RenderEngineInterface::m_windowHandle;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Windowed = TRUE;
		ComPtr<IDXGISwapChain> swapChain;
		throwIfFailed(pFactory->CreateSwapChain(
			m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
			&swapChainDesc,
			&swapChain
		), "swap chain created failed");
		swapChain.As(&m_swapChain);
	}
	{
		// This sample does not support fullscreen transitions.
		throwIfFailed(pFactory->MakeWindowAssociation(m_windowHandle, DXGI_MWA_NO_ALT_ENTER),"error while bind windows");

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		// Create descriptor heaps.
		{
			// Describe and create a render target view (RTV) descriptor heap.
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = m_frameCount;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			throwIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)), "create descriptor heaps failed");

			m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}

		// Create frame resources.
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

			// Create a RTV for each frame.
			for (UINT n = 0; n < m_frameCount; n++)
			{
				throwIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])), "Create frame resources");
				m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
				rtvHandle.Offset(1, m_rtvDescriptorSize);
			}
		}

		throwIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)), "create command allocator failed");
	}
}

void Directx12RenderEngine::init() {
	initPipeLine();
	loadAssets();
}