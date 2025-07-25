/*----------------------------------------------------------------------------------------

Copyright (c) 2025 AUDIOKINETIC Inc.

This file is licensed to use under the license available at:
https://github.com/audiokinetic/ReaWwise/blob/main/License.txt (the "License").
You may not use this file except in compliance with the License.

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License.

----------------------------------------------------------------------------------------*/

#include "ReaperContext.h"

#include "Helpers/StringHelper.h"

#include <catch2/catch_all.hpp>
#include <catch2/trompeloeil.hpp>

namespace AK::ReaWwise::Test
{
#ifdef WIN32
	juce::File projectDirectory = juce::File("C:\\MyProjectDirectory");
	juce::File otherProjectDirectory = juce::File("C:\\OtherProjectDirectory");
#else
	juce::File projectDirectory = juce::File("/MyProjectDirectory");
	juce::File otherProjectDirectory = juce::File("/OtherProjectDirectory");
#endif

	struct TestParams
	{
		TestParams(const juce::File& projectDirectory)
			: projectDirectory(projectDirectory)
		{
			resolvedDummyRenderPattern = {
				projectDirectory.getChildFile("-001.wav").getFullPathName(),
				projectDirectory.getChildFile("-002.wav").getFullPathName(),
			};

			renderTargets = {
				projectDirectory.getChildFile("audio-file-001.wav").getFullPathName(),
				projectDirectory.getChildFile("audio-file-002.wav").getFullPathName(),
			};

			resolvedOriginalsSubfolder = {
				projectDirectory.getChildFile("-001.wav").getFullPathName(),
				projectDirectory.getChildFile("-002.wav").getFullPathName(),
			};

			resolvedObjectPaths = {
				"\\Actor-Mixer Hierarchy\\Default Work Unit\\<Random Container>Footsteps\\<SoundSFX>audio-file-001.wav",
				"\\Actor-Mixer Hierarchy\\Default Work Unit\\<Random Container>Footsteps\\<SoundSFX>audio-file-002.wav",
			};

			renderStats << "FILE:" + projectDirectory.getChildFile("audio-file-001.wav").getFullPathName() + ";PEAK:-0000.000000;LRA:-0000.000000;LUFSMMAX:-0000.000000;LUFSSMAX:-0000.000000;LUFSI:-0000.000000;"
						<< "FILE:" + projectDirectory.getChildFile("audio-file-002.wav").getFullPathName() + ";PEAK:-0000.000000;LRA:-0000.000000;LUFSMMAX:-0000.000000;LUFSSMAX:-0000.000000;LUFSI:-0000.000000;";
		}

		juce::File projectDirectory;
		juce::String outputFilename;
		juce::String outputDirectory;

		std::vector<juce::String> resolvedDummyRenderPattern;
		std::vector<juce::String> renderTargets;
		std::vector<juce::String> resolvedOriginalsSubfolder;
		std::vector<juce::String> resolvedObjectPaths;

		juce::String renderStats;
	};

	class MockReaperPlugin : public trompeloeil::mock_interface<IReaperPlugin>
	{
	public:
		IMPLEMENT_CONST_MOCK0(getCallerVersion);
		IMPLEMENT_CONST_MOCK2(registerFunction);
		IMPLEMENT_CONST_MOCK0(isValid);

		IMPLEMENT_MOCK0(getMainHwnd);
		IMPLEMENT_MOCK0(addExtensionsMainMenu);
		IMPLEMENT_MOCK3(enumProjects);
		IMPLEMENT_MOCK4(getSetProjectInfo_String);
		IMPLEMENT_MOCK5(resolveRenderPattern);
		IMPLEMENT_MOCK2(main_OnCommand);
		IMPLEMENT_MOCK5(getProjExtState);
		IMPLEMENT_MOCK4(setProjExtState);
		IMPLEMENT_MOCK1(markProjectDirty);
		IMPLEMENT_MOCK1(getProjectStateChangeCount);
		IMPLEMENT_MOCK4(getSetProjectInfo);
		IMPLEMENT_MOCK2(reallocCmdRegisterBuf);
		IMPLEMENT_MOCK1(reallocCmdClear);
		IMPLEMENT_MOCK0(supportsReallocCommands);
	};

