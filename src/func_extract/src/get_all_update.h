#ifndef FUNC_EXTRACT_GET_ALL_UPDATE_H
#define FUNC_EXTRACT_GET_ALL_UPDATE_H

#include <set>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <utility>
#include <regex>
#include <thread>
#include <mutex>
#include <functional>
#include <atomic>
#include "global_data_struct.h"

namespace funcExtract {


struct WorkSet_t {
  public:
    void mtxInsert(std::string reg);
    void mtxErase(std::set<std::string>::iterator it);
    void mtxAssign(std::set<std::string> &set);
    void mtxClear();
    bool empty();
    std::set<std::string>::iterator begin();
    void copy(std::set<std::string> &copySet);
    bool mtxExist(std::string reg);

  private:
    std::mutex mtx;
    std::set<std::string> workSet;
    
};


struct RunningThreadCnt_t {
  public:
    void increase();
    void decrease();
    uint32_t get();
    RunningThreadCnt_t() : cnt(0) {}

  private:
    std::mutex mtx;
    uint32_t cnt;
};


struct ThreadSafeVector_t {
  public:
    void push_back(std::string var);
    std::vector<std::string>::iterator begin();
    std::vector<std::string>::iterator end();

  private:
    std::mutex mtx;
    std::vector<std::string> vec;
};



template <typename T>
struct ThreadSafeMap_t {
  public:
    void emplace(const std::string& var, const T& data);
    typename std::map<std::string, T>::iterator begin();
    typename std::map<std::string, T>::iterator end();
    bool contains(const std::string& var);
    T& at(const std::string& var);

  private:
    std::mutex mtx;
    std::map<std::string, T> mp;
};


class FuncExtractFlow {

private:
  UFGenFactory& m_genFactory;
  ModuleInfo& m_info;
  const bool m_innerLoopIsInstrs;
  const bool m_reverseCycleOrder;


  // Gathers data to be written to func_info.txt and asv_info.txt,
  // which later get read by sim_gen.
  // The first key is instr name, the second key is target name (scalar or vector)
  std::map<std::string, std::map<std::string, ArgVec_t>> m_dependVarMap;
  std::mutex m_dependVarMapMtx;

  std::mutex m_TimeFileMtx;

  ThreadSafeMap_t<WidthCycles_t> m_asvSet;

  WorkSet_t m_workSet;
  ThreadSafeVector_t m_fileNameVec;
  std::shared_ptr<ModuleInfo_t> m_topModuleInfo;
  WorkSet_t m_visitedTgt;



public:
  FuncExtractFlow(UFGenFactory& genFactory, ModuleInfo& info,
                  bool innerLoopIsInstrs, bool reverseCycleOrder);

  FuncExtractFlow() = delete;

  // Main entry point - calls the other functions.
  void get_all_update();


  bool clean_main_func(llvm::Module& M,
                       std::string funcName);

  std::string create_wrapper_func(llvm::Module& M,
                           std::string mainFuncName);

  bool gather_wrapper_func_args(llvm::Module& M,
                        std::string wrapperFuncName,
                        std::string target,
                        int delayBound,
                        ArgVec_t &argVec);

  std::vector<uint32_t>
  get_delay_bounds(std::string var, const InstrInfo_t& instrInfo);

  void print_func_info(std::ofstream &output);

  void print_asv_info(std::ofstream &output);

  void print_llvm_script( std::string fileName);

  void get_update_function(std::string target,
                           uint32_t delayBound,
                           bool isVec,
                           InstrInfo_t instrInfo,
                           uint32_t instrIdx);

  llvm::Function *remove_dead_args(llvm::Function *func);


};

} // end of namespace
#endif
