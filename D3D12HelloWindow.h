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

#pragma once

#include "DXSample.h"
#include <vector>
#include <directxmath.h>
#include <directxcolors.h>
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/transform.hpp"
#include "Tracker.h"
// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;
using namespace DirectX;

#include <d3d11on12.h>
#pragma comment(lib, "d3d11.lib")

class D3D12HelloWindow : public DXSample
{
	enum EyeType
	{
		LeftEye,
		RightEye,
		Both
	};

	struct VertexUV
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Texcoord;
	};

	struct CBDistortion {
		float distortscale;
		float k0;
		float k1;
		float k2;
		float k3;
		float k4;
		float k5;
		float k6;

		float rk0;
		float rk1;
		float rk2;
		float rk3;
		float rk4;
		float rk5;
		float rk6;

		float gk0;
		float gk1;
		float gk2;
		float gk3;
		float gk4;
		float gk5;
		float gk6;

		float bk0;
		float bk1;
		float bk2;
		float bk3;
		float bk4;
		float bk5;
		float bk6;

		float nouse0;
		float nouse1;
		float nouse2;
	};
	struct CBChromatic {
		float GDist;
		float BDist;
		float UnUsed0;
		float UnUsed1;
	};
	struct CBTimewarp
	{
		glm::mat4 textureMtx;
		glm::mat4 skewSquashMatrix;
	};	
	struct TimewarpParams {
		glm::quat last_quat;
		glm::quat cur_quat;
		int frame_index;//如果有最新的帧输入，则为0,通过计算时间来确定
		int64_t last_frame_time;//上一个输入帧的时间
		int64_t cur_frame_time;//如果没有最新的帧输入，则和上一个一样
	};	
	
	inline static glm::quat GetHMDQuat() {
		float data[4] = { 0 };
		Tracker::SZVR_GetHMDRotate(data);
		glm::quat quat = { -data[0],data[1],-data[2],-data[3] };
		return quat;
	}
	constexpr static UINT CB_TIMEWARP_INDEX = 1;
	constexpr static UINT CB_CHROMATIC_INDEX = 2;

public:
    D3D12HelloWindow(UINT width, UINT height, std::wstring name); 
	static inline XMFLOAT3 GetGridPos(float i, float j, uint32_t _Grid_Count)
	{
		auto unitUV = 1.0f / _Grid_Count;
		return XMFLOAT3(i*unitUV * 2 - 1, 1.0f - j * unitUV * 2, 0);
	}
	static void CreateGridMeshVertexUV(UINT gridCount, UINT& vertex_num, UINT& indicies_num, std::vector<VertexUV>& grid_vertex, std::vector<DWORD>& grid_indicies) {
		vertex_num = (gridCount + 1)*(gridCount + 1);
		indicies_num = gridCount * gridCount * 6;
		grid_vertex.resize(vertex_num);
		grid_indicies.resize(indicies_num);

		auto k = 0;
		auto n = gridCount + 1;
		float du = 1.0f / (gridCount);
		float dv = 1.0f / (gridCount);
		for (uint32_t j = 0; j < gridCount + 1; j++)
		{
			for (uint32_t i = 0; i < gridCount + 1; i++)
			{
				grid_vertex[k++].Pos = GetGridPos(static_cast<float>(i), static_cast<float>(j), gridCount);
				float uv[2]{ j * du ,i * dv };
				auto index = i * n + j;
				grid_vertex[index].Texcoord.x = j * du;
				grid_vertex[index].Texcoord.y = i * dv;
			}
		}
		auto mid = gridCount / 2;
		k = 0;
		for (uint32_t j = 0; j < gridCount; j++)
		{
			for (uint32_t i = 0; i < gridCount; i++)
			{
				if ((i < mid&&j < mid) || (i >= mid && j >= mid))
				{
					grid_indicies[k++] = i + j * (gridCount + 1);
					grid_indicies[k++] = i + 1 + j * (gridCount + 1);
					grid_indicies[k++] = i + (j + 1)*(gridCount + 1);
					grid_indicies[k++] = i + (j + 1)*(gridCount + 1);
					grid_indicies[k++] = i + 1 + j * (gridCount + 1);
					grid_indicies[k++] = i + 1 + (j + 1)*(gridCount + 1);
				}
				else
				{
					grid_indicies[k++] = i + j * (gridCount + 1);
					grid_indicies[k++] = i + 1 + j * (gridCount + 1);
					grid_indicies[k++] = i + 1 + (j + 1)*(gridCount + 1);
					grid_indicies[k++] = i + j * (gridCount + 1);
					grid_indicies[k++] = i + 1 + (j + 1)*(gridCount + 1);
					grid_indicies[k++] = i + (j + 1)*(gridCount + 1);
				}
			}
		}
	}
	static std::vector<UINT8> GenerateTextureData(UINT TextureWidth, UINT TextureHeight,UINT numPitch,UINT numHeight)
	{
		const static UINT TexturePixelSize = 4;
		const UINT rowPitch = TextureWidth * TexturePixelSize;
		const UINT cellPitch = rowPitch / numPitch;        // The width of a cell in the checkboard texture.
		const UINT cellHeight = TextureWidth / numHeight;    // The height of a cell in the checkerboard texture.
		const UINT textureSize = rowPitch * TextureHeight;

		std::vector<UINT8> data(textureSize);
		UINT8* pData = &data[0];

		for (UINT n = 0; n < textureSize; n += TexturePixelSize)
		{
			UINT x = n % rowPitch;
			UINT y = n / rowPitch;
			UINT i = x / cellPitch;
			UINT j = y / cellHeight;

			if (i % 2 == j % 2)
			{
				pData[n] = 0x00;        // R
				pData[n + 1] = 0x00;    // G
				pData[n + 2] = 0x00;    // B
				pData[n + 3] = 0xff;    // A
			}
			else
			{
				pData[n] = 0xff;        // R
				pData[n + 1] = 0xff;    // G
				pData[n + 2] = 0xff;    // B
				pData[n + 3] = 0xff;    // A
			}
		}

		return data;
	}
	glm::mat4 CalculateProjectionMatrix(EyeType eye);
	void GenerateDistortionTexture();

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();


