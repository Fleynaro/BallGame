
#include "Game.h"

Game* g_game;
Ball* ball;

#define KEY_PRESSED(key) ( 1 << 15 ) & GetAsyncKeyState(key)




class PhysicControl : public IWindow
{
public:
	PhysicControl()
		: IWindow("Physic control window")
	{
		setWidth(400);
		setHeight(200);

		getMainContainer()
			.addItem(
				new GUI::Elements::Input::Float(
					"Enter speed",
					new GUI::Events::EventUI(
						EVENT_LAMBDA(info) {
							auto sender = (GUI::Elements::Input::Float*)info->getSender();
							float speed = sender->getInputValue();

							g_game->m_player->m_speed = speed;
						}
					)
				)
			);
	}
};


class ServerControl : public IWindow
{
public:
	GUI::Elements::Input::Text* nickname;
	GUI::Elements::Input::Text* ip;

	ServerControl()
		: IWindow("Connect to the server")
	{
		setWidth(400);
		setHeight(200);
		
		getMainContainer()
			.text("Here you can connect to the server to play online.")
			.addItem(
				new GUI::Elements::Input::Text(
					"Your nickname",
					20
				),
				(Item**)& nickname
			)
			.addItem(
				new GUI::Elements::Input::Text(
					"Enter ip of server (port = 50000)",
					30
				),
				(Item**)& ip
			)
			.addItem(
				new GUI::Elements::Button::ButtonStd(
					"Connect",
					new GUI::Events::EventUI(
						EVENT_LAMBDA(info) {
							g_game->m_netGame->connectToServer(ip->m_inputValue, "50000");
						}
					)
				)
			);

		nickname->m_inputValue = "Fleynaro";
		ip->m_inputValue = "localhost";
	}
};





#include "TDs.h"

namespace TL
{
	class BallTest : public Simulation
	{
	public:
		BallTest(::Ball* ball)
			: m_ball(ball)
		{}

		bool process() override
		{
			::TD::GeneralState* gState = m_tl_generalState.getAt(m_currentFrame).get();
			if (gState == nullptr)
				return false;
				
			if (m_currentFrame % 1 == 0) {
				m_ball->setPos(gState->m_data.position);
				m_ball->setRot(gState->m_data.rotation);
				m_ball->m_rigidBody->setLinearVelocity(gState->m_data.velocity);
				m_ball->m_rigidBody->setAngularVelocity(gState->m_data.angularVelocity);
			}

			::TD::Shoot* shootEvent = m_tl_shoot.getAt(m_currentFrame);
			if (shootEvent != nullptr) {
				m_ball->shootAt(shootEvent->m_data.shootDir, shootEvent->m_data.size, shootEvent->m_data.mass, shootEvent->m_data.speed);
			}

			m_counter++;
			return true;
		}
			
		TL::State<::TD::GeneralState> m_tl_generalState;
		TL::Event<::TD::Shoot> m_tl_shoot;
	private:
		::Ball* m_ball;
		uint32_t m_counter = 0;
	};

	class LocalBall : public Optimization
	{
	public:
		LocalBall() {}

		TL::State<::TD::GeneralState> m_tl_generalState;
	};
};

TL::BallTest* g_tl_ball;
TL::LocalBall* g_tl_myBall;



extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (UI::WinManager::isVisible()) {
		ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
	}

	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYUP:
	{
		if (wParam == VK_SPACE) {
			static ULONGLONG prevTime;
			ULONGLONG curTime = GetTickCount64();
			if (curTime - prevTime > 1000) {
				ball->m_rigidBody->setLinearVelocity(
					ball->m_rigidBody->getLinearVelocity() + btVector3(0, 7, 0)
				);

				prevTime = curTime;
			}
		}

		if (wParam == VK_F1)
		{
			static bool state = true;
			UI::WinManager::setVisibleForAll(state ^= 1);
		}
		break;
	}

	case WM_LBUTTONUP:
	{
		static ULONGLONG prevTime;
		ULONGLONG curTime = GetTickCount64();
		if (curTime - prevTime > 200) {
			auto shootEvent = new ::TD::Shoot;
			shootEvent->m_data.shootDir = g_game->m_player->getLookAtDir();
			shootEvent->m_data.size = 0.3;
			shootEvent->m_data.mass = 0.4;
			shootEvent->m_data.speed = 60;
			g_tl_ball->m_tl_shoot.setData(getTimestamp(), shootEvent);
			
			g_game->m_netGame->send(ID_PLAYER_STATE, shootEvent, HIGH_PRIORITY, RELIABLE_ORDERED, 0);

			ball->shootAt(g_game->m_player->getLookAtDir(), shootEvent->m_data.size, shootEvent->m_data.mass, shootEvent->m_data.speed);
			prevTime = curTime;
		}
		break;
	}

	case WM_RBUTTONUP:
	{
		static ULONGLONG prevTime;
		ULONGLONG curTime = GetTickCount64();
		if (curTime - prevTime > 3000) {
			auto shootEvent = new ::TD::Shoot;
			shootEvent->m_data.shootDir = g_game->m_player->getLookAtDir();
			shootEvent->m_data.size = 0.6;
			shootEvent->m_data.mass = 1.5;
			shootEvent->m_data.speed = 50;
			g_tl_ball->m_tl_shoot.setData(getTimestamp(), shootEvent);

			g_game->m_netGame->send(ID_PLAYER_STATE, shootEvent, HIGH_PRIORITY, RELIABLE_ORDERED, 0);

			ball->shootAt(g_game->m_player->getLookAtDir(), shootEvent->m_data.size, shootEvent->m_data.mass, shootEvent->m_data.speed);
			prevTime = curTime;
		}
		break;
	}

	case WM_SETFOCUS:
	{
		if (UI::WinManager::isVisible())
			break;
		g_window->restrictCursorArea();
		break;
	}

	case WM_SETCURSOR:
		if (UI::WinManager::isVisible())
			break;

		if (LOWORD(lParam) == HTCLIENT) {
			SetCursor(NULL);
		}
		break;

	case WM_MOUSEMOVE:
	{
		if (UI::WinManager::isVisible())
			break;

		const float betaMax = 89.0;
		const float sensetivity = 0.7;

		short mouseX = LOWORD(lParam);
		short mouseY = HIWORD(lParam);

		//printf("x,y = %i, %i\n", mouseX, mouseY);

		Camera* cam = g_game->m_player->m_camera;

		if (mouseX > 10) {
			cam->m_alpha += XMConvertToRadians(sensetivity * 1.3);
		}
		else if (mouseX < 10) {
			cam->m_alpha -= XMConvertToRadians(sensetivity * 1.3);
		}

		if (mouseY > 10) {
			cam->m_beta -= XMConvertToRadians(sensetivity);
		}
		else if (mouseY < 10) {
			cam->m_beta += XMConvertToRadians(sensetivity);
		}
		
		if (cam->m_beta < -XMConvertToRadians(betaMax)) {
			cam->m_beta = -XMConvertToRadians(betaMax);
		} else if (cam->m_beta > XMConvertToRadians(betaMax)) {
			cam->m_beta = XMConvertToRadians(betaMax);
		}

		RECT rc;
		GetClientRect(hWnd, &rc);
		MapWindowPoints(hWnd, GetParent(hWnd), (LPPOINT)& rc, 2);
		SetCursorPos(rc.left + 10, rc.top + 10);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

Ball* ball2;

//test map
void buildDefMap()
{
	//ball
	ball = new Ball;
	ball->setPos(btVector3(5, -6, 5));
	ball->m_rigidBody->setFriction(10.9);
	//ball->m_rigidBody->setRollingFriction(0.9);
	ball->m_rigidBody->setRestitution(1.0);
	ball->m_rigidBody->setAngularVelocity(btVector3(0, 0, 0));
	ball->m_rigidBody->setLinearVelocity(btVector3(0, 3, 0));
	//ball->m_rigidBody->setGravity(btVector3(20, 20, 0));
	ball->m_rigidBody->setDamping(0.5, 0.5);
	g_game->m_world->addEntity(ball);
	g_game->m_player->m_camera->setEntityToLookAt(ball);
	g_game->m_player->setEntityToControl(ball);

	//g_game->m_world->addEntity(new Ball);


	ball2 = new Ball;
	ball2->setPos(btVector3(5, 5, 5));
	ball2->m_rigidBody->setFriction(10.9);
	ball2->m_rigidBody->setRestitution(1.0);
	ball2->m_rigidBody->setDamping(0.5, 0.5);
	ball2->m_rigidBody->setAngularVelocity(btVector3(0, 1, 0));
	ball2->m_rigidBody->setLinearVelocity(btVector3(0, 1, 0));
	//ball2->m_rigidBody->setGravity(btVector3(0,0,0));
	//ball2->m_rigidBody->setActivationState(0);
	g_game->m_world->addEntity(ball2);

	
	//tramp
	auto base = new Base(btVector3(1, 1, 5), 1);
	base->setPos(btVector3(0, -4, 1));
	base->m_rigidBody->setFriction(0.6);

	btQuaternion q;
	q.setEuler(0, 45, 0);
	base->setRot(q);

	base->m_rigidBody->setAngularVelocity(btVector3(0, -1, 0));

	g_game->m_world->addEntity(base);

	//down base
	auto mainBase = new Base(btVector3(20, 1, 20), 0);
	mainBase->setPos(btVector3(0, -10, 0));
	mainBase->m_rigidBody->setFriction(1.6);
	//mainBase->m_rigidBody->setRollingFriction(0.01);
	mainBase->m_rigidBody->setRestitution(0.4);
	g_game->m_world->addEntity(mainBase);
}

int main(int argc, char** argv)
{
	int nCmdShow = SW_SHOW;
	HINSTANCE hInstance = GetModuleHandle(NULL);

	g_window = new Window;
	if (FAILED(g_window->Init(WndProc, hInstance, nCmdShow)))
		return 1;


	g_direct3d = new GameDirect3D;
	if (FAILED(g_direct3d->InitDevice()))
		return 1;
	if (FAILED(g_direct3d->InitGeometry()))
		return 1;

	g_game = new Game;
	UI::WinManager::addWindow(new PhysicControl);
	UI::WinManager::addWindow(new ServerControl);

	buildDefMap();

	g_tl_ball = new TL::BallTest(ball2);
	g_tl_myBall = new TL::LocalBall();

	// Главный цикл сообщений
	MSG msg = { 0 };
	std::size_t counter = 0;
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			static ULONGLONG prevTime;
			ULONGLONG curTime = GetTickCount64();
			if (curTime - prevTime > 1000 / GAME_FPS) {
				if (g_game->m_netGame->m_state != NetGame::GameState::Not) {
					g_game->m_netGame->updateNetwork();
				}

				g_game->render();
				g_game->m_world->getPhysic().stepSimulation(1.f / float(GAME_FPS), 10);
				prevTime = curTime;

				float speed = g_game->m_player->m_speed;
				float acceler = 50.0 / 60;


				const int sec = 5;
				static bool repeat = false;
				static TimeStamp tm = 0;
				if (repeat)
				{
					static bool isFirst = true;
					if (isFirst) {
						g_tl_ball->setStartFrame(tm + 1);
						isFirst = false;
					}
					const int k = 1;
					if (counter % k == 0) {
						if (g_tl_ball->process()) {

							for(int i = 0; i < k; i ++)
								g_tl_ball->nextFrame();
							
						}
						else {
							g_tl_ball->setStartFrame(tm + 1);
						}
					}
				}

				
				if (true) //check contact with the ground https://gamedev.stackexchange.com/questions/58012/detect-when-a-bullet-rigidbody-is-on-ground
				{
					if (KEY_PRESSED(VK_SHIFT)) {
						speed *= 1.7;
					} else if (KEY_PRESSED(VK_CONTROL)) {
						speed *= 0.7;
					}
					if (KEY_PRESSED(VK_UP) || KEY_PRESSED(0x57)
						|| KEY_PRESSED(VK_DOWN) || KEY_PRESSED(0x53)
						|| KEY_PRESSED(VK_LEFT) || KEY_PRESSED(0x41)
						|| KEY_PRESSED(VK_RIGHT) || KEY_PRESSED(0x44)
						)
					{
						float alpha = g_game->m_player->m_camera->m_alpha;
						float direction = 1;
						if (KEY_PRESSED(VK_DOWN) || KEY_PRESSED(0x53)) {
							direction *= -1;
						}
						else if (KEY_PRESSED(VK_RIGHT) || KEY_PRESSED(0x44)) {
							alpha += XM_PI / 2;
						}
						else if (KEY_PRESSED(VK_LEFT) || KEY_PRESSED(0x41)) {
							alpha -= XM_PI / 2;
						}

						XMVECTOR xmv_dir = XMVector3Transform(
							XMVectorSet(0.0, 0.0, 1.0, 1.0),
							XMMatrixRotationY(alpha)
						);

						auto v_dir = -MathConvert::toBtVector(xmv_dir) * direction;
						auto velocity = ball->m_rigidBody->getLinearVelocity().length();

						if (ball->m_rigidBody->getLinearVelocity().dot(velocity * v_dir) < speed * speed) {
							ball->m_rigidBody->setLinearVelocity(
								ball->m_rigidBody->getLinearVelocity() + v_dir * acceler
							);
						}
					}
				}

				if (!repeat && counter % 8 == 0 && false) {
					auto gState = new TD::GeneralState;
					gState->setTimeline(&g_tl_myBall->m_tl_generalState);
					gState->m_data.position = ball->getPos();
					gState->m_data.rotation = ball->getRot();
					gState->m_data.velocity = ball->m_rigidBody->getLinearVelocity();
					gState->m_data.angularVelocity = ball->m_rigidBody->getAngularVelocity();
					
					g_game->m_netGame->send(ID_PLAYER_STATE, gState, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0);

					g_tl_ball->m_tl_generalState.setData(getTimestamp(), gState);
					g_tl_myBall->m_tl_generalState.setData(getTimestamp(), gState);
					if (tm == 0) {
						tm = getTimestamp();
					}

					if (false && counter / GAME_FPS >= sec) {
						repeat = true;
					}
				}

				//g_game->m_player->m_camera->m_alpha += XM_PI * 2 / 180.0;
				if (counter % 60 == 0) {
					//g_window->restrictCursorArea();
				}

				counter++;
			}
		}
	}

	g_game->m_netGame->m_rakClient->Shutdown(300);
	SLNet::RakPeerInterface::DestroyInstance(g_game->m_netGame->m_rakClient);

	return 0;
}