/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LW_GLGUI_FILEDIALOG_H
#define LW_GLGUI_FILEDIALOG_H

#include "movablepanel.h"

namespace glgui
{
	class CFileDialog : public CMovablePanel
	{
		DECLARE_CLASS(CFileDialog, CMovablePanel);

		using CMovablePanel::Close;

	public:
									CFileDialog(const tstring& sDirectory, const tstring& sExtension, bool bSave);
		virtual						~CFileDialog();

	public:
		virtual void				Layout();

		EVENT_CALLBACK(CFileDialog, NewDirectory);
		EVENT_CALLBACK(CFileDialog, Explore);
		EVENT_CALLBACK(CFileDialog, FileSelected);
		EVENT_CALLBACK(CFileDialog, NewFileChanged);
		EVENT_CALLBACK(CFileDialog, Select);
		EVENT_CALLBACK(CFileDialog, Close);
		EVENT_CALLBACK(CFileDialog, FileConfirmed);
		EVENT_CALLBACK(CFileDialog, FileType);

		void						FileConfirmed(const tstring& sFile);

		static void					ShowOpenDialog(const tstring& sDirectory, const tstring& sExtension, IEventListener* pListener, IEventListener::Callback pfnCallback);
		static void					ShowSaveDialog(const tstring& sDirectory, const tstring& sExtension, IEventListener* pListener, IEventListener::Callback pfnCallback);
		static tstring				GetFile();
		static void					SetDefaultDirectory(const tstring sDirectory) { s_sDirectory = sDirectory; }

	protected:
		CControl<CLabel>			m_hDirectoryLabel;
		CControl<CTextField>		m_hDirectory;
		CControl<CButton>			m_hOpenInExplorer;

		CControl<CLabel>			m_hFilesLabel;
		CControl<CTree>				m_hFileList;

		CControl<CTextField>		m_hNewFile;
		CControl<CButton>			m_hSelect;
		CControl<CButton>			m_hCancel;

		CControl<CMenu>				m_hFileTypes;

		tstring						m_sDirectory;
		tvector<tstring>			m_asExtensions;
		size_t						m_iCurrentExtension;	// (m_iCurrentExtension - 1) is an index into m_asExtensions. 0 means all.
		bool						m_bSave;

		IEventListener*				m_pSelectListener;
		IEventListener::Callback	m_pfnSelectCallback;

		static CControl<CFileDialog>	s_hDialog;
		static tstring				s_sDirectory;
	};
};

#endif
