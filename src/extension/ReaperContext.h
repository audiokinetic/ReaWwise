#pragma once

#include "Core/DawContext.h"
#include "Model/Import.h"

class ReaProject;
namespace AK::ReaWwise
{
	class ReaperPluginInterface
	{
	public:
		virtual ~ReaperPluginInterface() = default;

		virtual int getCallerVersion() const = 0;
		virtual int registerFunction(const char* name, void* infoStruct) const = 0;
		virtual bool isValid() const = 0;

		virtual void* getMainHwnd() = 0;
		virtual bool addExtensionsMainMenu() = 0;
		virtual ReaProject* enumProjects(int idx, char* projfnOutOptional, int projfnOutOptional_sz) = 0;
		virtual juce::String getProjectString(ReaProject* project, const char* key) = 0;
		virtual int resolveRenderPattern(ReaProject* proj, const char* path, const char* pattern, char* targets, int targets_sz) = 0;
		virtual void main_OnCommand(int command, int flag) = 0;
		virtual int getProjExtState(ReaProject* proj, const char* extname, const char* key, char* valOutNeedBig, int valOutNeedBig_sz) = 0;
		virtual int setProjExtState(ReaProject* proj, const char* extname, const char* key, const char* value) = 0;
		virtual void markProjectDirty(ReaProject* proj) = 0;
		virtual int getProjectStateChangeCount(ReaProject* proj) = 0;
		virtual double getSetProjectInfo(ReaProject* proj, const char* desc, double value, bool is_set) = 0;
	};

	class ReaperContext
		: public WwiseTransfer::DawContext
	{
	public:
		ReaperContext(ReaperPluginInterface& pluginInfo);
		~ReaperContext() override;

		bool sessionChanged() override;
		juce::String getSessionName() override;
		bool saveState(juce::ValueTree applicationState) override;
		juce::ValueTree retrieveState() override;
		void renderItems() override;
		std::vector<WwiseTransfer::Import::PreviewItem> getItemsForPreview(const WwiseTransfer::Import::Options& options) override;
		std::vector<WwiseTransfer::Import::Item> getItemsForImport(const WwiseTransfer::Import::Options& options) override;

	private:
		struct ProjectInfo
		{
			ReaProject* projectReference{};
			juce::String projectName;
			juce::String projectPath;
		};

		struct StateInfo
		{
			int projectStateCount{0};
			double renderSource{0.0};
			double renderBounds{0.0};
			juce::String renderFile;
			juce::String renderPattern;
		};

		std::vector<juce::String> splitDoubleNullTerminatedString(const char*);
		std::vector<juce::String> getItemListFromRenderPattern(ReaProject* project, const juce::String& pattern, bool suppressIllegalPaths = true);
		ProjectInfo getProjectInfo() const;
		juce::String getRenderPattern(const ProjectInfo& projectInfo) const;
		juce::File getRenderDirectory(const ProjectInfo& projectInfo) const;

		juce::File defaultRenderDirectory;

		juce::CriticalSection apiAccess;

		ReaperPluginInterface& reaperPluginInterface;

		StateInfo stateInfo;
	};
} // namespace AK::ReaWwise
