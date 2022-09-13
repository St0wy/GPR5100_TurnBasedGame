#include "MorpionGrid.hpp"

#include <spdlog/spdlog.h>
#include <SFML/Graphics/CircleShape.hpp>

stw::MorpionGrid::MorpionGrid(const float drawSize) : _drawSize(drawSize)
{
	_top = sf::RectangleShape(sf::Vector2f(_drawSize, LINE_THICKNESS));
	_down = sf::RectangleShape(sf::Vector2f(_drawSize, LINE_THICKNESS));
	_right = sf::RectangleShape(sf::Vector2f(LINE_THICKNESS, _drawSize));
	_left = sf::RectangleShape(sf::Vector2f(LINE_THICKNESS, _drawSize));

	_top.setPosition(0, 0);
	_down.setPosition(0, _drawSize);
	_right.setPosition(_drawSize, 0);
	_left.setPosition(0, 0);
}

const stw::GridType& stw::MorpionGrid::Grid() const
{
	return _grid;
}

bool stw::MorpionGrid::Play(const sf::Vector2i move, const PlayerNumber player)
{
	PlayerNumber& cell = _grid[move.x][move.y];
	if (cell == PlayerNumber::None)
	{
		cell = player;
		return true;
	}

	return false;
}

std::optional<stw::PlayerNumber> stw::MorpionGrid::GetWinner() const
{
	// TODO

	return{};
}

void stw::MorpionGrid::UpdateSelection(const sf::Vector2i mousePos)
{
	const sf::Vector2f pos = getPosition();
	const int x = static_cast<int>(pos.x);
	const int y = static_cast<int>(pos.y);
	const sf::Vector2i intGridPosition(x, y);
	const sf::Vector2i relativeMousePos = mousePos - intGridPosition;

	const int xSelection = FindSelection(relativeMousePos.x);
	const int ySelection = FindSelection(relativeMousePos.y);

	if (xSelection < 0 || ySelection < 0)
	{
		_selection = {};
		return;
	}

	_selection = sf::Vector2i(xSelection, ySelection);
}

void stw::MorpionGrid::ResetSelection()
{
	_selection = {};
}

const std::optional<sf::Vector2i>& stw::MorpionGrid::Selection() const
{
	return _selection;
}

void stw::MorpionGrid::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	// Apply this object's transform (position, rotation, etc.)
	states.transform *= getTransform();

	// Draw contour
	sf::RectangleShape horizontalLine(sf::Vector2f(_drawSize + LINE_THICKNESS, LINE_THICKNESS));
	sf::RectangleShape verticalLine(sf::Vector2f(LINE_THICKNESS, _drawSize));
	const float oneThird = _drawSize / 3.0f;
	const float twoThird = oneThird * 2.0f;

	horizontalLine.setPosition(0, 0);
	target.draw(horizontalLine, states);
	horizontalLine.setPosition(0, oneThird);
	target.draw(horizontalLine, states);
	horizontalLine.setPosition(0, twoThird);
	target.draw(horizontalLine, states);
	horizontalLine.setPosition(0, _drawSize);
	target.draw(horizontalLine, states);

	verticalLine.setPosition(_drawSize, 0);
	target.draw(verticalLine, states);
	verticalLine.setPosition(oneThird, 0);
	target.draw(verticalLine, states);
	verticalLine.setPosition(twoThird, 0);
	target.draw(verticalLine, states);
	verticalLine.setPosition(0, 0);
	target.draw(verticalLine, states);

	for (std::size_t i = 0; i < GRID_SIZE; ++i)
	{
		for (std::size_t j = 0; j < GRID_SIZE; ++j)
		{
			MorpionShape morpionShape(_grid[i][j], { static_cast<int>(i), static_cast<int>(j) }, _drawSize);
			target.draw(morpionShape, states);
		}
	}

	if (_selection.has_value())
	{
		sf::RectangleShape selectionRectangle(sf::Vector2f(oneThird, oneThird));
		selectionRectangle.setFillColor(sf::Color(255, 255, 255, 100));

		sf::Vector2i selection = _selection.value();
		auto selectionPos = sf::Vector2f(static_cast<float>(selection.x), static_cast<float>(selection.y));
		selectionPos *= oneThird;

		selectionRectangle.setPosition(selectionPos);

		target.draw(selectionRectangle, states);
	}
}

int stw::MorpionGrid::FindSelection(const int pos) const
{
	static const int INT_SIZE = static_cast<int>(_drawSize);
	static const int CELL_SIZE = INT_SIZE / static_cast<int>(GRID_SIZE);

	if (pos < 0 || pos > INT_SIZE)
	{
		return -1;
	}

	int previousCell = 0;
	for (std::size_t i = 0; i < GRID_SIZE; ++i)
	{
		const int currentCell = (static_cast<int>(i) + 1) * CELL_SIZE;
		if (pos > previousCell && pos <= currentCell)
		{
			return static_cast<int>(i);
		}
		previousCell = currentCell;
	}

	return -1;
}

stw::MorpionShape::MorpionShape(const PlayerNumber playerNumber)
	: MorpionShape(playerNumber, sf::Vector2i(0, 0))
{
}

stw::MorpionShape::MorpionShape(const PlayerNumber playerNumber, const sf::Vector2i positionOnGrid, const float drawSize)
	: _playerNumber(playerNumber), _drawSize(drawSize)
{
	SetPosition(positionOnGrid);
}

void stw::MorpionShape::SetPosition(const sf::Vector2i positionOnGrid)
{
	const auto x = static_cast<float>(positionOnGrid.x);
	const auto y = static_cast<float>(positionOnGrid.y);
	sf::Vector2f pos(x, y);
	pos *= _drawSize / GRID_SIZE;
	setPosition(pos);
}

void stw::MorpionShape::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	// Apply this object's transform (position, rotation, etc.)
	states.transform *= getTransform();

	const float oneThird = _drawSize / 3.0f;
	const float length = std::sqrt(oneThird * oneThird + oneThird * oneThird);

	sf::RectangleShape first({ length, LINE_THICKNESS });
	first.setRotation(45.0f);
	sf::RectangleShape second({ length, LINE_THICKNESS });
	second.setRotation(-45.0f);
	second.setPosition({ 0, oneThird });

	constexpr float circleOffset = 10.0f;
	sf::CircleShape shape((oneThird / 2.0f) - circleOffset);
	shape.setOutlineThickness(LINE_THICKNESS);
	shape.setFillColor(sf::Color(0, 0, 0));
	shape.setPosition(circleOffset, circleOffset);

	switch (_playerNumber)
	{
	case PlayerNumber::None:
		return;
	case PlayerNumber::P1:
		target.draw(first, states);
		target.draw(second, states);
		break;
	case PlayerNumber::P2:
		target.draw(shape, states);
		break;
	}
}
