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
	queue = null;
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

	// STEP08 : Define the clear color
	clearColor = { r: 0.0, g: 0.0, b: 1.0, a: 1.0 };
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

	// STEP12: Start the RenderPass
	const renderPassEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);



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
