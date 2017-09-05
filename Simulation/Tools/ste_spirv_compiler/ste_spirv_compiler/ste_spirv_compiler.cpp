// ste_spirv_compiler.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ste_shader_factory.hpp"

#include "../glslang/build/install/include/glslang/Public/ShaderLang.h"
#include "../glslang/build/install/include/glslang/Include/ResourceLimits.h"
#include "../glslang/build/install/include/SPIRV/Logger.h"
#include "../glslang/build/install/include/SPIRV/GlslangToSpv.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>

#include <iostream>
#include <future>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <cstdlib>

// Command-line options
enum TOptions {
    EOptionNone = 0,
    EOptionIntermediate = (1 << 0),
    EOptionSuppressInfolog = (1 << 1),
    EOptionMemoryLeakMode = (1 << 2),
    EOptionRelaxedErrors = (1 << 3),
    EOptionGiveWarnings = (1 << 4),
    EOptionLinkProgram = (1 << 5),
    EOptionMultiThreaded = (1 << 6),
    EOptionDumpConfig = (1 << 7),
    EOptionDumpReflection = (1 << 8),
    EOptionSuppressWarnings = (1 << 9),
    EOptionDumpVersions = (1 << 10),
    EOptionSpv = (1 << 11),
    EOptionHumanReadableSpv = (1 << 12),
    EOptionVulkanRules = (1 << 13),
    EOptionDefaultDesktop = (1 << 14),
    EOptionOutputPreprocessed = (1 << 15),
    EOptionOutputHexadecimal = (1 << 16),
    EOptionReadHlsl = (1 << 17),
    EOptionCascadingErrors = (1 << 18),
    EOptionAutoMapBindings = (1 << 19),
    EOptionFlattenUniformArrays = (1 << 20),
    EOptionNoStorageFormat = (1 << 21),
    EOptionKeepUncalled = (1 << 22),
    EOptionHlslOffsets = (1 << 23),
    EOptionHlslIoMapping = (1 << 24),
    EOptionAutoMapLocations = (1 << 25),
    EOptionDebug = (1 << 26),
};

const TBuiltInResource DefaultTBuiltInResource = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .limits = */{
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    } 
};

// Simple bundling of what makes a compilation unit for ease in passing around,
// and separation of handling file IO versus API (programmatic) compilation.
struct ShaderCompUnit {
    EShLanguage stage;
    static const int maxCount = 1;
    int count;                          // live number of strings/names
    const char* text[maxCount];         // memory owned/managed externally
    std::string fileName[maxCount];     // hold's the memory, but...
    const char* fileNameList[maxCount]; // downstream interface wants pointers

    ShaderCompUnit(EShLanguage stage) : stage(stage), count(0) { }

    ShaderCompUnit(const ShaderCompUnit& rhs)
    {
        stage = rhs.stage;
        count = rhs.count;
        for (int i = 0; i < count; ++i) {
            fileName[i] = rhs.fileName[i];
            text[i] = rhs.text[i];
            fileNameList[i] = rhs.fileName[i].c_str();
        }
    }

    void addString(std::string& ifileName, const char* itext)
    {
        assert(count < maxCount);
        fileName[count] = ifileName;
        text[count] = itext;
        fileNameList[count] = fileName[count].c_str();
        ++count;
    }
};

//
// For linking mode: Will independently parse each compilation unit, but then put them
// in the same program and link them together, making at most one linked module per
// pipeline stage.
//
// Uses the new C++ interface instead of the old handle-based interface.
//

