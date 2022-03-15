# MeshSync DCC Plugins

MeshSync DCC Plugins は DCC ツール上のモデルの編集をリアルタイムに Unity に反映させるためのパッケージ: 
[MeshSync](https://docs.unity3d.com/Packages/com.unity.meshsync@latest) と連携するための DCC プラグインの格納するパッケージです。  
MeshSync と MeshSyncDCCPlugin が連携することで、ゲーム上でどう見えるかをその場で確認しながらモデリングすることができます。

## フィーチャー

|                              | Maya                 | 3ds Max              | MotionBuilder        | Blender              | Modo                 | Metasequoia          | 
| -----------------------------| -------------------- | -------------------- | -------------------- | -------------------- | -------------------- | -------------------- | 
| ポリゴンメッシュの同期         | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |  
| カメラの同期                  | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |  
| ライトの同期                  | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |  
| 両面 (Double-sided) メッシュ  | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |  
| 負のスケール                  | :small_red_triangle: | :small_red_triangle: | :small_red_triangle: | :small_red_triangle: | :small_red_triangle: |                      |
| マルチ UV                     |                      | :heavy_check_mark:   |                      | :heavy_check_mark:   |                      |                      |  
| Scene Cache を出力            | :heavy_check_mark:   | :heavy_check_mark:   |                      | :heavy_check_mark:   | :heavy_check_mark:   |                      |  
| ポリゴン以外の形状データ       |                      |                      |                      |                      |                      |                      |  

### 注意

* 負のスケール：一部の DCC ツールにサポートされています.  
  XYZ 全てが負の場合は正しく同期できますが、X だけ、Y だけ負のような場合も Unity 側では XYZ 全てが負として扱われてしまいます。
  DCC ツールによっては、この場合でもメッシュを同期できる *Bake Transform* オプションがありますが、デフォーマの情報は失われます。

## サポートされている DCC ツール

|                    | Windows            | Mac                | Linux              |
|--------------------| ------------------ | ------------------ |------------------- |
| Maya 2018          | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| Maya 2019          | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| Maya 2020          | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| Maya 2022          | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| Maya LT 2019 +     | :heavy_check_mark: |                    | :x:                |
| 3ds Max 2018       | :heavy_check_mark: | :x:                | :x:                |
| 3ds Max 2019       | :heavy_check_mark: | :x:                | :x:                |
| 3ds Max 2020       | :heavy_check_mark: | :x:                | :x:                |
| 3ds Max 2021       | :heavy_check_mark: | :x:                | :x:                |
| MotionBuilder 2017 | :heavy_check_mark: | :x:                | :heavy_check_mark: |
| MotionBuilder 2018 | :heavy_check_mark: | :x:                | :heavy_check_mark: |
| MotionBuilder 2019 | :heavy_check_mark: | :x:                | :heavy_check_mark: |
| MotionBuilder 2020 | :heavy_check_mark: | :x:                | :heavy_check_mark: |
| Blender 2.83       | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Blender 2.90       | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Blender 2.91       | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Blender 2.92       | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Blender 2.93       | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Blender 3.0        | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Blender 3.1        | :white_check_mark: | :white_check_mark: | :white_check_mark: |
| Modo 12            | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| Modo 13            | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| Metasequoia 4.x    | :heavy_check_mark: | :heavy_check_mark: |                    |

メモ：
* :white_check_mark: : 対応済み
* :x: : 対応することが不可能 (その OS で、DCC ツールが動作しないなど)
* empty : これから対応する可能性がある

# DCC プラグインのインストール方法

![MeshSyncPreferences](../images/MeshSyncPreferences.png)

[MeshSync](https://docs.unity3d.com/Packages/com.unity.meshsync@latest) の環境設定で、
いくつかのDCCツールに簡単にインストールできます。  
または、[手動インストール](Installation.md)もできます。
