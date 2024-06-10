#ifndef LIR_H
#define LIR_H
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "nlohmann_json.h"

using json = nlohmann::json;



struct Type {
    enum { INT, STRUCT, FN, PTR } tag;

    std::vector<Type*> funcParams;
    Type* funcRetType = nullptr;
    Type* ptrRef = nullptr;
    std::string structName;

    void printType() const {
        if (this->tag == INT) {
            std::cout << "confirmed INT type\n";
        }
        else if (this->tag == STRUCT) {
            std::cout << "confirmed STRUCT type\n";
            std::cout << this->structName << std::endl;
        }
        else if (this->tag == FN) {
            std::cout << "confirmed FN type\n";
            std::cout << "STARTING PARAMS\n";
            for (auto& elm : this->funcParams) {
                elm->printType();
            }

            if (this->funcRetType != nullptr) {
                std::cout << "STARTING RETURN\n";
                this->funcRetType->printType();
            }
        }
        else if (this->tag == PTR) {
            std::cout << "confirmed PTR type\n";
            this->ptrRef->printType();
        }
    }
};

struct Struct {
    std::string name;
    std::unordered_map<std::string, Type*> fields;

    void printStruct() const {
        std::cout << "Struct: " << name << std::endl;
        for (const auto& elm : this->fields) {
            std::cout << elm.first << ": ";
            elm.second->printType();
        }
    }
};

struct Structs {
    std::unordered_map<std::string, Struct> structs;

    void printStructs() const {
        for (const auto& [name, s] : this->structs) {
            s.printStruct();
        }
    }
};

struct Globals {
    std::unordered_map<std::string, Type*> variables;

    void printGlobals() const {
        for (const auto& [name, type] : this->variables) {
            std::cout << name << ": ";
            type->printType();
        }
    }
};

struct Extern {
    std::string name;
    Type* type;

    void printExtern() const {
        std::cout << "Extern: " << this->name << std::endl;
        this->type->printType();
    }
};

struct Externs {
    std::unordered_map<std::string, Extern> externs;

    void printExterns() const {
        for (const auto& [name, type] : this->externs) {
            type.printExtern();
        }
    }
};

enum ComparisonOp{
    EQUAL,
    NOTEQ,
    LT,
    LTE,
    GT,
    GTE
};

enum ArithmeticOp{
    ADD,
    SUB,
    MULT,
    DIV
};

struct Operand{
    enum { CONST, VAR } tag;
    struct Const{
        int32_t num;
    };
    struct Var{
        std::string id;
    };
    Const c;
    Var v;
};

struct Terminal {
    enum { BRANCH, CALL_DIRECT, CALL_INDIRECT, JUMP, RET } tag;

    struct Branch {
        Operand* guard;
        std::string tt;
        std::string ff;
    };

    struct CallDirect {
        std::string lhs;
        std::string callee;
        std::vector<Operand*> args;
        std::string next_bb;
    };

    struct CallIndirect {
        std::string lhs;
        std::string callee;
        std::vector<Operand*> args;
        std::string next_bb;
    };

    struct Jump {
        std::string next_bb;
    };

    struct Ret {
        Operand* op;
    };

    Branch branch;
    CallDirect callDirect;
    CallIndirect callIndirect;
    Jump jump;
    Ret ret;

    void printTerminal() const {
        switch (tag) {
            case BRANCH:
                std::cout << "Branch { guard: " << (branch.guard ? branch.guard->v.id : "")
                          << ", tt: " << branch.tt << ", ff: " << branch.ff << " }\n";
                break;
            case CALL_DIRECT:
                std::cout << "CallDirect { lhs: " << (callDirect.lhs != "" ? callDirect.lhs : "NO LHS")
                          << ", callee: " << callDirect.callee << ", args: [";
                for (const auto& op : callDirect.args) {
                    std::cout << (op ? op->v.id : "") << " ";
                }
                std::cout << "], next_bb: " << callDirect.next_bb << " }\n";
                break;
            case CALL_INDIRECT:
                std::cout << "CallIndirect { lhs: " << (callIndirect.lhs != "" ? callIndirect.lhs : "NO LHS")
                          << ", callee: " << callIndirect.callee << ", args: [";
                for (const auto& op : callIndirect.args) {
                    std::cout << (op ? op->v.id : "") << " ";
                }
                std::cout << "], next_bb: " << callIndirect.next_bb << " }\n";
                break;
            case JUMP:
                std::cout << "Jump { next_bb: " << jump.next_bb << " }\n";
                break;
            case RET:
                std::cout << "Ret { op: " << (ret.op ? ret.op->v.id : "") << " }\n";
                break;
        }
    }
};
struct LirInst {
    enum { ALLOC, ARITH, CALL_EXT, CMP, COPY, GEP, GFP, LOAD, STORE } tag;

