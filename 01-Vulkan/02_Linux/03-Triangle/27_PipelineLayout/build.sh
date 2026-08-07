clear

# Compiling the source file to create an object file
g++ -c -I $HOME/VulkanSDK/vulkan/x86_64/include -o vk.o vk.cpp

# Linking the object file to create the executable
g++ -o vk vk.o -lX11 -lm -L $HOME/VulkanSDK/vulkan/x86_64/lib -lvulkan
