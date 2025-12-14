#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace pma::metric {

struct FieldInfo 
{
    std::string name;
    // String, Data, ...
    std::string type;  

};

struct ParamInfo 
{ 
    std::string external_name;
    std::string local_name;
    std::string type; 
};

struct MethodInfo 
{
    std::string name;
    bool isInit=false;
    bool isDeinit=false;
    std::vector<ParamInfo> params;
    std::string returnType;

    std::unordered_set<std::string> usedFields;     // LCOM
    std::unordered_set<std::string> calledMethods;  // RFC
    std::unordered_set<std::string> referencedTypes;// CBO 
};

struct ClassInfo 
{
    std::string name;
    std::string base; 
    std::vector<FieldInfo> fields;
    std::vector<MethodInfo> methods;

    std::vector<std::string> children;
};

struct OOModel 
{
    std::unordered_map<std::string, ClassInfo> classes; // name -> info
};

} // namespace pma::metric