bool CompileAndLinkShaderUnits(ShaderCompUnit compUnit, std::vector<unsigned int> &spirv, std::ostringstream &cout) {
    // keep track of what to free
    std::list<glslang::TShader*> shaders;
    std::vector<std::string> Processes;
    std::vector<std::string> baseResourceSetBinding;

    EShMessages messages = EShMsgDefault;
    TBuiltInResource resource = DefaultTBuiltInResource;

    //
    // Per-shader processing...
    //

    glslang::TProgram& program = *new glslang::TProgram;
    glslang::TShader* shader = new glslang::TShader(compUnit.stage);
    shader->setStringsWithLengthsAndNames(compUnit.text, nullptr, compUnit.fileNameList, compUnit.count);
//        if (entryPointName) // HLSL todo: this needs to be tracked per compUnits
//            shader->setEntryPoint(entryPointName);
//        if (sourceEntryPointName)
//            shader->setSourceEntryPoint(sourceEntryPointName);
//        if (UserPreamble.isSet())
//            shader->setPreamble(UserPreamble.get());
    shader->addProcesses(Processes);

    shader->setShiftSamplerBinding(0);
    shader->setShiftTextureBinding(0);
    shader->setShiftImageBinding(0);
    shader->setShiftUboBinding(0);
    shader->setShiftSsboBinding(0);
    shader->setShiftUavBinding(0);
    shader->setFlattenUniformArrays(false);
    shader->setNoStorageFormat(false);
    shader->setResourceSetBinding(baseResourceSetBinding);

    shader->setEnvInput(glslang::EShSourceGlsl,
                        compUnit.stage, glslang::EShClientOpenGL, 110);
    shader->setEnvClient(glslang::EShClientVulkan, 150);
    shader->setEnvTarget(glslang::EshTargetSpv, 100);           // Target Vulkan/SPIR-v !

    shaders.push_back(shader);

    const int defaultVersion = 110;

    glslang::TShader::ForbidIncluder includer;
    if (!shader->parse(&resource, defaultVersion, false, messages, includer)) {
        cout << shader->getInfoLog();
        cout << shader->getInfoDebugLog();
        return false;
    }

    program.addShader(shader);

//        if (!(Options & EOptionSuppressInfolog) &&
//            !(Options & EOptionMemoryLeakMode)) {
//            PutsIfNonEmpty(compUnit.fileName[0].c_str());
//            PutsIfNonEmpty(shader->getInfoLog());
//            PutsIfNonEmpty(shader->getInfoDebugLog());
//        }

    if (!program.link(messages) || !program.mapIO()) {
        cout << shader->getInfoLog();
        cout << shader->getInfoDebugLog();
        return false;
    }

    cout << shader->getInfoLog();
    cout << shader->getInfoDebugLog();

    // Report
//    if (!(Options & EOptionSuppressInfolog) &&
//        !(Options & EOptionMemoryLeakMode)) {
//        PutsIfNonEmpty(program.getInfoLog());
//        PutsIfNonEmpty(program.getInfoDebugLog());
//    }
    
    // Dump SPIR-V
    for (int stage = 0; stage < EShLangCount; ++stage) {
        if (program.getIntermediate((EShLanguage)stage)) {
            std::string warningsErrors;
            spv::SpvBuildLogger logger;
            glslang::SpvOptions spvOptions;
            glslang::GlslangToSpv(*program.getIntermediate((EShLanguage)stage), spirv, &logger, &spvOptions);

            cout << logger.getAllMessages().c_str();

//                if (Options & EOptionHumanReadableSpv) {
//                    spv::Disassemble(std::cout, spirv);
//                }
        }
    }

    // Free everything up, program has to go before the shaders
    // because it might have merged stuff from the shaders, and
    // the stuff from the shaders has to have its destructors called
    // before the pools holding the memory in the shaders is freed.
    delete &program;
    while (shaders.size() > 0) {
        delete shaders.back();
        shaders.pop_back();
    }

    return true;
}

struct compile_result_t {
    std::string cerr;
    unsigned code{ 0 };

    boost::filesystem::path path;
    std::string glsl_source;
    StE::shader_blob_header header;

    compile_result_t() = default;
    explicit compile_result_t(boost::filesystem::path path, unsigned c) : code(c), path(path) {}
    explicit compile_result_t(boost::filesystem::path path, std::string s) : cerr(s), code(2), path(path) {}
};

static constexpr unsigned up_to_date_code = 0xda7e;

auto parm_to_path(std::string s) {
    if (s[0] == '\'' || s[0] == '\"') {
        s.erase(s.begin());
        s.erase(s.size() - 1, 1);
    }

    return boost::filesystem::path(s);
}

