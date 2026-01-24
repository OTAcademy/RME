#ifndef RME_LIVE_LIVE_MANAGER_H
#define RME_LIVE_LIVE_MANAGER_H

#include "app/rme_forward_declarations.h"

class Editor;
class LiveServer;
class LiveClient;
class LiveSocket;
class DirtyList;

class LiveManager {
public:
	LiveManager(Editor& editor);
	LiveManager(Editor& editor, LiveClient* client);
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
	LiveServer* live_server;
	LiveClient* live_client;
};

#endif
