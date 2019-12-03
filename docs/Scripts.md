MotionBuilder Scripts
=====================


Copy Layer For Other Takes
--------------------------

 This is an action script.
  For every selected model the script will copy current layer TRS animation
   into every other existing take in a scene

Fix Takes Time Span
-------------------

  This is an action script.
   For selected models the script will go through every scene take
  and adjust timeline range to actual models keyframes time range


Print Animated Takes For Selection
-----------------------------------
 This is an action script.
  The script will print names of takes on which your selected models are animated

Render From A Camera Switcher
----------------------------
  The script has a project/pipeline specified functions, the legacy realization of them are listened in public_common.py. Please see “How to Use” section for more information.

 This is an action script.
   The script will switch to camera switcher and render scene into a file
  Features:
  * It will use *.mov format and x264 codec
  * If you have p4 (perforce) module installed, you can add renderer file into a new change list
  * in public_common.py you can find general pipeline function for sanitize relative path to your project
       and a function to build an export file path out of your scene file
   Customize these function depends on your needs.
   
Tool Scripts
============

CheckTransitions
----------------
 This is a tool script.
 Animations should be stored as scene takes and the script will help to preview transitions between them by automating setup in Story Editor.
 
 Features:
   * automate process of story clips managing to preview transition between animations
   * store/retreive a motion transition graph
   * make a blend speed transition between motions (to test phase blend between walk and run, for example)

 