#include "../src/read_instr.h"
#include "../src/helper.h"
#include "../src/util.h"
//#include "../../../taint_method/src/global_data.h"
#include "../src/global_data_struct.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <map>
#define toStr(a) std::to_string(a)

using namespace funcExtract;
using namespace taintGen;


std::ofstream g_output;
std::regex pX("(\\d+)'[b|h][x|X]$");

enum DESIGN{AES, PICO, URV, VTA, BI, OTHER, NON_RANDOM};
enum DESIGN g_design = OTHER;

void to_file(std::string line) {
  g_output << line << std::endl;
}


std::string materialize_num(std::string val) {
  if(val == "11'hx")
    toCoutVerb("Find it!");
  assert(val.find("+") == std::string::npos);
  std::smatch m;

  if (val == "x") return val;

  if(!is_x(val)) {
    if(val.find("x") != std::string::npos) {
      toCout("Error: x value is not correctly formatted: "+val);
      abort();
    }
    return val;
  }
  else {
    if(!std::regex_match(val, m, pX)) {
      toCout("Error: does not match pX");
      abort();
    }
    uint32_t width = std::stoi(m.str(1));
    uint32_t randVal;

    // FIXME: special treatment for 5'hx(no 1 and 2) because
    // the update function does not cover 1 and 2 currenly.
    // Remove the following code when the bug is fixed.
    if(width == 5) {
      randVal = rand() % 29;
      randVal += 3;
      std::string hexVal = dec2hex(randVal);
      return "5'h"+hexVal;
    }

    if(width == 32) randVal = rand();
    else if(width > 0) randVal = rand() % (2 << (width-1));
    else randVal = rand() % 2;
    std::string hexVal = dec2hex(randVal);
    if(width == 8 && hexVal.size() > 2) hexVal = hexVal.substr(0, 2);
    return toStr(width)+"'h"+hexVal;
  }
}


// convert 4'h1+4'h2 to { 4'h1, 4'h2 }
std::string replace_x(std::string val) {
  std::smatch m;
  std::regex pWidth("(\\d+)'(h|d|b)");
  if(val.find("+") == std::string::npos)
    return materialize_num(val);
  else {
    std::string ret;
    remove_two_end_space(val);
    std::vector<std::string> vec;
    split_by(val, "+", vec);
    // replace x with number value
    int zValue = -1;
    for(auto it = vec.begin(); it != vec.end(); it++) {
      if((*it).find("Z") != std::string::npos) {
        // ==================
        // allow adjacent variables to have relations:
        // Zp3 and Z: the former is the sum of 3 and the latter
        // Zm3 and Z: the former is the later subtracted by 3

        std::string num = *it;
        if(num.find("(") != std::string::npos) {
          auto pos = num.find("(");
          std::string formedWidth = num.substr(0, pos);
          std::string expr = num.substr(pos+1);
          expr.pop_back();
          if(!std::regex_match(formedWidth, m, pWidth)) {
            toCout("Error: the width does not match pWidth: "+formedWidth);
            abort();
          }
          uint32_t width = std::stoi(m.str(1));
          uint32_t base = exp2(width-1);
          if(g_design == VTA) base = 100;
          if(expr.find("p") != std::string::npos) {
            auto pos = expr.find("p");
            assert(pos == 1);
            uint32_t operand = std::stoi(expr.substr(pos+1));
            uint32_t newVal = rand() % (base-operand);
            zValue = newVal;
            newVal += operand;
            std::string hexVal = dec2hex(newVal);
            *it = toStr(width)+"'h"+hexVal;            
          }
          else if(num.find("m") != std::string::npos) {
            auto pos = expr.find("m");
            assert(pos == 1);
            uint32_t operand = std::stoi(expr.substr(pos+1));
            uint32_t newVal = rand() % (base-operand);
            zValue = newVal + operand;
            std::string hexVal = dec2hex(newVal);
            *it = toStr(width)+"'h"+hexVal;
          }
          else {
            toCout("Error: unexpected number format: "+num);
            abort();
          }
        }
        else { // if does not have braces
          std::regex pZ("(\\d+)'(h|d|b)Z");
          std::smatch m;
          std::string num = *it;
          if(!std::regex_match(num, m, pZ)) {
            toCout("Error: does not match pZ: "+num);
            abort();
          }
          std::string width = m.str(1);
          if(zValue == -1) {
            toCout("Error: zValue is not assigned!");
            abort();
          }
          std::string hexVal = dec2hex(zValue);
          *it = width+"'h"+hexVal;
        }
        continue;
      }

      if(!std::regex_match(*it, m, pX)) continue;
      uint32_t width = std::stoi(m.str(1));
      std::string hexVal;
      if(width != 5) {
        uint32_t base = exp2(width-1);
        uint32_t newVal = rand() % base;
        hexVal = dec2hex(newVal);
      }
      else if(g_design == PICO){
        uint32_t newVal = rand() % 4;
        switch(newVal) {
          case 0:
            hexVal = "0";
            break;
          case 1:
            hexVal = "1";
            break;
          case 2:
            hexVal = "2";
            break;
          case 3:
            hexVal = "1f";
            break;
        }
      }
      else if(g_design == URV) {
        uint32_t randVal = rand() % 29;
        randVal += 3;
        hexVal = dec2hex(randVal);
      }
      else if(g_design == BI) {
        uint32_t randVal = rand() % 31;
        randVal += 1;
        hexVal = dec2hex(randVal);
      }
      *it = toStr(width)+"'h"+hexVal;
    }
    ret = merge_with(vec, "+");
    return ret;
  }
}


