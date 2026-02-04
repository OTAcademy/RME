//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "live/live_manager.h"
#include "editor/editor.h"
#include "editor/action_queue.h"
#include "live/live_server.h"
#include "live/live_client.h"
#include "live/live_action.h"
#include "net/net_connection.h"
#include "app/definitions.h" // For ASSERT/newd

LiveManager::LiveManager(Editor& editor) :
	editor(editor),
	live_server(nullptr),
	live_client(nullptr) {
}

LiveManager::LiveManager(Editor& editor, std::unique_ptr<LiveClient> client) :
	editor(editor),
	live_server(nullptr),
	live_client(std::move(client)) {
}

LiveManager::~LiveManager() {
	if (IsLive()) {
		CloseServer();
	}
}

bool LiveManager::IsClient() const {
	return live_client != nullptr;
}

bool LiveManager::IsServer() const {
	return live_server != nullptr;
}

bool LiveManager::IsLive() const {
	return IsClient() || IsServer();
}

bool LiveManager::IsLocal() const {
	return !IsLive();
}

LiveClient* LiveManager::GetClient() const {
	return live_client;
}

LiveServer* LiveManager::GetServer() const {
	return live_server;
}

LiveSocket& LiveManager::GetSocket() const {
	if (live_server) {
		return *live_server;
	}
	return *live_client;
}

LiveServer* LiveManager::StartServer() {
	ASSERT(IsLocal());
	live_server = std::make_unique<LiveServer>(editor);

	delete editor.actionQueue;
	editor.actionQueue = newd NetworkedActionQueue(editor);

	return live_server.get();
}

void LiveManager::CloseServer() {
	if (live_server) {
		live_server->close();
		live_server.reset();
	}

	if (live_client) {
		live_client->close();
		live_client.reset();
	}

	delete editor.actionQueue;
	editor.actionQueue = newd ActionQueue(editor);

	NetworkConnection& connection = NetworkConnection::getInstance();
	connection.stop();
}

void LiveManager::BroadcastNodes(DirtyList& dirtyList) {
	if (IsClient()) {
		live_client->sendChanges(dirtyList);
	} else if (IsServer()) {
		live_server->broadcastNodes(dirtyList);
	}
}
