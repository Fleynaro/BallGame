#pragma once

#include "ObjModelLoader.h"
#include "Main.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include "DirectXMesh.h"

using namespace DirectX;


class Window
{
public:
	static const int Width = 1280;
	static const int Height = 720;

	Window() = default;

	HRESULT Init(WNDPROC WndProc, HINSTANCE hInstance, int nCmdShow)
	{
		// Регистрация классa
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = NULL;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = "WindowClass1";
		wcex.hIconSm = NULL;
		if (!RegisterClassEx(&wcex))
			return E_FAIL;

		// Создание окна
		m_hInst = hInstance;
		RECT rc = { 0, 0, Width, Height };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
		m_hWnd = CreateWindow("WindowClass1", "Ball", WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
		if (!m_hWnd)
			return E_FAIL;

		ShowWindow(m_hWnd, nCmdShow);
		return S_OK;
	}

	void restrictCursorArea() {
		RECT rc;
		GetClientRect(m_hWnd, &rc);
		// Convert the client area to screen coordinates.
		POINT pt = { rc.left, rc.top };
		POINT pt2 = { rc.right, rc.bottom };
		ClientToScreen(m_hWnd, &pt);
		ClientToScreen(m_hWnd, &pt2);
		SetRect(&rc, pt.x, pt.y, pt2.x, pt2.y);
		ClipCursor(&rc);
	}

	std::wstring getCurrentDirectory() {
		WCHAR wpath[MAX_PATH];
		GetModuleFileNameW(m_hInst, wpath, MAX_PATH);

		std::wstring path = wpath;
		auto p = std::string(path.begin(), path.end());

		p = p.substr(0, p.find_last_of("/\\"));
		return std::wstring(p.begin(), p.end());
	}
	
	HINSTANCE	m_hInst;
	HWND		m_hWnd;
};

extern Window* g_window;


class GameDirect3D
{
public:
	// Структура вершины
	struct Vertex
	{
		XMFLOAT3 Pos;	 // Координаты точки в пространстве
		XMFLOAT3 Normal; // Нормаль вершины
		XMFLOAT2 Tex;	 // Координаты текстуры
	};

	// Структура константного буфера (совпадает со структурой в шейдере)
	struct CMatrixBuffer
	{
		XMMATRIX mWorld;		// Матрица мира
		XMMATRIX mView;			// Матрица вида
		XMMATRIX mProjection;	// Матрица проекции
	};

	// Структура константного буфера (совпадает со структурой в шейдере)
	struct CLightBuffer
	{
		XMFLOAT4 vLightDir;	// Направление света
		XMFLOAT4 vLightColor;// Цвет источника
	};


	class Shader
	{
	public:
		std::wstring m_file;
		std::string m_entryPoint;
		std::string m_version;
		
		Shader(std::wstring file, std::string entryPoint, std::string version)
			: m_file(file), m_entryPoint(entryPoint), m_version(version)
		{}

		bool compile(ID3DBlob** pBlobOut)
		{
			auto hr = CompileShaderFromFile(m_file.data(), m_entryPoint.data(), m_version.data(), pBlobOut);
			if (FAILED(hr)) {
				MessageBox(NULL, "Невозможно скомпилировать файл FX.", "Ошибка", MB_OK);
				return false;
			}
			return true;
		}
	};


	ID3D11Device* m_pd3dDevice = NULL;						// Устройство (для создания объектов)
	ID3D11DeviceContext* m_pImmediateContext = NULL;		// Контекст (устройство рисования)
	IDXGISwapChain* m_pSwapChain = NULL;					// Цепь связи (буфера с экраном)
	ID3D11RenderTargetView* m_pRenderTargetView = NULL;		// Объект вида, задний буфер
	ID3D11Texture2D* m_pDepthStencil = NULL;				// Текстура буфера глубин
	ID3D11DepthStencilView* m_pDepthStencilView = NULL;		// Объект вида, буфер глубин
	D3D_DRIVER_TYPE m_driverType = D3D_DRIVER_TYPE_NULL;

	Shader* m_stdVertexShader;
	Shader* m_stdPixelShader;

	//Матрицы
	XMMATRIX                m_World;					// Матрица мира
	XMMATRIX                m_View;						// Матрица вида
	XMMATRIX                m_Projection;				// Матрица проекции

	//Освещение
	XMFLOAT4				m_vLightDir;				// Направление света (позиция источника)
	XMFLOAT4				m_vLightColor;				// Цвет источника


	ID3D11SamplerState* m_pSamplerLinear = NULL;

	GameDirect3D()
	{
		m_stdVertexShader = new Shader(g_window->getCurrentDirectory() + L"\\shader.fx", "VS", "vs_4_0");
		m_stdPixelShader = new Shader(g_window->getCurrentDirectory() + L"\\shader.fx", "PS", "ps_4_0");

		InitMatrixes();
		setDefaultLightDir();
	}

