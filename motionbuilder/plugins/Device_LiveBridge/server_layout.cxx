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
/**	\file	server_layout.cxx
*	Definition of the layout for a server output device.
*
*/

//--- Class declarations
#include "server_layout.h"

//--- Registration define
#define SERVERDEVICE__LAYOUT	CServerDeviceLayout

//--- FiLMBOX implementation and registration
FBDeviceLayoutImplementation(	SERVERDEVICE__LAYOUT		);
FBRegisterDeviceLayout		(	SERVERDEVICE__LAYOUT,
								SERVERDEVICE__CLASSSTR,
								FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX constructor.
 ************************************************/
bool CServerDeviceLayout::FBCreate()
{
	// Get a handle on the device.
	m_Device = ((CServerDevice *)(FBDevice *)Device);

	UICreate	();
	UIConfigure	();
	UIReset		();

	m_Device->OnStatusChange.Add	( this,(FBCallback)&CServerDeviceLayout::EventDeviceStatusChange   );
	m_System.OnUIIdle.Add		( this,(FBCallback)&CServerDeviceLayout::EventUIIdle               );

	return true;
}


/************************************************
 *	FiLMBOX destructor.
 ************************************************/
void CServerDeviceLayout::FBDestroy()
{
	m_Device->OnStatusChange.Remove	( this,(FBCallback)&CServerDeviceLayout::EventDeviceStatusChange   );
	m_System.OnUIIdle.Remove			( this,(FBCallback)&CServerDeviceLayout::EventUIIdle               );
}


/************************************************
 *	UI Creation.
 ************************************************/
void CServerDeviceLayout::UICreate()
{
	int lS, lH;		// space, height
	lS = 5;
	lH = 25;

	// Create regions
	AddRegion	( "TabPanel",	"TabPanel",		0,		kFBAttachLeft,		"",			1.00,
												0,		kFBAttachTop,		"",			1.00,
												0,		kFBAttachRight,		"",			1.00,
												lH,		kFBAttachNone,		NULL,		1.00 );
	AddRegion	( "MainLayout",	"MainLayout",	lS,		kFBAttachLeft,		"TabPanel",	1.00,
												lS,		kFBAttachBottom,	"TabPanel",	1.00,
												-lS,	kFBAttachRight,		"TabPanel",	1.00,
												-lS,	kFBAttachBottom,	"",			1.00 );

	// Assign regions
	SetControl	( "TabPanel",	mTabPanel				);
	SetControl	( "MainLayout",	mLayoutSetup	);

	// Create sub layouts
	UICreateLayout0();
	UICreateLayout1();
}


/************************************************
 *	Create the sub-layout 0.
 ************************************************/
void CServerDeviceLayout::UICreateLayout0()
{
	int lS, lW, lW2, lH;		// space, width, height.
	lS = 6;
	lW = 300;
	lW2 = 100;
	lH = 20;

	// Add regions
	mLayoutSetup.AddRegion("LabelExportRate",	"LabelExportRate",
													lS,			kFBAttachLeft,		"",							1.0,
													lS,			kFBAttachTop,		"",							1.0,
													lW2,			kFBAttachNone,		NULL,						1.0,
													lH,			kFBAttachNone,		NULL,						1.0 );
	mLayoutSetup.AddRegion("EditNumberExportRate",		"EditNumberExportRate",
													lS,			kFBAttachRight,		"LabelExportRate",		1.0,
													0,			kFBAttachTop,		"LabelExportRate",		1.0,
													(3*lW2)/2,	kFBAttachNone,		NULL,					1.0,
													0,			kFBAttachHeight,	"LabelExportRate",		1.0 );
	

	mLayoutSetup.AddRegion("PairName", "PairName",
		lS, kFBAttachLeft,		"", 1.0,
		lS, kFBAttachBottom,	"LabelExportRate", 1.0,
		lW, kFBAttachNone,		"", 1.0,
		lH, kFBAttachNone,		"", 1.0);
	mLayoutSetup.AddRegion("JointSet", "JointSet",
		lS, kFBAttachRight, "PairName", 1.0,
		lS, kFBAttachBottom, "LabelExportRate", 1.0,
		lW, kFBAttachNone, "", 1.0,
		lH, kFBAttachNone, "", 1.0);

	mLayoutSetup.AddRegion("LookAtRootObject", "LookAtRootObject",
		lS, kFBAttachLeft, "", 1.0,
		lS, kFBAttachBottom, "PairName", 1.0,
		lW, kFBAttachNone, "", 1.0,
		lH, kFBAttachNone, "", 1.0);
	mLayoutSetup.AddRegion("LookAtLeftObject", "LookAtLeftObject",
		lS, kFBAttachRight, "LookAtRootObject", 1.0,
		lS, kFBAttachBottom, "PairName", 1.0,
		lW, kFBAttachNone, "", 1.0,
		lH, kFBAttachNone, "", 1.0);
	mLayoutSetup.AddRegion("LookAtRightObject", "LookAtRightObject",
		lS, kFBAttachRight, "LookAtLeftObject", 1.0,
		lS, kFBAttachBottom, "PairName", 1.0,
		lW, kFBAttachNone, "", 1.0,
		lH, kFBAttachNone, "", 1.0);

	mLayoutSetup.AddRegion("AutoLocateLookAt", "AutoLocateLookAt",
		lS, kFBAttachLeft, "", 1.0,
		lS, kFBAttachBottom, "LookAtRootObject", 1.0,
		lW, kFBAttachNone, "", 1.0,
		lH, kFBAttachNone, "", 1.0);

	mLayoutSetup.AddRegion("ImportTarget", "ImportTarget",
		lS, kFBAttachLeft, "", 1.0,
		lS, kFBAttachBottom, "AutoLocateLookAt", 1.0,
		lW, kFBAttachNone, "", 1.0,
		lH, kFBAttachNone, "", 1.0);
	mLayoutSetup.AddRegion("ExportTarget", "ExportTarget",
		lS, kFBAttachRight, "ImportTarget", 1.0,
		lS, kFBAttachBottom, "AutoLocateLookAt", 1.0,
		lW, kFBAttachNone, "", 1.0,
		lH, kFBAttachNone, "", 1.0);

	mLayoutSetup.AddRegion("SaveSetup", "SaveSetup",
		lS, kFBAttachLeft, "", 1.0,
		lS, kFBAttachBottom, "ImportTarget", 1.0,
		lW, kFBAttachNone, "", 1.0,
		lH, kFBAttachNone, "", 1.0);

	// additional functions

	mLayoutSetup.AddRegion("SelectTarget", "SelectTarget",
		lS, kFBAttachLeft, "", 1.00,
		lS, kFBAttachBottom, "SaveSetup", 1.00,
		0.5 * lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, NULL, 1.00);

	mLayoutSetup.AddRegion("ButtonSwitchPipeline", "ButtonSwitchPipeline",
		lS, kFBAttachRight, "SelectTarget", 1.0,
		lS, kFBAttachBottom, "SaveSetup", 1.0,
		0.5 * lW, kFBAttachNone, "", 1.0,
		lH, kFBAttachNone, "", 1.0);

	mLayoutSetup.AddRegion("SwitchFastIdle", "SwitchFastIdle",
		lS, kFBAttachLeft, "", 1.00,
		lS, kFBAttachBottom, "ButtonSwitchPipeline", 1.00,
		0.33 * lW, kFBAttachNone, NULL, 1.0,
		lH, kFBAttachNone, NULL, 1.00);
	mLayoutSetup.AddRegion("SwitchFastIdleOn", "SwitchFastIdleOn",
		lS, kFBAttachRight, "SwitchFastIdle", 1.00,
		lS, kFBAttachBottom, "ButtonSwitchPipeline", 1.00,
		0.33 * lW, kFBAttachNone, NULL, 1.0,
		lH, kFBAttachNone, NULL, 1.00);
	mLayoutSetup.AddRegion("SwitchFastIdleOff", "SwitchFastIdleOff",
		lS, kFBAttachRight, "SwitchFastIdleOn", 1.00,
		lS, kFBAttachBottom, "ButtonSwitchPipeline", 1.00,
		0.33 * lW, kFBAttachNone, NULL, 1.0,
		lH, kFBAttachNone, NULL, 1.00);


	// Add regions (serial)
	mLayoutSetup.AddRegion("LayoutRegion", "LayoutRegion",
		lS, kFBAttachLeft, "", 1.00,
		lS, kFBAttachTop, "", 1.00,
		lW, kFBAttachNone, NULL, 1.00,
		lH * 7, kFBAttachNone, NULL, 1.00);
	

	// Assign regions
	
	mLayoutSetup.SetControl("LabelExportRate",		mLabelExportRate		);
	mLayoutSetup.SetControl("EditNumberExportRate",	mEditNumberExportRate	);
	mLayoutSetup.SetControl("PairName", mEditPairName);
	mLayoutSetup.SetControl("JointSet", mEditJointSet);

	mLayoutSetup.SetControl("LookAtRootObject", mEditLookAtRootObject);
	mLayoutSetup.SetControl("LookAtLeftObject", mEditLookAtLeftObject);
	mLayoutSetup.SetControl("LookAtRightObject", mEditLookAtRightObject);

	mLayoutSetup.SetControl("ImportTarget", mEditImportTarget);
	mLayoutSetup.SetControl("ExportTarget", mEditExportTarget);
	mLayoutSetup.SetControl("SaveSetup", mEditSaveRotationSetup);

	mLayoutSetup.SetControl("SelectTarget", mButtonSelectTarget);
	mLayoutSetup.SetControl("ButtonSwitchPipeline", mButtonSwitchPipeline);
	mLayoutSetup.SetControl("SwitchFastIdle", mLabelFastIdle);
	mLayoutSetup.SetControl("SwitchFastIdleOn", mButtonSwitchOnFastIdle);
	mLayoutSetup.SetControl("SwitchFastIdleOff", mButtonSwitchOffFastIdle);

	mLayoutSetup.SetBorder("LayoutRegionSerial", kFBEmbossBorder, false, true, 2, 1, 90.0, 0);
}


/************************************************
 *	Create the information layout.
 ************************************************/
void CServerDeviceLayout::UICreateLayout1()
{
	int lS, lW, lH;		// space, width, height.
	lS = 5;
	lW = 200;
	lH = 25;

	// Add regions
	mLayoutInformation.AddRegion (	"LabelInformation",	"LabelInformation",
													lS,		kFBAttachLeft,		"",		1.00,
													lS,		kFBAttachTop,		"",		1.00,
													lW,		kFBAttachNone,		NULL,	1.00,
													lH,		kFBAttachNone,		NULL,	1.00 );
	mLayoutInformation.AddRegion("Export", "Export",
		lS, kFBAttachLeft, "", 1.00,
		lS, kFBAttachBottom, "LabelInformation", 1.00,
		lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, NULL, 1.00);
	mLayoutInformation.AddRegion("Import", "Import",
		lS, kFBAttachLeft, "", 1.00,
		lS, kFBAttachBottom, "Export", 1.00,
		lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, NULL, 1.00);

	// Assign regions
	mLayoutInformation.SetControl(	"LabelInformation",	mLabelInformation );
	mLayoutInformation.SetControl("Export", mButtonPresetExport);
	mLayoutInformation.SetControl("Import", mButtonPresetImport);
}


/************************************************
 *	Configure the UI elements.
 ************************************************/
void CServerDeviceLayout::UIConfigure()
{
	SetBorder ("MainLayout", kFBStandardBorder, false,true, 1, 0,90,0);

	mTabPanel.Items.SetString("Setup~Information");
	mTabPanel.OnChange.Add( this, (FBCallback)&CServerDeviceLayout::EventTabPanelChange );

	UIConfigureLayout0();
	UIConfigureLayout1();
}


/************************************************
 *	Configure the setup layout.
 ************************************************/
void CServerDeviceLayout::UIConfigureLayout0()
{
	// Configure elements
	
	mLabelExportRate.Caption		= "Export Rate :";
	mEditNumberExportRate.Min		= 1.0;
	mEditNumberExportRate.Max		= 100.0;

	mEditPairName.Caption = "Pair Name:";
	mEditPairName.Property = &m_Device->PairName;

	mEditLookAtRootObject.Caption = "LookAt Model:";
	mEditLookAtRootObject.Property = &m_Device->LookAtRoot;

	mEditLookAtLeftObject.Caption = "LookAt Left:";
	mEditLookAtLeftObject.Property = &m_Device->LookAtLeft;

	mEditLookAtRightObject.Caption = "LookAt Right:";
	mEditLookAtRightObject.Property = &m_Device->LookAtRight;

	mEditImportTarget.Caption = "Import Target";
	mEditImportTarget.Property = &m_Device->ImportTarget;

	mEditExportTarget.Caption = "Export Target";
	mEditExportTarget.Property = &m_Device->ExportTarget;

	mEditSaveRotationSetup.Caption = "Save";
	mEditSaveRotationSetup.Property = &m_Device->SaveRotationSetup;

	// Add callbacks
	mEditNumberExportRate.OnChange.Add	( this, (FBCallback)&CServerDeviceLayout::EventEditNumberExportRateChange  );

	// additional functions
	mButtonSelectTarget.Caption = "Select Target";
	mButtonSelectTarget.OnClick.Add(this, (FBCallback)&CServerDeviceLayout::EventSelectTargetClick);

	mButtonSwitchPipeline.Caption = "Switch Pipeline";
	mButtonSwitchPipeline.OnClick.Add(this, (FBCallback)&CServerDeviceLayout::EventButtonSwitchPipelineClick);

	mLabelFastIdle.Caption = "Switch Fast Idle";

	mButtonSwitchOnFastIdle.Caption = "Switch On";
	mButtonSwitchOnFastIdle.OnClick.Add(this, (FBCallback)&CServerDeviceLayout::EventSwitchOnFastIdleClick);
	mButtonSwitchOffFastIdle.Caption = "Switch Off";
	mButtonSwitchOffFastIdle.OnClick.Add(this, (FBCallback)&CServerDeviceLayout::EventSwitchOffFastIdleClick);
}

/************************************************
 *	Configure the information layout.
 ************************************************/
void CServerDeviceLayout::UIConfigureLayout1()
{
	mLabelInformation.Caption = "Live Bridge Server device v2...\n Avalanche Studios 2020";

	mButtonPresetExport.Caption = "Joint Set Export";
	mButtonPresetExport.OnClick.Add(this, (FBCallback)&CServerDeviceLayout::EventButtonPresetExportClick);

	mButtonPresetImport.Caption = "Joint Set Import";
	mButtonPresetImport.OnClick.Add(this, (FBCallback)&CServerDeviceLayout::EventButtonPresetImportClick);
}


/************************************************
 *	Refresh the UI.
 ************************************************/
void CServerDeviceLayout::UIRefresh()
{
}


/************************************************
 *	Reset the UI components from the device.
 ************************************************/
void CServerDeviceLayout::UIReset()
{
	mEditNumberExportRate.Value		= m_Device->GetExportRate();
	
}

void CServerDeviceLayout::SwitchFastIdle(const int value)
{
	FBSystem		lSystem;
	FBScene*		pScene = lSystem.Scene;

	for (int i = 0, count = pScene->Components.GetCount(); i < count; ++i)
	{
		FBComponent* comp = pScene->Components[i];
		if (0 == strcmp("Application", comp->Name))
		{
			FBProperty* prop = nullptr;

			prop = comp->PropertyList.Find("FastIdleTimerInterval");
			if (nullptr != prop)
				prop->SetInt(50);

			prop = comp->PropertyList.Find("FastIdleOnDeactivate");
			if (nullptr != prop)
			{
				prop->SetInt(value);
				break;
			}
		}
	}
}

void CServerDeviceLayout::EventSwitchOnFastIdleClick(HISender pSender, HKEvent pEvent)
{
	SwitchFastIdle(1);
}

void CServerDeviceLayout::EventSwitchOffFastIdleClick(HISender pSender, HKEvent pEvent)
{
	SwitchFastIdle(0);
}

void CServerDeviceLayout::EventButtonSwitchPipelineClick(HISender pSender, HKEvent pEvent)
{
	FBEvaluateManager::TheOne().ParallelEvaluation = false;
}

void CServerDeviceLayout::EventSelectTargetClick(HISender pSender, HKEvent pEvent)
{
	if (m_Device && m_Device->LookAtRoot.GetCount() > 0)
	{
		m_Device->LookAtRoot.GetAt(0)->Selected = true;
	}
}

/************************************************
 *	Filename change callback.
 ************************************************/
void CServerDeviceLayout::EventEditExportFilenameChange( HISender pSender, HKEvent pEvent )
{
	if( m_Device->Online )
	{
		m_Device->DeviceSendCommand( FBDevice::kOpStop );
	}
	
}


/************************************************
 *	Export rate change callback.
 ************************************************/
void CServerDeviceLayout::EventEditNumberExportRateChange( HISender pSender, HKEvent pEvent )
{
	if( m_Device->Online )
	{
		m_Device->DeviceSendCommand( FBDevice::kOpStop );
	}
	m_Device->SetExportRate( mEditNumberExportRate.Value );
	mEditNumberExportRate.Value = m_Device->GetExportRate();
}


/************************************************
 *	Tab panel change callback.
 ************************************************/
void CServerDeviceLayout::EventTabPanelChange( HISender pSender, HKEvent pEvent )
{
	switch( mTabPanel.ItemIndex )
	{
		case 0:	SetControl("MainLayout", mLayoutSetup	);	break;
		case 1:	SetControl("MainLayout", mLayoutInformation	);	break;
	}
}


/************************************************
 *	Device status change callback.
 ************************************************/
void CServerDeviceLayout::EventDeviceStatusChange( HISender pSender, HKEvent pEvent )
{
	UIReset();
}


/************************************************
 *	UI Idle callback.
 ************************************************/
void CServerDeviceLayout::EventUIIdle( HISender pSender, HKEvent pEvent )
{
	if( m_Device->Online )
	{
		UIRefresh();
	}
}

void CServerDeviceLayout::EventButtonPresetExportClick(HISender pSender, HKEvent pEvent)
{
	if (m_Device)
	{
		FBFilePopup popup;
		popup.Filter = "*.xml";
		popup.Style = kFBFilePopupSave;

		if (popup.Execute())
		{
			m_Device->ExportJointSet(popup.FullFilename);
		}
	}
}

void CServerDeviceLayout::EventButtonPresetImportClick(HISender pSender, HKEvent pEvent)
{
	if (m_Device)
	{
		FBFilePopup popup;
		popup.Filter = "*.xml";
		popup.Style = kFBFilePopupOpen;

		if (popup.Execute())
		{
			m_Device->ImportJointSet(popup.FullFilename);
		}
	}
}