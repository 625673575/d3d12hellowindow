//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "D3D12HelloWindow.h"
#include "vs_solid.h"
#include "ps_solid.h"
#include "WarpVS.h"
#include "WarpPS.h"
#include "DistortionMapPixel.h"
#include "Distortion_PS.h"


D3D12HelloWindow::D3D12HelloWindow(UINT width, UINT height, std::wstring name) :
	DXSample(width, height, name),
	m_fenceValues{0,0},
	m_frameIndex(0),
	m_pCbvDataBegin(nullptr),
	m_rtvDescriptorSize(0),
	m_timewarpData{}
{
}

void D3D12HelloWindow::OnInit()
{
	if (Tracker::LoadTrackDll() == false) {
		MessageBox(nullptr, L"Error", L"er", 0);
	}
	LoadPipeline();
}

// Load the rendering pipeline dependencies.
void D3D12HelloWindow::LoadPipeline()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));//创建DXGI工厂

	if (m_useWarpDevice)
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));//枚举设备

		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(&m_device)
		));//创建Device
	}
	else
	{
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(factory.Get(), &hardwareAdapter);

		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(&m_device)
		));
	}

	// 创建CommandQueue,DIRECT用于执行CommandList
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	// 创建SwapChain,需要传入commandQueue和窗口的hwnd
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
		Win32Application::GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	// 不支持ALT+ENTER切换全屏
	ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();//m_frameIndex初始化赋值

	// 创建RenderTarget的Descriptor Heap,注意Type是D3D12_DESCRIPTOR_HEAP_TYPE_RTV,Num代表要包含几个RenderTarget,这里+1用于处理中间层
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount + 1;//用于中间写入用的
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap)));

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);//获取单个Heap的大小，用于获取handle的时候进行偏移的获取
	}

	// 创建RenderTarget,和对应的CommandAllocator,同样的需要对应的CommandListType
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		for (UINT n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);

			ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])));
		}
	}

	// 创建RootSignature,这个描述了shader的各种资源srv,cbv,uav的register位置
	{
		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		m_featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &m_featureData, sizeof(m_featureData))))
		{
			m_featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 rangesTex[1];
		rangesTex[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);//这里用到了t0和t1分别传入原贴图和distortion map,通常尽量保持连续

		CD3DX12_ROOT_PARAMETER1 rootParameters[3];//Root Parameter的下标很重要,在后面SetGraphicsRootDescriptorTable要传入对应的0,SetGraphicsRootConstantBufferView要传入1,2
		//创建DescriptorTable存储多个cbv
		rootParameters[0].InitAsDescriptorTable(1, &rangesTex[0], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[CB_TIMEWARP_INDEX].InitAsConstantBufferView(3, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);//这里因为不是连续的,且Shader的Visibility不一样,所以选择单独创建多个ConstantBufferView
		rootParameters[CB_CHROMATIC_INDEX].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

		ComPtr<ID3DBlob> signature;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, m_featureData.HighestVersion, &signature, nullptr));//通过desc序列化Signature描述
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));//创建root signature
	}
	// Size the viewport
	{
		const auto rtDesc = m_renderTargets[0]->GetDesc();

		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;
		m_viewport.Width = static_cast<float>(rtDesc.Width);
		m_viewport.Height = static_cast<float>(rtDesc.Height);
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		m_scissorRect.left = 0;
		m_scissorRect.top = 0;
		m_scissorRect.right = static_cast<LONG>(rtDesc.Width);
		m_scissorRect.bottom = static_cast<LONG>(rtDesc.Height);
	}

	// pipeline state object 定义了和shader相关联的一切,Input Layout,Shader二进制流,BlendState,RasterizerState,DepthStencilState,PrimitiveTopologyType,等
	{
		const D3D12_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{ };
		pipelineStateDesc.pRootSignature = m_rootSignature.Get();
		pipelineStateDesc.VS = { g_WARP_VS , sizeof(g_WARP_VS) };
		pipelineStateDesc.PS = { g_PS_Distortion, sizeof(g_PS_Distortion) };//g_WARP_PS 是直接输出贴图
		pipelineStateDesc.BlendState.AlphaToCoverageEnable = FALSE;
		pipelineStateDesc.BlendState.IndependentBlendEnable = FALSE;
		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		{
			pipelineStateDesc.BlendState.RenderTarget[i].BlendEnable = FALSE;
			pipelineStateDesc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;
			pipelineStateDesc.BlendState.RenderTarget[i].SrcBlend = D3D12_BLEND_ONE;
			pipelineStateDesc.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_ZERO;
			pipelineStateDesc.BlendState.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
			pipelineStateDesc.BlendState.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
			pipelineStateDesc.BlendState.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;
			pipelineStateDesc.BlendState.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			pipelineStateDesc.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
			pipelineStateDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		}
		pipelineStateDesc.SampleMask = UINT_MAX;
		pipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		pipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		pipelineStateDesc.RasterizerState.FrontCounterClockwise = FALSE;
		pipelineStateDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		pipelineStateDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		pipelineStateDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		pipelineStateDesc.RasterizerState.DepthClipEnable = TRUE;
		pipelineStateDesc.RasterizerState.MultisampleEnable = FALSE;
		pipelineStateDesc.RasterizerState.AntialiasedLineEnable = FALSE;
		pipelineStateDesc.RasterizerState.ForcedSampleCount = 0;
		pipelineStateDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		pipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
		pipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
		pipelineStateDesc.InputLayout = { layout, _countof(layout) };
		pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipelineStateDesc.NumRenderTargets = 1;
		pipelineStateDesc.RTVFormats[0] = m_renderTargets[0]->GetDesc().Format;
		pipelineStateDesc.SampleDesc.Count = 1;

		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(m_pipelineState.GetAddressOf())));
	}

	std::vector<VertexUV> vertices;
	std::vector<DWORD> indicies;
	D3D12_HEAP_PROPERTIES heapProps{ };
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDesc{ };
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	CreateGridMeshVertexUV(4, m_vertex_num, m_indicies_num, vertices, indicies);
	{
		UINT VertexDataSize = m_vertex_num * sizeof(VertexUV);

		resourceDesc.Width = VertexDataSize;
		ThrowIfFailed(m_device->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer)));//创建读取的Buffer

		UINT8* pVertexDataBegin = nullptr;
		ThrowIfFailed(m_vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pVertexDataBegin)));//map映射到CPU可操作的位置
		memcpy(pVertexDataBegin, &vertices[0], VertexDataSize);//拷贝数据到Buffer中
		m_vertexBuffer->Unmap(0, nullptr);//中间不会再做更改了直接Unmap
		// Initialize the vertex buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();//GPU位置
		m_vertexBufferView.StrideInBytes = sizeof(VertexUV);//单个数据的大小
		m_vertexBufferView.SizeInBytes = VertexDataSize;//整个数据段的大小
	}
	// Create the index buffer.
	{
		UINT IndexDataSize = m_indicies_num * sizeof(DWORD);

		resourceDesc.Width = IndexDataSize;

		ThrowIfFailed(m_device->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(&m_indexBuffer)));

		UINT8* pIndexDataBegin = nullptr;
		ThrowIfFailed(m_indexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pIndexDataBegin)));
		memcpy(pIndexDataBegin, &indicies[0], IndexDataSize);
		m_indexBuffer->Unmap(0, nullptr);

		// Describe the index buffer view.
		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;//需注意,如果创建的是WORD类型,sizeof(WORD)==2,则是DXGI_FORMAT_R16_UINT
		m_indexBufferView.SizeInBytes = IndexDataSize;
	}

	// Create the command list.用于记录提交的命令
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), nullptr, IID_PPV_ARGS(&m_commandList)));

	// 存储srv cbv的descriptor heap,因为要传入shader,所以flag要设为D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	{
		D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = {};
		cbvSrvHeapDesc.NumDescriptors = 10;                            //
		cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(&m_leftCbvSrvHeap)));
		m_cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);//获取增量大小,用于后面handle的获取
	}
	// Dx12中的ConstantBuffer可以一次包含多个变量,然后通过获取handle进行设置到指定的slot
	// memcpy(m_pCbvDataBegin + m_constantBufferPerSize * 2, &m_timewarpData, sizeof(m_timewarpData));
	// m_commandList->SetGraphicsRootConstantBufferView(1, m_constantBuffer->GetGPUVirtualAddress() + m_constantBufferPerSize * 2);
	{
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_constantBufferPerSize * 3),//左右眼的Timewarp 矩阵+chromatic数据
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBuffer)));

		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
		
		CBChromatic chromeData;
		chromeData.GDist = 1.0025f;
		chromeData.BDist = 1.014001f;
		memcpy(m_pCbvDataBegin, &chromeData, sizeof(chromeData));//固定不变的数据,所以在这里直接赋值

		//memcpy(m_pCbvDataBegin + m_constantBufferPerSize , &m_timewarpData, sizeof(m_timewarpData));
		//memcpy(m_pCbvDataBegin + m_constantBufferPerSize*2 , &m_timewarpData, sizeof(m_timewarpData));

	}
	// Note: ComPtr's are CPU objects but this resource needs to stay in scope until
	// the command list that references it has finished executing on the GPU.
	// We will flush the GPU at the end of this method to ensure the resource is not
	// prematurely destroyed.
	ComPtr<ID3D12Resource> textureUploadHeap[2];
	// Create the texture.
	{
		// Describe and create a Texture2D.
		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.MipLevels = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.Width = 2048;
		textureDesc.Height = 2048;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_leftTexture))); 
		ThrowIfFailed(m_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&textureDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_rightTexture)));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_leftTexture.Get(), 0, 1);

		// Create the GPU upload buffer.这个用于为Texture内容进行赋值,这里必须要创建两个
		//因为涉及到ResourceBarrier的操作需要在CommandList执行的时候才会起作用
		//所以必须要创建不同的副本,并保证其资源在ExecuteCommandList之前都需要存在
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&textureUploadHeap[0])));
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&textureUploadHeap[1])));

		std::vector<UINT8> texture = GenerateTextureData(2048, 2048,4,4);
		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = &texture[0];
		textureData.RowPitch = 2048 * 4;
		textureData.SlicePitch = textureData.RowPitch * 2048;
		//更新textureData到m_texture
		UpdateSubresources(m_commandList.Get(), m_leftTexture.Get(), textureUploadHeap[0].Get(), 0, 0, 1, &textureData);
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_leftTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		// 创建m_texture的srv,放到m_cbvSrvHeap的开始,采取的是descriptor heap存储,所以需要创建View并指定Handle的地址,后面的SetDescriptorHeap的时候也要传入地址
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		m_device->CreateShaderResourceView(m_leftTexture.Get(), &srvDesc, m_leftCbvSrvHeap->GetCPUDescriptorHandleForHeapStart());

		std::vector<UINT8> texture2 = GenerateTextureData(2048, 2048, 12, 8);
		D3D12_SUBRESOURCE_DATA textureData2 = {};
		textureData2.pData = &texture2[0];
		textureData2.RowPitch = 2048 * 4;
		textureData2.SlicePitch = textureData2.RowPitch * 2048;
		UpdateSubresources(m_commandList.Get(), m_rightTexture.Get(), textureUploadHeap[1].Get(), 0, 0, 1, &textureData2);
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_rightTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle2(m_leftCbvSrvHeap->GetCPUDescriptorHandleForHeapStart(), 2, m_cbvSrvDescriptorSize);//偏移为2
		m_device->CreateShaderResourceView(m_rightTexture.Get(), &srvDesc, srvHandle2);
	}
	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	//创建一个DistortionMap到Intermediate Render Target,用于创建畸变
	GenerateDistortionTexture();
	// Create synchronization object 
	{
		ThrowIfFailed(m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceValues[m_frameIndex]++;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		WaitForGPU();
	}
}

