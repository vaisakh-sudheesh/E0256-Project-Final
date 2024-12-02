#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

#include "LibcCallGraphUtils.h"



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
    void injectDummySyscall(llvm::Module &M, llvm::Function &F, llvm::Instruction &I, int syscallNum);

    void nameBasicBlocks(llvm::Function &F);
};

#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;

#define DEBUG_TYPE "libc-sandboxing"



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

void LibcSandboxing::injectDummySyscall(Module &M, Function &F, Instruction &I, int syscallNum){
    
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
    }
}

//-----------------------------------------------------------------------------
// InjectFuncCall implementation
//-----------------------------------------------------------------------------
bool LibcSandboxing::runOnModule(Module &M, ModuleAnalysisManager &MAM, FunctionAnalysisManager &FAM) {
    bool InsertedAtLeastOnePrintf = false;

    setupDummySyscall(M);

    for (auto &F : M) {
        if (F.isDeclaration()) continue;            // Skip external functions
        std::string funcName = F.getName().str();
        if (funcName.find("llvm.") == 0) continue; // Skip internal LLVM functions
        if (funcName.find("syscall") == 0) continue; // Skip the syscall wrapper function used for injection

        DEBUG_PRINT(GREEN<<"\n===== Function: " << WHITE << funcName << GREEN << " =====\n"<<RESET);
        LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
        for (BasicBlock &BB : F) {
            ///// Name the basic blocks
            nameBasicBlocks(F);

            ///// Generate the graph
            DEBUG_PRINT_BB(BB);

            ///// Inject syscall for libc calls
            for (Instruction &I : BB) {
                if (CallInst *CI = dyn_cast<CallInst>(&I)) {
                    Function *Callee = CI->getCalledFunction();
                    if (Callee) {
                        std::string funcName = Callee->getName().str();
                        if (funcName.find("syscall") == 0) continue;
                        if (fileToMapReader.isStringInMap(funcName)) {
                            DEBUG_PRINT(YELLOW << "Found libc call: " << WHITE << funcName << "\n" << RESET);
                            injectDummySyscall(M, F, I, fileToMapReader.getValueFromMap(funcName));
                            InsertedAtLeastOnePrintf = true;
                        }
                    }
                }
            }
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
