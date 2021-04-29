#pragma once

#include "../Items/IWindow.h"


namespace GUI::Window
{

	class MainControl : public IWindow
	{
	public:
		ChildContainer* m_console = nullptr;

		MainControl()
			: IWindow("Main control window")
		{
			setWidth(500);
			setHeight(300);

			getMainContainer()
				.ftext("Press F1 to hide/show Main control window.").separator()
				.addItem(
					new Elements::Button::ButtonStd(
						"Exit from game",
						new Events::EventUI(
							EVENT_LAMBDA(info) {
								PostQuitMessage(0);
							}
						)
					)
				)
				.newLine()
				.beginChild("##bottom", &m_console)
					.setBorder(true)
					.setWidth(0)
					.setHeight(100)
				.end();

			m_console->setScrollbarToBottom();
		}
	};
};