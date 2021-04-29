#pragma once


#include "World.h"
#include "Camera.h"
#include "UI.h"
#include "Server/NetGame.h"


class LocalPlayer
{
public:
	Camera* m_camera = nullptr;
	IBall* m_entity = nullptr;
	float m_speed = 8.f;

	LocalPlayer()
	{}

	void setEntityToControl(IBall* entity) {
		m_entity = entity;
	}

	void setCamera(Camera* camera) {
		m_camera = camera;
	}

	void update()
	{
		m_camera->updateViewMatrix();
	}

	btVector3 getLookAtDir()
	{
		XMVECTOR xmv_dir = XMVector3Transform(
			XMVectorSet(0.0, 0.0, 1.0, 1.0),
			XMMatrixRotationX(m_camera->m_beta) * XMMatrixRotationY(m_camera->m_alpha)
		);
		return -MathConvert::toBtVector(xmv_dir);
	}
};


class Game
{
public:
	World* m_world;
	LocalPlayer* m_player;
	UI* m_ui;
	NetGame* m_netGame;

	Game()
	{
		World::m_world = m_world = new World;
		m_player = new LocalPlayer;
		m_ui = new UI;
		NetGame::m_instance = m_netGame = new NetGame;
		m_netGame->m_password = "MyGame";
		m_netGame->init();
		UI::WinManager::registerWindows();

		m_player->setCamera(new Camera);
	}

	void render()
	{
		static bool m_once = false;
		if (m_once) return;

		m_player->update();

		// Очищаем задний буфер в синий цвет
		float ClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
		g_direct3d->m_pImmediateContext->ClearRenderTargetView(g_direct3d->m_pRenderTargetView, ClearColor);
		// Очищаем буфер глубин до едицины (максимальная глубина)
		g_direct3d->m_pImmediateContext->ClearDepthStencilView(g_direct3d->m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		for (auto entity : m_world->getPool())
		{
			entity->draw();
		}

		m_ui->render();

		g_direct3d->m_pSwapChain->Present(0, 0);
		//m_once = true;
	}
};

extern Game* g_game;