cls

glslangValidator -V -o shader.vert.spv shader.vert

glslangValidator -V -o shader.frag.spv shader.frag

rc VK.rc

@REM cl /EHsc /I C:\VulkanSDK\Vulkan\Include /c VK.cpp

nvcc.exe -I C:\VulkanSDK\Vulkan\Include -L C:\VulkanSDK\Vulkan\Lib -o VK.exe user32.lib gdi32.lib vulkan-1.lib vk.res vk.cu -diag-suppress 20012 -diag-suppress 20013 -diag-suppress 20014 -diag-suppress 20015

del VK.exp

del VK.lib

@REM link VK.obj VK.res /LIBPATH:C:\VulkanSDK\Vulkan\Lib user32.lib gdi32.lib vulkan-1.lib /SUBSYSTEM:WINDOWS

VK.exe
