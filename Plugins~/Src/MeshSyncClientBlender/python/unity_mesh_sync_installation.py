import platform
from os import path
import json
import subprocess
import time
import bpy
import os
import socket
from contextlib import closing

EDITOR_COMMAND_ADD_SERVER = 1
EDITOR_COMMAND_GET_PROJECT_PATH = 2


from . import MeshSyncClientBlender as ms
msb_context = ms.Context()

unity_process = None

def msb_is_project_open(directory):
    full_path = path.join(directory, "Temp", "UnityLockfile")
    if not path.exists(full_path):
        return False
    try:
        with open(full_path, 'wb') as file:
            os = platform.system()
            if os == "Darwin" or os == "Linux":
                import fcntl
                fcntl.lockf(file.fileno(), fcntl.LOCK_EX|fcntl.LOCK_NB)
                fcntl.lockf(file.fileno(), fcntl.LOCK_UN)
            elif os == "Windows":
                import msvcrt
                msvcrt.locking(file.fileno(), msvcrt.LK_NBLCK, 0)
                msvcrt.locking(file.fileno(), msvcrt.LK_UNLCK, 0)
    except:
        return True

    return False

def MS_MessageBox(message = "", title = "MeshSync", icon = 'INFO'):
    def draw(self, context):
        self.layout.label(text=message)
    bpy.context.window_manager.popup_menu(draw, title = title, icon = icon)

def msb_get_meshsync_entry():
    return "0.15.0-preview"

def msb_get_min_supported_meshsync_version():
    return "0.15.0-preview"

def msb_get_most_recent_version(v1, v2):
    tokens1 = v1.replace("-preview","").split('.')
    tokens2 = v2.replace("-preview","").split('.')

    if len(tokens1) != len(tokens2):
        return 'VERSION_MISMATCH'

    for i in range(len(tokens1)):
        if tokens1[i] == tokens2[i]:
            continue
        
        if int(tokens1[i]) > int(tokens2[i]):
            return v1

        return v2

def msb_preferences(context):
    return context.preferences.addons[__package__].preferences

def msb_error_messages_for_status(status, context):
    if status == 'SUCCESS':
        return True
    if status == 'INVALID_PATH':
        MS_MessageBox("Path "+msb_preferences(context).project_path+" is not a Unity Project.")
    elif status == 'EDITOR_NOT_EXISTS':
        MS_MessageBox("Could not find Unity Editors in path "+ msb_preferences(context).editors_path+" .")
    elif status == 'SERVER_NOT_ADDED':
        MS_MessageBox("Could not add server to scene.")
    elif status == 'LAUNCH FAILED':
        MS_MessageBox("Could not launch project in path "+msb_preferences(context).project_path+" .")
    elif status == 'SERVER_UNAVAILABLE':
        MS_MessageBox("Could not connect with scene server.")
    elif status == 'VERSION_MISMATCH':
        MS_MessageBox("Project at "+msb_preferences(context).project_path+ " has an incompatible version of MeshSync installed.")

    return False

def msb_get_editor_path_suffix():
    os = platform.system()
    if os == 'Windows':
        return path.join("Editor","Unity.exe")
    elif os == 'Darwin':
        return path.join("Unity.app","Contents","MacOS","Unity")
    elif os == 'Linux':
        return path.join("Editor","Unity")
    return None

def msb_get_editor_path(context, editor_version):
    return path.join(msb_preferences(context).editors_path,editor_version,msb_get_editor_path_suffix())

def msb_validate_project_path(directory):
    project_version_path = path.join(directory,"ProjectSettings","ProjectVersion.txt")
    if path.exists(project_version_path):
        return True
    return False

def msb_try_get_valid_project_path(context):
    directory = msb_preferences(context).project_path

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
    msb_context.sendEditorCommand(EDITOR_COMMAND_GET_PROJECT_PATH, None)
    server_reply = msb_context.editor_command_reply
    return server_reply

def msb_add_meshsync_to_unity_manifest(manifest_path, lock_path, entry):

    with open(lock_path, "r+") as file:
        data = json.load(file);
    
        # check if MeshSync is installed on selected project
        # if not installed, install it
        found = False

        for package in data['dependencies']:
            if package == 'com.unity.meshsync':
                version = data['dependencies']['com.unity.meshsync']['version']
                if version != entry:
                    min_version = msb_get_min_supported_meshsync_version()
                    most_recent = msb_get_most_recent_version(min_version, version)
                    if most_recent == min_version or most_recent == 'VERSION_MISMATCH':
                        return 'VERSION_MISMATCH'
                found = True
                break
            elif package == 'com.unity.meshsync':
                print(data['dependencies']['com.unity.meshsync'])

    if found == False:
        with open(manifest_path, "r+") as file:
            data = json.load(file);
            #install for user
            dependencies = data["dependencies"];
            dependencies["com.unity.meshsync"] = entry
            file.seek(0)
            file.truncate(0)
            json.dump(data, file)

    return 'SUCCESS'

