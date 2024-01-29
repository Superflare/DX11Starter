# Rendering Engine Class Project
Began with a fork of the professors starter code for a DX11 project, then over the course of a semester implemented new features
- - -

The project started as a barebones DirectX 11 application provided by the professor. The underlying functionality of creating a new application window and using DirectX to display a frame of pixels with a swap chain was already implemented at the start. From there I built it up to be something that could display 3D objects in a virtual environment with fairly accurate materials and lighting. The project also makes use of the professor's own SimpleShader library which makes the process of implementing and integrating shaders into a DirectX application a lot more convenient.

Features individually worked on and implemented are: a virtual camera, camera controls, skyboxes, Dear ImGui UI for runtime modification, custom pixel and vertex shaders written in hlsl, game entities with associated transforms, meshes, and PBR materials, multiple light types, and shadow maps.
