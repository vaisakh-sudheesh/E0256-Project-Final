#ifndef LLVM_ANALYSIS_UTILS_LIBCCALLGRAPHDEBUG_H
#define LLVM_ANALYSIS_UTILS_LIBCCALLGRAPHDEBUG_H


/**
 * @brief ANSI color codes for terminal output
 */
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define GREY   "\033[90m"      /* Grey */
#define BOLD_BLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLD_RED     "\033[1m\033[31m"      /* Bold Red */
#define BOLD_GREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLD_YELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLD_BLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLD_MAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLD_CYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLD_WHITE   "\033[1m\033[37m"      /* Bold White */
#define BOLD_GREY   "\033[1m\033[90m"      /* Bold Grey */


/**
 * @brief Debug print macro
 */
#define DEBUG_PRINT(x) if (libcCallGraphDebug) errs()<<x
#define DEBUG_BRANCH_TARGET_INSN(pre, x) if (libcCallGraphDebug){errs()<<pre;x->getFirstNonPHI()->print(errs());errs()<< "\n";}

unsigned int PrintAlignment = 0;

#define PRINT_INDENT() for (unsigned int i = 0; i < PrintAlignment; i++) {errs()<<"\t";}
#define INCREMENT_INDENT PrintAlignment++
#define DECREMENT_INDENT (PrintAlignment > 0 ? PrintAlignment-- : PrintAlignment)

#define ERROR_PRINT(x) errs()<<x
#define DEBUG_PRINT_BB(x) if (libcCallGraphDebug){                                  \
            int LoopCounter = LI.getLoopDepth(&x);                                  \
            PRINT_INDENT();                                                       \
            for  (int i = 0; i < LoopCounter; i++) errs()<<"\t";              \
            errs()<<BOLD_YELLOW << "Basic Block: "<< BOLD_MAGENTA << x.getName();   \
            errs() << GREY<< " - ";x.front().print(errs()); \
            errs()<< RESET << " , ";                                                \
        }errs()<< RESET << "\n"

#define DEBUG_PRINT_BB_PTR(x) if (libcCallGraphDebug){                                  \
            int LoopCounter = LI.getLoopDepth(x);                                  \
            PRINT_INDENT();                                                       \
            for  (int i = 0; i < LoopCounter; i++) errs()<<"\t";              \
            errs()<<BOLD_YELLOW << "Basic Block: "<< BOLD_MAGENTA << x->getName();   \
            errs() << GREY<< " - ";x->front().print(errs()); \
            errs()<< RESET << " , ";                                                \
        }errs()<< RESET << "\n"

#endif // LLVM_ANALYSIS_UTILS_LIBCCALLGRAPHDEBUG_H
