#ifndef FUNC_EXTRACT_CHECK_REGS_H 
#define FUNC_EXTRACT_CHECK_REGS_H

#include "../../live_analysis/src/varWidth.h"
#include "../../live_analysis/src/taint_gen.h"
#include "../../live_analysis/src/global_data.h"
#include "../../live_analysis/src/helper.h"
#include "global_data_struct.h"
#include "ins_context_stack.h"
#include "ast.h"
#include <string>
#include <queue>
#include <cmath>
#include <regex>
#include <utility>
#include <fstream>
#include <time.h>
#include <math.h>
//#include "z3++.h"
#define context std::shared_ptr<llvm::LLVMContext>
#define builder std::shared_ptr<llvm::IRBuilder<>>


namespace funcExtract {

// class for destination reg of IR
class DestInfo {
  private:
    std::string destAndSlice;
    uint32_t destWidth;
    std::vector<std::string> destVec;
    std::string modName;
    std::string insName;
    std::string instrName;

  public:
    bool isVector;
    bool isMemVec;
    bool isSingleMem;
    std::string get_dest_name();
    llvm::Type* get_ret_type(std::shared_ptr<llvm::LLVMContext> TheContext);
    llvm::Type* get_ret_element_type(std::shared_ptr<llvm::LLVMContext> TheContext);
    llvm::Type* get_padded_ret_element_type(std::shared_ptr<llvm::LLVMContext> TheContext);
    std::vector<std::string> get_no_slice_name();
    std::string get_mod_name();
    std::string get_ins_name();
    std::string get_instr_name();
    std::string get_func_name();
    void set_dest_and_slice(std::string var, uint32_t width);
    void set_module_name(std::string var);
    void set_instr_name(std::string var);
    void set_instance_name(std::string var);
    void set_dest_vec(const std::vector<std::string> &vec);
    llvm::Type* get_arr_type(std::shared_ptr<llvm::LLVMContext> TheContext);
};


struct MemDynInfo_t {
  uint32_t lastWriteTimeIdx;
  bool hasBeenWritten;
  llvm::Value* memValue;
};


struct ModuleDynInfo_t {
  /// data for making ir
  /// These data are different for making different irs
  // first key is target name
  std::map<std::string, std::map<std::string, llvm::Value*>> existedExpr;
  // the first key is target name
  std::map<std::string, std::map<std::string, uint32_t>> minInOutDelay;
  std::map<std::string, std::pair<llvm::Function*, uint32_t>> out2FuncMp;
  // first key is output, second key is input
  std::map<std::string, 
           std::map<std::string, uint32_t>> out2InDelayMp; 
  bool isFunctionedSubMod = true;

  void clean_ir_data();  

  std::map<std::string, MemDynInfo_t> memDynInfo;
};


// An implementation of ModuleInfo which uses data from g_moduleInfoMap.
class ModuleInfoImpl : public ModuleInfo {

public:
  // A blank modname implies the top module.
  uint32_t get_var_width_simp(const std::string& varAndSlice,
                              const std::string& modname) override;

  uint32_t get_var_width_cmplx(const std::string& varAndSlice) override;

  bool is_module(const std::string& modname) override;

  // Return true if the given wire is an output port of the top module,
  // and is driven by a fifo instance.
  bool is_fifo_output(const std::string& wire) override;

  // If instname names an instance in the given parent mod, return its module
  // name.  Otherwise return a blank string.  A blank parentmod implies
  // the top module.
  std::string get_module_of_inst(const std::string& instname,
                                 const std::string& parentmod) override;

  // Fill in the provided (empty) vector with the output ports of the given module.
  // Of course, a blank modname implies the top module.
  void get_module_outputs(std::vector<std::string>& outputs,
                          const std::string& modname) override;