glm::mat4 D3D12HelloWindow::CalculateProjectionMatrix(EyeType eye)
{
	//Project the UVs in NDC onto the far plane and convert from NDC to viewport space
	glm::mat4 retMtx;
	if (eye == EyeType::LeftEye) {
		retMtx[0] = glm::vec4(2.0f, 0.0f, 0.0f, 1.0f);
		retMtx[1] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
		retMtx[2] = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
		retMtx[3] = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
	}
	else if (eye == EyeType::RightEye) {
		retMtx[0] = glm::vec4(2.0f, 0.0f, 0.0f, -1.0f);
		retMtx[1] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
		retMtx[2] = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
		retMtx[3] = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
	}
	else {
		retMtx[0] = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
		retMtx[1] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
		retMtx[2] = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
		retMtx[3] = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
	}
	return glm::transpose(retMtx);
}
void D3D12HelloWindow::GenerateDistortionTexture()
{
	const float IntermediateClearColor[4] = { 0.0f, 0.2f, 0.3f, 1.0f };
	//create intermediate render target
	{
		// 创建一个贴图的描述
		D3D12_RESOURCE_DESC renderTargetDesc = m_renderTargets[0]->GetDesc();
		renderTargetDesc.Width = 2048;
		renderTargetDesc.Height = 2048;
		renderTargetDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
		renderTargetDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		memcpy(clearValue.Color, IntermediateClearColor, sizeof(IntermediateClearColor));
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), FrameCount, m_rtvDescriptorSize);//创建在正常Frame的RenderTarget后面
		// Create an intermediate render target that is the same dimensions as the swap chain.
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_SHARED,
			&renderTargetDesc,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			&clearValue,
			IID_PPV_ARGS(&m_intermediateRenderTarget)));

		m_device->CreateRenderTargetView(m_intermediateRenderTarget.Get(), nullptr, rtvHandle);//创建RenderTargetView

		// Create a SRV of the intermediate render target.用于后续的显示,起点用于m_texture 的显示,所以这个采取1的位置
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = renderTargetDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_leftCbvSrvHeap->GetCPUDescriptorHandleForHeapStart(),1,m_cbvSrvDescriptorSize);//偏移为1,0是主贴图,1是DistortionMap因为在RangeTeX中的位置是1
		m_device->CreateShaderResourceView(m_intermediateRenderTarget.Get(), &srvDesc, srvHandle);
		CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle2(m_leftCbvSrvHeap->GetCPUDescriptorHandleForHeapStart(), 3, m_cbvSrvDescriptorSize);//偏移为1,0是主贴图,1是DistortionMap因为在RangeTeX中的位置是1
		m_device->CreateShaderResourceView(m_intermediateRenderTarget.Get(), &srvDesc, srvHandle2);
	}
	// Create root signature
	{
		CD3DX12_DESCRIPTOR_RANGE1 rangesTex[1];
		rangesTex[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		CD3DX12_ROOT_PARAMETER1 rootParameters[2];
		rootParameters[0].InitAsConstantBufferView(3, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);   //CBTimewarp 在c3
		rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);    //CBDistortion 在c1
		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

		ComPtr<ID3DBlob> signature;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, m_featureData.HighestVersion, &signature, nullptr));
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_distortionSignature)));
	}
	// Size the viewport
	{
		const auto rtDesc = m_intermediateRenderTarget->GetDesc();

		m_intermediateViewport.TopLeftX = 0.0f;
		m_intermediateViewport.TopLeftY = 0.0f;
		m_intermediateViewport.Width = static_cast<float>(rtDesc.Width);
		m_intermediateViewport.Height = static_cast<float>(rtDesc.Height);
		m_intermediateViewport.MinDepth = 0.0f;
		m_intermediateViewport.MaxDepth = 1.0f;

		m_scissorRect.left = 0;
		m_scissorRect.top = 0;
		m_scissorRect.right = static_cast<LONG>(rtDesc.Width);
		m_scissorRect.bottom = static_cast<LONG>(rtDesc.Height);
	}

	//create Quad mesh
	{
		static const VertexUV quadVertices[] =
		{
			{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },    // Bottom Left
			{ { -1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },    // Top Left
			{ { 1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } },    // Bottom Right
			{ { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },        // Top Right
		};
		const UINT vertexBufferSize = sizeof(quadVertices);


		D3D12_HEAP_PROPERTIES heapProps{ };
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC resourceDesc{ };
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resourceDesc.Width = vertexBufferSize;

		ThrowIfFailed(m_device->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_quadVertexBuffer)));

		UINT8* pVertexDataBegin = nullptr;
		ThrowIfFailed(m_quadVertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, &quadVertices[0], vertexBufferSize);
		m_quadVertexBuffer->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_quadVbv.BufferLocation = m_quadVertexBuffer->GetGPUVirtualAddress();
		m_quadVbv.StrideInBytes = sizeof(VertexUV);
		m_quadVbv.SizeInBytes = vertexBufferSize;
	}

	// Create pipeline state object
	{
		const D3D12_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{ };
		pipelineStateDesc.pRootSignature = m_distortionSignature.Get();
		pipelineStateDesc.VS = { g_WARP_VS , sizeof(g_WARP_VS) };
		pipelineStateDesc.PS = { g_PS_DistortionMap, sizeof(g_PS_DistortionMap) };
		pipelineStateDesc.BlendState.AlphaToCoverageEnable = FALSE;
		pipelineStateDesc.BlendState.IndependentBlendEnable = FALSE;
		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		{
			pipelineStateDesc.BlendState.RenderTarget[i].BlendEnable = FALSE;
			pipelineStateDesc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;
			pipelineStateDesc.BlendState.RenderTarget[i].SrcBlend = D3D12_BLEND_ONE;
			pipelineStateDesc.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_ZERO;
			pipelineStateDesc.BlendState.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
			pipelineStateDesc.BlendState.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
			pipelineStateDesc.BlendState.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;
			pipelineStateDesc.BlendState.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			pipelineStateDesc.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
			pipelineStateDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		}
		pipelineStateDesc.SampleMask = UINT_MAX;
		pipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		pipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		pipelineStateDesc.RasterizerState.FrontCounterClockwise = FALSE;
		pipelineStateDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		pipelineStateDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		pipelineStateDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		pipelineStateDesc.RasterizerState.DepthClipEnable = FALSE;
		pipelineStateDesc.RasterizerState.MultisampleEnable = FALSE;
		pipelineStateDesc.RasterizerState.AntialiasedLineEnable = FALSE;
		pipelineStateDesc.RasterizerState.ForcedSampleCount = 0;
		pipelineStateDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		pipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
		pipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
		pipelineStateDesc.InputLayout = { layout, _countof(layout) };
		pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipelineStateDesc.NumRenderTargets = 1;
		pipelineStateDesc.RTVFormats[0] = m_intermediateRenderTarget->GetDesc().Format;
		pipelineStateDesc.SampleDesc.Count = 1;

		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(m_distortionPipelineState.GetAddressOf())));
	}

	//create constant buffer
	{
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_constantBufferPerSize + m_distortionConstantBufferPerSize),//用于vs的和ps的各一个constant buffer
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_distortionConstantBuffer)));
		UINT8* pCbvDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(m_distortionConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pCbvDataBegin)));
		UINT8* pCbDistortionBegin = pCbvDataBegin + m_constantBufferPerSize;

		CBTimewarp warpData{ CalculateProjectionMatrix(EyeType::Both),glm::mat4(1) };
		memcpy(pCbvDataBegin, &warpData, sizeof(warpData));

		CBDistortion distortionData;
		distortionData.k0 = 0.851f;
		distortionData.k1 = -0.034544f;
		distortionData.k2 = 0.71243f;
		distortionData.k3 = -2.5189f;
		distortionData.k4 = 6.6348f;
		distortionData.k5 = -7.96f;
		distortionData.k6 = 3.8567f;

		distortionData.rk0 = 0.9968f;
		distortionData.rk1 = -0.0007;
		distortionData.rk2 = 0.011852f;
		distortionData.rk3 = -0.04772f;
		distortionData.rk4 = 0.122f;
		distortionData.rk5 = -0.1454141f;
		distortionData.rk6 = 0.0f;

		distortionData.gk0 = 1.0f;
		distortionData.gk1 = .0f;
		distortionData.gk2 = .0f;
		distortionData.gk3 = .0f;
		distortionData.gk4 = .0f;
		distortionData.gk5 = .0f;
		distortionData.gk6 = .0f;

		distortionData.bk0 = 1.00135f;
		distortionData.bk1 = 0.000026f;
		distortionData.bk2 = -0.00473f;
		distortionData.bk3 = 0.0185596f;
		distortionData.bk4 = -0.0475f;
		distortionData.bk5 = 0.0568f;
		distortionData.bk6 = 0.0f;

		memcpy(pCbDistortionBegin, &distortionData, sizeof(distortionData));
	}

	//Draw to render target
	{
		//ThrowIfFailed(m_commandAllocator->Reset());

		ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), m_distortionPipelineState.Get()));

		// Set necessary state.
		m_commandList->SetGraphicsRootSignature(m_distortionSignature.Get());

		ID3D12DescriptorHeap* ppHeaps[] = { m_leftCbvSrvHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->RSSetViewports(1, &m_viewport);
		m_commandList->RSSetScissorRects(1, &m_scissorRect);

		CD3DX12_CPU_DESCRIPTOR_HANDLE intermediateRtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), FrameCount, m_rtvDescriptorSize);

		m_commandList->OMSetRenderTargets(1, &intermediateRtvHandle, FALSE, nullptr);

		// Record commands.
		m_commandList->ClearRenderTargetView(intermediateRtvHandle, IntermediateClearColor, 0, nullptr);

		// Draw the scene as normal into the intermediate buffer.
		{
			// Set up the state for a fullscreen quad.
			m_commandList->IASetVertexBuffers(0, 1, &m_quadVbv);
			m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			m_commandList->SetPipelineState(m_distortionPipelineState.Get());
			m_commandList->RSSetViewports(1, &m_intermediateViewport);
			m_commandList->SetGraphicsRootConstantBufferView(0, m_distortionConstantBuffer->GetGPUVirtualAddress());
			m_commandList->SetGraphicsRootConstantBufferView(1, m_distortionConstantBuffer->GetGPUVirtualAddress() + m_constantBufferPerSize);
			m_commandList->DrawInstanced(4, 1, 0, 0);
		}

		// intermediate render target will be used as a SRV.
		D3D12_RESOURCE_BARRIER barriers[] = {
			CD3DX12_RESOURCE_BARRIER::Transition(m_intermediateRenderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		};

		m_commandList->ResourceBarrier(_countof(barriers), barriers);
		m_commandList->Close();
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		//WriteRenderTargetToBMP("E:\\1.bmp", m_intermediateRenderTarget.Get());
	}
}
glm::mat4 CalculateWarpMatrix(glm::fquat& origPose, glm::fquat& latestPose)
//-----------------------------------------------------------------------------
{
	glm::fquat diff = origPose * glm::inverse(latestPose);
	glm::fquat invDiff = glm::inverse(diff);

	invDiff.z *= -1.0f;

	return glm::mat4_cast(invDiff);
}

