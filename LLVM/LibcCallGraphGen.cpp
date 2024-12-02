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


/**
 * @brief Command line option to specify the output filename prefix
 *
 * @details This option allows the user to specify a prefix for the output filename.
 */
static cl::opt<std::string> OuputFilenamePrefix(
    "cg-output-name",
    cl::desc("Prefix name for the output file."),
    cl::value_desc("filename"),
    cl::init(""));

/**
 * @brief Command line option to specify the output filepath prefix
 *
 * @details This option allows the user to specify a prefix for the output filepath.
 */
static cl::opt<std::string> OuputFilepathPrefix(
    "cg-output-path",
    cl::desc("Prefix path for the output file."),
    cl::value_desc("filepath"),
    cl::init(""));

/**
 * @brief Command line option to specify the input file containing the libc functions listing
 *
 * @details This option allows the user to specify the input file containing the libc functions listing.
 */
static cl::opt<std::string> InputLibFuncsPath(
    "cg-lib-funcs-path",
    cl::desc("Input file for the libc functions listing generated by library_func_dump utility"),
    cl::value_desc("filepath"),
    cl::init(""));


/**
 * @brief Command line option to print the control flow graph in DOT format, i.e. the first pass
 *
 * @details This option allows the user to print the control flow graph in DOT format, i.e. the first pass - for debugging purposes
 */
static cl::opt<bool> PrintControlflowGraph(
    "cg-print-cfg",
    cl::desc("Print the control flow graph in DOT format"),
    cl::Hidden,
    cl::init(false));


/**
 * @brief Command line option to print the libc call graph in DOT format, i.e. the second pass
 *
 * @details This option allows the user to print the libc call graph in DOT format, i.e. the second pass - for debugging purposes
 */
static cl::opt<bool> PrintLibcCallGraph(
    "cg-print-libc-cg",
    cl::desc("Print the libc call graph in DOT format"),
    cl::Hidden,
    cl::init(false));
    
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
    std::string funcName = F.getName().str();

    for (BasicBlock &BB : F) {
        BB.printAsOperand(bbStream, false);
        BB.setName("["+funcName+"]"+bbStream.str()); // setting the name of the basic block to its label
        bbStr.clear();
    }    
}

//-----------------------------------------------------------------------------
// InjectFuncCall implementation
//-----------------------------------------------------------------------------

struct funcBBGraphMeta {
    std::string funcName;

    // Entry and exit node names
    std::string entryNode;
    std::string exitNode;

    // Map to store the libc calls for each basic block
    std::map <std::string, std::vector<std::string>> bbToLibcMap;

    LibcCallgraph bbGraph;          // Just basic block control flow graph
    LibcCallgraph bbExpandedGraph;  // Graph with libc calls expanded
    LibcCallgraph libcCallGraph;    // Graph with libc calls and program abstract state 
};
std::map<std::string, funcBBGraphMeta> funcBBToMetaMap;
LibcCallgraph finalGraph;       // Final graph with libc calls and program abstract state

/**
 * @brief Expand the basic block graph to include function calls
 */
