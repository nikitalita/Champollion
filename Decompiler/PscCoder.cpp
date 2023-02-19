#include "PscCoder.hpp"

#include <algorithm>
#include <cassert>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <locale>
#include <map>
#include <string>
#include <regex>

#include "PscDecompiler.hpp"

#include "EventNames.hpp"

/**
 * @brief Constructor
 * Builds an object associated with an output writer.
 *
 * @param writer Pointer to the output writer. The ownership is transferred.
 * @param commentAsm True to output assembly instruction comments (default: false).
 * @param writeHeader True to write the header (default: false).
 * @param traceDecompilation True to output decompilation tracing to the rebuild log (default: false).
 * @param dumpTree True to output the entire tree for each block (true by default if traceDecompilation is true).
 * @param traceDir If tracing is enabled, write rebuild logs to this dir (default is cwd)
 */
Decompiler::PscCoder::PscCoder( OutputWriter* writer,
                                bool commentAsm = false,
                                bool writeHeader = false,
                                bool traceDecompilation = false,
                                bool dumpTree = true,   
                                bool writeDebugFuncs = false,
                                bool printDebugLineNo = false,
                                std::string traceDir = ""):
    Coder(writer),
    m_CommentAsm(commentAsm),
    m_WriteHeader(writeHeader),
    m_TraceDecompilation(traceDecompilation),
    m_DumpTree(dumpTree), // Note that while dumpTree is true by default, it will not do anything unless traceDecompilation is true
    m_WriteDebugFuncs(writeDebugFuncs),
    m_OutputDir(traceDir),
    m_PrintDebugLineNo(printDebugLineNo)
{
    
}

/**
 * @brief Constructor
 * Builds an object associated with an output writer.
 *
 * @param writer Pointer to the output writer. The ownership is transferred.
 */
Decompiler::PscCoder::PscCoder(Decompiler::OutputWriter *writer)  :
    Coder(writer),
    m_CommentAsm(false),
    m_WriteHeader(false),
    m_TraceDecompilation(false),
    m_DumpTree(true),
    m_WriteDebugFuncs(false),
    m_PrintDebugLineNo(false),
    m_OutputDir("")
{
}

/**
 * @brief Default desctructor
 */
Decompiler::PscCoder::~PscCoder()
{
}

/**
 * @brief Decompile a PEX binary to a Papyrus file.
 * @param pex
 */
void Decompiler::PscCoder::code(const Pex::Binary &pex)
{
    if (m_WriteHeader) 
    {
        writeHeader(pex);
    }
    for(auto& object : pex.getObjects())
    {
        writeObject(object, pex);
    }

}

/**
 * @brief Set the option to output Assembly instruction in comments
 * @param commentAsm True to write the comments.
 * @return A reference to this.
 */
Decompiler::PscCoder &Decompiler::PscCoder::outputAsmComment(bool commentAsm)
{
    m_CommentAsm = commentAsm;
    return *this;
}

/**
 * @brief Set the option to write decompilation trace information to the rebuild log
 * @param traceDecompilation True to trace decompilation.
 * @return A reference to this.
 */
Decompiler::PscCoder &Decompiler::PscCoder::outputDecompilationTrace(bool traceDecompilation)
{
    m_TraceDecompilation = traceDecompilation;
    return *this;
}

/**
 * @brief Set the option to output the tree for each node during decompilation tracing
 * @param dumpTree True to dump node trees during decompilation tracing.
 * @return A reference to this.
 */
Decompiler::PscCoder &Decompiler::PscCoder::outputDumpTree(bool dumpTree)
{
    m_DumpTree = dumpTree;
    return *this;
}

/**
 * @brief Set the option to add a header to the decompiled script.
 * @param writeHeader True to write the header.
 * @return A reference to this.
 */
Decompiler::PscCoder &Decompiler::PscCoder::outputWriteHeader(bool writeHeader)
{
    m_WriteHeader = writeHeader;
    return *this;
}

/**
 * @brief Write the content of the PEX header as a block comment.
 * @param pex Binary to decompile.
 */
void Decompiler::PscCoder::writeHeader(const Pex::Binary &pex)
{
    auto& header = pex.getHeader();
    auto& debug  = pex.getDebugInfo();
    write(";/ Decompiled by Champollion V1.1.3"); // TODO: Make this get the version number dynamically
    write(indent(0) << "PEX format v" << (int)header.getMajorVersion() << "." << (int)header.getMinorVersion() << " GameID: " << header.getGameID());
    write(indent(0) << "Source   : " << header.getSourceFileName());
    if (debug.getModificationTime() != 0)
    {
        write(indent(0) << "Modified : " << std::put_time(std::localtime(&debug.getModificationTime()), "%Y-%m-%d %H:%M:%S"));
        //for (auto& f : debug.getFunctionInfos()) {
        //  write(indent(0) << f.getObjectName().asString() << ":" << f.getStateName().asString() << ":" << f.getFunctionName().asString() << " type: " << (int)f.getFunctionType());
        //  for (auto& l : f.getLineNumbers())
        //    write(indent(1) << "Line: " << l);
        //}
    }
    write(indent(0) << "Compiled : " << std::put_time(std::localtime(&header.getCompilationTime()), "%Y-%m-%d %H:%M:%S"));
    write(indent(0) << "User     : " << header.getUserName());
    write(indent(0) << "Computer : " << header.getComputerName());
    write("/;");
}

