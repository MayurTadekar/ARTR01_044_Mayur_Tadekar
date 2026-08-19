/**************************************************
 * S.G.M.P
 * @file: canvas.js
 * @brief: Colored Cube 3D Rotation
 * @author: Mayur S. Tadekar(mrtadekar@gmail.com)
 * @date: 06 August 2026 (Thursday) 01:29:43 PM
 **************************************************/

var canvas = null;

var bFullscreen = false;

var canvasOriginalWidth = 0;
var canvasOriginalHeight = 0;

// WebGPU Related
var clearColor;

let device = null;
let context = null;
let queue = null;
let canvasFormat = null;
let animationFrameID = null;

let renderPipeline = null;
let perspectiveProjectionMatrix = null;

let bufferPositionPyramid = null;
let bufferColorPyramid = null;

let bufferPositionCube = null;
let bufferColorCube = null;

let bufferMVPUniformPyramid = null;
let bufferMVPUniformCube = null;

let bindgroupMVPUniformPyramid = null;
let bindgroupMVPUniformCube = null;
let depthTexture = null;

let rotate_angle = 0.0;

// Animation Related
var requestAnimationFrame =	window.requestAnimationFrame		||
							window.webkitRequestAnimationFrame	||
							window.mozRequestAnimationFrame		||
							window.oRequestAnimationFrame		||
							window.msRequestAnimationFrame;

var cancelAnimationFrame = 	window.cancelAnimationFrame					||
							window.webkitCancelRequestAnimationFrame	||
							window.webkitCancelAnimationFrame			||
							window.mozCancelRequestAnimationFrame		||
							window.mozCancelAnimationFrame				||
							window.oCancelRequestAnimationFrame			||
							window.oCancelAnimationFrame				||
							window.msCancelRequestAnimationFrame		||
							window.msCancelAnimationFrame;

// On load Function
async function main() {
	
	canvas = document.getElementById("MST");
	if(!canvas)
		console.log("Obtaining Canvas Failed")
	else
		console.log("Obtaining Canvas Succeded")

	console.log("Canvas Width = " + canvas.width + " Canvas Height = " + canvas.height)

	canvasOriginalWidth = canvas.width;
	canvasOriginalHeight = canvas.height;

	// Register Event Handlers
	window.addEventListener("keydown", keyDown, false);
	window.addEventListener("click", mouseDown, false);
	window.addEventListener("resize", resize, false);
	// Best Practise for WebGPU during Fullscreen
	document.addEventListener("fullscreenchange", onFullscreenChange, false);
	document.addEventListener("webkitfullscreenchange", onFullscreenChange, false);

	// Initialize WebGPU

	// STEPS 
	
	// STEP01 : Get GPU Interface
	const gpu = navigator.gpu;
	if(null == gpu)
	{
		console.log("WebGPU is not supported by this browser\n");
		throw new Error("WebGPU is not supported by this browser");
	}
	else
	{
		console.log("WebGPU is Supported by this browser\n");
	}

	// STEP02 : Get GPUAdapter Object from GPUInterface
	const adapter = await gpu.requestAdapter();
	if(null == adapter)
	{
		console.log("Failed to obtain GPUAdapter\n");
		throw new Error("Failed to obtain GPUAdapter");
	}
	else
	{
		console.log("GPUAdapter obtained successfully\n");
	}

	// STEP03 : Get GPUDevice Object from adapter
	device = await adapter.requestDevice();
	if(null == device)
	{
		console.log("Failed to obtain GPUDevice\n");
		throw new Error("Failed to obtain GPUDevice");
	}
	else
	{
		console.log("GPUDevice obtained successfully\n");
	}

	// STEP04 : As Browser's can be on Mobile Devices and other Devices, Devices may get lost
	// maybe due to reset, switchOff, switching, disconnection
	// In such circumstances, we may not have capturable error. So register one generic handler
	// for uncapturable errors with device
	device.addEventListener("uncapturederror", onUncapturedError);
	// Register a specific device loss handler
	device.lost.then(onDeviceLost);

	// Call Stub Functions from here
	initialize();

	resize();

	display();
}

