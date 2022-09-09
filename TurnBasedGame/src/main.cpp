#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>

int main()
{
    sf::RenderWindow window(sf::VideoMode(640 * 2, 480 * 2), "Client");
	if (!ImGui::SFML::Init(window))
	{
		std::cout << "Error loading ImGui" << std::endl;
		return EXIT_FAILURE;
	}

	// Set window scale for HDPI screens
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 2;

	sf::Clock deltaClock;
	while (window.isOpen()) {
		sf::Event event{};
		while (window.pollEvent(event)) {
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

		ImGui::ShowDemoWindow();

		window.clear();
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
    return EXIT_SUCCESS;
}