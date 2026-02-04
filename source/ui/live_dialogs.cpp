//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "ui/live_dialogs.h"
#include "ui/gui.h"
#include "ui/dialog_util.h"
#include "editor/editor.h"
#include "live/live_server.h"
#include "live/live_client.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

extern GUI g_gui;

void LiveDialogs::ShowHostDialog(wxWindow* parent, Editor* editor) {
	if (!editor) {
		DialogUtil::PopupDialog("Error", "You need to have a map open to start a live mapping session.", wxOK);
		return;
	}
	if (editor->live_manager.IsLive()) {
		DialogUtil::PopupDialog("Error", "You can not start two live servers on the same map (or a server using a remote map).", wxOK);
		return;
	}

	wxDialog* live_host_dlg = newd wxDialog(parent, wxID_ANY, "Host Live Server", wxDefaultPosition, wxDefaultSize);

	wxSizer* top_sizer = newd wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer* gsizer = newd wxFlexGridSizer(2, 10, 10);
	gsizer->AddGrowableCol(0, 2);
	gsizer->AddGrowableCol(1, 3);

	// Data fields
	wxTextCtrl* hostname;
	wxSpinCtrl* port;
	wxTextCtrl* password;
	wxCheckBox* allow_copy;

	gsizer->Add(newd wxStaticText(live_host_dlg, wxID_ANY, "Server Name:"));
	gsizer->Add(hostname = newd wxTextCtrl(live_host_dlg, wxID_ANY, "RME Live Server"), 0, wxEXPAND);

	gsizer->Add(newd wxStaticText(live_host_dlg, wxID_ANY, "Port:"));
	gsizer->Add(port = newd wxSpinCtrl(live_host_dlg, wxID_ANY, "31313", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535, 31313), 0, wxEXPAND);

	gsizer->Add(newd wxStaticText(live_host_dlg, wxID_ANY, "Password:"));
	gsizer->Add(password = newd wxTextCtrl(live_host_dlg, wxID_ANY), 0, wxEXPAND);

	top_sizer->Add(gsizer, 0, wxALL, 20);

	top_sizer->Add(allow_copy = newd wxCheckBox(live_host_dlg, wxID_ANY, "Allow copy & paste between maps."), 0, wxRIGHT | wxLEFT, 20);
	allow_copy->SetToolTip("Allows remote clients to copy & paste from the hosted map to local maps.");

	wxSizer* ok_sizer = newd wxBoxSizer(wxHORIZONTAL);
	auto okBtn = newd wxButton(live_host_dlg, wxID_OK, "OK");
	okBtn->SetToolTip("Start server");
	ok_sizer->Add(okBtn, 1, wxCENTER);
	auto cancelBtn = newd wxButton(live_host_dlg, wxID_CANCEL, "Cancel");
	cancelBtn->SetToolTip("Cancel");
	ok_sizer->Add(cancelBtn, wxCENTER, 1);
	top_sizer->Add(ok_sizer, 0, wxCENTER | wxALL, 20);

	live_host_dlg->SetSizerAndFit(top_sizer);

	while (true) {
		int ret = live_host_dlg->ShowModal();
		if (ret == wxID_OK) {
			LiveServer* liveServer = editor->live_manager.StartServer();
			liveServer->setName(hostname->GetValue());
			liveServer->setPassword(password->GetValue());
			liveServer->setPort(port->GetValue());

			const wxString& error = liveServer->getLastError();
			if (!error.empty()) {
				DialogUtil::PopupDialog(live_host_dlg, "Error", error, wxOK);
				editor->live_manager.CloseServer();
				continue;
			}

			if (!liveServer->bind()) {
				DialogUtil::PopupDialog("Socket Error", "Could not bind socket! Try another port?", wxOK);
				editor->live_manager.CloseServer();
			} else {
				liveServer->createLogWindow(g_gui.tabbook.get());
			}
			break;
		} else {
			break;
		}
	}
	live_host_dlg->Destroy();
}

void LiveDialogs::ShowJoinDialog(wxWindow* parent) {
	wxDialog* live_join_dlg = newd wxDialog(parent, wxID_ANY, "Join Live Server", wxDefaultPosition, wxDefaultSize);

	wxSizer* top_sizer = newd wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer* gsizer = newd wxFlexGridSizer(2, 10, 10);
	gsizer->AddGrowableCol(0, 2);
	gsizer->AddGrowableCol(1, 3);

	// Data fields
	wxTextCtrl* name;
	wxTextCtrl* ip;
	wxSpinCtrl* port;
	wxTextCtrl* password;

	gsizer->Add(newd wxStaticText(live_join_dlg, wxID_ANY, "Name:"));
	gsizer->Add(name = newd wxTextCtrl(live_join_dlg, wxID_ANY, ""), 0, wxEXPAND);

	gsizer->Add(newd wxStaticText(live_join_dlg, wxID_ANY, "IP:"));
	gsizer->Add(ip = newd wxTextCtrl(live_join_dlg, wxID_ANY, "localhost"), 0, wxEXPAND);

	gsizer->Add(newd wxStaticText(live_join_dlg, wxID_ANY, "Port:"));
	gsizer->Add(port = newd wxSpinCtrl(live_join_dlg, wxID_ANY, "31313", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535, 31313), 0, wxEXPAND);

	gsizer->Add(newd wxStaticText(live_join_dlg, wxID_ANY, "Password:"));
	gsizer->Add(password = newd wxTextCtrl(live_join_dlg, wxID_ANY), 0, wxEXPAND);

	top_sizer->Add(gsizer, 0, wxALL, 20);

	wxSizer* ok_sizer = newd wxBoxSizer(wxHORIZONTAL);
	auto okBtn = newd wxButton(live_join_dlg, wxID_OK, "OK");
	okBtn->SetToolTip("Connect to server");
	ok_sizer->Add(okBtn, 1, wxRIGHT);
	auto cancelBtn = newd wxButton(live_join_dlg, wxID_CANCEL, "Cancel");
	cancelBtn->SetToolTip("Cancel");
	ok_sizer->Add(cancelBtn, 1, wxRIGHT);
	top_sizer->Add(ok_sizer, 0, wxCENTER | wxALL, 20);

	live_join_dlg->SetSizerAndFit(top_sizer);

	while (true) {
		int ret = live_join_dlg->ShowModal();
		if (ret == wxID_OK) {
			auto liveClient = std::make_unique<LiveClient>();
			liveClient->setPassword(password->GetValue());

			wxString tmp = name->GetValue();
			if (tmp.empty()) {
				tmp = "User";
			}
			liveClient->setName(tmp);

			const wxString& error = liveClient->getLastError();
			if (!error.empty()) {
				DialogUtil::PopupDialog(live_join_dlg, "Error", error, wxOK);
				liveClient.reset();
				continue;
			}

			const wxString& address = ip->GetValue();
			int32_t portNumber = port->GetValue();

			liveClient->createLogWindow(g_gui.tabbook.get());
			if (!liveClient->connect(nstr(address), portNumber)) {
				DialogUtil::PopupDialog("Connection Error", liveClient->getLastError(), wxOK);
			} else {
				// Transfer ownership to GUI to handle pending connection
				g_gui.AddPendingLiveClient(std::move(liveClient));
			}

			break;
		} else {
			break;
		}
	}
	live_join_dlg->Destroy();
}