    struct Alloc {
        std::string lhs;
        Operand num;
    };

    struct Arith {
        std::string lhs;
        ArithmeticOp aop;
        Operand left;
        Operand right;
    };

    struct CallExt {
        std::string lhs;  
        std::string callee;
        std::vector<Operand> args;
    };

    struct Cmp {
        std::string lhs;
        ComparisonOp aop;
        Operand left;
        Operand right;
    };

    struct Copy {
        std::string lhs;
        Operand op;
    };

    struct Gep {
        std::string lhs;
        std::string src;
        Operand idx;
    };

    struct Gfp {
        std::string lhs;
        std::string src;
        std::string field;
    };

    struct Load {
        std::string lhs;
        std::string src;
    };

    struct Store {
        std::string dst;
        Operand op;
    };

    
    Alloc alloc;
    Arith arith;
    CallExt callExt;
    Cmp cmp;
    Copy copy;
    Gep gep;
    Gfp gfp;
    Load load;
    Store store;
    

    void printLirInst() const {
      switch (tag) {
            case ALLOC:
                std::cout << "Alloc { lhs: " << alloc.lhs << ", num: ";
                if (alloc.num.tag == Operand::CONST) {
                    std::cout << alloc.num.c.num;
                } else {
                    std::cout << alloc.num.v.id;
                }
                std::cout << " }\n";
                break;
            case ARITH:
                std::cout << "Arith { lhs: " << arith.lhs << ", aop: " << arith.aop
                        << ", left: ";
                if (arith.left.tag == Operand::CONST) {
                    std::cout << arith.left.c.num;
                } else {
                    std::cout << arith.left.v.id;
                }
                std::cout << ", right: ";
                if (arith.right.tag == Operand::CONST) {
                    std::cout << arith.right.c.num;
                } else {
                    std::cout << arith.right.v.id;
                }
                std::cout << " }\n";
                break;
            case CALL_EXT:
                std::cout << "CallExt { lhs: ";
                if (callExt.lhs != "") {
                    std::cout << callExt.lhs;
                } else {
                    std::cout << "NO LHS";
                }
                std::cout << ", callee: " << callExt.callee << ", args: [";
                for (const auto& op : callExt.args) {
                    if (op.tag == Operand::CONST) {
                        std::cout << op.c.num;
                    } else {
                        std::cout << op.v.id;
                    }
                    std::cout << " ";
                }
                std::cout << "] }\n";
                break;
            case CMP:
                std::cout << "Cmp { lhs: " << cmp.lhs << ", aop: " << cmp.aop
                        << ", left: ";
                if (cmp.left.tag == Operand::CONST) {
                    std::cout << cmp.left.c.num;
                } else {
                    std::cout << cmp.left.v.id;
                }
                std::cout << ", right: ";
                if (cmp.right.tag == Operand::CONST) {
                    std::cout << cmp.right.c.num;
                } else {
                    std::cout << cmp.right.v.id;
                }
                std::cout << " }\n";
                break;
            case COPY:
                std::cout << "Copy { lhs: " << copy.lhs << ", op: ";
                if (copy.op.tag == Operand::CONST) {
                    std::cout << copy.op.c.num;
                } else {
                    std::cout << copy.op.v.id;
                }
                std::cout << " }\n";
                break;
            case GEP:
                std::cout << "Gep { lhs: " << gep.lhs << ", src: " << gep.src << ", idx: ";
                if (gep.idx.tag == Operand::CONST) {
                    std::cout << gep.idx.c.num;
                } else {
                    std::cout << gep.idx.v.id;
                }
                std::cout << " }\n";
                break;
            case GFP:
                std::cout << "Gfp { lhs: " << gfp.lhs << ", src: " << gfp.src << ", field: " << gfp.field << " }\n";
                break;
            case LOAD:
                std::cout << "Load { lhs: " << load.lhs << ", src: " << load.src << " }\n";
                break;
            case STORE:
                std::cout << "Store { dst: " << store.dst << ", op: ";
                if (store.op.tag == Operand::CONST) {
                    std::cout << store.op.c.num;
                } else {
                    std::cout << store.op.v.id;
                }
                std::cout << " }\n";
                break;
        }

    }
};

struct BasicBlock{
    std::string label;
    std::vector<LirInst> insts;
    Terminal term;

    void printBasicBlock() const {
        std::cout << "BasicBlock: " << this->label << std::endl;
        for (const auto& inst : this->insts) {
            inst.printLirInst();
        }
        this->term.printTerminal();
    }
};