	struct GetItemsForPreviewExpectations
	{
		GetItemsForPreviewExpectations(MockReaperPlugin& plugin, const TestParams& params)
			: plugin(plugin)
			, reaproject(42)
			, reaperProjectPath(params.projectDirectory.getChildFile("test.rpp").getFullPathName())
			, outputDirectory(params.outputDirectory)
			, dummyResolvedRenderPatternDblNullTerminated(WwiseTransfer::StringHelper::createDoubleNullTerminatedStringBuffer(params.resolvedDummyRenderPattern))
			, resolvedOutputFilenameDblNullTerminated(WwiseTransfer::StringHelper::createDoubleNullTerminatedStringBuffer(params.renderTargets))
			, resolvedOriginalsSubfolderDblNullTerminated(WwiseTransfer::StringHelper::createDoubleNullTerminatedStringBuffer(params.resolvedOriginalsSubfolder))
			, resolvedObjectPathsDblNullTerminated(WwiseTransfer::StringHelper::createDoubleNullTerminatedStringBuffer(params.resolvedObjectPaths))
			, renderFile("RENDER_FILE")
			, renderPattern("RENDER_PATTERN")
			, renderTargetsEx("RENDER_TARGETS_EX")
		{
			using trompeloeil::_; // wild card for matching any value

			expectations[0] = NAMED_ALLOW_CALL(plugin, enumProjects(-1, _, _))
			                      .SIDE_EFFECT(memset(_2, '\0', size_t(reaperProjectPath.length())))
			                      .SIDE_EFFECT(memcpy(_2, reaperProjectPath.getCharPointer(), size_t(reaperProjectPath.length())))
			                      .RETURN((ReaProject*)&reaproject);

			expectations[1] = NAMED_ALLOW_CALL(plugin, supportsReallocCommands())
			                      .RETURN(false);

			expectations[2] = NAMED_REQUIRE_CALL(plugin, getSetProjectInfo_String(_, _, _, false))
			                      .TIMES(1)
			                      .WITH(juce::String(_2) == renderTargetsEx)
			                      .SIDE_EFFECT(memset(_3, '\0', size_t(resolvedOutputFilenameDblNullTerminated.size())))
			                      .SIDE_EFFECT(memcpy(_3, &resolvedOutputFilenameDblNullTerminated[0], size_t(resolvedOutputFilenameDblNullTerminated.size())))
			                      .RETURN(true)
			                      .IN_SEQUENCE(sequence);

			expectations[3] = NAMED_REQUIRE_CALL(plugin, resolveRenderPattern(_, _, _, nullptr, 0))
			                      .TIMES(1)
			                      .RETURN(dummyResolvedRenderPatternDblNullTerminated.size())
			                      .IN_SEQUENCE(sequence);

			expectations[4] = NAMED_REQUIRE_CALL(plugin, resolveRenderPattern(_, _, _, _, _))
			                      .TIMES(1)
			                      .WITH(_4 != nullptr)
			                      .SIDE_EFFECT(memset(_4, '\0', size_t(dummyResolvedRenderPatternDblNullTerminated.size())))
			                      .SIDE_EFFECT(memcpy(_4, &dummyResolvedRenderPatternDblNullTerminated[0], size_t(dummyResolvedRenderPatternDblNullTerminated.size())))
			                      .RETURN(dummyResolvedRenderPatternDblNullTerminated.size())
			                      .IN_SEQUENCE(sequence);

			expectations[5] = NAMED_REQUIRE_CALL(plugin, getSetProjectInfo_String(_, _, _, false))
			                      .TIMES(1)
			                      .WITH(juce::String(_2) == renderPattern)
			                      .SIDE_EFFECT(memset(_3, '\0', size_t(renderPattern.length())))
			                      .SIDE_EFFECT(memcpy(_3, renderPattern.getCharPointer(), size_t(renderPattern.length())))
			                      .RETURN(true)
			                      .IN_SEQUENCE(sequence);

			expectations[6] = NAMED_REQUIRE_CALL(plugin, resolveRenderPattern(_, _, _, nullptr, 0))
			                      .TIMES(1)
			                      .RETURN(resolvedOutputFilenameDblNullTerminated.size())
			                      .IN_SEQUENCE(sequence);

			expectations[7] = NAMED_REQUIRE_CALL(plugin, resolveRenderPattern(_, _, _, _, _))
			                      .TIMES(1)
			                      .WITH(_4 != nullptr)
			                      .SIDE_EFFECT(memset(_4, '\0', size_t(resolvedOutputFilenameDblNullTerminated.size())))
			                      .SIDE_EFFECT(memcpy(_4, &resolvedOutputFilenameDblNullTerminated[0], size_t(resolvedOutputFilenameDblNullTerminated.size())))
			                      .RETURN(resolvedOutputFilenameDblNullTerminated.size())
			                      .IN_SEQUENCE(sequence);

			expectations[8] = NAMED_REQUIRE_CALL(plugin, resolveRenderPattern(_, _, _, nullptr, 0))
			                      .TIMES(1)
			                      .RETURN(resolvedOriginalsSubfolderDblNullTerminated.size())
			                      .IN_SEQUENCE(sequence);

			expectations[9] = NAMED_REQUIRE_CALL(plugin, resolveRenderPattern(_, _, _, _, _))
			                      .TIMES(1)
			                      .WITH(_4 != nullptr)
			                      .SIDE_EFFECT(memset(_4, '\0', size_t(resolvedOriginalsSubfolderDblNullTerminated.size())))
			                      .SIDE_EFFECT(memcpy(_4, &resolvedOriginalsSubfolderDblNullTerminated[0], size_t(resolvedOriginalsSubfolderDblNullTerminated.size())))
			                      .RETURN(resolvedOriginalsSubfolderDblNullTerminated.size())
			                      .IN_SEQUENCE(sequence);

			expectations[10] = NAMED_REQUIRE_CALL(plugin, resolveRenderPattern(_, nullptr, _, nullptr, 0))
			                       .TIMES(1)
			                       .RETURN(resolvedObjectPathsDblNullTerminated.size())
			                       .IN_SEQUENCE(sequence);

			expectations[11] = NAMED_REQUIRE_CALL(plugin, resolveRenderPattern(_, nullptr, _, _, _))
			                       .TIMES(1)
			                       .WITH(_4 != nullptr)
			                       .SIDE_EFFECT(memset(_4, '\0', size_t(resolvedObjectPathsDblNullTerminated.size())))
			                       .SIDE_EFFECT(memcpy(_4, &resolvedObjectPathsDblNullTerminated[0], size_t(resolvedObjectPathsDblNullTerminated.size())))
			                       .RETURN(resolvedObjectPathsDblNullTerminated.size())
			                       .IN_SEQUENCE(sequence);
		}

