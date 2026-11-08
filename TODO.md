# Lumen Engine TODO

**Goal:** a cross-platform, ultra-realistic graphics rendering engine written in C++ and Vulkan.
Target features: PBR, Ray Tracing, Path Tracing, SSR, SSAO, MSAA, FXAA and other anti-aliasing algorithms, shadow mapping, ray-traced shadows, deferred / forward / Forward+ rendering.

---

## Renderer foundation

- [ ] `Mesh` (vertex/index buffers + draw), `Texture`, `Material`
- [ ] Scene: `Transform`, object list, lightweight ECS (e.g. `entt`) or a simple node tree
- [ ] `DescriptorAllocator` with growable pools, `DescriptorSetLayoutBuilder`, `DescriptorWriter`
- [ ] Per-frame UBO (camera, time, lights), one per frame in flight; `set = 0` global, `set = 1` material
- [ ] Frame-indexed deletion queue: resize and resource unloading without `vkDeviceWaitIdle`
- [ ] Refactor `Renderer` into a `RenderContext` (frame, commands) and a set of `RenderPass`es
- [ ] Minimal **render graph**: resource and pass declaration, automatic barriers and layout transitions
- [ ] Transient attachments (G-buffer, HDR, depth) recreated on resize
- [ ] Shader hot reload
- [ ] GPU timestamps (`vkCmdWriteTimestamp2`) per pass
- [ ] Tracy and RenderDoc integration, debug object names (`VK_EXT_debug_utils`)
- [ ] **ImGui** (dynamic rendering): FPS, pass timings, settings panels

## Assets

- [ ] glTF 2.0 loading (`fastgltf`): meshes, materials, textures, node hierarchy, cameras, lights
- [ ] Vertex layout: `pos, normal, tangent, uv0` (+ uv1, vertex color as needed)
- [ ] Tangent generation (MikkTSpace)
- [ ] Compressed textures: KTX2 + BC7/BC5/BC6H (Basis Universal for cross-platform)
- [ ] Asynchronous loading: transfer queue, job system, timeline semaphores
- [ ] One large vertex/index buffer per scene with suballocation
- [ ] Reference scenes: Sponza, Damaged Helmet, Bistro

## PBR and basic lighting

- [ ] HDR render target `R16G16B16A16_SFLOAT`, tone mapping (ACES / AgX / Khronos PBR Neutral), exposure
- [ ] Linear workflow: sRGB textures for albedo, UNORM for normal/ORM
- [ ] Cook-Torrance BRDF (GGX NDF, Smith G, Schlick F), metallic/roughness workflow
- [ ] Normal mapping, AO map, emissive
- [ ] Light types: directional, point, spot (physical units, inverse-square falloff)
- [ ] **IBL**: HDR cubemap, irradiance (SH or convolution), prefiltered specular, BRDF LUT
- [ ] Skybox, procedural sky (Hosek-Wilkie / Preetham)
- [ ] PBR extensions: clearcoat, sheen, transmission, subsurface (approximate), anisotropy
- [ ] Alpha blend / alpha test, transparent sorting or OIT (weighted blended)

## Shadows

- [ ] Directional shadow map: depth-only pipeline, depth bias, slope-scaled bias
- [ ] **Cascaded Shadow Maps** (cascade stabilization, texel snapping)
- [ ] Filtering: PCF, then PCSS (soft shadows), optionally VSM/EVSM
- [ ] Spot shadows (2D) and point shadows (cubemap or dual paraboloid)
- [ ] Shadow atlas and caching of static shadows
- [ ] Screen-space contact shadows

## Anti-aliasing

- [ ] **MSAA** (2x/4x/8x): resolve in the render graph, depth resolve, alpha-to-coverage
- [ ] **FXAA**
- [ ] **SMAA**
- [ ] **TAA**: jitter (Halton), motion vectors, history reprojection, neighborhood clamping, YCoCg
- [ ] Upscaling: FSR 2/3 or a custom TAAU
- [ ] ImGui comparison panel: switch AA algorithms at runtime

## Post-processing and screen-space effects

- [ ] Separate normal and depth attachments, hierarchical-Z (Hi-Z) mip chain
- [ ] **SSAO** (hemisphere sampling, bilateral blur), then **GTAO / HBAO**
- [ ] **SSR**: Hi-Z ray marching, IBL fallback, roughness handling, previous-frame reprojection
- [ ] Bloom (dual filter), vignette, chromatic aberration, film grain
- [ ] Motion blur, depth of field (bokeh)
- [ ] Auto-exposure (histogram), color grading (LUT)
- [ ] Volumetric fog / god rays (froxel-based)