function onUncapturedError(event)
{
	// Code
	console.error("WebGPU Uncaptured Error:", event.error.message);
}

function onDeviceLost(info)
{
	// Code
	console.warning("WebGPU Device Lost Reason:", info.reason, "message", info.message);
	
	bufferPositionPyramid = null;
	bufferColorPyramid = null;

	bufferPositionCube = null;
	bufferColorCube = null;

	bufferMVPUniformPyramid = null;
	bufferMVPUniformCube = null;

	bindgroupMVPUniformPyramid = null;
	bindgroupMVPUniformCube = null;
	
	device = null;
	queue = null;
	depthTexture = null;
}


function toggleFullscreen()
{
	var fullscreen_elements = 	document.fullscreenElement 			||
								document.webkitFullscreenElement	||
								document.mozkitFullscreenElement	||
								document.msFullscreenElement		||
								null;

	if(null == fullscreen_elements)
	{
		if( canvas.requestFullscreen)
		{
			canvas.requestFullscreen();
		}
		else if( canvas.webkitRequestFullscreen )
		{
			canvas.webkitRequestFullscreen();
		}
		else if( canvas.mozkitRequestFullscreen )
		{
			canvas.mozkitRequestFullscreen();
		}
		else if( canvas.mskitRequestFullscreen )
		{
			canvas.mskitRequestFullscreen();
		}

		// In WebGL we initialized bFullscreen = here, Not thinking about async operations
		// But in WebGPU, Considering possibility of Cross Browser Fulscreen Compatibility.
		// We will do this on fullscreen change
	}
	else
	{
		if( document.exitFullscreen )
		{
			document.exitFullscreen()
		}
		else if( document.webkitExitFullscreen )
		{
			document.webkitExitFullscreen()
		}
		else if( document.mozCancelFullscreen )
		{
			document.mozCancelFullscreen()
		}
		else if( document.msExitFullscreen )
		{
			document.msExitFullscreen()
		}

		// In WebGL we initialized bFullscreen = here, Not thinking about async operations
		// But in WebGPU, Considering possibility of Cross Browser Fulscreen Compatibility.
		// We will do this on fullscreen change
	}
}

function onFullscreenChange()
{
	// Code
	var fullscreen_elements = 	document.fullscreenElement 			||
								document.webkitFullscreenElement	||
								document.mozkitFullscreenElement	||
								document.msFullscreenElement		||
								null;

	if(null != fullscreen_elements)
	{
		bFullscreen = true;
	}
	else
	{
		bFullscreen = false;
	}

	// Call resize here
	resize();
}