	HRESULT InitDevice(D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0)
	{
		HRESULT hr = S_OK;

		RECT rc;
		GetClientRect(g_window->m_hWnd, &rc);
		UINT width = rc.right - rc.left;	// получаем ширину
		UINT height = rc.bottom - rc.top;	// и высоту окна

		UINT createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};
		UINT numDriverTypes = ARRAYSIZE(driverTypes);

		// Тут мы создаем список поддерживаемых версий DirectX
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		UINT numFeatureLevels = ARRAYSIZE(featureLevels);

		// Сейчас мы создадим устройства DirectX. Для начала заполним структуру,
		// которая описывает свойства переднего буфера и привязывает его к нашему окну.
		DXGI_SWAP_CHAIN_DESC sd;			// Структура, описывающая цепь связи (Swap Chain)
		ZeroMemory(&sd, sizeof(sd));	// очищаем ее
		sd.BufferCount = 1;					// у нас один буфер
		sd.BufferDesc.Width = width;		// ширина буфера
		sd.BufferDesc.Height = height;		// высота буфера
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// формат пикселя в буфере
		sd.BufferDesc.RefreshRate.Numerator = 75;			// частота обновления экрана
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// назначение буфера - задний буфер
		sd.OutputWindow = g_window->m_hWnd;							// привязываем к нашему окну
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;						// не полноэкранный режим

		for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			m_driverType = driverTypes[driverTypeIndex];
			hr = D3D11CreateDeviceAndSwapChain(NULL, m_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
				D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &featureLevel, &m_pImmediateContext);
			if (SUCCEEDED(hr))  // Если устройства созданы успешно, то выходим из цикла
				break;
		}
		if (FAILED(hr)) return hr;

		// Теперь создаем задний буфер. Обратите внимание, в SDK
		// RenderTargetOutput - это передний буфер, а RenderTargetView - задний.
		// Извлекаем описание заднего буфера
		ID3D11Texture2D* pBackBuffer = NULL;
		hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
		if (FAILED(hr)) return hr;