// Update frame-based values.
void D3D12HelloWindow::OnUpdate()
{
	cur_quat = GetHMDQuat();
}

// Render the scene.
void D3D12HelloWindow::OnRender()
{
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(m_swapChain->Present(1, 0));

	MoveToNextFrame();
	last_quat = cur_quat;
}

void D3D12HelloWindow::OnDestroy()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForGPU();

	CloseHandle(m_fenceEvent);
}

void D3D12HelloWindow::PopulateCommandList()
{
	ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());

	// Reset command list to recording state
	ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get()));
	// Set root signature, view, and rect
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = {m_leftCbvSrvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_leftCbvSrvHeap->GetGPUDescriptorHandleForHeapStart(), 0, m_cbvSrvDescriptorSize);
	m_commandList->SetGraphicsRootDescriptorTable(0, srvHandle);

	// Set render target
	auto rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += m_frameIndex * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, TRUE, nullptr);

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// Transition to render
	{
		D3D12_RESOURCE_BARRIER resourceBarrier{ };
		resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceBarrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
		resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		m_commandList->ResourceBarrier(1, &resourceBarrier);
	}

	const FLOAT clearColor[] = { 0.0f, 0.25f, 1, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	m_commandList->SetGraphicsRootConstantBufferView(CB_CHROMATIC_INDEX, m_constantBuffer->GetGPUVirtualAddress());//root parameter slot 2 is CBChromatic.

	glm::vec3 scale{ 1.0f,1.0f,1.0f };
	glm::mat4 eyeProjectionMatrix = glm::scale(CalculateProjectionMatrix(EyeType::LeftEye), scale);

	glm::mat4 leftWarpMatrix = glm::mat4(1.0f);
	glm::mat4 leftTextureMatrix = glm::mat4(1.0f);
	glm::mat4 skewSquashMatrix = glm::mat4(1.0f);

	leftWarpMatrix = CalculateWarpMatrix(last_quat, cur_quat);
	leftTextureMatrix = leftWarpMatrix * eyeProjectionMatrix;
	m_timewarpData.textureMtx = leftTextureMatrix;
	m_timewarpData.skewSquashMatrix = glm::mat4(1);
	memcpy(m_pCbvDataBegin + m_constantBufferPerSize * 1, &m_timewarpData, sizeof(m_timewarpData));
	// Issue draw command
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetIndexBuffer(&m_indexBufferView);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->SetGraphicsRootConstantBufferView(CB_TIMEWARP_INDEX, m_constantBuffer->GetGPUVirtualAddress() + m_constantBufferPerSize);//root parameter slot 1 is CBTimewarp.
	m_commandList->DrawIndexedInstanced(m_indicies_num, 1, 0, 0, 0);

	 srvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_leftCbvSrvHeap->GetGPUDescriptorHandleForHeapStart(), 2, m_cbvSrvDescriptorSize);//t0-t1,所以偏移是2
	m_commandList->SetGraphicsRootDescriptorTable(0, srvHandle);

	eyeProjectionMatrix = glm::scale(CalculateProjectionMatrix(EyeType::RightEye), scale);
	glm::mat4 rightWarpMatrix = glm::mat4(1.0f);
	glm::mat4 rightTextureMatrix = glm::mat4(1.0f);
	//glm::mat4 skewSquashMatrix = glm::mat4(1.0f);
	rightWarpMatrix = CalculateWarpMatrix(last_quat, cur_quat);
	rightTextureMatrix = rightWarpMatrix * eyeProjectionMatrix;
	m_timewarpData.textureMtx = rightTextureMatrix;
	m_timewarpData.skewSquashMatrix = glm::mat4(1);
	memcpy(m_pCbvDataBegin + +m_constantBufferPerSize * 2, &m_timewarpData, sizeof(m_timewarpData));
	m_commandList->SetGraphicsRootConstantBufferView(CB_TIMEWARP_INDEX, m_constantBuffer->GetGPUVirtualAddress() + m_constantBufferPerSize * 2);
	m_commandList->DrawIndexedInstanced(m_indicies_num, 1, 0, 0, 0);

	// Transition to present
	{
		D3D12_RESOURCE_BARRIER resourceBarrier{ };
		resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceBarrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
		resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		m_commandList->ResourceBarrier(1, &resourceBarrier);
	}

	// Finish populating commands
	m_commandList->Close();
}

