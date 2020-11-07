#include "Networks.h"
#include "ScreenMainMenu.h"

void ScreenMainMenu::enable()
{
	/*LOG("Example Info log entry...");
	DLOG("Example Debug log entry...");
	WLOG("Example Warning log entry...");
	ELOG("Example Error log entry...");*/

	LOG("starting Death Statr engines...");
	//DLOG("Awaiting darth sidious command...");
	//WLOG("WARNING Rebels in the area...");
	//ELOG("ERROR 404 JEDI not found...");
}

void ScreenMainMenu::gui()
{
	ImGui::ShowDemoWindow();

	ImGui::Begin("Main Menu");
	
	Texture *banner = App->modResources->banner;
	ImVec2 bannerSize(400.0f, 400.0f * banner->height / banner->width);
	ImGui::Image(banner->shaderResource, bannerSize);

	ImGui::Spacing();


	ImGui::Text("Server");

	static int localServerPort = 8888;
	ImGui::InputInt("Server port", &localServerPort);

	if (ImGui::Button("Start server"))
	{
		App->modScreen->screenGame->isServer = true;
		App->modScreen->screenGame->serverPort = localServerPort;
		App->modScreen->swapScreensWithTransition(this, App->modScreen->screenGame);
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Client");

	static char serverAddressStr[64] = "127.0.0.1";
	ImGui::InputText("Server address", serverAddressStr, sizeof(serverAddressStr));

	static int remoteServerPort = 8888;
	ImGui::InputInt("Server port", &remoteServerPort);

	static char playerNameStr[64] = "Darth Vader";
	ImGui::InputText("Player name", playerNameStr, sizeof(playerNameStr));
	
	/*static ImVec4 color = ImVec4(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
	ImGui::Text("Choose Color Font"); ImGui::SameLine(); 
	ImGui::ColorEdit4("MyColor##3", (float*)&color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel );
*/
	if (ImGui::Button("Connect to Death Star server"))
	{
		App->modScreen->screenGame->isServer = false;
		App->modScreen->screenGame->serverPort = remoteServerPort;
		App->modScreen->screenGame->serverAddress = serverAddressStr;
		App->modScreen->screenGame->playerName = playerNameStr;
		//App->modScreen->screenGame->VecColor = std::vector<float>{color.x,color.y,color.z,color.w };
		App->modScreen->swapScreensWithTransition(this, App->modScreen->screenGame);
	}

	ImGui::End();
}
