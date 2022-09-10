#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>
#include <spdlog/spdlog.h>

#include "MorpionGrid.hpp"


int main()
{
	spdlog::set_level(spdlog::level::debug);

	sf::RenderWindow window(sf::VideoMode(640 * 2, 480 * 2), "Online Morpion");
	if (!ImGui::SFML::Init(window))
	{
		std::cout << "Error loading ImGui" << std::endl;
		return EXIT_FAILURE;
	}

	// Set window scale for HDPI screens
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 2;

	// Session var
	stw::MorpionGrid grid{};
	grid.setPosition(30.0f, 30.0f);
	bool oldMousePressed = false;

	sf::Clock deltaClock;
	while (window.isOpen())
	{
		sf::Event event{};
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(window, event);

			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
			else if (event.type == sf::Event::Resized)
			{
				sf::Vector2f size = { static_cast<float>(event.size.width), static_cast<float>(event.size.height) };
				sf::Vector2f center = size / 2.0f;
				window.setView(sf::View(center, size));
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		grid.UpdateSelection(sf::Mouse::getPosition(window));

		bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
		if (mousePressed && !oldMousePressed)
		{
			std::optional<sf::Vector2i> selection = grid.Selection();
			if (selection.has_value())
			{
				sf::Vector2i val = selection.value();
				spdlog::debug("x:{};y:{}", val.x, val.y);
			}

		}
		oldMousePressed = mousePressed;

		window.clear();

		ImGui::SFML::Render(window);
		window.draw(grid);

		window.display();
	}

	ImGui::SFML::Shutdown();
	return EXIT_SUCCESS;
}