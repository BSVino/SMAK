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

#ifndef TINKER_TREE_H
#define TINKER_TREE_H

#include "panel.h"
#include "picturebutton.h"

namespace glgui
{
	class CTreeNode : public CPanel, public IDraggable, public glgui::IEventListener
	{
		DECLARE_CLASS(CTreeNode, CPanel);

	public:
											CTreeNode(CControl<CTreeNode> hParent, CControl<CTree> hTree, const tstring& sText, const tstring& sFont);
											CTreeNode(const CTreeNode& c);
		virtual								~CTreeNode();

	public:
		virtual float						GetNodeHeight();
		virtual float						GetNodeSpacing() { return 0; };
		virtual void						LayoutNode();
		virtual void						Paint() { CPanel::Paint(); };
		virtual void						Paint(float x, float y) { CPanel::Paint(x, y); };
		virtual void						Paint(float x, float y, float w, float h);
		virtual void						Paint(float x, float y, float w, float h, bool bFloating);

		size_t								AddNode(const tstring& sName);
		template <typename T>
		size_t								AddNode(const tstring& sName, T* pObject);
		size_t								AddNode(CBaseControl* pNode);
		void								RemoveNode(CTreeNode* pNode);
		CControl<CTreeNode>					GetNode(size_t i);
		size_t								GetNumNodes() { return m_ahNodes.size(); };

		virtual void						AddVisibilityButton() {};

		virtual void						Selected();

		bool								IsExpanded() { return m_hExpandButton->IsExpanded(); };
		void								SetExpanded(bool bExpanded) { m_hExpandButton->SetExpanded(bExpanded); };

		void								SetIcon(const CMaterialHandle& hMaterial) { m_hIconMaterial = hMaterial; };
		virtual void						SetDraggable(bool bDraggable) { m_bDraggable = true; };

		virtual bool						IsVisible();

		// IDraggable
		virtual void						SetHoldingRect(const FRect&);
		virtual FRect						GetHoldingRect();

		virtual IDroppable*					GetDroppable();
		virtual void						SetDroppable(IDroppable* pDroppable);

		virtual float						GetWidth() { return BaseClass::GetWidth(); };
		virtual float						GetHeight() { return BaseClass::GetHeight(); };

		virtual DragClass_t					GetClass() { return DC_UNSPECIFIED; };
		virtual IDraggable*					MakeCopy() { return new CTreeNode(*this); };
		virtual bool						IsDraggable() { return m_bDraggable; };

		EVENT_CALLBACK(CTreeNode, Expand);

	public:
		tvector<CControl<CTreeNode>>		m_ahNodes;
		CControl<CTreeNode>					m_hParent;
		CControl<CTree>						m_hTree;
		CControl<CLabel>					m_hLabel;

		CMaterialHandle						m_hIconMaterial;

		CControl<CPictureButton>			m_hVisibilityButton;
		CControl<CPictureButton>			m_hEditButton;

		bool								m_bDraggable;

		class CExpandButton : public CPictureButton
		{
		public:
											CExpandButton(const CMaterialHandle& hMaterial);

		public:
			void							Think();
			void							Paint() { CButton::Paint(); };
			void							Paint(float x, float y, float w, float h);

			bool							IsExpanded() { return m_bExpanded; };
			void							SetExpanded(bool bExpanded);

		public:
			bool							m_bExpanded;
			float							m_flExpandedCurrent;
			float							m_flExpandedGoal;
		};

		CControl<CExpandButton>				m_hExpandButton;
	};

	class CTree : public CPanel, public IDroppable
	{
		DECLARE_CLASS(CTree, CPanel);

		friend class CTreeNode;

	public:
											CTree(const CMaterialHandle& hArrowMaterial = CMaterialHandle(), const CMaterialHandle& hEditMaterial = CMaterialHandle(), const CMaterialHandle& hVisibilityMaterial = CMaterialHandle());
		virtual								~CTree();

	public:
		virtual void						Layout();
		virtual void						Think();
		virtual void						Paint();
		virtual void						Paint(float x, float y);
		virtual void						Paint(float x, float y, float w, float h);

		virtual bool						MousePressed(int code, int mx, int my);
		virtual bool						MouseReleased(int iButton, int mx, int my);
		virtual bool						MouseDoubleClicked(int iButton, int mx, int my);

		virtual CControlHandle				AddControl(CBaseControl* pControl, bool bToTail = false);
		virtual void						RemoveControl(CBaseControl* pControl);

		void								ClearTree();

		size_t								AddNode(const tstring& sName);
		template <typename T>
		size_t								AddNode(const tstring& sName, T* pObject);
		size_t								AddNode(CBaseControl* pNode, size_t iPosition = ~0);
		void								RemoveNode(CTreeNode* pNode);
		CControl<CTreeNode>					GetNode(size_t i);

