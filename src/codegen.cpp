//
// Created by zack on 7/25/21.
//

#include "include/codegen.h"
#include "include/analysis.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"


// todo optimization passes
void generateOutput(const vector<ASTNode*>& nodes) {
    cout << "Initialized module" << endl;
    auto* context = new llvm::LLVMContext();
    auto* module = new llvm::Module("module", *context);
    auto* builder = new llvm::IRBuilder<>(*context);
    llvm::FunctionType* ft = llvm::FunctionType::get(llvm::Type::getVoidTy(*context), false);
    llvm::Function* func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "STELLAR_ENTRY", *module);
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(*context, "entry", func);
    builder->SetInsertPoint(entryBlock);
    map<string, llvm::Value*> namedValues;
    map<string, string> objectsTypes;
    map<string, ClassData> classes;
    auto generics = new map<string, VariableType>();
    CodegenData cd = { builder, context, entryBlock, &namedValues, module, &objectsTypes, &classes, generics };
    analyze(nodes, cd);
    for (auto const &node : nodes) {
        cout << node->toString() << endl;
        node->codegen(cd);
    }

    cout << "Adding return to main" << endl;
    builder->SetInsertPoint(&func->back());
    builder->CreateRetVoid();
    cout << "Writing object file..." << endl;
    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!target) {
        llvm::errs() << error;
        exit(EXIT_FAILURE);
    }
    cout << "target name: " << target->getName() << endl;
    auto CPU = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto targetMachine = target->createTargetMachine(targetTriple, CPU, features, opt, RM);
    cout << "Setting data layout" << endl;
    module->setDataLayout(targetMachine->createDataLayout());
    module->setTargetTriple(targetTriple);
    module->setPIELevel(llvm::PIELevel::Large);
    module->setPICLevel(llvm::PICLevel::BigPIC);
    module->print(llvm::outs(), nullptr);
    auto filename = "output.o";
    error_code EC;
    llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);
    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message();
        exit(EXIT_FAILURE);
    }
    llvm::legacy::PassManager pass;
    auto fileType = llvm::CGFT_ObjectFile;
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
        llvm::errs() << "Target machine cannot emit a file of this type";
        exit(EXIT_FAILURE);
    }
    cout << "Running pass" << endl;
    pass.run(*module);
    cout << "Pass complete" << endl;
    dest.flush();
}
