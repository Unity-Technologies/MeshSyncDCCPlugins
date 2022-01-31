using UnityEngine;
using UnityEditor;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor.PackageManager;
using PackageInfo = UnityEditor.PackageManager.PackageInfo;

public static class PackageChecker {
    
    [InitializeOnLoadMethod]
    static void PackageChecker_OnLoad() {
        PackageRequestJobManager.CreateListRequest(offlineMode: false, includeIndirectIndependencies:false, 
            onSuccess: (reqResult) => {
                PackageCollection result = reqResult.Result;
                foreach (PackageInfo packageInfo in result) {
                    if (packageInfo.name != "com.unity.meshsync.dcc-plugins")
                        continue;

                    m_packageVersion = packageInfo.version;
                }
            }, null
        );
    }

    public static string GetPackageVersion() => m_packageVersion;
    
//----------------------------------------------------------------------------------------------------------------------

    private static string m_packageVersion = null;

    

}
