from pathlib import Path
import re

base = Path('/home/marshalling-void/Desktop/ARTR01/ARTR01_044_Mayur_Tadekar/01-Vulkan/02_Linux/02-Bluescreen')
source = (Path('/home/marshalling-void/Desktop/ARTR01/ARTR01_044_Mayur_Tadekar/01-Vulkan/02_Linux/03-Triangle/34_Perspective_Corrected_Perspective_Way/vk.cpp')).read_text()

source = source.replace('uint32_t swapchainImageCount = UINT32_MAX;', 'uint32_t swapchainImageCount = 0;')
source = source.replace('    vkClearColorValue.float32[0] = 0.0f;\n    vkClearColorValue.float32[1] = 0.0f;\n    vkClearColorValue.float32[2] = 0.0f;\n    vkClearColorValue.float32[3] = 1.0f;', '    vkClearColorValue.float32[0] = 0.0f;\n    vkClearColorValue.float32[1] = 0.0f;\n    vkClearColorValue.float32[2] = 1.0f;\n    vkClearColorValue.float32[3] = 1.0f;')

call_blocks = {
    'CreateVulkanInstance': """    //  Get Vulkan Instance
    vkResult = CreateVulkanInstance();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, \"Initialize()   =>  CreateVulkanInstance() failed\\n\");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, \"Initialize()   =>  CreateVulkanInstance() successful\\n\");
    }
""",
    'GetSupportedSurface': """    //  Create Vulkan Presentation Surface
    vkResult = GetSupportedSurface();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, \"Initialize()   =>  GetSupportedSurface() failed\\n\");
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, \"Initialize()   =>  GetSupportedSurface() successful\\n\");
    }
""",
    'GetPhysicalDevice': """    //  Step 05 - Physical Device - Enumerate & Select Physical Device and its Queue Family Index
    vkResult = GetPhysicalDevice();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, \"Initialize()    =>  GetPhysicalDevice() failed %d\\n\", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, \"Initialize()    =>  GetPhysicalDevice() successful\\n\");
    }
""",
    'PrintVkInfo': """    //  Step 06 - Print VK Info
    vkResult = PrintVkInfo();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, \"Initialize()    =>  PrintVkInfo() failed %d\\n\", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, \"Initialize()    =>  PrintVkInfo() successful\\n\");
    }
""",
    'CreateVulkanDevice': """    // Step 08 - Vulkan Device
    vkResult = CreateVulkanDevice();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateVulkanDevice() failed %d\\n\", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateVulkanDevice() successful\\n\");
    }
""",
    'GetDeviceQueue': """    GetDeviceQueue();
""",
    'CreateSwapchain': """    // Swapchain
    vkResult = CreateSwapchain(VK_FALSE);
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateSwapchain() failed %d\\n\", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateSwapchain() successful\\n\");
    }
""",
    'CreateImagesAndImageViews': """    // Swapchain Images and ImageViews
    vkResult = CreateImagesAndImageViews();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateImagesAndImageViews() failed %d\\n\", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateImagesAndImageViews() successful\\n\");
    }
""",
    'CreateCommandPool': """    // Create Command Pool
    vkResult = CreateCommandPool();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateCommandPool() failed %d\\n\", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateCommandPool() successful\\n\");
    }
""",
    'CreateCommandBuffers': """    // Create Command Buffers
    vkResult = CreateCommandBuffers();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateCommandBuffers() failed %d\\n\", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateCommandBuffers() successful\\n\");
    }
""",
    'CreateRenderPass': """    // Create RenderPass
    vkResult = CreateRenderPass();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateRenderPass() failed %d\\n\", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateRenderPass() successful\\n\");
    }
""",
    'CreateFrameBuffers': """    // Create Frame Buffer
    vkResult = CreateFrameBuffers();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateFrameBuffers() failed %d\\n\", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateFrameBuffers() successful\\n\");
    }
""",
    'CreateSemaphores': """    // Create Semaphores
    vkResult = CreateSemaphores();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateSemaphores() failed %d\\n\", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateSemaphores() successful\\n\");
    }
""",
    'CreateFences': """    // Create Semaphores
    vkResult = CreateFences();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateFences() failed %d\\n\", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, \"Initialize()    =>  CreateFences() successful\\n\");
    }
""",
    'BuildCommandBuffers': """    // Build Command Buffers
    vkResult = BuildCommandBuffers();
    if (VK_SUCCESS != vkResult)
    {
        fprintf(gpLogFile, \"Initialize()    =>  BuildCommandBuffers() failed %d\\n\", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpLogFile, \"Initialize()    =>  BuildCommandBuffers() successful\\n\");
    }
""",
}