  // Fill in the provided (empty) vector with the fifo instance names in the given module.
  // Of course, a blank modname implies the top module.
  void get_fifo_insts(std::vector<std::string>& fifos,
                          const std::string& modname) override;
};


// An implementation of UFGenerator which uses data from
// g_moduleInfoMap and the design's AST.
class UpdateFunctionGen : public UFGenerator {
  public:
    UpdateFunctionGen();

    void print_llvm_ir(DestInfo &destInfo, 
                       const uint32_t bound, 
                       uint32_t instrIdx,
                       std::string fileName) override;

    // make_llvm_basename() is inherited from the base class

  private:
    
    // first of pair is instance name, however, the first in the vector should be
    // module name, instead of instance name.
    // Currently there is a restriction: the first module in the vector can only
    // have one instance
    HierCtx insContextStk;
    uint32_t cct_cnt = 0;
    uint32_t ext_cnt = 0;
    std::map<std::string, uint32_t> cctNameIdxMap;
    std::map<std::string, uint32_t> extNameIdxMap;
//    std::vector<std::pair<std::string, uint32_t>> regWidth;
    std::vector<std::pair<std::string, std::string>> memInstances;
    struct InstrInfo_t currInstrInfo;
    std::map<std::string, llvm::Function::arg_iterator> topFuncArgMp;
    std::map<std::string, llvm::Function*> extractFunc;
    std::map<std::string, llvm::Function*> concatFunc;
    bool ignoreSubModules;
    bool skipCheck;
    // CLEAN_QUEUE includes: input, not-current AS and num
    std::map<astNode*, uint32_t> CLEAN_QUEUE;
    std::map<astNode*, uint32_t> DIRTY_QUEUE;
    std::set<std::string> resetedReg;

    // for llvm
    std::shared_ptr<llvm::LLVMContext> TheContext;
    std::shared_ptr<llvm::Module> TheModule;
    llvm::Function *TheFunction;
    std::shared_ptr<llvm::IRBuilder<>> Builder;
    llvm::BasicBlock *BB;
    std::map<std::string, std::shared_ptr<ModuleDynInfo_t>> dynDataMp;
    std::map<std::string, astNode*> memNodeMap;

    llvm::Value* add_constraint(std::string varAndSlice, uint32_t timeIdx, context &c,
                                std::shared_ptr<llvm::IRBuilder<>> &b,
                                const uint32_t bound);
    
    llvm::Value* add_constraint(astNode* const node, uint32_t timeIdx, context &c,
                                std::shared_ptr<llvm::IRBuilder<>> &b,
                                const uint32_t bound );

    llvm::Value* add_nb_constraint(astNode* const node, 
                                   uint32_t timeIdx, context &c, 
                                   std::shared_ptr<llvm::IRBuilder<>> &b,
                                   const uint32_t bound);
    
    llvm::Value* add_ssa_constraint(astNode* const node, uint32_t timeIdx, context &c,  
                                    std::shared_ptr<llvm::IRBuilder<>> &b, const uint32_t bound);
    
    //void add_child_constraint(astNode* const parentNode, uint32_t timeIdx, 
    //                          context &c, z3::solver &s, z3::goal &g, 
    //                          const uint32_t bound, bool isSolve);

    void push_clean_queue(astNode* node, uint32_t timeIdx);
    
    void push_dirty_queue(astNode* node, uint32_t timeIdx);
    
    void clean_all_mod_dynamic_info();
    
    void save_regs_for_expand(std::set<std::string> &varToExpand);
    
    bool is_in_clean_queue(std::string var);
    
    bool is_in_dirty_queue(std::string var);
    
    void clean_data();

    void initialize_min_delay(std::shared_ptr<ModuleInfo_t> &modInfo, 
                              std::string outPort);

    // llvm-related functions
    llvm::Value* long_bv_val(std::string formedBinVar, context &c,
                             std::shared_ptr<llvm::IRBuilder<>> &b );

    llvm::Value* get_arg(std::string regName, llvm::Function *func);

    llvm::Value* get_arg(std::string regName);

