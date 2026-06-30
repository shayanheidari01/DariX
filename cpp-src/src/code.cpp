#include "darix/code.hpp"
#include <sstream>
#include <stdexcept>

namespace darix {

static Definition definitions[] = {
    /* OpConstant       */ {"OpConstant",       {2}},
    /* OpAdd            */ {"OpAdd",            {}},
    /* OpSub            */ {"OpSub",            {}},
    /* OpMul            */ {"OpMul",            {}},
    /* OpDiv            */ {"OpDiv",            {}},
    /* OpMod            */ {"OpMod",            {}},
    /* OpEqual          */ {"OpEqual",          {}},
    /* OpNotEqual       */ {"OpNotEqual",       {}},
    /* OpGreaterThan    */ {"OpGreaterThan",    {}},
    /* OpLessThan       */ {"OpLessThan",       {}},
    /* OpGreaterEqual   */ {"OpGreaterEqual",   {}},
    /* OpLessEqual      */ {"OpLessEqual",      {}},
    /* OpMinus          */ {"OpMinus",          {}},
    /* OpBang           */ {"OpBang",           {}},
    /* OpTrue           */ {"OpTrue",           {}},
    /* OpFalse          */ {"OpFalse",          {}},
    /* OpNull           */ {"OpNull",           {}},
    /* OpPop            */ {"OpPop",            {}},
    /* OpJump           */ {"OpJump",           {2}},
    /* OpJumpNotTruthy  */ {"OpJumpNotTruthy",  {2}},
    /* OpSetGlobal      */ {"OpSetGlobal",      {2}},
    /* OpGetGlobal      */ {"OpGetGlobal",      {2}},
    /* OpPrint          */ {"OpPrint",          {2}},
    /* OpNop            */ {"OpNop",            {}},
    /* OpArray          */ {"OpArray",          {2}},
    /* OpIndex          */ {"OpIndex",          {}},
    /* OpSetIndex       */ {"OpSetIndex",       {}},
    /* OpStringConcat   */ {"OpStringConcat",   {2}},
    /* OpLen            */ {"OpLen",            {}},
    /* OpType           */ {"OpType",           {}},
    /* OpCall           */ {"OpCall",           {2}},
    /* OpReturnValue    */ {"OpReturnValue",    {}},
    /* OpReturn         */ {"OpReturn",         {}},
    /* OpGetLocal       */ {"OpGetLocal",       {2}},
    /* OpSetLocal       */ {"OpSetLocal",       {2}},
    /* OpSwap           */ {"OpSwap",           {}},
};

const Definition* Lookup(Opcode op) {
    auto idx = static_cast<int>(op);
    if (idx < 0 || idx >= static_cast<int>(sizeof(definitions) / sizeof(definitions[0]))) {
        return nullptr;
    }
    return &definitions[idx];
}

static void PutUint16(uint8_t* buf, uint16_t val) {
    buf[0] = static_cast<uint8_t>((val >> 8) & 0xFF);
    buf[1] = static_cast<uint8_t>(val & 0xFF);
}

static uint16_t ReadUint16(const uint8_t* buf) {
    return static_cast<uint16_t>(buf[0]) << 8 | static_cast<uint16_t>(buf[1]);
}

Instructions Make(Opcode op, const std::vector<int>& operands) {
    const Definition* def = Lookup(op);
    if (!def) {
        return {};
    }

    size_t instrLen = 1;
    for (int w : def->operandWidths) {
        instrLen += w;
    }

    Instructions ins(instrLen, 0);
    ins[0] = static_cast<uint8_t>(op);

    size_t offset = 1;
    for (size_t i = 0; i < def->operandWidths.size(); i++) {
        int width = def->operandWidths[i];
        int operand = (i < operands.size()) ? operands[i] : 0;
        switch (width) {
            case 2:
                PutUint16(&ins[offset], static_cast<uint16_t>(operand));
                break;
            default:
                throw std::runtime_error("unsupported operand width");
        }
        offset += width;
    }
    return ins;
}

std::pair<std::vector<int>, int> ReadOperands(const Definition* def, const uint8_t* ins, size_t length) {
    std::vector<int> operands(def->operandWidths.size(), 0);
    size_t offset = 0;
    for (size_t i = 0; i < def->operandWidths.size(); i++) {
        int width = def->operandWidths[i];
        switch (width) {
            case 2:
                if (offset + 2 <= length) {
                    operands[i] = static_cast<int>(ReadUint16(ins + offset));
                }
                break;
            default:
                throw std::runtime_error("unsupported operand width");
        }
        offset += width;
    }
    return {operands, static_cast<int>(offset)};
}

std::string Disassemble(const Instructions& ins) {
    std::ostringstream out;
    size_t offset = 0;
    while (offset < ins.size()) {
        uint8_t opByte = ins[offset];
        Opcode op = static_cast<Opcode>(opByte);
        const Definition* def = Lookup(op);

        if (!def) {
            out << "ERROR: unknown opcode " << static_cast<int>(opByte) << "\n";
            offset++;
            continue;
        }

        // Format instruction
        auto [operands, read] = ReadOperands(def, ins.data() + offset + 1, ins.size() - offset - 1);

        char buf[32];
        std::snprintf(buf, sizeof(buf), "%04d ", static_cast<int>(offset));
        out << buf;

        out << def->name;
        if (!operands.empty()) {
            out << " ";
            for (size_t i = 0; i < operands.size(); i++) {
                if (i > 0) out << " ";
                out << operands[i];
            }
        }
        out << "\n";

        offset += 1 + read;
    }
    return out.str();
}

} // namespace darix
