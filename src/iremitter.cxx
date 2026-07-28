#include "iremitter.hxx"
#include "ast.hxx"

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Pass.h>

#include <list>
#include <system_error>
#include <utility>
#include <vector>

namespace {

char typeOfName(std::string_view name)
{
    if( name.back() == '?' )
        return 'B';

    if( name.back() == '$' )
        return 'T';

    return 'N';
}

char exprType(const basic::Expression::Ptr& e)
{
    using namespace basic;
    switch( e->kind ) {
        case NodeKind::Boolean: return 'B';
        case NodeKind::Number:  return 'N';
        case NodeKind::Text:    return 'T';
        case NodeKind::Variable: {
            auto v = std::static_pointer_cast<Variable>(e);
            return typeOfName(v->_name);
        }
        case NodeKind::Apply: {
            auto a = std::static_pointer_cast<Apply>(e);
            return typeOfName(a->_callee);
        }
        case NodeKind::Unary: {
            auto u = std::static_pointer_cast<Unary>(e);
            return u->_operation == Operation::Not ? 'B' : 'N';
        }
        case NodeKind::Binary: {
            auto b = std::static_pointer_cast<Binary>(e);
            auto l = exprType(b->_left);
            auto r = exprType(b->_right);
            if( l != r ) return 'V';
            if( b->_operation >= Operation::Eq && b->_operation <= Operation::Le )
                return 'B';
            if( b->_operation == Operation::Conc )
                return 'T';
            if( b->_operation == Operation::And || b->_operation == Operation::Or )
                return 'B';
            return l;
        }
        default: return 'V';
    }
}

} // anonymous namespace

