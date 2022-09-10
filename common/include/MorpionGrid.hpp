#pragma once
#include <array>
#include <optional>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "Packet.hpp"

namespace stw
{
	using GridType = std::array<std::array<PlayerNumber, GRID_SIZE>, GRID_SIZE>;

	class MorpionGrid final : public sf::Drawable, public sf::Transformable
	{
	public:
		MorpionGrid() : MorpionGrid(DEFAULT_GRID_DRAW_SIZE) {}
		explicit MorpionGrid(float drawSize);

		[[nodiscard]] const GridType& Grid() const;
		bool Play(sf::Vector2<char> move, PlayerNumber player);
		[[nodiscard]] std::optional<PlayerNumber> GetWinner() const;
		void UpdateSelection(sf::Vector2i mousePos);
		const std::optional<sf::Vector2i>& Selection() const;

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	private:
		GridType _grid{};
		std::optional<sf::Vector2i> _selection;
		float _drawSize;

		// Contour
		sf::RectangleShape _top;
		sf::RectangleShape _right;
		sf::RectangleShape _down;
		sf::RectangleShape _left;

		int FindSelection(int pos) const;
	};

	class MorpionShape final : public sf::Drawable, public sf::Transformable
	{
	public:
		explicit MorpionShape(PlayerNumber playerNumber);
		MorpionShape(PlayerNumber playerNumber, sf::Vector2i positionOnGrid, float drawSize = DEFAULT_GRID_DRAW_SIZE);
		void SetPosition(sf::Vector2i positionOnGrid);

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	private:
		PlayerNumber _playerNumber;
		float _drawSize;
	};
}
