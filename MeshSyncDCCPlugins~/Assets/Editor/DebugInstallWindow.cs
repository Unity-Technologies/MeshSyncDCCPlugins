﻿using System.Collections.Generic;
using System.IO;
using Unity.MeshSync.Editor;
using UnityEngine;
using UnityEditor;
using UnityEngine.UIElements;

public class DebugInstallWindow : EditorWindow {
    

//----------------------------------------------------------------------------------------------------------------------        
    private void CreateGUI() {

        m_root = rootVisualElement;
        LoadAndAddStyle( m_root.styleSheets, MeshSyncEditorConstants.USER_SETTINGS_STYLE_PATH);	
        m_root.Clear();
        m_installPluginButtons.Clear();

        VisualTreeAsset container = LoadVisualTreeAsset(
            MeshSyncEditorConstants.DCC_TOOLS_SETTINGS_CONTAINER_PATH
        );
        
        VisualTreeAsset dccToolInfoTemplate = LoadVisualTreeAsset(
            MeshSyncEditorConstants.DCC_TOOL_INFO_TEMPLATE_PATH
        );

        TemplateContainer containerInstance = container.CloneTree();
        ScrollView scrollView = containerInstance.Query<ScrollView>().First();

        string[] invisibleButtonNames = new[] { "AutoDetectDCCButton", "AddDCCToolButton", "ChecksPluginUpdatesButton" };
        foreach (string buttonName in invisibleButtonNames) {
            Button btn = containerInstance.Query<Button>(buttonName).First();
            btn.visible = false;
        }

        //Add detected DCCTools to ScrollView
        MeshSyncEditorSettings settings = MeshSyncEditorSettings.GetOrCreateSettings();
        foreach (KeyValuePair<string, DCCToolInfo> dccToolInfo in settings.GetDCCToolInfos()) {
            AddDCCToolSettingsContainer(dccToolInfo.Value, scrollView, dccToolInfoTemplate);                
        }            
        
        //Add the container of this tab to root
        m_root.Add(containerInstance);

    }

//----------------------------------------------------------------------------------------------------------------------        

    private void AddDCCToolSettingsContainer(DCCToolInfo dccToolInfo, VisualElement top, VisualTreeAsset dccToolInfoTemplate) {
        string desc = dccToolInfo.GetDescription();
        TemplateContainer container = dccToolInfoTemplate.CloneTree();
        Label nameLabel = container.Query<Label>("DCCToolName").First();
        nameLabel.text = desc;
        
        //Load icon
        Texture2D iconTex = LoadIcon(dccToolInfo.IconPath);
        if (null != iconTex) {
            container.Query<Image>("DCCToolImage").First().image = iconTex;
        } else {
            container.Query<Label>("DCCToolImageLabel").First().text = desc[0].ToString();
        }
        
        container.Query<Label>("DCCToolPath").First().text = "Path: " + dccToolInfo.AppPath;
        BaseDCCIntegrator integrator = DCCIntegratorFactory.Create(dccToolInfo);
            
        //Buttons
        {
            Button button = container.Query<Button>("LaunchDCCToolButton").First();
            button.clickable.clickedWithEventInfo += OnLaunchDCCToolButtonClicked;
            button.userData                       =  dccToolInfo;
        }
        {
            Button button = container.Query<Button>("InstallPluginButton").First();
            button.clickable.clickedWithEventInfo += OnInstallPluginButtonClicked;
            button.userData                       =  integrator;
            m_installPluginButtons.Add(button);
        }
        {
            Button button = container.Query<Button>("RemoveDCCToolButton").First();
            button.visible =  false;
        }

        
        
        top.Add(container);
    }
    
//----------------------------------------------------------------------------------------------------------------------        

    #region Button callbacks

    void OnLaunchDCCToolButtonClicked(EventBase evt) {
        DCCToolInfo dccToolInfo = GetEventButtonUserDataAs<DCCToolInfo>(evt.target);           
        if (null==dccToolInfo || string.IsNullOrEmpty(dccToolInfo.AppPath) || !File.Exists(dccToolInfo.AppPath)) {
            Debug.LogWarning("[MeshSync] Failed to launch DCC Tool");
            return;
        }
        
        DiagnosticsUtility.StartProcess(dccToolInfo.AppPath);
    }

    void OnInstallPluginButtonClicked(EventBase evt) {

        // if (null == m_latestCompatibleDCCPluginVersion) {
        //     EditorUtility.DisplayDialog("MeshSync",
        //         $"DCC Plugin compatible with MeshSync@{MeshSyncEditorConstants.GetPluginVersion()} is not found", 
        //         "Ok"
        //     );
        //     return;
        // }            
        //
        // BaseDCCIntegrator integrator = GetEventButtonUserDataAs<BaseDCCIntegrator>(evt.target);           
        // if (null==integrator) {
        //     Debug.LogWarning("[MeshSync] Failed to Install Plugin");
        //     return;
        // }
        //
        // integrator.Integrate(m_latestCompatibleDCCPluginVersion.ToString(), () => {
        //     DCCToolInfo dccToolInfo = integrator.GetDCCToolInfo();                
        //     if (!m_dccStatusLabels.ContainsKey(dccToolInfo.AppPath)) {
        //         SetupInternal(m_root);
        //         return;
        //     }
        //
        //     UpdateDCCPluginStatusLabel(m_dccStatusLabels[dccToolInfo.AppPath]);
        // });

    }
    #endregion
    

//----------------------------------------------------------------------------------------------------------------------        

    Texture2D LoadIcon(string iconPath) {

        if (string.IsNullOrEmpty(iconPath) || !File.Exists(iconPath)) {
            return null;
        }

        //TODO-sin: 2020-5-11: Support ico ?
        string ext = Path.GetExtension(iconPath).ToLower();
        if (ext != ".png") {
            return null;
        }

        byte[] fileData = File.ReadAllBytes(iconPath);
        Texture2D tex = new Texture2D(2, 2);
        tex.LoadImage(fileData, true);
        return tex;
    }

    private static VisualTreeAsset LoadVisualTreeAsset(string pathWithoutExt, string ext = ".uxml") {
        string          path  = pathWithoutExt + ext;
        VisualTreeAsset asset = AssetDatabase.LoadAssetAtPath<VisualTreeAsset>(path);
        if (null == asset) {
            Debug.LogError("[AnimeToolbox] Can't load VisualTreeAsset: " + path);
            return null;
        }
        return asset;
    }    

    private static void LoadAndAddStyle(VisualElementStyleSheetSet set, string pathWithoutExt, string ext = ".uss") {
        string     path  = pathWithoutExt + ext;
        StyleSheet asset = AssetDatabase.LoadAssetAtPath<StyleSheet>(path);
        if (null == asset) {
            Debug.LogError("[AnimeToolbox] Can't load style: " + path);
            return;
        }
        set.Add(asset);
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    static T GetEventButtonUserDataAs<T>(IEventHandler eventTarget) where T: class{
        Button button = eventTarget as Button;
        if (null == button) {
            return null;
        }

        T dccToolInfo = button.userData as T;
        return dccToolInfo;
    }
//----------------------------------------------------------------------------------------------------------------------

    private List<Button> m_installPluginButtons = new List<Button>();
   
    private VisualElement m_root             = null;
    
    
}