void D3D12HelloWindow::WaitForGPU()
{
	// Signal and increment the fence value.
	// Schedule a Signal command in the queue.
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

	// Wait until the fence has been processed.
	ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	m_fenceValues[m_frameIndex]++;
}


// Prepare to render the next frame.
void D3D12HelloWindow::MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

	// Update the frame index.
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}



bool D3D12HelloWindow::WriteRenderTargetToBMP(const char * path, ID3D12Resource * target)
{
	ComPtr<ID3D12GraphicsCommandList> commandList;

	// Open file first - if that fails, we can skip the remaining altogether
	FILE* pFile = nullptr;
	if (fopen_s(&pFile, path, "wb") != 0)
		return false;

	// Create a D3D11on12 context 
	ComPtr<ID3D11Device> d3d11Device;
	ComPtr<ID3D11On12Device> d3d11On12Device;
	ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
	if (S_OK != D3D11On12CreateDevice(m_device.Get(), D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0,
		reinterpret_cast<IUnknown**>(m_commandQueue.GetAddressOf()), 1, 0, &d3d11Device, &d3d11DeviceContext, nullptr))
	{
		return false;
	}
	d3d11Device.As(&d3d11On12Device);
	ComPtr<ID3D11Resource> wrappedRenderTarget;
	D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
	d3d11On12Device->CreateWrappedResource(target, &d3d11Flags,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE, IID_PPV_ARGS(&wrappedRenderTarget));

	// Read from the render target (given via renderef frame index)
	D3D12_RESOURCE_DESC rtvDesc = target->GetDesc();
	D3D11_TEXTURE2D_DESC texDesc =
	{
		(UINT)rtvDesc.Width, (UINT)rtvDesc.Height, 1, 1,
		rtvDesc.Format, {1, 0},
		D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ, 0,
	};
	std::vector<byte> data(texDesc.Width * texDesc.Height * 4); // !!Hardcoded 4=32bit depth. If required get from colour format

	ComPtr<ID3D11Texture2D> pTextureStaging;
	d3d11Device->CreateTexture2D(&texDesc, nullptr, &pTextureStaging);
	d3d11On12Device->AcquireWrappedResources(wrappedRenderTarget.GetAddressOf(), 1); // Acquire D3D12 resource for D3D11 interop
	d3d11DeviceContext->CopyResource(pTextureStaging.Get(), wrappedRenderTarget.Get());

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	d3d11DeviceContext->Map(pTextureStaging.Get(), 0, D3D11_MAP_READ, 0, &mapped);
	int rowSize = texDesc.Width * 4;
	for (unsigned int y = 0; y < texDesc.Height; ++y) // Read one row at a time and push into the vector
	{
		memcpy((void*)((byte*)&data[0] + y * rowSize), (void*)((byte*)mapped.pData + (y*mapped.RowPitch)), rowSize);
	}
	d3d11DeviceContext->Unmap(pTextureStaging.Get(), 0);
	d3d11On12Device->ReleaseWrappedResources(wrappedRenderTarget.GetAddressOf(), 1); // Release D3D12 resource

	std::vector<byte> fileBuffer;
	//compose the buffer, starting with the bitmap header and infoheader
	BITMAPFILEHEADER bmpFileHeader = // - BitmapFileHeader
	{
		0x4d42,
		0, 0, 0,
		sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER),
	};
	BITMAPINFOHEADER bmpInfoHeader = // - BitmapInfoHeader
	{
		sizeof(BITMAPINFOHEADER),
		(LONG)texDesc.Width, -(LONG)texDesc.Height,
		1,32, BI_RGB,
	};
	fileBuffer.resize(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + texDesc.Width * texDesc.Height * 4); // replace with size from format
	memcpy(&fileBuffer[0], &bmpFileHeader, sizeof(bmpFileHeader));
	memcpy(&fileBuffer[sizeof(bmpFileHeader)], &bmpInfoHeader, sizeof(bmpInfoHeader));
	// Copying one pixel at a time to rearrange the planes from RGBA to BGRA
	for (unsigned int i = 0; i < texDesc.Width * texDesc.Height; ++i)
	{
		fileBuffer[sizeof(bmpFileHeader) + sizeof(bmpInfoHeader) + i * 4] = data[i * 4 + 1];
		fileBuffer[sizeof(bmpFileHeader) + sizeof(bmpInfoHeader) + i * 4 + 1] = data[i * 4 + 2];
		fileBuffer[sizeof(bmpFileHeader) + sizeof(bmpInfoHeader) + i * 4 + 2] = data[i * 4 + 3];
		fileBuffer[sizeof(bmpFileHeader) + sizeof(bmpInfoHeader) + i * 4 + 3] = data[i * 4 + 0];
	}

	// Now the buffer is ready, simply write to file
	if (fwrite(&fileBuffer[0], fileBuffer.size(), 1, pFile) < 1)
	{
		fclose(pFile);
		return false;
	}
	fclose(pFile);
	return true;
}