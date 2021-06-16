
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

# show fbx file info as a tooltip for the file name under a cursor
# modified version of extension from https://github.com/jbrooksuk/Intellitip

import subprocess
import sublime_plugin
import sublime
import json
import webbrowser
import re
import os
from time import time
import pprint
import ctypes

settings = {}

import json
import xml.etree.cElementTree as et
from collections import OrderedDict
import tempfile

fbx_parser_verbose = True
fbx_volatile_test = False

# TODO: please specify a location of the FbxUnpack tool which you can find in ACT repository
#  any custom logic of finding the tool location can be inserted into get_fbx_info function
fbx_unpack_tool_path = 'FbxUnpack.exe'


def get_full_path(text, ext_list, target_ext):
    # TODO: provide here any custom rule in case we want to convert relative path to a full path somehow
    return os.path.normpath(text.replace("'", "").replace('\\', '/'))


#######################################################################
# FBX

def load_scene(manager, scene, file_name):
    importer = FbxImporter.Create(manager, "")    
    result = importer.Initialize(file_name, -1, manager.GetIOSettings())
    if not result:
        return False
    
    if importer.IsFBX():
        manager.GetIOSettings().SetBoolProp(EXP_FBX_MATERIAL, False)
        manager.GetIOSettings().SetBoolProp(EXP_FBX_TEXTURE, False)
        manager.GetIOSettings().SetBoolProp(EXP_FBX_EMBEDDED, False)
        manager.GetIOSettings().SetBoolProp(EXP_FBX_SHAPE, False)
        manager.GetIOSettings().SetBoolProp(EXP_FBX_GOBO, False)
        manager.GetIOSettings().SetBoolProp(EXP_FBX_ANIMATION, True)
        manager.GetIOSettings().SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, True)
    
    result = importer.Import(scene)
    importer.Destroy()
    return result


def get_compile_options(scene):

    node = get_node_by_label(scene, OPTIONS_NODE_NAME)
    prop = FbxPropertyString(node.FindProperty('compile_options')) if node is not None else None
    if prop is not None and prop.IsValid():
        compile_options_dict = OrderedDict()
        compile_options_dict = json.loads(str(prop.Get()), object_pairs_hook=OrderedDict)

        if not compile_options_dict.has_key('skeleton'):
            raise Exception('options node does not have skeleton attribute!')

        return compile_options_dict

    return None


def get_fbx_info(file_path):
    #Take the input file path and unpack it in to a json format so we can read it

    latest_fbx_unpack_version = fbx_unpack_tool_path
    # fbx.pyd, FbxCommon.py
    if fbx_parser_verbose:
        print(latest_fbx_unpack_version)

    thumbnail_path = os.path.join(tempfile.gettempdir(), 'thumbnail.bmp')
    if fbx_parser_verbose:
        print (thumbnail_path)

    cmd = '%s -t %s %s' % (latest_fbx_unpack_version, thumbnail_path, file_path)
    if fbx_parser_verbose:
        print("Full command:", cmd)
    #Run command
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    result = proc.wait()
    std_out, std_err = proc.communicate()

    if fbx_parser_verbose:
        print("stdout", std_out)
        print("stderr", std_err)

    #Return the outputpath so we can open the json and read it.
    return std_out.decode("utf-8")


