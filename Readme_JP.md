![demo](Documentation~/images/Demo.gif)

# Other Languages
- [English](Readme.md)

# MeshSync DCC Plugins

[![](https://badge-proxy.cds.internal.unity3d.com/b681f940-bd27-45c9-832f-e87e6282aa9f)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync.dcc-plugins/build-info?branch=dev&testWorkflow=package-isolation)
[![](https://badge-proxy.cds.internal.unity3d.com/6b18f37e-3925-4b8d-a243-2582b0077f47)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync.dcc-plugins/dependencies-info?branch=dev&testWorkflow=updated-dependencies)
[![](https://badge-proxy.cds.internal.unity3d.com/0db3f8f4-b2d4-40f8-b08a-0b65bcc02245)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync.dcc-plugins/dependants-info)
[![](https://badge-proxy.cds.internal.unity3d.com/1c9fbd13-0736-40ed-96d5-89f237ce91ca)](https://badges.cds.internal.unity3d.com/packages/com.unity.meshsync.dcc-plugins/warnings-info?branch=dev)

![ReleaseBadge](https://badge-proxy.cds.internal.unity3d.com/2e9a8300-389b-47be-9806-246c5121830b)
![ReleaseBadge](https://badge-proxy.cds.internal.unity3d.com/6f4a7e27-d53f-4ad3-bef5-9d3961bb68fb)

DCC ツール上のモデルの編集をリアルタイムに Unity に反映させるためのパッケージ: 
[MeshSync](https://docs.unity3d.com/Packages/com.unity.meshsync@latest) と連携するための DCC プラグインのソースをこのリポジトリで提供します。  
MeshSync と MeshSyncDCCPlugin が連携することで、ゲーム上でどう見えるかをその場で確認しながらモデリングすることができます。

# フィーチャー

|                              | Maya                 | 3ds Max              | MotionBuilder        | Blender              | Modo                 | Metasequoia          | 
| -----------------------------| -------------------- | -------------------- | -------------------- | -------------------- | -------------------- | -------------------- | 
| ポリゴンメッシュの同期         | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |  
| カメラの同期                  | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |  
| ライトの同期                  | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |  
| 両面 (Double-sided) メッシュ  | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   | :heavy_check_mark:   |  
| 負のスケール                  | :small_red_triangle: | :small_red_triangle: | :small_red_triangle: | :small_red_triangle: | :small_red_triangle: |                      |
| マルチ UV                     |                      |                      |                      | :heavy_check_mark:   |                      |                      |  
| Scene Cache Export           | :heavy_check_mark:   | :heavy_check_mark:   |                      | :heavy_check_mark:   | :heavy_check_mark:   |                      |  
| Non-polygon shape            |                      |                      |                      |                      |                      |                      |  

## 注意

* 負のスケール：一部の DCC ツールにサポートされています.  
  XYZ 全てが負の場合は正しく同期できますが、X だけ、Y だけ負のような場合も Unity 側では XYZ 全てが負として扱われてしまいます。
  DCC ツールによっては、この場合でもメッシュを同期できる *Bake Transform* オプションがありますが、デフォーマの情報は失われます。


## サポートされている DCC ツール

|                     | Windows            | Mac                | Linux              | 
| --------------------| ------------------ | ------------------ |------------------- | 
| Maya 2017           | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Maya 2018           | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Maya 2019           | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Maya 2020           | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Maya LT 2019 +      | :heavy_check_mark: |                    | :x:                | 
| 3ds Max 2017        | :heavy_check_mark: | :x:                | :x:                | 
| 3ds Max 2018        | :heavy_check_mark: | :x:                | :x:                | 
| 3ds Max 2019        | :heavy_check_mark: | :x:                | :x:                | 
| 3ds Max 2020        | :heavy_check_mark: | :x:                | :x:                | 
| MotionBuilder 2017  | :heavy_check_mark: | :x:                | :heavy_check_mark: | 
| MotionBuilder 2018  | :heavy_check_mark: | :x:                | :heavy_check_mark: | 
| MotionBuilder 2019  | :heavy_check_mark: | :x:                | :heavy_check_mark: | 
| MotionBuilder 2020  | :heavy_check_mark: | :x:                | :heavy_check_mark: | 
| Blender 2.79b       | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Blender 2.80        | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Blender 2.81        | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Blender 2.82        | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Blender 2.83        | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Blender 2.90        | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Modo 12             | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Modo 13             | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | 
| Metasequoia 4.x     | :heavy_check_mark: | :heavy_check_mark: |                    | 

メモ：
* :heavy_check_mark: : 対応済み
* :x: : 対応することが不可能 (その OS で、DCC ツールが動作しないなど)
* empty : これから対応する可能性がある

# DCC プラグインのインストール方法

![MeshSyncPreferences](Documentation~/images/MeshSyncPreferences.png)

[MeshSync](https://docs.unity3d.com/Packages/com.unity.meshsync@latest) の環境設定で、
いくつかのDCCツールに簡単にインストールできます。  
または、[手動インストール](Documentation~/jp/Installation.md)もできます。

 
# Building
- [DCC プラグインをビルド](Plugins~/Docs/en/BuildDCCPlugins.md) (現在英語のみ)

# License
- [License](LICENSE.md)
- [Third Party Notices](Third%20Party%20Notices.md)

# 現在整理中

このドキュメントは現在整理中です。
ご参考までに、前のバージョンのドキュメントは下記においてあります。

## 使い方
1. [Maya](#maya)
2. [Maya LT](#maya-lt)
3. [3ds Max](#3ds-max)
4. [MotionBuilder](#motionbuilder)
5. [Blender](#blender)
6. [Modo](#modo)
7. [メタセコイア](#メタセコイア)
8. [Unity](#unity)

<img align="right" src="https://user-images.githubusercontent.com/1488611/39971860-7f6d1330-573e-11e8-9a1e-9d95709cbd50.png" height=400>

### Maya
Maya 2016, 2016.5, 2017, 2018, 2019 + Windows, Mac, Linux (CentOS 7) で動作を確認しています。
- インストール：
  - Windows: %MAYA_APP_DIR% が設定されている場合はそこに、ない場合は %USERPROFILE%\Documents\maya (←を Explorer のアドレスバーへコピペで直行) に modules ディレクトリをそのままコピー。
    - 2016 以前の場合はバージョン名のディレクトリへコピーします。(%MAYA_APP_DIR%\2016 など)
  - Mac: /Users/Shared/Autodesk/modules/maya に UnityMeshSync ディレクトリと .mod ファイルをコピー。
  - Linux: ~/maya/(Maya のバージョン) に modules ディレクトリをそのままコピー。
- Maya を起動し、Windows -> Settings/Preferences -> Plug-in Manager を開き、MeshSyncClient の Loaded にチェックを入れてプラグインを有効化します。
- UnityMeshSync シェルフが追加されているので、それの歯車アイコンで設定メニューを開きます。
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます。
- Animations の Sync を押すと、開始フレームから終了フレームまで時間を進めつつアニメーションをベイクして Unity に送ります。
- "Export Cache" で全フレームのデータをファイルにエクスポートできます。エクスポートしたファイルは Unity で再生できます。より詳しくは [Scene Cache](Documentation~/SceneCache.md) を参照ください。
- 歯車アイコン以外のボタンはそれぞれ手動同期、アニメーション同期相当のボタンになっています。

&nbsp;  

- ポリゴンメッシュはスキニング/ボーン (SkinCluster) と BlendShape もそのまま Unity へ持ってこれるようになっています。
  - これら以外のデフォーマも適用を試みますが、前後に SkinCluster があった場合などに正しく適用されない可能性があります。
  - "Bake Deformers" をチェックすると、デフォーマを全て適用した結果を同期します。Maya 側と Unity 側で Mesh の内容がほぼ一致するようになりますが、代償として Skinning や Blendshape の情報が失われます。
  - "Bake Transform" をチェックすると、位置/回転/スケールを Mesh の頂点に適用し、Unity 側の Transform は初期値になります。pivot が絡む複雑な Transform は Unity では再現できないことがありますが、そのような場合でもこのオプションを使うと Mesh の見た目は一致するようになります。このオプションは "Bake Deformers" が有効なときのみ有効です。
- NURBS などポリゴン以外の形状データは対応していません。
- インスタンシングは対応していますが、スキニングされたメッシュのインスタンスは現在未対応です (Unity 側では全て元インスタンスと同じ位置になっていまいます)。
- MEL にもコマンドが登録されており、全ての機能に MEL 経由でアクセスできるようになっています。こちらの詳細は[ソースコードを参照ください](https://github.com/unity3d-jp/MeshSync/blob/master/Plugin~/MeshSyncClientMaya/msmayaCommands.cpp)。


### Maya LT
現在 Windows のみ対応で、Maya LT 2019 + Windows で動作を確認しています。Maya LT は本来外部プラグインをサポートしないため、問題が起きる可能性が高いことに留意ください。Maya LT 側のマイナーバージョンアップでも互換性が失われる可能性が考えられます。  
パッケージは別になっているものの、インストールや使い方は [非 LT の Maya](#maya) と同じです。


### 3ds Max
3ds Max 2016, 2017, 2018, 2019, 2020 + Windows で動作を確認しています。
- インストール：
  - MeshSyncClient3dsMax.dlu をプラグイン用パスとして登録されているディレクトリにコピー
    - プラグイン用パスは max 内の Customize -> Configure User and System Paths -> 3rd Party Plug-Ins の Add で追加できます
    - デフォルトで用意されているパス (C:\Program Files\Autodesk\3ds Max 2019\Plugins など) でもおそらく機能しますが、デフォルトとそれ以外で別のパスを用意しておくことをおすすめします
- インストール後、メインメニューバーに "UnityMeshSync" が追加されているので、それの "Window" から設定ウィンドウを開けます。
  - メニューバーを編集する場合、Action に "UnityMeshSync" カテゴリが追加されているので、そちらから MeshSync の機能にアクセスできます
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます。
- Animations の Sync を押すと、開始フレームから終了フレームまで時間を進めつつアニメーションをベイクして Unity に送ります。
- "Export Cache" で全フレームのデータをファイルにエクスポートできます。エクスポートしたファイルは Unity で再生できます。より詳しくは [Scene Cache](Documentation~/SceneCache.md) を参照ください。

&nbsp;  

- モディファイアは大体対応していますが、対応できないケースもあります。以下のルールに従います。
  - Morph も Skin もない場合、全てのモディファイアを適用した状態で同期します。
  - Morph か Skin がある場合、その一つ前までのモディファイアを適用した状態で同期します。
    - Morph / Skin が複数ある場合、一番下のものが基準として選ばれます。
  - Morh と Skin は Unity 側にそのまま Blendshape / Skin として同期します。
    - Unity 側では常に Blendshape -> Skin の順番で適用されるため、Max 側で順番が逆だと意図しない結果になる可能性があります。
  - "Bake Modifiers" をチェックすると、モディファイアを適用した結果を送ります。Max 側と Unity 側で Mesh の内容がほぼ一致するようになりますが、代償として Skinning や Blendshape の情報が失われます。
  - "Bake Transform" をチェックすると、位置/回転/スケールを Mesh の頂点に適用し、Unity 側の Transform は初期値になります。pivot が絡む複雑な Transform は Unity では再現できないことがありますが、そのような場合でもこのオプションを使うと Mesh の見た目は一致するようになります。このオプションは "Bake Modifiers" が有効なときのみ有効です。
  - "Use Render Meshes" をチェックすると、レンダリング用の Mesh からデータを抽出します。例えば Turbo Smooth は viewport 用とレンダリング用で別の Iteration を指定できますが、レンダリング用の設定が Unity 側に反映されるようになります。また、Fluid などのレンダリング時にしか現れない Mesh や、Space Warps なども正しく反映されるようになります。
- "Ignore Non-Rebderable" をチェックすると、renderable ではない Mesh を無視します。例えばボーンの viewport の表示の四角錐のような形状などが renderable ではない Mesh に該当します。
- Max script にもコマンドが追加されており、全ての機能に Max script 経由でアクセスできるようになっています。こちらの詳細は[ソースコードを参照ください](https://github.com/unity3d-jp/MeshSync/blob/master/Plugin~/MeshSyncClient3dsMax/msmaxEntryPoint.cpp)


<img align="right" src="https://user-images.githubusercontent.com/1488611/45682175-8a919100-bb7a-11e8-96a1-efe2e28146c3.png" height=200>

### MotionBuilder
MotionBuilder 2016, 2017, 2018, 2019 + Windows, Linux (CentOS 7) で動作を確認しています
- インストール：
  - MeshSyncClientMotionBuilder.dll をプラグイン用パスとして登録されているディレクトリにコピー
    - プラグイン用パスは MotionBuilder 内の Settings -> Preferences -> SDK メニューから追加できます
- インストール後、Asset Browser 内の Templates -> Devices に UnityMeshSync というオブジェクトが追加されているので、それをシーンに追加します
- Navigator 内の Devices -> UnityMeshSync を選択することで各種設定や機能にアクセスできます 
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます
- Animations の Sync を押すと、開始フレームから終了フレームまで時間を進めつつアニメーションをベイクして Unity に送ります。

&nbsp;  

- ポリゴンメッシュはスキニング/ボーンと BlendShape もそのまま Unity へ持ってこれるようになっています。
- NURBS などポリゴン以外の形状データは対応していません


<img align="right" src="https://user-images.githubusercontent.com/1488611/49272332-79d39480-f4b4-11e8-8ca3-0ce0bc90a965.png" height=400>

### Blender
Blender 2.79b, 2.80 + Windows, Mac, Linux (CentOS 7) で動作を確認しています。[開発版の Blender](https://builder.blender.org/download/) はサポート外で、ほぼ動作しません。  
実装の都合上、**Blender のバージョンが上がると互換性が失われる可能性が高い** ことにご注意ください。[Blender version issue](https://github.com/unity3d-jp/MeshSync/issues/89) で最新の状況や hot fix を提供予定です。
- インストール：
  - Blender 側で File -> User Preferences -> Add-ons (2.80 以降では Edit -> User Preferences) を開き、画面下部の "Install Add-on from file" を押し、プラグインの zip ファイルを指定します。
  - **古いバージョンをインストール済みの場合、事前に削除しておく必要があります**。Add-ons メニューから "Import-Export: Unity Mesh Sync" を選択して  **Remove した後、blender を再起動** してから上記手順を踏んでください。
- "Import-Export: Unity Mesh Sync" が追加されるので、チェックを入れて有効化します。
- MeshSync パネルが追加されるので、そちらから設定や手動の同期を行います。
  - 2.8 系ではパネルの場所がややわかりにくい場所になっています。右スクリーンショットを参照ください。
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます。
- Animations の Sync を押すと、開始フレームから終了フレームまで時間を進めつつアニメーションをベイクして Unity に送ります。
- "Export Cache" で全フレームのデータをファイルにエクスポートできます。エクスポートしたファイルは Unity で再生できます。より詳しくは [Scene Cache](Documentation~/SceneCache.md) を参照ください。

&nbsp;  

- ポリゴンメッシュはスキニング/ボーン (Armature) と BlendShape もそのまま Unity へ持ってこれるようになっています。Mirror デフォーマも対応しています。これら以外のモディファイアは無視されます。
  - "Bake Modifiers" をチェックすると、モディファイアを全て適用した結果を同期します。Blender 側と Unity 側で Mesh の内容がほぼ一致するようになりますが、代償として Skinning や Blendshape の情報が失われます。
  - "Bake Transform" をチェックすると、位置/回転/スケールを Mesh の頂点に適用し、Unity 側の Transform は初期値になります。pivot が絡む複雑な Transform は Unity では再現できないことがありますが、そのような場合でもこのオプションを使うと Mesh の見た目は一致するようになります。このオプションは "Bake Modifiers" が有効なときのみ有効です。
- "Curves as Mesh" をチェックすると、Curve や Text などポリゴンに変換可能なオブジェクトを変換して同期します。


### Modo

<img src="https://user-images.githubusercontent.com/1488611/55697991-d9135980-59fe-11e9-8e9f-8fcfba1b234f.png" height=300><img src="https://user-images.githubusercontent.com/1488611/55697990-d9135980-59fe-11e9-9312-29c95e20e5b0.png" height=300>

Modo 12, 13 + Windows, Mac, Linux (CentOS 7) で動作を確認しています。
- インストール：
  - Modo 内の System -> Add Plug-in で MeshSyncClientModo.fx を指定
  - **古いバージョンをインストール済みの場合、古いプラグインをロードしていない状態で再度上記手順を踏む必要があります**。プラグインがロードされるタイミングは主に下記 view (Application -> Custom View -> UnityMeshSync) を表示したタイミングなので、view を出していない状態で一度 modo を終了、再度起動、プラグインをインストール、とすると確実です。
- インストール後は新たな View が追加されており、ここから各種設定や機能にアクセスできます (Application -> Custom View -> UnityMeshSync)
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます
- Animations の Sync を押すと、開始フレームから終了フレームまで時間を進めつつアニメーションをベイクして Unity に送ります。

&nbsp;

- Mesh Instance や Replicator も部分的にサポートしています。
- ポリゴンメッシュはスキニング / Joint と Morph も Unity へ持ってこれるようになっていますが、デフォーマの扱いには注意が必要です。
  - MeshSync が解釈できるデフォーマは Joint + Weight Map 方式のスキニング、および Morph のみです。それ以外のデフォーマは無視されます。
  - "Bake Deformers" をチェックすると、デフォーマを全て適用した結果を送ります。複雑なデフォーマ構成であっても Unity 側の Mesh の内容がほぼ一致するようになりますが、代償としてスキニングや Morph/Blendshape の情報が失われます。
  - "Bake Transform" をチェックすると、位置/回転/スケールを Mesh の頂点に適用し、Unity 側の Transform は初期値になります。pivot が絡む複雑な Transform は Unity では再現できないことがありますが、そのような場合でもこのオプションを使うと Mesh の見た目は一致するようになります。このオプションは "Bake Deformers" が有効なときのみ有効です。
  - Mesh Instance や Replicator のスキニングは正しく Unity 側に反映できません。"Bake Deformers" を使う必要があります。
- コマンドからも MeshSync の機能にアクセスできます。unity.meshsync.settings で設定の変更、unity.meshsync.export でエクスポートできます
- "Export Cache" で全フレームのデータをファイルにエクスポートできます。エクスポートしたファイルは Unity で再生できます。より詳しくは [Scene Cache](Documentation~/SceneCache.md) を参照ください。

&nbsp;

Modo は 13 以降 [Modo Bridge for Unity](https://learn.foundry.com/modo/content/help/pages/appendices/modo_bridge.html) という機能が搭載されており、Unity に直接 Mesh や Material を送ることができるようになっています。MeshSync と機能的に近い部分もありますが、以下のような違いがあります。(2019/04 現在)
- Modo Bridge は Modo <-> Unity の双方向の同期をサポートします。MeshSync は Modo -> Unity の一方向のみをサポートします。
- MeshSync は Replicator、Mesh の Skinning/Morph、アニメーションを同期できます。Mood Bridge は現状これらはサポートしていません。
- MeshSync は できるだけ FBX 経由で Unity にデータを持っていった時と近い結果になるように努めています。一方、Modo Bridge では座標系が異なる (Z 方向が反転する)、Mesh のインデックスが展開されている (1000 triangles のモデルは 3000 頂点になっている) などの顕著な違いが出ます。


### メタセコイア
Windows 版 3 系と 4 系 (32bit & 64bit)、Mac 版 (4 系のみ) に対応しています。3 系はたぶん全てのバージョンに対応していますが、4 系は 4.6.4 以上である必要があります。(このバージョン以上でないとボーンの出力がサポートできないため)
また、4.7 系以降用は dll が別になっています。これは 4.7 でボーンの仕様が変わり、プラグインの互換性が失われたためです。4.7 ではモーフの出力にも対応しています。
- インストール：
  - メタセコイア側で Help -> About Plug-ins を開き、ダイアログ左下の "Install" からプラグインファイルを指定します。ちなみにプラグインのタイプは Station です。
  - **古いバージョンをインストール済みの場合、事前に手動で削除しておく必要があります**。メタセコイアを起動していない状態で該当ファイルを削除、または直接新しい dll で置き換えてください。
- インストール後 パネル -> Unity Mesh Sync が追加されるのでこれを開き、"Auto Sync" をチェックします。
- "Auto Sync" がチェックされている間は編集が自動的に Unity 側に反映されます。Auyo Sync が無効でも "Manual Sync" ボタンを押すことで手動で反映できます。
- "Sync Camera" をチェックすると、パースペクティブビューのカメラを同期します。"Camera Path" が Unity 側のカメラのパスになります。
- "Import Unity Scene" を押すと現在 Unity で開かれているシーンをインポートすることができます。インポートしたシーンの編集もリアルタイムに反映可能です。

&nbsp;

- ミラーリング、スムーシングは Unity にも反映されます。
  - ただし、ミラーリングの "左右を接続した鏡面" は非サポートです。
- メタセコイアで非表示のオブジェクトは Unity でも非表示になります。非表示のオブジェクトはメッシュの内容は送られないので、シーン内にオブジェクトが増えて同期が重くなってきた場合適切に非表示にすることで同期も速くなるはずです。
- マテリアルは Unity には反映されませんが、マテリアル ID に応じて適切にサブメッシュに分割されます。
- サブディビジョンやメタボールはフリーズするまで Unity には反映されません。
- メタセコイア 4 系でサポートされた法線の編集は "Sync Normals" にチェックを入れることで反映できます。
- メタセコイア 4 系でサポートされたボーンは "Sync Bones" にチェックを入れることで反映できます。 "Sync Poses" にチェックを入れると "スキニング" で設定したポーズも反映します。


##  関連
- [NormalPainter](https://github.com/unity3d-jp/NormalPainter): Unity 上で法線を編集できるようにするツール
- [BlendShapeBuilder](https://github.com/unity3d-jp/BlendShapeBuilder): Unity 上で BlendShape を構築できるようにするツール