	protected:
		MockReaperPlugin& plugin;
		trompeloeil::sequence sequence;
		int reaproject;
		juce::String reaperProjectPath;

	private:
		juce::String outputDirectory;
		std::vector<char> dummyResolvedRenderPatternDblNullTerminated;
		std::vector<char> resolvedOutputFilenameDblNullTerminated;
		std::vector<char> resolvedOriginalsSubfolderDblNullTerminated;
		std::vector<char> resolvedObjectPathsDblNullTerminated;
		juce::String renderFile;
		juce::String renderPattern;
		juce::String renderTargetsEx;

		std::array<std::unique_ptr<trompeloeil::expectation>, 12> expectations;
	};

	struct GetItemsForImportExpectations : private GetItemsForPreviewExpectations
	{
		GetItemsForImportExpectations(MockReaperPlugin& plugin, const TestParams& params)
			: GetItemsForPreviewExpectations(plugin, params)
			, renderStatsKey("RENDER_STATS")
			, renderStats(params.renderStats)
		{
			using trompeloeil::_; // wild card for matching any value

			expectations[0] = NAMED_REQUIRE_CALL(plugin, getSetProjectInfo_String(_, _, _, false))
			                      .TIMES(1)
			                      .WITH(juce::String(_2) == renderStatsKey)
			                      .SIDE_EFFECT(memset(_3, '\0', size_t(renderStats.length())))
			                      .SIDE_EFFECT(memcpy(_3, renderStats.getCharPointer(), size_t(renderStats.length())))
			                      .RETURN(true)
			                      .IN_SEQUENCE(sequence);
		}

