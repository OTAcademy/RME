#ifndef RME_LIVE_LIVE_MANAGER_H
#define RME_LIVE_LIVE_MANAGER_H

#include "app/rme_forward_declarations.h"
#include <memory>

class Editor;
class LiveServer;
class LiveClient;
class LiveSocket;
class DirtyList;

class LiveManager {
public:
	LiveManager(Editor& editor);
	LiveManager(Editor& editor, std::unique_ptr<LiveClient> client);
	~LiveManager();

	LiveServer* StartServer();
	void CloseServer();

	LiveClient* GetClient() const;
	LiveServer* GetServer() const;
	LiveSocket& GetSocket() const;

	bool IsLive() const;
	bool IsLocal() const;
	bool IsServer() const;
	bool IsClient() const;

	void BroadcastNodes(DirtyList& dirty_list);

private:
	Editor& editor;
	std::unique_ptr<LiveServer> live_server;
	std::unique_ptr<LiveClient> live_client;
};

#endif
