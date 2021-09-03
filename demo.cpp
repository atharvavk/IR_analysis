// C++ standard libraries
#include <stack>
#include <vector>
#include <string>
#include <unordered_map>
#include <set>
// LLVM components
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "rda"

using namespace llvm;

namespace
{
    // Reaching Definition Analysis Pass
    class RDA : public FunctionPass
    {
    public:
        static char ID;

    public:
        RDA() : FunctionPass(ID) {}
        ~RDA() {}

    public:
        bool runOnFunction(Function &f) override
        {

            std::unordered_map<BasicBlock *, std::set<BasicBlock *>> CFG;
            errs() << "\n\nFunction : " << f.getName() << "\n";
            for (Function::iterator bb = f.begin(); bb != f.end(); ++bb)
            {
                CFG[static_cast<BasicBlock *>(&*bb)] = std::set<BasicBlock *>();
                errs() << "\tBasic Block : " << bb->getName() << "\n";
                for (BasicBlock::iterator i = (*bb).begin(); i != (*bb).end(); ++i)
                {
                    if (BranchInst *bi = dyn_cast_or_null<BranchInst>(&*i))
                    {
                        for (int i = 0; i < bi->getNumSuccessors(); i++)
                        {
                            errs() << "\t\tjumps to : " << bi->getSuccessor(i)->getName() << "\n";
                            CFG[&*bb].insert(&*bi->getSuccessor(i));
                        }
                    }
                }
            }
            return true;
        }
    };
}

char RDA::ID = 0;
static RegisterPass<RDA> X("rda", "Reaching Definition Analysis Pass");