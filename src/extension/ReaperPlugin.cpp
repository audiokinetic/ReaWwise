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

#include "ReaperPlugin.h"

namespace AK::ReaWwise
{

	ReaperPlugin::ReaperPlugin(reaper_plugin_info_t* pluginInfo)
		: pluginInfo(pluginInfo)
	{
		_getMainHwnd = decltype(GetMainHwnd)(pluginInfo->GetFunc("GetMainHwnd"));
		_addExtensionsMainMenu = decltype(AddExtensionsMainMenu)(pluginInfo->GetFunc("AddExtensionsMainMenu"));
		_enumProjects = decltype(EnumProjects)(pluginInfo->GetFunc("EnumProjects"));
		_getSetProjectInfo_String = decltype(GetSetProjectInfo_String)(pluginInfo->GetFunc("GetSetProjectInfo_String"));
		_resolveRenderPattern = decltype(ResolveRenderPattern)(pluginInfo->GetFunc("ResolveRenderPattern"));
		_main_OnCommand = decltype(Main_OnCommand)(pluginInfo->GetFunc("Main_OnCommand"));
		_getProjExtState = decltype(GetProjExtState)(pluginInfo->GetFunc("GetProjExtState"));
		_setProjExtState = decltype(SetProjExtState)(pluginInfo->GetFunc("SetProjExtState"));
		_markProjectDirty = decltype(MarkProjectDirty)(pluginInfo->GetFunc("MarkProjectDirty"));
		_getProjectStateChangeCount = decltype(GetProjectStateChangeCount)(pluginInfo->GetFunc("GetProjectStateChangeCount"));
		_getSetProjectInfo = decltype(GetSetProjectInfo)(pluginInfo->GetFunc("GetSetProjectInfo"));
		_realloc_cmd_register_buf = decltype(realloc_cmd_register_buf)(pluginInfo->GetFunc("realloc_cmd_register_buf"));
		_realloc_cmd_clear = decltype(realloc_cmd_clear)(pluginInfo->GetFunc("realloc_cmd_clear"));
	}

	int ReaperPlugin::getCallerVersion() const
	{
		return pluginInfo->caller_version;
	}

	int ReaperPlugin::registerFunction(const char* name, void* infoStruct) const
	{
		return pluginInfo->Register(name, infoStruct);
	}

	bool ReaperPlugin::isValid() const
	{
		if(_getMainHwnd &&
			_addExtensionsMainMenu &&
			_enumProjects &&
			_getSetProjectInfo_String &&
			_resolveRenderPattern &&
			_main_OnCommand &&
			_getProjExtState &&
			_setProjExtState &&
			_markProjectDirty &&
			_getProjectStateChangeCount &&
			_getSetProjectInfo)
			return true;

		return false;
	}

	void* ReaperPlugin::getMainHwnd()
	{
		return _getMainHwnd();
	}

	bool ReaperPlugin::addExtensionsMainMenu()
	{
		return _addExtensionsMainMenu();
	}

	ReaProject* ReaperPlugin::enumProjects(int idx, char* projfnOutOptional, int projfnOutOptional_sz)
	{
		return _enumProjects(idx, projfnOutOptional, projfnOutOptional_sz);
	}

	int ReaperPlugin::resolveRenderPattern(ReaProject* project, const char* path, const char* pattern, char* targets, int targets_sz)
	{
		return _resolveRenderPattern(project, path, pattern, targets, targets_sz);
	}

	void ReaperPlugin::main_OnCommand(int command, int flag)
	{
		_main_OnCommand(command, flag);
	}

	int ReaperPlugin::getProjExtState(ReaProject* proj, const char* extname, const char* key, char* valOutNeedBig, int valOutNeedBig_sz)
	{
		return _getProjExtState(proj, extname, key, valOutNeedBig, valOutNeedBig_sz);
	}

	int ReaperPlugin::setProjExtState(ReaProject* proj, const char* extname, const char* key, const char* value)
	{
		return _setProjExtState(proj, extname, key, value);
	}

	void ReaperPlugin::markProjectDirty(ReaProject* proj)
	{
		return _markProjectDirty(proj);
	}

	int ReaperPlugin::getProjectStateChangeCount(ReaProject* proj)
	{
		return _getProjectStateChangeCount(proj);
	}

	double ReaperPlugin::getSetProjectInfo(ReaProject* proj, const char* desc, double value, bool is_set)
	{
		return _getSetProjectInfo(proj, desc, value, is_set);
	}

	bool ReaperPlugin::getSetProjectInfo_String(ReaProject* project, const char* desc, char* valuestrNeedBig, bool is_set)
	{
		return _getSetProjectInfo_String(project, desc, valuestrNeedBig, is_set);
	}

	int ReaperPlugin::reallocCmdRegisterBuf(char** ptr, int* ptr_size)
	{
		return _realloc_cmd_register_buf(ptr, ptr_size);
	}

	void ReaperPlugin::reallocCmdClear(int tok)
	{
		_realloc_cmd_clear(tok);
	}

	bool ReaperPlugin::supportsReallocCommands()
	{
		return _realloc_cmd_register_buf && _realloc_cmd_clear;
	}
} // namespace AK::ReaWwise
