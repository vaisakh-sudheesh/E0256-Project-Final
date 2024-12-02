#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

#include "LibcCallGraphUtils.h"

#include <map>
#include <string>
#include <vector>

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
struct LibcSandboxing : public llvm::PassInfoMixin<LibcSandboxing> {
private:
    llvm::FileToMapReader fileToMapReader;
    llvm::Function *SyscallF;
    llvm::FunctionCallee Syscall;

public:
    llvm::PreservedAnalyses run(llvm::Module &M,
                                llvm::ModuleAnalysisManager &);
    bool runOnModule(llvm::Module &M, llvm::ModuleAnalysisManager &MAM, llvm::FunctionAnalysisManager &FAM);

    static bool isRequired() { return true; }

    void setupDummySyscall(llvm::Module &M);
    void injectDummySyscall(llvm::Instruction &I, int syscallNum);

    void nameBasicBlocks(llvm::Function &F);
};

#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;

#define DEBUG_TYPE "libc-sandboxing"

#include "GraphLib.hpp"

/**
 * @brief Command line option to enable debug mode
 *
 * @details This option allows the user to enable debug mode for the libc call graph pass.
 */
static cl::opt<bool>
libcCallGraphDebug("cg-debug",
                  cl::desc("Enable debug mode for libc call graph pass."),
                  cl::Hidden,
                  cl::init(true));

#include "LibcCallGraphDebug.h"

//-----------------------------------------------------------------------------
// Utility functions
//-----------------------------------------------------------------------------

void LibcSandboxing::setupDummySyscall(Module &M) {
    // Create a function that will be called by the injected call
    auto &CTX = M.getContext();
    IntegerType *Int64Ty = Type::getInt64Ty(CTX);

    FunctionType *SyscallTy = FunctionType::get(
        Type::getInt64Ty(CTX),
        Int64Ty,
        /*isVarArg=*/true);

    
    Syscall = M.getOrInsertFunction("syscall", SyscallTy);
    assert(Syscall && "Syscall function not found");

    // Set attributes as per inferLibFuncAttributes in BuildLibCalls.cpp
    SyscallF = dyn_cast<Function>(Syscall.getCallee());
    SyscallF->setDoesNotThrow();
}

void LibcSandboxing::injectDummySyscall(Instruction &I, int syscallNum){
    
    IRBuilder<> Builder(&I);

    llvm::Value *syscallNumber = Builder.getInt64(2513);
    Builder.CreateCall(
        Syscall, {syscallNumber, Builder.getInt64(syscallNum)});
    
}

void LibcSandboxing::nameBasicBlocks(llvm::Function &F){
    std::string bbStr;
    raw_string_ostream bbStream(bbStr);

    for (BasicBlock &BB : F) {
        BB.printAsOperand(bbStream, false);
        BB.setName(bbStream.str()); // setting the name of the basic block to its label
        bbStr.clear();
    }    
}

//-----------------------------------------------------------------------------
// InjectFuncCall implementation
//-----------------------------------------------------------------------------
bool LibcSandboxing::runOnModule(Module &M, ModuleAnalysisManager &MAM, FunctionAnalysisManager &FAM) {
    bool InsertedAtLeastOnePrintf = false;
    

    setupDummySyscall(M);

    std::map<std::string, std::vector<std::string>> funcToLibcMap;

    for (auto &F : M) {
        LibcCallgraph libcCallGraph;
        if (F.isDeclaration()) continue;            // Skip external functions
        std::string funcName = F.getName().str();
        if (funcName.find("llvm.") == 0) continue; // Skip internal LLVM functions
        if (funcName.find("syscall") == 0) continue; // Skip the syscall wrapper function used for injection

        DEBUG_PRINT(GREEN<<"\n===== Function: " << WHITE << funcName << GREEN << " =====\n"<<RESET);
        LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
        nameBasicBlocks(F);
        ///// Name the basic blocks
        for (BasicBlock &BB : F) {
            
            std::vector<std::string> libCalls = fileToMapReader.getLibraryCalls(BB);
            funcToLibcMap[funcName].insert(funcToLibcMap[funcName].end(), libCalls.begin(), libCalls.end());
        }

        ///// Generate the call graph - vertices/basicblocks
        for (BasicBlock &BB : F) {
            DEBUG_PRINT_BB(BB);
            if (BB.hasNPredecessors(0)) {
                libcCallGraph.add_vertex(BB.getName().str());
                DEBUG_PRINT("ENTRY\n");
            } else {
                auto *TI = BB.getTerminator();
                if (isa<ReturnInst>(TI)) {
                    libcCallGraph.add_vertex(BB.getName().str());
                    DEBUG_PRINT("EXIT\n");
                } else {
                    libcCallGraph.add_vertex(BB.getName().str());
                    DEBUG_PRINT("INTERNAL\n");
                }
            }
        }

        ///// Generate the call graph - populate edges
        for (BasicBlock &BB : F) {
            for (BasicBlock *Succ : successors(&BB)) {
                libcCallGraph.add_edge(BB.getName().str(), Succ->getName().str(), "control");
            }
        }
        
        libcCallGraph.dump_todot("libc-callgraph"+funcName+".dot");
       
        
        ///// Inject the dummy syscall
        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                if (CallInst *CI = dyn_cast<CallInst>(&I)) {
                    Function *Callee = CI->getCalledFunction();
                    if (Callee) {
                        std::string funcName = Callee->getName().str();
                        if (fileToMapReader.isStringInMap(funcName)) {
                            int syscallNum = fileToMapReader.getValueFromMap(funcName);
                            // DEBUG_PRINT(BOLD_YELLOW << "Found libc call: " << BOLD_MAGENTA << funcName << RESET << " - syscall number: " << syscallNum << "\n");
                            injectDummySyscall(I, syscallNum);
                            InsertedAtLeastOnePrintf = true;
                        }
                    }
                }
            }
        }
    
  }
   // Dump the function to libc call map
    for (const auto &entry : funcToLibcMap) {
        DEBUG_PRINT(BOLD_GREEN << "-----------------Function: " << BOLD_WHITE << entry.first << RESET << "\n");
        for (const auto &libCall : entry.second) {
            DEBUG_PRINT(BOLD_YELLOW << "  Libc Call: " << BOLD_MAGENTA << libCall << RESET << "\n");
        }
    }

  return InsertedAtLeastOnePrintf;
}

PreservedAnalyses LibcSandboxing::run(llvm::Module &M,
                                       llvm::ModuleAnalysisManager &AM) {
    FunctionAnalysisManager &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
    fileToMapReader.readFileToMap("/home/vaisakhps/developer/E0256-Security/E0-256_ProjectFinal/test/libc_listing.lst");
    bool Changed =  runOnModule(M, AM, FAM);

    return (Changed ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all());
}


//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getInjectFuncCallPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "libc-sandboxing", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "libc-sandboxing") {
                    MPM.addPass(LibcSandboxing());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getInjectFuncCallPluginInfo();
}
