#pragma once

namespace stw
{
	enum class GameState
	{
		ConnectingToServer,
		WaitingForPlayerNumber,
		WaitingForP2Connexion,
		Playing,
		WaitingForMove,
		Win,
		Lose
	};
}