void ExpandBBGraph(){
    for (auto &entry : funcBBToMetaMap) {
        auto &funcMeta = entry.second;
        const auto &funcName = entry.first;
        const auto &bbGraph = funcMeta.bbGraph;
        auto &bbExpandedGraph = funcMeta.bbExpandedGraph;
        // DEBUG_PRINT(BOLD_RED << "===================================================== " RESET << "\n");
        // DEBUG_PRINT(BOLD_GREEN << "Function: " << BOLD_WHITE << funcName << RESET << "\n");
        // DEBUG_PRINT(BOLD_RED << "===================================================== " RESET << "\n");
        bbExpandedGraph = bbGraph;

        // Expand the edges
        int counter=1;
        bool hadLibcCalls = false;
        std::string tempBBName, prevVertex, finalVertex, firstVertex;
        std::vector<std::string> neighbors;
        for (const auto &bbEntry : funcMeta.bbToLibcMap) {
            const auto &bbName = bbEntry.first;
            
            // DEBUG_PRINT(BOLD_YELLOW << "Checking neighbors for : "<< BOLD_WHITE << bbEntry.first<< RESET << "\n");
            firstVertex = bbEntry.first;
            neighbors = bbExpandedGraph.get_neighbors(bbEntry.first);
            // for (const auto &vertex : neighbors) {
            //      DEBUG_PRINT(BOLD_YELLOW << "\tVertex outgoing edge: " << BOLD_WHITE << firstVertex << BOLD_YELLOW << " -> " << BOLD_WHITE << vertex << RESET << "\n");
            // }
            
            counter = 1;
            
            prevVertex =  tempBBName  = bbName;

            const auto &libCalls = bbEntry.second;
            std::string vertexName;
            for (const auto &libCall : libCalls) {
                vertexName = bbName + ((libCall.find("user:") == 0) ? "-user_" : "-libc_") + std::to_string(counter++);
                bbExpandedGraph.add_vertex(vertexName);

                bbExpandedGraph.add_edge(prevVertex, vertexName, libCall);
                // DEBUG_PRINT(BOLD_YELLOW << "$$ Adding edge: " << BOLD_WHITE << prevVertex << BOLD_YELLOW << " -> " << BOLD_WHITE << vertexName << BOLD_YELLOW << " (" << BOLD_WHITE << libCall << BOLD_YELLOW << ")" << RESET << "\n");
                prevVertex = vertexName;
                hadLibcCalls = true;
            }
            if (funcMeta.exitNode == bbName) {
                funcMeta.exitNode = vertexName;
            }
            if (!neighbors.empty()) {
                // DEBUG_PRINT(BOLD_YELLOW << "Adding edge: " << BOLD_WHITE << firstVertex << BOLD_YELLOW << " -> " << BOLD_WHITE << funcMeta.exitNode << RESET << "\n");
                for (const auto &vertex : neighbors) {
                    bbExpandedGraph.remove_edge(firstVertex, vertex);
                    // DEBUG_PRINT(BOLD_YELLOW << "@@ Removing edge: " << BOLD_WHITE << firstVertex << BOLD_YELLOW << " -> " << BOLD_WHITE << vertex << RESET << "\n");
                    bbExpandedGraph.add_edge(prevVertex, vertex, "control");
                    // DEBUG_PRINT(BOLD_YELLOW << "@@ Adding edge: " << BOLD_WHITE << prevVertex << BOLD_YELLOW << " -> " << BOLD_WHITE << vertex << RESET << "\n");
                }
                neighbors.clear();
            }
            // if (counter > 1) {
            //     // DEBUG_PRINT(BOLD_YELLOW << "Removing edge: " << BOLD_WHITE << tempBBName << BOLD_YELLOW << " -> " << BOLD_WHITE << funcMeta.exitNode << RESET << "\n");
            //     bbExpandedGraph.remove_edge(tempBBName, funcMeta.exitNode);
            //     // DEBUG_PRINT(BOLD_YELLOW << "Adding edge: " << BOLD_WHITE << prevVertex << BOLD_YELLOW << " -> " << BOLD_WHITE << funcMeta.exitNode << RESET << "\n");
            //     bbExpandedGraph.add_edge(prevVertex, funcMeta.exitNode, "control");
            // }
        }

        std::string outputFilename = OuputFilepathPrefix +'/'+ OuputFilenamePrefix + funcName + "-expanded.dot";
        funcMeta.bbExpandedGraph.dump_todot(outputFilename);
        // DEBUG_PRINT(BOLD_GREEN << "Output filename: " << BOLD_WHITE << outputFilename << RESET << "\n");
        // DEBUG_PRINT(BOLD_GREEN << "\tEntry Node: " << BOLD_WHITE << funcMeta.entryNode << RESET << "\n");
        // DEBUG_PRINT(BOLD_GREEN << "\tExit Node: " << BOLD_WHITE << funcMeta.exitNode << RESET << "\n");        
    }
}


/**
 * @brief Convert the basic block graph to a libc call graph
 */