    std::shared_ptr<ModuleDynInfo_t>
    get_dyn_data(std::shared_ptr<ModuleInfo_t> curMod);
    
    
    llvm::Value* var_expr(std::string varAndSlice, uint32_t timeIdx, context &c, 
                          std::shared_ptr<llvm::IRBuilder<>> &b, 
                          bool isTaint, uint32_t width=0);
    
    llvm::Value* bv_val(std::string var, context &c);
    
    llvm::Value* bool_expr(std::string var, uint32_t timeIdx, context &c, bool isTaint=false);
    
    llvm::Value* input_constraint(astNode* const node, uint32_t timeIdx, context &c, 
                                  std::shared_ptr<llvm::IRBuilder<>> &b, 
                                  const uint32_t bound);
    
    llvm::Value* mixed_value_expr(std::string value, context &c, std::string varName, 
                                  uint32_t timeIdx, uint32_t idx,
                                  std::shared_ptr<llvm::IRBuilder<>> &b );
    
    llvm::Value* single_expr(std::string value, context &c, std::string varName, 
                             uint32_t timeIdx, uint32_t idx,
                             std::shared_ptr<llvm::IRBuilder<>> &b );
    
    llvm::Value* num_constraint(astNode* const node, uint32_t timeIdx, context &c,
                                std::shared_ptr<llvm::IRBuilder<>> &b);
    
    llvm::Value* two_op_constraint(astNode* const node, uint32_t timeIdx, context &c, 
                                   std::shared_ptr<llvm::IRBuilder<>> &b, const uint32_t bound);
    
    llvm::Value* one_op_constraint(astNode* const node, uint32_t timeIdx, 
                                   context &c, builder &b, const uint32_t bound);
    
    llvm::Value* reduce_one_op_constraint(astNode* const node, uint32_t timeIdx, 
                                          context &c, builder &b, const uint32_t bound);
    
    llvm::Value* sel5_op_constraint(astNode* const node, uint32_t timeIdx, 
                                    context &c, builder &b, const uint32_t bound );
    
    llvm::Value* sel_op_constraint(astNode* const node, uint32_t timeIdx, 
                                   context &c, builder &b, const uint32_t bound );
    
    llvm::Value* src_concat_op_constraint(astNode* const node, uint32_t timeIdx, 
                                          context &c, builder &b, const uint32_t bound );
    
    llvm::Value* add_one_concat_expr(astNode* const node, uint32_t nxtIdx, uint32_t timeIdx, 
                                     context &c, builder &b, const uint32_t bound, bool noinline=true );
    
    llvm::Value* ite_op_constraint(astNode* const node, uint32_t timeIdx, context &c, 
                                   builder &b, const uint32_t bound );
    
    llvm::Value* case_constraint(astNode* const node, uint32_t timeIdx, 
                                 context &c, builder &b, const uint32_t bound);
    
    llvm::Value* switch_constraint(astNode* const node, uint32_t timeIdx, 
                                   context &c, builder &b, const uint32_t bound);

    llvm::Value* add_one_case_branch_expr(astNode* const node, llvm::Value* &caseVarExpr, 
                                          uint32_t idx, uint32_t timeIdx, context &c, 
                                          builder &b, const uint32_t bound, 
                                          const std::string &destTimed);
    
    llvm::Value* memMod_constraint(astNode* const node, uint32_t timeIdx, context &c,
                               builder &b, const uint32_t bound);

    llvm::Value* bbMod_constraint(astNode* const node, uint32_t timeIdx, context &c, 
                                   builder &b, const uint32_t bound);
    
    llvm::Value* submod_constraint(astNode* const node, uint32_t timeIdx, context &c, 
                                   builder &b, const uint32_t bound);

    llvm::Value* dyn_sel_constraint( astNode* const node, uint32_t timeIdx, context &c,  
                                    std::shared_ptr<llvm::IRBuilder<>> &b, const uint32_t bound);

