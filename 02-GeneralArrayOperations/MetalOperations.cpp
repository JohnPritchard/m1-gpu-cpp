#include "MetalOperations.hpp"
#include <iostream>
#include <list>
#include <map>

MetalOperations::MetalOperations(MTL::Device *device)
{

    _mDevice = device;

    NS::Error *error = nullptr;

    // Load the shader files with a .metal file extension in the project
    auto filepath = NS::String::string("./ops.metallib", NS::ASCIIStringEncoding);
    MTL::Library *opLibrary = _mDevice->newLibrary(filepath, &error);

    if (opLibrary == nullptr)
    {
        std::cout << "Failed to find the default library. Error: "
                  << error->description()->utf8String() << std::endl;
        return;
    }

    // Get all function names
    auto fnNames = opLibrary->functionNames();

    std::cout << "Available Metal functions in 'ops.metallib':" << std::endl;

    for (size_t i = 0; i < fnNames->count(); i++)
    {

        auto name_nsstring = fnNames->object(i)->description();
        auto name_utf8 = name_nsstring->utf8String();

        // Output function to stdout
        std::cout << name_utf8 << std::endl;

        // Load function into a map
        functionMap[name_utf8] =
            (opLibrary->newFunction(name_nsstring));

        // Create pipeline from function
        functionPipelineMap[name_utf8] =
            _mDevice->newComputePipelineState(functionMap[name_utf8], &error);

        if (functionPipelineMap[name_utf8] == nullptr)
        {
            std::cout << "Failed to created pipeline state object for "
                      << name_utf8 << ", error "
                      << error->description()->utf8String() << std::endl;
            return;
        }
    }

    std::cout << std::endl;

    _mCommandQueue = _mDevice->newCommandQueue();
    if (_mCommandQueue == nullptr)
    {
        std::cout << "Failed to find the command queue." << std::endl;
        return;
    }
}

void MetalOperations::addArrays(const MTL::Buffer *x_array,
                                const MTL::Buffer *y_array,
                                MTL::Buffer *r_array,
                                size_t arrayLength)
{
    MTL::CommandBuffer *commandBuffer = _mCommandQueue->commandBuffer();
    assert(commandBuffer != nullptr);
    MTL::ComputeCommandEncoder *computeEncoder = commandBuffer->computeCommandEncoder();
    assert(computeEncoder != nullptr);

    // Encode the pipeline state object and its parameters.
    computeEncoder->setComputePipelineState(functionPipelineMap["add_arrays"]);
    computeEncoder->setBuffer(x_array, 0, 0);
    computeEncoder->setBuffer(y_array, 0, 1);
    computeEncoder->setBuffer(r_array, 0, 2);

    MTL::Size gridSize = MTL::Size::Make(arrayLength, 1, 1);

    // Calculate a threadgroup size.
    NS::UInteger threadGroupSize =
        functionPipelineMap["add_arrays"]->maxTotalThreadsPerThreadgroup();

    if (threadGroupSize > arrayLength)
    {
        threadGroupSize = arrayLength;
    }
    MTL::Size threadgroupSize = MTL::Size::Make(threadGroupSize, 1, 1);

    // Encode the compute command.
    computeEncoder->dispatchThreads(gridSize, threadgroupSize);
    computeEncoder->endEncoding();

    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();
}

void MetalOperations::addMultiply(const MTL::Buffer *x_array,
                                  const MTL::Buffer *y_array,
                                  MTL::Buffer *r_array,
                                  size_t arrayLength)
{
    // Example compound operator. Computes (x + y) * y.

    MTL::CommandBuffer *commandBuffer = _mCommandQueue->commandBuffer();
    assert(commandBuffer != nullptr);
    MTL::ComputeCommandEncoder *computeEncoder = commandBuffer->computeCommandEncoder();
    assert(computeEncoder != nullptr);

    MTL::Size gridSize = MTL::Size::Make(arrayLength, 1, 1);
    NS::UInteger threadGroupSize =
        functionPipelineMap["add_arrays"]->maxTotalThreadsPerThreadgroup();
    if (threadGroupSize > arrayLength)
        threadGroupSize = arrayLength;
    MTL::Size threadgroupSize = MTL::Size::Make(threadGroupSize, 1, 1);

    computeEncoder->setComputePipelineState(functionPipelineMap["add_arrays"]);
    computeEncoder->setBuffer(x_array, 0, 0);
    computeEncoder->setBuffer(y_array, 0, 1);
    computeEncoder->setBuffer(r_array, 0, 2);
    computeEncoder->dispatchThreads(gridSize, threadgroupSize);

    computeEncoder->setComputePipelineState(functionPipelineMap["multiply_arrays"]);
    computeEncoder->setBuffer(r_array, 0, 0);
    computeEncoder->setBuffer(y_array, 0, 1);
    computeEncoder->setBuffer(r_array, 0, 2);
    computeEncoder->dispatchThreads(gridSize, threadgroupSize);

    computeEncoder->endEncoding();
    commandBuffer->commit();

    commandBuffer->waitUntilCompleted();
}

