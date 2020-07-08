
#pragma once

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

/**	\file	MarkerWireManip.h

  Manipulator to store global list of scene wire markers and render then in a current pane.

*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <vector>

//--- Registration defines
#define MARKER_WIRE_MANIP__CLASSNAME	CMarkerWireManip
#define MARKER_WIRE_MANIP__CLASSSTR		"CMarkerWireManip"


//! Manipulator Marker Wire
class CMarkerWireManip : public FBManipulator
{
	//--- FiLMBOX declaration.
	FBManipulatorDeclare(CMarkerWireManip, FBManipulator);

public:
	//! FiLMBOX Constructor.
	virtual bool FBCreate();

	//!< FiLMBOX Destructor.
	virtual void FBDestroy();

	//! Manipulator expose function
	virtual void ViewExpose();

	void OnPerFrameRenderingPipelineCallback(HISender pSender, HKEvent pEvent);
	
	static void RegisterMarkerModel(FBModel* pModel);
	static void UnRegisterMarkerModel(FBModel* pModel);

protected:
	
	FBSystem		mSystem;
	int				mRenderPaneIndex;

	static std::vector<FBModel*>		m_Markers;

};

