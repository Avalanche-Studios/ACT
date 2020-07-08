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

/**	\file	MarkerWire.cxx

 Implementation of a custom marker model with linked models property

*/

//--- Class declaration
#include "MarkerWire.h"

#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include <windows.h>
#include <GL/GL.h>

#include "MarkerWireManip.h"

/** Element Class implementation. (Asset system)
*	This should be placed in the source code file for a class.
*/
#define FBElementClassImplementationCustomMarker(ClassName, AssetName, IconFileName)\
	HIObject RegisterElement##ClassName##Create(HIObject /*pOwner*/, const char* pName, void* /*pData*/){\
		ClassName* Class = new ClassName(pName);\
		Class->mAllocated = true;\
		if( Class->FBCreate() ){\
            __FBRemoveModelFromScene( Class->GetHIObject() ); /* Hack in MoBu2013, we shouldn't add object to the scene/entity automatically*/\
			return Class->GetHIObject();\
		} else {\
			delete Class;\
			return NULL;}}\
	FBLibraryModule(ClassName##Element){\
		FBRegisterObject(ClassName##R2, "Browsing/Templates/Elements", AssetName, "", RegisterElement##ClassName##Create, true, IconFileName);}


FBClassImplementation( CMarkerWire );								                //!< Register class
FBStorableCustomModelImplementation( CMarkerWire, MARKERWIRE__DESCSTR );			//!< Register to the store/retrieve system
FBElementClassImplementationCustomMarker( CMarkerWire, "Marker Wire", "\\browsing\\template_model_marker.png" );	                //!< Register to the asset system

/************************************************
*	Constructor.
************************************************/
CMarkerWire::CMarkerWire( const char* pName, HIObject pObject )
    : FBModelMarker( pName, pObject )
{
    FBClassInit;

	FBPropertyPublish(this, LinkToObjects, "Link To Objects", nullptr, nullptr);
	LinkToObjects.SetSingleConnect(false);
	LinkToObjects.SetFilter(FBModel::GetInternalClassId());

	CMarkerWireManip::RegisterMarkerModel(this);
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool CMarkerWire::FBCreate()
{
    ShadingMode = kFBModelShadingTexture;
    Size = 100.0;
    Length = 1.0;
    ResLevel = kFBMarkerMediumResolution;
    Look = kFBMarkerLookCube;
    Type = kFBMarkerTypeStandard;
    Show = true;
    mPickedSubItem = -1;

    return true;
}

/************************************************
*	FiLMBOX Destructor.
************************************************/
void CMarkerWire::FBDestroy()
{
	CMarkerWireManip::UnRegisterMarkerModel(this);

    ParentClass::FBDestroy();
}

/** Custom display
*/
void CMarkerWire::CustomModelDisplay( FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight)
{
	
    FBMatrix		 MatrixView;
    FBMatrix		 MatrixProjection;
	
	// BUG: this method give a mess with display mode between panes
    FBViewingOptions* lViewingOptions = mSystem.Renderer->GetViewingOptions();
	bool lIsSelectBufferPicking = lViewingOptions->IsInSelectionBufferPicking();
	bool lIsColorBufferPicking = lViewingOptions->IsInColorBufferPicking();

	double lScale;
	Size.GetData(&lScale, sizeof(double));
	lScale *= 0.01;
	const bool render_mode = (!lIsSelectBufferPicking && !lIsColorBufferPicking);
	
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        {
			FBMatrix tm;
			GetMatrix(tm, kModelTransformation, true);
			glMultMatrixd(tm);

            if (render_mode) 
            { 
                if ((bool)Selected) 
                {
                    if (mPickedSubItem == 0)        //!< First subitem picked, RED
                        glColor3d(1.0,0.0,0.0);
                    else if (mPickedSubItem == 1)   //!< Second subitem picked, GREEN
                        glColor3d(0.0, 1.0, 0.0);
                    else
                        glColor3d(0.2, 0.8, 0.2);   //!< Otherwise, BLUE.
                } 
                else 
                {
					FBColor color = Color;
                    glColor3dv(color);
                }
            }

			const FBMarkerLook look = Look;

			switch (look)
			{
			case kFBMarkerLookCube:
				DrawCube(render_mode, lScale, lScale, lScale);
				break;
			case kFBMarkerLookHardCross:
				DrawHardCross(render_mode, lScale);
				break;
			default:

				glLineWidth(3.0f);    //!< Draw line wider to easy picking.

				// Draw with Model's Unique ColorID  (or selection name (-1) set by Mobu Internally). 
				glBegin(GL_LINES);
				glVertex3d(lScale, 0.0, 0.0);
				glVertex3d(-lScale, 0.0, 0.0);

				glVertex3d(0.0, lScale, 0.0);
				glVertex3d(0.0, -lScale, 0.0);

				glVertex3d(0.0, 0.0, lScale);
				glVertex3d(0.0, 0.0, -lScale);
				glEnd();
			}

        }
        glPopMatrix();

		// draw links between marker and link nodes
		
		if (LinkToObjects.GetCount() > 0 && render_mode)
		{
			glPushMatrix();

			glColor3d(0.5, 0.5, 0.5);
			glLineWidth(1.0f);    //!< Draw line wider to easy picking.
			glLineStipple(1, 0x1C47); // 0x5557);

			glEnable(GL_LINE_STIPPLE);

			FBVector3d v1, v2;
			GetVector(v1);

			for (int i = 0; i < LinkToObjects.GetCount(); ++i)
			{
				if (FBIS(LinkToObjects[i], FBModel))
				{
					FBModel* pModel = (FBModel*)LinkToObjects[i];
					pModel->GetVector(v2);

					glBegin(GL_LINES);
					glVertex3dv(v1);
					glVertex3dv(v2);
					glEnd();
				}
			}

			glDisable(GL_LINE_STIPPLE);
			glPopMatrix();
		}

		glLineWidth(1.0f);
    }
}

bool CMarkerWire::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    return true;
}

bool CMarkerWire::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    return true;
}

