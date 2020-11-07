#pragma once

class ScreenGame : public Screen
{
public:

	bool isServer = true;
	int serverPort;
	const char *serverAddress = "127.0.0.1";
	const char *playerName = "player";
	//std::vector<float> VecColor;

	//ImVec4 playerColor = ImVec4(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);

private:

	void enable() override;

	void update() override;

	void gui() override;

	void disable() override;
};

