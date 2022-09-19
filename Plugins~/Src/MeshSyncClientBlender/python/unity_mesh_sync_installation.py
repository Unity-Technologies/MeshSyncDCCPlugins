import platform
from os import path
import json
import subprocess
import time
import bpy

EDITOR_COMMAND_ADD_SERVER = 1
EDITOR_COMMAND_GET_PROJECT_PATH = 2

DEBUG = 1

from . import MeshSyncClientBlender as ms
msb_context = ms.Context()

unity_process = None

def MS_MessageBox(message = "", title = "MeshSync", icon = 'INFO'):
    def draw(self, context):
        self.layout.label(text=message)
    bpy.context.window_manager.popup_menu(draw, title = title, icon = icon)

def msb_error_messages_for_status(status, context):
    if status == 'SUCCESS':
        return True
    if status == 'INVALID_PATH':
        MS_MessageBox("Path "+context.scene.meshsync_unity_project_path+" is not a Unity Project")
    elif status == 'EDITOR_NOT_EXISTS':
        MS_MessageBox("Could not find Unity Editors in path "+ context.scene.meshsync_unity_editors_path)
    elif status == 'SERVER_NOT_ADDED':
        MS_MessageBox("Could not add server to scene")

    return False

def msb_get_editor_path_prefix():
    path = bpy.context.scene.meshsync_unity_editors_path
    
    if path[-1] == '/' or path[-1] == '\'':
        return path

    os = platform.system()
    if os == 'Windows':
        path = path + '\\'
    elif os == 'Darwin' or os == 'Linux':
        path = path + '/'

    return path

def msb_get_editor_path_suffix():
    os = platform.system()
    if os == 'Windows':
        path = "\\Editor\\Unity.exe"
    elif os == 'Darwin':
        path = "/Unity.app/Contents/MacOS/Unity"
    elif os == 'Linux':
        path = "/Unity.app/Contents/Linux/Unity"
    return path

def msb_get_editor_path(editor_version):
    return msb_get_editor_path_prefix() + editor_version + msb_get_editor_path_suffix()

def msb_validate_project_path(directory):
    project_version_path = directory +"/ProjectSettings/ProjectVersion.txt"
    if path.exists(project_version_path):
        return True
    return False

def msb_try_get_valid_project_path(context):
    directory = context.scene.meshsync_unity_project_path

    #Validate that the path is a project
    if msb_validate_project_path(directory) == False:
        #Try to fill a valid path from an already running project
        directory = msb_try_get_path_from_server()
        if msb_validate_project_path(directory) == False:
            return 'INVALID_PATH'

    return directory

def msb_try_get_path_from_server():
    if not msb_context.is_editor_server_available:
        return 'NO SERVER'

        #Get the project path
    msb_context.sendEditorCommand(EDITOR_COMMAND_GET_PROJECT_PATH)
    server_reply = msb_context.editor_command_reply
    return server_reply

def msb_add_meshsync_to_unity_manifest(path, entry):
    file = open(path, "r+");
    data = json.load(file);
    
    # check if MeshSync is installed on selected project
    # if not installed, install it
    found = False

    for package in data['dependencies']:
        if package == 'com.unity.meshsync' and data['dependencies']['com.unity.meshsync'] == entry:
            found = True
            break

    if found == False:
        #install for user
        dependencies = data["dependencies"];
        dependencies["com.unity.meshsync"] = entry
        file.seek(0)
        file.truncate(0)
        json.dump(data, file)
        
        file.close()

def msb_try_install_meshsync_to_unity_project(directory):

        if directory[-1] == '/' or directory[-1] == '\\':
            manifest_path = directory + "Packages/manifest.json"
        else:
            manifest_path = directory + "/Packages/manifest.json"

        #TODO replace with release that has the Editor Commands changes
        manifest_entry = "0.14.5-preview"
        if DEBUG == 1:
            manifest_entry = "file:C:\\Users\\Sean Dillon\\MeshSync\\MeshSync~\\Packages\\com.unity.meshsync"

        msb_add_meshsync_to_unity_manifest(manifest_path, manifest_entry)

def msb_try_setup_scene_server(context):

    # Try get valid path
    path = msb_try_get_valid_project_path(context)
    if path == 'INVALID_PATH':
        return 'INVALID_PATH'

    # Write back the valid path, in case it comes from a listening server
    context.scene.meshsync_unity_project_path = path

    # Try install to unity project
    msb_try_install_meshsync_to_unity_project(path)

    # Try starting unity if not already started
    start_status = msb_try_start_unity_project(path)

    #If the project was launched now, wait until the Editor server is available
    if start_status == 'STARTED':
        while(msb_context.is_editor_server_available is False):
            time.sleep(0.1)
    elif start_status == 'EDITOR_NOT_EXISTS':
        return 'EDITOR_NOT_EXISTS'

    # Send a command to add a scene server (if it doesn't exist already)
    msb_context.sendEditorCommand(EDITOR_COMMAND_ADD_SERVER)
    reply = msb_context.editor_command_reply

    if not reply == 'ok':
        return 'SERVER_NOT_ADDED'

    return 'SUCCESS'

def msb_get_editor_version(directory):
    project_version_path = directory + "/ProjectSettings/ProjectVersion.txt"
    with open(project_version_path, "r+") as file:
        first_line = file.readline()
        version = first_line[len("m_EditorVersion: "):]
        version = version[:-1]
        return version

def msb_launch_project(editor_path, project_path):
    os = platform.system()
    if os == 'Windows':
        path = editor_path + " -projectPath \"" + project_path + "\""
    elif os == 'Darwin' or os == 'Linux':
        path = editor_path + " -projectPath " + project_path

    return subprocess.Popen(path)

def msb_try_start_unity_project (directory):
        
    global unity_process

    #Check if we have launched the project as a subprocess
    if unity_process is not None and unity_process.poll() is None:
        return 'ALREADY_STARTED'

    #Check if there is an editor server listening from the target project
    if msb_context.is_editor_server_available:
        msb_context.sendEditorCommand(EDITOR_COMMAND_GET_PROJECT_PATH)
        reply_path = msb_context.editor_command_reply;
        if reply_path == directory:
            return 'ALREADY_STARTED'

    editor_version = msb_get_editor_version(directory)
    editor_path = msb_get_editor_path(editor_version)

    if not path.exists(editor_path):
        return 'EDITOR_NOT_EXISTS'

    #Launch the editor with the project
    unity_process = msb_launch_project(editor_path, directory)

    return 'STARTED'