		virtual CControl<CTreeNode>			GetSelectedNode() const { if (m_iSelected == ~0) return CControl<CTreeNode>(); return m_ahAllNodes[m_iSelected]; };
		virtual size_t						GetSelectedNodeId() { return m_iSelected; };
		virtual void						Unselect() { m_iSelected = ~0; }
		virtual void						SetSelectedNode(size_t iNode);
		virtual void						SetSelectedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);
		virtual void						SetConfirmedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);

		virtual void						SetDroppedListener(IEventListener* pListener, IEventListener::Callback pfnCallback);

		// IDroppable
		virtual const FRect					GetHoldingRect() { return GetAbsDimensions(); };

		virtual void						AddDraggable(IDraggable*) {};
		virtual void						SetDraggable(IDraggable*, bool bDelete = true);
		virtual IDraggable*					GetDraggable(int i) { return static_cast<IDraggable*>(m_hDragging.Get()); };
		virtual IDraggable*					GetCurrentDraggable() { return static_cast<IDraggable*>(m_hDragging.Get()); };

		// I already know.
		virtual void						SetGrabbale(bool bGrabbable) {};
		virtual bool						IsGrabbale() { return true; };

		virtual bool						CanDropHere(IDraggable*) { return m_bDroppable; };
		virtual void						SetDroppable(bool bDrop) { m_bDroppable = bDrop; };

		virtual bool						IsInfinite() { return true; };
		virtual bool						IsVisible() { return BaseClass::IsVisible(); };

	public:
		tvector<CControl<CTreeNode>>		m_ahNodes;
		tvector<CControl<CTreeNode>>		m_ahAllNodes;

		float								m_flCurrentHeight;
		float								m_flCurrentDepth;

		size_t								m_iHilighted;
		size_t								m_iSelected;

		CMaterialHandle						m_hArrowMaterial;
		CMaterialHandle						m_hVisibilityMaterial;
		CMaterialHandle						m_hEditMaterial;

		IEventListener::Callback			m_pfnSelectedCallback;
		IEventListener*						m_pSelectedListener;

		IEventListener::Callback			m_pfnConfirmedCallback;
		IEventListener*						m_pConfirmedListener;

		IEventListener::Callback			m_pfnDroppedCallback;
		IEventListener*						m_pDroppedListener;

		bool								m_bMouseDown;
		int									m_iMouseDownX;
		int									m_iMouseDownY;

		bool								m_bDroppable;

		CControl<CTreeNode>					m_hDragging;
		int									m_iAcceptsDragType;
	};

	template <typename T>
	class CTreeNodeObject : public CTreeNode
	{
	public:
		CTreeNodeObject(T* pObject, CControlHandle hParent, CControlHandle hTree, const tstring& sName)
			: CTreeNode(hParent, hTree, sName, "sans-serif")
		{
			m_pObject = pObject;
		}

		~CTreeNodeObject()
		{
			TAssertNoMsg(!GetParent());
		}

	public:
		typedef void (*EditFnCallback)(T*, const tstring& sArgs);

		virtual void LayoutNode()
		{
			CTreeNode::LayoutNode();

			float iHeight = m_hLabel->GetTextHeight();

			if (m_hVisibilityButton)
			{
				m_hVisibilityButton->SetPos(GetWidth()-iHeight-14, 0);
				m_hVisibilityButton->SetSize(iHeight, iHeight);
			}

			if (m_hEditButton)
			{
				m_hEditButton->SetPos(GetWidth()-iHeight*2-16, 0);
				m_hEditButton->SetSize(iHeight, iHeight);
			}

			m_hLabel->SetAlpha(m_pObject->IsVisible()?255:100);
		}

		virtual void AddVisibilityButton()
		{
			m_hVisibilityButton = AddControl(new CPictureButton("@", m_hTree->m_hVisibilityMaterial));
			m_hVisibilityButton->SetClickedListener(this, Visibility);
		}

		virtual void AddEditButton(EditFnCallback pfnCallback)
		{
			m_hEditButton = AddControl(new CPictureButton("*", m_hTree->m_hEditMaterial));
			m_hEditButton->SetClickedListener(this, Edit);
			m_pfnCallback = pfnCallback;
		}

		virtual T* GetObject() { return m_pObject; }

		virtual IDraggable*				MakeCopy() { return new CTreeNodeObject<T>(*this); };

		EVENT_CALLBACK(CTreeNodeObject, Visibility);
		EVENT_CALLBACK(CTreeNodeObject, Edit);

	protected:
		T*									m_pObject;

		EditFnCallback						m_pfnCallback;
	};

	template <typename T>
	inline void CTreeNodeObject<T>::VisibilityCallback(const tstring& sArgs)
	{
		m_pObject->SetVisible(!m_pObject->IsVisible());

		m_hLabel->SetAlpha(m_pObject->IsVisible()?255:100);
	}

	template <typename T>
	inline void CTreeNodeObject<T>::EditCallback(const tstring& sArgs)
	{
		m_pfnCallback(m_pObject, sArgs);
	}

	template <typename T>
	inline size_t CTreeNode::AddNode(const tstring& sName, T* pObject)
	{
		return AddNode(new CTreeNodeObject<T>(pObject, m_hThis, m_hTree, sName));
	}

	template <typename T>
	inline size_t CTree::AddNode(const tstring& sName, T* pObject)
	{
		return AddNode(new CTreeNodeObject<T>(pObject, CControlHandle(), m_hThis, sName));
	}
};

#endif
