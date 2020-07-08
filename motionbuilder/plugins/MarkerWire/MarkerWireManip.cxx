/*
#
# Copyright(c) 2020 Avalanche Studios.All rights reserved.
# Licensed under the MIT License.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE
#
*/

/**	\file	MarkerWireManip.cxx
 
  Manipulator to store global list of scene wire markers and render then in a current pane.

*/

//--- Class declarations
#include "MarkerWireManip.h"
#include "MarkerWire.h"

#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include <Windows.h>
#include <GL\GL.h>

#include <vector>
#include <algorithm>

//--- Registration defines
#define MARKER_WIRE_MANIP__CLASS	MARKER_WIRE_MANIP__CLASSNAME
#define MARKER_WIRE_MANIP__LABEL	"Manip Marker Wire"
#define MARKER_WIRE_MANIP__DESC		"Manipulator To Draw Marker Wire Connections"

//--- FiLMBOX implementation and registration
FBManipulatorImplementation(MARKER_WIRE_MANIP__CLASS);
FBRegisterManipulator(		MARKER_WIRE_MANIP__CLASS,
							MARKER_WIRE_MANIP__LABEL,
							MARKER_WIRE_MANIP__DESC,
							FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)

//
std::vector<FBModel*>		CMarkerWireManip::m_Markers;

void CMarkerWireManip::RegisterMarkerModel(FBModel* pModel)
{
	m_Markers.push_back(pModel);
}
void CMarkerWireManip::UnRegisterMarkerModel(FBModel* pModel)
{
	auto iter = std::find(begin(m_Markers), end(m_Markers), pModel);

	if (iter != end(m_Markers))
	{
		m_Markers.erase(iter);
	}
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool CMarkerWireManip::FBCreate()
{
	if( FBManipulator::FBCreate() )
	{
		Active = true;
		AlwaysActive = true;
		Visible = false;

		// Properties
		DefaultBehavior		= true;
		ViewerText			= "Draw Marker Wires Manipulator";

		// Callbacks
		
		FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Add(this, (FBCallback)&CMarkerWireManip::OnPerFrameRenderingPipelineCallback);
		return true;
	}
	return false;
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void CMarkerWireManip::FBDestroy()
{
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Remove(this, (FBCallback)&CMarkerWireManip::OnPerFrameRenderingPipelineCallback);
	
	FBManipulator::FBDestroy();
}


/************************************************
 *	Draw function for manipulator
 ************************************************/
void CMarkerWireManip::ViewExpose()
{
	int paneIndex = mRenderPaneIndex;
	mRenderPaneIndex += 1;

	// DONE: grab current pane dimensions

	const int panex = GetPanePosX();
	const int paney = GetPanePosY();
	const int panew = GetPaneWidth();
	const int paneh = GetPaneHeight();

	GLint lViewport[4];
	glGetIntegerv(GL_VIEWPORT, lViewport);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0, lViewport[2], lViewport[3], 0.0, -1.0, 1.0);

	FBRenderer* pRenderer = mSystem.Renderer;
	FBCamera* pCamera = pRenderer->GetCameraInPane(paneIndex);

	if (pCamera && FBIS(pCamera, FBCameraSwitcher))
		pCamera = ((FBCameraSwitcher*)pCamera)->CurrentCamera;

	if (pCamera == nullptr)
	{
		return;
	}

	// DONE: connect render_mod with current pane display mode

	bool render_mode = true;

	if (pRenderer->GetPaneCount() == 1)
	{
		FBViewingOptions* pOptions = pRenderer->GetViewingOptions();
		render_mode = (pOptions->PickingMode() != kFBPickingModeModelsOnly);
	}

	FBMatrix modelview, proj;

	pCamera->GetCameraMatrix(modelview, kFBModelView);
	pCamera->GetCameraMatrix(proj, kFBProjection);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixd(proj);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixd(modelview);

	for (FBModel *pModel : m_Markers)
	{
		CMarkerWire* marker = static_cast<CMarkerWire*>(pModel);

		if (marker->LinkToObjects.GetCount() > 0 && render_mode)
		{
			glPushMatrix();

			glColor3d(0.5, 0.5, 0.5);
			glLineWidth(1.0f);    //!< Draw line wider to easier picking.
			glLineStipple(1, 0x1C47); // 0x5557);

			glEnable(GL_LINE_STIPPLE);

			FBVector3d v1, v2;
			marker->GetVector(v1);

			for (int i = 0; i < marker->LinkToObjects.GetCount(); ++i)
			{
				if (FBIS(marker->LinkToObjects[i], FBModel))
				{
					FBModel* linked_model = static_cast<FBModel*>(marker->LinkToObjects[i]);
					linked_model->GetVector(v2);

					glBegin(GL_LINES);
					glVertex3dv(v1);
					glVertex3dv(v2);
					glEnd();
				}
			}

			glDisable(GL_LINE_STIPPLE);
			glPopMatrix();
		}
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


void CMarkerWireManip::OnPerFrameRenderingPipelineCallback(HISender pSender, HKEvent pEvent)
{
	FBEventEvalGlobalCallback lFBEvent(pEvent);

	switch (lFBEvent.GetTiming())
	{
	case kFBGlobalEvalCallbackBeforeRender:
		mRenderPaneIndex = 0;
		break;
	}
}