void CMarkerWire::DrawCube(const bool render_mode, const double sizex, const double sizey, const double sizez)
{
	#define number_of_vertices 24
	#define number_of_quads 6

	static double vertices[number_of_vertices][4] = {
		// front
		{ 1.0, 1.0, 1.0, 1.0 },
		{ -1.0, 1.0, 1.0, 1.0 },
		{ -1.0, -1.0, 1.0, 1.0 },
		{ 1.0, -1.0, 1.0, 1.0 },
		// back
		{ 1.0, 1.0, -1.0, 1.0 },
		{ -1.0, 1.0, -1.0, 1.0 },
		{ -1.0, -1.0, -1.0, 1.0 },
		{ 1.0, -1.0, -1.0, 1.0 },
		// top
		{ 1.0, 1.0, 1.0, 1.0 },
		{ -1.0, 1.0, 1.0, 1.0 },
		{ -1.0, 1.0, -1.0, 1.0 },
		{ 1.0, 1.0, -1.0, 1.0 },
		// bottom
		{ 1.0, -1.0, 1.0, 1.0 },
		{ -1.0, -1.0, 1.0, 1.0 },
		{ -1.0, -1.0, -1.0, 1.0 },
		{ 1.0, -1.0, -1.0, 1.0 },
		// left
		{ 1.0, 1.0, 1.0, 1.0 },
		{ 1.0, -1.0, 1.0, 1.0 },
		{ 1.0, -1.0, -1.0, 1.0 },
		{ 1.0, 1.0, -1.0, 1.0 },
		// right
		{ -1.0, 1.0, 1.0, 1.0 },
		{ -1.0, -1.0, 1.0, 1.0 },
		{ -1.0, -1.0, -1.0, 1.0 },
		{ -1.0, 1.0, -1.0, 1.0 },
	};

	static double normals[number_of_vertices][4] = {
		// front
		{ 0.0, 0.0, 1.0, 1.0 },
		{ 0.0, 0.0, 1.0, 1.0 },
		{ 0.0, 0.0, 1.0, 1.0 },
		{ 0.0, 0.0, 1.0, 1.0 },
		// back
		{ 0.0, 0.0, -1.0, 1.0 },
		{ 0.0, 0.0, -1.0, 1.0 },
		{ 0.0, 0.0, -1.0, 1.0 },
		{ 0.0, 0.0, -1.0, 1.0 },
		// top
		{ 0.0, 1.0, 0.0, 1.0 },
		{ 0.0, 1.0, 0.0, 1.0 },
		{ 0.0, 1.0, 0.0, 1.0 },
		{ 0.0, 1.0, 0.0, 1.0 },
		// bottom
		{ 0.0, -1.0, 0.0, 1.0 },
		{ 0.0, -1.0, 0.0, 1.0 },
		{ 0.0, -1.0, 0.0, 1.0 },
		{ 0.0, -1.0, 0.0, 1.0 },
		// left
		{ 1.0, 0.0, 0.0, 1.0 },
		{ 1.0, 0.0, 0.0, 1.0 },
		{ 1.0, 0.0, 0.0, 1.0 },
		{ 1.0, 0.0, 0.0, 1.0 },
		// right
		{ -1.0, 0.0, 0.0, 1.0 },
		{ -1.0, 0.0, 0.0, 1.0 },
		{ -1.0, 0.0, 0.0, 1.0 },
		{ -1.0, 0.0, 0.0, 1.0 },
	};

	glPushMatrix();

	glScaled(sizex, sizey, sizez);

	if (render_mode)
	{
		glEnable(GL_LIGHTING);
		glDisable(GL_AUTO_NORMAL);
		glEnable(GL_COLOR_MATERIAL);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(4, GL_DOUBLE, GLsizei(0), vertices);
	glNormalPointer(GL_DOUBLE, sizeof(double)*4, normals);

	glDrawArrays(GL_QUADS, 0, GLsizei(number_of_vertices));

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	if (render_mode)
	{
		glDisable(GL_LIGHTING);
	}
	
	glPopMatrix();
}

void CMarkerWire::DrawHardCross(const bool render_mode, const double size)
{
	const double small_size = 0.1 * size;
	DrawCube(render_mode, size, small_size, small_size);
	DrawCube(render_mode, small_size, size, small_size);
	DrawCube(render_mode, small_size, small_size, size);
}