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
// file: boxes.cxx
//

#include <math.h>

//--- Class declaration
#include "boxes.h"

//--- Registration defines
#define BOXSMARTMIX__CLASS		BOXSMARTMIX__CLASSNAME
#define BOXSMARTMIX__NAME		BOXSMARTMIX__CLASSSTR
#define	BOXSMARTMIX__LOCATION	"Avalanche"
#define BOXSMARTMIX__LABEL		"Smart Mix"
#define	BOXSMARTMIX__DESC		"Smart Mix Box"

#define BOXRAYPLANEINTERSECT__CLASS		BOXRAYPLANEINTERSECT__CLASSNAME
#define BOXRAYPLANEINTERSECT__NAME		BOXRAYPLANEINTERSECT__CLASSSTR
#define	BOXRAYPLANEINTERSECT__LOCATION	"Avalanche"
#define BOXRAYPLANEINTERSECT__LABEL		"Ray-Plane Intersect"
#define	BOXRAYPLANEINTERSECT__DESC		"Ray-Plane Intersect Box"

//--- TRANSPORT implementation and registration
FBBoxImplementation	(BOXSMARTMIX__CLASS	);	// Box class name
FBRegisterBox		(	BOXSMARTMIX__NAME,		// Unique name to register box.
						BOXSMARTMIX__CLASS,		// Box class name
						BOXSMARTMIX__LOCATION,	// Box location ('plugins')
						BOXSMARTMIX__LABEL,		// Box label (name of box to display)
						BOXSMARTMIX__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

FBBoxImplementation(BOXRAYPLANEINTERSECT__CLASS);	// Box class name
FBRegisterBox(BOXRAYPLANEINTERSECT__NAME,		// Unique name to register box.
	BOXRAYPLANEINTERSECT__CLASS,		// Box class name
	BOXRAYPLANEINTERSECT__LOCATION,	// Box location ('plugins')
	BOXRAYPLANEINTERSECT__LABEL,		// Box label (name of box to display)
	BOXRAYPLANEINTERSECT__DESC,		// Box long description.
	FB_DEFAULT_SDK_ICON);	// Icon filename (default=Open Reality icon)

////////////////////////////////////////////////////////
// Helper math functions

double Clamp(const double x, const double lower, const double upper)
{
	double res = x;
	if (res < lower) res = lower;
	if (res > upper) res = upper;
	return res;
}

FBVector3d VectorAdd(const FBVector3d &v1, const FBVector3d &v2)
{
	return FBVector3d(v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2]);
}

FBVector3d VectorSubtract(const FBVector3d &v1, const FBVector3d &v2)
{
	return FBVector3d(v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2]);
}

