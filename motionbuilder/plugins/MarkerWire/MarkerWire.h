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

/**	\file	MarkerWire.h

 Implementation of a custom marker model with linked models property

*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration define
#define MARKERWIRE__CLASSNAME	CMarkerWire
#define MARKERWIRE__CLASSSTR	"CMarkerWire"
#define MARKERWIRE__DESCSTR	    "Custom Marker Model with a displable wire connection"

/**	Marker Wire.
*/
class CMarkerWire : public FBModelMarker
{
	//--- FiLMBOX class declaration.
	FBStorableClassDeclare(CMarkerWire, FBModelMarker );

public:
	CMarkerWire(const char *pName, HIObject pObject= NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate() override;		//!< FiLMBOX Creation function.
	virtual void FBDestroy() override;		//!< FiLMBOX Destruction function.

	virtual bool HasCustomDisplay() override { return false; }

	/** Custom display function called when HasCustomDisplay returns true
	*/
	virtual void CustomModelDisplay(FBCamera* pCamera, FBModelShadingMode pShadingMode, 
									FBModelRenderPass pRenderPass, float pPickingAreaWidth, 
									float pPickingAreaHeight) override;

    /** Override from FBPlug 
    */
    
	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;

    /** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return false; }

public:

	FBPropertyListObject		LinkToObjects;

protected:

	FBSystem					mSystem;

    int mPickedSubItem;

	void DrawCube(const bool render_mode, const double sizex, const double sizey, const double sizez);
	void DrawHardCross(const bool render_mode, const double size);
};
