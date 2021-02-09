using System.Collections.Generic;
using System.IO;
using UnityEditor;
using UnityEngine;

public static class Menu {
    
    [MenuItem("MeshSyncDCCPlugins/Rename Plugin Zip files")]
    static void RenamePluginZipFiles() {

        string fullPath = Path.GetFullPath("Packages/com.unity.meshsync.dcc-plugins/Editor/Plugins~");

        IEnumerable<string> files = Directory.EnumerateFiles(fullPath, "*.zip", SearchOption.TopDirectoryOnly);
        foreach(string fileName in files) {
            string[] tokens = fileName.Split('_');

            //already split
            if (tokens.Length <= 3)
                continue;

            string destFileName = $"{tokens[0]}_{tokens[2]}_{tokens[3]}";
            
            File.Move(fileName, destFileName);
            Debug.Log($"Moving {fileName} to {destFileName}");
            
        }
    }    

}
