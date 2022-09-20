#pragma once

class ReaProject;

class IReaperPlugin
{
public:
	virtual ~IReaperPlugin() = default;

	virtual int getCallerVersion() const = 0;
	virtual int registerFunction(const char* name, void* infoStruct) const = 0;
	virtual bool isValid() const = 0;
	virtual void* getMainHwnd() = 0;
	virtual bool addExtensionsMainMenu() = 0;
	virtual ReaProject* enumProjects(int idx, char* projfnOutOptional, int projfnOutOptional_sz) = 0;
	virtual int resolveRenderPattern(ReaProject* proj, const char* path, const char* pattern, char* targets, int targets_sz) = 0;
	virtual void main_OnCommand(int command, int flag) = 0;
	virtual int getProjExtState(ReaProject* proj, const char* extname, const char* key, char* valOutNeedBig, int valOutNeedBig_sz) = 0;
	virtual int setProjExtState(ReaProject* proj, const char* extname, const char* key, const char* value) = 0;
	virtual void markProjectDirty(ReaProject* proj) = 0;
	virtual int getProjectStateChangeCount(ReaProject* proj) = 0;
	virtual double getSetProjectInfo(ReaProject* proj, const char* desc, double value, bool is_set) = 0;
	virtual bool getSetProjectInfo_String(ReaProject* project, const char* desc, char* valuestrNeedBig, bool is_set) = 0;
	virtual int reallocCmdRegisterBuf(char** ptr, int* ptr_size) = 0;
	virtual void reallocCmdClear(int tok) = 0;
	virtual bool supportsReallocCommands() = 0;
};