proto_lines = {
    'CreateVulkanInstance': '    VkResult CreateVulkanInstance(void);',
    'GetSupportedSurface': '    VkResult GetSupportedSurface(void);',
    'GetPhysicalDevice': '    VkResult GetPhysicalDevice(void);',
    'PrintVkInfo': '    VkResult PrintVkInfo(void);',
    'CreateVulkanDevice': '    VkResult CreateVulkanDevice(void);',
    'GetDeviceQueue': '    void GetDeviceQueue(void);',
    'CreateSwapchain': '    VkResult CreateSwapchain(VkBool32);',
    'CreateImagesAndImageViews': '    VkResult CreateImagesAndImageViews(void);',
    'CreateCommandPool': '    VkResult CreateCommandPool(void);',
    'CreateCommandBuffers': '    VkResult CreateCommandBuffers(void);',
    'CreateVertexBuffer': '    VkResult CreateVertexBuffer(void);',
    'CreateUniformBuffer': '    VkResult CreateUniformBuffer(void);',
    'CreateShaders': '    VkResult CreateShaders(void);',
    'CreateDescriptorSetLayout': '    VkResult CreateDescriptorSetLayout(void);',
    'CreatePipelineLayout': '    VkResult CreatePipelineLayout(void);',
    'CreateDescriptorPool': '    VkResult CreateDescriptorPool(void);',
    'CreateDescriptorSet': '    VkResult CreateDescriptorSet(void);',
    'CreateRenderPass': '    VkResult CreateRenderPass(void);',
    'CreatePipeline': '    VkResult CreatePipeline(void);',
    'CreateFrameBuffers': '    VkResult CreateFrameBuffers(void);',
    'CreateSemaphores': '    VkResult CreateSemaphores(void);',
    'CreateFences': '    VkResult CreateFences(void);',
    'BuildCommandBuffers': '    VkResult BuildCommandBuffers(void);',
}

