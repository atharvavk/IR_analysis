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
    struct RDA : public FunctionPass
    {
    public:
        static char ID;

    public:
        RDA() : FunctionPass(ID) {}
        ~RDA() {}

    public:
        bool runOnFunction(Function &f) override
        {
            errs() << "Function name: ";
            errs().write_escaped(f.getName()) << "\n";

            std::map<std::string, std::vector<unsigned>> allvar;
            std::map<llvm::Instruction *, unsigned> index;
            std::map<unsigned, std::string> names;
            unsigned indexCount = 0;

            //CFG Generation:
            //CFG is stored as map of bounding blocks to set of bounding blocks
            //it has edges to
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
            //CFG has been constructed

            // Collect all variable informations
            for (Function::iterator bb = f.begin(); bb != f.end(); ++bb)
            {
                for (BasicBlock::iterator i = (*bb).begin(); i != (*bb).end(); ++i)
                {
                    if (StoreInst *storeInst = static_cast<StoreInst *>(&*i))
                    {
                        if (storeInst->getPointerOperand()->hasName())
                        {
                            std::string name = storeInst->getPointerOperand()->getName().str();
                            names.insert(std::pair<unsigned, std::string>(indexCount, name));
                            if (allvar.find(name) != allvar.end())
                            {
                                std::vector<unsigned> temp;
                                allvar.insert(std::pair<std::string, std::vector<unsigned>>(name, std::vector<unsigned>()));
                            }
                            index.insert(std::pair<llvm::Instruction *, unsigned>(storeInst, indexCount));
                            allvar[name].push_back(indexCount);
                            ++indexCount;
                        }
                    }
                }
            }
            errs() << "Collected all variable informations.\n";
            for (auto vi = allvar.begin(); vi != allvar.end(); ++vi)
            {
                auto v = *vi;
                errs() << "VAR [" << v.first << "]: ";
                auto is = v.second;
                for (auto i = is.begin(); i != is.end(); ++i)
                    errs() << *i << " ";
                errs() << "\n";
            }
            errs() << "-------------------\n";

            // Collect GEN/KILL for Reaching Definition Analysis
            errs() << "~~~ GEN/KILL ~~~\n";

            std::map<llvm::BasicBlock *, std::vector<unsigned>> gens;
            std::map<llvm::BasicBlock *, std::vector<unsigned>> kills;
            std::vector<llvm::BasicBlock *> basicblocks;

            // Looping Basic blocks
            for (Function::iterator bb = f.begin(); bb != f.end(); ++bb)
            {
                BasicBlock &b = *bb;
                std::vector<unsigned> gen;
                std::vector<unsigned> kill;
                errs() << "Basic block name : " << bb->getName() << "\n";
                // Looping instructions
                for (BasicBlock::iterator i = b.begin(); i != b.end(); ++i)
                {
                    if (StoreInst *storeInst = static_cast<StoreInst *>(&*i))
                    {
                        if (allvar.find(storeInst->getPointerOperand()->getName().str()) == allvar.end())
                            continue;

                        unsigned id = index[storeInst];
                        // gen
                        if (std::find(gen.begin(), gen.end(), id) == gen.end())
                            gen.push_back(id);
                        std::sort(gen.begin(), gen.end());
                        // kill
                        std::vector<unsigned> others = allvar[storeInst->getPointerOperand()->getName().str()];
                        for (auto o = others.begin(); o != others.end(); ++o)
                        {
                            if (std::find(gen.begin(), gen.end(), *o) != gen.end())
                                continue;
                            if (std::find(kill.begin(), kill.end(), *o) == kill.end())
                                kill.push_back(*o);
                        }
                    }
                }

                // Delete from KILL if contained GEN
                for (auto g = gen.begin(); g != gen.end(); g++)
                {
                    for (auto k = kill.begin(); k != kill.end(); k++)
                    {
                        if (*k == *g)
                        {
                            kill.erase(k);
                            break;
                        }
                    }
                }
                std::sort(kill.begin(), kill.end());

                errs() << "GEN: ";
                for (auto g = gen.begin(); g != gen.end(); ++g)
                    errs() << *g << " ";
                errs() << "\n";
                errs() << "KILL: ";
                for (auto k = kill.begin(); k != kill.end(); ++k)
                    errs() << *k << " ";
                errs() << "\n";
                errs() << "============================================\n";

                gens.insert(std::pair<llvm::BasicBlock *, std::vector<unsigned>>(&b, gen));
                kills.insert(std::pair<llvm::BasicBlock *, std::vector<unsigned>>(&b, kill));

                basicblocks.push_back(&b);
            }

            errs() << "\n!!!!!!!!!!!!!Gen kill over !!!!!!!!!!!\n";

            // Collect IN/OUT for Reaching Definition Analysis
            errs() << "~~~ IN/OUT ~~~\n";

            std::map<llvm::BasicBlock *, std::set<unsigned>> ins;
            std::map<llvm::BasicBlock *, std::set<unsigned>> outs;

            for (auto bb = f.begin(); bb != f.end(); bb++)
            {
                outs[static_cast<BasicBlock *>(&*bb)] = std::set<unsigned>();
                for (unsigned v : gens[static_cast<BasicBlock *>(&*bb)])
                {
                    outs[static_cast<BasicBlock *>(&*bb)].insert(v);
                }
            }

            for (auto bb = f.begin(); bb != f.end(); bb++)
            {
                ins[static_cast<BasicBlock *>(&*bb)] = std::set<unsigned>();
                for (auto bb_ = f.begin(); bb_ != f.end(); bb_++)
                {
                    BasicBlock *P = static_cast<BasicBlock *>(&*bb_);
                    BasicBlock *B = static_cast<BasicBlock *>(&*bb);
                    if (P != B)
                    {
                        if (CFG[P].find(B) != CFG[P].end())
                        {
                            for (unsigned v : outs[P])
                            {
                                ins[B].insert(v);
                            }
                        }
                    }
                }
            }
            for (auto bb = f.begin(); bb != f.end(); bb++)
            {
                for (unsigned v : gens[static_cast<BasicBlock *>(&*bb)])
                {
                    outs[static_cast<BasicBlock *>(&*bb)].insert(v);
                }
                std::set<unsigned> setD;
                std::set_difference(ins[&*bb].begin(), ins[&*bb].end(), kills[&*bb].begin(), kills[&*bb].end(), std::inserter(setD, setD.begin()));
                for (unsigned v : setD)
                {
                    outs[&*bb].insert(v);
                }
            }
            for (auto bb = f.begin(); bb != f.end(); bb++)
            {
                ins[static_cast<BasicBlock *>(&*bb)] = std::set<unsigned>();
                for (auto bb_ = f.begin(); bb_ != f.end(); bb_++)
                {
                    BasicBlock *P = static_cast<BasicBlock *>(&*bb_);
                    BasicBlock *B = static_cast<BasicBlock *>(&*bb);
                    if (P != B)
                    {
                        if (CFG[P].find(B) != CFG[P].end())
                        {
                            for (unsigned v : outs[P])
                            {
                                ins[B].insert(v);
                            }
                        }
                    }
                }
            }

            for (auto bb = f.begin(); bb != f.end(); ++bb)
            {
                std::set<unsigned> in = ins[&*bb];
                std::set<unsigned> out = outs[&*bb];
                errs() << "Basic Block : " << bb->getName() << "\n";
                errs() << "IN: ";
                for (auto i = in.begin(); i != in.end(); ++i)
                    errs() << *i << " ";
                errs() << "\n";
                errs() << "OUT: ";
                for (auto o = out.begin(); o != out.end(); ++o)
                    errs() << *o << " ";
                errs() << "\n";
                errs() << "=========================================\n";
            }

            errs() << "\n\n************Dominator block*************\n\n";
            BasicBlock *r = &*f.begin();
            std::unordered_map<BasicBlock *, std::set<BasicBlock *>> dom_result;
            dom_result = dom(r, &CFG);
            for (auto bb = f.begin(); bb != f.end(); ++bb)
            {

                errs() << "{" << bb->getName() << "} : {";
                for (BasicBlock *b : dom_result[&*bb])
                {
                    errs() << b->getName() << " , ";
                }
                errs() << " }\n";
            }

            errs() << "\n\n******************Natural Loops*****************\n\n";
            std::vector<std::pair<BasicBlock *, BasicBlock *>> loops = backEdges(&CFG, dom_result);
            for (std::pair<BasicBlock *, BasicBlock *> backEdge : loops)
            {
                std::set<BasicBlock *> body = std::set<BasicBlock *>();
                body.insert(backEdge.second);
                std::stack<BasicBlock *> preds = std::stack<BasicBlock *>();
                preds.push(backEdge.first);
                while (!preds.empty())
                {
                    BasicBlock *b = preds.top();
                    preds.pop();
                    if (body.find(b) == body.end())
                    {
                        for (auto i = CFG.begin(); i != CFG.end(); i++)
                        {
                            if (i->first != b && i->second.find(b) != i->second.end())
                            {
                                preds.push(i->first);
                            }
                        }
                        body.insert(b);
                    }
                }
                for (BasicBlock *b : body)
                {
                    errs() << b->getName() << "\t";
                }
                errs() << "\n";
            }

            // COMPLETE
            return true;
        }

        std::vector<std::pair<BasicBlock *, BasicBlock *>> backEdges(std::unordered_map<BasicBlock *, std::set<BasicBlock *>> *CFG, std::unordered_map<BasicBlock *, std::set<BasicBlock *>> dom_result)
        {
            std::vector<std::pair<BasicBlock *, BasicBlock *>> retval;
            BasicBlock *m, *n;
            //edge from m to n
            //n dominates m
            for (auto i = CFG->begin(); i != CFG->end(); i++)
            {
                for (auto j = CFG->begin(); j != CFG->end(); j++)
                {
                    m = i->first;
                    n = j->first;
                    if (i->second.find(n) != i->second.end())
                    {
                        //edge from m to n exists
                        if (dom_result[m].find(n) != dom_result[m].end())
                        {
                            //n dominates m
                            retval.push_back(std::pair<BasicBlock *, BasicBlock *>(m, n));
                        }
                    }
                }
            }
            return retval;
        }

        std::unordered_map<BasicBlock *, std::set<BasicBlock *>> dom(BasicBlock *r, std::unordered_map<BasicBlock *, std::set<BasicBlock *>> *CFG)
        {
            std::unordered_map<BasicBlock *, std::set<BasicBlock *>> dominars;
            for (auto i = CFG->begin(); i != CFG->end(); i++)
            {
                if (i->first == r)
                {
                    dominars[r] = std::set<BasicBlock *>();
                    dominars[r].insert(r);
                }
                else
                {
                    dominars[i->first] = std::set<BasicBlock *>();
                    for (auto j = CFG->begin(); j != CFG->end(); j++)
                        dominars[i->first].insert(j->first);
                }
            }
            bool change = true;
            while (change)
            {
                change = false;
                for (auto i = CFG->begin(); i != CFG->end(); i++)
                {
                    std::set<BasicBlock *> new_set = std::set<BasicBlock *>();
                    std::set<BasicBlock *> temp_set = std::set<BasicBlock *>();
                    if (i->first != r)
                    {
                        bool predecessor_found = false;
                        for (auto p = CFG->begin(); p != CFG->end(); p++)
                        {
                            if (p != i)
                            {
                                //found predecessor
                                if (p->second.find(i->first) != p->second.end())
                                {
                                    if (!predecessor_found)
                                    {
                                        new_set.insert(i->first);
                                        for (BasicBlock *b : dominars[p->first])
                                        {
                                            new_set.insert(b);
                                        }
                                        predecessor_found = true;
                                    }
                                    else
                                    {
                                        temp_set = std::set<BasicBlock *>();
                                        std::set_intersection(new_set.begin(), new_set.end(), dominars[p->first].begin(), dominars[p->first].end(), std::inserter(temp_set, temp_set.begin()));
                                        new_set = temp_set;
                                        new_set.insert(i->first);
                                    }
                                }
                            }
                        }
                        if (new_set != dominars[i->first])
                        {
                            dominars[i->first] = new_set;
                            change = true;
                        }
                    }
                }
            }
            return dominars;
        }
    };
}

char RDA::ID = 0;
static RegisterPass<RDA> X("rda", "Reaching Definition Analysis Pass");