function initialize()
{
	// Code

	// After Receiving GPUInterface, Adapter and Device, Get GPUQueue Object from device
	// remember getting queue is always SYNCHRONOUS, never fails if we already have GPU, Adapter and Device
	// So there is no need for await, nor error checking
	queue = device.queue;
	console.log("WebGPU Queue obtained successfully:", queue);

	// STEP05 : Now get the WebGPU Context
	context = canvas.getContext("webgpu");
	if(null == context)
	{
		console.log("Failed to obtain WebGPU Context");
		throw new Error("Failed to obtain WebGPU Context");
	}

	// STEP06 : Get the preferred WebGPU color format for canvas whose type is GPUTextureFormat is a String Representation
	canvasFormat = navigator.gpu.getPreferredCanvasFormat();

	// STEP07 : Configure the Canvas with the preferred format to become functionable and to suit our needs
	const canvasConfiguration = 
	{
		device: device,
		format: canvasFormat,
		usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.COPY_SRC,
		alphaMode: "opaque"
	};

	context.configure(canvasConfiguration);
	console.log("WebGPU Canvas configured successfully:", canvasConfiguration, "\n");

	// Vertex Shader Source Code in WGSL
	const vertexShaderSourceCode = `
		struct MVPUniform
		{
			mvpMatrix: mat4x4<f32>
		};

		struct VertexOutput
		{
			@builtin(position) ovPosition: vec4<f32>,
			@location(0) ovColor: vec4<f32>
		};

		@group(0) @binding(0) var<uniform> mvpUniform: MVPUniform;
		
		@vertex fn main(@location(0) vpos : vec4<f32>, @location(1) vcolor : vec4<f32>) -> VertexOutput
		{
			var output: VertexOutput;
			let vPosition = mvpUniform.mvpMatrix * vpos;

			output.ovPosition = vPosition;
			output.ovColor = vcolor;
			return(output);
		}
	`;

	const shaderModuleDescriptor_vertexShader = 
	{
		code: vertexShaderSourceCode
	};

	//  GPUShaderModule
	const shaderModule_vertexShader = device.createShaderModule(shaderModuleDescriptor_vertexShader);
	if(null == shaderModule_vertexShader)
	{
		console.log("Failed to create Vertex Shader Module");
		throw new Error("Failed to create Vertex Shader Module");
	}
	else
	{
		console.log("Vertex Shader Module created successfully");
	}

	// Fragment Shader Source Code in WGSL
	const fragmentShaderSourceCode = `

		struct VertexOutput
		{
			@builtin(position) ovPosition: vec4<f32>,
			@location(0) ovColor: vec4<f32>
		};

		@fragment fn main(output: VertexOutput) -> @location(0) vec4<f32>
		{
			return(output.ovColor);
		}
	`;

	const shaderModuleDescriptor_fragmentShader = 
	{
		code: fragmentShaderSourceCode
	};

	//  GPUShaderModule
	const shaderModule_fragmentShader = device.createShaderModule(shaderModuleDescriptor_fragmentShader);
	if(null == shaderModule_fragmentShader)
	{
		console.log("Failed to create Fragment Shader Module");
		throw new Error("Failed to create Fragment Shader Module");
	}
	else
	{
		console.log("Fragment Shader Module created successfully");
	}

	// Declare Position array
	const vertexPositionPyramid = new Float32Array([
		0.0, 1.0, 0.0, 1.0,
		-1.0, -1.0, 1.0, 1.0,
		1.0, -1.0, 1.0, 1.0,

		0.0, 1.0, 0.0, 1.0,
		1.0, -1.0, 1.0, 1.0,
		1.0, -1.0, -1.0, 1.0,

		0.0, 1.0, 0.0, 1.0,
		-1.0, -1.0, -1.0, 1.0,
		1.0, -1.0, -1.0, 1.0,

		0.0, 1.0, 0.0, 1.0,
		-1.0, -1.0, 1.0, 1.0,
		-1.0, -1.0, -1.0, 1.0,
	]);

	const vertexColorPyramid = new Float32Array([
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,

		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,

		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,

		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
	]);

	const vertexPositionCube = new Float32Array([
		// Front
		-1.0, 1.0, 1.0, 1.0,
		-1.0, -1.0, 1.0, 1.0,
		1.0, -1.0, 1.0, 1.0,
		-1.0, 1.0, 1.0, 1.0,
		1.0, -1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,

		// Back
		-1.0, 1.0, -1.0, 1.0,
		-1.0, -1.0, -1.0, 1.0,
		1.0, -1.0, -1.0, 1.0,
		-1.0, 1.0, -1.0, 1.0,
		1.0, -1.0, -1.0, 1.0,
		1.0, 1.0, -1.0, 1.0,

		// Right
		1.0, 1.0, 1.0, 1.0,
		1.0, -1.0, 1.0, 1.0,
		1.0, -1.0, -1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, -1.0, -1.0, 1.0,
		1.0, 1.0, -1.0, 1.0,

		// Left
		-1.0, 1.0, 1.0, 1.0,
		-1.0, -1.0, 1.0, 1.0,
		-1.0, -1.0, -1.0, 1.0,
		-1.0, 1.0, 1.0, 1.0,
		-1.0, -1.0, -1.0, 1.0,
		-1.0, 1.0, -1.0, 1.0,

		// Top
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, -1.0, 1.0,
		-1.0, 1.0, -1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		-1.0, 1.0, -1.0, 1.0,
		-1.0, 1.0, 1.0, 1.0,

		// Bottom
		1.0, -1.0, 1.0, 1.0,
		1.0, -1.0, -1.0, 1.0,
		-1.0, -1.0, -1.0, 1.0,
		1.0, -1.0, 1.0, 1.0,
		-1.0, -1.0, -1.0, 1.0,
		-1.0, -1.0, 1.0, 1.0,
	]);

	const vertexColorCube = new Float32Array([
		// Front
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,

		// Back
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,

		// Right
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,

		// Left
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,


		// Top
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		
		// Bottom
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
	]);

	// As we have only 1 MVP Uniform, The BindGroupLayout is common/shared for both Pyramid and Cube

	// Pyramid: Its PositionBuffer, Its UniformBuffer, Its BindGroup
	let bindGroupLayoutEntryMVPUniformPyramid;
	{
		// GPUBindGroupLayoutEntry
		bindGroupLayoutEntryMVPUniformPyramid = createBindGroupLayoutUniform(0, 
																			GPUShaderStage.VERTEX, 
																			"uniform");
		// GPUBufferDescriptor
		const bufferDescriptor_Position_Pyramid = 
		{
			size: vertexPositionPyramid.byteLength,
			usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
		};

		// GPUBuffer
		bufferPositionPyramid = device.createBuffer(bufferDescriptor_Position_Pyramid);
		if(null == bufferPositionPyramid)
		{
			console.log("Failed to create Position Buffer for Pyramid");
			throw new Error("Failed to create Position Buffer for Pyramid");
		}
		else
		{
			console.log("Position Buffer for Pyramid created successfully");
		}

		// Write position array data to position buffer
		queue.writeBuffer(bufferPositionPyramid, 0, vertexPositionPyramid, 0, vertexPositionPyramid.length);
		console.log("Position data written to Pyramid buffer");

		// GPUBufferDescriptor for Color Buffer
		const bufferDescriptor_Color_Pyramid = 
		{
			size: vertexColorPyramid.byteLength,
			usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
		};

		// GPUBuffer for Color
		bufferColorPyramid = device.createBuffer(bufferDescriptor_Color_Pyramid);
		if(null == bufferColorPyramid)
		{
			console.log("Failed to create Color Buffer for Pyramid");
			throw new Error("Failed to create Color Buffer for Pyramid");
		}
		else
		{
			console.log("Color Buffer for Pyramid created successfully");
		}

		// Write color array data to color buffer
		queue.writeBuffer(bufferColorPyramid, 0, vertexColorPyramid, 0, vertexColorPyramid.length);
		console.log("Color data written to Pyramid buffer");

		// MVPUniform Buffer for Pyramid
		const mvpUniformBufferSize = 16 * 4; // 4x4 matrix of floats
		bufferMVPUniformPyramid = createUniformBuffer(mvpUniformBufferSize, GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST);

		// Create Bind Group For MVPUniform Buffer for Pyramid
		bindgroupMVPUniformPyramid = createBindGroupForUniform(bufferMVPUniformPyramid,
																0,
																mvpUniformBufferSize,
																0,
																bindGroupLayoutEntryMVPUniformPyramid
		);
	}

	// Cube: Its PositionBuffer, Its UniformBuffer, Its BindGroup
	let bindGroupLayoutEntryMVPUniformCube;
	{
		// GPUBindGroupLayoutEntry
		bindGroupLayoutEntryMVPUniformCube = createBindGroupLayoutUniform(0, 
																			GPUShaderStage.VERTEX, 
																			"uniform");
		// GPUBufferDescriptor
		const bufferDescriptor_Position_Cube = 
		{
			size: vertexPositionCube.byteLength,
			usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
		};

		// GPUBuffer
		bufferPositionCube = device.createBuffer(bufferDescriptor_Position_Cube);
		if(null == bufferPositionCube)
		{
			console.log("Failed to create Position Buffer for Cube");
			throw new Error("Failed to create Position Buffer for Cube");
		}
		else
		{
			console.log("Position Buffer for Cube created successfully");
		}

		// Write position array data to position buffer
		queue.writeBuffer(bufferPositionCube, 0, vertexPositionCube, 0, vertexPositionCube.length);
		console.log("Position data written to Cube buffer");

		// GPUBufferDescriptor for Color Buffer
		const bufferDescriptor_Color_Cube = 
		{
			size: vertexColorCube.byteLength,
			usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
		};

		// GPUBuffer for Color
		bufferColorCube = device.createBuffer(bufferDescriptor_Color_Cube);
		if(null == bufferColorCube)
		{
			console.log("Failed to create Color Buffer for Cube");
			throw new Error("Failed to create Color Buffer for Cube");
		}
		else
		{
			console.log("Color Buffer for Cube created successfully");
		}

		// Write color array data to color buffer
		queue.writeBuffer(bufferColorCube, 0, vertexColorCube, 0, vertexColorCube.length);
		console.log("Color data written to Cube buffer");

		// MVPUniform Buffer for Cube
		const mvpUniformBufferSize = 16 * 4; // 4x4 matrix of floats
		bufferMVPUniformCube = createUniformBuffer(mvpUniformBufferSize, GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST);

		// Create Bind Group For MVPUniform Buffer for Cube
		bindgroupMVPUniformCube = createBindGroupForUniform(bufferMVPUniformCube,
																0,
																mvpUniformBufferSize,
																0,
																bindGroupLayoutEntryMVPUniformCube
		);
	}

	
	// const bindGroupLayoutEntry_mvpUniform = 
	// {
	// 	binding: 0,		// matches with 0th Group's 0th Slot binding in Vertex Shader
	// 	visibility: GPUShaderStage.VERTEX,
	// 	buffer: 
	// 	{
	// 		type: "uniform"
	// 	}
	// };

	// // GPUBindGroupLayoutDescriptor
	// const bindGroupLayoutDescriptor = 
	// {
	// 	entries: [bindGroupLayoutEntry_mvpUniform]
	// };

	// // GPUBindGroupLayout
	// const bindGroupLayout_mvpUniform = device.createBindGroupLayout(bindGroupLayoutDescriptor);
	// if(null == bindGroupLayout_mvpUniform)
	// {
	// 	console.log("Failed to create Bind Group Layout");
	// 	throw new Error("Failed to create Bind Group Layout");
	// }
	// else
	// {
	// 	console.log("Bind Group Layout created successfully");
	// }

	// To Create Pipeline Layout we will create PipelineLayoutDescriptor
	// GPUPipelineLayoutDescriptor
	const pipelineLayoutDescriptor = 
	{
		bindGroupLayouts: [bindGroupLayoutEntryMVPUniformPyramid]
	};

	const pipelineLayout = device.createPipelineLayout(pipelineLayoutDescriptor);
	if(null == pipelineLayout)
	{
		console.log("Failed to create Pipeline Layout");
		throw new Error("Failed to create Pipeline Layout");
	}
	else
	{
		console.log("Pipeline Layout created successfully");
	}

	// // Create Uniform Buffer
	// const mvpUniformBufferDescriptor = 
	// {
	// 	size: mvpUniformBufferSize,
	// 	usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST
	// };
	// buffermvpUniform = device.createBuffer(mvpUniformBufferDescriptor);
	// if(null == buffermvpUniform)
	// {
	// 	console.log("Failed to create Uniform Buffer");
	// 	throw new Error("Failed to create Uniform Buffer");
	// }
	// else
	// {
	// 	console.log("Uniform Buffer created successfully");
	// }

	// // GPUBufferBinding
	// const bufferBinding_mvpUniform = 
	// {
	// 	buffer: buffermvpUniform,
	// 	offset: 0,
	// 	size: mvpUniformBufferSize
	// };

	// // GPUBindGroupEntry
	// const bindGroupEntry_mvpUniform = 
	// {
	// 	binding: 0,
	// 	resource: bufferBinding_mvpUniform
	// };

	// // GPUBindGroupDescriptor
	// const bindGroupDescriptor_mvpUniform = 
	// {
	// 	layout: bindGroupLayout_mvpUniform,
	// 	entries: [bindGroupEntry_mvpUniform]
	// };

	// // GPUBindGroup
	// bindingGroupMVPUniform = device.createBindGroup(bindGroupDescriptor_mvpUniform);
	// if(null == bindingGroupMVPUniform)
	// {
	// 	console.log("Failed to create Bind Group");
	// 	throw new Error("Failed to create Bind Group");
	// }
	// else
	// {
	// 	console.log("Bind Group created successfully");
	// }

	// Pipeline State Object - PSO

	// GPUVertexAttribute
	const positionVertexAttribute = 
	{
		shaderLocation: 0,	// matches with location 0 in vertex shader's main() parameter
		offset: 0,
		format: "float32x4"
	};

	// GPUVertexBufferLayout
	const positionVertexBufferLayout = 
	{
		attributes: [positionVertexAttribute],
		arrayStride: 4 * 4, // 4 floats for each vertex position
		stepMode: "vertex"
	};

	// GPUVertexAttribute for Color
	const colorVertexAttribute = 
	{
		shaderLocation: 1,	// matches with location 1 in vertex shader's main() parameter
		offset: 0,
		format: "float32x4"
	};

	// GPUVertexBufferLayout for Color
	const colorVertexBufferLayout = 
	{
		attributes: [colorVertexAttribute],
		arrayStride: 4 * 4, // 4 floats for each vertex color
		stepMode: "vertex"
	};

	// GPUVertexState
	const vertexShaderState = 
	{
		module: shaderModule_vertexShader,
		entryPoint: "main",
		buffers: [positionVertexBufferLayout, colorVertexBufferLayout],
	};

	// GPUColorTargetState
	const colorTargetState = 
	{
		format: canvasFormat,
		blend: {
			color: { srcFactor: "src-alpha", dstFactor: "one-minus-src-alpha" },
			alpha: { srcFactor: "src-alpha", dstFactor: "one-minus-src-alpha" }
		},
		writeMask: GPUColorWrite.ALL
	};

	// GPUFragmentState
	const fragmentShaderState = 
	{
		module: shaderModule_fragmentShader,
		entryPoint: "main",
		targets: [colorTargetState]
	};

	// GPUPrimitiveState
	const primitiveState =
	{
		frontFace: "ccw", // CounterClockwise
		cullMode: "none", // No Culling
		topology: "triangle-list",
	};

	// Depth Stencil State
	// GPUDepthStencilState
	const depthStencilState = 
	{
		depthWriteEnabled: true,
		depthCompare: "less-equal",	// GL_LEQUAL
		format: "depth24plus-stencil8",
	};

	// Now Create the final PSO
	// GPURenderPipelineDescriptor
	const pipelineDescriptor = 
	{
		layout: pipelineLayout,
		vertex: vertexShaderState,
		fragment: fragmentShaderState,
		primitive: primitiveState,
		depthStencil: depthStencilState
	};

	// Create Pipelie
	// GPURenderPipeline
	renderPipeline = device.createRenderPipeline(pipelineDescriptor);
	if(null == renderPipeline)
	{
		console.log("Failed to create Render Pipeline");
		throw new Error("Failed to create Render Pipeline");
	}
	else
	{
		console.log("Render Pipeline created successfully");
	}

	// Initialize Perspective Projection Matrix
	perspectiveProjectionMatrix = mat4.create();

	// STEP08 : Define the clear color
	clearColor = { r: 0.0, g: 0.0, b: 0.0, a: 1.0 };
}