def msb_try_install_meshsync_to_unity_project(directory):
        lock_path = path.join(directory,"Packages","packages-lock.json")
        manifest_path = path.join(directory,"Packages","manifest.json")

        manifest_entry = msb_get_meshsync_entry()

        return msb_add_meshsync_to_unity_manifest(manifest_path, lock_path, manifest_entry)

def msb_try_setup_scene_server(context):

    #Check if scene server is listening
    if msb_context.is_server_available:
        return 'SUCCESS'

    # Try to auto config the server settings
    msb_try_auto_config_server_settings(context)

    # Try get valid path
    path = msb_try_get_valid_project_path(context)
    if path == 'INVALID_PATH':
        return 'INVALID_PATH'

    # Write back the valid path, in case it comes from a listening server
    msb_preferences(context).project_path = path

    # Try install to unity project
    install_status = msb_try_install_meshsync_to_unity_project(path)
    if install_status != 'SUCCESS':
        return install_status

    # Try starting unity if not already started
    start_status = msb_try_start_unity_project(context, path)

    #If the project was launched now, wait until the Editor server is available
    if start_status == 'STARTED':
        while msb_context.is_editor_server_available is False:
            time.sleep(0.1)
            # If the process is killed for any reason, stop waiting
            if not msb_is_unity_process_alive():
                return 'LAUNCH FAILED'
    elif start_status == 'EDITOR_NOT_EXISTS':
        return 'EDITOR_NOT_EXISTS'
    elif start_status == 'FAILED' or start_status == 'UNKNOWN':
        return 'LAUNCH FAILED'

    # Send a command to add a scene server (if it doesn't exist already)
    msb_context.sendEditorCommand(EDITOR_COMMAND_ADD_SERVER, str(context.scene.meshsync_server_port))
    reply = msb_context.editor_command_reply

    if not reply == 'ok':
        return 'SERVER_NOT_ADDED'

    #The is_server_available request has a 1s timeout
    if msb_context.is_server_available:
        return 'SUCCESS'

    return 'SERVER_UNAVAILABLE'

def msb_get_editor_version(directory):
    project_version_path = path.join(directory,"ProjectSettings","ProjectVersion.txt")
    with open(project_version_path, "r+") as file:
        first_line = file.readline()
        version = first_line[len("m_EditorVersion: "):]
        version = version[:-1]
        return version

def msb_launch_project(context, editor_path, project_path):
    port = context.scene.meshsync_editor_server_port
    try:      
        return 'SUCCESS', subprocess.Popen([editor_path, "-projectPath", project_path,"SERVER_ACTIVE", "True", "PORT", str(port)])
    except:
        return 'FAILED', None

def msb_is_unity_process_alive():
    global unity_process
    if unity_process is not None and unity_process.poll() is None:
        return True
    return False

def msb_try_start_unity_project (context, directory):
        

    if msb_is_project_open(directory):
        return 'ALREADY_STARTED'

    global unity_process

    editor_version = msb_get_editor_version(directory)
    editor_path = msb_get_editor_path(context, editor_version)

    if not path.exists(editor_path):
        return 'EDITOR_NOT_EXISTS'

    #Launch the editor with the project
    status, unity_process = msb_launch_project(context, editor_path, directory)
    if status == 'FAILED':
        return 'FAILED'
    elif status == 'SUCCESS':
        return 'STARTED'

    return 'UNKNOWN'

def msb_is_port_available(port):
    try:
        with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as s:
            s.bind(('', port))
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        return True
    except:
        return False

def msb_bind_next_available_socket(port):
    if not msb_is_port_available(port):
        port = 0

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(('', port))
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    return s

def msb_try_auto_config_server_settings(context):
    if not context.scene.meshsync_auto_config_server:
        return

    context.scene.meshsync_server_address = "127.0.0.1"

    # If the editor server is available, only change the port for the scene server 
    if msb_context.is_editor_server_available:
        with closing(msb_bind_next_available_socket(context.scene.meshsync_server_port)) as sceneSocket:
            context.scene.meshsync_server_port = sceneSocket.getsockname()[1]
        return
    
    with closing(msb_bind_next_available_socket(context.scene.meshsync_editor_server_port)) as editorSocket:
        context.scene.meshsync_editor_server_port = editorSocket.getsockname()[1]
        with closing(msb_bind_next_available_socket(context.scene.meshsync_server_port)) as sceneSocket:
            context.scene.meshsync_server_port = sceneSocket.getsockname()[1]