stages = [
    ('02-InstanceExtensions', 'Linux - Vulkan Instance Extensions', 'MST: Linux - Vulkan Instance Extensions', [], False, False),
    ('03-Instance', 'Linux - Vulkan Instance', 'MST: Linux - Vulkan Instance', ['CreateVulkanInstance'], False, False),
    ('04-PresentationSurface', 'Linux - Vulkan Presentation Surface', 'MST: Linux - Vulkan Presentation Surface', ['CreateVulkanInstance', 'GetSupportedSurface'], False, False),
    ('05-PhysicalDevice', 'Linux - Vulkan Physical Device', 'MST: Linux - Vulkan Physical Device', ['CreateVulkanInstance', 'GetSupportedSurface', 'GetPhysicalDevice'], False, False),
    ('06-PrintVKInfo', 'Linux - Vulkan Print VK Info', 'MST: Linux - Vulkan Print VK Info', ['CreateVulkanInstance', 'GetSupportedSurface', 'GetPhysicalDevice', 'PrintVkInfo'], False, False),
    ('07-DeviceExtensions', 'Linux - Vulkan Device Extensions', 'MST: Linux - Vulkan Device Extensions', ['CreateVulkanInstance', 'GetSupportedSurface', 'GetPhysicalDevice', 'PrintVkInfo'], False, False),
    ('08-VulkanDevice', 'Linux - Vulkan Device', 'MST: Linux - Vulkan Device', ['CreateVulkanInstance', 'GetSupportedSurface', 'GetPhysicalDevice', 'PrintVkInfo', 'CreateVulkanDevice'], False, False),
    ('09-DeviceQueue', 'Linux - Vulkan Device Queue', 'MST: Linux - Vulkan Device Queue', ['CreateVulkanInstance', 'GetSupportedSurface', 'GetPhysicalDevice', 'PrintVkInfo', 'CreateVulkanDevice', 'GetDeviceQueue'], False, False),
    ('10-SurfaceFormatAndSurfaceColorSpace', 'Linux - Vulkan Surface Format And Surface Color Space', 'MST: Linux - Vulkan Surface Format And Surface Color Space', ['CreateVulkanInstance', 'GetSupportedSurface', 'GetPhysicalDevice', 'PrintVkInfo', 'CreateVulkanDevice', 'GetDeviceQueue'], False, False),
    ('11-PresentationMode', 'Linux - Vulkan Presentation Mode', 'MST: Linux - Vulkan Presentation Mode', ['CreateVulkanInstance', 'GetSupportedSurface', 'GetPhysicalDevice', 'PrintVkInfo', 'CreateVulkanDevice', 'GetDeviceQueue'], False, False),
    ('12-Swapchain', 'Linux - Vulkan Swapchain', 'MST: Linux - Vulkan Swapchain', ['CreateVulkanInstance', 'GetSupportedSurface', 'GetPhysicalDevice', 'PrintVkInfo', 'CreateVulkanDevice', 'GetDeviceQueue', 'CreateSwapchain'], False, False),
    ('13-SwapchainImagesAndImageViews', 'Linux - Vulkan Swapchain Images And Image Views', 'MST: Linux - Vulkan Swapchain Images And Image Views', ['CreateVulkanInstance', 'GetSupportedSurface', 'GetPhysicalDevice', 'PrintVkInfo', 'CreateVulkanDevice', 'GetDeviceQueue', 'CreateSwapchain', 'CreateImagesAndImageViews'], False, False),
    ('14-CommandPool', 'Linux - Vulkan Command Pool', 'MST: Linux - Vulkan Command Pool', ['CreateVulkanInstance', 'GetSupportedSurface', 'GetPhysicalDevice', 'PrintVkInfo', 'CreateVulkanDevice', 'GetDeviceQueue', 'CreateSwapchain', 'CreateImagesAndImageViews', 'CreateCommandPool'], False, False),
    ('15-CommandBuffer', 'Linux - Vulkan Command Buffer', 'MST: Linux - Vulkan Command Buffer', ['CreateVulkanInstance', 'GetSupportedSurface', 'GetPhysicalDevice', 'PrintVkInfo', 'CreateVulkanDevice', 'GetDeviceQueue', 'CreateSwapchain', 'CreateImagesAndImageViews', 'CreateCommandPool', 'CreateCommandBuffers'], False, False),
    ('16-RenderPass', 'Linux - Vulkan Render Pass', 'MST: Linux - Vulkan Render Pass', ['CreateVulkanInstance', 'GetSupportedSurface', 'GetPhysicalDevice', 'PrintVkInfo', 'CreateVulkanDevice', 'GetDeviceQueue', 'CreateSwapchain', 'CreateImagesAndImageViews', 'CreateCommandPool', 'CreateCommandBuffers', 'CreateRenderPass'], False, False),
    ('17-Framebuffers', 'Linux - Vulkan Framebuffers', 'MST: Linux - Vulkan Framebuffers', ['CreateVulkanInstance', 'GetSupportedSurface', 'GetPhysicalDevice', 'PrintVkInfo', 'CreateVulkanDevice', 'GetDeviceQueue', 'CreateSwapchain', 'CreateImagesAndImageViews', 'CreateCommandPool', 'CreateCommandBuffers', 'CreateRenderPass', 'CreateFrameBuffers'], False, False),
    ('18-FencesAndSemaphores', 'Linux - Vulkan Fences And Semaphores', 'MST: Linux - Vulkan Fences And Semaphores', ['CreateVulkanInstance', 'GetSupportedSurface', 'GetPhysicalDevice', 'PrintVkInfo', 'CreateVulkanDevice', 'GetDeviceQueue', 'CreateSwapchain', 'CreateImagesAndImageViews', 'CreateCommandPool', 'CreateCommandBuffers', 'CreateRenderPass', 'CreateFrameBuffers', 'CreateSemaphores', 'CreateFences'], False, False),
    ('19-BuildCommandBuffers', 'Linux - Vulkan Build Command Buffers', 'MST: Linux - Vulkan Build Command Buffers', ['CreateVulkanInstance', 'GetSupportedSurface', 'GetPhysicalDevice', 'PrintVkInfo', 'CreateVulkanDevice', 'GetDeviceQueue', 'CreateSwapchain', 'CreateImagesAndImageViews', 'CreateCommandPool', 'CreateCommandBuffers', 'CreateRenderPass', 'CreateFrameBuffers', 'CreateSemaphores', 'CreateFences', 'BuildCommandBuffers'], True, False),
    ('20-Render', 'Linux - Vulkan Render', 'MST: Linux - Vulkan Render', ['CreateVulkanInstance', 'GetSupportedSurface', 'GetPhysicalDevice', 'PrintVkInfo', 'CreateVulkanDevice', 'GetDeviceQueue', 'CreateSwapchain', 'CreateImagesAndImageViews', 'CreateCommandPool', 'CreateCommandBuffers', 'CreateRenderPass', 'CreateFrameBuffers', 'CreateSemaphores', 'CreateFences', 'BuildCommandBuffers'], True, True),
    ('21-Validation', 'Linux - Vulkan Bluescreen Validation', 'MST: Linux - Vulkan Bluescreen Validation', ['CreateVulkanInstance', 'GetSupportedSurface', 'GetPhysicalDevice', 'PrintVkInfo', 'CreateVulkanDevice', 'GetDeviceQueue', 'CreateSwapchain', 'CreateImagesAndImageViews', 'CreateCommandPool', 'CreateCommandBuffers', 'CreateRenderPass', 'CreateFrameBuffers', 'CreateSemaphores', 'CreateFences', 'BuildCommandBuffers'], True, True),
]