bool Decompiler::PscCoder::isNativeObject(const Pex::Object &object, const Pex::Binary::ScriptType &scriptType) const {
    if (scriptType == Pex::Binary::ScriptType::Fallout4Script)
    {
        for (auto& native : Fallout4::NativeClasses)
        {
            if (_stricmp(object.getName().asString().c_str(), native.c_str()) == 0){
                return true;
            }
        }
    }
    else if (scriptType == Pex::Binary::ScriptType::StarfieldScript)
    {
        for (auto& native : Starfield::NativeClasses)
        {
            if (_stricmp(object.getName().asString().c_str(), native.c_str()) == 0){
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief Write an object contained in the binary.
 * @param object Object to decompile
 * @param pex Binary to decompile.
 */
void Decompiler::PscCoder::writeObject(const Pex::Object &object, const Pex::Binary &pex)
{
    size_t lineCount = 0;
    size_t nextTargetLine = 0;
    auto stream = indent(0);
    stream <<"ScriptName " << object.getName().asString();
    if (! object.getParentClassName().asString().empty())
    {
        stream << " Extends " << object.getParentClassName().asString();
    }

    if (isNativeObject(object, pex.getGameType()))
        stream << " Native";

    if (object.getConstFlag())
      stream << " Const";

    writeUserFlag(stream, object, pex);
    write(stream.str());
    lineCount++;

    writeDocString(0, object);
    lineCount += countDocStringLineNos(object);

    auto naiveCountBeforeFuncs = countStructsLineNos(object, pex, true) + 2 // blank lines
            + object.getVariables().size() + 2 // blank lines
            + object.getGuards().size() + 2;

    auto propGroupsToWrite = makePropGroupsToWrite(object, pex);
    auto statesToWrite = makeStatesToWrite(object, pex);

    std::vector<size_t> groupStartingLines;
    size_t groupLineCount = 0;
    bool propsWithGettersSetters = false;
    for (auto & group : propGroupsToWrite){
        auto start = group.getStartingLine(false);
        groupStartingLines.push_back(start);
        if (start > 0){
            propsWithGettersSetters = true;
        }
        groupLineCount += group.getLineCount(false);
    }

    if (groupLineCount > 0){
        lineCount += groupLineCount + 2; // blank lines
    }



    if (object.getStructInfos().size()) {
        write("");
        write(";-- Structs -----------------------------------------");
        writeStructs(object, pex);
    }

    if (object.getVariables().size()) {
        write("");
        write(";-- Variables ---------------------------------------");
        writeVariables(object, pex);
    }

    if (object.getGuards().size()) {
        write("");
        write(";-- Guards ------------------------------------------");
        write(std::string(Decompiler::WARNING_COMMENT_PREFIX) + " WARNING: Guard declaration syntax is EXPERIMENTAL, subject to change");
        writeGuards(object, pex);
    }

    if (object.getProperties().size()) {
        write("");
        write(";-- Properties --------------------------------------");
        writeProperties(object, pex);
    }

    writeStates(object, pex);
}

/**
* @brief Write the struct definitions stored in the object.
* @param object Object containing the struct definitions.
* @param pex Binary to decompile.
*/
void Decompiler::PscCoder::writeStructs(const Pex::Object& object, const Pex::Binary& pex) {
    for (auto& sInfo : object.getStructInfos()) {
        write(indent(0) << "Struct " << sInfo.getName().asString());

        bool foundInfo = false;
        if (pex.getDebugInfo().getStructOrders().size()) {
            // If we have debug info, we have information on the order
            // they were in the original source file, so use that order.
            for (auto& sOrder : pex.getDebugInfo().getStructOrders()) {
                if (sOrder.getObjectName() == object.getName() && sOrder.getOrderName() == sInfo.getName()) {
                    if (sOrder.getNames().size() == sInfo.getMembers().size()) {
                        foundInfo = true;
                        for (auto& orderName : sOrder.getNames()) {
                            for (auto& member : sInfo.getMembers()) {
                                if (member.getName() == orderName) {
                                    writeStructMember(member, pex);
                                    goto ContinueOrder;
                                }
                            }
                            // If we get here, then we failed to find the struct
                            // member in the debug info :(
                            throw std::runtime_error("Unable to locate the struct member by the name of '" + orderName.asString() + "'");
                        ContinueOrder:
                            continue;
                        }
                    } else {
                        // This shouldn't happen, but it's possible that the
                        // debug info doesn't include all members of the struct,
                        // so write them in whatever order they are in the file.
                        break;
                    }
                }
            }
        }

        if (!foundInfo) {
            for (auto& mem : sInfo.getMembers())
                writeStructMember(mem, pex);
        }

        write(indent(0) << "EndStruct");
        write("");
    }
}

/**
* @brief Write the struct member passed in.
* @param member The member to output.
* @param pex Binary to decompile.
*/
void Decompiler::PscCoder::writeStructMember(const Pex::StructInfo::Member& member, const Pex::Binary& pex)
{
    auto stream = indent(1);
    stream << mapType(member.getTypeName().asString()) << " " << member.getName().asString();

    if (member.getValue().getType() != Pex::ValueType::None) {
        stream << " = " << member.getValue().toString();
    }
    writeUserFlag(stream, member, pex);
    if (member.getConstFlag())
      stream << " Const";
    write(stream.str());
    writeDocString(1, member);
}

/**
 * @brief Write the properties definitions stored in the object.
 * @param object Object containing the properties definitions.
 * @param pex Binary to decompile.
 */
void Decompiler::PscCoder::writeProperties(const Pex::Object &object, const Pex::Binary &pex)
{
    bool foundInfo = false;
    if (pex.getDebugInfo().getPropertyGroups().size()) {
        size_t totalProperties = 0;
        // If we have debug info, we have information on the order
        // they were in the original source file, so use that order.
        for (auto& propGroup : pex.getDebugInfo().getPropertyGroups()) {
            if (propGroup.getObjectName() == object.getName()) {
                totalProperties += propGroup.getNames().size();
            }
        }

        if (totalProperties == object.getProperties().size()) {
            foundInfo = true;

            for (auto& propGroup : pex.getDebugInfo().getPropertyGroups()) {
                if (propGroup.getObjectName() == object.getName()) {
                    int propertyIndent = 0;
                    if (!propGroup.getGroupName().asString().empty()) {
                        auto stream = indent(0);
                        stream << "Group " << propGroup.getGroupName();
                        writeUserFlag(stream, propGroup, pex);
                        write(stream.str());
                        writeDocString(0, propGroup);
                        propertyIndent = 1;
                    }

                    for (auto& propName : propGroup.getNames()) {
                        for (auto& prop : object.getProperties()) {
                            if (prop.getName() == propName) {
                                writeProperty(propertyIndent, prop, object, pex);
                                goto ContinueOrder;
                            }
                        }
                        // If we get here, then we failed to find the struct
                        // member in the debug info :(
                        throw std::runtime_error("Unable to locate the property by the name of '" + propName.asString() + "' referenced in the debug info");
                    ContinueOrder:
                        continue;
                    }

                    if (!propGroup.getGroupName().asString().empty()) {
                        write(indent(0) << "EndGroup");
                        write("");
                    }
                }
            }
        }
    }

    if (!foundInfo) {
        for (auto& prop : object.getProperties())
            writeProperty(0, prop, object, pex);
    }
}

/**
* @brief Write the property definition.
* @param i The indent level.
* @param prop The property to write.
* @param object Object containing the properties definitions.
* @param pex Binary to decompile.
*/
void Decompiler::PscCoder::writeProperty(int i, const Pex::Property& prop, const Pex::Object &object, const Pex::Binary& pex)
{
    const auto noState = pex.getStringTable().findIdentifier("");
    auto stream = indent(i);
    bool isAutoReadOnly = isPropAutoReadOnly(prop);
    stream << mapType(prop.getTypeName().asString()) << " Property " << prop.getName().asString();
    if (prop.hasAutoVar()) {
        auto var = object.getVariables().findByName(prop.getAutoVarName());
        if (var == nullptr)
            throw std::runtime_error("Auto variable for property not found");

        auto initialValue = var->getDefaultValue();
        if (initialValue.getType() != Pex::ValueType::None)
            stream << " = " << initialValue.toString();
        stream << " Auto";

        // The flags defined on the variable must be set on the property
        writeUserFlag(stream, *var, pex);
        if (var->getConstFlag())
          stream << " Const";
    } else if (isAutoReadOnly) {
      stream << " = " << prop.getReadFunction().getInstructions()[0].getArgs()[0].toString();
      stream << " AutoReadOnly";
    }
    writeUserFlag(stream, prop, pex);
    write(stream.str());
    writeDocString(i, prop);

    if (!prop.hasAutoVar() && !isAutoReadOnly) {
        if (prop.isReadable())
            writeFunction(i + 1, prop.getReadFunction(), object, pex, pex.getDebugInfo().getFunctionInfo(object.getName(),noState, prop.getName(), Pex::DebugInfo::FunctionType::Getter), "Get");
        if (prop.isWritable())
            writeFunction(i + 1, prop.getWriteFunction(), object, pex, pex.getDebugInfo().getFunctionInfo(object.getName(),noState, prop.getName(), Pex::DebugInfo::FunctionType::Setter), "Set");
        write(indent(i) << "EndProperty");
    }
}

bool Decompiler::PscCoder::isPropAutoReadOnly(const Pex::Property &prop) const {
  return !prop.hasAutoVar() &&
          prop.isReadable() &&
          !prop.isWritable() &&
           prop.getReadFunction().getInstructions().size() == 1 &&
           prop.getReadFunction().getInstructions()[0].getOpCode() == Pex::OpCode::RETURN &&
           prop.getReadFunction().getInstructions()[0].getArgs().size() == 1 &&
           prop.getReadFunction().getInstructions()[0].getArgs()[0].getType() != Pex::ValueType::Identifier;
}

/**
 * @brief Write the variables stored in the object.
 * @param object Object containing the variables.
 * @param pex Binary to decompile.
 */
void Decompiler::PscCoder::writeVariables(const Pex::Object &object, const Pex::Binary &pex)
{
    for (auto& var : object.getVariables())
    {
        auto name = var.getName().asString();
        bool compilerGenerated = (name.size() > 2 && name[0] == ':' && name[1] == ':');
        auto stream = indent(0);

        if (compilerGenerated)
            stream << "; ";

        stream << mapType(var.getTypeName().asString()) << " " << name;
        auto initialValue = var.getDefaultValue();
        if (initialValue.getType() != Pex::ValueType::None)
        {
            stream << " = " << initialValue.toString();
        }
        writeUserFlag(stream, var, pex);
        if (var.getConstFlag())
          stream << " Const";

        if (m_CommentAsm || !compilerGenerated)
        {
            write(stream.str());
        }
    }
}

/**
 * @brief Write the guards contained in the object.
 * @param object Object containing the guards.
 * @param pex Binary to decompile.
 */
void Decompiler::PscCoder::writeGuards(const Pex::Object& object, const Pex::Binary& pex) {
    for (auto& guard : object.getGuards()) {
        auto name = guard.getName().asString();
        write("Guard " + name);
    }
}

/**
 * @brief Write the states contained in the object.
 * @param object Object containing the states.
 * @param pex Binary to decompile.
 */
void Decompiler::PscCoder::writeStates(const Pex::Object &object, const Pex::Binary &pex)
{
    for (auto& state : object.getStates())
    {
        auto name = state.getName().asString();
        if (name.empty())
        {
            if (state.getFunctions().size()) {
                write("");
                write(";-- Functions ---------------------------------------");
                writeFunctions(0, state, object, pex);
            }
        }
        else
        {
            write("");
            write(";-- State -------------------------------------------");
            auto stream = indent(0);

            // The auto state name canbe a different index than the state name, event if it is the same value.
            if (_stricmp(state.getName().asString().c_str(), object.getAutoStateName().asString().c_str()) == 0)
            {
                stream << "Auto ";
            }
            write(stream.str() + "State " + state.getName().asString());
            writeFunctions(1, state, object, pex);
            write(indent(0) << "EndState");
        }
    }
}

/**
 * @brief Writes the functions associated with a state.
 * @param i The indentation level.
 * @param state State containing the functions.
 * @param object Object Containing the states.
 * @param pex Binary to decompile.
 */
void Decompiler::PscCoder::writeFunctions(int i, const Pex::State &state, const Pex::Object& object, const Pex::Binary &pex)
{
    for (auto& func : state.getFunctions())
    {
        write("");
        writeFunction(i, func, object, pex, pex.getDebugInfo().getFunctionInfo(object.getName(), state.getName(), func.getName()));
    }
}
static const std::regex tempRegex = std::regex("::temp\\d+");

/**
 * @brief Decompile a function.
 * @param i The indentation level.
 * @param function The function to decompile.
 * @param object The Object containing the function.
 * @param pex Binary to decompile.
 * @param name Name of the function. This parameter override the name stored in the function object.
 */
void Decompiler::PscCoder::writeFunction(int i, const Pex::Function &function, const Pex::Object &object,
                                         const Pex::Binary &pex, const Pex::DebugInfo::FunctionInfo *functionInfo,
                                         const std::string &name)
{
    std::string functionName = name;

    if (functionName.empty())
    {
        functionName = function.getName().asString();
    }

    bool isEvent = false;
    
    if (functionName.size() > 2 && !_stricmp(functionName.substr(0, 2).c_str(), "on")) {
        // We'd have to check for full inheritence to do this by object type
        // Right now, we're just seeing if matches all the built-in event names.
        std::string functionLower = functionName;
        std::transform(functionLower.begin(), functionLower.end(), functionLower.begin(), ::tolower);
        if (pex.getGameType() == Pex::Binary::ScriptType::SkyrimScript){
            if (std::find(Skyrim::EventNamesLowerCase.begin(), Skyrim::EventNamesLowerCase.end(), functionLower) != Skyrim::EventNamesLowerCase.end()) {
                isEvent = true;
            }
        } else if (pex.getGameType() == Pex::Binary::ScriptType::Fallout4Script){
            if (std::find(Fallout4::EventNamesLowerCase.begin(), Fallout4::EventNamesLowerCase.end(), functionLower) != Fallout4::EventNamesLowerCase.end()) {
                isEvent = true;
            }
        } else if (pex.getGameType() == Pex::Binary::ScriptType::StarfieldScript) {
            if (std::find(Starfield::EventNamesLowerCase.begin(), Starfield::EventNamesLowerCase.end(), functionLower) != Starfield::EventNamesLowerCase.end()) {
                isEvent = true;
            }
        }
    }

    if (functionName.size() > 9 && !_stricmp(functionName.substr(0, 9).c_str(), "::remote_")) {
      isEvent = true;
      functionName = functionName.substr(9);
      functionName[function.getParams()[0].getTypeName().asString().size()] = '.';
    }

    if (isCompilerGeneratedFunc(functionName, object, pex.getGameType())) {
        write(indent(i) << "; Skipped compiler generated " << functionName);
        return;
    }

    auto stream = indent(i);
    if (_stricmp(function.getReturnTypeName().asString().c_str(), "none") != 0)
        stream << mapType(function.getReturnTypeName().asString()) << " ";

    if (isEvent)
      stream << "Event ";
    else
      stream << "Function ";
    stream << functionName << "(";

    auto first = true;
    for (auto& param : function.getParams())
    {
        if (first)
        {
            first = false;
        }
        else
        {
            stream << ", ";
        }
        stream << mapType(param.getTypeName().asString()) << " " << param.getName();
    }
    stream << ")";


    if (function.isGlobal())
    {
        stream << " Global";
    }
    if (function.isNative())
    {
        stream << " Native";
        writeUserFlag(stream, function, pex);
        write(stream.str());
        writeDocString(i, function);
    } else {
        auto decomp = PscDecompiler(function, object, functionInfo, m_CommentAsm, m_TraceDecompilation, m_DumpTree,
                                    m_OutputDir);
        auto ids_in_use = decomp.getIdsInUse();

        auto stringids = std::vector<std::string>();
        auto propsInUse = std::vector<Pex::StringTable::Index>();
        auto varsInUse = std::vector<Pex::StringTable::Index>();
        for (auto& id : ids_in_use) {
          if (id.isValid()){
            stringids.push_back(id.asString());
            if (id.asString() == "self"){
              continue;
            }
            // check if this is a property autovar
            if (id.asString().starts_with("::") && id.asString().ends_with("_var")) {
              for (auto &prop: object.getProperties()) {
                if (prop.getAutoVarName() == id) {
                  propsInUse.push_back(prop.getName());
                }
              }
            } else {
              // check if this is an object variable
              for (auto &var: object.getVariables()) {
                if (var.getName() == id) {
                  varsInUse.push_back(id);
                }
              }
            }
          }
        }
        if (decomp.isDebugFunction()) {
            // Starfield debug function fixup hacks
            // These functions were supposed to have been compiled out of the pex, but the compiler left it in without restoring whatever the temp variable pointed to
            // This causes the recompilation to fail, so we need to replace the temp variable with false
          bool fixed = fixupFunction(object, pex, functionName, decomp);
          if (fixed){
              write(indent(i) << "; Fixup hacks for debug-only function: " << functionName);
            } else if (m_WriteDebugFuncs) {
              write(indent(i) << "; WARNING: possibly inoperative debug function " << functionName << "");
            } else {
              write(indent(i) << "; Skipped inoperative debug function " << functionName);
              return;
            }
        } else if (_stricmp(functionName.c_str(), "GotoState") == 0 || _stricmp(functionName.c_str(), "GetState") == 0) {
            // Starfield GotoState/GetState function fixup hacks
            write(indent(i) << "; Fixup hacks for native ScriptObject::GotoState/GetState");
            fixupFunction(object, pex, functionName, decomp);
        }
        writeUserFlag(stream, function, pex);
        write(stream.str());
        writeDocString(i, function);
        auto index = 0;
        for (auto &line: decomp) {
            auto & linemap = decomp.getLineMap();
            if (m_PrintDebugLineNo){
              // get index of line
              auto result = linemap[index];
              if (result.size() > 0){
                line += " ; #DEBUG_LINE_NO:";
                for (auto i = 0; i < result.size(); ++i)
                {
                    if (i > 0){
                    line += ",";
                    }
                    line += std::to_string(result[i]);
                }
              }
            }
            write(indent(i+1) << line);
            index++;
        }
        if (isEvent)
          write(indent(i) << "EndEvent");
        else
          write(indent(i) << "EndFunction");
    }

}

bool
Decompiler::PscCoder::fixupFunction(const Pex::Object &object, const Pex::Binary &pex, const std::string &functionName,
                                    Decompiler::PscDecompiler &decomp) {
  bool fixed = false;
  if (pex.getGameType() == Pex::Binary::StarfieldScript) {
      if (functionName == "warning" ||
          (_stricmp(object.getName().asString().c_str(), "ENV_Hazard_ParentScript") == 0 &&
           functionName == "GlobalWarning") || // only present on this script
          (_stricmp(object.getName().asString().c_str(), "ENV_AfflictionScript") == 0 &&
           functionName == "TraceStats")) { // Only present on this script
        // find the `::temp\d+` variable in the lines with regex
        // replace it with `false`
        fixed = true;
        for (auto &line: decomp) {
          if (std::regex_search(line, tempRegex)) {
            line = std::regex_replace(line, tempRegex, "false");
          }
        }
      } else if ((_stricmp(object.getName().asString().c_str(), "RobotQuestRunner") == 0)) {
          if (functionName == "UpdateState") {
            fixed = true;
            for (auto &line: decomp) {
              if (std::regex_search(line, tempRegex)) {
                line = std::regex_replace(line, tempRegex, "None");
              }
            }
          } else if (functionName == "MakeQuestNameSave") {
            fixed = true;
            for (auto &line: decomp) {
              if (std::regex_search(line, tempRegex)) {
                line = std::regex_replace(line, tempRegex, "questName");
              }
            }
          }
      }
  }
  if (_stricmp(object.getName().asString().c_str(), "ScriptObject") == 0) {
    // find the `::State` variable in the lines
    // replace it with `__state`
    for (auto &line : decomp){
      if (line.find("::State") != std::string::npos)
      {
        line = std::regex_replace(line, std::regex("::State"), "__state");
      }
    }
    fixed = true;
  }

  return fixed;
}

/**
 * @brief Write the user flags associated with an item.
 * @param stream Stream to write the flags to.
 * @param flagged Flagged item.
 * @param pex Binary to decompile.
 */
void Decompiler::PscCoder::writeUserFlag(std::ostream& stream, const Pex::UserFlagged &flagged, const Pex::Binary &pex)
{
    auto flags = flagged.getUserFlags();
    for (auto& flag : pex.getUserFlags())
    {
        if (flags & flag.getFlagMask())
        {
            stream << " " << flag.getName().asString();
        }
    }
}

/**
 * @brief Write the documentation string of an item.
 * @param i Indentation level.
 * @param item Documented item.
 */
void Decompiler::PscCoder::writeDocString(int i, const Pex::DocumentedItem &item)
{
    if (! item.getDocString().asString().empty())
    {
        write(indent(i) << "{ " << item.getDocString().asString() << " }");
    }
}

static const std::map<std::string, std::string> prettyTypeNameMap {
    // Builtin Types
    { "bool", "Bool" },
    { "float", "Float" },
    { "int", "Int" },
    { "string", "String" },
    { "var", "Var" },

    // Special
    { "self", "Self" },

    // General Types
    { "action", "Action" },
    { "activator", "Activator" },
    { "activemagiceffect", "ActiveMagicEffect" },
    { "actor", "Actor" },
    { "actorbase", "ActorBase" },
    { "actorvalue", "ActorValue" },
    { "alias", "Alias" },
    { "ammo", "Ammo" },
    { "apparatus", "Apparatus" },
    { "armor", "Armor" },
    { "associationtype", "AssociationType" },
    { "book", "Book" },
    { "cell", "Cell" },
    { "class", "Class" },
    { "constructibleobject", "ConstructibleObject" },
    { "container", "Container" },
    { "debug", "Debug" },
    { "door", "Door" },
    { "effectshader", "EffectShader" },
    { "enchantment", "Enchantment" },
    { "encounterzone", "EncounterZone" },
    { "explosion", "Explosion" },
    { "faction", "Faction" },
    { "flora", "Flora" },
    { "form", "Form" },
    { "formlist", "FormList" },
    { "furniture", "Furniture" },
    { "game", "Game" },
    { "globalvariable", "GlobalVariable" },
    { "hazard", "Hazard" },
    { "idle", "Idle" },
    { "imagespacemodifier", "ImageSpaceModifier" },
    { "impactdataset", "ImpactDataSet" },
    { "ingredient", "Ingredient" },
    { "key", "Key" },
    { "keyword", "Keyword" },
    { "leveledactor", "LeveledActor" },
    { "leveleditem", "LeveledItem" },
    { "leveledspell", "LeveledSpell" },
    { "light", "Light" },
    { "location", "Location" },
    { "locationalias", "LocationAlias" },
    { "locationreftype", "LocationRefType" },
    { "magiceffect", "MagicEffect" },
    { "math", "Math" },
    { "message", "Message" },
    { "miscobject", "MiscObject" },
    { "musictype", "MusicType" },
    { "objectreference", "ObjectReference" },
    { "outfit", "Outfit" },
    { "package", "Package" },
    { "perk", "Perk" },
    { "potion", "Potion" },
    { "projectile", "Projectile" },
    { "quest", "Quest" },
    { "race", "Race" },
    { "referencealias", "ReferenceAlias" },
    { "refcollectionalias", "RefCollectionAlias" },
    { "scene", "Scene" },
    { "scroll", "Scroll" },
    { "scriptobject", "ScriptObject" },
    { "shout", "Shout" },
    { "soulgem", "SoulGem" },
    { "sound", "Sound" },
    { "soundcategory", "SoundCategory" },
    { "spell", "Spell" },
    { "static", "Static" },
    { "talkingactivator", "TalkingActivator" },
    { "topic", "Topic" },
    { "topicinfo", "TopicInfo" },
    { "utility", "Utility" },
    { "visualeffect", "VisualEffect" },
    { "voicetype", "VoiceType" },
    { "weapon", "Weapon" },
    { "weather", "Weather" },
    { "wordofpower", "WordOfPower" },
    { "worldspace", "WorldSpace" },
};

void str_to_lower(std::string &p_str){
    for (int i = 0; i < p_str.size(); i++){
        p_str[i] = tolower(p_str[i]);
    }
}
/**
* @brief Map the type name used by the compiler to the form most used by people.
* @param type The type to map.
*/
std::string Decompiler::PscCoder::mapType(std::string type)
{
    std::replace(type.begin(), type.end(), '#', ':');
    if (type.length() > 2 && type[type.length() - 2] == '[' && type[type.length() - 1] == ']')
        return mapType(type.substr(0, type.length() - 2)) + "[]";
    auto lowerType = type;
    str_to_lower(lowerType);
    auto a = prettyTypeNameMap.find(lowerType);
    if (a != prettyTypeNameMap.end())
        return a->second;
    return type;
}

bool Decompiler::PscCoder::isCompilerGeneratedFunc(const std::string &name, const Pex::Object &object, Pex::Binary::ScriptType scriptType) const {
    static const std::vector<std::string> globalCompilerGeneratedFuncs = {
            "getstate",
            "gotostate",
    };
    static const std::vector<std::string> starfieldCompilerGeneratedFuncs = {
    };
    // Do not remove these for the actual `scriptobject` script which is the base class for all scripts
    if (_stricmp(object.getName().asString().c_str(), "ScriptObject") == 0){
        return false;
    }
    std::string nameLower = name;
    str_to_lower(nameLower);
    if (std::find(globalCompilerGeneratedFuncs.begin(), globalCompilerGeneratedFuncs.end(), nameLower) != globalCompilerGeneratedFuncs.end())
        return true;
    if (scriptType == Pex::Binary::ScriptType::StarfieldScript && std::find(starfieldCompilerGeneratedFuncs.begin(), starfieldCompilerGeneratedFuncs.end(), name) != starfieldCompilerGeneratedFuncs.end())
        return true;
    return false;
}

void Decompiler::PscCoder::store(const std::string &line) {
  m_Text.push_back(line);
  std::find(m_Text.begin(), m_Text.end(), line);
}

void Decompiler::PscCoder::store(std::ostream &&stream) {
  auto& sstream = static_cast<std::ostringstream&>(stream);
  m_Text.push_back(sstream.str());

}

Decompiler::PscCoder::FunctionToWrite Decompiler::PscCoder::getFunctionToWrite(int i, const Pex::Function &function, const Pex::Object &object,
                                                                               const Pex::Binary &pex, const Pex::DebugInfo::FunctionInfo *functionInfo,
                                                                               const std::string &name) {
  std::string functionName = name;

  FunctionToWrite func;

  func.name = function.getName();
  func.debugInfo = functionInfo;
  auto nostate = pex.getStringTable().findIdentifier("");
  if (functionName.empty())
  {
    functionName = function.getName().asString();
  }

  func.isEvent = IsEvent(pex.getGameType(), functionName);

  if (functionName.size() > 9 && !_stricmp(functionName.substr(0, 9).c_str(), "::remote_")) {
    func.isEvent = true;
    functionName = functionName.substr(9);
    functionName[function.getParams()[0].getTypeName().asString().size()] = '.';
  }
  func.nameAsWritten = functionName;


  if (isCompilerGeneratedFunc(functionName, object, pex.getGameType())) {
    func.skipped = true;
    func.precedingComment = (indent(i) << "; Skipped compiler generated debug function " << functionName).str();
    return func;
  }

  auto stream = indent(i);
  if (_stricmp(function.getReturnTypeName().asString().c_str(), "none") != 0)
    stream << mapType(function.getReturnTypeName().asString()) << " ";

  if (func.isEvent)
    stream << "Event ";
  else
    stream << "Function ";
  stream << functionName << "(";

  auto first = true;
  for (auto& param : function.getParams())
  {
    if (first)
    {
      first = false;
    }
    else
    {
      stream << ", ";
    }
    stream << mapType(param.getTypeName().asString()) << " " << param.getName();
  }
  stream << ")";


  if (function.isGlobal())
  {
    stream << " Global";
  }
  if (function.isNative())
  {
    stream << " Native";
    writeUserFlag(stream, function, pex);
    func.BodyLines.push_back(stream.str());
    func.docString = (indent(i) << "{ "  << function.getDocString().asString() << " }").str();
    return func;
  }

  auto decomp = PscDecompiler(function, object, functionInfo, m_CommentAsm, m_TraceDecompilation, m_DumpTree,
                              m_OutputDir);

  if (decomp.isDebugFunction()) {
    // Starfield debug function fixup hacks
    // These functions were supposed to have been compiled out of the pex, but the compiler left it in without restoring whatever the temp variable pointed to
    // This causes the recompilation to fail, so we need to replace the temp variable with false
    bool fixed = fixupFunction(object, pex, functionName, decomp);
    if (fixed){
      func.precedingComment = (indent(i) << "; Fixup hacks for debug-only function: " << functionName).str();
    } else if (m_WriteDebugFuncs) {
      func.precedingComment = (indent(i) << "; WARNING: possibly inoperative debug function " << functionName << "").str();
    } else {
      func.skipped = true;
      func.precedingComment = (indent(i) << "; Skipped inoperative debug function " << functionName).str();
      return func;
    }
  } else if (_stricmp(functionName.c_str(), "GotoState") == 0 || _stricmp(functionName.c_str(), "GetState") == 0) {
    // Starfield GotoState/GetState function fixup hacks
    func.precedingComment = (indent(i) << "; Fixup hacks for native ScriptObject::GotoState/GetState").str();
    fixupFunction(object, pex, functionName, decomp);
  }

  // Find the props and object vars in use in this function
  auto ids_in_use = decomp.getIdsInUse();
  auto stringids = std::vector<std::string>();
  for (auto& id : ids_in_use) {
    if (id.isValid()){
      stringids.push_back(id.asString());
      if (id.asString() == "self"){
        continue;
      }
      // check if this is a property autovar
      if (id.asString().starts_with("::") && id.asString().ends_with("_var")) {
        for (auto &prop: object.getProperties()) {
          if (prop.getAutoVarName() == id) {
            func.propsInUse.push_back(prop.getName());
          }
        }
      } else {
        // check if this is an object variable
        // NOTE: We assume that object variables override local variables, as PCompiler does by default
        for (auto &var: object.getVariables()) {
          if (var.getName() == id) {
            func.varsInUse.push_back(id);
          }
        }
      }
    }
  }

  writeUserFlag(stream, function, pex);
  func.declLine = stream.str();
  func.docString = (indent(i) << "{ "  << function.getDocString().asString() << " }").str();
  auto index = 0;
  auto & linemap = decomp.getLineMap();
  for (auto &line: decomp) {
    if (m_PrintDebugLineNo && !linemap.empty()){
      // get index of line
      auto result = linemap[index];
      if (result.size() > 0){
        line += " ; #DEBUG_LINE_NO:";
        for (auto i = 0; i < result.size(); ++i)
        {
          if (i > 0){
            line += ",";
          }
          line += std::to_string(result[i]);
        }
      }
    }
    if (line.empty()){
      func.BodyLines.emplace_back();
    } else {
      func.BodyLines.push_back((indent(i + 1) << line).str());
    }
    index++;
  }
  if (functionInfo->getLineNumbers().size()){
    func.bodyStartLine = functionInfo->getLineNumbers().front();
  }
  return func;
}

bool Decompiler::PscCoder::IsEvent(Pex::Binary::ScriptType scriptType, const std::string &functionName) {
  bool isEvent = false;
  if (functionName.size() > 2 && !_stricmp(functionName.substr(0, 2).c_str(), "on")) {
    // We'd have to check for full inheritence to do this by object type
    // Right now, we're just seeing if matches all the built-in event names.
    std::string functionLower = functionName;
    std::transform(functionLower.begin(), functionLower.end(), functionLower.begin(), tolower);
    const std::vector<std::string> * eventNamesLowerCase;
    switch(scriptType)
    {
      case Pex::Binary::SkyrimScript:
        eventNamesLowerCase = &Skyrim::EventNamesLowerCase;
        break;
      case Pex::Binary::Fallout4Script:
        eventNamesLowerCase = &Fallout4::EventNamesLowerCase;
        break;
      case Pex::Binary::StarfieldScript:
        eventNamesLowerCase = &Starfield::EventNamesLowerCase;
        break;

    }
    if (std::find(eventNamesLowerCase->begin(), eventNamesLowerCase->end(), functionLower) != eventNamesLowerCase->end()) {
      isEvent = true;
    }
  }
  return isEvent;
}

size_t Decompiler::PscCoder::countDocStringLineNos(const Pex::DocumentedItem &item) {
  if (item.getDocString().asString().empty()) {
    return 0;
  }
  // count how many `\n` there are in the string
  std::string docString = item.getDocString().asString();
  return std::count(docString.begin(), docString.end(), '\n') + 1;
}

size_t Decompiler::PscCoder::countStructLineNos(const Pex::StructInfo &structInfo, bool blank_lines = true) {
  size_t lineNumbers = 2; // struct and endstruct
  for (auto& member : structInfo.getMembers()) {
    lineNumbers++; // member decl;

    auto doclines = countDocStringLineNos(member);
    if (blank_lines && doclines > 0){
      lineNumbers += 1; // blank space after docstring
    }
    lineNumbers += doclines;
  }
  return lineNumbers;
}

size_t Decompiler::PscCoder::countStructsLineNos(const Pex::Object &object, const Pex::Binary &pex, bool blank_lines = true) {
  size_t lineNumbers = 0;
  for (auto& sInfo : object.getStructInfos()) {
    lineNumbers += countStructLineNos(sInfo, blank_lines) + (blank_lines ? 1 : 0);
  }
  return lineNumbers;
}

size_t Decompiler::PscCoder::countPropertyGroupLineNos(const Pex::DebugInfo::PropertyGroup &propGroup, const Pex::Object &object,
                                                       const Pex::Binary &pex, bool blank_lines = true) {
  return 0; // not implemented
}

size_t Decompiler::PscCoder::countPropertiesLineNos(const Pex::Object &object, const Pex::Binary &pex, bool blank_lines = true) {
  size_t lineNumbers = 0;
  for (auto& propgroup : pex.getDebugInfo().getPropertyGroups()){
    lineNumbers += 2 + (blank_lines ? 1 : 0); // Group, EndGroup, Blank Space
    lineNumbers += countDocStringLineNos(propgroup) + (blank_lines ? 1 : 0);
  }
  for (auto& prop : object.getProperties()) {
    auto propcount = countPropertyLineNos(prop, object, pex, blank_lines);
    if (propcount > 1 && blank_lines){
      lineNumbers++; // space between this and next property
    }
    lineNumbers += propcount;
  }
  return 0;
}

size_t Decompiler::PscCoder::countPropertyLineNos(const Pex::Property &prop, const Pex::Object &object,
                                                  const Pex::Binary &pex, bool blank_lines = true) {

  const auto noState = pex.getStringTable().findIdentifier("");

  size_t lineNumbers = 1 + countDocStringLineNos(prop);
  if (isPropAutoReadOnly(prop) || prop.hasAutoVar()) {
    return lineNumbers;
  }
  lineNumbers+=1; // "EndProperty"
  if (prop.isReadable())
  {
    lineNumbers += countFunctionLineNos(prop.getReadFunction(), object, pex, pex.getDebugInfo().getFunctionInfo(object.getName(),noState, prop.getName(), Pex::DebugInfo::FunctionType::Getter), blank_lines);
  }
  if (prop.isWritable()){
    lineNumbers += countFunctionLineNos(prop.getReadFunction(), object, pex, pex.getDebugInfo().getFunctionInfo(object.getName(),noState, prop.getName(), Pex::DebugInfo::FunctionType::Setter), blank_lines);
  }
  return 0;
}



size_t Decompiler::PscCoder::countFunctionLineNos(const Pex::Function &function,
                                                  const Pex::Object &object,
                                                  const Pex::Binary &pex,
                                                  const Pex::DebugInfo::FunctionInfo *functionInfo,
                                                  bool blank_lines = true) {
  size_t lineNumbers = 2 + countDocStringLineNos(function); // Function + EndFunction + DocString
  if (!functionInfo){
    return lineNumbers;
  }
  auto &debugLineNos = functionInfo->getLineNumbers();
  if (debugLineNos.empty())
    return lineNumbers;
  auto max = std::max_element(debugLineNos.begin(), debugLineNos.end());
  lineNumbers += *max;
  // however many elements there are past the index of element with the largest number, that's a closing `EndIf` or `EndGuard`.
  lineNumbers += (debugLineNos.size() - (std::distance(debugLineNos.begin(), max) + 1));
  return lineNumbers;
}

size_t Decompiler::PscCoder::countStateLineNos(const Pex::State &state, const Pex::Object &object, const Pex::Binary &pex, bool blank_lines = true) {
  size_t lineNumbers = 2; // State, EndState
  auto name = state.getName().asString();
  if (name.empty())
  {
    for (auto& func : state.getFunctions())
    {
      // check if it's a getter/setter
      auto funcInfo = pex.getDebugInfo().getFunctionInfo(object.getName(), state.getName(), func.getName());
      // if it's a setter or a getter, we're not adding it to the state line count, it'll be part of its property line count
      if (funcInfo->getFunctionType() == Pex::DebugInfo::FunctionType::Getter || funcInfo->getFunctionType() == Pex::DebugInfo::FunctionType::Setter){
        continue;
      }
      lineNumbers += countFunctionLineNos(func, object, pex, funcInfo, blank_lines);
    }
  }
  else
  {
    for (auto& func : state.getFunctions())
    {
      lineNumbers += countFunctionLineNos(func, object, pex, pex.getDebugInfo().getFunctionInfo(object.getName(), state.getName(), func.getName()), blank_lines);
    }
  }
  return lineNumbers;
}

size_t Decompiler::PscCoder::countStatesLineNos(const Pex::Object &object, const Pex::Binary &pex, bool blank_lines = true) {

  size_t lineNumbers = 0;
  for (auto& state : object.getStates())
  {
    lineNumbers += countStateLineNos(state, object, pex, blank_lines) + (blank_lines ? 1 : 0);
  }
  return 0;
}

Decompiler::PscCoder::PropertyToWrite
Decompiler::PscCoder::makePropertyToWrite(int i, const Pex::Property &prop, const Pex::Object &object,
                                          const Pex::Binary &pex) {
  PropertyToWrite propToWrite;
  propToWrite.name = prop.getName();
  const auto noState = pex.getStringTable().findIdentifier("");
  auto stream = indent(i);
  bool isAutoReadOnly = isPropAutoReadOnly(prop);
  stream << mapType(prop.getTypeName().asString()) << " Property " << prop.getName().asString();
  if (prop.hasAutoVar()) {
    auto var = object.getVariables().findByName(prop.getAutoVarName());
    if (var == nullptr)
      throw std::runtime_error("Auto variable for property not found");

    auto initialValue = var->getDefaultValue();
    if (initialValue.getType() != Pex::ValueType::None)
      stream << " = " << initialValue.toString();
    stream << " Auto";

    // The flags defined on the variable must be set on the property
    writeUserFlag(stream, *var, pex);
    if (var->getConstFlag())
      stream << " Const";
  } else if (isAutoReadOnly) {
    stream << " = " << prop.getReadFunction().getInstructions()[0].getArgs()[0].toString();
    stream << " AutoReadOnly";
  }
  writeUserFlag(stream, prop, pex);
  propToWrite.propertyDecl = stream.str();
  propToWrite.docString = (indent(i) << "{ "  << prop.getDocString().asString() << " }").str();

  if (!prop.hasAutoVar() && !isAutoReadOnly) {
    if (prop.isReadable()) {
      propToWrite.hasGetter = true;
      propToWrite.getter = getFunctionToWrite(i + 1, prop.getReadFunction(), object, pex,
                                              pex.getDebugInfo().getFunctionInfo(object.getName(), noState,
                                                                                 prop.getName(),
                                                                                 Pex::DebugInfo::FunctionType::Getter),
                                              "Get");
    }
    if (prop.isWritable()) {
      propToWrite.hasSetter = true;
      propToWrite.setter = getFunctionToWrite(i + 1, prop.getWriteFunction(), object, pex,
                                         pex.getDebugInfo().getFunctionInfo(object.getName(), noState, prop.getName(),
                                                                            Pex::DebugInfo::FunctionType::Setter),
                                         "Set");
    }
  }

  return propToWrite;
}

Decompiler::PscCoder::StatesToWrite Decompiler::PscCoder::makeStatesToWrite(const Pex::Object &object, const Pex::Binary &pex) {
  StatesToWrite states;
  for (auto& state : object.getStates())
  {
    StateToWrite stow;
    int i = 0;
    stow.name = state.getName();

    if (!stow.name.asString().empty()) {
      i = 1;
      auto stream = indent(0);

      // The auto state name canbe a different index than the state name, event if it is the same value.
      if (_stricmp(state.getName().asString().c_str(), object.getAutoStateName().asString().c_str()) == 0) {
        stream << "Auto ";
      }
      stow.declLine = stream.str() + "State " + state.getName().asString();
    }
    for (auto& func : state.getFunctions())
    {
      stow.funcs.push_back(getFunctionToWrite(i, func, object, pex, pex.getDebugInfo().getFunctionInfo(object.getName(), state.getName(), func.getName())));
    }
  }

}

Decompiler::PscCoder::PropGroupsToWrite
Decompiler::PscCoder::makePropGroupsToWrite(const Pex::Object &object, const Pex::Binary &pex) {
  bool foundInfo = false;
  PropGroupsToWrite propGroupsToWrite;
  if (pex.getDebugInfo().getPropertyGroups().size()) {
    size_t totalProperties = 0;
    // If we have debug info, we have information on the order
    // they were in the original source file, so use that order.
    for (auto& propGroup : pex.getDebugInfo().getPropertyGroups()) {
      if (propGroup.getObjectName() == object.getName()) {
        totalProperties += propGroup.getNames().size();
      }
    }

    if (totalProperties == object.getProperties().size()) {
      foundInfo = true;

      for (auto& propGroup : pex.getDebugInfo().getPropertyGroups()) {
        PropGroupToWrite pgtw;
        if (propGroup.getObjectName() == object.getName()) {
          int propertyIndent = 0;
          auto name = propGroup.getGroupName();
          auto &propsTarget = !propGroup.getGroupName().asString().empty() ? pgtw.props : propGroupsToWrite.looseProps;
          if (!propGroup.getGroupName().asString().empty()) {
            pgtw.name = propGroup.getGroupName();
            auto stream = indent(0);
            stream << "Group " << propGroup.getGroupName();
            writeUserFlag(stream, propGroup, pex);
            pgtw.declString = stream.str();
            pgtw.docString = (indent(0) << "{ "  << propGroup.getDocString().asString() << " }").str();
            propertyIndent = 1;
          }

          for (auto& propName : propGroup.getNames()) {
            for (auto& prop : object.getProperties()) {
              if (prop.getName() == propName) {
                propsTarget.push_back(makePropertyToWrite(propertyIndent, prop, object, pex));
                goto ContinueOrder;
              }
            }
            // If we get here, then we failed to find the struct
            // member in the debug info :(
            throw std::runtime_error("Unable to locate the property by the name of '" + propName.asString() + "' referenced in the debug info");
            ContinueOrder:
            continue;
          }
        }
        if (pgtw.name.isValid()){
          propGroupsToWrite.propGroups.push_back(pgtw);
        }
      }
    }
  }

  if (!foundInfo) {
    for (auto& prop : object.getProperties())
      propGroupsToWrite.looseProps.push_back(makePropertyToWrite(0, prop, object, pex));
  }
}

std::string Decompiler::PscCoder::makeVariableToWrite(const Pex::Variable &var, const Pex::Binary &pex) {
  auto name = var.getName().asString();
  bool compilerGenerated = (name.size() > 2 && name[0] == ':' && name[1] == ':');
  auto stream = indent(0);

  if (compilerGenerated)
    stream << "; ";

  stream << mapType(var.getTypeName().asString()) << " " << name;
  auto initialValue = var.getDefaultValue();
  if (initialValue.getType() != Pex::ValueType::None)
  {
    stream << " = " << initialValue.toString();
  }
  writeUserFlag(stream, var, pex);
  if (var.getConstFlag())
    stream << " Const";

  if (m_CommentAsm || !compilerGenerated)
  {
    return stream.str();
  }
  return "";
}


size_t Decompiler::PscCoder::PropGroupToWrite::getStartingLine(bool blank_lines = true) {
  size_t preceding = name.asString().empty() ? 0 : 1;
  auto docCount = docString.empty() ? 0 : std::count(docString.begin(), docString.end(), '\n') + 1;
  preceding += docCount;
  if (docCount > 0 && blank_lines){
    preceding += 1; // blank line after docstring
  }
  for (auto &prop : props) {
    auto start = prop.getStartingLine(blank_lines);
    if (start > 0){
      return start - preceding;
    }
  }
  return 0;
}

size_t Decompiler::PscCoder::PropGroupToWrite::getLineCount(bool blank_lines) {
  size_t lineCount = name.asString().empty() ? 0 : 2; // start/end group
  auto docCount = docString.empty() ? 0 : std::count(docString.begin(), docString.end(), '\n') + 1;
  lineCount += docCount;
  if (docCount > 0 && blank_lines){
    lineCount += 1; // blank line after docstring
  }
  for (auto &prop : props) {
    auto prcount = prop.getLineCount(blank_lines);
    if (prcount > 1){
      lineCount++; // blank line after multi-line prop
    }
    lineCount += prcount;
  }
  return lineCount;
}

size_t Decompiler::PscCoder::PropertyToWrite::getStartingLine(bool blank_lines = true) {
  // check the functions for what line they need to be on
  if (!(hasGetter || hasSetter)){
    return 0;
  }
  size_t preceding = 1; // property decl

  auto docCount = docString.empty() ? 0 : std::count(docString.begin(), docString.end(), '\n') + 1;
  preceding += docCount;
  if (docCount > 0 && blank_lines){
    preceding += 1; // blank line after docstring
  }
  size_t getterStart = hasGetter ? getter.getStartingLine(blank_lines) : 0;
  size_t setterStart = hasSetter ? setter.getStartingLine(blank_lines) : 0;
  size_t funcStart = std::min(getterStart, setterStart);
  return funcStart - preceding;
}

size_t Decompiler::PscCoder::PropertyToWrite::getLineCount(bool blank_lines) {
  size_t lineCount = 2; // property decl + end

  auto docCount = docString.empty() ? 0 : std::count(docString.begin(), docString.end(), '\n') + 1;
  lineCount += docCount;
  if (docCount > 0 && blank_lines){
    lineCount += 1; // blank line after docstring
  }

  if (!(hasGetter || hasSetter)){
    return lineCount;
  }

  size_t getterStart = hasGetter ? getter.getStartingLine(blank_lines) : 0;
  size_t setterStart = hasSetter ? setter.getStartingLine(blank_lines) : 0;
  if (hasGetter && hasSetter){
    // blank lines between the two
    lineCount += std::abs((int64_t)(getterStart - setterStart));
  }
  size_t getterCount = hasGetter ? getter.getLineCount(blank_lines) : 0;
  size_t setterCount = hasSetter ? setter.getLineCount(blank_lines) : 0;
  return lineCount += getterCount + setterCount;
}

size_t Decompiler::PscCoder::StateToWrite::getStartingLine(bool blank_lines = true) {
  size_t preceding = name.asString().empty() ? 0 : 1;

  for (int i = 0; i < funcs.size(); i++){
    auto func = funcs[i];
    if (func.bodyStartLine > 0){
      return func.getStartingLine(blank_lines) - preceding; // wherever the body starts minus however many preceeding lines
    }
  }
  return 0;
}

size_t Decompiler::PscCoder::StateToWrite::getLineCount(bool blank_lines) {
  if (name.asString().empty()){
    return 0; // blank state, line count not relevant
  }
  size_t preceding = name.asString().empty() ? 0 : 2; // start/end state

  return 0;
}


size_t Decompiler::PscCoder::FunctionToWrite::getLineCount(bool blank_lines = true) {
  size_t lineNumbers = 1; // start function
  lineNumbers += (precedingComment.empty() ? 0 : 1);
  auto docCount = docString.empty() ? 0 : std::count(docString.begin(), docString.end(), '\n') + 1;
  if (docCount > 0 && blank_lines){
    lineNumbers += 1; // blank line after docstring
  }
  lineNumbers += docCount;
  lineNumbers += BodyLines.size();
  lineNumbers += 1; // end line
  return lineNumbers;
}

size_t Decompiler::PscCoder::FunctionToWrite::getStartingLine(bool blank_lines = true) {
  if (bodyStartLine == 0){
    return 0;
  }
  return bodyStartLine - (getLineCount(blank_lines) - 1 - BodyLines.size()); // wherever the body starts minus however many preceeding lines
}