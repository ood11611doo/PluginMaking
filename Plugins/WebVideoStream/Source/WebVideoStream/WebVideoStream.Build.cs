using System.IO;
using UnrealBuildTool;

public class WebVideoStream : ModuleRules
{
	public WebVideoStream(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", "CoreUObject", "Engine", "InputCore", "RHI", "RenderCore", "Projects", "Http", "Json", "JsonUtilities"
		});

		string ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/LibVLC/"));
		string LibPath = Path.Combine(ThirdPartyPath, "Lib");
		string IncludePath = Path.Combine(ThirdPartyPath, "Include");

		PublicIncludePaths.Add(IncludePath);
		PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libvlc.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libvlccore.lib"));

		RuntimeDependencies.Add("$(BinaryOutputDir)/libvlc.dll", Path.Combine(LibPath, "libvlc.dll"));
		RuntimeDependencies.Add("$(BinaryOutputDir)/libvlccore.dll", Path.Combine(LibPath, "libvlccore.dll"));

		string PluginSourcePath = Path.Combine(LibPath, "plugins");
		if (Directory.Exists(PluginSourcePath))
		{
			foreach (string RuntimeFile in Directory.GetFiles(PluginSourcePath, "*.*", SearchOption.AllDirectories))
			{
				string RelativePath = Path.GetRelativePath(LibPath, RuntimeFile);
				RuntimeDependencies.Add("$(BinaryOutputDir)/" + RelativePath, RuntimeFile);
			}
		}
	}
}