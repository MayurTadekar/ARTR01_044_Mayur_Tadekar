cls

glslangValidator -V -o shader.vert.spv shader.vert

glslangValidator -V -o shader.tesc.spv shader.tesc

glslangValidator -V -o shader.tese.spv shader.tese

glslangValidator -V -o shader.frag.spv shader.frag

cl /EHsc /I C:\VulkanSDK\Vulkan\Include /c VK.cpp

rc VK.rc

link VK.obj VK.res /LIBPATH:C:\VulkanSDK\Vulkan\Lib user32.lib gdi32.lib vulkan-1.lib /SUBSYSTEM:WINDOWS

VK.exe
