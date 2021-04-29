

#include "main.h"
#include "NetGame.h"


NetGame* g_pNetGame;


int main()
{
	printf("This is a server\n\n");

	NetGame::m_instance = g_pNetGame = new NetGame;
	g_pNetGame->m_port = 50000;
	g_pNetGame->m_password = "MyGame";
	g_pNetGame->init();


	while (true)
	{
		RakSleep(30);

		//if (_kbhit())
			g_pNetGame->process();
	}

	system("pause");
}