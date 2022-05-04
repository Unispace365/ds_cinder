#include "stdafx.h"
#include "ds/util/ada/tools/fs.h"
#include "ds/util/ada/tools/text.h"


#include <iostream>     // cout
#include <fstream>      // File
#include <iterator>     // std::back_inserter
#include <algorithm>    // std::unique
#include <sys/stat.h>

#ifdef _WIN32
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#else
#include "glob.h"
#endif


namespace ada {

bool haveExt(const std::string& _file, const std::string& _ext){
    return _file.find( "." + _ext) != std::string::npos;
}

bool urlExists(const std::string& _name) {
    struct stat buffer;
    return (stat (_name.c_str(), &buffer) == 0);
}

std::string getExt(const std::string& _filename) {
    if (_filename.find_last_of(".") != std::string::npos)
        return _filename.substr(_filename.find_last_of(".") + 1);
    return "";
}

std::string getBaseDir(const std::string& filepath) {
    std::string base_dir = "";

    if (filepath.find_last_of("/\\") != std::string::npos)
        base_dir =  filepath.substr(0, filepath.find_last_of("/\\"));
    else 
        base_dir = ".";
    
#ifdef _WIN32
    base_dir += "\\";
#else
    base_dir += "/";
#endif

    return base_dir;
}

#if defined (_WIN32)
const char* realpath(const char* str, void*)
{
    return str;
}
#endif 
std::string getAbsPath(const std::string& _path) {
    std::string abs_path = realpath(_path.c_str(), NULL);
    std::size_t found = abs_path.find_last_of("\\/");
    if (found) return abs_path.substr(0, found);
    else return "";
}

std::string urlResolve(const std::string& _path, const std::string& _pwd, const List &_include_folders) {
    std::string url = _pwd +'/'+ _path;

    // If the path is not in the same directory
    if (urlExists(url)) 
        return realpath(url.c_str(), NULL);

    // .. search on the include path
    else {
        for ( uint32_t i = 0; i < _include_folders.size(); i++) {
            std::string new_path = _include_folders[i] + "/" + _path;
            if (urlExists(new_path)) 
                return realpath(new_path.c_str(), NULL);
        }
        return _path;
    }
}

bool extractDependency(const std::string &_line, std::string *_dependency) {
    // Search for ocurences of #include or #pargma include (OF)
    if (_line.find("#include ") == 0 || _line.find("#pragma include ") == 0) {
        unsigned begin = _line.find_first_of("\"");
        unsigned end = _line.find_last_of("\"");
        if (begin != end) {
            (*_dependency) = _line.substr(begin+1,end-begin-1);
            return true;
        }
    }
    return false;
}

bool alreadyInclude(const std::string &_path, List *_dependencies) {
    for (unsigned int i = 0; i < _dependencies->size(); i++) {
        if ( _path == (*_dependencies)[i]) {
            return true;
        }
    }
    return false;
}

bool loadFromPath(const std::string &_path, std::string *_into, const std::vector<std::string> &_include_folders, List *_dependencies) {
    std::ifstream file;
    file.open(_path.c_str());

    // Skip if it's already open
    if (!file.is_open()) 
        return false;

    // Get absolute home folder
    std::string original_path = getAbsPath(_path);

    std::string line;
    std::string dependency;
    std::string newBuffer;
    while (!file.eof()) {
        dependency = "";
        getline(file, line);

        if (extractDependency(line, &dependency)) {
            dependency = urlResolve(dependency, original_path, _include_folders);
            newBuffer = "";
            if (loadFromPath(dependency, &newBuffer, _include_folders, _dependencies)) {
                if (!alreadyInclude(dependency, _dependencies)) {
                    // Insert the content of the dependency
                    (*_into) += "\n" + newBuffer + "\n";

                    // Add dependency to dependency list
                    _dependencies->push_back(dependency);
                }
            }
            else {
                std::cerr << "Error: " << dependency << " not found at " << original_path << std::endl;
            }
        }
        else {
            (*_into) += line + "\n";
        }
    }

    file.close();
    return true;
}


std::vector<std::string> glob(const std::string& _pattern) {
    std::vector<std::string> files;

#if defined(_WIN32)
    int err = 0;
    WIN32_FIND_DATAA finddata;
    HANDLE hfindfile = FindFirstFileA(_pattern.c_str(), &finddata);

    if (hfindfile != INVALID_HANDLE_VALUE) {
        do {
            files.push_back(std::string(finddata.cFileName));
        } while (FindNextFileA(hfindfile, &finddata));
        FindClose(hfindfile);
    }

#else
    std::vector<std::string> folders = split(_pattern, '/', true);
    std::string folder = "";

    for (size_t i = 0; i < folders.size() - 1; i++)
        folder += folders[i] + "/";

    glob::glob glob(_pattern.c_str());
    while (glob) {
        files.push_back( folder + glob.current_match() );
        glob.next();
    }

#endif
    return files;
}

}