	private:
		juce::String renderStatsKey;
		juce::String renderStats;
		std::array<std::unique_ptr<trompeloeil::expectation>, 1> expectations;
	};

	std::vector<WwiseTransfer::Import::PreviewItem> getItemsForPreview(const TestParams& params)
	{
		WwiseTransfer::Import::Options importOptions{"", "", ""};

		MockReaperPlugin plugin;
		ReaperContext reaperContext(plugin);

		GetItemsForPreviewExpectations expectations(plugin, params);

		return reaperContext.getItemsForPreview(importOptions);
	}

	std::vector<WwiseTransfer::Import::Item> getItemsForImport(const TestParams& params)
	{
		WwiseTransfer::Import::Options importOptions{"", "", ""};

		MockReaperPlugin plugin;
		ReaperContext reaperContext(plugin);

		GetItemsForImportExpectations expectations(plugin, params);

		return reaperContext.getItemsForImport(importOptions);
	}

	SCENARIO("ReaperContext getItemsForImport")
	{
		TestParams params(projectDirectory);

		GIVEN("Default test params")
		{
			WHEN("Render stats contains a semi-colon at the end")
			{
				auto importItems = getItemsForImport(params);

				THEN("The resulting render file paths must be parsed correctly")
				{
					REQUIRE(importItems.size() == 2);
					REQUIRE(importItems[0].renderFilePath == projectDirectory.getChildFile("audio-file-001.wav").getFullPathName());
					REQUIRE(importItems[1].renderFilePath == projectDirectory.getChildFile("audio-file-002.wav").getFullPathName());
				}
			}

			AND_WHEN("Render stats does not contain semi-colon at the end")
			{
				params.renderStats.clear();
				params.renderStats << "FILE:" + projectDirectory.getChildFile("audio-file-001.wav").getFullPathName() + ";PEAK:-0000.000000;LRA:-0000.000000;LUFSMMAX:-0000.000000;LUFSSMAX:-0000.000000;LUFSI:-0000.000000;"
								   << "FILE:" + projectDirectory.getChildFile("audio-file-002.wav").getFullPathName() + ";PEAK:-0000.000000;LRA:-0000.000000;LUFSMMAX:-0000.000000;LUFSSMAX:-0000.000000;LUFSI:-0000.000000";

				auto importItems = getItemsForImport(params);

				THEN("The resulting render file paths must be parsed correctly")
				{
					REQUIRE(importItems.size() == 2);
					REQUIRE(importItems[0].renderFilePath == projectDirectory.getChildFile("audio-file-001.wav").getFullPathName());
					REQUIRE(importItems[1].renderFilePath == projectDirectory.getChildFile("audio-file-002.wav").getFullPathName());
				}
			}

			AND_WHEN("Render stats contains some unexpected text before the first \"FILE:\"")
			{
				params.renderStats.clear();
				params.renderStats << "Unex:pect;edTextFILE:" + projectDirectory.getChildFile("audio-file-001.wav").getFullPathName() + ";PEAK:-0000.000000;LRA:-0000.000000;LUFSMMAX:-0000.000000;LUFSSMAX:-0000.000000;LUFSI:-0000.000000;"
								   << "FILE:" + projectDirectory.getChildFile("audio-file-002.wav").getFullPathName() + ";PEAK:-0000.000000;LRA:-0000.000000;LUFSMMAX:-0000.000000;LUFSSMAX:-0000.000000;LUFSI:-0000.000000";

				auto importItems = getItemsForImport(params);

				THEN("The resulting render file paths must be parsed correctly")
				{
					REQUIRE(importItems.size() == 2);
					REQUIRE(importItems[0].renderFilePath == projectDirectory.getChildFile("audio-file-001.wav").getFullPathName());
					REQUIRE(importItems[1].renderFilePath == projectDirectory.getChildFile("audio-file-002.wav").getFullPathName());
				}
			}

			AND_WHEN("Render stats does not contain \"FILE:\"")
			{
				params.renderStats.clear();
				params.renderStats << projectDirectory.getChildFile("audio-file-001.wav").getFullPathName() + ";PEAK:-0000.000000;LRA:-0000.000000;LUFSMMAX:-0000.000000;LUFSSMAX:-0000.000000;LUFSI:-0000.000000;"
								   << projectDirectory.getChildFile("audio-file-002.wav").getFullPathName() + ";PEAK:-0000.000000;LRA:-0000.000000;LUFSMMAX:-0000.000000;LUFSSMAX:-0000.000000;LUFSI:-0000.000000";

				auto importItems = getItemsForImport(params);

				THEN("No import items are returned")
				{
					REQUIRE(importItems.size() == 0);
				}
			}

			AND_WHEN("Render stats is empty")
			{
				params.renderStats.clear();

				auto importItems = getItemsForImport(params);

				THEN("No import items are returned")
				{
					REQUIRE(importItems.size() == 0);
				}
			}
		}
	}

