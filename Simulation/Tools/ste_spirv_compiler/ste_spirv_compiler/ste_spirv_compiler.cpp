// ste_spirv_compiler.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ste_shader_factory.hpp"

#include <glslang/Public/ShaderLang.h>
#include <glslang/Include/ResourceLimits.h>
#include <SPIRV/Logger.h>
#include <SPIRV/GlslangToSpv.h>

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

void parse_glslang_errors(std::ostringstream &cout, std::string &&str) {
    static std::string prefixes[2] = { "error", "warning" };

    std::stringstream ss(str);
    std::string line;
    while (std::getline(ss, line, '\n')) {
        for (auto &prefix : prefixes) {
            auto l = prefix.length();
            if (line.length() < l + 3) 
                continue;

            auto line_prefix = line.substr(0, l);
            std::transform(line_prefix.begin(), line_prefix.end(), line_prefix.begin(), ::tolower);
            if (line_prefix == prefix && line[l] == ':') {
                while (::isspace(line[++l])) {}

                // Find first ':', ignoring path's ":/" and ":\"
                auto i1 = std::find(line.begin() + l, line.end(), ':');
                while (i1 != line.end() && std::next(i1) != line.end() && 
                    (*std::next(i1) == '/' || *std::next(i1) == '\\')) 
                    i1 = std::find(std::next(i1), line.end(), ':');
                if (i1 == line.end())
                    break;
                // Find second ':'
                const auto i2 = std::find(std::next(i1), line.end(), ':');
                if (i2 == line.end())
                    break;

                // Lower case path, and backslashes
                std::transform(line.begin() + l, i1, line.begin() + l, ::tolower);
                std::replace(line.begin() + l, i1, '/', '\\');

                // format error/warning message
                *i1 = '(';
                const std::string suffix = std::string(",0): ") + prefix + " " + static_cast<char>(::toupper(prefix[0])) + "1000";
                line.insert(i2, suffix.begin(), suffix.end());

                // Erase old prefix
                line.erase(0, l);

                break;
            }
        }

        cout << line << std::endl;
    }
}

