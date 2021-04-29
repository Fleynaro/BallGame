#pragma once

#include "DirectX.h"
#include <WICTextureLoader.h>

class Model
{
public:
	Model()
	{

	}

	HRESULT loadFromFileObj(std::wstring file)
	{
		WaveFrontReader<WORD> obj;
		obj.Load(file.data());

		// Структура, описывающая создаваемый буфер
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(GameDirect3D::Vertex) * obj.vertices.size();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		// Структура, содержащая данные буфера
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));	// очищаем ее
		InitData.pSysMem = obj.vertices.data();				// указатель на наши 8 вершин
		auto hr = g_direct3d->m_pd3dDevice->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
		if (FAILED(hr)) return hr;


		// Структура, описывающая создаваемый буфер
		bd.Usage = D3D11_USAGE_DEFAULT;
		m_indicesDrawCount = bd.ByteWidth = sizeof(WORD) * obj.indices.size();
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;

		InitData.pSysMem = obj.indices.data();
		hr = g_direct3d->m_pd3dDevice->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
		if (FAILED(hr)) return hr;

		return S_OK;
	}

	void setBuffers() {
		// Установка буфера вершин
		UINT stride = sizeof(GameDirect3D::Vertex);
		UINT offset = 0;
		g_direct3d->m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

		// Установка буфера индексов
		g_direct3d->m_pImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

		// Установка способа отрисовки вершин в буфере
		g_direct3d->m_pImmediateContext->IASetPrimitiveTopology(m_topology);
	}

	void draw()
	{
		setBuffers();
		g_direct3d->m_pImmediateContext->DrawIndexed(m_indicesDrawCount, 0, 0);
	}
private:
	D3D11_PRIMITIVE_TOPOLOGY m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	ID3D11Buffer* m_pIndexBuffer = NULL;
	ID3D11Buffer* m_pVertexBuffer = NULL;
	int m_indicesDrawCount = 0;
};


class Texture
{
public:
	Texture()
	{}

	HRESULT loadFromImageFile(std::wstring file)
	{
		// Загрузка текстуры из файла
		auto hr = CreateWICTextureFromFile(g_direct3d->m_pd3dDevice, file.data(), nullptr, &m_pTextureRV);
		if (FAILED(hr)) return hr;

		return S_OK;
	}

	void draw()
	{
		g_direct3d->m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTextureRV);
		g_direct3d->m_pImmediateContext->PSSetSamplers(0, 1, &g_direct3d->m_pSamplerLinear);
	}
private:
	ID3D11ShaderResourceView* m_pTextureRV = NULL;
};



class Models
{
public:
	static Model* Load(std::wstring file)
	{
		Model* model = new Model;
		if (!FAILED(model->loadFromFileObj(file))) {
			return model;
		}
		delete model;
		return nullptr;
	}

	inline static std::map<std::wstring, std::pair<Model*, int>> loadedModel;

	static Model* LoadFromModelDir(std::wstring file)
	{
		if (loadedModel.find(file) != loadedModel.end()) {
			if (loadedModel[file].second != 0) {
				loadedModel[file].second++;
				return loadedModel[file].first;
			}
		}

		loadedModel[file].second = 1;
		return loadedModel[file].first = Load(g_window->getCurrentDirectory() + L"\\models\\" + file);
	}

	static void UnloadModel(Model* model)
	{
		for (auto& it : loadedModel) {
			if (it.second.first == model) {
				if (--it.second.second == 0) {
					delete model;
				}
			}
		}
	}

	static Model* LoadSphereModel()
	{
		return LoadFromModelDir(L"sphere.obj");
	}

	static Model* LoadCubeModel()
	{
		return LoadFromModelDir(L"cube.obj");
	}
};



class Textures
{
public:
	static Texture* Load(std::wstring file)
	{
		Texture* tex = new Texture;
		if (!FAILED(tex->loadFromImageFile(file))) {
			return tex;
		}
		delete tex;
		return nullptr;
	}

	inline static std::map<std::wstring, std::pair<Texture*, int>> loadedTex;

	static Texture* LoadFromModelDir(std::wstring file)
	{
		if (loadedTex.find(file) != loadedTex.end()) {
			loadedTex[file].second++;
			return loadedTex[file].first;
		}

		loadedTex[file].second = 1;
		return loadedTex[file].first = Load(g_window->getCurrentDirectory() + L"\\textures\\" + file);
	}

	static void UnloadTexture(Texture* tex)
	{
		for (auto& it : loadedTex) {
			if (it.second.first == tex) {
				if (--it.second.second == 0) {
					delete tex;
				}
			}
		}
	}

	static Texture* LoadBallDef()
	{
		return LoadFromModelDir(L"ball_def.png");
	}

	static Texture* LoadBallEvgen()
	{
		return LoadFromModelDir(L"ball_evgen.png");
	}

	static Texture* LoadBallFleynaro()
	{
		return LoadFromModelDir(L"ball_fleynaro.png");
	}

	static Texture* LoadBallStrike()
	{
		return LoadFromModelDir(L"ball_strike.png");
	}

	static Texture* LoadSand()
	{
		return LoadFromModelDir(L"sand.jpg");
	}
};