    void mem_assign_constraint( astNode* const node, uint32_t timeIdx, context &c,  
                                std::shared_ptr<llvm::IRBuilder<>> &b, const uint32_t bound);

    llvm::Value* extract_func(llvm::Value* in, uint32_t high, uint32_t low,
                          std::shared_ptr<llvm::LLVMContext> &c, 
                          std::shared_ptr<llvm::IRBuilder<>> &b, uint32_t timeIdx,
                          const llvm::Twine &name="", bool noinline=false);
    
    
    //llvm::Value* extract(llvm::Value* in, uint32_t high, uint32_t low, 
    //                      std::shared_ptr<llvm::LLVMContext> &c, 
    //                      std::shared_ptr<llvm::IRBuilder<>> &b,
    //                      const llvm::Twine &name="");
    //
    //
    //llvm::Value* extract(llvm::Value* in, uint32_t high, uint32_t low, 
    //                      std::shared_ptr<llvm::LLVMContext> &c, 
    //                      std::shared_ptr<llvm::IRBuilder<>> &b, 
    //                      const std::string &name);
    
    
    llvm::Value* concat_value(llvm::Value* val1, llvm::Value* val2, 
                              std::shared_ptr<llvm::LLVMContext> &c,
                              std::shared_ptr<llvm::IRBuilder<>> &b);
    
    llvm::Value* concat_func(llvm::Value* val1, llvm::Value* val2, 
                             std::shared_ptr<llvm::LLVMContext> &c,
                             std::shared_ptr<llvm::IRBuilder<>> &b,
                             uint32_t timeIdx,                             
                             bool noinline=false);

    llvm::Value* zext(llvm::Value* v1, uint32_t width,
                      std::shared_ptr<llvm::LLVMContext> &c,
                      std::shared_ptr<llvm::IRBuilder<>> &b);


    llvm::Value* sext(llvm::Value* v1, uint32_t width,
                      std::shared_ptr<llvm::LLVMContext> &c,
                      std::shared_ptr<llvm::IRBuilder<>> &b);


    llvm::Value* make_llvm_instr(std::shared_ptr<llvm::IRBuilder<>> &b, 
                             context &c, std::string op, 
                             llvm::Value* op1Expr, llvm::Value* op2Expr, 
                             uint32_t destWidth, uint32_t op1Width, uint32_t op2Width,
                             const llvm::Twine &name, bool isSigned=false);

    llvm::Value* make_llvm_instr(builder &b, context &c, std::string op, 
                             llvm::Value* op1Expr, uint32_t op1WidthNum,
                             const llvm::Twine &name);
};




//extern bool g_ignoreSubModules;
// seems useless
extern bool g_seeInputs;
// seems useless
extern uint32_t g_maxDelay;

void check_all_regs();

void clean_data();

void print_llvm_ir_without_submodules(std::string destAndSlice, 
                                      const uint32_t bound, 
                                      uint32_t instrIdx);

//void check_single_reg_and_slice(std::string regAndSlice, 
//                                uint32_t cycleCnt, 
//                                uint32_t instrIdx);


//void add_clean_constraint(astNode* const node, uint32_t timeIdx, context &c, 
//                          z3::solver &s, z3::goal &g, const uint32_t bound, bool isSolve);


//void add_all_clean_constraints(context &c, z3::solver &s, z3::goal &g, 
//                               const uint32_t bound, bool isSolve=true);

//void add_dirty_constraint(astNode* const node, uint32_t timeIdx, 
//                          context &c, z3::solver &s, const uint32_t bound);


//void add_all_dirty_constraints(context &c, z3::solver &s, const uint32_t bound);

//void add_input_values(context &c, z3::solver &s, const uint32_t bound);

//void add_nop(context &c, z3::solver &s, const uint32_t bound);

void print_time(); 

void clean_module_inputs();

void print_mem_info(std::vector<std::string> &mems);

void print_context_info(Context_t& insCntxt);

} // end of namespace funcExtract

#endif
