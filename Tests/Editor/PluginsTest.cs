using NUnit.Framework;
using System.IO;

namespace Unity.MeshSyncDCCPlugin {

class PluginsTest {

	[Test]
	public void DCCPluginsExist() {
		
		string path = Path.Combine("Packages", "com.unity.meshsync.dcc-plugins","Editor","Plugins~");
		int numFiles = Directory.GetFiles(path, "*", SearchOption.TopDirectoryOnly).Length;
		Assert.Greater(numFiles,0,"There are no DCC plugins");
	}

}
	
} //end namespace