void MetalOperations::multiplyArrays(const MTL::Buffer *x_array,
                                     const MTL::Buffer *y_array,
                                     MTL::Buffer *r_array,
                                     size_t arrayLength)
{

    MTL::CommandBuffer *commandBuffer = _mCommandQueue->commandBuffer();
    assert(commandBuffer != nullptr);
    MTL::ComputeCommandEncoder *computeEncoder = commandBuffer->computeCommandEncoder();
    assert(computeEncoder != nullptr);

    // Encode the pipeline state object and its parameters.
    computeEncoder->setComputePipelineState(functionPipelineMap["multiply_arrays"]);
    computeEncoder->setBuffer(x_array, 0, 0);
    computeEncoder->setBuffer(y_array, 0, 1);
    computeEncoder->setBuffer(r_array, 0, 2);

    MTL::Size gridSize = MTL::Size::Make(arrayLength, 1, 1);

    // Calculate a threadgroup size.
    NS::UInteger threadGroupSize =
        functionPipelineMap["multiply_arrays"]->maxTotalThreadsPerThreadgroup();

    if (threadGroupSize > arrayLength)
    {
        threadGroupSize = arrayLength;
    }
    MTL::Size threadgroupSize = MTL::Size::Make(threadGroupSize, 1, 1);

    // Encode the compute command.
    computeEncoder->dispatchThreads(gridSize, threadgroupSize);
    computeEncoder->endEncoding();

    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();
}

void MetalOperations::saxpyArrays(const MTL::Buffer *alpha,
                                  const MTL::Buffer *x_array,
                                  const MTL::Buffer *y_array,
                                  MTL::Buffer *r_array,
                                  size_t arrayLength)
{
    MTL::CommandBuffer *commandBuffer = _mCommandQueue->commandBuffer();
    assert(commandBuffer != nullptr);
    MTL::ComputeCommandEncoder *computeEncoder = commandBuffer->computeCommandEncoder();
    assert(computeEncoder != nullptr);

    // Encode the pipeline state object and its parameters.
    computeEncoder->setComputePipelineState(functionPipelineMap["saxpy"]);
    computeEncoder->setBuffer(alpha, 0, 0);
    computeEncoder->setBuffer(x_array, 0, 1);
    computeEncoder->setBuffer(y_array, 0, 2);
    computeEncoder->setBuffer(r_array, 0, 3);

    MTL::Size gridSize = MTL::Size::Make(arrayLength, 1, 1);

    // Calculate a threadgroup size.
    NS::UInteger threadGroupSize =
        functionPipelineMap["saxpy"]->maxTotalThreadsPerThreadgroup();

    if (threadGroupSize > arrayLength)
    {
        threadGroupSize = arrayLength;
    }
    MTL::Size threadgroupSize = MTL::Size::Make(threadGroupSize, 1, 1);

    // Encode the compute command.
    computeEncoder->dispatchThreads(gridSize, threadgroupSize);
    computeEncoder->endEncoding();

    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();
}

void MetalOperations::central_difference(const MTL::Buffer *delta,
                                         const MTL::Buffer *x_array,
                                         MTL::Buffer *r_array,
                                         size_t arrayLength)
{
    MTL::CommandBuffer *commandBuffer = _mCommandQueue->commandBuffer();
    assert(commandBuffer != nullptr);
    MTL::ComputeCommandEncoder *computeEncoder = commandBuffer->computeCommandEncoder();
    assert(computeEncoder != nullptr);

    // Encode the pipeline state object and its parameters.
    computeEncoder->setComputePipelineState(functionPipelineMap["central_difference"]);
    computeEncoder->setBuffer(delta, 0, 0);
    computeEncoder->setBuffer(x_array, 0, 1);
    computeEncoder->setBuffer(r_array, 0, 2);

    MTL::Size gridSize = MTL::Size::Make(arrayLength, 1, 1);

    // Calculate a threadgroup size.
    NS::UInteger threadGroupSize =
        functionPipelineMap["central_difference"]->maxTotalThreadsPerThreadgroup();

    if (threadGroupSize > arrayLength)
    {
        threadGroupSize = arrayLength;
    }
    MTL::Size threadgroupSize = MTL::Size::Make(threadGroupSize, 1, 1);

    // Encode the compute command.
    computeEncoder->dispatchThreads(gridSize, threadgroupSize);
    computeEncoder->endEncoding();

    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();
}