function resize()
{
	if( true == bFullscreen)
	{
		canvas.width = window.innerWidth;
		canvas.height = window.innerHeight;
	}
	else
	{
		canvas.width = canvasOriginalWidth;
		canvas.height = canvasOriginalHeight;
	}

	// Depth Texture
	if(null != device)
	{
		if(null != depthTexture)
		{
			depthTexture.destroy();
			depthTexture = null;
		}

		// To create Depth Texture we need Depth Texture Descriptor

		const depthTextureDescriptor = 
		{
			size: [canvas.width, canvas.height, 1],
			dimension: "2d",
			format: "depth24plus-stencil8",
			usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.COPY_SRC
		};

		// Now create the depth texture using above depthTextureDescriptor
		depthTexture = device.createTexture(depthTextureDescriptor);

		if(null == depthTexture)
		{
			throw new Error("Failed to create Depth Texture");
		}
	}

	mat4.perspective(	perspectiveProjectionMatrix, 
						45.0 * Math.PI / 180.0, 
						parseFloat(canvas.width) / parseFloat(canvas.height), 
						0.1, 
						100.0);
}

function display()
{
	// Device may be lost
	// Initialization may not be done yet

	if(null == device)
	{
		return;
	}

	// STEP09 : Get Command Encoder From Device
	// due to Async nature and possibility of device loss.
	// It is better to create it per frame in Display
	// Type: GPUCommandEncoder
	const commandEncoder = device.createCommandEncoder();
	if(null == commandEncoder)
	{
		console.log("Failed to create Command Encoder");
		throw new Error("Failed to create Command Encoder");
	}

	// STEP10: Create Render Pass Color Attachment of Type GPURenderPassColorAttachment
	const renderpassColorAttachment = 
	{
		view: context.getCurrentTexture().createView(),
		clearValue: clearColor,
		loadOp: "clear",
		storeOp: "store"
	};

	// Create RenderPass Depth Attachment
	const renderpassDepthAttachment = 
	{
		view: depthTexture.createView(),
		depthClearValue: 1.0,
		depthLoadOp: "clear",
		depthStoreOp: "store",
		stencilClearValue: 1.0,
		stencilLoadOp: "clear",
		stencilStoreOp: "store",
	}

	// STEP11: According to above, Create RenderPass Descriptor
	// Type: GPURenderPassDescriptor
	const renderPassDescriptor = 
	{
		colorAttachments: [renderpassColorAttachment],
		depthStencilAttachment: renderpassDepthAttachment
	};

	// STEP12: Start the RenderPass
	const renderPassEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);

	// Set Pipeline
	renderPassEncoder.setPipeline(renderPipeline);

	// Set Viewport
	renderPassEncoder.setViewport(0, 0, canvas.width, canvas.height, 0, 1);

	// Set Scissor Rectangle
	renderPassEncoder.setScissorRect(0, 0, canvas.width, canvas.height);

	const modelViewMatrix = mat4.create();
	const modelViewProjectionMatrix = mat4.create();

	// // Pyramid
	// mat4.identity(modelViewMatrix);
	// mat4.translate(modelViewMatrix, modelViewMatrix, [-2.0, 0.0, -6.0]);
	// mat4.rotate(modelViewMatrix, modelViewMatrix, degToRad(rotate_angle), [0.0, 1.0, 0.0]);
	// mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

	// queue.writeBuffer(bufferMVPUniformPyramid, 0, modelViewProjectionMatrix, 0, modelViewProjectionMatrix.length);

	// // Set Vertex Buffer
	// renderPassEncoder.setVertexBuffer(0, bufferPositionPyramid);
	// renderPassEncoder.setVertexBuffer(1, bufferColorPyramid);

	// // Set Bind Group
	// renderPassEncoder.setBindGroup(0, bindgroupMVPUniformPyramid);

	// // Draw Pyramid
	// renderPassEncoder.draw(12, 1, 0, 0);

	// Cube
	mat4.identity(modelViewMatrix);
	mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -8.0]);
	mat4.rotate(modelViewMatrix, modelViewMatrix, degToRad(rotate_angle), [1.0, 1.0, 1.0]);
	mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

	queue.writeBuffer(bufferMVPUniformCube, 0, modelViewProjectionMatrix, 0, modelViewProjectionMatrix.length);

	// Set Vertex Buffer
	renderPassEncoder.setVertexBuffer(0, bufferPositionCube);
	renderPassEncoder.setVertexBuffer(1, bufferColorCube);

	// Set Bind Group
	renderPassEncoder.setBindGroup(0, bindgroupMVPUniformCube);

	// Draw Cube
	renderPassEncoder.draw(36, 1, 0, 0);

	// STEP13: End RenderPass
	renderPassEncoder.end();

	// STEP14: Finish the Command Encoder. There can be multiple command encoders
	const commandBuffer = commandEncoder.finish();

	// we have one and submit Command Encoder(s) to the Queue 
	// STEP15: Submit the Command Buffer to the GPU Queue
	queue.submit([commandBuffer]);

	// Animation Loop
	animationFrameID = requestAnimationFrame(display);

	update();
}

