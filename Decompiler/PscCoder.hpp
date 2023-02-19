#pragma once
#include "Coder.hpp"
#include "PscDecompiler.hpp"


namespace Decompiler {
static const char* WARNING_COMMENT_PREFIX = ";***";
/**
 * @brief Write a PEX file as a PSC file.
 */
class PscCoder :
        public Coder
{
public:
    PscCoder(OutputWriter *writer, bool commentAsm, bool writeHeader, bool traceDecompilation, bool dumpTree,
             bool writeDebugFuncs, bool printDebugLineNo, std::string traceDir);
    PscCoder(OutputWriter* writer);
    ~PscCoder();

    virtual void code(const Pex::Binary& pex);

    PscCoder& outputDecompilationTrace(bool traceDecompilation);
    PscCoder& outputDumpTree(bool dumpTree);
    PscCoder& outputAsmComment(bool commentAsm);
    PscCoder& outputWriteHeader(bool writeHeader);
    static std::string mapType(std::string type);
protected:

    struct SortableWriteObject{
        virtual ~SortableWriteObject() = default;
        virtual size_t getLineCount(bool blank_lines) = 0;
        virtual size_t getStartingLine(bool blank_lines) = 0;
        virtual bool dependencyOf(SortableWriteObject& other) = 0;
    };
    struct FunctionToWrite {
        bool skipped;
        bool isEvent;
        const Pex::DebugInfo::FunctionInfo *debugInfo;
        Pex::StringTable::Index name;
        std::string precedingComment;
        std::string nameAsWritten;
        std::string declLine;
        std::string docString{};
        std::vector<std::string> BodyLines;
        uint16_t bodyStartLine;
        size_t minimumStartingLine;
        std::vector<Pex::StringTable::Index> propsInUse;
        std::vector<Pex::StringTable::Index> varsInUse;
        size_t getLineCount(bool blank_lines);
        size_t getStartingLine(bool blank_lines);
    };

    struct StateToWrite {
        Pex::StringTable::Index name;
        std::string declLine;
        std::vector<FunctionToWrite> funcs;
        size_t getLineCount(bool blank_lines);
        size_t getStartingLine(bool blank_lines);
    };
    struct StatesToWrite{
        std::vector<FunctionToWrite> looseFuncs;
        std::vector<StateToWrite> states;
    };

    struct PropertyToWrite {
        bool isAuto;
        bool hasGetter;
        bool hasSetter;
        Pex::StringTable::Index name;
        std::string docString;
        std::string propertyDecl;
        FunctionToWrite getter;
        FunctionToWrite setter;
        bool dependencyOf();
        size_t getLineCount(bool blank_lines);
        size_t getStartingLine(bool blank_lines);
    };
    struct PropGroupToWrite{
        Pex::StringTable::Index name;
        std::string declString{};
        std::string docString{};
        std::vector<PropertyToWrite> props;
        size_t getLineCount(bool blank_lines);
        size_t getStartingLine(bool blank_lines);
    };
    struct PropGroupsToWrite{
        std::vector<PropertyToWrite> looseProps;
        std::vector<PropGroupToWrite> propGroups;
    };

    void writeHeader(const Pex::Binary& pex);
    void writeObject(const Pex::Object& object, const Pex::Binary& pex);
    void writeStructs(const Pex::Object& object, const Pex::Binary& pex);
    void writeStructMember(const Pex::StructInfo::Member& member, const Pex::Binary& pex);
    void writeProperties(const Pex::Object& object, const Pex::Binary& pex);
    void writeProperty(int i, const Pex::Property& prop, const Pex::Object &object, const Pex::Binary& pex);
    void writeVariables(const Pex::Object& object, const Pex::Binary& pex);
    void writeGuards(const Pex::Object& object, const Pex::Binary& pex);
    void writeStates(const Pex::Object& object, const Pex::Binary& pex);
    void writeFunctions(int i, const Pex::State& state, const Pex::Object &object, const Pex::Binary& pex);
    void writeFunction(int i, const Pex::Function &function, const Pex::Object &object,
                       const Pex::Binary &pex, const Pex::DebugInfo::FunctionInfo *functionInfo,
                       const std::string &name = "");

    void writeUserFlag(std::ostream &stream, const Pex::UserFlagged& flagged, const Pex::Binary& pex);
    void writeDocString(int i, const Pex::DocumentedItem& item);
    FunctionToWrite getFunctionToWrite(int i, const Pex::Function &function, const Pex::Object &object,
                                       const Pex::Binary &pex, const Pex::DebugInfo::FunctionInfo *functionInfo,
                                       const std::string &name = "");
    void store(const std::string& line);
    void store(std::ostream&& stream);
protected:
    bool m_CommentAsm;
    bool m_WriteHeader;
    bool m_TraceDecompilation;
    bool m_DumpTree;
    bool m_WriteDebugFuncs;
    bool m_PrintDebugLineNo;
    std::vector<std::string> m_Text;
    std::vector<FunctionToWrite> m_FunctionsToWrite;
    std::string m_OutputDir;



    bool isNativeObject(const Pex::Object &object, const Pex::Binary::ScriptType &scriptType) const;

    bool isCompilerGeneratedFunc(const std::string &name, const Pex::Object &object,
                                 Pex::Binary::ScriptType scriptType) const;

    static bool fixupFunction(const Pex::Object &object, const Pex::Binary &pex, const std::string &functionName,
                       Decompiler::PscDecompiler &decomp) ;

    static bool IsEvent(Pex::Binary::ScriptType scriptType, const std::string &functionName) ;
    size_t countDocStringLineNos(const Pex::DocumentedItem &item);
    size_t countStructLineNos(const Pex::StructInfo &structInfo, bool blank_lines);
    size_t countStructsLineNos(const Pex::Object &object, const Pex::Binary &pex, bool blank_lines);
    size_t countPropertiesLineNos(const Pex::Object& object, const Pex::Binary & pex, bool blank_lines);
    size_t countPropertyLineNos(const Pex::Property& prop, const Pex::Object& object, const Pex::Binary & pex, bool blank_lines);
    size_t countStatesLineNos(const Pex::Object& object, const Pex::Binary & pex, bool blank_lines);
    size_t countFunctionLineNos(const Pex::Function &function, const Pex::Object &object,
                                const Pex::Binary &pex, const Pex::DebugInfo::FunctionInfo *functionInfo, bool blank_lines);
    size_t countPropertyGroupLineNos(const Pex::DebugInfo::PropertyGroup &propGroup, const Pex::Object &object, const Pex::Binary &pex, bool blank_lines);

    bool isPropAutoReadOnly(const Pex::Property &prop) const;

    size_t countStateLineNos(const Pex::State &state, const Pex::Object &object, const Pex::Binary &pex, bool blank_lines);

    Decompiler::PscCoder::PropertyToWrite
    makePropertyToWrite(int i, const Pex::Property &prop, const Pex::Object &object,
                        const Pex::Binary &pex);

    StatesToWrite makeStatesToWrite(const Pex::Object& object, const Pex::Binary& pex);
    Decompiler::PscCoder::PropGroupsToWrite makePropGroupsToWrite(const Pex::Object& object, const Pex::Binary& pex);
    std::string makeVariableToWrite(const Pex::Variable& var, const Pex::Binary& pex);

};
}