private:
    static const UINT FrameCount = 2;
	glm::quat last_quat;
	glm::quat cur_quat;

    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
	//创建两个RenderTarget和CommandAllocator,如果只用一个CommandAllocator则会造成画面撕裂
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
    ComPtr<ID3D12CommandQueue> m_commandQueue;//CommandQueue 和 CommandList只需要一个就行了
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12CommandAllocator> m_bundleAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_bundleInit;
	ComPtr<ID3D12GraphicsCommandList> m_bundleDrawer[2];
    ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;//rendertarget的存储heap
    UINT m_rtvDescriptorSize;//单个Descriptor的大小，用于获取Handle偏移地址的时候使用
    ComPtr<ID3D12PipelineState> m_pipelineState;//渲染的管线状态

	ComPtr<ID3D12DescriptorHeap> m_cbvSrvHeap;//ShaderResourceView和ConstantBufferView的存储heap
	
	UINT m_cbvSrvDescriptorSize;//单个Descriptor的大小，用于获取Handle偏移地址的时候使用

	D3D12_FEATURE_DATA_ROOT_SIGNATURE m_featureData;//获取RootSignature的功能版本
	D3D12_VIEWPORT m_viewport;//渲染的视口
	D3D12_RECT m_scissorRect;//裁剪区域
	ComPtr<ID3D12RootSignature> m_rootSignature;//相当于InputLayout的定义
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_indexBuffer;
	ComPtr<ID3D12Resource> m_leftTexture;
	ComPtr<ID3D12Resource> m_rightTexture;
	ComPtr<ID3D12Resource> m_constantBuffer;
	UINT m_vertex_num;
	UINT m_indicies_num;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;//vertexBufferView,新的IASetVertexBuffers采取设定View的方式进行装载
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;//同上
	CBTimewarp m_timewarpData;//Timewarp结构数据
	static constexpr UINT m_constantBufferPerSize = (sizeof(CBTimewarp) + 255) & ~255;//Timewarp结构对齐到256
	UINT8* m_pCbvDataBegin;//当前constantBuffer的CPU地址起点,通过Map获取值,用于memcpy更新ConstantBuffer的数据

	//intermediate resource
	ComPtr<ID3D12RootSignature> m_distortionSignature;
	ComPtr<ID3D12Resource> m_intermediateRenderTarget;
	ComPtr<ID3D12PipelineState> m_distortionPipelineState;
	ComPtr<ID3D12Resource> m_quadVertexBuffer;
	ComPtr<ID3D12Resource> m_distortionConstantBuffer;
	ComPtr<ID3D12Resource> m_distortionTexture;
	D3D12_VERTEX_BUFFER_VIEW m_quadVbv;
	D3D12_VIEWPORT m_intermediateViewport;
	static constexpr UINT m_distortionConstantBufferPerSize = (sizeof(CBDistortion) + 255) & ~255;

    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;	//Fence
	UINT64 m_fenceValues[FrameCount];//创建两个FenceValue

    void LoadPipeline();
    void PopulateCommandList();
    void WaitForGPU();
	void MoveToNextFrame();
	bool WriteRenderTargetToBMP(const char * path, ID3D12Resource* target);
};