struct Function{
    struct Param{
        std::string name;
        Type* type;
    };
    std::string name;
    std::vector<Param> params;
    Type* rettyp;
    std::unordered_map<std::string, Type*> locals;
    std::unordered_map<std::string, BasicBlock> body;

    void printFunction() const {
        std::cout << "Function: " << this->name << std::endl;
        for (const auto& param : this->params) {
            std::cout << "Param: " << param.name << ", ";
            param.type->printType();
        }
        for (const auto& [name, bb] : this->body) {
            bb.printBasicBlock();
        }
        std::cout << "Rettyp: ";
        if(this->rettyp) {
            this->rettyp->printType();
        } 
        std::cout << "Locals: " << std::endl;
        for (const auto& [name, type] : this->locals) {
            std::cout << name << ": ";
            type->printType();
        }
    }
};
struct Functions {
    std::unordered_map<std::string, Function> functions;

    void printFunctions() const {
        for (const auto& [name, func] : this->functions) {
            func.printFunction();
        }
    }
};

Type* parseType(const json& typeJson);

Type* parseTypeHelper(const json& typeJson) {
    if (typeJson.is_string() && typeJson == "Int") {
        Type* type = new Type();
        type->tag = Type::INT;
        return type;
    }
    if (typeJson.contains("Struct")) {
        Type* type = new Type();
        type->tag = Type::STRUCT;
        type->structName = typeJson["Struct"];
        return type;
    }
    if (typeJson.contains("Fn")) {
        Type* type = new Type();
        type->tag = Type::FN;
        for (const auto& param : typeJson["Fn"]["param_ty"]) {
            type->funcParams.push_back(parseType(param));
        }
        type->funcRetType = parseType(typeJson["Fn"]["ret_ty"]);
        return type;
    }
    if (typeJson.contains("Ptr")) {
        Type* type = new Type();
        type->tag = Type::PTR;
        type->ptrRef = parseType(typeJson["Ptr"]);
        return type;
    }
    return nullptr;
}

Type* parseType(const json& typeJson) {
    if (typeJson.is_string()) {
        return parseTypeHelper(typeJson);
    } else if (typeJson.is_object()) {
        return parseTypeHelper(typeJson);
    }
    return nullptr;
}

Globals parseGlobals(const json& globalsJson) {
    Globals globals;
    for (const auto& varJson : globalsJson) {
        std::string name = varJson["name"];
        Type* type = parseType(varJson["typ"]);
        globals.variables[name] = type;
    }
    return globals;
}

Structs parseStructs(const json& structsJson) {
    Structs structs;
    for (auto it = structsJson.begin(); it != structsJson.end(); ++it) {
        Struct st;
        st.name = it.key();
        for (const auto& field : it.value()) {
            std::string fieldName = field["name"];
            Type* fieldType = parseType(field["typ"]);
            st.fields[fieldName] = fieldType;
        }
        structs.structs[st.name] = st;
    }
    return structs;
}

Externs parseExterns(const json& externsJson) {
    Externs externs;
    for (auto it = externsJson.begin(); it != externsJson.end(); ++it) {
        Extern ext;
        ext.name = it.key();
        ext.type = parseType(it.value());
        externs.externs[ext.name] = ext;
    }
    return externs;
}
// Function to parse Operand from JSON
Operand parseOperand(const json& operandJson) {
    Operand operand;
    if (operandJson.is_number_integer()) {
        operand.tag = Operand::CONST;
        operand.c.num = operandJson.get<int32_t>();
    } else if (operandJson.is_string()) {
        operand.tag = Operand::VAR;
        operand.v.id = operandJson.get<std::string>();
    }
    return operand;
}