function update()
{
	// Code
	rotate_angle += 1.0;
	if(rotate_angle >= 360.0)
	{
		rotate_angle -= 360.0;
	}
}

function uninitialize()
{
	// Code

	// STEP16: Use animationFrameID for safe animation cancellation and uninitialization
	if(null != animationFrameID)
	{
		cancelAnimationFrame(animationFrameID);
		animationFrameID = null;
	}

	// STEP17: Unconfigure/Destroy the context
	if(null != context)
	{
		context.unconfigure();
		context = null;
	}

	// STEP18: Destroy the Device
	if(null != device)
	{
		device.destroy();
		device = null;
		queue = null;
		
		bufferPositionPyramid = null;
		bufferPositionCube = null;
		
		bufferMVPUniformPyramid = null;
		bufferMVPUniformCube = null;

		bindingGroupMVPUniformPyramid = null;
		bindingGroupMVPUniformCube = null;

		renderPipeline = null;
		perspectiveProjectionMatrix = null;
	}

	console.log("WebGPU Canvas uninitialized successfully");
}

function keyDown(event)
{
	// switch(event.keyCode) // keyCode is depricated
	switch(event.key) // String Representation
	{
		case "Escape":
			uninitialize();
			window.close(); // This may not behave as required across browsers
			break;

		case "f":
		case "F":
			toggleFullscreen();
			break;

		default:
			break;
	}
}