namespace basic {

IrEmitter::IrEmitter(llvm::LLVMContext& cx, llvm::Module& md)
    : context{cx}, moduler{md}, builder{context}
{
}

bool IrEmitter::emitFor(Program::Ptr prog)
{
    try {
        visit(prog);
        llvm::verifyModule(moduler);
    }
    catch(...) {
        return false;
    }

    return true;
}

void IrEmitter::visit(Program::Ptr prog)
{
    prepareLibrary();
    declareSubroutines(prog);
    defineSubroutines(prog);
    createEntryPoint();
}

void IrEmitter::visit(Subroutine::Ptr subr)
{
    auto* func = moduler.getFunction(subr->_name);

    if( nullptr == func )
        return;

    auto* start = llvm::BasicBlock::Create(context, "start", func);
    builder.SetInsertPoint(start);

    for( auto& arg : func->args() ) {
        int ix = arg.getArgNo();
        arg.setName(subr->_parameters[ix]);
    }

    varAddresses.clear();

    std::list<llvm::Value*> localTexts;

    for( auto& arg : func->args() )
        if( arg.getType()->isPointerTy() ) {
            auto parval = createLibraryFuncCall("text_clone", { &arg });
            builder.CreateStore(parval, varAddresses[arg.getName().str()]);
            localTexts.remove(varAddresses[arg.getName().str()]);
        }
        else
            builder.CreateStore(&arg, varAddresses[arg.getName().str()]);

    auto one = builder.getInt64(1);
    for( auto* vp : localTexts ) {
        auto deva = createLibraryFuncCall("malloc", { one });
        builder.CreateStore(deva, vp);
    }

    visit(subr->_body);

    if( func->getReturnType()->isVoidTy() )
        builder.CreateRetVoid();
    else {
        auto rv = builder.CreateLoad(func->getReturnType(), varAddresses[subr->_name]);
        builder.CreateRet(rv);
    }

    llvm::verifyFunction(*func);
}

void IrEmitter::visit(Sequence::Ptr seq)
{
    for( auto& st : seq->_items )
        visit(st);
}

void IrEmitter::visit(Dim::Ptr dim)
{
}

void IrEmitter::visit(Let::Ptr let)
{
    visit(let->_expr);
    auto* val = _result;
    auto* addr = varAddresses[let->_variable->_name];

    auto pt = typeOfName(let->_variable->_name);
    if( pt == 'T' ) {
        auto* dera = builder.CreateLoad(TextualTy, addr);
        createLibraryFuncCall("free", {dera});
        if( !createsTempText(let->_expr) )
            val = createLibraryFuncCall("text_clone", { val });
    }
    else if( pt == 'B' ) {
        val = builder.CreateZExt(val, builder.getInt8Ty());
    }

    builder.CreateStore(val, addr);
}

void IrEmitter::visit(Input::Ptr inp)
{
    auto pt = typeOfName(inp->_variable->_name);
    std::string_view funcName;
    if( pt == 'B' )
        funcName = "bool_input";
    if( pt == 'N' )
        funcName = "number_input";
    else if( pt == 'T' )
        funcName = "text_input";

    auto* prompt = builder.CreateGlobalString("? ", "prompt");
    auto* val = createLibraryFuncCall(funcName, {prompt});
    builder.CreateStore(val, varAddresses[inp->_variable->_name]);
}

void IrEmitter::visit(Print::Ptr pri)
{
    visit(pri->_expr);
    auto* expr = _result;

    auto et = exprType(pri->_expr);
    if( et == 'T' ) {
        createLibraryFuncCall("text_print", {expr});
        if( createsTempText(pri->_expr) )
            createLibraryFuncCall("free", {expr});
    }
    else if( et == 'N' )
        createLibraryFuncCall("number_print", {expr});
}

void IrEmitter::visit(If::Ptr sif)
{
    auto* func = builder.GetInsertBlock()->getParent();

    auto* endIf = llvm::BasicBlock::Create(context, "", func);

    auto* first = llvm::BasicBlock::Create(context, "", func, endIf);
    setCurrentBlock(func, first);

    Statement::Ptr sp = sif;
    while( auto ifp = std::dynamic_pointer_cast<If>(sp) ) {
        auto* thenBlock = llvm::BasicBlock::Create(context, {}, func, endIf);
        auto* elseBlock = llvm::BasicBlock::Create(context, {}, func, endIf);

        visit(ifp->_condition);
        auto* cnd = _result;

        builder.CreateCondBr(cnd, thenBlock, elseBlock);

        setCurrentBlock(func, thenBlock);

        visit(ifp->_decision);
        builder.CreateBr(endIf);

        setCurrentBlock(func, elseBlock);

        sp = ifp->_alternative;
    }

    if( sp->kind != NodeKind::Empty )
        visit(sp);

    setCurrentBlock(func, endIf);
}

void IrEmitter::visit(While::Ptr swhi)
{
    auto* func = builder.GetInsertBlock()->getParent();

    auto* condBlock = llvm::BasicBlock::Create(context, {}, func);
    auto* bodyBlock = llvm::BasicBlock::Create(context, {}, func);
    auto* endWhile = llvm::BasicBlock::Create(context, {}, func);

    setCurrentBlock(func, condBlock);

    visit(swhi->_condition);
    auto* condEx = _result;
    builder.CreateCondBr(condEx, bodyBlock, endWhile);

    setCurrentBlock(func, bodyBlock);

    visit(swhi->_body);
    builder.CreateBr(condBlock);

    setCurrentBlock(func, endWhile);
}

void IrEmitter::visit(For::Ptr sfor)
{
    auto* func = builder.GetInsertBlock()->getParent();

    auto* condBlock = llvm::BasicBlock::Create(context, "", func);
    auto* bodyBlock = llvm::BasicBlock::Create(context, "", func);
    auto* endFor = llvm::BasicBlock::Create(context, "", func);

    auto* param = varAddresses[sfor->_parameter->_name];
    visit(sfor->_begin);
    auto* init = _result;
    builder.CreateStore(init, param);
    visit(sfor->_end);
    auto* finish = _result;
    auto* step = llvm::ConstantFP::get(NumericTy, sfor->_step->_value);

    setCurrentBlock(func, condBlock);

    auto* parVal = builder.CreateLoad(NumericTy, param);
    llvm::Value* coex = nullptr;
    if( sfor->_step->_value > 0.0 )
        coex = builder.CreateFCmpOLT(parVal, finish);
    else if( sfor->_step->_value < 0.0 )
        coex = builder.CreateFCmpOGT(parVal, finish);
    builder.CreateCondBr(coex, bodyBlock, endFor);

    setCurrentBlock(func, bodyBlock);

    visit(sfor->_body);

    auto* parval = builder.CreateLoad(NumericTy, param);
    auto* nwpv = builder.CreateFAdd(parval, step);
    builder.CreateStore(nwpv, param);

    builder.CreateBr(condBlock);

    setCurrentBlock(func, endFor);
}

void IrEmitter::visit(Call::Ptr cal)
{
    visit(cal->_subrCall);
}

void IrEmitter::visit(Text::Ptr txt)
{
    if( const auto sri = globalTexts.find(txt->_value); sri != globalTexts.end() )
        _result = sri->second;
    else {
        auto* strp = builder.CreateGlobalString(txt->_value, "g_str");
        globalTexts[txt->_value] = strp;
        _result = strp;
    }
}

void IrEmitter::visit(Number::Ptr num)
{
    _result = llvm::ConstantFP::get(NumericTy, num->_value);
}

void IrEmitter::visit(Boolean::Ptr bv)
{
    _result = llvm::ConstantInt::getBool(BooleanTy, bv->_value);
}

void IrEmitter::visit(Variable::Ptr var)
{
    auto* vaddr = varAddresses[var->_name];

    if( typeOfName(var->_name) == 'B' ) {
        llvm::Type* ByteType = builder.getInt8Ty();
        llvm::LoadInst* res = builder.CreateLoad(ByteType, vaddr, var->_name);
        _result = llvm::dyn_cast<llvm::UnaryInstruction>(builder.CreateTrunc(res, BooleanTy));
    }
    else
        _result = builder.CreateLoad(llvmType(var->_name), vaddr, var->_name);
}

void IrEmitter::visit(Array::Ptr)
{
}

void IrEmitter::visit(Apply::Ptr apy)
{
    llvm::SmallVector<llvm::Value*> argus, temps;
    for( const auto& ai : apy->_arguments ) {
        visit(ai);
        auto ap = _result;
        argus.push_back(ap);
        if( createsTempText(ai) )
            temps.push_back(ap);
    }

    auto callee = userFunction(apy->_callee);
    auto* calv = builder.CreateCall(callee, argus);

    for( auto* ai : temps )
        if( ai->getType()->isPointerTy() )
            createLibraryFuncCall("free", { ai });

    _result = calv;
}

void IrEmitter::visit(Binary::Ptr bin)
{
    const bool textuals = exprType(bin->_left) == 'T'
                       && exprType(bin->_right) == 'T';
    const bool numerics = exprType(bin->_left) == 'N'
                       && exprType(bin->_right) == 'N';
    const bool booleans = exprType(bin->_left) == 'B'
                       && exprType(bin->_right) == 'B';

    visit(bin->_left);
    auto* lhs = _result;
    visit(bin->_right);
    auto* rhs = _result;

    llvm::Value* ret = nullptr;
    switch( bin->_operation ) {
        case Operation::Add:
            ret = builder.CreateFAdd(lhs, rhs, "add");
            break;
        case Operation::Sub:
            ret = builder.CreateFSub(lhs, rhs, "sub");
            break;
        case Operation::Mul:
            ret = builder.CreateFMul(lhs, rhs, "mul");
            break;
        case Operation::Div:
            ret = builder.CreateFDiv(lhs, rhs, "div");
            break;
        case Operation::Mod:
            ret = builder.CreateFRem(lhs, rhs, "rem");
            break;
        case Operation::Pow:
            ret = createLibraryFuncCall("pow", {lhs, rhs});
            break;

        case Operation::Eq:
            if( textuals )
                ret = createLibraryFuncCall("text_eq", {lhs, rhs});
            else if( numerics )
                ret = builder.CreateFCmpOEQ(lhs, rhs, "eq");
            else if( booleans )
                ret = builder.CreateICmpEQ(lhs, rhs, "eq");
            break;
        case Operation::Ne:
            if( textuals )
                ret = createLibraryFuncCall("text_ne", {lhs, rhs});
            else if( numerics )
                ret = builder.CreateFCmpONE(lhs, rhs, "ne");
            else if( booleans )
                ret = builder.CreateICmpNE(lhs, rhs, "ne");
            break;
        case Operation::Gt:
            if( textuals )
                ret = createLibraryFuncCall("text_gt", {lhs, rhs});
            else if( numerics )
                ret = builder.CreateFCmpOGT(lhs, rhs, "gt");
            break;
        case Operation::Ge:
            if( textuals )
                ret = createLibraryFuncCall("text_ge", {lhs, rhs});
            else if( numerics )
                ret = builder.CreateFCmpOGE(lhs, rhs, "ge");
            break;
        case Operation::Lt:
            if( textuals )
                ret = createLibraryFuncCall("text_lt", {lhs, rhs});
            else if( numerics )
                ret = builder.CreateFCmpOLT(lhs, rhs, "lt");
            break;
        case Operation::Le:
            if( textuals )
                ret = createLibraryFuncCall("text_le", {lhs, rhs});
            else if( numerics )
                ret = builder.CreateFCmpOLE(lhs, rhs, "le");
            break;

        case Operation::And:
            ret = builder.CreateAnd(lhs, rhs, "and");
            break;
        case Operation::Or:
            ret = builder.CreateOr(lhs, rhs, "or");
            break;

        case Operation::Conc:
            ret = createLibraryFuncCall("text_conc", {lhs, rhs});
            break;
        default:
            break;
    }

    _result = ret;
}

void IrEmitter::visit(Unary::Ptr un)
{
    visit(un->_operand);
    auto* val = _result;

    if( Operation::Sub == un->_operation )
        _result = builder.CreateFNeg(val, "neg");
    else if( Operation::Not == un->_operation )
        _result = builder.CreateNot(val);
    else
        _result = val;
}

void IrEmitter::setCurrentBlock(llvm::Function* fun, llvm::BasicBlock* bl)
{
    if( auto* ib = builder.GetInsertBlock(); nullptr != ib && nullptr == ib->getTerminator() )
        builder.CreateBr(bl);

    builder.ClearInsertionPoint();
    fun->insert(fun->end(), bl);
    builder.SetInsertPoint(bl);
}

void IrEmitter::prepareLibrary()
{
    declareLibraryFunction("text_clone", "T(T)");
    declareLibraryFunction("text_input", "T(T)");
    declareLibraryFunction("text_print", "V(T)");
    declareLibraryFunction("text_conc", "T(TT)");
    declareLibraryFunction("text_mid", "T(TNN)");
    declareLibraryFunction("text_str", "T(N)");
    declareLibraryFunction("text_eq", "B(TT)");
    declareLibraryFunction("text_ne", "B(TT)");
    declareLibraryFunction("text_gt", "B(TT)");
    declareLibraryFunction("text_ge", "B(TT)");
    declareLibraryFunction("text_lt", "B(TT)");
    declareLibraryFunction("text_le", "B(TT)");

    declareLibraryFunction("number_input", "N(T)");
    declareLibraryFunction("number_print", "V(N)");

    declareLibraryFunction("pow", "N(NN)");
    declareLibraryFunction("sqrt", "N(N)");

    library["malloc"] = llvm::FunctionType::get(
            builder.getPtrTy(), {builder.getInt64Ty()}, false);
    library["free"] = llvm::FunctionType::get(
            VoidTy, {builder.getPtrTy()}, false);
}

void IrEmitter::declareLibraryFunction(std::string_view name, std::string_view signature)
{
    auto charType = [&](char c) -> llvm::Type* {
        switch( c ) {
            case 'B': return BooleanTy;
            case 'N': return NumericTy;
            case 'T': return TextualTy;
            default:  return VoidTy;
        }
    };

    auto* returnType = charType(signature[0]);

    signature.remove_prefix(2);
    signature.remove_suffix(1);

    llvm::SmallVector<llvm::Type*> paramTypes;
    for( const char t : signature )
        paramTypes.push_back(charType(t));

    library[std::string{name}] = llvm::FunctionType::get(returnType, paramTypes, false);
}

llvm::FunctionCallee IrEmitter::libraryFunction(std::string_view name)
{
    return moduler.getOrInsertFunction(name, library[std::string{name}]);
}

llvm::FunctionCallee IrEmitter::userFunction(std::string_view name)
{
    if( "MID$" == name )
        return libraryFunction("text_mid");

    if( "STR$" == name )
        return libraryFunction("text_str");

    if( "SQR" == name )
        return libraryFunction("sqrt");

    return moduler.getFunction(name);
}

void IrEmitter::createEntryPoint()
{
    auto* Int32Ty = builder.getInt32Ty();

    auto* mainType = llvm::FunctionType::get(Int32Ty, {}, false);
    const auto linkage = llvm::GlobalValue::ExternalLinkage;
    auto* mainFunc = llvm::Function::Create(mainType, linkage, "main", &moduler);

    auto* start  = llvm::BasicBlock::Create(context, "start", mainFunc);
    builder.SetInsertPoint(start);

    if( auto* udMain = moduler.getFunction("Main"); nullptr != udMain )
        builder.CreateCall(udMain, {});

    auto* returnValue = llvm::ConstantInt::get(Int32Ty, 0);
    builder.CreateRet(returnValue);
}

void IrEmitter::declareSubroutines(Program::Ptr prog)
{
    for( const auto& subr : prog->_subroutines ) {
        llvm::SmallVector<llvm::Type*> paramTypes;
        for( const auto& pr : subr->_parameters )
            paramTypes.push_back(llvmType(pr));

        llvm::Type* returnType = builder.getVoidTy();

        auto* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
        const auto linkage = llvm::GlobalValue::ExternalLinkage;
        llvm::Function::Create(funcType, linkage, subr->_name, &moduler);
    }
}

void IrEmitter::defineSubroutines(Program::Ptr prog)
{
    for( const auto& subr : prog->_subroutines )
        visit(subr);
}

llvm::Type* IrEmitter::llvmType(std::string_view name)
{
    switch( typeOfName(name) ) {
        case 'B': return BooleanTy;
        case 'N': return NumericTy;
        case 'T': return TextualTy;
        default:  return VoidTy;
    }
}

bool IrEmitter::createsTempText(Expression::Ptr expr)
{
    auto et = exprType(expr);
    if( et == 'N' || et == 'B' )
        return false;

    if( NodeKind::Text == expr->kind || NodeKind::Variable == expr->kind )
        return false;

    return true;
}

llvm::CallInst* IrEmitter::createLibraryFuncCall(std::string_view fname,
            const llvm::ArrayRef<llvm::Value*>& args)
{
    return builder.CreateCall(libraryFunction(fname), args);
}

} // namespace basic
