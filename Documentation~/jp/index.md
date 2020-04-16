# MeshSync DCC Plugins

MeshSync DCC Plugins は DCC ツール上のモデルの編集をリアルタイムに Unity に反映させるためのパッケージ: 
[MeshSync](https://docs.unity3d.com/Packages/com.unity.meshsync@latest) と連携するための DCC プラグインの格納するパッケージです。  
MeshSync と MeshSyncDCCPlugin が連携することで、ゲーム上でどう見えるかをその場で確認しながらモデリングすることができます。

サポートされている DCC ツールにインストールする手順については [インストール](Docs/jp/Installation.md) を参照して下さい。

## サポートされている DCC ツール

|                     | Windows            | Mac                | Linux              | 
| --------------------| ------------------ | ------------------ |------------------- | 
| Maya 2017           | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Maya 2018           | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Maya 2019           | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Maya 2020           | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Maya LT 2019 +      | :white_check_mark: |                    | :x:                | 
| 3ds Max 2017        | :white_check_mark: | :x:                | :x:                | 
| 3ds Max 2018        | :white_check_mark: | :x:                | :x:                | 
| 3ds Max 2019        | :white_check_mark: | :x:                | :x:                | 
| 3ds Max 2020        | :white_check_mark: | :x:                | :x:                | 
| MotionBuilder 2017  | :white_check_mark: | :x:                | :white_check_mark: | 
| MotionBuilder 2018  | :white_check_mark: | :x:                | :white_check_mark: | 
| MotionBuilder 2019  | :white_check_mark: | :x:                | :white_check_mark: | 
| MotionBuilder 2020  | :white_check_mark: | :x:                | :white_check_mark: | 
| Blender 2.79b       | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Blender 2.80        | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Modo 12             | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Modo 13             | :white_check_mark: | :white_check_mark: | :white_check_mark: | 
| Metasequoia 4.x     | :white_check_mark: | :white_check_mark: |                    | 

メモ:
* :white_check_mark: : 対応済み
* :x: : 対応することが不可能 (その OS で、DCC ツールが動作しないなど)
* empty : これから対応する可能性がある