// Function to parse LirInst from JSON
LirInst parseLirInst(const json& instJson) {
    LirInst inst;
    std::string tag = instJson["tag"];
    
    if (tag == "Alloc") {
        inst.tag = LirInst::ALLOC;
        inst.alloc.lhs = instJson["lhs"];
        inst.alloc.num = parseOperand(instJson["num"]);
    } else if (tag == "Arith") {
        inst.tag = LirInst::ARITH;
        inst.arith.lhs = instJson["lhs"];
        inst.arith.aop = static_cast<ArithmeticOp>(instJson["aop"].get<int>());
        inst.arith.left = parseOperand(instJson["left"]);
        inst.arith.right = parseOperand(instJson["right"]);
    } else if (tag == "CallExt") {
        inst.tag = LirInst::CALL_EXT;
        inst.callExt.lhs = instJson["lhs"];
        inst.callExt.callee = instJson["callee"];
        for (const auto& arg : instJson["args"]) {
            inst.callExt.args.push_back(parseOperand(arg));
        }
    } else if (tag == "Cmp") {
        inst.tag = LirInst::CMP;
        inst.cmp.lhs = instJson["lhs"];
        inst.cmp.aop = static_cast<ComparisonOp>(instJson["aop"].get<int>());
        inst.cmp.left = parseOperand(instJson["left"]);
        inst.cmp.right = parseOperand(instJson["right"]);
    } else if (tag == "Copy") {
        inst.tag = LirInst::COPY;
        inst.copy.lhs = instJson["lhs"];
        inst.copy.op = parseOperand(instJson["op"]);
    } else if (tag == "Gep") {
        inst.tag = LirInst::GEP;
        inst.gep.lhs = instJson["lhs"];
        inst.gep.src = instJson["src"];
        inst.gep.idx = parseOperand(instJson["idx"]);
    } else if (tag == "Gfp") {
        inst.tag = LirInst::GFP;
        inst.gfp.lhs = instJson["lhs"];
        inst.gfp.src = instJson["src"];
        inst.gfp.field = instJson["field"];
    } else if (tag == "Load") {
        inst.tag = LirInst::LOAD;
        inst.load.lhs = instJson["lhs"];
        inst.load.src = instJson["src"];
    } else if (tag == "Store") {
        inst.tag = LirInst::STORE;
        inst.store.dst = instJson["dst"];
        inst.store.op = parseOperand(instJson["op"]);
    }
    return inst;
}

// Function to parse Terminal from JSON
Terminal parseTerminal(const json& termJson) {
    Terminal term;
    std::string tag = termJson["tag"];
    
    if (tag == "Branch") {
        term.tag = Terminal::BRANCH;
        term.branch.guard = new Operand(parseOperand(termJson["guard"]));
        term.branch.tt = termJson["tt"];
        term.branch.ff = termJson["ff"];
    } else if (tag == "CallDirect") {
        term.tag = Terminal::CALL_DIRECT;
        term.callDirect.lhs = termJson["lhs"];
        term.callDirect.callee = termJson["callee"];
        for (const auto& arg : termJson["args"]) {
            term.callDirect.args.push_back(new Operand(parseOperand(arg)));
        }
        term.callDirect.next_bb = termJson["next_bb"];
    } else if (tag == "CallIndirect") {
        term.tag = Terminal::CALL_INDIRECT;
        term.callIndirect.lhs = termJson["lhs"];
        term.callIndirect.callee = termJson["callee"];
        for (const auto& arg : termJson["args"]) {
            term.callIndirect.args.push_back(new Operand(parseOperand(arg)));
        }
        term.callIndirect.next_bb = termJson["next_bb"];
    } else if (tag == "Jump") {
        term.tag = Terminal::JUMP;
        term.jump.next_bb = termJson["next_bb"];
    } else if (tag == "Ret") {
        term.tag = Terminal::RET;
        term.ret.op = new Operand(parseOperand(termJson["op"]));
    }
    return term;
}

// Function to parse BasicBlock from JSON
BasicBlock parseBasicBlock(const json& bbJson) {
    BasicBlock bb;
    bb.label = bbJson["id"];
    for (const auto& instJson : bbJson["insts"]) {
        bb.insts.push_back(parseLirInst(instJson));
    }
    bb.term = parseTerminal(bbJson["term"]);
    return bb;
}

Functions parseFunctions(const json& functionsJson) {
    Functions functions;
    for (auto it = functionsJson.begin(); it != functionsJson.end(); ++it) {
        std::cout << "Parsing function: " << it.key() << std::endl; // Debug print
        Function fn;
        fn.name = it.key();
        
        std::cout << "Parsing parameters..." << std::endl; // Debug print
        for (const auto& paramJson : it.value()["params"]) {
            std::cout << "Param: " << paramJson.dump() << std::endl; // Debug print
            Function::Param param;
            param.name = paramJson["name"];
            param.type = parseType(paramJson["typ"]);
            fn.params.push_back(param);
        }

        std::cout << "Parsing return type..." << std::endl; // Debug print
        fn.rettyp = parseType(it.value()["ret_ty"]);

        std::cout << "Parsing locals..." << std::endl; // Debug print
        for (const auto& localJson : it.value()["locals"]) {
            std::string localName = localJson["name"];
            std::cout << "Local: " << localName << " - " << localJson.dump() << std::endl; // Debug print
            Type* localType = parseType(localJson["typ"]);
            fn.locals[localName] = localType;
        }

        std::cout << "Parsing body..." << std::endl; // Debug print
        for (const auto& bbJson : it.value()["body"]) {
            std::cout << "BasicBlock: " << bbJson.dump() << std::endl; // Debug print
            BasicBlock bb = parseBasicBlock(bbJson);
            fn.body[bb.label] = bb;
        }

        functions.functions[fn.name] = fn;
    }
    return functions;
}

#endif