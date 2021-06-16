Sublime Text 3 Extensions
=====================

[![AnimationTooltip](https://github.com/Avalanche-Studios/ACT/blob/master/docs/images/sublime_animation_tooltip.jpg)]()

  Animation Tooltip - sublime text 3 extensions to show up a tooltip with fbx file internal data info when a cursor is under the fbx file path.

The extension requires to have FbxUnpack compiled tool


How to Use
---------------------
  Default path for sublime text 3 extensions are in your user AppData\Roaming\ Sublime Text 3\Packages. You copy or create AnimationTooltip folder there and put the script inside.

In the script you should specify a path to your pre-compiled FbxUnpack tool. The binary version of the tool you can download from a release section of the github ACT repository.
 There is a fbx_unpack_tool_path variable for a default tool path or you can put some code to find out a proper location in get_fbx_info function

 Another function get_full_path needs some customization if you want to have a  rule to extract a full animation path from a relative path provided in the text.


Third party used
---------------------
FBX SDK
