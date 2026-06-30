#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace darix {

using Instructions = std::vector<uint8_t>;

enum class Opcode : uint8_t {
    OpConstant,
    OpAdd,
    OpSub,
    OpMul,
    OpDiv,
    OpMod,
    OpEqual,
    OpNotEqual,
    OpGreaterThan,
    OpLessThan,
    OpGreaterEqual,
    OpLessEqual,
    OpMinus,
    OpBang,
    OpTrue,
    OpFalse,
    OpNull,
    OpPop,
    OpJump,
    OpJumpNotTruthy,
    OpSetGlobal,
    OpGetGlobal,
    OpPrint,
    OpNop,
    OpArray,
    OpIndex,
    OpSetIndex,
    OpStringConcat,
    OpLen,
    OpType,
    OpCall,
    OpReturnValue,
    OpReturn,
    OpGetLocal,
    OpSetLocal,
    OpSwap,
};

struct Definition {
    std::string name;
    std::vector<int> operandWidths;
};

const Definition* Lookup(Opcode op);
Instructions Make(Opcode op, const std::vector<int>& operands = {});
std::pair<std::vector<int>, int> ReadOperands(const Definition* def, const uint8_t* ins, size_t length);
std::string Disassemble(const Instructions& ins);

} // namespace darix