################################################################################
class AnimationTooltipCommand(sublime_plugin.EventListener):

    last_path = None
    last_name = 'No name'
    last_key_info = 'no keys info'
    last_thumbnail = False
    last_jsondata = None
    
    
    def on_hover(self, view, point, hover_zone):
        if hover_zone == sublime.HOVER_TEXT:
            sublime.set_timeout(lambda:self.run(view, point), 0)


    def process_fbx_file(self, line_text, view, where):
        try:
            ext_list = ['.fbx']
            target_ext = '.fbx'
            file_path = get_full_path( text=line_text, ext_list=ext_list, target_ext=target_ext )
            if fbx_parser_verbose:
                print( file_path )
        
        except Exception as e:
            print (e)
            view.hide_popup()
            return

        view.hide_popup()

        key_info = self.last_key_info
        has_thumbnail = self.last_thumbnail
        preview_name = self.last_name

        if file_path != self.last_path:
            
            if fbx_parser_verbose:
                print ('reading fbx...')

            json_data = get_fbx_info(file_path)
            
            data = json.loads(json_data)
            self.last_jsondata = data

            if fbx_parser_verbose:
                print (data)

            if data.get('File', None) is not None:
                has_thumbnail = data['File']['thumbnail']
                preview_name = data['File']['name']

            if data.get('stacks', None) is not None:
                stack_dict = data['stacks'][0]
                start_frame = stack_dict.get('start_frame', 0)
                end_frame = stack_dict.get('end_frame', 0)
                duration = stack_dict.get('frame_count', 0)
                key_info = "timeline: {}-{}, duration {}".format(start_frame, end_frame, duration)

            if data.get('compile_options', None) is not None:
                options_data = data['compile_options']
                compile_options_dict = OrderedDict()
                options_dict = json.loads(options_data, object_pairs_hook=OrderedDict)
                self.last_jsondata = options_dict

                if fbx_parser_verbose:
                    print (options_dict)

                if options_dict.get('parts', None) is not None:
                    parts = options_dict.get('parts')
                    key_info = " <b>Takes:</b> {}<br>".format(len(parts))

                    ndx = 0
                    for key, value in parts.items():
                        duration = value['end_frame'] - value['start_frame'] - 1
                        key_info = key_info + "<b>{}</b>-{}: [{}-{}] - {}<br>".format(ndx, key, value['start_frame'], value['end_frame'], duration)
                        ndx += 1

            if data.get('user_information', None) is not None:
                user_information_data = data['user_information']

                if fbx_parser_verbose:
                    print (user_information_data)

                key_info = key_info + "<br><b>Notes : </b><br>{}<br>".format(user_information_data)

            if data.get('linked_takes', None) is not None:
                linked_takes_data = data['linked_takes']
                linked_takes_dict = OrderedDict()
                linked_takes_dict = json.loads(linked_takes_data, object_pairs_hook=OrderedDict)

                if fbx_parser_verbose:
                    print (linked_takes_data)

                for key, value in linked_takes_dict.items():
                    if isinstance(value, dict):
                        # this is for the new way of saving linked takes in dictionary format.
                        for key_1, value_1 in value.items():
                            key_info = key_info + "<br><b>Linked Takes : </b><br>Rig Name : {}<br>Animation Name : {}<br>Path : {}<br>".format(value_1['rig_name'], value_1['take_name'], value_1['take_path'])
                    else:
                        # still supporting the old way of saving linkedtakes info via list
                        key_info = key_info + "<br><b>Linked Takes : </b><br>Rig Name : {}<br>Animation Name : {}<br>Path : {}<br>".format(value[0], value[1], value[2])

            self.last_key_info = key_info
            self.last_thumbnail = has_thumbnail
            self.last_name = preview_name

        self.last_path = file_path

        # Find in completions
        
        menus = ["<h3>"+os.path.basename(preview_name)+"</h3>"]
        if has_thumbnail:
            thumbnail_path = os.path.join(tempfile.gettempdir(), 'thumbnail.bmp')
            menus.append('<p style="float: left;"><img src="file://' + thumbnail_path + '" align="left"></p>')
        
        menus.append(key_info)
        # DONE: get project intermediate folder
        menus.append('<br><a href="locate_in_explorer">Locate In Explorer</a>')
        menus.append('<br><a href="locate_in_p4">Locate In Perforce (Under Construction)</a>')
        menus.append('<br><a href="open_compile_options">Open Compile Options</a>')
        
        # on_navigate=self.on_navigate
        popup_location = where if not isinstance(where, str) else -1
        view.show_popup(' '.join(menus), flags=sublime.HIDE_ON_MOUSE_MOVE_AWAY, location=popup_location, max_width=800, on_navigate=self.on_navigate)
    

    def run(self, view, where):
        
        if not isinstance(where, str):
            line_region = view.line(where)
        else:
            sel = view.sel()
            line_region = view.line(sel[0])

        if fbx_parser_verbose:
            print(view.rowcol(line_region.a))
            print(view.rowcol(line_region.b))
        
        line_text = view.substr(line_region)

        if fbx_parser_verbose:
            print( line_text )
      
        if line_text.find('.fbx') > 0:
            self.process_fbx_file(line_text, view, where)
        

    def appendLinks(self, menus, found):
        for pattern, link in sorted(Pref.help_links.items()):
            if re.match(pattern, found["path"]):
                host = re.match(".*?//(.*?)/", link).group(1)
                menus.append('<br>Open docs: <a href="%s">Docs</a>' % found["name"])
                break
        return menus


    def on_navigate(self, link):
        
        if link == 'open_compile_options':
            compile_options_temp_path = os.path.join(tempfile.gettempdir(), 'compile_options.txt')
            if not os.path.isfile(compile_options_temp_path):
                self.create_temp_compile_file(compile_options_temp_path)

            with open(compile_options_temp_path, mode='w') as f:
                f.write(self.last_name + '\n\n')
                if self.last_jsondata is not None:
                    pretty_dict_str = pprint.pformat(self.last_jsondata)
                    f.write(pretty_dict_str)

            os.startfile(compile_options_temp_path)

        elif link == 'locate_in_explorer':
            
            win_dir = os.path.normpath(self.last_path)
            ctypes.windll.shell32.ShellExecuteW(None, u'open', u'explorer.exe', 
                                    u'/n,/select, ' + win_dir, None, 1)

        elif link == 'open_for_editing':
            sublime.active_window().open_file(self.last_path, sublime.TRANSIENT)

        elif link == 'locate_in_p4':
            print ('under construction')

        else:
            webbrowser.open_new_tab(link)


    def create_temp_compile_file(self, full_path):
        """
        create a temp compile_options.txt 
        """
        with open(full_path, mode='w') as f:
            f.write('TEMP_COMPILE_FILE_CREATED')


    def debug(self, *text):
        if Pref.debug:
            print(*text)