void assign_instr_value(std::string var, std::string value, bool rand) {
  value = replace_x(value);
  to_file(""+var+" = "+value);
}


void make_instr(uint32_t instrIdx, bool constantEncod=true) {
   const struct InstrInfo_t& instrInfo = g_instrInfo[instrIdx];

  // FIXME: currently assume 
  // first assign instruction encodings
  to_file("// instr: "+instrInfo.name);
  uint32_t instrLen = instrInfo.instrEncoding.begin()->second.size();
  for(uint32_t i = 0; i < instrLen; i++) {
    to_file("("+toStr(i)+")");
    for(auto pair: instrInfo.instrEncoding) {
      std::string var = pair.first;
      //if( !instrInfo.dataIn.first.empty()
      //     && var == instrInfo.dataIn.first 
      //     && varIdxMap[var] == instrInfo.dataIn.second-1 ) {
      //  to_file(""+var+" = FROM_MEM");
      //}
      //else {
      if(constantEncod) {
        // Always use the first clock cycle's value
        assign_instr_value(var, pair.second[0], true);
      } else {
        assign_instr_value(var, pair.second[i], true);        
      }
      //}
    }
  }
  to_file("");
}


void make_rand_instrs(int instrNum, bool constantEncod=true) {
  for (int idx = 0; idx < instrNum; ++idx) {
    uint32_t instrIdx = rand() % g_instrInfo.size();
    make_instr(instrIdx, constantEncod);
  }
}


void gen_rand_dmem(int width, int num) {
  std::ofstream output(g_path+"/dmem.txt");
  int i = 0;
  if(width == 32) {
    while(i++ < num) {
      uint32_t randNum = rand();
      output << randNum << std::endl;
    }
  }
  else if (width == 16) {
    while(i++ < num) {
      unsigned short randNum = rand();
      output << randNum << std::endl;
    }
  }
  else {
    toCout("Error: unexpected mem width: "+toStr(width));
    abort();
  }
}


int main(int argc, char *argv[]) {

  std::string usageStr = std::string("usage: ")+argv[0]+ " [<path>] [<instr_num>] [<design_opt>] [-verbose]";

  g_path = ".";   // Default path is current dir
  g_verb = false;
  g_design = OTHER;

  bool userVerbose = false;
  bool userQuiet = false;

  int ndesopts = 0;

  int instrNum = -1;


  for (int n = 1; n < argc; ++n) {
    const char *arg = argv[n];

    if (n == 1 && arg[0] != '-') {
      // If the first arg does not begin with '-', it is assumed to be the path
      g_path = arg; 
    } else if (isdigit(arg[0])) {
      // Any numeric arg is the instruction count
      if (instrNum > 0) {
        toCout("Error: Instruction count specified more than once!");
        toCout(usageStr);
        exit(-1);
      }
      instrNum = std::stoi(arg);
    } else if (!strcmp(arg, "-verbose")) {
      userVerbose = true;
    } else if (!strcmp(arg, "-quiet")) {
      userQuiet = true;
    } else if (!strcmp(argv[n], "-aes")) {
      ndesopts++;
      g_design = AES;
    } else if (!strcmp(argv[n], "-pico")) {
      ndesopts++;
      g_design = PICO;
    } else if (!strcmp(argv[n], "-urv")) {
      ndesopts++;
      g_design = URV;
    } else if (!strcmp(argv[n], "-vta")) {
      ndesopts++;
      g_design = VTA;
    } else if (!strcmp(argv[n], "-bi")) {
      ndesopts++;
      g_design = BI;
    } else if (!strcmp(argv[n], "-other")) {
      ndesopts++;
      g_design = OTHER;
    } else if (!strcmp(argv[n], "-non_random")) {
      ndesopts++;
      g_design = NON_RANDOM;
    } else {
      toCout(std::string("Unknown option ")+argv[n]);
      exit(-1);
    }
  }

  // Can't give both -verbose and -quiet
  if (userVerbose && userQuiet) {
    toCout(usageStr);
    exit(-1);
  }

  if (ndesopts > 1) {
    toCout("Multiple design options specified!");
    toCout(usageStr);
    exit(-1);
  }

  if (g_design != NON_RANDOM && instrNum < 0) {
    toCout("Error: did not specify the number of instructions to be executed!");
    toCout(usageStr);
    exit(-1);
  }

  // Override any g_verb setting from the config file with any setting from the command line.
  if (userVerbose) g_verb = true;
  if (userQuiet) g_verb = false;

  read_in_instructions(g_path+"/instr.txt");
  toCout("Writing "+g_path+"/tb.txt");
  g_output.open(g_path+"/tb.txt", std::ios::out);
  srand(time(NULL));
  //srand(1);

  if(g_design == PICO) {
    make_rand_instrs(instrNum, true);
  }
  else if(g_design == AES) {
    // for AES, execute start instr. every two instructions
    bool doStart = false;
    int idx = 0;
    while(idx++ < instrNum) {
      toCout("Make instr: "+toStr(idx));
      if(doStart) {
        make_instr(0);
        doStart = false;
      }
      else {
        uint32_t instrIdx = rand() % g_instrInfo.size();
        make_instr(instrIdx);
        doStart = true;
      }
    }
  }
  else if(g_design == URV ||
          g_design == BI) {
    gen_rand_dmem(32, 64);
    make_rand_instrs(instrNum, true);
  }
  else if(g_design == VTA ||
          g_design == OTHER) {
    make_rand_instrs(instrNum, false);
  }
  else if(g_design == NON_RANDOM) {
    // Do all instructions once, in order.
    for (uint32_t idx = 0; idx < g_instrInfo.size(); ++idx) {
      make_instr(idx, false);
    }
  }
}


