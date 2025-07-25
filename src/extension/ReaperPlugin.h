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

#pragma once

#include "IReaperPlugin.h"

#include <reaper_plugin_functions.h>

namespace AK::ReaWwise
{
	class ReaperPlugin : public IReaperPlugin
	{
	public:
		ReaperPlugin(reaper_plugin_info_t* pluginInfo);
		~ReaperPlugin() override = default;

		int getCallerVersion() const override;
		int registerFunction(const char* name, void* infoStruct) const override;
		bool isValid() const override;
		void* getMainHwnd() override;
		bool addExtensionsMainMenu() override;
		ReaProject* enumProjects(int idx, char* projfnOutOptional, int projfnOutOptional_sz) override;
		int resolveRenderPattern(ReaProject* project, const char* path, const char* pattern, char* targets, int targets_sz) override;
		void main_OnCommand(int command, int flag) override;
		int getProjExtState(ReaProject* proj, const char* extname, const char* key, char* valOutNeedBig, int valOutNeedBig_sz) override;
		int setProjExtState(ReaProject* proj, const char* extname, const char* key, const char* value) override;
		void markProjectDirty(ReaProject* proj) override;
		int getProjectStateChangeCount(ReaProject* proj) override;
		double getSetProjectInfo(ReaProject* proj, const char* desc, double value, bool is_set) override;
		bool getSetProjectInfo_String(ReaProject* project, const char* desc, char* valuestrNeedBig, bool is_set) override;
		int reallocCmdRegisterBuf(char** ptr, int* ptr_size) override;
		void reallocCmdClear(int tok) override;
		bool supportsReallocCommands() override;

	private:
		reaper_plugin_info_t* pluginInfo;
		decltype(GetMainHwnd) _getMainHwnd;
		decltype(AddExtensionsMainMenu) _addExtensionsMainMenu;
		decltype(EnumProjects) _enumProjects;
		decltype(GetSetProjectInfo_String) _getSetProjectInfo_String;
		decltype(ResolveRenderPattern) _resolveRenderPattern;
		decltype(Main_OnCommand) _main_OnCommand;
		decltype(GetProjExtState) _getProjExtState;
		decltype(SetProjExtState) _setProjExtState;
		decltype(MarkProjectDirty) _markProjectDirty;
		decltype(GetProjectStateChangeCount) _getProjectStateChangeCount;
		decltype(GetSetProjectInfo) _getSetProjectInfo;
		decltype(realloc_cmd_register_buf) _realloc_cmd_register_buf;
		decltype(realloc_cmd_clear) _realloc_cmd_clear;
	};
} // namespace AK::ReaWwise
