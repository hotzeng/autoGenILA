#ifndef LIVE_ANALYSIS_TAINT_GEN_H
#define LIVE_ANALYSIS_TAINT_GEN_H

#include <string>
#include <iostream>
#include "helper.h"
#include "op_taint_gen.h"
#include "global_data.h"
#include "varWidth.h"
#include "types.h"

namespace taintGen {

#define INPUT           1 
#define REG             2
#define WIRE            3
#define TWO_OP          4
#define ONE_OP          5
#define ITE             6
#define NONBLOCK        7
#define NONBLOCKCONCAT  8
//#define OUTPUT          9
const int OUTPUT      = 9;
#define CASE            10
#define SRC_CONCAT      11
#define FUNCDEF         12
#define FUNCEND         13
#define BOTH_CONCAT     14
#define ALWAYS_CLKRST   15
#define ALWAYS          16
#define REDUCE1         17
#define INSTANCEBEGIN   18
#define MEM             19
#define ALWAYS_FAKE     20
#define SEL             21
#define NONBLOCKIF      22
#define ALWAYS_STAR     23
#define ALWAYS_NEG      24
#define DEST_CONCAT     25
#define IF              26
#define ELSE            27
#define MODULE          28
#define FUNC            29
#define SUBMODOUT       30
#define SUBMODIN        31
#define SWITCH          32
#define DYNSEL          33
#define MEM_IF_ASSIGN   34
#define UNSUPPORT       99
#define NONE            100

void clean_file(std::string fileName, bool useLogic);

void remove_functions(std::string fileName);

void add_line_taints(std::string line, std::ofstream &output, std::ifstream &input);

void read_in_clkrst(std::string pathFile, std::string fileName);

void add_file_taints(std::string fileName, std::map<std::string, std::vector<std::string>> &moduleInputsMap, std::map<std::string, std::vector<std::string>> &moduleOutputsMap, std::map<std::string, std::vector<std::string>> &moduleRFlagsMap);

void add_module_name(std::string fileName, std::map<std::string, std::vector<std::string>> &moduleInputsMap, std::map<std::string, std::vector<std::string>> &moduleOutputsMap, std::map<std::string, std::vector<std::string>> &moduleRFlagsMap, bool isTopIn);

bool isNum(std::string name);

bool isOutput(std::string var);

bool isReg(std::string var);

int parse_verilog_line(std::string line, bool ignoreWrongOp=false);

void add_func_taints(std::ifstream &input, std::ofstream &output, std::string funcDefinition);

// The goal of this function: given the firstLine, generate a new fucntion which outputs the taint specified with taintBits. The original function does not need to be printed
void add_case_taints(std::ifstream &input, std::ofstream &output, std::string firstLine, std::string taintBits);

void add_func_taints_limited(std::ifstream &input, std::ofstream &output, std::string firstLine);
  
void add_func_taints_call_limited(std::string line, std::ofstream &output);

void extend_module_instantiation(std::ifstream &input, std::ofstream &output, std::string moduleFirstLine, std::map<std::string, std::vector<std::string>> &moduleInputsMap, std::map<std::string, std::vector<std::string>> &moduleOutputsMap);

void merge_taints(std::string fileName);

bool extract_concat(std::string line, std::ofstream &output, std::string &returnedStmt, std::string &fangyuanDeclaration, std::string &fangyuanAssign, bool isFuncCall);

void gen_assert_property(std::ofstream &output);

void gen_reg_output(std::string fileName);

void gen_wire_output(std::string fileName);

void remove_function_wrapper(std::string firstLine, std::ifstream &input, std::ofstream &output);

void analyze_reg_path( std::string fileName );

void add_case_taints_limited(std::ifstream &input, std::ofstream &output, std::string caseFirstLine);

void collect_ite_dest(const std::string &line);

void collect_case_dest(const std::string &line);

void assert_reg_map_gen();

void map_gen(std::string moduleName, std::string instanceName, std::ofstream &output);

void read_changed_regs(std::string fileName, 
                       StrSet_t & changedRegVec);

void find_reg(std::string parentModName, StrVec_t &path2Reg);

void determine_regs_to_check();

void check_changed_regs(std::string modName);

void convert_nb_if_to_ite(std::ifstream &input, 
                          std::ofstream &output, 
                          std::string line);

std::string separate_modules(std::string fileName, 
                             std::vector<std::string> &modules,
                             std::map<std::string, std::vector<std::string>> &childModules,
                             uint32_t &totalRegCnt,
                             std::unordered_map<std::string,
                                                Str2StrUmap_t>& instance2moduleMap,
                             bool getIO,
                             Str2StrUmap_t &moduleInputsMap,
                             Str2StrUmap_t &moduleOutputsMap);

std::string separate_modules(std::string fileName, 
                             std::vector<std::string> &modules, 
                             std::map<std::string, std::vector<std::string>> &childModules, 
                             uint32_t &totalRegCnt, 
                             std::unordered_map<std::string, 
                                                Str2StrUmap_t>& instance2moduleMap);

int taint_gen(std::string fileName, uint32_t stage, bool isTopIn, std::map<std::string, std::vector<std::string>> &moduleInputsMap, std::map<std::string, std::vector<std::string>> &moduleOutputsMap, std::map<std::string, std::vector<std::string>> &moduleRFlagsMap, uint32_t totalRegCnt, uint32_t &nextSig, bool doProcessPathInfo);

} // end of namespace taintGen
#endif
