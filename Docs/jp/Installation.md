# Installation

1. [Maya](#maya)

<img align="right" src="../Images/MeshSyncClientMaya.png" height=400>

## Maya


* Autodesk のライセンスのため, [手動のビルド](../end/BuildDCCPlugins.md) (現在英語のみ) が必要です。
* プラグインをコピーする
  - Windows:   
    `MAYA_APP_DIR` の環境変数が設定されている場合はそこにコピーする。  
    ない場合は `%USERPROFILE%\Documents\maya` ( Explorer のアドレスバーへコピペで直行) に *modules* ディレクトリをそのままコピーする。
  - Mac:   
    `/Users/Shared/Autodesk/modules/maya` に *UnityMeshSync* ディレクトリと *UnityMeshSync.mod* ファイルをコピーする。
  - Linux: 
    `~/maya/<maya_version>` に *modules* ディレクトリをそのままコピーする。  

* Maya を起動し、Windows -> Settings/Preferences -> Plug-in Manager を開く。
* MeshSyncClient の Loaded にチェックを入れてプラグインを有効化する。
* シェルフ UI に UnityMeshSync が追加されているはずです。  
  歯車アイコンで設定メニューを開き、MeshSync と色々遊んでみましょう。

  



