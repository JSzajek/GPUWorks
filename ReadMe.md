# GPUWorks a GPGPU Plugin for Unreal Engine
GPUWorks provides an OpenCL/CUDA integration plugin for Unreal Engine. Enabling low-level access to GPU compute capabilities from within UE Projects. It forms a bridge between OpenCL/CUDA compute kernels and Unreal Engine data structures (i.e. UTexture, FRHIBuffer, etc.). Allowing the use of GPU compute across AMD, Intel, and NVIDIA hardware.

## Features
- Load/Compile and Execute OpenCL/CUDA program at runtime.
- Transfer data to and from OpenCL/CUDA buffers and images.
- Synchronize results into corresponding Unreal Engine UTextures types.
- Integrated Unit Test support using Unreal Engine's automation framework.
- Cross-platform design supporting Windows and Linux.

## Use Cases
- **Custom GPU Compute Shaders**: Bypass HLSL material limitations and graphics pipeline restrictions by using OpenCL/CUDA programs directly.
- **Offline Processing**: Use the GPU for mesh generation, image processing, or data analysis.
- **Physics or Simulation Pipelines**: Run GPU parallel computations (e.g. fluid, cloth, or particle systems).
- **Scientific or Data-Driven UE Apps**: Visualize and process large datasets on the GPU.

## Installation
1) Clone into your Unreal Engine project `Plugins/` folder.
    - Example: `git clone https://github.com/JSzajek/GPUWorks.git Plugins/GPUWorks`
2) Generate the Visual Studio project files (if necessary).

## Testing
The plugin utilizes Unreal Engine automation framework's SPECs(). It is possible to execute those test in two ways:
1) Command line execution:
   - `UnrealEditor-Cmd.exe <PROJECT_HERE.uproject> -ExecCmds="Automation RunTests GPUWorks Unit Test" -unattended -nopause`
2) Editor execution:
   - Open `Tools` -> `Session Frontend`. Navigate to the `Automation` tab.
   - Run the "GPUWorks Unit Test" tests.


## Core Classes
### GPUContext
Represents an OpenCL/CUDA context. Manages the lifetime of all OpenCL/CUDA resources (programs, buffers, images, etc.) and encapsulates the selected platform and device.
 - Automatically created or can be user-managed.
 - Central hub for GPU resource management.
> Blueprint Note: When Using Blueprints, a shared global context is provided automatically unless manually overridden. 

### GPUDevice
Represents an OpenCL/CUDA-capable device (GPU or CPU).
- Use this to query capabilities, memory size, supported extensions, etc.

### GPUProgram/GPUProgamAsset
Represents a compiled OpenCL shader program.
 - Program Asset source code is editable in Unreal Editor.
 - Supports multi-kernel source files.
> Editor Note: Source code text editor is provided in the Editor and provides both compilation checking and error logging to expedite development.


### GPUCommandQueue
Represents the command queue used to dispatch program execution and memory transfers.
- Created per context/device.
- Suuports async dispatches (if avaliable).
> Blueprint Note: When Using Blueprints, a shared global command queue is provided automatically unless manually overridden. 


### GPUBuffer
Represents a linear memory buffer (e.g. float[], int[], struct[]).
- Mappable to and from Unreal TArrays (i.e. TArray\<float>).
- Can be shared and transferred between CPU and GPU.
> Blueprint Note: Compatible functions are Upload and Readback.

### GPUImage
Represents a 2D or 3D image or image array.
- Ideal for texture style data and operations.
- Read/Write support.
- Compatible with Unreal Textures (UTexture2D, UTexture2DArray).

## Blueprints
### Control Paths & Program
![BP Control](./Resources/BP_Control.png)

### Buffer
![BP Buffer Creation](./Resources/BP_Buffer_Creation.png)
![BP Buffer Read](./Resources/BP_Buffer_Read.png)

### Image
![BP Image](./Resources/BP_Image.png)



## Editor
### Program Editor
![Editor](./Resources/Program_Editor.png)

## Example Usage
#### Program 
```
OpenCL::Device device;
OpenCL::Context context(device);

OpenCL::Program program(context, mDefaultDevice);
program.ReadFromString("__kernel void test() { }");
```

#### Buffer Creation
```
OpenCL::Device device;
OpenCL::Context context(device);

const size_t count = 10;
std::vector<float> src_data(count, 0.0f);
OpenCL::Buffer buffer(context, 
                      src_data.data(), 
                      count * sizeof(float), 
                      OpenCL::AccessType::READ_WRITE,
                      OpenCL::MemoryStrategy::STREAM);
```

#### Texture Creation
```
OpenCL::Device device;
OpenCL::Context context(device);

OpenCL::Image cltexture(context,
                        mDefaultDevice,
                        256, 
                        256, 
                        1,		
                        OpenCL::Image::Format::RGBA8, 
                        OpenCL::Image::Type::Texture2D,
                        OpenCL::AccessType::READ_WRITE);
```

### Dependencies
 - OpenCL 1.2 or later (avaliable via CPU or GPU drivers).
 - Unreal Engine 5.0+.
 - C++20 compatible compiler.

### Future Roadmap
- [ ] Texture3D Support
- [ ] Benchmarking Framework
- [ ] Asynchronous Data Transfer
- [ ] Vulkan/DirectX12 Interop

### License
Licenses Under the **Apache 2.0** License.