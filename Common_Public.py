
#--------------------------------------------------------------------------------------
# File: Common_Public.py
#
# This is a startup script.
#  Here there are general pipeline functions to work with project path and relative path
# These functions could be customized according to your project needs and setup
#
# Copyright (c) Avalanche Studios. All rights reserved.
# Licensed under the MIT License.
#-------------------------------------------------------------------------------------

import os

def GetSanitizedRelPath():
    file_name = g_App.FBXFileName
    return os.path.dirname(file_name)


# This is a logic to make your output path from the scene file name
def BuildAnExportPath(file_name):

    export_path = os.path.dirname(file_name)
    new_fname = os.path.splitext(os.path.basename(file_name))[0]+'.mov'
    export_path = os.path.join(export_path, new_fname)

    return export_path