double	VectorLength(const FBVector3d &v)
{
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

FBVector3d LinearInterpolation(const FBVector3d &v1, const FBVector3d &v2, const double t)
{
	FBVector3d	result;
	result = FBVector3d((v2[0] - v1[0]) * t + v1[0],
		(v2[1] - v1[1]) * t + v1[1],
		(v2[2] - v1[2]) * t + v1[2]);
	return result;
}

double DotProduct(const FBVector3d& v1, const double *v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void VectorMult(FBVector3d &v, const double factor)
{
	v[0] *= factor;
	v[1] *= factor;
	v[2] *= factor;
}

/************************************************
 *	TRANSPORT Creation
 ************************************************/
bool CBoxSmartMix::FBCreate()
{	
	/*
	*	Create the nodes for the box.
	*/
	
	m_CtrlOut = AnimationNodeOutCreate( 0, "Ctrl Out",	ANIMATIONNODE_TYPE_VECTOR );
	
	m_CtrlIn = AnimationNodeInCreate( 1, "Ctrl In",		ANIMATIONNODE_TYPE_VECTOR );
	m_WeightIn = AnimationNodeInCreate( 2, "Weight",		ANIMATIONNODE_TYPE_NUMBER );
	m_ModeWeightIn = AnimationNodeInCreate( 3, "Mode Weight",		ANIMATIONNODE_TYPE_NUMBER );
	m_DriverPosIn = AnimationNodeInCreate( 4, "Driver Pos",		ANIMATIONNODE_TYPE_VECTOR );
	m_CenterPosIn = AnimationNodeInCreate( 5, "Center Pos",		ANIMATIONNODE_TYPE_VECTOR );
	m_UseOvermove = AnimationNodeInCreate(6, "Use Overmove", ANIMATIONNODE_TYPE_NUMBER);

	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void CBoxSmartMix::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool CBoxSmartMix::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	FBVector3d lCtrlPos, lDriverPos, lCenterPos;
	double lWeight = 0.0; 
	double lModeWeight = 0.0;
	double	lTrim = 0.0;

	bool lStatus1 ,lStatus2, lStatus3;

	bool result = false;

	lStatus1 = m_CtrlIn->ReadData( lCtrlPos, pEvaluateInfo );
	lStatus2 = m_DriverPosIn->ReadData(lDriverPos, pEvaluateInfo);
	lStatus3 = m_CenterPosIn->ReadData(lCenterPos, pEvaluateInfo);
	m_WeightIn->ReadData(&lWeight, pEvaluateInfo);
	m_ModeWeightIn->ReadData(&lModeWeight, pEvaluateInfo);
	m_UseOvermove->ReadData(&lTrim, pEvaluateInfo);

	if( lStatus1 && lStatus2 && lStatus3 )
	{
		FBVector3d lOut(lCtrlPos);

		if (lWeight > 0.0)
		{
			if (lModeWeight > 0.0)
			{
				FBVector3d offsetPos = VectorSubtract(lDriverPos, lCenterPos);
				offsetPos = VectorAdd(lCtrlPos, offsetPos);
				lDriverPos = LinearInterpolation(lDriverPos, offsetPos, 0.01 * lModeWeight);

				if (lTrim > 0.0)
				{
					double l1 = VectorLength(VectorSubtract(lCtrlPos, lCenterPos));
					l1 = Clamp(l1, 0.5*lTrim, lTrim) - 0.5*lTrim;
					double trimWeight = 1.0 - (l1 / (0.5*lTrim));

					lDriverPos = LinearInterpolation(lCtrlPos, lDriverPos, trimWeight);
				}
			}
			
			lOut = LinearInterpolation(lCtrlPos, lDriverPos, 0.01 * lWeight);
		}

		result = m_CtrlOut->WriteData( lOut, pEvaluateInfo );
	}

	return result;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool CBoxSmartMix::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/
	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool CBoxSmartMix::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/
	return true;
}



/************************************************
*	 Creation
************************************************/
bool CBoxRayPlaneIntersect::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/

	m_PosOut = AnimationNodeOutCreate(0, "Pos Out", ANIMATIONNODE_TYPE_VECTOR);

	m_RayStart = AnimationNodeInCreate(1, "Ray Start", ANIMATIONNODE_TYPE_VECTOR);
	m_RayDir = AnimationNodeInCreate(2, "Ray Dir", ANIMATIONNODE_TYPE_VECTOR);
	m_Plane = AnimationNodeInCreate(3, "Plane Offset", ANIMATIONNODE_TYPE_VECTOR);
	
	return true;
}


/************************************************
*	 Destruction.
************************************************/
void CBoxRayPlaneIntersect::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
}

/************************************************
*	Real-time engine evaluation
************************************************/
bool CBoxRayPlaneIntersect::AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo)
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	FBVector3d lRayStart, lRayDir, lPlane;
	bool lStatus1, lStatus2, lStatus3;

	bool result = false;

	lStatus1 = m_RayStart->ReadData(lRayStart, pEvaluateInfo);
	lStatus2 = m_RayDir->ReadData(lRayDir, pEvaluateInfo);
	lStatus3 = m_Plane->ReadData(lPlane, pEvaluateInfo);
	
	if (lStatus1 && lStatus2 && lStatus3)
	{
		FBVector3d lOut(lPlane);

		FBVector3d n(0.0, 0.0, 1.0);
		FBVector3d w = VectorSubtract(lRayStart, lPlane);
		FBVector3d u = VectorSubtract(lRayDir, lRayStart);

		double N = -DotProduct(n, w);
		double D = DotProduct(n, u);

		double sI = N / D;
		VectorMult(u, sI);
		lOut = VectorAdd(lRayStart, u);

		result = m_PosOut->WriteData(lOut, pEvaluateInfo);
	}

	return result;
}


/************************************************
*	FBX Storage.
************************************************/
bool CBoxRayPlaneIntersect::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	/*
	*	Store box parameters.
	*/
	return true;
}


/************************************************
*	FBX Retrieval.
************************************************/
bool CBoxRayPlaneIntersect::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	/*
	*	Retrieve box parameters.
	*/
	return true;
}