## Render paths

- [ ] **Forward** (baseline, already working)
- [ ] **Depth pre-pass** for forward
- [ ] **Deferred**: G-buffer (albedo, normal, ORM, emissive, depth) and lighting pass
- [ ] G-buffer optimization (octahedral normals, packing)
- [ ] **Forward+**: compute light culling per tile (16x16 / 32x32), light list buffer
- [ ] **Clustered forward / deferred**: 3D clusters, thousands of lights
- [ ] Runtime switching between render paths and performance comparison
- [ ] Indirect draw, GPU-driven culling (frustum, Hi-Z occlusion), meshlets / mesh shaders (optional)
- [ ] Bindless resources (descriptor indexing, buffer device address)

## Ray tracing

- [ ] Extension checks: `VK_KHR_acceleration_structure`, `VK_KHR_ray_query`, `VK_KHR_ray_tracing_pipeline`, `VK_KHR_deferred_host_operations`
- [ ] BLAS/TLAS: build, compaction, updates for dynamic objects, scratch buffers
- [ ] Shader binding table and RT pipeline (raygen / closest-hit / miss / any-hit)
- [ ] **RT shadows** (ray query, one ray per light, denoising)
- [ ] **RT reflections** (hybrid with SSR, with fallback)
- [ ] **RT ambient occlusion**
- [ ] **RT global illumination**: DDGI probes or 1-bounce with ReSTIR GI
- [ ] Denoisers: SVGF / A-SVGF, then ReBLUR/ReLAX-style or NRD
- [ ] "Hybrid renderer" mode: rasterization + RT effects

## Path tracing

- [ ] Reference path tracer (progressive accumulation, "ground truth" mode for validating the rasterizer)
- [ ] Monte Carlo: BRDF importance sampling, next event estimation (NEE), MIS, Russian roulette
- [ ] Sampling: Sobol / blue noise, low-discrepancy sequences
- [ ] Materials: dielectrics, glass (refraction), subsurface, emission, MIS with environment lighting
- [ ] Efficient lighting: **ReSTIR DI / GI / PT**
- [ ] Real-time mode: 1 spp + denoiser + temporal accumulation
- [ ] OIDN integration (offline mode) and NRD (real-time)
- [ ] Scene BVH updates for animation

## Cross-platform support

- [ ] Windows and Linux (already in CMake), test on AMD / NVIDIA / Intel
- [ ] **Apple**: CMake currently fails on Apple; plan MoltenVK support, limited Vulkan 1.3 features, no RT, fallback paths
- [ ] **Android**: Vulkan 1.1+/1.3, tile-based GPUs, memory and feature limits
- [ ] Capability-based renderer: RT / mesh shaders / bindless enabled by device capabilities
- [ ] Platform abstraction layer: window, input, file system
- [ ] Wayland and X11 on Linux
- [ ] CI: Windows / Linux builds (GitHub Actions), validation layers, tests on lavapipe
- [ ] Quality tiers: tier 0 (mobile: forward + basic PBR), tier 1 (desktop: deferred/Forward+ + SSR/SSAO), tier 2 (RT hybrid and path tracing)

## Tooling and quality

- [ ] Editor / viewer: object picking, gizmos, material inspector
- [ ] Scene serialization, processed-asset cache
- [ ] Automated tests: screenshot comparison (golden images), math unit tests
- [ ] Memory profiling (VMA statistics), VRAM budget
- [ ] Documentation: renderer architecture, pass graph
- [ ] Benchmark scenes and a results table for every rendering mode

---

## Dependency notes

1. Phase 0 then Phase 1: the render graph and ImGui matter more than any individual effect; otherwise every new pass means hand-writing barriers again.
2. Phases 2 and 3: without glTF and PBR there is nothing meaningful to validate other features against.
3. Phases 4, 5 and 6 can run in parallel, but SSR/SSAO need normal + depth + Hi-Z, so build the G-buffer first.
4. Phase 7 (deferred, Forward+) comes before Phase 8 for hybrid RT and large scenes.
5. RT and path tracing order: RT shadows first, then reflections/GI, then the path tracer as the reference for evaluating all other techniques.
6. Design for cross-platform (capability flags) from the start, but do Android/Apple only after the desktop renderer is stable.
7. "Ultra realistic" + cross-platform + RT/path tracing everywhere is not achievable at once: RT exists only on modern desktop GPUs (and some new Android devices), and MoltenVK does not support it. Use the tier mechanism above.