void ConvertBBGraphToLibcCallGraph(){
    for (auto &entry : funcBBToMetaMap) {
        auto &funcMeta = entry.second;
        const auto &funcName = entry.first;
        auto &bbExpandedGraph = funcMeta.bbExpandedGraph;
        auto &libcCallGraph = funcMeta.libcCallGraph;
        // DEBUG_PRINT(BOLD_RED << "===================================================== " RESET << "\n");
        // DEBUG_PRINT(BOLD_GREEN << "Function: " << BOLD_WHITE << funcName << RESET << "\n");
        // DEBUG_PRINT(BOLD_RED << "===================================================== " RESET << "\n");

        libcCallGraph = bbExpandedGraph;

        bool noMergeFound = false;
        int count = 0;

        while (!noMergeFound) {
            // DEBUG_PRINT(BOLD_YELLOW << "__________________________________________________\n");
            // DEBUG_PRINT(BOLD_YELLOW << "Iteration: " << BOLD_WHITE << count++ << RESET << "\n");
            noMergeFound = true;
            for (const auto &vertex : libcCallGraph.get_vertices()) {
                const std::vector<std::string> neighbors = libcCallGraph.get_control_edge_neighbors(vertex);
                // DEBUG_PRINT(BOLD_YELLOW << "Checking neighbors for : "<< BOLD_WHITE << vertex << " (" << neighbors.size() << ")" << RESET);
                // for (const auto &neighbor : neighbors) {
                //     DEBUG_PRINT(BOLD_YELLOW << " -> " << BOLD_WHITE << neighbor << RESET);
                // }
                // DEBUG_PRINT("\n");

                if (neighbors.size() >= 1) {
                    for (const auto &neighbor : neighbors) {

                        // DEBUG_PRINT(BOLD_YELLOW << "Merging: " << BOLD_WHITE << vertex << BOLD_YELLOW << " -> " << BOLD_WHITE << neighbor << RESET << "\n");
                        libcCallGraph.combine_vertex(vertex, neighbor);
                        if (funcMeta.exitNode == neighbor) {
                            funcMeta.exitNode = vertex;
                        }
                    }
                    noMergeFound = false;
                }
            }
        }

        std::string outputFilename = OuputFilepathPrefix +'/'+ OuputFilenamePrefix + funcName + "-libc.dot";
        funcMeta.libcCallGraph.dump_todot(outputFilename);
        // DEBUG_PRINT(BOLD_GREEN << "Output filename: " << BOLD_WHITE << outputFilename << RESET << "\n");
        // DEBUG_PRINT(BOLD_GREEN << "\tEntry Node: " << BOLD_WHITE << funcMeta.entryNode << RESET << "\n");
        // DEBUG_PRINT(BOLD_GREEN << "\tExit Node: " << BOLD_WHITE << funcMeta.exitNode << RESET << "\n");
    }
}

void CombineLibcgGraph (){
    finalGraph = funcBBToMetaMap["main"].libcCallGraph;
    DEBUG_PRINT(BOLD_YELLOW << "Main function: " << BOLD_WHITE << "main" << RESET << "\n");
    for(auto &entry : finalGraph.get_vertices()){
        DEBUG_PRINT(BOLD_YELLOW << "Vertex: " << BOLD_WHITE << entry << RESET << "\n");
        for(auto &edge : finalGraph.get_outgoing_edges(entry)){
            DEBUG_PRINT(BOLD_YELLOW << "    Edge: " << BOLD_WHITE << edge << RESET << "\n");
            if (edge.find("user:") == 0) {
                std::string functionCall = edge.substr(5);
                DEBUG_PRINT(BOLD_YELLOW << "        Function call: " << BOLD_WHITE << functionCall << RESET << "\n");
                LibcCallgraph funcGraph = funcBBToMetaMap[functionCall].libcCallGraph;
                std::string entry = funcBBToMetaMap[functionCall].entryNode;
                std::string exit = funcBBToMetaMap[functionCall].exitNode;

                DEBUG_PRINT(BOLD_YELLOW << "        Entry: " << BOLD_WHITE << entry << RESET << "\n");
                DEBUG_PRINT(BOLD_YELLOW << "        Exit: " << BOLD_WHITE << exit << RESET << "\n");
                finalGraph.insert_graph(funcGraph, entry, exit);   
            }
        }
    }
    std::string outputFilename = OuputFilepathPrefix +'/'+ OuputFilenamePrefix + "final.dot";
    finalGraph.dump_todot(outputFilename);
    DEBUG_PRINT(BOLD_GREEN << "Output filename: " << BOLD_WHITE << outputFilename << RESET << "\n");
    DEBUG_PRINT(BOLD_GREEN << "\tEntry Node: " << BOLD_WHITE << finalGraph.get_vertices().front() << RESET << "\n");
    DEBUG_PRINT(BOLD_GREEN << "\tExit Node: " << BOLD_WHITE << finalGraph.get_vertices().back() << RESET << "\n");
}


