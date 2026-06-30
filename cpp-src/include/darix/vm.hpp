#pragma once

#include "darix/code.hpp"
#include "darix/compiler.hpp"
#include "darix/object.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace darix {

constexpr int StackSize = 2048;
constexpr int InitialGlobs = 1024;

struct HotPath;

class VM {
public:
    explicit VM(std::shared_ptr<Bytecode> bc);

    ObjectPtr run();
    void setInstructionBudget(int n);
    void enableJIT(bool enabled);
    void enableProfiling(bool enabled);

private:
    ObjectPtr push(ObjectPtr obj);
    ObjectPtr pop();
    std::pair<ObjectPtr, ObjectPtr> popChecked();
    std::tuple<ObjectPtr, ObjectPtr, ObjectPtr> popTwo();
    std::tuple<ObjectPtr, ObjectPtr, ObjectPtr, ObjectPtr> popThree();
    ObjectPtr pushChecked(ObjectPtr obj);

    ObjectPtr binaryOp(Opcode op);
    ObjectPtr execBinary(Opcode op, ObjectPtr left, ObjectPtr right);
    ObjectPtr compareOp(Opcode op);
    ObjectPtr execCompare(Opcode op, ObjectPtr left, ObjectPtr right);
    ObjectPtr execMinus(ObjectPtr operand);
    ObjectPtr execIndex(ObjectPtr left, ObjectPtr index);
    ObjectPtr execSetIndex(ObjectPtr target, ObjectPtr index, ObjectPtr value);
    ObjectPtr execLen(ObjectPtr obj);
    ObjectPtr execType(ObjectPtr obj);

    ObjectPtr opPrint(int argc);
    ObjectPtr opArray(int numElements);
    ObjectPtr opStringConcat(int n);
    ObjectPtr opSwap();

    ObjectPtr runCompiledFunction(std::shared_ptr<CompiledFunction> fn, const std::vector<ObjectPtr>& args);

    void setGlobal(int idx, ObjectPtr val);
    ObjectPtr getGlobal(int idx);

    ObjectPtr errorWithLoc(const std::string& msg);
    std::shared_ptr<StackTrace> buildStackTrace();
    std::shared_ptr<StackFrame> currentFrame();
    void lookupDebug(int ip, std::string& file, int& line, int& col, std::string& fn);

    std::vector<ObjectPtr> constants_;
    std::vector<ObjectPtr> globals_;
    std::vector<ObjectPtr> stack_;
    int sp_ = 0;
    int ip_ = 0;
    Instructions instructions_;
    std::string bcMagic_;
    std::string bcVersion_;
    DebugInfo debug_;
    int instrBudget_ = 0;

    // JIT
    std::shared_ptr<HotPath> jitGetCompiledPath(int ip);
    bool jitRecordExecution(int ip);
    std::shared_ptr<HotPath> jitShouldCompile(int ip);
    void jitCompileHotPath(std::shared_ptr<HotPath> hp);
    ObjectPtr jitExecuteCompiledPath(std::shared_ptr<HotPath> hp);

    // Profiling
    bool profiling_ = false;
    uint64_t opCounts_[256] = {};

    // JIT data
    std::unordered_map<int, int> execCounts_;
    std::unordered_map<int, std::shared_ptr<HotPath>> hotPaths_;
    int jitThreshold_ = 100;
};

} // namespace darix