bool CompileAndLinkShaderUnits(ShaderCompUnit compUnit, std::vector<unsigned int> &spirv, std::ostringstream &cout) {
    // keep track of what to free
    const std::vector<std::string> Processes;
    const std::vector<std::string> baseResourceSetBinding;

    const EShMessages messages = EShMsgDefault;
    const TBuiltInResource resource = DefaultTBuiltInResource;

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

    const int defaultVersion = 110;

    glslang::TShader::ForbidIncluder includer;
    if (!shader->parse(&resource, defaultVersion, false, messages, includer)) {
        parse_glslang_errors(cout, std::string(shader->getInfoLog()));
        parse_glslang_errors(cout, std::string(shader->getInfoDebugLog()));
        return false;
    }

    program.addShader(shader);

    if (!program.link(messages) || !program.mapIO()) {
        parse_glslang_errors(cout, std::string(shader->getInfoLog()));
        parse_glslang_errors(cout, std::string(shader->getInfoDebugLog()));
        return false;
    }

    parse_glslang_errors(cout, std::string(shader->getInfoLog()));
    parse_glslang_errors(cout, std::string(shader->getInfoDebugLog()));

    // Dump SPIR-V
    for (int stage = 0; stage < EShLangCount; ++stage) {
        if (program.getIntermediate(static_cast<EShLanguage>(stage))) {
            std::string warningsErrors;
            spv::SpvBuildLogger logger;
            glslang::SpvOptions spvOptions;
            glslang::GlslangToSpv(*program.getIntermediate(static_cast<EShLanguage>(stage)), 
                                  spirv, 
                                  &logger, &spvOptions);

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
    delete shader;

    return true;
}

struct compile_result_t {
    std::string cerr;
    unsigned code{ 0 };

    boost::filesystem::path path;

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
    unsigned ret = 0;
    std::vector<boost::filesystem::path> paths;

    if (argc != 2) {
        std::cerr << "Expected arguments: [path_prefix]" << std::endl;
        return 2;
    }
    
    auto prefix_path = parm_to_path(argv[1]);
    const boost::filesystem::path shader_binary_output_path = prefix_path / boost::filesystem::path(R"(Data\programs)");
    const boost::filesystem::path temp_path = prefix_path / boost::filesystem::path(R"(temp)");
    const boost::filesystem::path source_path = prefix_path / boost::filesystem::path(R"(src)");

    // Find all source files
    {
        for (auto it = boost::filesystem::recursive_directory_iterator(source_path); it != boost::filesystem::recursive_directory_iterator(); ++it) {
            auto ext = it->path().extension().string();
            if (!ext.length())
                continue;

            if (ext == std::string(".vert") ||
                ext == std::string(".frag") ||
                ext == std::string(".geom") ||
                ext == std::string(".comp") ||
                ext == std::string(".tesc") ||
                ext == std::string(".tese"))
                paths.push_back(it->path());
        }
    }

    // Make sure programs output directory exists
    if (!boost::filesystem::exists(shader_binary_output_path))
        boost::filesystem::create_directory(shader_binary_output_path);

    // Init
    ShInitialize();

    // Submit async tasks (in chunks)
    for (std::size_t path_idx = 0; path_idx < paths.size();) {
        std::vector<std::pair<std::string, std::future<compile_result_t>>> futures;
        std::mutex m;

        for (std::size_t t=0; path_idx < paths.size() && t<std::thread::hardware_concurrency()*2; ++path_idx, ++t) {
            const auto &path = paths[path_idx];

            auto name = path.stem().string();
            auto f = std::async(std::launch::async, [path, &temp_path, &shader_binary_output_path, &source_path, &m]() -> compile_result_t {
                auto name = path.stem().string();

                try {
                    if (!boost::filesystem::exists(path)) {
                        return compile_result_t(path, path.string() + ": Fatal error - Does not exists\n");
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
                    StE::shader_blob_header header;
                    std::string glsl_source = StE::ste_shader_factory::compile_shader(path,
                                                                                      source_path,
                                                                                      header);

                    // Create SPIR-v compiler
                    EShLanguage compiler_language;
                    switch (header.type) {
                    case StE::ste_shader_type::vertex_program:		compiler_language = EShLangVertex; break;
                    case StE::ste_shader_type::geometry_program:	        compiler_language = EShLangGeometry; break;
                    case StE::ste_shader_type::fragment_program:	        compiler_language = EShLangFragment; break;
                    case StE::ste_shader_type::compute_program:		compiler_language = EShLangCompute; break;
                    case StE::ste_shader_type::tesselation_control_program:		compiler_language = EShLangTessControl; break;
                    case StE::ste_shader_type::tesselation_evaluation_program:	compiler_language = EShLangTessEvaluation; break;
                    default:
                        return compile_result_t(path, path.string() + ": Fatal error - Not an StE shader file\n");
                    }

                    // Generate SPIR-v binary
                    std::vector<unsigned int> spirv;
                    std::ostringstream cout;
                    {
                        ShaderCompUnit compUnit(compiler_language);
                        compUnit.addString(name,
                                           glsl_source.data());

                        std::unique_lock<std::mutex> l(m);

                        CompileAndLinkShaderUnits(compUnit,
                                                  spirv,
                                                  cout);

                        if (!spirv.size()) {
                            return compile_result_t(path, cout.str());
                        }
                    }

                    // Write compiled module with StE header
                    {
                        std::ofstream of;
                        of.exceptions(of.exceptions() | std::ios::failbit | std::ifstream::badbit);
                        of.open(output.string(), std::ios::out | std::ios::binary);
                        of.write(reinterpret_cast<const char*>(&header), sizeof(header));
                        of.write(reinterpret_cast<const char*>(spirv.data()), sizeof(unsigned int) * spirv.size());
                    }

                    return result;
                }
                catch (std::exception e) {
                    return compile_result_t(path, path.string() + ": Fatal error - " + std::string(e.what()) + "\n");
                }
            });

            futures.emplace_back(path.filename().string(),
                                 std::move(f));
        }

        // Wait
        for (auto &f : futures) {
            const auto r = f.second.get();
            if (r.code == up_to_date_code) {
                continue;
            }

            std::cout << f.first << "..." << std::endl << std::flush;
            if (r.code != 0) {
                std::cout << r.cerr << std::flush;
                ret = 1;
            }
            else {
                auto output = shader_binary_output_path / r.path.filename();
                std::cout << "Binary installed " << output.string() << "." << std::endl << std::flush;
            }
        }
    }

    ShFinalize();

    return ret;
}