		// По полученному описанию создаем поверхность рисования
		hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTargetView);
		pBackBuffer->Release();
		if (FAILED(hr)) return hr;

		// Переходим к созданию буфера глубин
		// Создаем текстуру-описание буфера глубин
		D3D11_TEXTURE2D_DESC descDepth;	// Структура с параметрами
		ZeroMemory(&descDepth, sizeof(descDepth));
		descDepth.Width = width;		// ширина и
		descDepth.Height = height;		// высота текстуры
		descDepth.MipLevels = 1;		// уровень интерполяции
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// формат (размер пикселя)
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;		// вид - буфер глубин
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		// При помощи заполненной структуры-описания создаем объект текстуры
		hr = m_pd3dDevice->CreateTexture2D(&descDepth, NULL, &m_pDepthStencil);
		if (FAILED(hr)) return hr;

		// Теперь надо создать сам объект буфера глубин
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;	// Структура с параметрами
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = descDepth.Format;		// формат как в текстуре
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		// При помощи заполненной структуры-описания и текстуры создаем объект буфера глубин
		hr = m_pd3dDevice->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDepthStencilView);
		if (FAILED(hr)) return hr;

		// Подключаем объект заднего буфера и объект буфера глубин к контексту устройства
		m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		// Установки вьюпорта (масштаб и система координат). В предыдущих версиях он создавался
		// автоматически, если не был задан явно.
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		m_pImmediateContext->RSSetViewports(1, &vp);

		return S_OK;
	}

	ID3D11VertexShader*		m_pVertexShader = NULL;		// Вершинный шейдер
	ID3D11PixelShader*		m_pPixelShader = NULL;		// Пиксельный шейдер для куба
	ID3D11InputLayout*		m_pVertexLayout = NULL;		// Описание формата вершин

	ID3D11Buffer* m_cMatrixBuffer = NULL;
	ID3D11Buffer* m_cLightBuffer = NULL;

	HRESULT InitGeometry()
	{
		//для вершинного
		ID3DBlob* pBlob = NULL;
		if (!m_stdVertexShader->compile(&pBlob))
			return E_FAIL;

		auto hr = m_pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &m_pVertexShader);
		if (FAILED(hr))
		{
			pBlob->Release();
			return hr;
		}


		// Создание шаблона вершин
		if (FAILED(createInputLayout(pBlob))) {
			return E_FAIL;
		}
		pBlob->Release();

		// Подключение шаблона вершин
		m_pImmediateContext->IASetInputLayout(m_pVertexLayout);


		//для пиксельного
		pBlob = NULL;
		if (!m_stdPixelShader->compile(&pBlob))
			return E_FAIL;

		hr = m_pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &m_pPixelShader);
		if (FAILED(hr))
		{
			pBlob->Release();
			return hr;
		}
		pBlob->Release();

		//создание константного буфера для матриц
		if (FAILED(createConstBuffer(&m_cMatrixBuffer, sizeof(CMatrixBuffer)))) {
			return E_FAIL;
		}

		//создание константного буфера для света
		if (FAILED(createConstBuffer(&m_cLightBuffer, sizeof(CLightBuffer)))) {
			return E_FAIL;
		}

		//создание параметров текстур
		if (FAILED(CreateTextureSample())) {
			return E_FAIL;
		}

		return S_OK;
	}

	// Создание шаблона вершин
	HRESULT createInputLayout(ID3DBlob* pBlob)
	{
		// Определение шаблона вершин
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout);

		// Создание шаблона вершин
		auto hr = m_pd3dDevice->CreateInputLayout(layout, numElements, pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(), &m_pVertexLayout);
		if (FAILED(hr)) return hr;
		return S_OK;
	}

	// Cоздание константного буфера
	HRESULT createConstBuffer(ID3D11Buffer** buffer, UINT size)
	{
		// Создание константного буфера
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = size;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		auto hr = m_pd3dDevice->CreateBuffer(&bd, NULL, buffer);
		if (FAILED(hr)) return hr;

		return S_OK;
	}

	HRESULT InitMatrixes()
	{
		RECT rc;
		GetClientRect(g_window->m_hWnd, &rc);
		UINT width = rc.right - rc.left;	// получаем ширину
		UINT height = rc.bottom - rc.top;	// и высоту окна

		// Инициализация матрицы мира
		m_World = XMMatrixIdentity();

		// Инициализация матрицы вида
		XMVECTOR Eye = XMVectorSet(50.0f, 0.0f, 1.0f, 0.0f);	// Откуда смотрим
		XMVECTOR At = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);	// Куда смотрим
		XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);	// Направление верха
		m_View = XMMatrixLookAtLH(Eye, At, Up);

		// Инициализация матрицы проекции
		// Параметры: 1) ширина угла объектива 2) "квадратность" пикселя
		// 3) самое ближнее видимое расстояние 4) самое дальнее видимое расстояние
		m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f);

		return S_OK;
	}

	HRESULT CreateTextureSample()
	{
		// Создание сэмпла (описания) текстуры
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;	// Тип фильтрации
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;		// Задаем координаты
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		// Создаем интерфейс сэмпла текстурирования
		auto hr = m_pd3dDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear);
		if (FAILED(hr)) return hr;

		return S_OK;
	}

	void setDefaultLightDir()
	{
		m_vLightDir = XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f);
		m_vLightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	void setLight()
	{
		CLightBuffer buf;
		buf.vLightDir = m_vLightDir;
		buf.vLightColor = m_vLightColor;

		m_pImmediateContext->UpdateSubresource(m_cLightBuffer, 0, NULL, &buf, 0, 0);
	}

	void setMatrixes()
	{
		CMatrixBuffer buf;
		buf.mWorld = XMMatrixTranspose(m_World);
		buf.mView = XMMatrixTranspose(m_View);
		buf.mProjection = XMMatrixTranspose(m_Projection);

		m_pImmediateContext->UpdateSubresource(m_cMatrixBuffer, 0, NULL, &buf, 0, 0);
	}

	void setDefaultShaders()
	{
		m_pImmediateContext->VSSetShader(m_pVertexShader, NULL, 0);
		m_pImmediateContext->PSSetShader(m_pPixelShader, NULL, 0);
	}

	void setDefaultConstBuffers()
	{
		m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_cMatrixBuffer);
		m_pImmediateContext->VSSetConstantBuffers(1, 1, &m_cLightBuffer);

		m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_cMatrixBuffer);
		m_pImmediateContext->PSSetConstantBuffers(1, 1, &m_cLightBuffer);
	}

	void setDefaultToDraw()
	{
		setLight();
		setMatrixes();
		setDefaultShaders();
		setDefaultConstBuffers();
	}

	static HRESULT CompileShaderFromFile(LPCWSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
	{
		HRESULT hr = S_OK;
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
		ID3DBlob* pErrorBlob;

		hr = D3DCompileFromFile(
			szFileName,
			NULL,
			NULL,
			szEntryPoint,
			szShaderModel,
			dwShaderFlags,
			0,
			ppBlobOut,
			&pErrorBlob
		);

		if (FAILED(hr))
		{
			if (pErrorBlob != NULL)
				OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			if (pErrorBlob) pErrorBlob->Release();
			return hr;
		}
		if (pErrorBlob) pErrorBlob->Release();

		return S_OK;
	}
};

extern GameDirect3D* g_direct3d;