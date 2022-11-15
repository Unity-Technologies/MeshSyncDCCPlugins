import platform
from os import path
import json
import subprocess
import time
import bpy
import os
import socket
import ssl
import re
from contextlib import closing
from urllib.request import urlopen

EDITOR_COMMAND_ADD_SERVER = 1
EDITOR_COMMAND_GET_PROJECT_PATH = 2


from . import MeshSyncClientBlender as ms
msb_context = ms.Context()

unity_process = None
meshsync_version = None

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
    except OSError as e:
        print ("Error when trying to lock file:" + str(e))
        return True

    return False

def MS_MessageBox(message = "", title = "MeshSync", icon = 'INFO'):
    def draw(self, context):
        self.layout.label(text=message)
    bpy.context.window_manager.popup_menu(draw, title = title, icon = icon)

def msb_version_match_major_minor(v1, v2):
    if v1 is None:
        return v2
    if v2 is None:
        return v1

    v1.replace("-preview", "")
    v2.replace("-preview", "")
    tokens1 = v1.split('.')
    tokens2 = v2.split('.')
    return float(tokens1[0]) == float(tokens2[0]) and float(tokens1[1]) == float(tokens2[1])

def msb_find_latest_compatible_version(versions):
    version_re = re.compile('[0-9]*\.[0-9]*\.[0-9]*-preview')

    latest_version = None
    for version in versions:
        if version_re.match(version) is None:
            continue
        if msb_version_match_major_minor(version, msb_context.PLUGIN_VERSION):
            latest_version = msb_get_most_recent_version(latest_version, version)
    return latest_version


def msb_get_meshsync_entry():

    global meshsync_version
    if meshsync_version is not None:
        return meshsync_version

    ssl._create_default_https_context = ssl._create_unverified_context
    
    page = 1

    for _ in range(100):
        releases = []

        with urlopen(f"https://api.github.com/repos/unity3d-jp/MeshSync/releases?per_page=100&page={page}") as response:
            response_content = response.read()
            response_content.decode('utf-8')
            json_response = json.loads(response_content)
            if len(json_response) == 0:
                break

            for json_release in json_response:
                name = json_release['name']
                releases.append(name)
            page = page + 1
        meshsync_version = msb_find_latest_compatible_version(releases)
        if meshsync_version is not None:
            return meshsync_version

def msb_get_min_supported_meshsync_version():
    return "0.15.0-preview"

def msb_get_local_package_version(dir):
    dir = dir.replace("file:", "")
    dir = path.join(dir, "package.json")

    #The test projects would have local paths on their manifest
    #We consider test projects to have the latest version
    
    if not path.isabs(dir):
        return "inf.inf.inf-preview"

    with open(dir, 'r') as file:
        data = json.load(file)
        return data['version']

def msb_get_most_recent_version(v1, v2):
    if v1 is None:
        return v2
    if v2 is None:
        return v1
    if "file:" in v1:
        v1 = msb_get_local_package_version(v1)
    if "file:" in v2:
        v2 = msb_get_local_package_version(v2)

    tokens1 = v1.replace("-preview","").split('.')
    tokens2 = v2.replace("-preview","").split('.')

    if len(tokens1) != len(tokens2):
        return 'VERSION_MISMATCH'

    for i in range(len(tokens1)):
        if tokens1[i] == tokens2[i]:
            continue
        
        if float(tokens1[i]) > float(tokens2[i]):
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

def msb_meshsync_info_json(json_path):
    if not os.path.exists(json_path):
        return ""
    with open(json_path, 'r') as file:
        dependencies = json.load(file)['dependencies']
        if 'com.unity.meshsync' in dependencies:
            return dependencies['com.unity.meshsync']
        return ""

def msb_meshsync_version_manifest(project_path):
    manifest_path = path.join(project_path,"Packages","manifest.json")
    return msb_meshsync_info_json(manifest_path)

def msb_meshsync_version_package_lock(project_path):
    lock_path = path.join(project_path,"Packages","packages-lock.json")
    info = msb_meshsync_info_json(lock_path)
    if info == "":
        return ""
    return info['version']

def msb_add_meshsync_to_unity_manifest(project_path, entry):
    manifest_path = path.join(project_path,"Packages","manifest.json")
    with open(manifest_path, "r+") as file:
        data = json.load(file)

        dependencies = data["dependencies"];
        dependencies["com.unity.meshsync"] = entry
        file.seek(0)
        file.truncate(0)
        json.dump(data, file, indent = 2)

def msb_try_install_meshsync_to_unity_project(project_path):
    installed_version = msb_get_meshsync_entry()
    version = msb_meshsync_version_package_lock(project_path)

    if installed_version != "" and version != installed_version:
        min_version = msb_get_min_supported_meshsync_version()
        most_recent = msb_get_most_recent_version(min_version, version)
        if most_recent == min_version or most_recent == 'VERSION_MISMATCH':
            return 'VERSION_MISMATCH'
    
    if installed_version == "":
        msb_add_meshsync_to_unity_manifest(project_path, manifest_entry)
    return 'SUCCESS'

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

def msb_get_hub_dir():
    system = platform.system()
    path = ""
    if system == "Windows":
        path = os.path.join(os.getenv('APPDATA'),"UnityHub")
    elif system == "Darwin":
        path = os.path.join(os.getenv("HOME"),"Library","Application Support","UnityHub")
    #TODO Linux

    return path

def msb_get_hub_info():
    hub_dir = msb_get_hub_dir()
    if not os.path.exists(hub_dir):
        return "", ""

    config_path = os.path.join(hub_dir, "hubInfo.json")

    if not os.path.exists(config_path):
        return "", ""

    with open(config_path, "r+") as file:
        data = json.load(file)
        path = os.path.normpath(data['executablePath'])
        version = data['version']
        if not os.path.exists(path):
            path = "", ""
        return path, version

def msb_get_hub_path():
    path, version = msb_get_hub_info()
    return path

def msb_get_hub_version():
    path, version = msb_get_hub_info()
    return version

def msb_get_editors_path():
    path = msb_get_hub_path()
    if not os.path.exists(path):
        return ""

    p = subprocess.Popen([path, "--", "--headless","ip", "-g" ], stdout = subprocess.PIPE)
    path = None
    for line in iter(p.stdout.readline, b''):
        path = line.rstrip()
    return path.decode('utf-8')