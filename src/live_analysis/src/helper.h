#ifndef LIVE_ANALYSIS_HELPER_H
#define LIVE_ANALYSIS_HELPER_H

#include <string>
#include <sstream>
#include <bitset>
#include <iostream>
#include <regex>
#include "global_data.h"

namespace taintGen {

bool isNum(std::string name);

bool is_number(const std::string& s);

bool is_all_digits(const std::string& num);

bool isOutput(const std::string& var);

bool isInput(const std::string& var);

bool isReg(std::string var);

bool isOAReg(const std::string& var);

bool isWire(const std::string& var);

bool isMem(const std::string& var);

std::string to_re(const std::string& input);

std::string remove_bracket(const std::string& name);

uint32_t cut_pos(const std::string& name);

bool split_slice(const std::string& slicedName, std::string &name, std::string &slice);

uint32_t get_width(const std::string& slice);

uint32_t get_begin(const std::string& slice);

uint32_t get_end(const std::string& slice);

uint32_t find_version_num(const std::string& opAndSlice, bool &isNew, std::ofstream &output, bool forceNewVer=false);

void free_bits(const std::string& op, std::vector<std::string> &freeBitsVec);

bool check_bits(const std::string& op, std::string opSlice, const std::vector<bool> &bitVec);

void merge_bits(const std::string& op, std::string opSlice, std::vector<bool> &bitVec);

void parse_taintBits(const std::string& taintBits, bool &tExist, bool &rExist, bool &xExist, bool &cExist);

void collapse_bits(const std::string& varName, uint32_t bound1, uint32_t bound2, std::ofstream &output);

std::string extend(const std::string& in, uint32_t length);

void debug_line(const std::string& line);

void ground_wires(const std::string& wireName, std::pair<uint32_t, uint32_t> idxPair, std::string slice, std::string blank, std::ofstream &output);

void parse_var_list(std::string list, std::vector<std::string> &vec, bool noSlice = false);

std::string get_nth_var_in_list(std::string list, uint32_t idx);

uint32_t get_var_slice_width( std::string varSlice, VarWidth &varWdithIn = varWidth);

std::string get_rhs_taint_list(std::vector<std::string> &updateVec, std::string taint, bool noSlice=false);

std::string get_rhs_taint_list(std::string updateList, std::string taint, bool noSlice=false);

std::string insert_taint(const std::string& signalAndSlice, std::string taint, std::string ver="");

std::string get_lhs_ver_taint_list(std::vector<std::string> &updateVec, std::string taint, std::ofstream &output, std::vector<uint32_t> localVer);

std::string get_lhs_ver_taint_list(std::string list, std::string taint, std::ofstream &output, std::vector<uint32_t> localVer);

std::string get_lhs_ver_taint_list(std::vector<std::string> &updateVec, std::string taint, std::string &newLogic, std::vector<uint32_t> verVec);

std::string get_lhs_ver_taint_list(std::string list, std::string taint, std::string &newLogic, std::vector<uint32_t> localVer);

std::string get_lhs_taint_list(std::vector<std::string> &destVec, std::string taint, std::ofstream &output);

std::string get_lhs_taint_list(std::string destList, std::string taint, std::ofstream &output);

std::string get_lhs_taint_list(std::vector<std::string> &destVec, std::string taint, std::string &newLogic);

std::string get_lhs_taint_list(std::string destList, std::string taint, std::string &newLogic);

std::string get_lhs_taint_list_no_slice(std::vector<std::string> &destVec, std::string taint, std::ofstream &output);

std::string get_lhs_taint_list_no_slice(std::string destList, std::string taint, std::ofstream &output);

void get_ver_vec(std::vector<std::string> varVec, std::vector<uint32_t> &verVec, std::ofstream &output);

void get_ver_vec(std::string list, std::vector<uint32_t> &verVec, std::ofstream &output);

void get_ver_vec(std::vector<std::string> varVec, std::vector<uint32_t> &verVec, std::vector<bool> &isNewVec, std::ofstream &output);

int str2int(const std::string& str, std::string info);

void toCout(const std::string& line);

void toCoutVerb(const std::string& line);

bool isSingleBit(const std::string& slice);

std::string further_clean_line(std::string line);

std::string get_recent_rst();

std::string get_rst();

bool isRFlag(const std::string& var);

void parse_func_statements(std::vector<std::pair<std::string, std::string>> &caseAssignPairs, std::vector<std::string> &inputWidth, std::ifstream &input, bool goToEnd=false);

std::string parse_case_statements(std::vector<std::pair<std::string, std::string>> &caseAssignPairs, std::ifstream &input, bool returnBegin=false);

std::string pairVec2taintString( std::vector<std::pair<std::string, std::string>> &pairVec, std::string notIncluded, std::string taint, std::ofstream &output );

std::string max_num(uint32_t width);

std::string max_num(const std::string& widthStr);

uint32_t max_num_dec(uint32_t width);

std::string dec2bin(uint32_t inNum);

//
std::string get_bits(const std::string& inNum, uint32_t highIdx, uint32_t lowIdx);

std::string add_taint(std::vector<std::string> &freeBitsVec, std::string taint);

void assert_info(bool val, std::string info);

void merge_vec(std::vector<std::string> &srcVec, std::vector<std::string> &destVec);

bool is_neg_rst(const std::string& rst);

void printAndAbort(const std::string& in);

void checkCond(bool cond, std::string in);

std::string expand_slice(const std::string& slice);

std::string extract_path(const std::string& fullFileName);

uint32_t get_dest_ver(const std::string& destAndSlice);

bool is_srcConcat(const std::string &line);

bool is_destConcat(const std::string &line);

bool is_srcDestConcat(const std::string &line);

//
std::string extract_bin(const std::string& num, uint32_t highIdx, uint32_t lowIdx);

std::string hex2bin(const std::string& hexNum);

std::string split_long_bit_vec(const std::string& varList);

std::string remove_signed(const std::string& line);

std::string split_long_hex(const std::string& var, uint32_t width, std::string num, std::string strToConcat);

std::string split_long_bin(const std::string& var, uint32_t width, std::string num, std::string strToConcat);

void fill_var_width(const std::string &line, VarWidth &varWidth);

void remove_back_space(std::string& str);

void remove_front_space(std::string& str);

void remove_two_end_space(std::string& str);

bool vec_has_only_vars(const std::vector<std::string> &vec);

bool is_concat(std::string line);

bool extract_concat(const std::string& var, std::vector<std::string> &vec);

bool split_concat(std::string var, std::vector<std::string> &vec);

bool check_input_val(const std::string& value);

void split_by(std::string str, std::string separator, std::vector<std::string> &vec);
void split_by_regex(const std::string& str, const std::regex& re, std::vector<std::string> &vec);
void split_by_regex(const std::string& str, const std::string& re_str, std::vector<std::string> &vec);


std::string merge_with(const std::vector<std::string> &vec, std::string connector);

std::string dec2hex(const std::string& decimalValue);

std::string dec2hex(uint32_t decimalValue);

void print_reg_list(const std::string& moduleName);
} // end of namespace taintGen
#endif
