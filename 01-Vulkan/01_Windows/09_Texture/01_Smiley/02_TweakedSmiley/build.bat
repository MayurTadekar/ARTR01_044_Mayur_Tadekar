cls

glslangValidator -V -o shader.vert.spv shaders\shader.vert

glslangValidator -V -o shader.frag.spv shaders\shader.frag

cl /EHsc /I C:\VulkanSDK\Vulkan\Include /c VK.cpp

rc VK.rc

link VK.obj VK.res /LIBPATH:C:\VulkanSDK\Vulkan\Lib user32.lib gdi32.lib vulkan-1.lib /SUBSYSTEM:WINDOWS

VK.exe
