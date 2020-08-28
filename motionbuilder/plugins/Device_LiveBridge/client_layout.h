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
/**	\file	client_layout.h
*	Declaration for the layout of client input device.
*
*/

//--- Class declaration
#include "client_device.h"

///////////////////////////////////////////////////////////////////////////
//! Client device layout.
class CClientDeviceLayout : public FBDeviceLayout
{
	//--- FiLMBOX declaration.
	FBDeviceLayoutDeclare(CClientDeviceLayout, FBDeviceLayout );

public:
	//--- FiLMBOX Creation/Destruction.
	virtual bool FBCreate();			//!< FiLMBOX constructor.
	virtual void FBDestroy();			//!< FiLMBOX destructor.

	// UI Management
	void	UICreate				();		// Create & assign UI regions
	void		UICreateLayout0		();
	void		UICreateLayout1		();
	void		UICreateLayout2		();
	void	UIConfigure				();		// Configure UI options
	void		UIConfigureLayout0	();
	void		UIConfigureLayout1	();
	void		UIConfigureLayout2	();
	void	UIRefresh				();		// Refresh on idle
	void		UIRefreshSpreadSheet();
	void	UIReset					();		// Reset from device values
	void		UIResetSpreadSheet	();

	// Main Layout: Events
	void	EventDeviceStatusChange					( HISender pSender, HKEvent pEvent );
	void	EventUIIdle								( HISender pSender, HKEvent pEvent );
	void	EventTabPanelChange						( HISender pSender, HKEvent pEvent );

	void	EventSelectTargetClick(HISender pSender, HKEvent pEvent);
	void	EventButtonSwitchPipelineClick(HISender pSender, HKEvent pEvent);
	void	EventSwitchOnFastIdleClick(HISender pSender, HKEvent pEvent);
	void	EventSwitchOffFastIdleClick(HISender pSender, HKEvent pEvent);

	// Layout 1: Events
	void	EventButtonPresetExportClick(HISender pSender, HKEvent pEvent);
	void	EventButtonPresetImportClick(HISender pSender, HKEvent pEvent);

private:
	FBTabPanel			mTabPanel;

	FBLayout			mLayoutMarkers;
		FBSpread			mSpreadMarkers;

	FBLayout			mLayoutSetup;
		FBEditProperty		mEditPairName;
		FBEditProperty		mEditSyncTimeWithStoryClip;
		FBEditProperty		mEditStoryClipIndex;
		FBEditProperty		mEditRootObject;
		FBEditProperty		mEditLeftObject;
		FBEditProperty		mEditRightObject;

		FBEditProperty		mEditImportTarget;
		FBEditProperty		mEditExportTarget;
		FBEditProperty		mEditLoadRotationSetup;

		// additional functions
		FBButton			mButtonSelectTarget;
		FBButton			mButtonSwitchPipeline;

		FBLabel				mLabelFastIdle;
		FBButton			mButtonSwitchOnFastIdle;
		FBButton			mButtonSwitchOffFastIdle;

	FBLayout			mLayoutInformation;
		FBLabel				mLabelInformation;
		FBButton			mButtonPresetExport;
		FBButton			mButtonPresetImport;

private:
	FBSystem				m_System;					//!< System interface.
	CClientDevice*			m_Device;					//!< Handle onto device.

	void SwitchFastIdle(const int value);
};
