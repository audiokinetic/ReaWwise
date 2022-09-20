#pragma once

#include "StubContext.h"
#include "UI/MainWindow.h"

namespace AK::WwiseTransfer
{
	class StandaloneWindow : public MainWindow
	{
	public:
		StandaloneWindow();
		~StandaloneWindow() override;

		void userTriedToCloseWindow() override;

	private:
		StubContext stubContext;
	};
} // namespace AK::WwiseTransfer