def replace_initialize(content, enabled, do_render):
    match = re.search(r'VkResult initialize\(void\)\n\{.*?\n\}\n\nVkResult resize', content, flags=re.S)
    protos = '\n'.join(proto_lines[name] for name in enabled)
    calls = ''.join(call_blocks[name] for name in enabled)
    render_tail = ''
    if do_render:
        render_tail = """
    memset((void *)&vkClearColorValue, 0, sizeof(VkClearColorValue));

    // FINALLY GIVING BLUE COLOR - Analogous to glClearColor;
    vkClearColorValue.float32[0] = 0.0f;
    vkClearColorValue.float32[1] = 0.0f;
    vkClearColorValue.float32[2] = 1.0f;
    vkClearColorValue.float32[3] = 1.0f;

    // Initialization is Completed
    bInitialized = True;
"""

    new_block = f"""VkResult initialize(void)
{{
    // Function Declaration
{protos}

    // Variable Declaration
    VkResult vkResult = VK_SUCCESS;

    // Code
    if (True == bValidation)
    {{
        fprintf(gpLogFile, \"CreateVulkanInstance()    ||> VALIDATION \\n\");
    }}

    fprintf(gpLogFile, \"Initialize()    >>>  function started\\n\");

{calls}{render_tail}
    fprintf(gpLogFile, \"Initialize()    -->  function completed\\n\");
    return (vkResult);
}}

"""
    return content[:match.start()] + new_block + content[match.end()-len('VkResult resize(int width, int height)'):]

def stub_resize_display(content):
    content = re.sub(r'VkResult resize\(int width, int height\)\n\{.*?\n\}\n\nVkResult display\(void\)\n\{.*?\n\}\n\nvoid update\(void\)\n\{\n    // Code\n\}',
                     'VkResult resize(int width, int height)\n{\n    (void)width;\n    (void)height;\n    return (VK_SUCCESS);\n}\n\nVkResult display(void)\n{\n    return ((VkResult)VK_FALSE);\n}\n\nvoid update(void)\n{\n    // Code\n}',
                     content, flags=re.S)
    return content

def strip_uniform_update(content):
    return re.sub(r'\n    vkResult = UpdateUniformBuffer\(\);\n    if \(VK_SUCCESS != vkResult\)\n    \{\n        fprintf\(gpLogFile, "Display\(\)   => UpdateUniformBuffer\(\) failed\\n"\);\n        return \(vkResult\);\n    \}\n', '\n', content)

def clear_only_build(content):
    return re.sub(r'\n        vkCmdBeginRenderPass\(vkCommandBuffer_array\[le\], &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE\);\n        \{.*?\n        \}\n        vkCmdEndRenderPass\(vkCommandBuffer_array\[le\]\);',
                  '\n        vkCmdBeginRenderPass(vkCommandBuffer_array[le], &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);\n        vkCmdEndRenderPass(vkCommandBuffer_array[le]);',
                  content, flags=re.S)

for stage, brief, title, enabled, build_only, render_enabled in stages:
    content = source.replace('Linux - Vulkan Triangle Perspective Matrix Way', brief, 1)
    content = content.replace('MST: Linux - Perspective Triangle', title, 1)
    content = replace_initialize(content, enabled, render_enabled)
    if build_only:
        content = clear_only_build(content)
    if render_enabled:
        content = strip_uniform_update(content)
        content = stub_resize_display(content)
    else:
        content = stub_resize_display(content)
    (base / stage / 'vk.cpp').write_text(content)
