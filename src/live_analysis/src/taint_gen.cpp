#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <queue>
#include <utility>
#include <assert.h>
#include <unordered_map>
#include <set>
#include <map>
#include <bitset>
#include <stack>
#include <algorithm>
#include "taint_gen.h"
//#include "pass_info.h"
#include <cmath>
#include <glog/logging.h>

#define toStr(a) std::to_string(a)


namespace taintGen {

using namespace syntaxPatterns;

/* Global data */
std::string moduleName;
std::vector<std::string> moduleInputs;
std::vector<std::string> moduleOutputs;
std::vector<std::string> extendInputs;
std::vector<std::string> extendOutputs;
std::vector<std::string> flagOutputs;
std::vector<std::string> moduleRegs;
std::vector<std::string> moduleTrueRegs;
std::unordered_map<std::string, uint32_t> moduleMems;
std::set<std::string> moduleWires;
std::set<std::string> g_iteDest;
std::set<std::string> g_wire2reg;
std::set<std::string> g_operators{"+", "-", "*", "/","%", "&&", "||", "==", "===", "!=", ">", ">=", "<", "<=", "|", "^", "&", "+:", "-:", "<<", ">>", "<<<", ">>>", "~", "!", "&", "~&", "~|", "~^", "^", "?", "<=", "always", "function"};
std::set<std::string> g_clk_set;
std::string clockName;
std::string resetName;
std::vector<std::string> rTaints;
StrSet_t g_changedRegVec;
std::unordered_map<std::string, uint32_t> nextVersion;
std::unordered_map<std::string, std::vector<bool>> nxtVerBits;
std::unordered_map<std::string, std::string> new_next;
std::unordered_map<std::string, std::string> update_reg;
std::unordered_map<std::string, std::pair<std::string, std::string>> memDims;
std::unordered_map<std::string, uint32_t> reg2sig;
std::unordered_map<std::string, uint32_t> addedVarItemNum; // used to check item number in case statementsrs
std::unordered_map<std::string, uint32_t> addedVarCaseSliceWidth; // width of each slice used in RHS of case
std::unordered_map<std::string, uint32_t> g_destVersion;
std::unordered_map<std::string, std::pair<std::string, bool>> g_moduleRst;
std::unordered_map<std::string, std::string> g_moduleClk;
// for generating assert names, first is module name, second is instance name, third is sub-mod name
std::unordered_map<std::string, std::unordered_map<std::string, std::string>> g_mod2instMap;
// key is module name, value is vector of asserted regs in this module
std::unordered_map<std::string, std::vector<std::string>> g_mod2assertMap;
std::map<std::string, std::set<std::string>> g_modChangedRegs;
// first key is module name: which module the instance is within
// second key is instance name. Value is the corresponding module name
std::unordered_map<std::string, Str2StrUmap_t> g_instance2moduleMap;
std::map<std::string, std::set<std::string>> g_trueReg2Slice;

VarWidth varWidth;
VarWidth funcVarWidth;
unsigned long int NEW_VAR = 0;
unsigned long int NEW_FANGYUAN = 0;
unsigned long int USELESS_VAR = 0;
bool did_clean_file = false;
std::string g_recentClk;
std::string g_recentRst;
bool g_recentRst_positive = true;
std::string g_possibleCLK;
std::string g_possibleRST;
bool g_possibleSign;
bool isTop = false;
bool g_hasRst;
bool g_hasClk;
bool g_verb = true;
bool g_has_read_taint;  // if false, read taint is replaced with x taint
bool g_rst_pos;
bool g_clkrst_exist = false;
bool g_use_reset_taint = false;
bool g_use_zy_count = false;
bool g_use_reset_sig = false;
bool g_remove_adff = false;
bool g_use_value_change = false;
bool g_clean_submod = false;
// TODO: set this configurations!
// // for func_extract, split long bitVec into multiple short ones
bool g_split_long_num = true;
// if true, rst.vcd file is parsed and rst values are used
bool g_use_vcd_parser = true;
// for find written ASV
bool g_write_assert = false; 
// used when start from arbitraty state, only reset taints
bool g_use_taint_rst = false;
// used to end verification after a certain time of instruction begins, TODO: enable it for 8051
bool g_use_end_sig = false;
// INSTR_IN_ZY needs to be defined, TODO: enable it for 8051
bool g_wt_keeped = false; 
// TODO: enable it for biRISCV
bool g_special_equal_taint = false; 
// set the read flag only if reg's value is not reset value
bool g_set_rflag_if_not_rst_val = true;  // TODO: usually enable it 
// TODO: enable it for biRISCV
bool g_set_rflag_if_not_norm_val = false; 
// TODO: seems problematic, be careful when considering enable it
// Disable for 8051
bool g_use_does_keep = false;  
// enable this to only check if reg's value is invariant when instruction finished
CheckInvarType g_check_invariance = CheckRst; // TODO: check this setting
// TODO: g_enable_taint set to false when only checking invariance
// (Used when checking for invariant registers to replace ASVs)
bool g_enable_taint = true;
// if is true, "assert()" will be generated for jaspergold to check
// otherwise, a verilog assert module will be generated for simulation-based check
bool g_use_jasper = true;
uint32_t g_assert_num = 0;
uint32_t g_case_reg_num = 0;
std::vector<std::string> g_assertNames;

std::string _t="_T";
std::string _r="_R";
std::string _x="_X";
std::string _c="_C";
std::string _sig="_S";
std::string TAINT_RST="zy_taint_rst";
std::string END_SIG="zy_end_sig";
std::string ASSERT_PROTECT="zy_assert_protect";
std::string srcConcatFeature = " = {";
std::string bothConcatFeature = "} = {";
std::string g_gatedClkFileName = "gated_clk.txt";
std::string g_path;
std::string idxedModuleName;
std::string g_topModule;
std::string orderFileName = "order.txt";
uint32_t g_reg_count;
uint32_t g_sig_width; // == log2(g_reg_count);
uint32_t g_next_sig;
uint32_t CONSTANT_SIG_NUM = 1;
std::string CONSTANT_SIG; // reserve sig=1 for constants
std::string RESET_SIG = "2"; // reserve sig=2 for reset

/* clean all the global data */
void clean_global_data(uint32_t totalRegCnt, uint32_t nextSig) {
  moduleName.clear();
  moduleInputs.clear();
  moduleOutputs.clear();
  extendInputs.clear();
  extendOutputs.clear();
  flagOutputs.clear();
  moduleRegs.clear();
  moduleMems.clear();
  moduleWires.clear();
  clockName.clear();
  resetName.clear();
  rTaints.clear();
  g_wire2reg.clear();
  nextVersion.clear();
  nxtVerBits.clear();
  new_next.clear();
  update_reg.clear();
  memDims.clear();
  varWidth.clear();
  funcVarWidth.clear();
  NEW_VAR = 0;
  NEW_FANGYUAN = 0;
  USELESS_VAR = 0;
  did_clean_file = false;
  g_recentClk.clear();
  g_recentRst.clear();
  g_recentRst_positive = true;
  g_hasRst = false;
  g_hasClk = false;
  g_has_read_taint = true; // if true, read taint takes effect
  g_rst_pos = true;
  g_clkrst_exist = false;
  g_reg_count = totalRegCnt;
  if(nextSig == 0)
    g_next_sig = 3; // 0 is reserved for unused
  else
    g_next_sig = nextSig;
  reg2sig.clear();
  g_use_reset_taint = false;
  addedVarItemNum.clear();
  addedVarCaseSliceWidth.clear();
  g_destVersion.clear();
  moduleTrueRegs.clear();
  g_backwardMap.clear();
  g_forwardMap.clear();
  g_passExprStore.clear();
  g_caseBackwardMap.clear();
  g_caseForwardMap.clear();
  g_caseStore.clear();
  g_passInfoMap.clear();
  g_regCondMap.clear();  
}


/*remove comments
  remove redundent blanks 
  extract concatenants 
  remove functions wrapping cases 
  collect information of select and concat*/
void clean_file(std::string fileName, bool useLogic) {
  std::ifstream cleanFileInput(fileName);
  std::ofstream output(fileName + ".nocomment");
  std::string line;
  std::string cleanLine;
  std::smatch match;
  static const std::regex pureComment("^\\s*\\(\\*.*\\*\\)$");
  static const std::regex partialComment("\\(\\*.*\\*\\) ");
  static const std::regex redundentBlank("(\\S)(\\s+)(\\S)");
  static const std::regex extraBlank("([a-zA-Z0-9_\\.'])(\\s)(\\[)");
  bool inFunc = false;
  std::string rsvdLine; // reserved line, not printed in last iteration
  if(g_moduleClk.find(moduleName) != g_moduleClk.end()) {
    g_hasClk = true;
    g_recentClk = g_moduleClk[moduleName];
  }
  if(g_moduleRst.find(moduleName) != g_moduleRst.end()) {
    g_hasRst = true;
    g_clkrst_exist = true;
    g_recentRst = g_moduleRst[moduleName].first;
    g_recentRst_positive = g_moduleRst[moduleName].second;
  }

  // check assert options
  //assert(!g_two_prev || !g_one_prev);
  assert(!g_set_rflag_if_not_rst_val || g_enable_taint);

  while( std::getline(cleanFileInput, line) ) {
    //toCout(line);
    if(line.find("S4 S4_0 (") != std::string::npos) {
      toCoutVerb("FIND IT!");
    }

    /// skipe comment line
    if(std::regex_match(line, match, pureComment) || line.substr(0,2) == "/*" || line.empty())
      continue;

    /// remove in-line comments
    line = std::regex_replace(line, partialComment, "");
    if(line.find("//") != std::string::npos)
      line = line.substr(0, line.find("//"));
    cleanLine = std::regex_replace(line, redundentBlank, "$1 $3");
    // TODO: deal with pDbSel;
    /// extract aways concatenations into new Fangyuan variables
    bool noConcat=true;
    /// if is always line, look ahead for non-blocking concatenation assignments
    std::string retStr;    
    std::string addedVarDeclaration;
    std::string addedVarAssign;
    noConcat = extract_concat(cleanLine, output, retStr, addedVarDeclaration, addedVarAssign, true);
    if( !is_srcDestConcat(line) && (!std::regex_match(line, match, pAlwaysClkRst) || !g_remove_adff) ) { // pAlwaysClkRst is printed below
      if(std::regex_match(cleanLine, match, pAlwaysClk)) {
        rsvdLine = cleanLine;
      }
      else if(useLogic && std::regex_match(line, match, pReg)) {
        std::string printedLine = line;        
        int pos = printedLine.find("reg", 0);
        if(g_use_jasper) printedLine.replace(pos, 3, "logic");
        output << printedLine << std::endl;
      }
      else if(useLogic && std::regex_match(line, match, pWire)) {
        std::string printedLine = line;
        int pos = printedLine.find("wire", 0);
        if(g_use_jasper) printedLine.replace(pos, 4, "logic");
        output << printedLine << std::endl;
      }
      else if(std::regex_match(line, match, pWireAssign)) {
        size_t pos = line.find("=");
        if(pos == std::string::npos) {
          toCout("Error: cannot find equal sign in wire-asign: "+line);
          abort();
        }
        std::string printedLine = line.substr(0, pos)+";";
        pos = printedLine.find("wire", 0);
        if(useLogic && g_use_jasper) printedLine.replace(pos, 4, "logic");
        output << printedLine << std::endl;
        std::string varName = match.str(3);
        std::string assignName = match.str(4);
        output << "  assign "+varName+" = "+assignName+";" << std::endl;
      }
      else if (noConcat) {
        if(!rsvdLine.empty()) output << rsvdLine << std::endl;
        output << cleanLine << std::endl;
        rsvdLine.clear();
      }
      else {
        if(!rsvdLine.empty()) output << rsvdLine << std::endl;        
        output << retStr << std::endl;
        cleanLine = retStr;
        rsvdLine.clear();        
      }
    }
    if(!addedVarDeclaration.empty()) {
      output << addedVarDeclaration << std::endl;
      output << addedVarAssign << std::endl;
    }
    /// store the width of wires and regs in varWidth
    uint32_t choice = parse_verilog_line(cleanLine, true);
    std::smatch m;
    switch (choice) {
      case INPUT:
        {
          std::regex_match(line, m, pInput);
          std::string slice = m.str(2);
          std::string var = m.str(3);
          bool insertDone = false;
          if(!inFunc) {
            if(!slice.empty())
              insertDone = varWidth.var_width_insert(var, get_end(slice), get_begin(slice));
            else
              insertDone = varWidth.var_width_insert(var, 0, 0);
          }
          else {
            if(!slice.empty())
              insertDone = funcVarWidth.force_insert(var, get_end(slice), get_begin(slice));
            else
              insertDone = funcVarWidth.force_insert(var, 0, 0);
          }     
          if (!insertDone) {
            std::cout << "insert failed in input case:" + line << std::endl;
            std::cout << "m.str(2):" + m.str(2) << std::endl;
            std::cout << "m.str(3):" + m.str(3) << std::endl;
          }
        }
        break;
      case REG:
        {
          if(!std::regex_match(line, m, pReg)
              && !std::regex_match(line, m, pRegConst) ) {
            toCout("Error in matching pReg or pRegConst: "+line);
            abort();
          }
          std::string slice = m.str(2);
          std::string var = m.str(3);
          bool insertDone = false;
          if(!inFunc) {
            if(!slice.empty())
              insertDone = varWidth.var_width_insert(var, get_end(slice), get_begin(slice));
            else
              insertDone = varWidth.var_width_insert(var, 0, 0);
          }
          else {
            if(!slice.empty())
              insertDone = funcVarWidth.force_insert(var, get_end(slice), get_begin(slice));
            else
              insertDone = funcVarWidth.force_insert(var, 0, 0);
          }
          if (!insertDone) {
            std::cout << "insert failed in reg case:" + line << std::endl;
            std::cout << "m.str(2):" + m.str(2) << std::endl;
            std::cout << "m.str(3):" + m.str(3) << std::endl;
          }
        }
        break;
      case WIRE:
        {
          std::regex_match(line, m, pWire);
          std::string slice = m.str(2);
          std::string var = m.str(3);
          bool insertDone = false;
          if(!inFunc) {
            if(!slice.empty())
              insertDone = varWidth.var_width_insert(var, get_end(slice), get_begin(slice));
            else
              insertDone = varWidth.var_width_insert(var, 0, 0);
          }
          else {
            if(!slice.empty())
              insertDone = funcVarWidth.force_insert(var, get_end(slice), get_begin(slice));
            else
              insertDone = funcVarWidth.force_insert(var, 0, 0);
          }
          if (!insertDone) {
            std::cout << "insert failed in wire case:" + line << std::endl;
            std::cout << "m.str(2):" + m.str(2) << std::endl;
            std::cout << "m.str(3):" + m.str(3) << std::endl;
          }
        }
        break;
      case OUTPUT:
        {
          std::regex_match(line, m, pOutput);
          std::string slice = m.str(2);
          std::string var = m.str(3);
          bool insertDone = false;
          if(!inFunc) {
            if(!slice.empty())
              insertDone = varWidth.var_width_insert(var, get_end(slice), get_begin(slice));
            else
              insertDone = varWidth.var_width_insert(var, 0, 0);
          }
          else {
            if(!slice.empty())
              insertDone = funcVarWidth.force_insert(var, get_end(slice), get_begin(slice));
            else
              insertDone = funcVarWidth.force_insert(var, 0, 0);
          }
          if (!insertDone) {
            std::cout << "insert failed in output case:" + line << std::endl;
            std::cout << "m.str(2):" + m.str(2) << std::endl;
            std::cout << "m.str(3):" + m.str(3) << std::endl;
          }
        }
        break;
      case FUNCDEF:
        {
          inFunc = true;
        }
        break;
      case FUNCEND:
        {
          inFunc = false;
        }
        break;
      //case SEL:
      //  {
      //    if (std::regex_match(line, m, pSel1)
      //        || std::regex_match(line, m, pSel2)
      //        || std::regex_match(line, m, pSel3)
      //        || std::regex_match(line, m, pSel4)) {}
      //    else
      //      abort();
      //    std::string destAndSlice = m.str(2);
      //    std::string op1AndSlice = m.str(3);
      //    std::string slice = m.str(4);
      //    std::string op2AndSlice = m.str(5);
      //    std::string op1, op1Slice;
      //    split_slice(op1AndSlice, op1, op1Slice);
      //    if(!isNum(op2AndSlice))
      //      toCout("!! Warning: select range has variable: "+line);
      //    if( g_varSelectRange.find(op1) != g_varSelectRange.end() ) {
      //      g_varSelectRange[op1].push_back(line);
      //    }
      //    else {
      //      g_varSelectRange.emplace(op1, std::vector<uint32_t>{line});
      //    }
      //  }
      //  break;
      //case ALWAYS_CLKRST:
      //  {
      //    std::getline(cleanFileInput, line); // if line
      //    std::getline(cleanFileInput, line); // if statement
      //    std::getline(cleanFileInput, line); // else line
      //    std::smatch m;          
      //    if(!std::regex_match(line, m, pElse)) {
      //      toCout("Error in matching else: "+line);
      //      abort();
      //    }
      //    std::getline(cleanFileInput, line); // else statement      
      //    if(!std::regex_match(line, m, pNonblock)) {
      //      toCout("match nonblock wrongly: "+line);
      //      abort();
      //    }
      //    std::string dest = m.str(2);
      //    std::string src = m.str(3);
      //    moduleTrueRegs.push_back(dest);
      //    fill_in_pass_relation(dest, src, line);
      //  }
      //  break;
       case BOTH_CONCAT:
        {
          //toCout("Matched in both_concat");          
          // split both_concat into src_concat and maybe also both_concat.
          if( !std::regex_match(line, m, pSrcDestBothConcat) ) {
          //if( !is_srcDestConcat(line) ) {
            toCout("Error: both_concat not matched: "+line);
            abort(); //
          }

          std::string blank = m.str(1);
          std::string destList = m.str(2);
          std::string srcList = m.str(3);
          // if the srcList can be cleanly divided into parts for each dest, then divide it
          // Otherwise, leave it.
          std::vector<std::string> destVec;
          std::vector<std::string> srcVec;
          parse_var_list(destList, destVec);
          parse_var_list(srcList, srcVec);
          uint32_t srcIdx = 0;
          uint32_t srcBits;
          int remainBits = 0;
          uint32_t idx = 0;
          std::string outputString;
          std::vector<std::string> lhsVec;
          for(std::string singleDest: destVec) {
            outputString.clear();            
            lhsVec.push_back(singleDest);
            if(lhsVec.size() == 1) {
              idx = srcIdx;
            }
            remainBits += get_var_slice_width(singleDest);            
            while(remainBits > 0) {
              srcBits = get_var_slice_width(srcVec[srcIdx++]);
              remainBits -= srcBits;
            }
            if(remainBits == 0) {
              if(lhsVec.size() == 1) {
                outputString = "  assign "+singleDest+" = { "+srcVec[idx++];
                while(idx < srcIdx) {
                  outputString += " , " + srcVec[idx++];
                }
                outputString += " };";
              }
              else {
                outputString = "  assign { " + lhsVec[0];
                auto it = lhsVec.begin();
                std::advance(it,1);
                while(it != lhsVec.end())
                  outputString += " , " + *(it++);
                outputString += " } = { "+srcVec[idx++];
                while(idx < srcIdx) {
                  outputString += " , " + srcVec[idx++];
                }
                outputString += " };";
                // outputString could be simple pass or sel operation
              }
              output << outputString << std::endl;
              lhsVec.clear();
            }
          }
        }
        break;
      case NONBLOCKCONCAT: // will be splitted into nonblock and src_concat
        {
          //toCout("Matched in nonblockconcat");          
          std::smatch m;
          if(!std::regex_match(line, m, pNonblockConcat)) {
            toCout("match nonblock wrongly: "+line);
            abort();
          }
          std::string destAndSlice = m.str(2);
          std::string dest, destSlice;
          split_slice(destAndSlice, dest, destSlice);
          assert(destSlice.empty());
          if(dest.back() == ' ') {
            toCout("Warning: the last char is empty: "+dest);
            dest.pop_back();
          }
          moduleTrueRegs.push_back(dest);
        }
        break;
      case ALWAYS_CLKRST:
        {
          if(!g_remove_adff)
            break;
          std::smatch m;
          if( !std::regex_match(line, m, pAlwaysClkRst) ) {
            std::cout << "!! Error in parsing always with clk & rst !!" << std::endl;
            abort();
          } 
          if(!g_hasRst) {
            g_recentClk = m.str(2);
            g_recentRst = m.str(3);
          }
          g_clk_set.insert(g_recentClk);
          output << "  always @(posedge "+g_recentClk+")" << std::endl;          
        }
        break;
      default:
        break;
    } // end of switch
  }
  output.close();
  cleanFileInput.close();
  did_clean_file = true;
}



// remove functions
// convert nonblockif/nonblockif-else to nonblock+ite
void remove_functions(std::string fileName) {
  std::ifstream input(fileName+".nocomment");
  std::ofstream output(fileName + ".clean");
  std::string line;
  while( std::getline(input, line) ) {
    uint32_t choice = parse_verilog_line(line, true);
    if ( choice == FUNCDEF && g_use_jasper) {
      remove_function_wrapper(line, input, output);
    }
    else if(line.find("always @(") != std::string::npos) {
      convert_nb_if_to_ite(input, output, line);
    }
    else
      output << line << std::endl;
  }
}


void analyze_reg_path( std::string fileName ) {
  std::ifstream input(fileName+".clean");
  std::string line;
  std::smatch m;
  while( std::getline(input, line) ) {
    //toCout(line);
    if(line.find("27'b000000000000000000000000000, of, 32'b00000000000000000000000000000000") != std::string::npos) {
      toCoutVerb("FIND IT!");
    }
    uint32_t choice = parse_verilog_line(line, true);
    switch(choice) {
      case ONE_OP:
        {
          //toCout("Matched in one_op");
          if(std::regex_match(line, m, pNone)) {
            std::string destAndSlice = m.str(2);
            std::string op1AndSlice = m.str(3);
            fill_in_pass_relation(destAndSlice, op1AndSlice, line);
          }
        }
        break;
      case SEL:
        {
          //toCout("Matched in sel");
          fill_in_sel_relation(line);
        }
        break;
      case ITE:
        {
          //toCout("Matched in ite");
          fill_in_ite_relation(line);
          collect_ite_dest(line);
        }
        break;
      case SRC_CONCAT:
        {
          //toCout("Matched in src_concat");          
          fill_in_src_concat_relation(line);
        }
        break;
      case BOTH_CONCAT:
        {
          //toCout("Matched in both_concat");          
          fill_in_both_concat_relation(line);
        }
        break;
      case DEST_CONCAT:
        {
          //toCout("Matched in both_concat");          
          fill_in_dest_concat_relation(line);
        }
        break;
      case NONBLOCK: 
        {
          //toCout("Matched in nonblock");          
          std::smatch m;
          if(!std::regex_match(line, m, pNonblock)) {
            toCout("match nonblock wrongly: "+line);
            abort();
          }
          std::string destAndSlice = m.str(2);
          std::string dest, destSlice;
          split_slice(destAndSlice, dest, destSlice);
          if(!destSlice.empty()) {
            toCout("Warning: non-empty destSlice found!!!! "+line);
          }
          std::string src = m.str(3);
          if(isNum(src))
            break;
          if(dest.back() == ' ') {
            toCout("Warning: the last char is empty: "+dest);
            dest.pop_back();
          }
          moduleTrueRegs.push_back(dest);
          if(!destSlice.empty()) {
            if(g_trueReg2Slice.find(dest) != g_trueReg2Slice.end()) {
              g_trueReg2Slice[dest].insert(destSlice);
            }
            else {
              g_trueReg2Slice.emplace(dest, std::set<std::string>{destSlice});
            }
          }
          fill_in_pass_relation(destAndSlice, src, line);
        }
        break;
      case NONBLOCKIF:
        {
          toCout("Found nonblockif, which is not analyzed for reg path!");    
          std::smatch m;
          if(!std::regex_match(line, m, pNonblockIf)) {
            toCout("match nonblockif wrongly: "+line);
            abort();
          }
          std::string destAndSlice = m.str(3);
          std::string dest, destSlice;
          split_slice(destAndSlice, dest, destSlice);
          std::string srcAndSlice = m.str(4);
          //if(isNum(srcAndSlice))
          //  break;
          //if(dest.back() == ' ') {
          //  toCout("Warning: the last char is empty: "+dest);
          //  dest.pop_back();
          //}
          moduleTrueRegs.push_back(dest);

          //fill_in_pass_relation(destAndSlice, srcAndSlice, line); 
        }
        // TODO: add support for pAlwaysClkRst 
        break;
      case CASE:
        {
          auto currentPos = input.tellg();
          std::string nextLine;
          std::getline(input, nextLine);
          std::getline(input, nextLine);
          input.seekg(currentPos);
          collect_case_dest(nextLine);
        }
      default:
        break;
    }
  }
}


void add_line_taints(std::string line, std::ofstream &output, std::ifstream &input) 
{
  line = further_clean_line(line);
  std::string printedLine = line;
  int choice = parse_verilog_line(line);
  if(choice == REG) {
    int pos = printedLine.find("reg", 0);
    if(g_use_jasper) printedLine.replace(pos, 3, "logic");
  }
  else if (choice == WIRE) {
    int pos = printedLine.find("wire", 0);
    if(g_use_jasper) printedLine.replace(pos, 4, "logic");
  }
  std::smatch m;
  // Do not print lines matching these patterns
  // because they are treated separately
  if ( !std::regex_match(line, m, pModule) 
      && !std::regex_match(line, m, pEndmodule)
      && !std::regex_match(line, m, pInstanceBegin))
    output << printedLine << std::endl;

  switch( choice ) {
    case INPUT:
      input_taint_gen(line, output);
      break;
    case REG:      
      reg_taint_gen(line, output);      
      break;
    case WIRE:  
      wire_taint_gen(line, output);
      break;
    case MEM:
      mem_taint_gen(line, output);
      break;
    case OUTPUT:
      output_insert_map(line, output, input);
      break;
    case TWO_OP:
      two_op_taint_gen(line, output);      
      break;
    case ONE_OP:
      one_op_taint_gen(line, output);      
      break;
    case REDUCE1:
      reduce_one_op_taint_gen(line, output);
      break;
    case SEL:
      sel_op_taint_gen(line, output);
      break;
    case SRC_CONCAT:
      mult_op_taint_gen(line, output);
      break;
    case BOTH_CONCAT:
      both_concat_op_taint_gen(line, output);
      break;
    case DEST_CONCAT:
      dest_concat_op_taint_gen(line, output);
      break;
    case ITE:
      ite_taint_gen(line, output);      
      break;
    case NONBLOCK:
      nonblock_taint_gen(line, output);
      break;
    case NONBLOCKCONCAT:
      nonblockconcat_taint_gen(line, output);
      break;
    case ALWAYS:
      always_taint_gen(line, input, output);
      break;
    case ALWAYS_CLKRST:
      always_clkrst_taint_gen(line, input, output);
      break;
    case ALWAYS_FAKE:
      always_fake_taint_gen(line, input, output);
      break;
    case ALWAYS_STAR:
      always_star_taint_gen(line, input, output);
      break;
    case ALWAYS_NEG:
      always_neg_taint_gen(line, input, output);      
    case FUNCDEF:
      break;
    case NONE:
      break;
    case UNSUPPORT:
      std::cout << "!!! Unsupported operator in: "+ line + "|" << std::endl;
      break;
    default:
      toCout("Unexpected operations found: "+line);
      break;
  }
}


// FIXME: maybe set t-taint and r-taint to clear if reg value is not changed
int parse_verilog_line(std::string line, bool ignoreWrongOp) {
  // for the purpose of parse, remove potential $sign()
  //line = remove_signed(line);
  remove_back_space(line);
  if(line.empty()) {
    return NONE;
  }
  if(line.find("addedVar0") != std::string::npos)
    toCoutVerb("Find it!");
  if(line.substr(0, 1) == "X") {
    toCout("begin debug");
    ignoreWrongOp = true;
  }
  std::smatch m;
  if ( std::regex_match(line, m, pModule) ) {
    moduleName = m.str(2);
    return MODULE;
  }
  else if (std::regex_match(line, pInput)) {
    return INPUT;
  }
  else if (std::regex_match(line, pOutput)) {
    return OUTPUT;
  }
  else if (std::regex_match(line, pMem)) {
    return MEM;
  }
  else if (std::regex_match(line, pReg)
            || std::regex_match(line, pRegConst)) {
    return REG;
  }
  /* every wire type also needs _t and _r, both are wires */
  /* The reason why these wires are named _t, _r not _t_ */
  else if (std::regex_match(line, pWire)) {
    return WIRE;
  }
  /* 2-operator assignment */
  else if (std::regex_match(line, pAdd)
            || std::regex_match(line, pSub)
            || std::regex_match(line, pMult)
            || std::regex_match(line, pMod)
            || std::regex_match(line, pDvd)
            || std::regex_match(line, pAnd)
            || std::regex_match(line, pEq)
            || std::regex_match(line, pEq3)
            || std::regex_match(line, pNeq)
            || std::regex_match(line, pLt)
            || std::regex_match(line, pLe)
            || std::regex_match(line, pSt)
            || std::regex_match(line, pSe)
            || std::regex_match(line, pSignedLt)
            || std::regex_match(line, pSignedLe)
            || std::regex_match(line, pSignedSt)
            || std::regex_match(line, pSignedSe)
            || std::regex_match(line, pOr)
            || std::regex_match(line, pBitOr)
            || std::regex_match(line, pBitExOr)
            || std::regex_match(line, pBitAnd)
            || std::regex_match(line, pBitXnor1)
            || std::regex_match(line, pBitXnor2)
            || std::regex_match(line, pLeftShift)
            || std::regex_match(line, pRightShift)
            || std::regex_match(line, pSignedRightShift)
            || std::regex_match(line, pSignedLeftShift)
            || std::regex_match(line, pBitOrRed2) ) {
    return TWO_OP;
  } // end of 2-operator
  /* 1-operator assignment */
  else if (std::regex_match(line, pInvert) 
            || std::regex_match(line, pPlus) 
            || std::regex_match(line, pMinus) 
            || std::regex_match(line, pNone)) {
    return ONE_OP;
  }
  else if ( std::regex_match(line, pRedOr)
            || std::regex_match(line, pNot)
            || std::regex_match(line, pRedAnd)
            || std::regex_match(line, pRedNand)
            || std::regex_match(line, pRedNor)
            || std::regex_match(line, pRedXor)
            || std::regex_match(line, pRedXnor) ) {
    return REDUCE1;
  }
  else if (std::regex_match(line, pSel1)
            || std::regex_match(line, pSel2)
            || std::regex_match(line, pSel3)
            || std::regex_match(line, pSel4)
            || std::regex_match(line, pSel5)) {
    return SEL;
  }
  //else if (std::regex_match(line, pSrcConcat)) {
  else if (is_srcConcat(line)) {
    return SRC_CONCAT;
  }
  //else if (std::regex_match(line, pSrcDestBothConcat)) {
  else if (is_srcDestConcat(line)) {
    return BOTH_CONCAT;
  }
  else if (is_destConcat(line)) {
    return DEST_CONCAT;
  }
  else if (std::regex_match(line, pIte)) { // if cond is rst, then does not add any taint
    return ITE;
  } // end of ite
  else if( std::regex_match(line, pEnd)
            || std::regex_match(line, pEndmodule) ){
    return NONE;
  }
  else if( std::regex_match(line, pAlwaysClk) ) {
    return ALWAYS;
  }
  else if( std::regex_match(line, pAlwaysClkRst) ) {
    return ALWAYS_CLKRST;
  }
  else if( std::regex_match(line, pAlwaysFake) ) {
    return ALWAYS_FAKE;
  }
  else if( line.find("always @*") != std::string::npos || std::regex_match(line, pAlwaysStar) ) {
    return ALWAYS_STAR;
  }
  else if( std::regex_match(line, pAlwaysNeg) ) {
    return ALWAYS_NEG;
  }
  else if( std::regex_match(line, pNonblockIf) ) {
    return NONBLOCKIF;
  }
  else if( std::regex_match(line, pNonblock) ) {
    return NONBLOCK;
  }
  else if( std::regex_match(line, pNonblockConcat) ) {
    return NONBLOCKCONCAT;
  }
  else if( std::regex_match(line, pCase) ) {
    return CASE;
  }
  else if( std::regex_match(line, pFunctionDef) ) {
    return FUNCDEF;
  }
  else if( std::regex_match(line, pEndfunction) ) {
    return FUNCEND;
  }
  else if( std::regex_match(line, pInstanceBegin) ) {
    return INSTANCEBEGIN;
  }
  else if( std::regex_match(line, pIf) ) {
    return IF;
  }
  else if( std::regex_match(line, pElse) ) {
    return ELSE;
  }
  else if( std::regex_match(line, pDynSel) ) {
    return DYNSEL;
  }
  else if(line.find("(*") == std::string::npos){
    if(!ignoreWrongOp) {
      std::cout << "!! Unsupported operator:" + line << std::endl;
      if(g_use_jasper) abort();
    }
  } 

  return UNSUPPORT; // ???
}


void read_in_clkrst(std::string filePath, std::string fileName) {
  // set default name for these two variables
  g_clkrst_exist = true;
  clockName = "clk";
  resetName = "rst";
  std::string signName;
  std::string path = extract_path(filePath);
  std::ifstream in(path+"/"+fileName);
  std::string line;
  std::string localModName;
  std::smatch match;
  static const std::regex pClk("^clk\\:\\s*([a-zA-Z0-9_:'\\[\\]]+)\\s*$");
  static const std::regex pRst("^rst\\:\\s*([a-zA-Z0-9_:'\\[\\]!]+)\\s*$");
  static const std::regex pSign("^sign\\:\\s*([a-zA-Z0-9_:'\\[\\]]+)\\s*$");
  static const std::regex pModuleLocal("^module\\:\\s*([a-zA-Z0-9_:'\\[\\]\\$\\\\]+)\\s*$");
  while( std::getline(in, line) ) {
    if ( line.substr(0, 7) == "module:" ) {
      if( !std::regex_match(line, match, pModuleLocal) ) {
        toCout("Error: module name does not match pattern: "+line);
        abort();
      }
      localModName = match.str(1);
    }
    else if( line.substr(0, 4) == "clk:" ) {
      if ( !std::regex_match(line, match, pClk) ) {
        toCout("Error: clk name does not match pattern: "+line);
        abort();
      }
      clockName = match.str(1);
      if(g_moduleClk.find(localModName) != g_moduleClk.end()) {
        toCout("Error: repeated clk found for: "+localModName);
        abort();
      }
      g_moduleClk.emplace(localModName, clockName);
      toCout("+++ Get clock: "+clockName);
    }
    else if ( line.substr(0, 4) == "rst:" ) {
      if ( !std::regex_match(line, match, pRst) ) {
        toCout("Error: rst name does not match pattern: "+line);
        abort();
      }
      resetName = match.str(1);
      if(g_moduleRst.find(localModName) != g_moduleRst.end()) {
        toCout("Error: repeated clk found for: "+localModName);
        abort();
      }
      bool isPos = resetName.front() != '!';
      if(!isPos)
        resetName = resetName.substr(1);
      g_moduleRst.emplace(localModName, std::make_pair(resetName, isPos));
      toCout("+++ Get reset: "+resetName);      
    }
    else if( std::regex_match(line, match, pSign) ) {
      signName = match.str(1);
      if(signName.compare("pos") == 0)
        g_possibleSign = true;
      else if(signName.compare("neg") == 0)
        g_possibleSign = false;
      else
        checkCond(false, "Error: false sign name is given: "+signName);
      break;
    }
  }
  in.close();
}


void add_file_taints(std::string fileName, 
                     std::map<std::string, 
                     std::vector<std::string>> &moduleInputsMap, 
                     std::map<std::string, std::vector<std::string>> &moduleOutputsMap, 
                     std::map<std::string, std::vector<std::string>> &moduleRFlagsMap) {
  // read pre-defined clk and rst
  if(g_moduleClk.find(moduleName) != g_moduleClk.end()) {
    g_hasClk = true;
    g_recentClk = g_moduleClk[moduleName];
  }
  if(g_moduleRst.find(moduleName) != g_moduleRst.end()) {
    g_hasRst = true;
    g_clkrst_exist = true;
    g_recentRst = g_moduleRst[moduleName].first;
    g_recentRst_positive = g_moduleRst[moduleName].second;
  }

  long long int lineNo = 0;
  std::ifstream input(fileName);
  std::ofstream output(fileName + ".tainted");
  std::string line;
  std::smatch match;
  // +2 because of reserving 0 and 1.
  g_sig_width = uint32_t(ceil(log(g_reg_count+3) / log(2)));
  checkCond(CONSTANT_SIG_NUM == 1, "unexpected CONSTANT_SIG_NUM!!");
  CONSTANT_SIG = toStr(g_sig_width) + "'b1";
  // Reserve first line for module declaration
  while( std::getline(input, line) ) {
    toCoutVerb(line);
    if(line.find("_000_") != std::string::npos)
      toCoutVerb("Find it!");
    lineNo++;
    if ( std::regex_match(line, match, pAlwaysComb) ) {
      add_case_taints_limited(input, output, line);
    }
    else if( !g_use_jasper && std::regex_match(line, match, pFunctionDef)) {
      print_function_lines(input, output, line);
    }
    else if (   g_use_jasper
                && std::regex_match(line, match, pFunctionCall) 
        // TODO: remove this if statement
                && match.str(3).compare("$signed") != 0 ) {
      //add_func_taints_call_limited(line, output);
      toCout("!! Error: function found!");
      abort();
    }
    else if ( std::regex_match(line, match, pInstanceBegin) ) {
      extend_module_instantiation(input, output, line, moduleInputsMap, moduleOutputsMap);
    }
    else
      add_line_taints(line, output, input);  
  }
  input.close();
  output.close();
}


/* merge _c, _r, _x */
void merge_taints(std::string fileName) {
  std::ofstream output(fileName, std::fstream::app);
  // assign _c, _x
  //std::vector<std::string> appendix{_x};
  //for (std::string app : appendix) {  
  //  for ( auto it = nextVersion.begin(); it != nextVersion.end(); ++it ) {
  //    if ( isNum(it->first) ) continue;
  //      output << "  assign " + it->first + app + " = ( ";
  //    for (uint32_t i = 0; i < it->second - 1; i++)
  //      output << it->first + app + std::to_string(i) + " ) | ( ";
  //    output << it->first + app + std::to_string(it->second - 1) + " );" << std::endl;
  //  }
  //}

  // _r
  if(g_enable_taint) {
    for ( auto it = nextVersion.begin(); it != nextVersion.end(); ++it ) {
      if(isMem(it->first)) {
        auto slicePair = memDims[it->first];
        std::string sliceTop = slicePair.second;
        std::string highIdx = toStr(get_end(sliceTop));
        output << "  always @(*) begin" << std::endl;
        output << "    for(i = 0; i < "+highIdx+"; i = i + 1) begin" << std::endl;
        output << "      "+it->first+_r+" [i] = (";
        for (uint32_t i = 0; i < it->second - 1; i++) {
          if(g_has_read_taint) {
            //output << it->first + _x + std::to_string(i) + " [i] & ";
            output << it->first + _r + std::to_string(i) + " [i] ) | ( ";
          }
          else { // if do not want read taint
            //output << it->first + _x + std::to_string(i) + " [i] ) | ( ";
          }
        }
        if(g_has_read_taint) {
          //output << it->first + _x + std::to_string(it->second - 1) + " [i] & ";
          output << it->first + _r + std::to_string(it->second - 1) + " [i] );" << std::endl;
        }
        else {
          //output << it->first + _x + std::to_string(it->second - 1) + " [i] );" << std::endl;
        }
        output << "    end" << std::endl;
        output << "  end" << std::endl;
      }
      else {
        output << "  assign " + it->first + _r+" = ( ";
        for (uint32_t i = 0; i < it->second - 1; i++) {
          if(g_has_read_taint) {
            //output << it->first + _x + std::to_string(i) + " & ";
            //output << it->first + _c + std::to_string(i) + " & ";
            output << it->first + _r + std::to_string(i) + " ) | ( ";
          }
          else {
            //output << it->first + _x + std::to_string(i) + " ) | ( ";
          }
        }
        if(g_has_read_taint) {      
          //output << it->first + _x + std::to_string(it->second - 1) + " & ";
          //output << it->first + _c + std::to_string(it->second - 1) + " & ";
          output << it->first + _r + std::to_string(it->second - 1) + " );" << std::endl;
        }
        else {
          //output << it->first + _x + std::to_string(it->second - 1) + " );" << std::endl;
        }
      }
    }
  }

  // ground taints for floating regs
  if(g_enable_taint) {
    output << " // ground taints for floating regs" << std::endl;
    for (auto reg : moduleRegs) {
      if ( isNum(reg) ) {
        std::cout << "find num in nextVersion: " + reg << std::endl;
        continue;
      }
      if ( nextVersion.find(reg) == nextVersion.end() ) {
        output << "  assign " + reg + _r+ " = 0;" << std::endl;  
      }
    }
    // these wires are never used as inputs
    output << " // ground taints for unused wires" << std::endl;
    std::string outputStr = "";
    for (auto wire : moduleWires) {
      if ( isNum(wire) ) {
        std::cout << "find num in nextVersion: " + wire << std::endl;
        continue;
      }
      if ( nextVersion.find(wire) == nextVersion.end() ) {
        outputStr = outputStr + wire + _r+ " , ";
      }
    }
    for (auto input : moduleInputs) {
      if ( nextVersion.find(input) == nextVersion.end() )   
        outputStr = outputStr + input + _r+ " , ";
    }
    if(outputStr.length() > 2) {
      outputStr.pop_back();
      outputStr.pop_back();
      output << "  assign { " + outputStr + " } = 0;" << std::endl; 
    }
  }


  if(g_check_invariance == CheckOneVal || g_check_invariance == CheckTwoVal) {
    for(std::string var: moduleTrueRegs) {
      uint32_t width = get_var_slice_width(var);
      if(isMem(var))
        continue;
      std::string rstVal;
      if(g_use_vcd_parser)
        rstVal = g_rstValMap[moduleName][var];
      if(rstVal.empty()) rstVal = "0";
      output << "  always @( posedge " + g_recentClk + " ) begin" << std::endl;
      if(g_hasRst) {
        output << "    if( rst_zy ) " + var + "_PREV_VAL1 <= " + rstVal + " ;" << std::endl;
        if(g_check_invariance == CheckTwoVal)
          output << "    if( rst_zy ) " + var + "_PREV_VAL2 <= " + rstVal + " ;" << std::endl;
      }
      else {
        output << "    if( rst_zy ) " + var + "_PREV_VAL1 <= " + rstVal + " ;" << std::endl;
        if(g_check_invariance == CheckTwoVal)  
          output << "    if( rst_zy ) " + var + "_PREV_VAL2 <= " + rstVal + " ;" << std::endl;
      }
      output << "    if( INSTR_IN_ZY ) " + var + "_PREV_VAL1 <= " + var + " ;"<< std::endl;
      if(g_check_invariance == CheckTwoVal)
        output << "    if( INSTR_IN_ZY ) " + var + "_PREV_VAL2 <= " + var + "_PREV_VAL1 ;" << std::endl;
      output << "  end" << std::endl;
    }
  }

  // some bits of taints are still floating
  //output << "// ground floating taints" << std::endl;
  output << " // ground taints for unused wire slices" << std::endl;
  if(g_enable_taint) {
    for(auto it = nxtVerBits.begin(); it != nxtVerBits.end(); ++it) {
      uint32_t verNum = nextVersion[it->first];
      std::vector<std::string> freeBitsVec;
      free_bits(it->first, freeBitsVec);
      if(freeBitsVec.size() > 0) {
        output << "  assign " + add_taint(freeBitsVec, _r+toStr(verNum-1)) + " = 0;" << std::endl;
        //output << "  assign " + add_taint(freeBitsVec, _x+toStr(verNum-1)) + " = 0;" << std::endl;
        //output << "  assign " + add_taint(freeBitsVec, _c+toStr(verNum-1)) + " = 0;" << std::endl;
      }
    }

    // _r_flag_top
    for( auto it = moduleMems.begin(); it != moduleMems.end(); ++it ) {
      std::string mem = it->first;
      uint32_t len = it->second;
      output << "  assign " + mem + "_r_flag_top  = "; 
      for(uint32_t i = 0; i < len-1; i++) {
        output << mem + "_r_flag [" + toStr(i) + "] | ";
      }
      output << mem + "_r_flag [" + toStr(len-1) + "] ;" << std::endl;
    }
  }

  gen_assert_property(output);
  if(g_hasRst && isTop)
    output << "  assign rst_zy = "+get_recent_rst()+" ;" << std::endl;

  // write_taint_exist
  //output << "  logic write_taint_exist = 0";
  //if(isTop) {
  //  for( auto it = moduleTrueRegs.begin(); it != moduleTrueRegs.end(); it++ ) {
  //    output << " || " + *it + _t + " > 0 ";
  //  }
  //  for( auto it = moduleMems.begin(); it != moduleMems.end(); it++ ) {
  //    output << " || " + it->first + _t + " > 0 ";
  //  }
  //}
  //output << " ;" << std::endl;

  output << "endmodule" << std::endl;
  output << std::endl;

  //if(!g_use_jasper) {
  //  output << "module assert(input clk, input test);" << std::endl;
  //  output << "    always @(posedge clk)" << std::endl;
  //  output << "    begin" << std::endl;
  //  output << "        if (test !== 1)" << std::endl;
  //  output << "        begin" << std::endl;
  //  output << "            $display(\"ASSERTION FAILED in %m\");" << std::endl;
  //  output << "        end" << std::endl;
  //  output << "    end" << std::endl;
  //  output << "endmodule" << std::endl;
  //}

  output.close();
}


void add_module_name(std::string fileName, 
                     std::map<std::string, std::vector<std::string>> &moduleInputsMap,
                     std::map<std::string, std::vector<std::string>> &moduleOutputsMap,
                     std::map<std::string, std::vector<std::string>> &moduleRFlagsMap, 
                     bool isTopIn) {
  std::ifstream in(fileName);
  std::ofstream out(fileName + ".final");
  std::string line;
  std::smatch match;
  if(!isTop || !g_use_jasper) {
    moduleInputs.push_back("INSTR_IN_ZY");    
  }
  if(g_use_taint_rst) moduleInputs.push_back(TAINT_RST);
  g_use_end_sig = g_use_end_sig && !g_use_jasper;
  if(g_use_end_sig) moduleInputs.push_back(END_SIG);  
  if(moduleName == "hls_target_Loop_1_proc") {
    toCoutVerb("Find it!");
  }
  if(g_check_invariance == CheckTwoVal) moduleInputs.push_back(ASSERT_PROTECT);  
  if(!isTop) moduleInputs.push_back("rst_zy");
  out << "module " + moduleName + " ( " + *moduleInputs.begin();  
  for (auto it = moduleInputs.begin()+1; it != moduleInputs.end(); ++it) 
    out << " , " + *it ;
  if(g_enable_taint)
    for (auto it = extendInputs.begin(); it != extendInputs.end(); ++it)
      out << " , " + *it ;
  for (auto it = moduleOutputs.begin(); it != moduleOutputs.end(); ++it)
    out << " , " + *it ;

  if(g_enable_taint && extendOutputs.size() > 0) {
    for (auto it = extendOutputs.begin(); it != extendOutputs.end(); ++it)
      out << " , " + *it ;
  }
  out << " );" << std::endl;  
  // if no reset, add a reset
  if(isTop && g_use_jasper )
    out << "  logic rst_zy;" << std::endl;
  else if(isTop)
    out << "  wire rst_zy;" << std::endl;    
  else
    out << "  input rst_zy;" << std::endl;
  if(g_use_taint_rst) out << "  input "+TAINT_RST+";" << std::endl;
  if(g_use_end_sig) out << "  input "+END_SIG+";" << std::endl;
  if(g_check_invariance == CheckTwoVal) 
    out << "  input "+ASSERT_PROTECT+";" << std::endl;
  out << "  integer i;" << std::endl;
  if(!isTop || !g_use_jasper)
    out << "  input INSTR_IN_ZY;" << std::endl;
  else if(g_enable_taint){
    out << "  wire INSTR_IN_ZY;" << std::endl;
    out << "  assign INSTR_IN_ZY = ";
    for (auto it = moduleInputs.begin(); it != moduleInputs.end(); ++it) {
      if((*it).compare(g_recentClk) == 0 
          || (*it).compare(g_recentRst) == 0 
          || (*it).compare("rst_zy") == 0 
          || (*it).compare(TAINT_RST) == 0 
          || (*it).compare(ASSERT_PROTECT) == 0 
          || (*it).compare(END_SIG) == 0)
        continue;
      out << *it + _t + " > 0 || ";
    }
    out << "0 ;" << std::endl;
  }
  while( std::getline(in, line) ) {
    out << line << std::endl;
  }
  if(!isTopIn) {
    moduleInputsMap.emplace(moduleName, moduleInputs);
    moduleOutputsMap.emplace(moduleName, moduleOutputs);
    moduleRFlagsMap.emplace(moduleName, flagOutputs);
    //moduleInputsMap[moduleName].insert( moduleInputsMap[moduleName].end(), extendInputs.begin(), extendInputs.end() );
    //moduleOutputsMap[moduleName].insert( moduleOutputsMap[moduleName].end(), extendOutputs.begin(), extendOutputs.end() );
  }
  in.close();
  out.close();
}


void fill_update(std::string fileName) {
  std::ifstream in(fileName);
  std::string line;
  std::smatch m;
  std::string reg;
  // push all the vars on RHS of nonblocking to new_reg
  while( std::getline(in, line) ) {
    if ( std::regex_match(line, m, pNonblock) )
      g_wire2reg.insert( m.str(3) );
    else if ( std::regex_match(line, m, pNonblockConcat) ) {
      std::vector<std::string> updateVec;
      parse_var_list(m.str(3), updateVec);
      for (std::string update: updateVec) {
        g_wire2reg.insert( update );
      }
    }
  }
}


/* assume func only contains case, case has only the 3rd input, the case condtion is non-numerical 
 * Print the original func, and also _t, _r, _x, _c taint functions */
void add_func_taints_limited(std::ifstream &input, std::ofstream &output, std::string funcDefinition) {
  std::cout << "DO: add_func_taints_limited" << std::endl;  
  std::smatch m;
  if ( !std::regex_match(funcDefinition, m, pFunctionDef) )
    return;
  std::string blank = m.str(1);
  std::string funcSlice = m.str(2);
  std::string funcName = m.str(3);
  uint32_t condWidthNum;
  
  // extract the (caseValue, caseAssign) pair from the case
  std::vector<std::pair<std::string, std::string>> caseAssignPairs;
  std::vector<std::string> inputSlice;
  parse_func_statements(caseAssignPairs, inputSlice, input);

  // print the original function
  std::string line;
  uint32_t lineNo = 0;
  while ( std::getline(input, line) && !std::regex_match(line, m, pEndfunction) ) {
    lineNo++;
    output << line << std::endl;
    if(lineNo == 3) {
      if (!std::regex_match(line, m, pInput))
        std::cout << "!! Error: Wrong " << std::endl;
      std::string slice = m.str(2);
      condWidthNum = get_width(slice);
    }
  }
  output << line << std::endl << std::endl;

  std::string rhs, rhsSlice;
  uint32_t destWidth = get_width(inputSlice[0]);
  assert( destWidth == get_width(funcSlice) );
  uint32_t aWidth = destWidth;
  uint32_t bWidth = get_width(inputSlice[1]);
  uint32_t sWidth = get_width(inputSlice[2]);
  uint32_t allInputWidth = destWidth + bWidth + sWidth;
  // print _t function
  output << blank + "function automatic" + funcSlice + funcName + _t +" ;" << std::endl;
  output << blank + "  input " + inputSlice[0] + " a"+_t+" ;" << std::endl;
  output << blank + "  input " + inputSlice[1] + " b"+_t+" ;" << std::endl;
  output << blank + "  input " + inputSlice[2] + " s"+_t+" ;" << std::endl;
  output << blank + "  input " + inputSlice[2] + " s ;" << std::endl;
  output << blank + "  reg s_t_1bit = 0;" << std::endl;
  output << blank + "  reg " + inputSlice[0] + " s_t_full = 0;"<< std::endl;
  output << blank + "  begin" << std::endl;
  output << blank + "    s_t_1bit = | s"+_t+" ;" << std::endl;  
  output << blank + "    s_t_full = " + extend("s_t_1bit", destWidth) + " ;" << std::endl;
  output << blank + "    casez (s) " << std::endl;
  for(auto localPair: caseAssignPairs) {
    split_slice(localPair.second, rhs, rhsSlice);
    output << blank + "    " + localPair.first + " :" << std::endl;
    output << blank + "      " + funcName + _t + " = s_t_full | " + rhs + _t + " " + rhsSlice + ";" << std::endl;
  }
  output << blank + "    endcase" << std::endl;
  output << blank + "  end" << std::endl;  
  output << blank + "endfunction" << std::endl << std::endl;

  // TODO: needs to be modified
  // print _r function, output is (a_r, b_r, s_r), input is (dest_r, s)
  std::string taintString = pairVec2taintString(caseAssignPairs, "no_reg_is_excluded_2333", _r, output);
  output << blank + "function automatic [" + toStr(allInputWidth-1) + ":0] " + funcName + _r + " ;" << std::endl;
  output << blank + "  input " + funcSlice + "dest"+_r+" ;" << std::endl;
  output << blank + "  input " + inputSlice[2] + " s ;" << std::endl;  
  output << blank + "  reg dest_r_1bit = 0;" << std::endl;
  output << blank + "  reg " + inputSlice[0] + " a"+_r+" = 0;" << std::endl; 
  output << blank + "  reg " + inputSlice[1] + " b"+_r+" = 0;" << std::endl; 
  output << blank + "  reg " + inputSlice[2] + " s"+_r+" = 0;" << std::endl;
  output << blank + "  begin" << std::endl;
  output << blank + "    dest_r_1bit = | dest"+_r+" ;" << std::endl;  
  output << blank + "    s"+_r+" = " + extend("dest_r_1bit", sWidth) + " ;" << std::endl;
  output << blank + "    " + funcName + _r+" = {a"+_r+", b"+_r+", s"+_r+"};" << std::endl;
  output << blank + "    b"+_r+" = 0 ;" << std::endl;
  output << blank + "    a"+_r+" = 0 ;" << std::endl;
  output << blank + "    casez (s) " << std::endl;
  for(auto localPair: caseAssignPairs) {
    split_slice(localPair.second, rhs, rhsSlice);
    output << blank + "    " + localPair.first + " :" << std::endl;
    output << blank + "      " + rhs + _r + " " + rhsSlice + " = dest"+_r+" ;" << std::endl;
  }
  output << blank + "    endcase" << std::endl;
  output << blank + "  end" << std::endl;  
  output << blank + "endfunction" << std::endl << std::endl;

  // print _x function, output is (a_x, b_x, s_x), input is (dest_x, s)
  //taintString = pairVec2taintString(caseAssignPairs, "no_reg_is_excluded_2333", _x, output);  
  //output << blank + "function automatic [" + toStr(allInputWidth-1) + ":0] " + funcName + _x+" ;" << std::endl;
  //output << blank + "  input " + funcSlice + "dest"+_x+" ;" << std::endl;
  //output << blank + "  input " + inputSlice[2] + " s ;" << std::endl;  
  //output << blank + "  reg dest_x_1bit = 0;" << std::endl;
  //output << blank + "  reg " + inputSlice[0] + " a"+_x+" = 0;" << std::endl; 
  //output << blank + "  reg " + inputSlice[1] + " b"+_x+" = 0;" << std::endl; 
  //output << blank + "  reg " + inputSlice[2] + " s"+_x+" = 0;" << std::endl; 
  //output << blank + "  begin" << std::endl;
  //output << blank + "    dest_x_1bit = | dest"+_x+" ;" << std::endl;  
  //output << blank + "    s"+_x+" = " + extend("dest_x_1bit", sWidth) + " ;" << std::endl;
  //output << blank + "    " + funcName + _x+" = {a"+_x+", b"+_x+", s"+_x+"};" << std::endl;
  //output << blank + "    b"+_x+" = 0 ;" << std::endl;
  //output << blank + "    a"+_x+" = 0 ;" << std::endl;
  //output << blank + "    casez (s) " << std::endl;
  //for(auto localPair: caseAssignPairs) {
  //  split_slice(localPair.second, rhs, rhsSlice);
  //  output << blank + "    " + localPair.first + " :" << std::endl;
  //  output << blank + "      " + rhs + _x+" " + rhsSlice + " = dest"+_x+" ;" << std::endl;
  //}
  //output << blank + "    endcase" << std::endl;
  //output << blank + "  end" << std::endl;  
  //output << blank + "endfunction" << std::endl << std::endl;

  // print _c function, output is (a_c, b_c, s_c), input is (dest_c, s)
  //taintString = pairVec2taintString(caseAssignPairs, "no_reg_is_excluded_2333", _c, output);
  //output << blank + "function automatic [" + toStr(allInputWidth-1) + ":0] " + funcName + _c+" ;" << std::endl;
  //output << blank + "  input " + funcSlice + "dest"+_c+" ;" << std::endl;
  //output << blank + "  input " + inputSlice[2] + " s ;" << std::endl;  
  //output << blank + "  reg " + inputSlice[0] + " a"+_c+" = 0;" << std::endl; 
  //output << blank + "  reg " + inputSlice[1] + " b"+_c+" = 0;" << std::endl; 
  //output << blank + "  reg " + inputSlice[2] + " s"+_c+" = 0;" << std::endl; 
  //output << blank + "  begin" << std::endl;
  //output << blank + "    s"+_c+" = " + extend("1'b1", sWidth) + " ;" << std::endl;
  //output << blank + "    " + funcName + _c+" = {a"+_c+", b"+_c+", s"+_c+"};" << std::endl;
  //output << blank + "    b"+_c+" = 0 ;" << std::endl;
  //output << blank + "    a"+_c+" = 0 ;" << std::endl;
  //output << blank + "    casez (s) " << std::endl;
  //for(auto localPair: caseAssignPairs) {
  //  split_slice(localPair.second, rhs, rhsSlice);
  //  output << blank + "    " + localPair.first + " :" << std::endl;
  //  output << blank + "      " + rhs + _c+" " + rhsSlice + " = dest"+_c+" ;" << std::endl;
  //}
  //output << blank + "    endcase" << std::endl;
  //output << blank + "  end" << std::endl;  
  //output << blank + "endfunction" << std::endl << std::endl;
}


/* firstLine is the function definition */
void remove_function_wrapper(std::string firstLine, std::ifstream &input, std::ofstream &output) {
  std::smatch m;
  if ( !std::regex_match(firstLine, m, pFunctionDef) )
    return;
  std::string blank = m.str(1);
  std::string funcSlice = m.str(2);
  std::string funcName = m.str(3);

  // extract the (caseValue, caseAssign) pair from the case
  std::vector<std::pair<std::string, std::string>> caseAssignPairs;
  std::vector<std::string> inputSlice;
  parse_func_statements(caseAssignPairs, inputSlice, input, true);
  // pop out the last, which is default assignment
  caseAssignPairs.pop_back();

  // the next line can be wire declarations or function calls
  std::string line;
  std::getline(input, line);
  uint32_t i = 0;
  while( !std::regex_match(line, m, pFunctionCall) ) {
    output << line << std::endl;
    std::getline(input, line);
    if(++i > 7) {
      toCout("!! Error in searching function calls");
      abort();
    }
  }

  std::string a, b, s; // used in function.
  std::string lineForParsing = line;
  if( !std::regex_match(lineForParsing, m, pFunctionCall) ) {
    toCout("Error in parsing function call");
    abort();
  }
  std::string result = m.str(2);
  std::string callFuncName = m.str(3);
  std::string arguments = m.str(4);
  std::vector<std::string> argVec;
  parse_var_list(arguments, argVec);
  a = argVec[0];
  b = argVec[1];
  s = argVec[2];
 
  bool bIsNum = isNum(b);
  bool aIsNum = isNum(a);

  //fill_in_case_relation(result, b, a, s, caseAssignPairs);

  if(!bIsNum) {
    // begin to print the new case
    output << blank + "always @("+a+" or "+b+" or "+s+") begin" << std::endl;
    output << blank + "  casez ("+s+")" << std::endl;
    std::string rhs, rhsSlice;
    for(auto localPair: caseAssignPairs) {
      split_slice(localPair.second, rhs, rhsSlice);
      output << blank + "    " + localPair.first + " :" << std::endl;
      output << blank + "      " + result + " = " + b + rhsSlice + " ;" << std::endl;
      // FIXME: figure out why this check is important? 
      //checkCond(b.find("addedVar") != std::string::npos, "RHS in case is not addedVar! "+ b);
      //fill_in_pass_relation(b+rhsSlice, result, localPair.first);
    }
    output << blank + "    default:" << std::endl;
    output << blank + "      " + result + " = " + a + " ;" << std::endl;
    output << blank + "  endcase" << std::endl;
    output << blank + "end" << std::endl;
    addedVarCaseSliceWidth.emplace(b, get_width(rhsSlice));
  }
  else {
    std::string localIdx = std::to_string(NEW_FANGYUAN++);
    if(!std::regex_search(b, m, pBin)
        && !std::regex_search(b, m, pHex)) {
      toCout("Error: the number for case input is not binary or hex!");
      toCout("First line: "+firstLine);
      toCout("b is: "+b);
      abort();
    }
    uint32_t addedVarWidth = std::stoi(m.str(1));
    bool insertDone = varWidth.var_width_insert("addedVar"+localIdx, addedVarWidth-1, 0);
    if (!insertDone) {
      std::cout << "insert failed for this line:" + line << std::endl;
      std::cout << "m.str(2):" + m.str(2) << std::endl;
      std::cout << "m.str(3):" + m.str(3) << std::endl;
    }
    //output << blank + "wire ["+toStr(addedVarWidth-1)+":0] addedVar"+localIdx+";" << std::endl;
    //output << blank + "assign addedVar"+localIdx+" = "+b+";" << std::endl;
    output << blank + "always @("+a+" or "+s+") begin" << std::endl;
    output << blank + "  casez ("+s+")" << std::endl;
    std::string rhs, rhsSlice;
    for(auto localPair: caseAssignPairs) {
      split_slice(localPair.second, rhs, rhsSlice);
      output << blank + "    " + localPair.first + " :" << std::endl;
      // extract the bits from number
      std::smatch m;
      uint32_t lowIdx = get_begin(rhsSlice);
      uint32_t highIdx = get_end(rhsSlice);
      //wholeNum = wholeNum >> lowIdx;
      //uint32_t selectedBits = wholeNum & ((1<<(highIdx-lowIdx+1))-1);
      //std::string binNum = dec2bin(selectedBits);
      //std::string binWidth = toStr(binNum.length());
      //std::string rhsNum = (selectedBits == 0) ? "0" : (binWidth + "'b" + binNum);
      std::string rhsBin = extract_bin(b, highIdx, lowIdx);
      //std::string rhsNum = "addedVar"+localIdx+"["+toStr(highIdx)+":"+toStr(lowIdx)+"]";
      output << blank + "      " + result + " = " + toStr(rhsBin.length()) + "'b" + rhsBin + " ;" << std::endl;
    }
    output << blank + "    default:" << std::endl;
    output << blank + "      " + result + " = " + a + " ;" << std::endl;
    output << blank + "  endcase" << std::endl;
    output << blank + "end" << std::endl;
  }
}


void add_case_taints_limited(std::ifstream &input, std::ofstream &output, std::string alwaysFirstLine) {
  output << alwaysFirstLine << std::endl;
  toCout("Do: add_case_taints_limited");
  std::smatch m;
  std::string caseFirstLine;
  std::getline(input, caseFirstLine);
  output << caseFirstLine << std::endl;
  if ( !std::regex_match(caseFirstLine, m, pCase) )
    return;
  std::string blank = m.str(1);
  std::string sAndSlice = m.str(3);
  std::vector<std::pair<std::string, std::string>> caseAssignPairs;
  std::vector<std::string> inputSlice;
  std::string destAndSlice = parse_case_statements(caseAssignPairs, input, true);
  // print the case statements
  std::string line;
  std::getline(input, line);
  while(line.find("endcase", 0) == std::string::npos ) {
    output << line << std::endl;
    std::getline(input, line);    
  }
  // print endcase
  output << line << std::endl;
  std::getline(input, line);
  // print end
  output << line << std::endl;
  if(!g_enable_taint) return;

  std::string dest, destSlice;
  std::string a, aSlice;
  std::string b, bSlice;
  std::string s, sSlice;
  std::string aAndSlice = caseAssignPairs.back().second;
  std::string bAndSlice = caseAssignPairs[0].second;
  split_slice(destAndSlice, dest, destSlice);
  split_slice(caseAssignPairs[0].second, b, bSlice);
  split_slice(caseAssignPairs.back().second, a, aSlice);
  split_slice(sAndSlice, s, sSlice);
  //assert_info(!isTop || !isOutput(s), "add_case_taints_limited:s is output, firstLine: "+alwaysFirstLine);
  //assert_info(!isTop || !isOutput(a), "add_case_taints_limited:s is output, firstLine: "+alwaysFirstLine);
  //assert_info(!isTop || !isOutput(b), "add_case_taints_limited:s is output, firstLine: "+alwaysFirstLine);
  // declare necessaey variables
  uint32_t destWidthNum, sWidthNum, aWidthNum, bWidthNum;
  std::string sWidth, aWidth, bWidth;
  std::string sVer, aVer, bVer;
  bool sIsNew, aIsNew, bIsNew;

  bool aIsNum = isNum(a);
  bool bIsNum = isNum(b);


  // assignmen for dest and s variables
  destWidthNum = get_var_slice_width(destAndSlice);
  sWidthNum = get_var_slice_width(sAndSlice);
  sWidth = toStr(sWidthNum);
  sVer = toStr(find_version_num(sAndSlice, sIsNew, output));

  if(!aIsNum && !bIsNum) {
    aWidthNum = get_var_slice_width(a);
    bWidthNum = get_var_slice_width(b);

    aWidth = toStr(aWidthNum);
    bWidth = toStr(bWidthNum);

    aVer = toStr(find_version_num(a, aIsNew, output, true));
    bVer = toStr(find_version_num(b, bIsNew, output, true));

    assert(aIsNew);
    assert(bIsNew);

    // print _t function
    output << blank + "always @( "+a+_t+" or "+b+_t+" or "+s+_t+" or "+s+" ) begin" << std::endl;
    output << blank + "  casez ("+sAndSlice+")" << std::endl;
    std::string rhs, rhsSlice;
    caseAssignPairs.pop_back();
    for(auto localPair: caseAssignPairs) {
      split_slice(localPair.second, rhs, rhsSlice);
      uint32_t bPos = localPair.first.find("b");
      if(bPos == std::string::npos) abort();
      uint32_t pos = localPair.first.find("1", bPos);
      uint32_t idx = localPair.first.length() - pos - 1;
      output << blank + "    " + localPair.first + " :" << std::endl;
      output << blank + "      " + dest+_t+destSlice + " = " + rhs + _t + rhsSlice 
                                                             + " | " + extend(s+_t+"["+toStr(idx)+"]", destWidthNum) + ";" << std::endl;
    }
    output << blank + "    default :" << std::endl;
    output << blank + "      " + dest+_t+destSlice + " = " + a + _t + aSlice 
                                                           + " | " + extend("| "+s+_t+sSlice, destWidthNum) + ";" << std::endl;
    output << blank + "  endcase" << std::endl;
    output << blank + "end" << std::endl;

    // print _sig function
    if(!g_use_value_change && b.find("addedVar") != std::string::npos) {
      uint32_t caseNum = 0;
      output << blank + "always @( "+a+_sig+" or "+b+_sig+" or "+s+" ) begin" << std::endl;
      output << blank + "  casez ("+sAndSlice+")" << std::endl;
      for(auto localPair: caseAssignPairs) {
        caseNum++;
        split_slice(localPair.second, rhs, rhsSlice);
        output << blank + "    " + localPair.first + " :" << std::endl;
        output << blank + "      " + dest + _sig + " = ( " + rhs + _sig + " [" + toStr(caseNum*g_sig_width-1) + " : " + toStr(g_sig_width*(caseNum-1)) + "] == " + CONSTANT_SIG + " ) ? " + s + _sig + " : " + rhs + _sig + " [" + toStr(caseNum*g_sig_width-1) + ":" + toStr(g_sig_width*(caseNum-1)) + "] ;" << std::endl;
      }
      output << blank + "    default :" << std::endl;
      output << blank + "      " + dest + _sig + " = ( " + a + _sig + " == " + CONSTANT_SIG + " ) ? " + s + _sig + " : " + a + _sig + " ;" << std::endl;
      output << blank + "  endcase" << std::endl;
      output << blank + "end" << std::endl;
      checkCond(caseNum == addedVarItemNum[rhs], "case number does not equal addedVar item number!, var: "+rhs+" , caseNum:"+toStr(caseNum)+" , itemNum:"+toStr(addedVarItemNum[rhs]));
    }
    else if(!g_use_value_change)
      output << "  assign " + dest + _sig + " = 0 ;" << std::endl;

    // print _r function
    if(sIsNew) {
    output << blank + "logic [" + sWidth + "-1:0] " + s + _r + sVer + " ;" << std::endl;
    //output << blank + "logic [" + sWidth + "-1:0] " + s + _x + sVer + " ;" << std::endl;
    //output << blank + "logic [" + sWidth + "-1:0] " + s + _c + sVer + " ;" << std::endl;
    }
    output << blank + "logic [" + aWidth + "-1:0] " + a + _r + aVer + " ;" << std::endl;
    //output << blank + "logic [" + aWidth + "-1:0] " + a + _x + aVer + " ;" << std::endl;
    //output << blank + "logic [" + aWidth + "-1:0] " + a + _c + aVer + " ;" << std::endl;

    output << blank + "logic [" + bWidth + "-1:0] " + b + _r + bVer + " ;" << std::endl;
    //output << blank + "logic [" + bWidth + "-1:0] " + b + _x + bVer + " ;" << std::endl;
    //output << blank + "logic [" + bWidth + "-1:0] " + b + _c + bVer + " ;" << std::endl;

    output << blank + "always @( "+dest+_r+destSlice+" or "+s+_t+sSlice+" or "+s+" ) begin" << std::endl;
    //output << blank + "  "+s+_r+sVer+" "+sSlice+" = " + extend("| "+dest+_r+destSlice, sWidthNum) + " ;" << std::endl;
    output << blank + "  "+b+_r+bVer+" = 0 ;" << std::endl;
    output << blank + "  "+a+_r+aVer+" = 0 ;" << std::endl;
    output << blank + "  casez ("+sAndSlice+")" << std::endl;
    for(auto localPair: caseAssignPairs) {
      split_slice(localPair.second, rhs, rhsSlice);
      output << blank + "    " + localPair.first + " :" << std::endl;
      output << blank + "      " + rhs + _r + bVer + rhsSlice + " = " + dest + _r + destSlice + " ;" << std::endl;
    }
    output << blank + "    default :" << std::endl;
    output << blank + "      " + a + _r + aVer + aSlice + " = " + dest + _r + destSlice + " ;" << std::endl;
    output << blank + "  endcase" << std::endl;
    output << blank + "end" << std::endl;  

    // _r for conditions
    output << blank + "always @( "+dest+_r+destSlice+" or "+s+" ) begin" << std::endl;
    output << blank + "  "+s+_r+sVer+" "+sSlice+" = 0 ;" << std::endl;
    output << blank + "  casez ("+sAndSlice+")" << std::endl;
    uint32_t i = 0;
    checkCond(sSlice.empty(), "Condition in case has slice: "+sAndSlice);
    for(auto localPair: caseAssignPairs) {
      split_slice(localPair.second, rhs, rhsSlice);
      output << blank + "    " + localPair.first + " :" << std::endl;
      output << blank + "      " + s+_r+sVer+" "+sSlice+"["+toStr(i++)+"]" + " = | " + dest + _r + destSlice + " ;" << std::endl;
    }
    //output << blank + "    default :" << std::endl;
    //output << blank + "      " + a + _r + aVer + aSlice + " = " + dest + _r + destSlice + " ;" << std::endl;
    output << blank + "  endcase" << std::endl;
    output << blank + "end" << std::endl;  

    // ground other wires of s
    auto sBoundPair = varWidth.get_idx_pair(s, alwaysFirstLine);
  } // end of !aIsNum && !bIsNum
  else if(!aIsNum && bIsNum) {
    aWidthNum = get_var_slice_width(a);
    aWidth = toStr(aWidthNum);
    bool aIsNew;
    aVer = toStr(find_version_num(a, aIsNew, output, true));
    assert(aIsNew);

    // print _t function
    output << blank + "always @( "+a+_t+" or "+s+_t+" or "+s+" ) begin" << std::endl;
    output << blank + "  casez ("+sAndSlice+")" << std::endl;
    std::string rhs, rhsSlice;
    // only the last one matters
    auto lastPair = caseAssignPairs.back();
    caseAssignPairs.pop_back();
    for(auto localPair: caseAssignPairs) {
      uint32_t bPos = localPair.first.find("b");
      if(bPos == std::string::npos) abort();
      uint32_t pos = localPair.first.find("1", bPos);
      uint32_t idx = localPair.first.length() - pos - 1;
      output << blank + "    " + localPair.first + " :" << std::endl;
      output << blank + "      " + dest+_t+destSlice+" = " + extend(s+_t+"["+toStr(idx)+"]", destWidthNum) + " ;" << std::endl;
    }
    output << blank + "    default:" << std::endl;
    output << blank + "      " +  dest+_t+destSlice+" = " + a + _t+ " " + aSlice + " | " + extend("| "+s+_t+sSlice, destWidthNum) + " ;" << std::endl;
    output << blank + "  endcase" << std::endl;
    output << blank + "end" << std::endl;
    //auto destBoundPair = varWidth.get_idx_pair(dest, line);
    //ground(dest+_t, destBoundPair, destSlice, blank, output);

    // _sig
    if(!g_use_value_change) {
      output << blank + "always @( "+a+_sig+" or "+s+" ) begin" << std::endl;
      output << blank + "  casez ("+sAndSlice+")" << std::endl;
      // only the last one matters
      for(auto localPair: caseAssignPairs) {
        output << blank + "    " + localPair.first + " :" << std::endl;
        output << blank + "      " + dest+_sig+destSlice+" = "+CONSTANT_SIG+" ;" << std::endl;
      }
      output << blank + "    default:" << std::endl;
      output << blank + "      " +  dest+_sig+destSlice+" = " + a + _sig+ " ;" << std::endl;
      output << blank + "  endcase" << std::endl;
      output << blank + "end" << std::endl;
    }

    // print _r function
    output << blank + "reg [" + sWidth + "-1:0] " + s + _r + sVer + " ;" << std::endl;

    output << blank + "reg [" + aWidth + "-1:0] " + a + _r + aVer + " ;" << std::endl;

    output << blank + "always @( "+dest+_r+destSlice+" or "+s+" ) begin" << std::endl;
    output << blank + "  "+s+_r+sVer+sSlice+" = " + extend("| "+dest+_r+destSlice, sWidthNum) + " ;" << std::endl;
    output << blank + "  "+a+_r+aVer+" = 0 ;" << std::endl;
    output << blank + "  if (" + sAndSlice + " == 0 )" << std::endl;
    output << blank + "    "+a+_r+aVer+" = "+dest+_r+destSlice+" ;" << std::endl;
    output << blank + "end" << std::endl;  

    // ground other wires of s
    auto sBoundPair = varWidth.get_idx_pair(s, alwaysFirstLine);

  } // end of !aIsNum && bIsNum
  else if(aIsNum && !bIsNum) {
    bWidthNum = get_var_slice_width(b);
    bWidth = toStr(bWidthNum);
    bVer = toStr(find_version_num(b, bIsNew, output, true));

    // print _t function
    output << blank + "always @( "+b+_t+" or "+s+_t+" or "+s+" ) begin" << std::endl;
    output << blank + "  casez ("+sAndSlice+")" << std::endl;
    std::string rhs, rhsSlice;
    caseAssignPairs.pop_back();
    for(auto localPair: caseAssignPairs) {
      split_slice(localPair.second, rhs, rhsSlice);
      uint32_t bPos = localPair.first.find("b");
      if(bPos == std::string::npos) abort();
      uint32_t pos = localPair.first.find("1", bPos);
      uint32_t idx = localPair.first.length() - pos - 1;
      output << blank + "    " + localPair.first + " :" << std::endl;
      output << blank + "      " + dest+_t+destSlice+" = " + rhs + _t+" " + rhsSlice + " | " + extend(s+_t+"["+toStr(idx)+"]", destWidthNum) + " ;" << std::endl;
    }
    output << blank + "    default:" << std::endl;
    output << blank + "      "+dest+_t+destSlice+" = "+ extend("| "+s+_t+sSlice, destWidthNum) + " ;" << std::endl;
    output << blank + "  endcase" << std::endl;
    output << blank + "end" << std::endl;


    // _sig
    if(!g_use_value_change && b.find("addedVar") != std::string::npos) {
      uint32_t caseNum = 0;
      output << blank + "always @( "+b+_sig+" or "+s+" ) begin" << std::endl;
      output << blank + "  casez ("+sAndSlice+")" << std::endl;
      for(auto localPair: caseAssignPairs) {
        caseNum++;
        split_slice(localPair.second, rhs, rhsSlice);
        output << blank + "    " + localPair.first + " :" << std::endl;
        output << blank + "      " + dest+_sig+" = " + rhs + _sig + " [" + toStr(caseNum*g_sig_width-1) + ":" + toStr(g_sig_width*(caseNum-1)) + "] ;" << std::endl;      
      }
      output << blank + "    default:" << std::endl;
      output << blank + "      " + dest+_sig+" = "+CONSTANT_SIG+" ;" << std::endl;
      output << blank + "  endcase" << std::endl;
      output << blank + "end" << std::endl;
    }
    else if(!g_use_value_change)
      output << "  assign " + dest + _sig + " = 0 ;" << std::endl;

    // print _r function
    output << blank + "reg [" + sWidth + "-1:0] " + s + _r + sVer + " ;" << std::endl;

    output << blank + "reg [" + bWidth + "-1:0] " + b + _r + bVer + " ;" << std::endl;

    output << blank + "always @( "+dest+_r+destSlice+" or "+s+" ) begin" << std::endl;
    output << blank + "  "+s+_r+sVer+sSlice+" = " + extend("| "+dest+_r+destSlice, sWidthNum) + " ;" << std::endl;
    output << blank + "  "+b+_r+bVer+" = 0 ;" << std::endl;
    output << blank + "  casez ("+sAndSlice+")" << std::endl;
    for(auto localPair: caseAssignPairs) {
      split_slice(localPair.second, rhs, rhsSlice);
      output << blank + "    " + localPair.first + " :" << std::endl;
      output << blank + "      " + rhs + _r + bVer + rhsSlice + " = " + dest + _r + destSlice + " ;" << std::endl;
    }
    output << blank + "  endcase" << std::endl;
    output << blank + "end" << std::endl;  

    // ground other wires of s
    auto sBoundPair = varWidth.get_idx_pair(s, alwaysFirstLine);

  } // end of aIsNum && !bIsNum
  else {
    // print _t function
    output << blank + "always @( "+s+_t+" or "+s+" ) begin" << std::endl;
    output << blank + "  casez ("+sAndSlice+")" << std::endl;
    std::string rhs, rhsSlice;
    // only the last one matters
    auto lastPair = caseAssignPairs.back();
    caseAssignPairs.pop_back();
    for(auto localPair: caseAssignPairs) {
      uint32_t bPos = localPair.first.find("b");
      if(bPos == std::string::npos) abort();
      uint32_t pos = localPair.first.find("1", bPos);
      uint32_t idx = localPair.first.length() - pos - 1;
      output << blank + "    " + localPair.first + " :" << std::endl;
      output << blank + "      " + dest+_t+destSlice+" = " + extend(s+_t+"["+toStr(idx)+"]", destWidthNum) + " ;" << std::endl;
    }
    output << blank + "    default:" << std::endl;
    output << blank + "      " + dest+_t+destSlice+" = " + extend("| "+s+_t+sSlice, destWidthNum) + " ;" << std::endl;
    output << blank + "  endcase" << std::endl;
    output << blank + "end" << std::endl;

    // print _sig function
    if(!g_use_value_change)
      output << blank + "assign " + dest + _sig + " = "+CONSTANT_SIG+" ;" << std::endl;

    // print _r function
    output << blank + "reg [" + sWidth + "-1:0] " + s + _r + sVer + " ;" << std::endl;

    output << blank + "always @( "+dest+_r+destSlice+" or "+s+" ) begin" << std::endl;
    output << blank + "  "+s+_r+sVer+sSlice+" = " + extend("| "+dest+_r+destSlice, sWidthNum) + " ;" << std::endl;
    output << blank + "end" << std::endl;  

  }
}


/* first print this func call, then print all the taint calls */
void add_func_taints_call(std::string line, std::ofstream &output) {
  std::smatch m;
  if( !std::regex_match(line, m, pFunctionCall) )
    return;
  std::string blank = m.str(1);
  std::string returnValue = m.str(2);
  std::string funcName = m.str(3);
  std::string arguments = m.str(4);

  // print the func call
  output << line << std::endl;

  // print taint func calls
  //// first parse the func arguments
  ////// preprocss the arguments string
  int inBracket = 0;
  for (size_t i = 0; i < arguments.length(); ++i) {
    if ( arguments.substr(i, 1).compare("{") == 0 )
      inBracket++;
    if ( arguments.substr(i, 1).compare("}") == 0 )
      inBracket--;
    if ( arguments.substr(i, 1).compare(",") == 0 && inBracket > 0 )
      arguments[i] = '`';
  }
  size_t previous, current;
  previous = -1;
  char delim = ',';
  std::vector<std::string> args;
  std::string arg;
  // collect all non-numerical args in vector args
  while( current != std::string::npos ) {
    current = arguments.find(delim, previous + 1);
    arg = arguments.substr(previous+1, current-1);
    if ( !isNum(arg) )
      args.push_back(arg);
    previous = current;
  }
  // Note: at most one 
  for (std::string arg: args) {
    std::regex pBracket("^\\{\\}$");
  }
  // TODO:
}


void add_func_taints_call_limited(std::string line, std::ofstream &output) {
  std::smatch m;
  if( !std::regex_match(line, m, pFunctionCall) )
    return;
  std::string blank = m.str(1);
  std::string returnArg = m.str(2);
  std::string funcName = m.str(3);
  std::string arguments = m.str(4);

  static const std::regex pArgComb("\\(.*\\)");
  if( !std::regex_search(line, m, pArgComb) ) {
    std::cout << "!! Error in parsing func args !!" << std::endl;
    abort();
  }
  std::string varArgs = m.str(0);
  std::string condArgForFunc = get_nth_var_in_list(varArgs, 3);

  std::string returnArgT = get_lhs_taint_list(returnArg, _t, output);
  std::string returnArgR = get_rhs_taint_list(returnArg, _r);
  //std::string returnArgX = get_rhs_taint_list(returnArg, _x);
  //std::string returnArgC = get_rhs_taint_list(returnArg, _c);

  std::string argTList = get_rhs_taint_list(varArgs, _t);

  std::vector<uint32_t> verVec;
  get_ver_vec(varArgs, verVec, output);
  std::string argRList = get_lhs_ver_taint_list(varArgs, _r, output, verVec);
  //std::string argXList = get_lhs_ver_taint_list(varArgs, _x, output, verVec);
  //std::string argCList = get_lhs_ver_taint_list(varArgs, _c, output, verVec);

  output << blank + "assign " + returnArgT + " = " + funcName + _t+"(" + argTList + ", " + condArgForFunc + ");" << std::endl;
  output << blank + "assign { " + argRList + " } = " + funcName + _r+"(" + returnArgR + ", " + condArgForFunc + ");" << std::endl;
  //output << blank + "assign { " + argXList + " } = " + funcName + _x+"(" + returnArgX + ", " + condArgForFunc + ");" << std::endl;
  //output << blank + "assign { " + argCList + " } = " + funcName + _c+"(" + returnArgC + ", " + condArgForFunc + ");" << std::endl;
}


void extend_module_instantiation(std::ifstream &input, 
                                 std::ofstream &output, 
                                 std::string moduleFirstLine, 
                                 std::map<std::string, std::vector<std::string>> &moduleInputsMap, 
                                 std::map<std::string, std::vector<std::string>> &moduleOutputsMap) {
  std::smatch m;
  if(!std::regex_match(moduleFirstLine, m, pInstanceBegin)) {
    toCout("Error in matching module definition!");
    abort();
  }
  std::string localModuleName = m.str(2);
  if(localModuleName.compare("hls_target_Loop_1_proc") == 0)
    toCoutVerb("find it!");

  std::string instanceName = m.str(3);
  if(instanceName.find("buffer") != std::string::npos) {
    toCoutVerb("Find it!");
  }
  if(g_mod2instMap.find(moduleName) != g_mod2instMap.end()) {
    g_mod2instMap[moduleName].emplace(instanceName, localModuleName);
  }
  else {
    g_mod2instMap.emplace(moduleName, std::unordered_map<std::string, std::string>{{instanceName, localModuleName}});
  }
  if( moduleInputsMap.find(localModuleName) == moduleInputsMap.end() ) {
    toCout("Error: IO ports of sub-modules not found!");
    abort();
  }
  std::string line;
  std::getline(input, line);
  // store the module port and their connected signals in the instantiation
  std::unordered_map<std::string, std::string> port2SignalMap;
  while(!std::regex_match(line, m, pInstanceEnd)) {
    if(!std::regex_match(line, m, pInstancePort)) {
      toCout("Error in matching module ports");
      abort();
    }
    port2SignalMap.emplace(m.str(2), m.str(3));
    std::getline(input, line);    
  }
 
  // declare new logics for _r,_c,_x taints of inputs
  // store the version for the taints of some signals, which are connected
  // to input ports
  std::unordered_map<std::string, std::vector<uint32_t>> input2SignalVerMap;
  std::unordered_map<std::string, std::vector<bool>> inputSignalIsNewMap;
  
  for(std::string input: moduleInputsMap[localModuleName]) {
    if(input.compare(g_recentClk) == 0 
       || input.compare("rst_zy") == 0 
       || input.compare("INSTR_IN_ZY") == 0 
       || input.compare(TAINT_RST) == 0 
       || input.compare(ASSERT_PROTECT) == 0 
       || input.compare(END_SIG) == 0)
      continue;
    if( port2SignalMap.find(input) == port2SignalMap.end() ) {
      LOG(INFO) << "input port of "+localModuleName+" is not connected: "+input;      
      continue;
    }
    std::string signalAndSliceList = port2SignalMap[input];
    if(signalAndSliceList.empty())
      continue;
    std::vector<std::string> signalAndSliceVec;
    parse_var_list(signalAndSliceList, signalAndSliceVec);

    std::vector<uint32_t> signalVerVec;
    std::vector<bool> isNewVec;
    get_ver_vec(signalAndSliceVec, signalVerVec, isNewVec, output);
    assert(signalVerVec.size() == isNewVec.size());

    input2SignalVerMap.emplace(input, signalVerVec);
    inputSignalIsNewMap.emplace(input, isNewVec);

    uint32_t i = 0;
    if(g_enable_taint)
      for(std::string signalAndSlice : signalAndSliceVec) {
        if(isNum(signalAndSlice)) {
          i++;
          continue;
        }
        std::string signal, signalSlice;
        split_slice(signalAndSlice, signal, signalSlice);
        auto signalIdxPair = varWidth.get_idx_pair(signal, "extend_module_instantiation:get_idx_pair");
        std::string signalHighIdx = toStr(signalIdxPair.first);
        std::string signalLowIdx = toStr(signalIdxPair.second);
        if(isNewVec[i]) {
          output << "  logic [" + signalHighIdx + ":" + signalLowIdx + "] " + signal + _r + toStr(signalVerVec[i++])   + " ;" << std::endl;
        }
      }
  }

  // declare new logic for _r_flag
  //for(std::string reg_r_flag: flagOutputs) {
  //  output << "  logic \\" + instanceName + "_" + reg_r_flag + " ;" << std::endl;
  //}

  // printed extended module instantiation
  std::vector<std::string> toAssignZero;
  output << "// module: "+localModuleName << std::endl;
  output << moduleFirstLine << std::endl;
  std::string newLogic;
  std::vector<std::string> newLogicVec;
  for(std::string inPort: moduleInputsMap[localModuleName]) {
    if(inPort.compare("rst_zy") == 0) {
      output << "    .rst_zy(rst_zy)," << std::endl;
      continue;
    }
    if(inPort.compare("INSTR_IN_ZY") == 0) {
      output << "    .INSTR_IN_ZY(INSTR_IN_ZY)," << std::endl;
      continue;
    }
    if(inPort.compare(TAINT_RST) == 0) {
      output << "    ."+TAINT_RST+"("+TAINT_RST+")," << std::endl;
      continue;
    }
    if(inPort.compare(END_SIG) == 0) {
      output << "    ."+END_SIG+"("+END_SIG+")," << std::endl;
      continue;
    }
    if(inPort.compare(ASSERT_PROTECT) == 0) {
      output << "    ."+ASSERT_PROTECT+"("+ASSERT_PROTECT+")," << std::endl;
      continue;
    }
    if(inPort.compare(g_recentClk) == 0 
        || g_clk_set.find(inPort) != g_clk_set.end() 
        || port2SignalMap.find(inPort) == port2SignalMap.end())
      continue;
    if(g_enable_taint) {
      if( !port2SignalMap[inPort].empty() ) {
        std::vector<uint32_t> localVerVec = input2SignalVerMap[inPort];
        output << "    ." + inPort + _t + " ( " 
                  + get_rhs_taint_list(port2SignalMap[inPort], _t) + " )," << std::endl;
        output << "    ." + inPort + _r + " ( " 
                  + get_lhs_ver_taint_list(port2SignalMap[inPort], _r, newLogic, localVerVec) + " )," 
                  << std::endl;
        newLogicVec.push_back(newLogic) ; 
        newLogicVec.push_back(newLogic) ;    
        newLogicVec.push_back(newLogic);
        // _sig
        if(!g_use_value_change) {
          std::string varAndSlice = port2SignalMap[inPort];
          std::string varConnect;
          if(!isNum(varAndSlice)) {
            std::vector<std::string> varVec;
            parse_var_list(varAndSlice, varVec);
            if(varVec.size() > 1)
              varConnect = "0";
            else
              varConnect = get_rhs_taint_list(varAndSlice, _sig, true);
          }
          else {
            if( !std::regex_match(varAndSlice, m, pNum)) {
              std::cout << "!! Error in matching number !!" << std::endl;
            }
            std::string numWidth = toStr(g_sig_width);
            varConnect = numWidth + "'h0";
          }
          output << "    ." + inPort + _sig + " ( " + varConnect + " )," << std::endl;    
        }
      }
      else {  // if the connected signal is empty
        output << "    ." + inPort + _t + " (0)," << std::endl; 
        output << "    ." + inPort + _r + " ()," << std::endl; 
        //output << "    ." + inPort + _x + " ()," << std::endl; 
        //output << "    ." + inPort + _c + " ()," << std::endl;
        if(!g_use_value_change)
        output << "    ." + inPort + _sig + " (0)," << std::endl;
      }
    }
  }
  if(g_enable_taint) {
    for(std::string outPort: moduleOutputsMap[localModuleName]) {
      if(port2SignalMap.find(outPort) == port2SignalMap.end()) {
        LOG(INFO) << "output port of "+localModuleName+" is floating: "+outPort;
        continue;
      }
      if( !port2SignalMap[outPort].empty() ) {
        output << "    ." + outPort + _t + " ( " + get_lhs_taint_list(port2SignalMap[outPort], _t, newLogic) + " )," << std::endl;
        newLogicVec.push_back(newLogic);      
        output << "    ." + outPort + _r + "0 ( " + get_rhs_taint_list(port2SignalMap[outPort], _r) + " )," << std::endl; 
        //output << "    ." + outPort + _x + "0 ( " + get_rhs_taint_list(port2SignalMap[outPort], _x) + " )," << std::endl; 
        //output << "    ." + outPort + _c + "0 ( " + get_rhs_taint_list(port2SignalMap[outPort], _c) + " )," << std::endl;
        // _sig
        if(!g_use_value_change) {
          std::string varAndSlice = port2SignalMap[outPort];
          std::string varConnect;
          if(!isNum(varAndSlice)) {
            std::vector<std::string> varVec;
            parse_var_list(varAndSlice, varVec);
            if(varVec.size() > 1) {
              varConnect = "";
              toAssignZero.push_back(get_rhs_taint_list(varAndSlice, _sig, true));
            }
            else
              varConnect = get_rhs_taint_list(varAndSlice, _sig, true);
          }
          else {
            if( !std::regex_match(varAndSlice, m, pNum )) {
              std::cout << "!! Error in matching number !!" << std::endl;
            }
            std::string numWidth = toStr(g_sig_width);
            varConnect = numWidth + "'h0";
          }
          output << "    ." + outPort + _sig + " ( " + varConnect + " )," << std::endl;
        }
      }
      else {
        output << "    ." + outPort + _t + " ()," << std::endl; 
        output << "    ." + outPort + _r + "0 (0)," << std::endl; 
        //output << "    ." + outPort + _x + "0 ()," << std::endl; 
        //output << "    ." + outPort + _c + "0 ()," << std::endl;
        if(!g_use_value_change)
        output << "    ." + outPort + _sig + " ()," << std::endl;
      }
    }
  }
  // TODO: we cannot leave these _r_flags in the sub-modules,
  //for(std::string reg_r_flag: flagOutputs) {
  //  output << "    ." + reg_r_flag + " ( \\" + instanceName + "_" + reg_r_flag + " )," << std::endl;
  //  flagOutputs.push_back( "\\"+instanceName+"_"+reg_r_flag );
  //}
  // original ports
  std::string lineToPrint;
  int i = 0;
  for(auto it = port2SignalMap.begin(); it != port2SignalMap.end(); ++it) {
    if(i++ > 0) output << lineToPrint << std::endl;
    lineToPrint = "    ." + it->first + " ( " + it->second + " ),";
  }
  lineToPrint.pop_back();
  output << lineToPrint << std::endl;
  output << "  );" << std::endl;

  // assign unconnected signatures
  for(std::string unwiredSig: toAssignZero)
    output << "  assign "+unwiredSig + " = 0 ;" << std::endl;

  for(std::string oneNewLogic: newLogicVec)
    if(!oneNewLogic.empty())
      output << oneNewLogic << std::endl;

  for (auto it = input2SignalVerMap.begin(); it != input2SignalVerMap.end(); ++it) {
    std::string signalList = port2SignalMap[it->first];
    std::vector<bool> localIsNewVec = inputSignalIsNewMap[it->first];    
    std::vector<uint32_t> localVerVec = it->second;
    std::vector<std::string> signalVec;
    parse_var_list(signalList, signalVec);
    uint32_t i = 0;
    for(auto localSignalAndSlice : signalVec) {
      std::string localSignal, localSignalSlice;
      split_slice(localSignalAndSlice, localSignal, localSignalSlice);
      auto boundPair = varWidth.get_idx_pair(localSignal, "extend_module_instantiation:2");
    }
  }
}


/* if a basic operator contains concatenated input, 
 * declare a new variable representing the concatenated input*/
// if a long number(>32bit) is found, split it if g_split_long_num is true
bool extract_concat(std::string line, 
                    std::ofstream &output, 
                    std::string &returnedStmt, 
                    std::string &addedVarDeclaration, 
                    std::string &addedVarAssign, 
                    bool isFuncCall) {
  if(line.find("alu_out") != std::string::npos) {
    toCoutVerb("find it");
  }
  std::string retStr = "";
  std::smatch m;
  int blankNo = line.find('a', 0);  
  static const std::regex pAssign("assign ");
  static const std::regex pBraces(to_re("\\{ ((?:NAME(?:\\s)?, )+NAME) \\}"));
  static const std::regex pSlice("\\[(\\d+)(:)?(\\d+)?\\]");

  // check if dest is {}
  uint32_t openBracePos = line.find("{");
  uint32_t equalPos = line.find("=");
  bool destIsBrace = openBracePos < equalPos;
  uint32_t braceIdx = 0;

  std::regex_token_iterator<std::string::iterator> rend;
  std::regex_token_iterator<std::string::iterator> it(line.begin(), line.end(), pBraces, 1);

  std::string varList;
  std::string newLine;
  std::vector<std::string> allVarList;
  std::queue<std::string> newVarQueue;
  // if isNonblockConcat, the declaration of addedVar is after the nonblock stmt
  bool isNonblockConcat = std::regex_match(line, m, pNonblockConcat);
  if ( (line.find("assign") != std::string::npos
       //&& !std::regex_match(line, m, pSrcConcat)
       && !is_srcConcat(line)
       && !is_destConcat(line)
       && !is_srcDestConcat(line)
       && std::regex_search(line, m, pBraces))
       || std::regex_match(line, m, pNonblockConcat) ) { // also extract from nonblockconcat
    // iterate over all matches
    while( it != rend ) {
      varList = *it++;
      if(g_split_long_num)
        varList = split_long_bit_vec(varList);

      allVarList.push_back(varList);
      int localIdxNum = NEW_FANGYUAN++;
      std::string localIdx = std::to_string(localIdxNum);
      uint32_t totalWidth = 0;
      std::vector<std::string> varVec;
      parse_var_list(varList, varVec);
      for(std::string var: varVec) {
        uint32_t localWidth = get_var_slice_width(var);
        if(localWidth == 0) {
          toCout("!! 0 width found for: " + var + ", in line: "+line);
          abort();
        }
        totalWidth += localWidth;
      }
      if(totalWidth > 4294967290) {
        std::cout << "!! Error in getting total width for this line:" << std::endl;
        std::cout << line << std::endl;
        abort();
      }
      bool insertDone = varWidth.var_width_insert("addedVar"+localIdx, totalWidth-1, 0);
      if (!insertDone) {
        std::cout << "insert failed for this line:" + line << std::endl;
        std::cout << "m.str(2):" + m.str(2) << std::endl;
        std::cout << "m.str(3):" + m.str(3) << std::endl;
      }
      if(!isNonblockConcat) {
        output << std::string(blankNo, ' ') + "wire [" + toStr(totalWidth-1) + ":0] addedVar" + localIdx + ";" << std::endl;        
        if(braceIdx == 0 && destIsBrace) // deal with "dest is brace" situation
          output << std::string(blankNo, ' ') + "assign { " + varList + " } = addedVar" + localIdx + ";" << std::endl;
        else
          output << std::string(blankNo, ' ') + "assign addedVar" + localIdx + " = { " + varList + " };" << std::endl;
      }
      else {
        addedVarDeclaration = "  wire [" + toStr(totalWidth-1) + ":0] addedVar" + localIdx + ";"; 
        addedVarAssign      = "  assign addedVar" + localIdx + " = { " + varList + " };";
      }
      newVarQueue.push("addedVar"+localIdx);
      braceIdx++;
    }
    char openBrace = '{';
    char closeBrace = '}';
    int openBracePos, closeBracePos = -1;
    /* if state=0, searching for openBrace
     * if state=1, searching for closeBrace */
    int state = 0;
    std::string part;
    // Assumption: the last search must be for openBrace
    while( openBracePos != std::string::npos ) {
      if (state == 0) {
        openBracePos = line.find(openBrace, closeBracePos+1);
        part = line.substr(closeBracePos+1, openBracePos - closeBracePos - 1);
        state = 1;
      }
      else if (state == 1) {
        closeBracePos = line.find(closeBrace, openBracePos+1);
        part = line.substr(openBracePos, closeBracePos - openBracePos + 1);
        state = 0;
      }
      else {
        toCout("Error!");
        abort();
      }
      if(state == 1) {// just find openBrace
        if(!isFuncCall)
          output << part;
        else 
          returnedStmt += part;
      }
      else { // just find closeBrace
        auto newVar = newVarQueue.front();
        if(!isFuncCall)
          output << newVar;
        else
          returnedStmt += newVar;
        newVarQueue.pop();
      }
    }
    return false;
  } // end of if
  return true; // true means no concatenation
}


void gen_assert_property(std::ofstream &output) {
  static const std::regex pRFlag("(\\S*)_r_flag");
  std::smatch m;
  g_mod2assertMap.emplace(moduleName, std::vector<std::string>{});
  std::string DOES_KEEP = "";
  std::string PREV_VAL_COMP = "";
  std::string USE_END_SIG = "";
  if(!g_write_assert) {
    for(std::string out: flagOutputs) {
      if(std::regex_search(out, m, pRFlag)) {
        std::string var = m.str(1);        
        if(var == "cpu_state") {
          toCoutVerb("Find it!");
        }
        g_mod2assertMap[moduleName].push_back(var);

        std::string rstVal; 
        if(g_use_vcd_parser)
          rstVal = g_rstValMap[moduleName][var];
        if(rstVal.empty()) rstVal = "0";

        if(g_use_does_keep) DOES_KEEP = " || ( " + var + "_DOES_KEEP == 0 )";
        g_use_end_sig = g_use_end_sig && !g_use_jasper;
        if(g_use_end_sig) USE_END_SIG = " || " + END_SIG;
        if(g_check_invariance == CheckTwoVal) 
          PREV_VAL_COMP = " || " + ASSERT_PROTECT + " || ( "  + var + "_PREV_VAL1 == " + var + "_PREV_VAL2 )";
        if(g_check_invariance == CheckOneVal) PREV_VAL_COMP = " || ( " + var + "_PREV_VAL1 == " + rstVal + " )";

        std::string assertion;

        if(!g_enable_taint && g_check_invariance == CheckRst) {
          assertion = "(!INSTR_IN_ZY) || " + var + " == " + rstVal;
        }
        else if (!g_enable_taint) {
          assertion = "(!INSTR_IN_ZY) " + PREV_VAL_COMP;
        }
        else if(!isMem(var)) 
          assertion = "( " + out + " == 0 ) " + PREV_VAL_COMP + USE_END_SIG + DOES_KEEP;
        else
          assertion = "( " + out + " == 0 )" + USE_END_SIG + DOES_KEEP;

        if(g_use_jasper) {
          if(g_modChangedRegs[moduleName].find(var) == g_modChangedRegs[moduleName].end())
            output << "  assert property( " + assertion + " );" << std::endl;
        }
        else {
          output << "  wire zy_assert"+toStr(g_assert_num) + " = " + assertion + " ;" << std::endl;
          output << "  assert "+var+"_asst (" << std::endl;
          output << "    .clk( " + g_recentClk + " )," << std::endl;
          output << "    .test( zy_assert"+toStr(g_assert_num++) + " )" << std::endl;
          output << "  );" << std::endl;
        }


        //else if(!isMem(var) && g_double_assert) {
        //  if(g_use_vcd_parser)
        //    output << "  assert property( " + out + " == 0 || " 
        //              + var + "_PREV_VAL1 == " + rstVal + " );" << std::endl;

        //  else if(g_set_rflag_if_not_rst_val)
        //  //else if(true)
        //    output << "  assert property( " + out + " == 0 );" << std::endl;
        //  else
        //    output << "  assert property( " + out + " == 0 || " 
        //              + var + "_PREV_VAL1 == " + var + "_PREV_VAL2 );" << std::endl;
        //}
        //else if(g_use_end_sig)
        //  output << "  assert property( " + out + " == 0 || " + END_SIG + DOES_KEEP + ");" << std::endl;          
        //else
        //  output << "  assert property( " + out + " == 0" + DOES_KEEP + ");" << std::endl;
      }
    }
  }
  
  // generate properties for checking written ASV by each instruction
  if(g_write_assert && isTop) {
    std::ifstream input(g_path+"/ILA/asv.txt");
    if(!input.is_open()) {
      toCout("Error: asv.txt file is not opened: "+g_path+"/ILA/asv.txt");
      abort();
    }
    std::string asv;
    while(std::getline(input, asv)) {
      output << "  assert property( " + asv + "_t_flag == 0 );" << std::endl;
    }
  }
}


void gen_reg_output(std::string fileName) {
  std::ofstream output(fileName);
  for( std::string out: moduleOutputs ) {
    if( isReg(out) && !isRFlag(out) ) 
      output << out << std::endl;
  }
}


void gen_wire_output(std::string fileName) {
  std::ofstream output(fileName);
  for( std::string out: moduleOutputs ) {
    if( !isReg(out) && !isRFlag(out) ) 
      output << out << std::endl;
  }
}


void collect_ite_dest(const std::string &line) {
  std::smatch m;
  if ( !std::regex_match(line, m, pIte) )
    return;
  std::string destAndSlice = m.str(2);
  std::string dest, destSlice;
  split_slice(destAndSlice, dest, destSlice);
  g_iteDest.insert(dest);
}


void collect_case_dest(const std::string &line) {
  std::smatch m;
  if ( !std::regex_match(line, m, pBlock) )
    return;
  std::string destAndSlice = m.str(2);
  std::string dest, destSlice;
  split_slice(destAndSlice, dest, destSlice);
  g_iteDest.insert(dest);
}


void assert_reg_map_gen() {
  std::ofstream output(g_path+"/assert_reg_map.txt");
  map_gen(g_topModule, g_topModule, output);
}


void map_gen(std::string moduleName, std::string instanceName, std::ofstream &output) {
  uint32_t i = 1;
  if(g_mod2assertMap.find(moduleName) != g_mod2assertMap.end()) {
    for(auto it = g_mod2assertMap[moduleName].begin(); 
          it != g_mod2assertMap[moduleName].end(); it++) {
      std::string reg = *it;
      if(g_modChangedRegs[moduleName].find(reg) != g_modChangedRegs[moduleName].end())
        continue;
      output << instanceName+"._assert_"+toStr(i++)+" : "+reg << std::endl;
    }
  }
  if(g_mod2instMap.find(moduleName) != g_mod2instMap.end()) {
    for(auto it = g_mod2instMap[moduleName].begin(); 
          it != g_mod2instMap[moduleName].end(); it++) {
      map_gen(it->second, instanceName+"."+it->first, output);
    }
  }
}


void read_changed_regs(std::string fileName, 
                       StrSet_t & changedRegSet) {
  std::string modName;
  std::string line;
  std::ifstream input(fileName);
  while(std::getline(input, line)) {
    if(line.empty() || line.substr(0, 9) != "ASSERTION") continue;
    else if(line.find(".") == std::string::npos) continue;
    else {
      size_t pos = line.find_last_of(".");
      // cutting the beginning 26 chars "ASSERTION FAILED in tb.u0."
      std::string changedReg = line.substr(26);
      uint32_t size = changedReg.size();
      changedReg = changedReg.substr(0, size-5);
      changedRegSet.insert(changedReg);
    }
  }
}


// according to g_instance2moduleMap and g_mod2ChangedRegMap,
// determine for each module which regs need to be checked
// by jaspergold
// result is stored in g_modChangedRegs
void determine_regs_to_check() {
  for(std::string path2Reg : g_changedRegVec) {
    assert(path2Reg.substr(0, 2) != "\\");
    StrVec_t pathVec;
    split_by(path2Reg, ".", pathVec);
    std::reverse(pathVec.begin(), pathVec.end());
    find_reg(g_topModule, pathVec);
  }
  if(!g_modChangedRegs.empty())
    toCout("Get changed regs!!");
}


void find_reg(std::string parentModName, StrVec_t &path2Reg) {
  if(path2Reg.size() == 1) {
    if(g_modChangedRegs.find(parentModName) == g_modChangedRegs.end()) {
      g_modChangedRegs.emplace(parentModName, StrSet_t{path2Reg.back()});
    }
    else
      g_modChangedRegs[parentModName].insert(path2Reg.back());
  }
  else {
    std::string insName = path2Reg.back();
    assert(g_instance2moduleMap[parentModName].find(insName) 
             != g_instance2moduleMap[parentModName].end() );
    std::string childModName = g_instance2moduleMap[parentModName][insName];
    path2Reg.pop_back();
    find_reg(childModName, path2Reg);
  }
}


// check if g_modChangedRegs is a subset of moduleTrueRegs
void check_changed_regs(std::string modName) {
  for(std::string reg: g_modChangedRegs[modName]) {
    if(std::find(moduleTrueRegs.begin(), moduleTrueRegs.end(), reg) == moduleTrueRegs.end()
       && moduleMems.find(reg) == moduleMems.end()
       && std::find(moduleTrueRegs.begin(), moduleTrueRegs.end(), "\\"+reg) == moduleTrueRegs.end() ) {
      toCout("Error: changed reg is not truely reg or mem, reg: "+reg+", module: "+modName);
      abort();
    }
  }
}


void convert_nb_if_to_ite(std::ifstream &input, 
                          std::ofstream &output, 
                          std::string line) {
  std::string line2;
  std::getline(input, line2);
  std::smatch m;
  if(!std::regex_match(line2, m, pNonblockIf)) {
    output << line << std::endl;  
    output << line2 << std::endl;
    return;
  }
  else { // if line2 is nb-if
    std::string ifCondAndSlice = m.str(2);
    std::string destAndSlice = m.str(3);
    std::string ifSrcAndSlice = m.str(4);
    uint32_t width = get_var_slice_width(destAndSlice);
    auto currentPos = input.tellg();
    std::string line3;
    std::getline(input, line3);
    std::string newVarIdx = toStr(NEW_FANGYUAN++);
    output << "  wire ["+toStr(width-1)+":0] NewNBIte"+newVarIdx+";" << std::endl;
    varWidth.var_width_insert("NewNBIte"+newVarIdx, width-1, 0);
    if(std::regex_match(line3, m, pNonblockElse)) {
      std::string elseSrcAndSlice = m.str(3);
      if(ifCondAndSlice.front() != '!')
        output << "  assign NewNBIte"+newVarIdx+" = "+ifCondAndSlice+" ? "+ifSrcAndSlice+" : "+elseSrcAndSlice+" ;" << std::endl;
      else
        output << "  assign NewNBIte"+newVarIdx+" = "+ifCondAndSlice.substr(1)+" ? "+elseSrcAndSlice+" : "+ifSrcAndSlice+" ;" << std::endl;
    }
    else if(std::regex_match(line3, m, pNBElseIf)) {
      std::string elseIfCondAndSlice = m.str(2);      
      std::string elseSrcAndSlice = m.str(4);
      std::string newVarIdx2 = toStr(NEW_FANGYUAN++);
      output << "  wire ["+toStr(width-1)+":0] NewNBIte"+newVarIdx2+";" << std::endl;
      varWidth.var_width_insert("NewNBIte"+newVarIdx2, width-1, 0);
      if(elseIfCondAndSlice.front() != '!')
        output << "  assign NewNBIte"+newVarIdx2+" = "+elseIfCondAndSlice+" ? "+elseSrcAndSlice+" : "+destAndSlice+" ;" << std::endl;
      else
        output << "  assign NewNBIte"+newVarIdx2+" = "+elseIfCondAndSlice.substr(1)+" ? "+destAndSlice+" : "+elseSrcAndSlice+" ;" << std::endl;

      if(ifCondAndSlice.front() != '!')
        output << "  assign NewNBIte"+newVarIdx+" = "+ifCondAndSlice+" ? "+ifSrcAndSlice+" : NewNBIte"+newVarIdx2+" ;" << std::endl;
      else
        output << "  assign NewNBIte"+newVarIdx+" = "+ifCondAndSlice.substr(1)+" ? NewNBIte"+newVarIdx2+" : "+ifSrcAndSlice+" ;" << std::endl;
    }
    else { // if line3 is neither
      if(ifCondAndSlice.front() != '!')
        output << "  assign NewNBIte"+newVarIdx+" = "+ifCondAndSlice+" ? "+ifSrcAndSlice+" : "+destAndSlice+" ;" << std::endl;
      else
        output << "  assign NewNBIte"+newVarIdx+" = "+ifCondAndSlice.substr(1)+" ? "+destAndSlice+" : "+ifSrcAndSlice+" ;" << std::endl;

      input.seekg(currentPos);
    }
    output << line << std::endl;
    output << "    "+destAndSlice+" <= NewNBIte"+newVarIdx+" ;" << std::endl;
    return;
  }
}
 

// 1. separate the original file into multiple files, each containing one module
// 2. analyze for each module, which sub-modules it uses
// Return name of top module
std::string separate_modules(std::string fileName, 
                             std::vector<std::string> &modules,
                             std::map<std::string, std::vector<std::string>> &childModules,
                             uint32_t &totalRegCnt,
                             std::unordered_map<std::string,
                                                Str2StrUmap_t>& instance2moduleMap,
                             bool getIO,
                             Str2StrVecMap_t &moduleInputsMap,
                             Str2StrVecMap_t &moduleOutputsMap) {
  toCout("... Begin separating modules!");
  totalRegCnt = 0;
  std::ifstream input(fileName);
  std::string line;
  std::smatch m;
  std::ofstream output;
  std::string topModule;
  std::string moduleName;
  std::string path = extract_path(fileName);
  std::set<std::string> regSet;
  std::stack<std::string> moduleNameStk;
  bool inModule = false;

  if(getIO) {
    assert(moduleInputsMap.empty());
    assert(moduleOutputsMap.empty());
  }


  while(std::getline(input, line)) {
    //toCout(line);
    if(line.find("<=") != std::string::npos) {
      std::string regAndSlice = "";        
      if(std::regex_match(line, m, pNonblock))
        regAndSlice = m.str(2);
      else if(std::regex_match(line, m, pNonblockConcat))
        regAndSlice = m.str(2);
      else if( std::regex_match(line, m, pNonblockIf))
        regAndSlice = m.str(3);
      else if( std::regex_match(line, m, pNonblockIf2))
        regAndSlice = m.str(3);

      if(!regAndSlice.empty()) {
        std::string reg, regSlice;
        split_slice(regAndSlice, reg, regSlice);
        if(regSet.find(reg) == regSet.end()) {
          regSet.insert(reg);
          totalRegCnt++;
        }
      }
    }

    if(line.find("module") != std::string::npos && line != "endmodule") {
      if(std::regex_match(line, m, pModule)) {
        // assume the first module encountered are top module
        inModule = true;
        moduleName = m.str(2);
        moduleNameStk.push(moduleName);
        if(moduleInputsMap.find(moduleName) == moduleInputsMap.end())
          moduleInputsMap.emplace(moduleName, std::vector<std::string>{});
        if(moduleOutputsMap.find(moduleName) == moduleOutputsMap.end())
          moduleOutputsMap.emplace(moduleName, std::vector<std::string>{});
        modules.push_back(moduleName);
        assert(!output.is_open());
        output.open(path+"/"+moduleName+"_NEW.v");
      }
      else {
        toCout("Warning: module name not matched: "+line);
        //abort();
      }
    }

    if(std::regex_match(line, m, pEndmodule)) {
      output << line << std::endl;
      if(!inModule) {
        toCout("Error: found endmodule when not in module definition, last module name: "+moduleName);
        abort();
      }
      moduleNameStk.pop();
      inModule = false;
      output.close();
    }


    // get IO for each module
    if(getIO) {
      std::smatch m;
      if(std::regex_match(line, m, pInput)) {
        std::string inputName = m.str(3);
        moduleInputsMap[moduleNameStk.top()].push_back(inputName);
      }
      else if(std::regex_match(line, m, pOutput)) {
        std::string outputName = m.str(3);
        moduleInputsMap[moduleNameStk.top()].push_back(outputName);
      }
    }


    // analyze module dependency
    if(inModule && std::regex_match(line, m, pInstanceBegin)) {
      std::string localModuleName = m.str(2);
      std::string instanceName = m.str(3);
      if(instance2moduleMap.find(moduleName) == instance2moduleMap.end()) {
        instance2moduleMap.emplace(moduleName, 
                                     std::unordered_map<std::string, 
                                                        std::string>{{instanceName, localModuleName}});
      }
      else {
        if(instance2moduleMap[moduleName].find(instanceName) != instance2moduleMap[moduleName].end()) {
          if(instance2moduleMap[moduleName][instanceName] != localModuleName) {
            toCout("Error: instance: "+instanceName+" matches multiple modules:"+localModuleName+", "+instance2moduleMap[moduleName][instanceName]);
            abort();
          }
        }
        else
          instance2moduleMap[moduleName].emplace(instanceName, localModuleName);
      }

      if(childModules.find(moduleName) == childModules.end())
        childModules.emplace(moduleName, std::vector<std::string>{m.str(2)});
      else
        childModules[moduleName].push_back(m.str(2));
    }

    if(!inModule) continue;
    else {
      output << line << std::endl;
    }
  } // end of getline while loop

  // find out the top module
  std::unordered_set<std::string> childSet;
  for(auto it = childModules.begin(); it != childModules.end(); ++it) {
    for(std::string singleChildModule: it->second) {
      childSet.emplace(singleChildModule);
    }
  }
  for(std::string singleModule: modules) {
    if(childSet.find(singleModule) == childSet.end()) {
      if(!topModule.empty()) {
        toCout("Two top modules found: "+topModule+" & "+singleModule);
        abort();
      }
      else {
        topModule = singleModule;
      }
    }
  }

  toCout("... Finished separating modules!");  
  return topModule;
}


std::string separate_modules(std::string fileName, 
                             std::vector<std::string> &modules,
                             std::map<std::string, std::vector<std::string>> &childModules,
                             uint32_t &totalRegCnt,
                             std::unordered_map<std::string,
                                                Str2StrUmap_t>& instance2moduleMap) {
  Str2StrVecMap_t _dummy_map1;
  Str2StrVecMap_t _dummy_map2;
  return separate_modules(fileName, 
                   modules,
                   childModules,
                   totalRegCnt,
                   instance2moduleMap,
                   false,
                   _dummy_map1,
                   _dummy_map2);
}


int taint_gen(std::string fileName, 
              uint32_t stage, 
              bool isTopIn, 
              std::map<std::string, std::vector<std::string>> &moduleInputsMap, 
              std::map<std::string, std::vector<std::string>> &moduleOutputsMap, 
              std::map<std::string, std::vector<std::string>> &moduleRFlagsMap, 
              uint32_t totalRegCnt, 
              uint32_t &nextSig, 
              bool doProcessPathInfo) {
  toCout("*** Begin add taint for module: "+fileName);  
  if (stage <= 0) {
    toCout("Clear Global data!");
    clean_global_data(totalRegCnt, nextSig);
    std::cout << "Begin cleaning!" << std::endl; //0
    std::string path = extract_path(fileName);
    clean_file(fileName, false);
  }
  if (stage <= 1) {
    std::cout << "Remove functions!" << std::endl;
    remove_functions(fileName);
  }
  if (stage <= 1) {
    std::cout << "Analyze register's path!" << std::endl;
    analyze_reg_path(fileName);
  }
  //if (stage <= 2) {  
  //  std::cout << "Begin fill update!" << std::endl; //2
  //  fill_update(fileName + ".clean");
  //}
  if (stage <= 2 && doProcessPathInfo) {  
    std::cout << "Process pass info!" << std::endl; //2
    process_pass_info(fileName + ".clean");
  }
  if (stage <= 3) {
    isTop = isTopIn;
    std::cout << "Begin add file taints!" << std::endl; //3
    add_file_taints(fileName + ".clean", moduleInputsMap, moduleOutputsMap, moduleRFlagsMap);
  }
  if (stage <= 4) {  
    std::cout << "Begin merge taints!" << std::endl; //4
    merge_taints(fileName + ".clean.tainted");
  }
  if (stage <= 5) {  
    std::cout << "Begin add module name!" << std::endl; //6
    add_module_name(fileName + ".clean.tainted", moduleInputsMap, moduleOutputsMap, moduleRFlagsMap, isTopIn);
  }
  if (stage <= 6 && isTopIn) {
    std::cout << "Generate info about outputs!" << std::endl; //7
    gen_reg_output(fileName + ".reg_output");
    gen_wire_output(fileName + ".wire_output");
  }
  nextSig = g_next_sig;
  check_changed_regs(moduleName);
  print_reg_list(moduleName);
  toCout("*** Finish add taint for module: "+fileName);  
  return 0;
}

} // end of namespace taintGen
