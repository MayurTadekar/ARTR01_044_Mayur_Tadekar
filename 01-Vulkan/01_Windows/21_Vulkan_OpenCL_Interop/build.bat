cls

cl /EHsc /I C:\VulkanSDK\Vulkan\Include /I "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.8\include" /c VK.cpp

rc VK.rc

link VK.obj VK.res /LIBPATH:C:\VulkanSDK\Vulkan\Lib /LIBPATH:"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.8\lib\x64" user32.lib gdi32.lib vulkan-1.lib /SUBSYSTEM:WINDOWS

VK.exe
