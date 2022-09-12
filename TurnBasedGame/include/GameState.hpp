#pragma once

namespace stw
{
	enum class GameState
	{
		ConnectingToServer,
		WaitingForPlayerNumber,
		Playing,
		WaitingForMove,
		Win,
		Lose
	};
}

