using System.IO;
using UnityEditor;
using UnityEngine;

public class PlatformUtility
{
    const string title = "Add DCC Tool";
    public string OpenDCCPathPanel()
    {
        if (Application.platform == RuntimePlatform.OSXEditor)
        {
            return OpenFilePanel(title);
        }
        else
        {
            return OpenFolderPanel(title);
        }
    }
    
    private string m_lastManualDir;

    private string OpenFolderPanel(string title)
    {
        string path = EditorUtility.OpenFolderPanel(title, m_lastManualDir, "");
        if (string.IsNullOrEmpty(path))
        {
            return null;
        }

        m_lastManualDir = path;
        return path;
    }

    private string OpenFilePanel(string title)
    {
        string file = EditorUtility.OpenFilePanel(title, m_lastManualDir, "");
        if (string.IsNullOrEmpty(file))
        {
            return null;
        }

        m_lastManualDir = Path.GetDirectoryName(file);
        return file;
    }
}