/**************************************************
 * S.G.M.P
 * @file: canvas.js
 * @brief: Colored Square
 * @author: Mayur S. Tadekar(mrtadekar@gmail.com)
 * @date: Wednesday 2026-08-19 10:40:06 PM
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

let bufferPositionTriangle = null;
let bufferColorTriangle = null;

let bufferPositionSquare = null;
let bufferTexcoordsSquare = null;

let bufferMVPUniformTriangle = null;
let bufferMVPUniformSquare = null;

let bindgroupMVPUniformTriangle = null;
let bindgroupMVPUniformSquare = null;
let depthTexture = null;

let textureSmiley = null;
let samplerSmiley = null;
let bindgroupTextureAndSampler = null;

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
	await initialize();

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
	
	bufferPositionTriangle = null;
	bufferColorTriangle = null;

	bufferPositionSquare = null;
	bufferTexcoordsSquare = null;

	bufferMVPUniformTriangle = null;
	bufferMVPUniformSquare = null;

	bindgroupMVPUniformTriangle = null;
	bindgroupMVPUniformSquare = null;
	
	depthTexture = null;

	device = null;
	queue = null;

	textureSmiley = null;
	samplerSmiley = null;
	bindgroupTextureAndSampler = null;
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

async function initialize()
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
			@location(0) ovTexcoords: vec2<f32>
		};

		@group(0) @binding(0) var<uniform> mvpUniform: MVPUniform;
		
		@vertex fn main(@location(0) vpos : vec4<f32>, @location(1) vtexcoords : vec2<f32>) -> VertexOutput
		{
			var output: VertexOutput;
			let vPosition = mvpUniform.mvpMatrix * vpos;

			output.ovPosition = vPosition;
			output.ovTexcoords = vtexcoords;
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
			@location(0) ovTexcoords: vec2<f32>
		};

		@group(1) @binding(0) var myTexture2D: texture_2d<f32>;
		@group(1) @binding(1) var mySampler: sampler;

		@fragment fn main(output: VertexOutput) -> @location(0) vec4<f32>
		{
			var color = textureSample(myTexture2D, mySampler, output.ovTexcoords);
			return(color);
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
	const vertexPositionTriangle = new Float32Array([
		0.0, 1.0, 0.0, 1.0,
		-1.0, -1.0, 0.0, 1.0,
		1.0, -1.0, 0.0, 1.0
	]);

	const vertexColorTriangle = new Float32Array([
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0,
	]);

	const vertexPositionSquare = new Float32Array([
		1.0, 1.0, 0.0, 1.0, 	// Right Top
		-1.0, 1.0, 0.0, 1.0, 	// Left Top
		-1.0, -1.0, 0.0, 1.0, 	// Left Bottom

		-1.0, -1.0, 0.0, 1.0,  	// Left Bottom
		1.0, -1.0, 0.0, 1.0, 	// Right Bottom
		1.0, 1.0, 0.0, 1.0,  	// Right Top
	]);

	const vertexTexcoordsSquare = new Float32Array([
		1.0, 1.0,
		0.0, 1.0,
		0.0, 0.0,

		0.0, 0.0,
		1.0, 0.0,
		1.0, 1.0,

		// 1.0, 0.0,
		// 0.0, 0.0,
		// 0.0, 1.0,

		// 0.0, 1.0,
		// 1.0, 1.0,
		// 1.0, 0.0,
	]);

	// As we have only 1 MVP Uniform, The BindGroupLayout is common/shared for both triangle and square

	// // Triangle: Its PositionBuffer, Its UniformBuffer, Its BindGroup
	// let bindGroupLayoutEntryMVPUniformTriangle;
	// {
	// 	// GPUBindGroupLayoutEntry
	// 	bindGroupLayoutEntryMVPUniformTriangle = createBindGroupLayoutUniform(0, 
	// 																		GPUShaderStage.VERTEX, 
	// 																		"uniform");
	// 	// GPUBufferDescriptor
	// 	const bufferDescriptor_Position_Triangle = 
	// 	{
	// 		size: vertexPositionTriangle.byteLength,
	// 		usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
	// 	};

	// 	// GPUBuffer
	// 	bufferPositionTriangle = device.createBuffer(bufferDescriptor_Position_Triangle);
	// 	if(null == bufferPositionTriangle)
	// 	{
	// 		console.log("Failed to create Position Buffer for Triangle");
	// 		throw new Error("Failed to create Position Buffer for Triangle");
	// 	}
	// 	else
	// 	{
	// 		console.log("Position Buffer for Triangle created successfully");
	// 	}

	// 	// Write position array data to position buffer
	// 	queue.writeBuffer(bufferPositionTriangle, 0, vertexPositionTriangle, 0, vertexPositionTriangle.length);
	// 	console.log("Position data written to Triangle buffer");

	// 	// GPUBufferDescriptor for Color Buffer
	// 	const bufferDescriptor_Color_Triangle = 
	// 	{
	// 		size: vertexColorTriangle.byteLength,
	// 		usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
	// 	};

	// 	// GPUBuffer for Color
	// 	bufferColorTriangle = device.createBuffer(bufferDescriptor_Color_Triangle);
	// 	if(null == bufferColorTriangle)
	// 	{
	// 		console.log("Failed to create Color Buffer for Triangle");
	// 		throw new Error("Failed to create Color Buffer for Triangle");
	// 	}
	// 	else
	// 	{
	// 		console.log("Color Buffer for Triangle created successfully");
	// 	}

	// 	// Write color array data to color buffer
	// 	queue.writeBuffer(bufferColorTriangle, 0, vertexColorTriangle, 0, vertexColorTriangle.length);
	// 	console.log("Color data written to Triangle buffer");

	// 	// MVPUniform Buffer for Triangle
	// 	const mvpUniformBufferSize = 16 * 4; // 4x4 matrix of floats
	// 	bufferMVPUniformTriangle = createUniformBuffer(mvpUniformBufferSize, GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST);

	// 	// Create Bind Group For MVPUniform Buffer for Triangle
	// 	bindgroupMVPUniformTriangle = createBindGroupForUniform(bufferMVPUniformTriangle,
	// 															0,
	// 															mvpUniformBufferSize,
	// 															0,
	// 															bindGroupLayoutEntryMVPUniformTriangle
	// 	);
	// }

	// Square: Its PositionBuffer, Its UniformBuffer, Its BindGroup
	let bindGroupLayoutEntryMVPUniformSquare;
	{
		// GPUBindGroupLayoutEntry
		bindGroupLayoutEntryMVPUniformSquare = createBindGroupLayoutUniform(0, 
																			GPUShaderStage.VERTEX, 
																			"uniform");
		// GPUBufferDescriptor
		const bufferDescriptor_Position_Square = 
		{
			size: vertexPositionSquare.byteLength,
			usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
		};

		// GPUBuffer
		bufferPositionSquare = device.createBuffer(bufferDescriptor_Position_Square);
		if(null == bufferPositionSquare)
		{
			console.log("Failed to create Position Buffer for Square");
			throw new Error("Failed to create Position Buffer for Square");
		}
		else
		{
			console.log("Position Buffer for Square created successfully");
		}

		// Write position array data to position buffer
		queue.writeBuffer(bufferPositionSquare, 0, vertexPositionSquare, 0, vertexPositionSquare.length);
		console.log("Position data written to Square buffer");

		// GPUBufferDescriptor for Color Buffer
		const bufferDescriptor_Texcoords_Square = 
		{
			size: vertexTexcoordsSquare.byteLength,
			usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
		};

		// GPUBuffer for Color
		bufferTexcoordsSquare = device.createBuffer(bufferDescriptor_Texcoords_Square);
		if(null == bufferTexcoordsSquare)
		{
			console.log("Failed to create Color Buffer for Square");
			throw new Error("Failed to create Color Buffer for Square");
		}
		else
		{
			console.log("Color Buffer for Square created successfully");
		}

		// Write color array data to color buffer
		queue.writeBuffer(bufferTexcoordsSquare, 0, vertexTexcoordsSquare, 0, vertexTexcoordsSquare.length);
		console.log("Color data written to Square buffer");

		// MVPUniform Buffer for Square
		const mvpUniformBufferSize = 16 * 4; // 4x4 matrix of floats
		bufferMVPUniformSquare = createUniformBuffer(	mvpUniformBufferSize, 
														GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST);

		// Create Bind Group For MVPUniform Buffer for Square
		bindgroupMVPUniformSquare = createBindGroupForUniform(bufferMVPUniformSquare,
																0,
																mvpUniformBufferSize,
																0,
																bindGroupLayoutEntryMVPUniformSquare
		);
	}

	// Load the Image to create texture
	textureSmiley = await loadTexture("smiley.png");
	if(null == textureSmiley)
	{
		console.log("Failed to load texture");
		throw new Error("Failed to load texture");
	}
	else
	{
		console.log("Texture loaded successfully");
	}

	// TextureSamplerDescriptor
	const samplerDescriptor = {
		magFilter: "linear",
		minFilter: "linear"
	}

	// Create Texture Sampler
	samplerSmiley = device.createSampler(samplerDescriptor);
	if(null == samplerSmiley)
	{
		console.log("Failed to create Texture Sampler");
		throw new Error("Failed to create Texture Sampler");
	}
	else
	{
		console.log("Texture Sampler created successfully");
	}

	const bindgroupLayoutTextureAndSampler = createBindGroupLayoutForTextureAndSampler(	"float",
																						"2d",
																						false,
																						0,
																						GPUShaderStage.FRAGMENT,
																						"filtering",
																						1,
																						GPUShaderStage.FRAGMENT
	);

	// Create Texture and Sampler BindGroup
	bindgroupTextureAndSampler = createBindGroupForTextureAndSampler(	0,
																		textureSmiley,
																		1,
																		samplerSmiley,
																		bindgroupLayoutTextureAndSampler
	);

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
		bindGroupLayouts: [bindGroupLayoutEntryMVPUniformSquare, bindgroupLayoutTextureAndSampler]
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
	const texcoordsVertexAttribute = 
	{
		shaderLocation: 1,	// matches with location 1 in vertex shader's main() parameter
		offset: 0,
		format: "float32x2"
	};

	// GPUVertexBufferLayout for Color
	const texcoordsVertexBufferLayout = 
	{
		attributes: [texcoordsVertexAttribute],
		arrayStride: 4 * 2, // 4 floats for each vertex texcoords
		stepMode: "vertex"
	};

	// GPUVertexState
	const vertexShaderState = 
	{
		module: shaderModule_vertexShader,
		entryPoint: "main",
		buffers: [positionVertexBufferLayout, texcoordsVertexBufferLayout],
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
		format: "depth24plus-stencil8"
	};

	// Now Create the final PSO
	// GPURenderPipelineDescriptor
	const pipelineDescriptor = 
	{
		layout: pipelineLayout,
		vertex: vertexShaderState,
		fragment: fragmentShaderState,
		primitive: primitiveState,
		depthStencil: depthStencilState,
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
	};

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

	// // Triangle
	// {
	// 	mat4.identity(modelViewMatrix);
	// 	mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -6.0]);
	// 	mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

	// 	queue.writeBuffer(bufferMVPUniformTriangle, 0, modelViewProjectionMatrix, 0, modelViewProjectionMatrix.length);

	// 	// Set Vertex Buffer
	// 	renderPassEncoder.setVertexBuffer(0, bufferPositionTriangle);
	// 	renderPassEncoder.setVertexBuffer(1, bufferColorTriangle);

	// 	// Set Bind Group
	// 	renderPassEncoder.setBindGroup(0, bindgroupMVPUniformTriangle);

	// 	// Draw Triangle
	// 	renderPassEncoder.draw(3, 1, 0, 0);
	// }

	// Square
	{
		mat4.identity(modelViewMatrix);
		mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -3.0]);
		mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

		queue.writeBuffer(bufferMVPUniformSquare, 0, modelViewProjectionMatrix, 0, modelViewProjectionMatrix.length);

		// Set Vertex Buffer
		renderPassEncoder.setVertexBuffer(0, bufferPositionSquare);
		renderPassEncoder.setVertexBuffer(1, bufferTexcoordsSquare);

		// Set Bind Group
		renderPassEncoder.setBindGroup(0, bindgroupMVPUniformSquare);
		renderPassEncoder.setBindGroup(1, bindgroupTextureAndSampler);

		// Draw Square
		renderPassEncoder.draw(6, 1, 0, 0);
	}

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
}

function uninitialize()
{
	// Code


	if(null != textureSmiley)
	{
		textureSmiley.destroy();
		textureSmiley = null;
	}

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
		
		bufferPositionTriangle = null;
		bufferPositionSquare = null;

		bufferColorTriangle = null;
		bufferTexcoordsSquare = null;
		
		bufferMVPUniformTriangle = null;
		bufferMVPUniformSquare = null;

		bindingGroupMVPUniformTriangle = null;
		bindingGroupMVPUniformSquare = null;

		samplerSmiley = null;
		bindgroupTextureAndSampler = null;

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

// 3 User Defined Functions for Texture and Sampler

async function loadTexture(_imageFileName)
{
	// Code
	const image = new Image();
	image.src = _imageFileName;

	await image.decode();

	const imageBitmap = await createImageBitmap(image);
	if(null == imageBitmap)
	{
		console.log("Failed to create Image Bitmap");
		throw new Error("Failed to create Image Bitmap");
	}
	else
	{
		console.log("Image Bitmap created successfully");
	}

	// Now create Texture Descriptor based on Above Image bitmap
	const textureDescriptor = 
	{
		size:
		[
			imageBitmap.width,
			imageBitmap.height,
			1,
		],
		dimension: "2d",
		format: "rgba8unorm",
		usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST | GPUTextureUsage.RENDER_ATTACHMENT,
	};

	const _texture = device.createTexture(textureDescriptor);
	if(null == _texture)
	{
		console.log("Failed to create Texture");
		throw new Error("Failed to create Texture");
	}
	else
	{
		console.log("Texture created successfully");
	}

	const copySource = 
	{
		source: imageBitmap,
		flipY: true,
	};

	const copyDestination = 
	{
		texture: _texture,
		miplevel: 0,
	};

	queue.copyExternalImageToTexture(
		copySource,
		copyDestination,
		textureDescriptor.size
	);

	return(_texture);
}

function createBindGroupLayoutForTextureAndSampler(_textureSampleType,
													_textureViewDimension,
													_isTextureMultiSampled,
													_textureBindingIndex,
													_textureShaderStageVisibility,
													_samplerType,
													_samplerBindingIndex,
													_samplerShaderStageVisibility
)
{
	// Code

	// Create Binding Layout
	const bindingLayoutTexture = 
	{
		sampleType: _textureSampleType,
		viewDimension: _textureViewDimension,
		multiSampled: _isTextureMultiSampled,
	};

	// Create BindGroupLayout Entry
	const bindGroupLayoutEntryTexture = 
	{
		binding: _textureBindingIndex,
		visibility: _textureShaderStageVisibility,
		texture: bindingLayoutTexture,
	};

	// Create Binding Layout for Sampler
	const bindingLayoutSampler =
	{
		type: _samplerType,
	};

	// Create BindGroupLayout Entry for Sampler
	const bindGroupLayoutEntrySampler = 
	{
		binding: _samplerBindingIndex,
		visibility: _samplerShaderStageVisibility,
		sampler: bindingLayoutSampler,
	};

	// Create BindGroupLayoutDescriptor
	const bindGroupLayoutDescriptor = 
	{
		entries: [bindGroupLayoutEntryTexture, bindGroupLayoutEntrySampler],
	};

	// Create BindGroupLayout
	const bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDescriptor);
	if(null == bindGroupLayout)
	{
		console.log("Failed to create Bind Group Layout for Texture and Sampler");
		throw new Error("Failed to create Bind Group Layout for Texture and Sampler");
	}
	else
	{
		console.log("Bind Group Layout for Texture and Sampler created successfully");
	}

	return(bindGroupLayout);
}

function createBindGroupForTextureAndSampler(	_textureBindingIndex,
												_texture,
												_samplerBindingIndex,
												_sampler,
												_bindGroupLayout)
{
	// Code

	// Create BindGroupEntry for Texture
	const bindGroupEntryTexture = 
	{
		binding: _textureBindingIndex,
		resource: _texture.createView(),
	};

	// Create BindGroupEntry for Sampler
	const bindGroupEntrySampler = 
	{
		binding: _samplerBindingIndex,
		resource: _sampler,
	};

	// Create BindGroupDescriptor for Texture and Sampler
	const bindGroupDescriptorForTextureAndSampler = 
	{
		layout: _bindGroupLayout,
		entries: [bindGroupEntryTexture, bindGroupEntrySampler],
	};

	// Create BindGroup for Texture and Sampler
	const bindGroup = device.createBindGroup(bindGroupDescriptorForTextureAndSampler);
	if(null == bindGroup)
	{
		console.log("Failed to create Bind Group for Texture and Sampler");
		throw new Error("Failed to create Bind Group for Texture and Sampler");
	}
	else
	{
		console.log("Bind Group for Texture and Sampler created successfully");
	}

	return(bindGroup);
}
