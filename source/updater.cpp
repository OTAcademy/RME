//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#ifdef _USE_UPDATER_

	#include <wx/sstream.h>
	#include <wx/protocol/http.h>
	#include <cpr/cpr.h>

	#include "json.h"
	#include "updater.h"

const wxEventType EVT_UPDATE_CHECK_FINISHED = wxNewEventType();

UpdateChecker::UpdateChecker() {
}

UpdateChecker::~UpdateChecker() {
}

void UpdateChecker::connect(wxEvtHandler* receiver) {
	UpdateConnectionThread* connection = newd UpdateConnectionThread(receiver);
	connection->Execute();
}

UpdateConnectionThread::UpdateConnectionThread(wxEvtHandler* receiver) :
	receiver(receiver) {
}

UpdateConnectionThread::~UpdateConnectionThread() {
}

wxThread::ExitCode UpdateConnectionThread::Entry() {
	cpr::Response response = cpr::Get(
		cpr::Url { __UPDATE_URL__ },
		cpr::Header { { "User-Agent", "OTAcademy-RME-Updater" } },
		cpr::Timeout { 10000 }
	);

	if (response.status_code != 200 || response.text.empty()) {
		return 0;
	}

	wxCommandEvent event(EVT_UPDATE_CHECK_FINISHED);
	event.SetClientData(newd std::string(response.text));
	if (receiver) {
		receiver->AddPendingEvent(event);
	}
	return 0;
}

#endif
