#include "Networks.h"
#include "ReplicationManagerClient.h"

// TODO(you): World state replication lab session


void ReplicationManagerClient::read(const InputMemoryStream & packet)
{
	while (packet.RemainingByteCount() > sizeof(uint32))
	{
		uint32 networkId;
		packet >> networkId;

		ReplicationAction action;
		packet >> action;

		if (action == ReplicationAction::Create)
		{
			GameObject* object = nullptr;

			object = App->modLinkingContext->getNetworkGameObject(networkId);

			if (object)
			{
				App->modLinkingContext->unregisterNetworkGameObject(object);
				Destroy(object);
			}

			object = Instantiate();

			App->modLinkingContext->registerNetworkGameObjectWithNetworkId(object, networkId);

			std::string texture;

			packet >> object->position.x;
			packet >> object->position.y;
			packet >> object->angle;
			packet >> object->size.x;
			packet >> object->size.y;
			packet >> texture;

			object->sprite = App->modRender->addSprite(object);
			object->sprite->order = 5;
			object->sprite->texture = App->modTextures->loadTexture(texture.c_str());

			object->sprite->order = 100;

			packet >> object->explosion;

			if (object->explosion)
			{
				object->animation = App->modRender->addAnimation(object);
				object->animation->clip = App->modResources->explosionClip;
			}


		}
		else if (action == ReplicationAction::Update)
		{
			GameObject* object = nullptr;
			object = App->modLinkingContext->getNetworkGameObject(networkId);

			float pos_x = 0.0f;
			float pos_y = 0.0f;
			float angle = 0.0f;

			packet >> pos_x;
			packet >> pos_y;
			packet >> angle;

			packet >> object->explosion;

			if (object)
			{

				//packet >> it_rc->first;
				//packet >> it_rc->second;

				object->position.x = pos_x;
				object->position.y = pos_y;
				object->angle = angle;
				//if (object->animation != nullptr)
				//packet >> object->animation;
			}
		}
		else
		{
			GameObject* object = nullptr;
			object = App->modLinkingContext->getNetworkGameObject(networkId);

			if (object)
			{
				App->modLinkingContext->unregisterNetworkGameObject(object);

				Destroy(object);
			}
		}
	}
}