bool LibcSandboxing::runOnModule(Module &M, ModuleAnalysisManager &MAM, FunctionAnalysisManager &FAM) {
    bool InsertedAtLeastOnePrintf = false;
    
    setupDummySyscall(M);

    std::map<std::string, std::vector<std::string>> funcToLibcMap;

    for (auto &F : M) {
        struct funcBBGraphMeta funcMeta;
        
        if (F.isDeclaration()) continue;            // Skip external functions
        std::string funcName = F.getName().str();
        if (funcName.find("llvm.") == 0) continue; // Skip internal LLVM functions
        if (funcName.find("syscall") == 0) continue; // Skip the syscall wrapper function used for injection

        // DEBUG_PRINT(GREEN<<"\n===== Function: " << WHITE << funcName << GREEN << " =====\n"<<RESET);
        LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
        funcMeta.funcName = funcName;
        ///// Name the basic blocks
        nameBasicBlocks(F);        

        ///// Generate the call graph - vertices/basicblocks
        for (BasicBlock &BB : F) {
            // DEBUG_PRINT_BB(BB);
            auto *TI = BB.getTerminator();
            if (BB.hasNPredecessors(0)) {
                funcMeta.entryNode = BB.getName().str();
                funcMeta.bbGraph.add_vertex(BB.getName().str(), (fileToMapReader.getLibraryCalls(BB).empty()) ? false : true);
                // DEBUG_PRINT("ENTRY\n");
            } 
                
            if (isa<ReturnInst>(TI)) {
                funcMeta.exitNode = BB.getName().str();
                funcMeta.bbGraph.add_vertex(BB.getName().str(),(fileToMapReader.getLibraryCalls(BB).empty()) ? false : true);
                // DEBUG_PRINT("EXIT\n");
                continue;
            }

            funcMeta.bbGraph.add_vertex(BB.getName().str(),
                                    (fileToMapReader.getLibraryCalls(BB).empty()) ? false : true);
            // DEBUG_PRINT("INTERNAL\n");
        }

        ///// Generate the call graph - populate edges
        for (BasicBlock &BB : F) {
            for (BasicBlock *Succ : successors(&BB)) {
                funcMeta.bbGraph.add_edge(BB.getName().str(), Succ->getName().str(), "control");
            }
        }

        ///// Generate the libc call list for each BB
        for (BasicBlock &BB : F) {
            std::vector<std::string> libCalls = fileToMapReader.getLibraryCalls(BB);
            if (!libCalls.empty()) {
                funcMeta.bbToLibcMap[BB.getName().str()] = libCalls;
            }
        }
        funcBBToMetaMap[funcName] = funcMeta;
        
        std::string outputFilename = OuputFilepathPrefix +'/'+ OuputFilenamePrefix + funcName + ".dot";
        // DEBUG_PRINT(BOLD_GREEN << "Output filename: " << BOLD_WHITE << outputFilename << RESET << "\n");
        funcMeta.bbGraph.dump_todot(outputFilename);
       
        
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
////////////////////////////////////////////////////////////
//    // Dump the function to libc call map
//     for (const auto &entry : funcToLibcMap) {
//         DEBUG_PRINT(BOLD_GREEN << "-----------------Function: " << BOLD_WHITE << entry.first << RESET << "\n");
//         for (const auto &libCall : entry.second) {
//             DEBUG_PRINT(BOLD_YELLOW << "  Libc Call: " << BOLD_MAGENTA << libCall << RESET << "\n");
//         }
//     }
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
//// Dump the function to meta map
// for (const auto &entry : funcBBToMetaMap) {
//     const auto &funcMeta = entry.second;
//     DEBUG_PRINT(BOLD_GREEN << "Function: " << BOLD_WHITE << funcMeta.funcName << RESET << "\n");
//     DEBUG_PRINT(BOLD_GREEN << "  Entry Node: " << BOLD_WHITE << funcMeta.entryNode << RESET << "\n");
//     DEBUG_PRINT(BOLD_GREEN << "  Exit Node: " << BOLD_WHITE << funcMeta.exitNode << RESET << "\n");
//     DEBUG_PRINT(BOLD_GREEN << "  Basic Blocks to Func Calls Map:" << RESET << "\n");
//     for (const auto &bbEntry : funcMeta.bbToLibcMap) {
//         DEBUG_PRINT(BOLD_YELLOW << "    Basic Block: " << BOLD_WHITE << bbEntry.first << RESET << "\n");
//         for (const auto &libCall : bbEntry.second) {
//             DEBUG_PRINT(BOLD_MAGENTA << "      Func Call: " << BOLD_WHITE << libCall << RESET << "\n");
//         }
//     }
// }
////////////////////////////////////////////////////////////
    ExpandBBGraph();
    ConvertBBGraphToLibcCallGraph();
    CombineLibcgGraph ();
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
