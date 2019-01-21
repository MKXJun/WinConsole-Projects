#pragma once
#include <string>
#include <filesystem>

struct IFTPOperation
{
	using FilePath = std::experimental::filesystem::path;
	virtual bool LogIn(char *userName, char *passWord) = 0;
	virtual bool SetCurrDir(const FilePath& path) = 0;
	virtual bool SetUpDir() = 0;
	virtual bool Rename(const FilePath& source,const FilePath& target) = 0;
	virtual bool MakeDir(const FilePath& path) = 0;
	virtual bool GetFile(const FilePath & fromFile) = 0;
	virtual bool PutFile(const FilePath & toPath, const std::string & fileName) = 0;
	virtual bool GetHelp() = 0;
	virtual bool Delete(const FilePath& p) = 0;
};

