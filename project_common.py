#
# Copyright (c) 2020 Avalanche Studios. All rights reserved.
# Licensed under the MIT License.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE
#
#--------------------------------------------------------------------------------------
# File: public_common.py
#
# This is a startup script.
#  Here there are general pipeline functions to work with project path and relative path
# These functions could be customized according to your project needs and setup
#
#-------------------------------------------------------------------------------------

import os
import sys
import inspect
from pyfbsdk import FBSystem

def GetSanitizedRelPath():
	file_name = g_App.FBXFileName
	return os.path.dirname(file_name)

def AddFileToChangeList(file_name, description):
	print('Trying to add {} to a version control, No project support implemented'.format(file_name))        

# This is a logic to make your output path from the scene file name
def BuildAnExportPath(file_name, extension='.mov'):

	export_path = os.path.dirname(file_name)
	new_fname = os.path.splitext(file_name)[0]+extension
	export_path = os.path.join(export_path, new_fname)

	return export_path


##################################### LIB CHECK CODE ###
g_CommonPublicScript = 'project_common.py'

def SetupLibPath(lib_filename=None):

	common_script_file = inspect.getfile(inspect.currentframe()) if lib_filename is None else lib_filename
	common_script_file = os.path.basename(common_script_file)

	paths = FBSystem().GetPythonStartupPath()
	for path in paths:        
		if os.path.isfile(os.path.join(path, common_script_file)):
			
			# check if that path is inside sys.path
			if not path in sys.path:
				sys.path.append(path)
			
			break
	else:
		raise Exception('Script File Is Not Found In Any Python StartUp Path!')
#
#
########################################################

SetupLibPath(g_CommonPublicScript)