int main(int argc,
         char *argv[],
         char *envp[]) {
    std::vector<boost::filesystem::path> paths;

    if (argc != 3) {
        std::cerr << "Expected arguments: [path_prefix] [comma separated module paths]" << std::endl;
        return 2;
    }

    // Read
    auto prefix_path = parm_to_path(argv[1]);
    {
        std::string segment;
        const auto ar = std::string(argv[2]);
        std::istringstream src(ar);
        while (std::getline(src, segment, ',')) {
            if (segment.length() > 0)
                paths.push_back(parm_to_path(segment));
        }
    }

    const boost::filesystem::path shader_binary_output_path = prefix_path / boost::filesystem::path(R"(Data\programs)");
    const boost::filesystem::path temp_path = prefix_path / boost::filesystem::path(R"(temp)");
    const boost::filesystem::path source_path = prefix_path / boost::filesystem::path(R"(src)");

    if (!boost::filesystem::exists(shader_binary_output_path))
        boost::filesystem::create_directory(shader_binary_output_path);

    // Submit async tasks
    std::vector<std::pair<std::string, std::future<compile_result_t>>> futures;
    for (auto &path : paths) {
        auto name = path.stem().string();
        auto f = std::async(std::launch::async, [path, &temp_path, &shader_binary_output_path, &source_path]() -> compile_result_t {
            auto name = path.stem().string();

            try {
                if (!boost::filesystem::exists(path)) {
                    return compile_result_t(path, path.string() + ": Fatal error - Does not exists");
                }

                auto spirv_temp_output = temp_path / (std::string("compiled_blob_") + path.filename().string());
                auto output = shader_binary_output_path / path.filename();

                // Check modification time of shader and dependencies
                // Ignore if up-to-date
                if (boost::filesystem::exists(output)) {
                    const auto target_modification_time = std::chrono::system_clock::from_time_t(boost::filesystem::last_write_time(output));
                    const auto source_modification_time = StE::ste_shader_factory::shader_modification_time(path, source_path);
                    if (source_modification_time <= target_modification_time)
                        return compile_result_t(path, up_to_date_code);
                }

                // Compile StE shader
                compile_result_t result = compile_result_t(path, 0);
                result.glsl_source = StE::ste_shader_factory::compile_shader(path,
                                                                             source_path,
                                                                             result.header);

                return result;
            }
            catch (std::exception e) {
                return compile_result_t(path, path.string() + ": Fatal error - " + std::string(e.what()));
            }
        });

        futures.emplace_back(name, std::move(f));
    }

    // Wait
    unsigned ret = 0;
    for (auto &f : futures) {
        const auto r = f.second.get();
        auto name = r.path.stem().string();

        std::cout << name << "..." << std::endl;

        if (r.code == up_to_date_code) {
            std::cout << "Up to date" << std::endl;
            continue;
        }
        if (r.code != 0) {
            std::cerr << r.cerr << std::endl;
            ret = 2;
        }

        // Create SPIR-v compiler
        EShLanguage compiler_language;
        switch (r.header.type) {
        case StE::ste_shader_type::vertex_program:		compiler_language = EShLangVertex; break;
        case StE::ste_shader_type::geometry_program:	        compiler_language = EShLangGeometry; break;
        case StE::ste_shader_type::fragment_program:	        compiler_language = EShLangFragment; break;
        case StE::ste_shader_type::compute_program:		compiler_language = EShLangCompute; break;
        case StE::ste_shader_type::tesselation_control_program:		compiler_language = EShLangTessControl; break;
        case StE::ste_shader_type::tesselation_evaluation_program:	compiler_language = EShLangTessEvaluation; break;
        default:
            std::cerr << r.path.string() + ": Fatal error - Not an StE shader file" << std::endl;
            ret = 2;
            continue;
        }

        ShInitialize();

        ShaderCompUnit compUnit(compiler_language);
        compUnit.addString(name,
                           r.glsl_source.data());

        std::vector<unsigned int> spirv;
        std::ostringstream cout;
        CompileAndLinkShaderUnits(compUnit, spirv, cout);

        if (!spirv.size()) {
            std::cerr << cout.str();
            ret = 2;
            continue;
        }

        ShFinalize();

        // Write compiled module with StE header
        {
            auto output = shader_binary_output_path / r.path.filename();

            cout << "Writing " << output.string() << "..." << std::endl;

            std::ofstream of;
            of.exceptions(of.exceptions() | std::ios::failbit | std::ifstream::badbit);
            of.open(output.string(), std::ios::out | std::ios::binary);
            of.write(reinterpret_cast<const char*>(&r.header), sizeof(r.header));
            of.write(reinterpret_cast<const char*>(spirv.data()), sizeof(unsigned int) * spirv.size());
        }
    }

    return ret;
}
