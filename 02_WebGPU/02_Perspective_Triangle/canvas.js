/**************************************************
 * S.G.M.P
 * @file: canvas.js
 * @brief: WebGPU BlueScreen
 * @author: Mayur S. Tadekar(mrtadekar@gmail.com)
 * @date: 16 July 2026 (Thursday) 01:30:11 AM
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

let bufferPosition = null;
let renderPipeline = null;
let buffermvpUniform = null;
let bindingGroupMVPUniform = null;
let perspectiveProjectionMatrix = null;


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
	device = null;
	queue = null;``
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

		@group(0) @binding(0) var<uniform> mvpUniform: MVPUniform;
		
		@vertex fn main(@location(0) vpos : vec4<f32>) -> @builtin(position) vec4<f32>
		{
			let vPosition = mvpUniform.mvpMatrix * vpos;
			return vPosition;
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
		@fragment fn main() -> @location(0) vec4<f32>
		{
			return vec4<f32>(1.0, 1.0, 1.0, 1.0);
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
	const vertexPosition = new Float32Array([
		0.0, 1.0, 0.0, 1.0,
		-1.0, -1.0, 0.0, 1.0,
		1.0, -1.0, 0.0, 1.0
	]);

	// GPUBufferDescriptor
	const bufferDescriptor_Position = 
	{
		size: vertexPosition.byteLength,
		usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
	};

	// GPUBuffer
	bufferPosition = device.createBuffer(bufferDescriptor_Position);
	if(null == bufferPosition)
	{
		console.log("Failed to create Position Buffer");
		throw new Error("Failed to create Position Buffer");
	}
	else
	{
		console.log("Position Buffer created successfully");
	}

	// Write position array data to position buffer
	queue.writeBuffer(bufferPosition, 0, vertexPosition, 0, vertexPosition.length);
	console.log("Position data written to buffer");

	// GPUBindGroupLayoutEntry
	const bindGroupLayoutEntry_mvpUniform = 
	{
		binding: 0,		// matches with 0th Group's 0th Slot binding in Vertex Shader
		visibility: GPUShaderStage.VERTEX,
		buffer: 
		{
			type: "uniform"
		}
	};

	// GPUBindGroupLayoutDescriptor
	const bindGroupLayoutDescriptor = 
	{
		entries: [bindGroupLayoutEntry_mvpUniform]
	};

	// GPUBindGroupLayout
	const bindGroupLayout_mvpUniform = device.createBindGroupLayout(bindGroupLayoutDescriptor);
	if(null == bindGroupLayout_mvpUniform)
	{
		console.log("Failed to create Bind Group Layout");
		throw new Error("Failed to create Bind Group Layout");
	}
	else
	{
		console.log("Bind Group Layout created successfully");
	}

	// To Create Pipeline Layout we will create PipelineLayoutDescriptor
	// GPUPipelineLayoutDescriptor
	const pipelineLayoutDescriptor = 
	{
		bindGroupLayouts: [bindGroupLayout_mvpUniform]
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

	// Create Uniform Buffer
	const mvpUniformBufferSize = 16 * 4; // 4x4 matrix of floats
	const mvpUniformBufferDescriptor = 
	{
		size: mvpUniformBufferSize,
		usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST
	};
	buffermvpUniform = device.createBuffer(mvpUniformBufferDescriptor);
	if(null == buffermvpUniform)
	{
		console.log("Failed to create Uniform Buffer");
		throw new Error("Failed to create Uniform Buffer");
	}
	else
	{
		console.log("Uniform Buffer created successfully");
	}

	// GPUBufferBinding
	const bufferBinding_mvpUniform = 
	{
		buffer: buffermvpUniform,
		offset: 0,
		size: mvpUniformBufferSize
	};

	// GPUBindGroupEntry
	const bindGroupEntry_mvpUniform = 
	{
		binding: 0,
		resource: bufferBinding_mvpUniform
	};

	// GPUBindGroupDescriptor
	const bindGroupDescriptor_mvpUniform = 
	{
		layout: bindGroupLayout_mvpUniform,
		entries: [bindGroupEntry_mvpUniform]
	};

	// GPUBindGroup
	bindingGroupMVPUniform = device.createBindGroup(bindGroupDescriptor_mvpUniform);
	if(null == bindingGroupMVPUniform)
	{
		console.log("Failed to create Bind Group");
		throw new Error("Failed to create Bind Group");
	}
	else
	{
		console.log("Bind Group created successfully");
	}
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

	// GPUVertexState
	const vertexShaderState = 
	{
		module: shaderModule_vertexShader,
		entryPoint: "main",
		buffers: [positionVertexBufferLayout],
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

	// Now Create the final PSO
	// GPURenderPipelineDescriptor
	const pipelineDescriptor = 
	{
		layout: pipelineLayout,
		vertex: vertexShaderState,
		fragment: fragmentShaderState,
		primitive: primitiveState
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

	// STEP11: According to above, Create RenderPass Descriptor
	// Type: GPURenderPassDescriptor
	const renderPassDescriptor = 
	{
		colorAttachments: [renderpassColorAttachment]
	};

	const modelViewMatrix = mat4.create();
	mat4.identity(modelViewMatrix);

	const modelViewProjectionMatrix = mat4.create();
	mat4.identity(modelViewProjectionMatrix);

	mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -3.0]);

	mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

	queue.writeBuffer(buffermvpUniform, 0, modelViewProjectionMatrix, 0, modelViewProjectionMatrix.length);

	// STEP12: Start the RenderPass
	const renderPassEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);

	// Set Pipeline
	renderPassEncoder.setPipeline(renderPipeline);

	// Set Viewport
	renderPassEncoder.setViewport(0, 0, canvas.width, canvas.height, 0, 1);

	// Set Scissor Rectangle
	renderPassEncoder.setScissorRect(0, 0, canvas.width, canvas.height);

	// Set Vertex Buffer
	renderPassEncoder.setVertexBuffer(0, bufferPosition);

	// Set Bind Group
	renderPassEncoder.setBindGroup(0, bindingGroupMVPUniform);

	// Draw Triangle
	renderPassEncoder.draw(3, 1, 0, 0);

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
		
		bufferPosition = null;
		renderPipeline = null;
		buffermvpUniform = null;
		bindingGroupMVPUniform = null;
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
