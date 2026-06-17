cls

cl /MD /std:c++17 /EHsc /I C:\VulkanSDK\Vulkan\Include /I C:\VulkanSDK\Vulkan\Include\glslang /c VK.cpp

rc VK.rc

link VK.obj VK.res /LIBPATH:C:\VulkanSDK\Vulkan\Lib user32.lib gdi32.lib vulkan-1.lib /SUBSYSTEM:WINDOWS

VK.exe
