# triangle-injection
This is a very small proof of concept project, demonstrating how to write a proxy dll file that will intercept D3D11 calls, and inject new graphics commands in a game that you otherwise don't have source access to. 

Details about how this project works can be found on [this blog post](http://kylehalladay.com/blog/2021/07/14/Dll-Search-Order-Hijacking-For-PostProcess-Injection.html)

The solution file builds a test app, which renders a spinning cube, and a proxy d3d11.dll file. When that file (and associated shader files) are placed in the same directory as a D3D11 application's binary file, that DLL will intercept calls to D3D11CreateDeviceAndSwapChain, and issue new graphics commands that result in a triangle being drawn across half the screen. 

This is intended as a minimal example only and has been made intentionally less functional in order to make it easier to understand. 

The DLL requires that the target app uses D3D11CreateDeviceAndSwapChain to create the associated D3D11 objects. If an app instead uses different API calls (like CreateDevice()), attempting to use this proxy dll will cause the program to crash. It also doesn't play nicely with the D3D11 debug layers. For an example of a more fully featured / production quality graphics injector, check out [Reshade](https://reshade.me)


![Screenshot of Skyrim with triangle drawn over top](https://github.com/khalladay/triangle-injection/blob/main/skyrim.jpg?raw=true)

![Screenshot of test app with triangle drawn over top](https://github.com/khalladay/triangle-injection/blob/main/test_app.png?raw=true)


# Changes

Implemented pixel shaders for [upsampling](https://github.com/erikrause/triangle-injection/blob/main/triangle-injection/d3d11_proxy_dll/hook_content/bilinearInterpolation_ps.shader) and [zooming](https://github.com/erikrause/triangle-injection/blob/main/triangle-injection/d3d11_proxy_dll/hook_content/topLeftQuadrant_ps.shader). There are used sampler for linear interpolation.

In order for pixel shaders to sample swapchain's backbuffer, it was:
  - placed fullscreen quad to texturing the viewport.
  - added shader resorce view for backbuffer.
  - created render target with 2x size of backbuffer texture.
  
Also added IDXGISwapChain::D3D11CreateDevice() passthrough function for Visual Studio GPU debugger.

For digits drawing DirectXTK is used with generated fonts ([fonts generating tutorial](https://github.com/microsoft/DirectXTK/wiki/Drawing-text)).

# Using

Put the d3d11.dll and the "hook_content" folder (contains shaders and fonts) in the 3D application folder that contains executable program, and run this program. 
