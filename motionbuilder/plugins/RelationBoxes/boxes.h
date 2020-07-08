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
// file: boxes.h
//

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration defines
#define	BOXSMARTMIX__CLASSNAME				CBoxSmartMix
#define BOXSMARTMIX__CLASSSTR				"CBoxSmartMix"

#define	BOXRAYPLANEINTERSECT__CLASSNAME		CBoxRayPlaneIntersect
#define BOXRAYPLANEINTERSECT__CLASSSTR		"CBoxRayPlaneIntersect"

/**	CBoxSmartMix class - 
*/
class CBoxSmartMix : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( CBoxSmartMix, FBBox );

public:
	//! creation function.
	virtual bool FBCreate() override;

	//! destruction function.
	virtual void FBDestroy() override;

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo) override;

	//! FBX Storage function
	virtual bool FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;

	//! FBX Retrieval functionc
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;


private:
	FBSystem			m_System;

	FBAnimationNode*	m_CtrlOut;			//!< out

	FBAnimationNode*	m_CtrlIn;
	FBAnimationNode*	m_WeightIn;
	FBAnimationNode*	m_ModeWeightIn;
	FBAnimationNode*	m_DriverPosIn;
	FBAnimationNode*	m_CenterPosIn;
	FBAnimationNode*	m_UseOvermove;
};

/**	BoxRayPlaneIntersect class -
*/
class CBoxRayPlaneIntersect : public FBBox
{
	//--- box declaration.
	FBBoxDeclare(CBoxRayPlaneIntersect, FBBox);

public:
	//! creation function.
	virtual bool FBCreate() override;

	//! destruction function.
	virtual void FBDestroy() override;

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo) override;

	//! FBX Storage function
	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;

	//! FBX Retrieval functionc
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;


private:
	FBSystem			m_System;

	FBAnimationNode*	m_PosOut;			//!< out

	FBAnimationNode*	m_RayStart;
	FBAnimationNode*	m_RayDir;
	FBAnimationNode*	m_Plane;
};