	SCENARIO("ReaperContext getItemsForPreview")
	{
		TestParams params(projectDirectory);

		const juce::String upOneDirectorySymbol = ".." + juce::File::getSeparatorString();

		GIVEN("Default test params")
		{
			THEN("The resulting audio file will match the render target returned by REAPER")
			{
				auto previewItems = getItemsForPreview(params);

				REQUIRE(previewItems.size() == 2);
				REQUIRE(previewItems[0].audioFilePath == params.renderTargets[0]);
				REQUIRE(previewItems[1].audioFilePath == params.renderTargets[1]);
			}

			AND_WHEN("The configured originals subfolder is empty")
			{
				THEN("The originals subfolder parameter for the import item should be empty")
				{
					auto previewItems = getItemsForPreview(params);

					REQUIRE(previewItems.size() == 2);
					REQUIRE(previewItems[0].originalsSubFolder.isEmpty());
					REQUIRE(previewItems[1].originalsSubFolder.isEmpty());
				}
			}

			AND_WHEN("The configured originals subfolder is not empty")
			{
				juce::String expectedOriginalsSubfolder = "OriginalsSubfolder";

				params.resolvedOriginalsSubfolder = {
					projectDirectory.getChildFile(expectedOriginalsSubfolder).getChildFile("-001.wav").getFullPathName(),
					projectDirectory.getChildFile(expectedOriginalsSubfolder).getChildFile("-002.wav").getFullPathName(),
				};

				THEN("The originals subfolder parameter for the import item should match the configured originals subfolder")
				{
					auto previewItems = getItemsForPreview(params);

					REQUIRE(previewItems.size() == 2);
					REQUIRE(previewItems[0].originalsSubFolder == expectedOriginalsSubfolder);
					REQUIRE(previewItems[1].originalsSubFolder == expectedOriginalsSubfolder);
				}

				AND_WHEN("The audio file path contains subdirectories")
				{
					params.renderTargets = {
						projectDirectory.getChildFile("AudioFolder").getChildFile("audio-file-001.wav").getFullPathName(),
						projectDirectory.getChildFile("AudioFolder").getChildFile("audio-file-002.wav").getFullPathName(),
					};

					THEN("The originals subfolder for the preview item should be a combination of the configured originals subfolder and the audio file path's folder relative to the render directory")
					{
						auto previewItems = getItemsForPreview(params);

						REQUIRE(previewItems.size() == 2);
						REQUIRE(previewItems[0].originalsSubFolder == juce::String("OriginalsSubfolder") + juce::File::getSeparatorChar() + "AudioFolder");
						REQUIRE(previewItems[1].originalsSubFolder == juce::String("OriginalsSubfolder") + juce::File::getSeparatorChar() + "AudioFolder");
					}
				}

				AND_WHEN("The audio file path contains " + upOneDirectorySymbol)
				{
					params.renderTargets = {
						projectDirectory.getChildFile(upOneDirectorySymbol + "audio-file-001.wav").getFullPathName(),
						projectDirectory.getChildFile(upOneDirectorySymbol + "audio-file-002.wav").getFullPathName(),
					};

					THEN("The originals subfolder for the preview item should be empty since it would be cancelled out due to the " + upOneDirectorySymbol)
					{
						auto previewItems = getItemsForPreview(params);

						REQUIRE(previewItems.size() == 2);
						REQUIRE(previewItems[0].originalsSubFolder.isEmpty());
						REQUIRE(previewItems[1].originalsSubFolder.isEmpty());
					}
				}
			}

			AND_WHEN("The audio file path contains a semi-colon")
			{
				params.renderTargets = {
					projectDirectory.getChildFile("audio;-file-001.wav").getFullPathName(),
					projectDirectory.getChildFile("audio;-file-002.wav").getFullPathName(),
				};

				THEN("The resulting audio file path should match the expected value")
				{
					auto previewItems = getItemsForPreview(params);

					REQUIRE(previewItems.size() == 2);
					REQUIRE(previewItems[0].audioFilePath == projectDirectory.getChildFile("audio;-file-001.wav").getFullPathName());
					REQUIRE(previewItems[1].audioFilePath == projectDirectory.getChildFile("audio;-file-002.wav").getFullPathName());
				}
			}

			AND_WHEN("The audio file path contains an extra period")
			{
				params.renderTargets = {
					projectDirectory.getChildFile("audio.file-001.wav").getFullPathName(),
					projectDirectory.getChildFile("audio.file-002.wav").getFullPathName(),
				};

				THEN("The resulting audio file path should match the expected value")
				{
					auto previewItems = getItemsForPreview(params);

					REQUIRE(previewItems.size() == 2);
					REQUIRE(previewItems[0].audioFilePath == projectDirectory.getChildFile("audio.file-001.wav").getFullPathName());
					REQUIRE(previewItems[1].audioFilePath == projectDirectory.getChildFile("audio.file-002.wav").getFullPathName());
				}
			}

			AND_WHEN("The object path contains an extra period")
			{
				params.resolvedObjectPaths = {
					"\\Actor-Mixer Hierarchy\\Default Work Unit\\<Random Container>Footsteps\\<SoundSFX>audio.file-001.wav",
					"\\Actor-Mixer Hierarchy\\Default Work Unit\\<Random Container>Footsteps\\<SoundSFX>audio.file-002.wav",
				};

				THEN("The resulting object path should be the resolved object path without the file extension")
				{
					auto previewItems = getItemsForPreview(params);

					REQUIRE(previewItems.size() == 2);
					REQUIRE(previewItems[0].path == "\\Actor-Mixer Hierarchy\\Default Work Unit\\<Random Container>Footsteps\\<SoundSFX>audio.file-001");
					REQUIRE(previewItems[1].path == "\\Actor-Mixer Hierarchy\\Default Work Unit\\<Random Container>Footsteps\\<SoundSFX>audio.file-002");
				}
			}
		}
	}
} // namespace AK::ReaWwise::Test
