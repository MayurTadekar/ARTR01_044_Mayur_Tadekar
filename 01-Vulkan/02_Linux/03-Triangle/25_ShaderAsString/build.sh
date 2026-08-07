clear

GLSLANG_CFLAGS="$(pkg-config --cflags glslang 2>/dev/null)"
GLSLANG_LIBS="$(pkg-config --libs glslang 2>/dev/null)"
if [ -z "$GLSLANG_LIBS" ]; then
	GLSLANG_LIBS="-lglslang -lMachineIndependent -lGenericCodeGen -lOSDependent -lSPIRV -lSPIRV-Tools -lSPIRV-Tools-opt -lglslang-default-resource-limits"
fi

# Compiling the source file to create an object file
g++ -c $GLSLANG_CFLAGS -I $HOME/VulkanSDK/vulkan/x86_64/include -o vk.o vk.cpp

# Linking the object file to create the executable
g++ -o vk vk.o -lX11 -lm -L $HOME/VulkanSDK/vulkan/x86_64/lib -lvulkan $GLSLANG_LIBS
