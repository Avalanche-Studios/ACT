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
/**	\file	client_layout.cxx
*	Definition of a layout class for a client input device.
*
*/

//--- Class declarations
#include "client_device.h"
#include "client_layout.h"

#define CLIENTDEVICE__LAYOUT		CClientDeviceLayout

//--- FiLMBOX implementation and registration
FBDeviceLayoutImplementation(	CLIENTDEVICE__LAYOUT		);
FBRegisterDeviceLayout		(	CLIENTDEVICE__LAYOUT,
								CLIENTDEVICE__CLASSSTR,
								FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX constructor.
 ************************************************/
bool CClientDeviceLayout::FBCreate()
{
	// Get a handle on the device.
	m_Device = ((CClientDevice *)(FBDevice *)Device);

	// Create/configure UI
	UICreate	();
	UIConfigure	();
	UIReset		();

	// Add device & system callbacks
	m_Device->OnStatusChange.Add	( this,(FBCallback)&CClientDeviceLayout::EventDeviceStatusChange  );
	m_System.OnUIIdle.Add		( this,(FBCallback)&CClientDeviceLayout::EventUIIdle              );

	return true;
}


/************************************************
 *	FiLMBOX destructor.
 ************************************************/
void CClientDeviceLayout::FBDestroy()
{
	// Remove device & system callbacks
	m_System.OnUIIdle.Remove		( this,(FBCallback)&CClientDeviceLayout::EventUIIdle              );
	m_Device->OnStatusChange.Remove	( this,(FBCallback)&CClientDeviceLayout::EventDeviceStatusChange  );
}


/************************************************
 *	Create the UI.
 ************************************************/
void CClientDeviceLayout::UICreate()
{
	int lS, lH;		// space, height
	lS = 4;
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
	SetControl	( "TabPanel",	mTabPanel		);
	SetControl	( "MainLayout",	mLayoutMarkers	);

	// Create sub layouts
	UICreateLayout0();
	UICreateLayout1();
	UICreateLayout2();
}


/************************************************
 *	Create the markers layout.
 ************************************************/
void CClientDeviceLayout::UICreateLayout0()
{
	// Add regions
	mLayoutMarkers.AddRegion( "SpreadMarkers", "SpreadMarkers",
													0,		kFBAttachLeft,		"",		1.00,
													0,		kFBAttachTop,		"",		1.00,
													0,		kFBAttachRight,		"",		1.00,
													0,		kFBAttachBottom,	"",		1.00 );

	// Assign regions
	mLayoutMarkers.SetControl( "SpreadMarkers", mSpreadMarkers );
}

/************************************************
*	Create the setup layout.
************************************************/
void CClientDeviceLayout::UICreateLayout1()
{
	int lS, lW, lH;		// space, width, height.
	lS = 6;
	lW = 300;
	lH = 20;

	// Add regions
	mLayoutSetup.AddRegion("PairName", "PairName",
		lS, kFBAttachLeft, "", 1.00,
		lS, kFBAttachTop, "", 1.00,
		lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, NULL, 1.00);

	mLayoutSetup.AddRegion("SyncTimeWithEditorClip", "SyncTimeWithEditorClip",
		lS, kFBAttachLeft, "", 1.00,
		lS, kFBAttachBottom, "PairName", 1.00,
		lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, NULL, 1.00);
	mLayoutSetup.AddRegion("StoryClipIndex", "StoryClipIndex",
		lS, kFBAttachRight, "SyncTimeWithEditorClip", 1.00,
		lS, kFBAttachBottom, "PairName", 1.00,
		lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, NULL, 1.00);

	mLayoutSetup.AddRegion("TargetObject", "TargetObject",
		lS, kFBAttachLeft, "", 1.00,
		lS, kFBAttachBottom, "SyncTimeWithEditorClip", 1.00,
		lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, NULL, 1.00);
	mLayoutSetup.AddRegion("LeftObject", "LeftObject",
		lS, kFBAttachRight, "TargetObject", 1.00,
		lS, kFBAttachBottom, "SyncTimeWithEditorClip", 1.00,
		lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, NULL, 1.00);
	mLayoutSetup.AddRegion("RightObject", "RightObject",
		lS, kFBAttachRight, "LeftObject", 1.00,
		lS, kFBAttachBottom, "SyncTimeWithEditorClip", 1.00,
		lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, NULL, 1.00);

	mLayoutSetup.AddRegion("MakeTarget", "MakeTarget",
		lS, kFBAttachLeft, "", 1.00,
		lS, kFBAttachBottom, "TargetObject", 1.00,
		lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, NULL, 1.00);

	mLayoutSetup.AddRegion("ImportTarget", "ImportTarget",
		lS, kFBAttachLeft, "", 1.00,
		lS, kFBAttachBottom, "MakeTarget", 1.00,
		lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, NULL, 1.00);
	mLayoutSetup.AddRegion("ExportTarget", "ExportTarget",
		lS, kFBAttachRight, "ImportTarget", 1.00,
		lS, kFBAttachBottom, "MakeTarget", 1.00,
		lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, NULL, 1.00);

	mLayoutSetup.AddRegion("LoadRotationSetup", "LoadRotationSetup",
		lS, kFBAttachLeft, "", 1.00,
		lS, kFBAttachBottom, "ImportTarget", 1.00,
		lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, NULL, 1.00);
	
	// additional functions

	mLayoutSetup.AddRegion("SelectTarget", "SelectTarget",
		lS, kFBAttachLeft, "", 1.00,
		lS, kFBAttachBottom, "LoadRotationSetup", 1.00,
		0.5 * lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, NULL, 1.00);

	mLayoutSetup.AddRegion("ButtonSwitchPipeline", "ButtonSwitchPipeline",
		lS, kFBAttachRight, "SelectTarget", 1.0,
		lS, kFBAttachBottom, "LoadRotationSetup", 1.0,
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

	// Assign regions
	mLayoutSetup.SetControl("PairName", mEditPairName);
	mLayoutSetup.SetControl("SyncTimeWithEditorClip", mEditSyncTimeWithStoryClip);
	mLayoutSetup.SetControl("StoryClipIndex", mEditStoryClipIndex);
	mLayoutSetup.SetControl("TargetObject", mEditRootObject);
	mLayoutSetup.SetControl("LeftObject", mEditLeftObject);
	mLayoutSetup.SetControl("RightObject", mEditRightObject);

	mLayoutSetup.SetControl("ImportTarget", mEditImportTarget);
	mLayoutSetup.SetControl("ExportTarget", mEditExportTarget);
	mLayoutSetup.SetControl("LoadRotationSetup", mEditLoadRotationSetup);

	mLayoutSetup.SetControl("SelectTarget", mButtonSelectTarget);
	mLayoutSetup.SetControl("ButtonSwitchPipeline", mButtonSwitchPipeline);
	mLayoutSetup.SetControl("SwitchFastIdle", mLabelFastIdle);
	mLayoutSetup.SetControl("SwitchFastIdleOn", mButtonSwitchOnFastIdle);
	mLayoutSetup.SetControl("SwitchFastIdleOff", mButtonSwitchOffFastIdle);
}

/************************************************
 *	Create the information layout.
 ************************************************/
void CClientDeviceLayout::UICreateLayout2()
{
	int lS, lW, lH;		// space, width, height.
	lS = 4;
	lW = 300;
	lH = 20;

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
 *	Configure the UI elements (main layout).
 ************************************************/
void CClientDeviceLayout::UIConfigure()
{
	SetBorder ("MainLayout", kFBStandardBorder, false,true, 1, 0,90,0);

	mTabPanel.Items.SetString("Markers~Setup~Information");
	mTabPanel.OnChange.Add( this, (FBCallback)&CClientDeviceLayout::EventTabPanelChange );

	UIConfigureLayout0();
	UIConfigureLayout1();
	UIConfigureLayout2();
}


/************************************************
 *	Configure the UI elements (marker layout).
 ************************************************/
void CClientDeviceLayout::UIConfigureLayout0()
{
}

/************************************************
*	Configure the UI elements (setup layout).
************************************************/
void CClientDeviceLayout::UIConfigureLayout1()
{
	
	mEditPairName.Caption = "Pair Name";
	mEditPairName.Property = &m_Device->PairName;

	mEditSyncTimeWithStoryClip.Caption = "Sync Time With Story";
	mEditSyncTimeWithStoryClip.Property = &m_Device->SyncTimeWithStoryClip;
	
	mEditStoryClipIndex.Caption = "Story Clip Index";
	mEditStoryClipIndex.Property = &m_Device->StoryClipIndex;

	mEditRootObject.Caption = "Root Object";
	mEditRootObject.Property = &m_Device->LookAtRoot;
	
	mEditLeftObject.Caption = "Left Object";
	mEditLeftObject.Property = &m_Device->LookAtLeft;
	mEditRightObject.Caption = "Right Object";
	mEditRightObject.Property = &m_Device->LookAtRight;
	
	mEditImportTarget.Caption = "Import";
	mEditImportTarget.Property = &m_Device->ImportTarget;
	mEditExportTarget.Caption = "Export";
	mEditExportTarget.Property = &m_Device->ExportTarget;

	mEditLoadRotationSetup.Caption = "Load";
	mEditLoadRotationSetup.Property = &m_Device->LoadRotationSetup;

	// additional functions

	mButtonSelectTarget.Caption = "Select Target";
	mButtonSelectTarget.OnClick.Add(this, (FBCallback)&CClientDeviceLayout::EventSelectTargetClick);

	mButtonSwitchPipeline.Caption = "Switch Pipeline";
	mButtonSwitchPipeline.OnClick.Add(this, (FBCallback)&CClientDeviceLayout::EventButtonSwitchPipelineClick);

	mLabelFastIdle.Caption = "Switch Fast Idle";

	mButtonSwitchOnFastIdle.Caption = "Switch On";
	mButtonSwitchOnFastIdle.OnClick.Add(this, (FBCallback)&CClientDeviceLayout::EventSwitchOnFastIdleClick);
	mButtonSwitchOffFastIdle.Caption = "Switch Off";
	mButtonSwitchOffFastIdle.OnClick.Add(this, (FBCallback)&CClientDeviceLayout::EventSwitchOffFastIdleClick);
}

/************************************************
 *	Configure the UI elements (information layout).
 ************************************************/
void CClientDeviceLayout::UIConfigureLayout2()
{
	mLabelInformation.Caption = "Client Animation Bridge Device... Avalanche Studios 2020";

	mButtonPresetExport.Caption = "Joint Set Export";
	mButtonPresetExport.OnClick.Add(this, (FBCallback)&CClientDeviceLayout::EventButtonPresetExportClick);

	mButtonPresetImport.Caption = "Joint Set Import";
	mButtonPresetImport.OnClick.Add(this, (FBCallback)&CClientDeviceLayout::EventButtonPresetImportClick);
}


/************************************************
 *	Refresh the UI.
 ************************************************/
void CClientDeviceLayout::UIRefresh()
{
	UIRefreshSpreadSheet();
}


/************************************************
 *	Refresh the spreadsheet content.
 ************************************************/
void CClientDeviceLayout::UIRefreshSpreadSheet()
{
  	for(int i=0; i<m_Device->GetChannelCount();i++)
	{
		mSpreadMarkers.SetCell( i, 0, m_Device->GetDataTX(i) );
		mSpreadMarkers.SetCell( i, 1, m_Device->GetDataTY(i) );
		mSpreadMarkers.SetCell( i, 2, m_Device->GetDataTZ(i) );
		mSpreadMarkers.SetCell( i, 3, m_Device->GetDataRX(i) );
		mSpreadMarkers.SetCell( i, 4, m_Device->GetDataRY(i) );
		mSpreadMarkers.SetCell( i, 5, m_Device->GetDataRZ(i) );
	}
}


/************************************************
 *	Reset the UI values from device.
 ************************************************/
void CClientDeviceLayout::UIReset()
{
	mTabPanel.ItemIndex = 1;
	EventTabPanelChange(nullptr, nullptr);
	UIResetSpreadSheet();
}


/************************************************
 *	Re-build the spreadsheet.
 ************************************************/
void CClientDeviceLayout::UIResetSpreadSheet()
{
	int i;

	mSpreadMarkers.Clear();

	// Spreadsheet
	int lColumnIndex = -1;
	mSpreadMarkers.GetColumn(lColumnIndex).Width = 200;

	// column 0: Translation X
	mSpreadMarkers.ColumnAdd ("PosX");
	lColumnIndex++;
	mSpreadMarkers.GetColumn(lColumnIndex).Width = 60;
	mSpreadMarkers.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

	// column 1: Translation Y
	mSpreadMarkers.ColumnAdd ("PosY");
	lColumnIndex++;
	mSpreadMarkers.GetColumn(lColumnIndex).Width = 60;
 	mSpreadMarkers.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

 	// column 2: Translation Z
	mSpreadMarkers.ColumnAdd ("PosZ");
	lColumnIndex++;
	mSpreadMarkers.GetColumn(lColumnIndex).Width = 60;
	mSpreadMarkers.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

	// column 3: Rotation X
	mSpreadMarkers.ColumnAdd ("RotX");
	lColumnIndex++;
	mSpreadMarkers.GetColumn(lColumnIndex).Width = 60;
	mSpreadMarkers.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

	// column 4: Rotation Y
	mSpreadMarkers.ColumnAdd ("RotY");
	lColumnIndex++;
	mSpreadMarkers.GetColumn(lColumnIndex).Width = 60;
 	mSpreadMarkers.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

 	// column 5: Rotation Z
	mSpreadMarkers.ColumnAdd ("RotZ");
	lColumnIndex++;
	mSpreadMarkers.GetColumn(lColumnIndex).Width = 60;
	mSpreadMarkers.GetColumn(lColumnIndex).Style = kFBCellStyleDouble;

	for (i=0;i<m_Device->GetChannelCount();i++)
	{
 		mSpreadMarkers.RowAdd( m_Device->GetChannelName(i), i );
	 	mSpreadMarkers.GetCell(i,lColumnIndex).ReadOnly = true;
	}
}


/************************************************
 *	Tab panel change callback.
 ************************************************/
void CClientDeviceLayout::EventTabPanelChange( HISender pSender, HKEvent pEvent )
{
	switch( mTabPanel.ItemIndex )
	{
		case 0:	SetControl("MainLayout", mLayoutMarkers			);	break;
		case 1:	SetControl("MainLayout", mLayoutSetup		);	break;
		case 2:	SetControl("MainLayout", mLayoutInformation);	break;
	}
}


/************************************************
 *	Device status change callback.
 ************************************************/
void CClientDeviceLayout::EventDeviceStatusChange( HISender pSender, HKEvent pEvent )
{
	UIReset();
}


/************************************************
 *	UI Idle callback.
 ************************************************/
void CClientDeviceLayout::EventUIIdle( HISender pSender, HKEvent pEvent )
{
	if( m_Device->Online )
	{
		UIRefresh();
	}
}

void CClientDeviceLayout::SwitchFastIdle(const int value)
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

void CClientDeviceLayout::EventSwitchOnFastIdleClick(HISender pSender, HKEvent pEvent)
{
	SwitchFastIdle(1);
}

void CClientDeviceLayout::EventSwitchOffFastIdleClick(HISender pSender, HKEvent pEvent)
{
	SwitchFastIdle(0);
}

void CClientDeviceLayout::EventButtonSwitchPipelineClick(HISender pSender, HKEvent pEvent)
{
	FBEvaluateManager::TheOne().ParallelEvaluation = false;
}

void CClientDeviceLayout::EventSelectTargetClick(HISender pSender, HKEvent pEvent)
{
	if (m_Device && m_Device->LookAtRoot.GetCount() > 0)
	{
		m_Device->LookAtRoot.GetAt(0)->Selected = true;
	}
}

void CClientDeviceLayout::EventButtonPresetExportClick(HISender pSender, HKEvent pEvent)
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

void CClientDeviceLayout::EventButtonPresetImportClick(HISender pSender, HKEvent pEvent)
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
