using NUnit.Framework;
using System.IO;

namespace Unity.MeshSyncDCCPlugin {

class PluginsTest {

	[Test]
	public void DCCPluginsExist() {
		
		//[TODO-sin: 2020-4-9] Check the filename if they have the same version as MeshSync's version
		string path = Path.Combine("Packages", "com.unity.meshsync.dcc-plugins","Editor","Plugins~");
		int numFiles = Directory.GetFiles(path, "*", SearchOption.TopDirectoryOnly).Length;
		Assert.Greater(numFiles,0,"There are no DCC plugins");
	}

}
	
} //end namespace
