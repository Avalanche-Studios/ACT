
/*
#
# Copyright(c) 2021 Avalanche Studios.All rights reserved.
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

#include "AnimLiveBridgeSession.h"
#include "AnimLiveBridgeSharedMemory.h"

CAnimLiveBridgeSession::CAnimLiveBridgeSession()
{
}

CAnimLiveBridgeSession::~CAnimLiveBridgeSession()
{
	Close();
}

int CAnimLiveBridgeSession::Open(const char* pair_name, const bool is_server)
{
	if (!m_Hardware)
	{
		m_Hardware = new CAnimLiveBridgeSharedMemory(this);
	}

	return (m_Hardware) ? m_Hardware->Open(pair_name, is_server) : -1;
}

int CAnimLiveBridgeSession::Close()
{
	if (m_Hardware)
	{
		m_Hardware->Close();

		delete m_Hardware;
		m_Hardware = nullptr;

		return 0;
	}
	return -1;
}

bool CAnimLiveBridgeSession::IsOpen() const 
{
	return (m_Hardware) ? m_Hardware->IsOpen() : false; 
}

int CAnimLiveBridgeSession::Commit(const bool auto_finish_event) 
{ 
	return (m_Hardware) ? m_Hardware->Commit(auto_finish_event) : -1; 
}

int CAnimLiveBridgeSession::ManualPostCommitFinish() 
{ 
	return (m_Hardware) ? m_Hardware->ManualPostCommitFinish() : -1; 
}