function mouseDown(event)
{
	// Code
}

// User Defined Functions
function createBindGroupLayoutUniform(_bindingIndex, _shaderStageVisibility, _uniformType)
{
	// Code
	const bindgroupLayoutEntry = 
	{
		binding: _bindingIndex,
		visibility: _shaderStageVisibility,
		buffer:
		{
			type: _uniformType
		}
	};

	const bindgroupLayoutDescriptor =
	{
		entries: [bindgroupLayoutEntry]	
	};

	const bindgroupLayout = device.createBindGroupLayout(bindgroupLayoutDescriptor);
	if(null == bindgroupLayout)
	{
		console.log("Failed to create Bind Group Layout for Uniform");
		throw new Error("Failed to create Bind Group Layout for Uniform");
	}

	return(bindgroupLayout);
}

function createUniformBuffer(_uniformBufferSize, _uniformBufferUsage)
{
	// Code
	const bufferDescriptor =
	{
		size: _uniformBufferSize,
		usage: _uniformBufferUsage
	};

	const uniformBuffer = device.createBuffer(bufferDescriptor);
	if(null == uniformBuffer)
	{
		console.log("Failed to create Uniform Buffer");
		throw new Error("Failed to create Uniform Buffer");
	}

	return(uniformBuffer);
}

function createBindGroupForUniform(	_uniformBuffer,
									_uniformBufferOffset,
									_uniformBufferSize,
									_bindingIndex,
									_bindgroupLayout)
{
	// Code
	const bufferBinding = 
	{
		buffer: _uniformBuffer,
		offset: _uniformBufferOffset,
		size: _uniformBufferSize
	};

	const bindGroupEntry =
	{
		binding: _bindingIndex,
		resource: bufferBinding
	};

	const bindGroupDescriptor =
	{
		entries: [bindGroupEntry],
		layout: _bindgroupLayout,
	};

	const bindGroup = device.createBindGroup(bindGroupDescriptor);
	if(null == bindGroup)
	{
		console.log("Failed to create Bind Group for Uniform");
		throw new Error("Failed to create Bind Group for Uniform");
	}

	return(bindGroup);
}

function degToRad(degrees)
{
	return(degrees * Math.PI / 180.0);
}
