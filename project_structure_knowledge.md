# Game Engine Project Structure & Architecture

## Project Overview
**Name:** Property-Based ECS Game Engine  
**Architecture:** Entity Component System (ECS) using EnTT  
**Build System:** CMake  
**C++ Standard:** C++20  
**Primary Libraries:** GLFW, GLAD, GLM, EnTT, Jolt Physics, FMOD, ImGui, Tracy, RapidJSON

---

## Complete Folder Structure

```
GameEngine/
├── CMakeLists.txt                 # Root CMake configuration
├── .gitignore                     # Git ignore file
├── README.md                      # Project documentation
│
├── Engine/                        # ENGINE LIBRARY (Static Library)
│   ├── CMakeLists.txt            # Engine CMake config
│   │
│   ├── Core/                     # Core Engine Systems
│   │   ├── Application.h         # Base application class (window, game loop)
│   │   ├── Application.cpp       # Handles initialization, main loop, shutdown
│   │   ├── Input.h              # Input system (keyboard, mouse, scroll)
│   │   └── Input.cpp            # Non-singleton input handling
│   │
│   ├── ECS/                      # Entity Component System
│   │   ├── Entity.h             # Entity wrapper around EnTT
│   │   ├── Scene.h              # Scene management (entity collections)
│   │   ├── Scene.cpp            # Scene implementation with serialization
│   │   ├── Components.h         # All component definitions (POD structs)
│   │   ├── System.h             # Base system interface (NEW)
│   │   └── SystemRegistry.h     # System management and execution (NEW)
│   │
│   ├── Serialization/            # Property-Based Serialization System
│   │   ├── Property.h           # Property reflection base classes
│   │   ├── ReflectionRegistry.h # Component metadata registry
│   │   ├── ComponentRegistry.h  # Component registration header
│   │   ├── ComponentRegistry.cpp # Register all components
│   │   ├── SceneSerializer.h    # Scene serialization interface
│   │   ├── SceneSerializer.cpp  # JSON-based scene serialization
│   │   └── SerializationCommon.h # Common serialization includes
│   │
│   ├── Graphics/                 # Rendering Systems (Future)
│   │   ├── Renderer.h           # Main renderer
│   │   ├── Renderer.cpp         
│   │   ├── Shader.h             # Shader management
│   │   ├── Shader.cpp
│   │   ├── Mesh.h               # Mesh data structures
│   │   ├── Mesh.cpp
│   │   ├── Material.h           # Material system
│   │   └── Material.cpp
│   │
│   ├── Physics/                  # Physics Systems (Future)
│   │   ├── PhysicsSystem.h      # Jolt physics integration
│   │   └── PhysicsWorld.h       # Physics world wrapper
│   │
│   ├── Audio/                    # Audio Systems (Future)
│   │   ├── AudioSystem.h        # FMOD audio integration
│   │   └── AudioEngine.h        # Audio engine wrapper
│   │
│   ├── Utility/                  # Helper Classes & Tools
│   │   ├── Logger.h             # Logging system (singleton)
│   │   ├── Logger.cpp           # Thread-safe logging with levels
│   │   ├── Timestep.h           # Delta time wrapper
│   │   ├── FileSystem.h         # File I/O utilities (future)
│   │   ├── FileSystem.cpp
│   │   ├── Random.h             # Random number generation (future)
│   │   └── Random.cpp           # Random number generation (future)   
│   │
│   └── Editor/                   # Editor Tools (Optional/Future)
│   │   ├── EditorCamera.h       # Editor-specific camera
│   │   ├── EditorCamera.cpp
│   │   ├── Gizmos.h             # Transform gizmos
│   │   └── Gizmos.cpp
│   │ 
│   └── Asset/                   # Asset Pipeline tools
│   
├── Game/                          # GAME EXECUTABLE
│   ├── CMakeLists.txt            # Game CMake config
│   ├── Main.cpp                 # Entry point (creates Game instance)
│   ├── Game.h                   # Game class (inherits from Application)
│   ├── Game.cpp                 # Game-specific logic
│   └── [GameSystems/]           # Game-specific systems (future)
│
├── External/                      # THIRD-PARTY LIBRARIES
│   ├── CMakeLists.txt            # External libs CMake config
│   ├── glfw/                    # Window management
│   ├── glad/                    # OpenGL loader
│   ├── glm/                     # Math library
│   ├── entt/                    # ECS framework
│   ├── jolt/                    # Physics engine
│   ├── fmod/                    # Audio system
│   ├── imgui/                   # UI library
│   ├── ImGuizmo/                # 3D gizmos
│   ├── tracy/                   # Profiler
│   ├── rapidjson/               # JSON parsing (for serialization)
│   └── openFBX/                 # FBX model loading
│   └── xresource_guid/          # GUID creation
│   └── xresource_mgr/           # Resource Loading
│
└── Resources/                     # GAME ASSETS
    ├──Sources                      # Original Files
        ├── Shaders/                  # GLSL shaders
        ├── Models/                   # 3D models
        ├── Textures/                 # Image files
        ├── Audio/                    # Sound files
        └── Scenes/                   # Scene JSON files
            ├── ExampleScene.json     # Example game scene
            ├── TestScene.json        # Arena test scene
            └── SavedScene.json       # Runtime save location
    ├──Descriptors                    # Descriptor Files for Assets
    ├──Compiled                       # Compiled format for Assets
```

---

## Architecture Details

### **Core Systems**

#### Application System
- **Purpose:** Manages window, game loop, and application lifecycle
- **Key Classes:** `Application` (base class for games)
- **Features:** 
  - GLFW window management
  - Fixed timestep game loop
  - FPS display in title bar
  - Input system integration
  - Calls `OnInit()` after construction (fixed virtual dispatch)

#### Input System  
- **Purpose:** Handle keyboard and mouse input
- **Design:** Direct class (not singleton/manager pattern)
- **Features:**
  - Key/button states (pressed, just pressed, just released)
  - Mouse position and delta tracking
  - Scroll wheel support (fixed event timing)
  - Cursor visibility control
  - Event polling before input update

### **ECS (Entity Component System)**

#### Design Philosophy
- **Entities:** Simple IDs (managed by EnTT)
- **Components:** Pure data structs (no logic)
- **Systems:** Functions that operate on components

#### Current Components
- `TagComponent` - Human-readable names
- `TransformComponent` - Position, rotation, scale
- `CameraComponent` - Camera settings
- `MeshRendererComponent` - Rendering data (future)
- `RigidbodyComponent` - Physics data (velocity, gravity, mass)

#### Entity ID Uniqueness
- EnTT guarantees unique entity IDs within a registry
- IDs are versioned internally to prevent reuse conflicts
- Each `Scene` has its own registry (isolated ID space)

### **Systems Architecture** (NEW)

#### System Base Class
- **File:** `Engine/ECS/System.h`
- **Purpose:** Interface for all game systems
- **Key Methods:**
  - `OnInit(Scene*)` - Called once when system is added
  - `OnUpdate(Scene*, Timestep)` - Called every frame (pure virtual)
  - `OnShutdown(Scene*)` - Called when system is removed
  - `GetPriority()` - Determines execution order (lower = earlier)
  - `GetName()` - Returns system name for debugging

#### SystemRegistry
- **File:** `Engine/ECS/SystemRegistry.h`
- **Purpose:** Manages all systems in a scene
- **Features:**
  - Add/remove systems dynamically
  - Automatic priority-based sorting
  - System enable/disable support
  - Initialize/update/shutdown lifecycle management
  - Template-based type-safe API

#### System Priority Convention
- **0-9:** Input & Events
- **10-19:** Physics & Collision
- **20-29:** Animation
- **30-49:** Transforms
- **50-79:** Game Logic
- **80-99:** Audio
- **100-199:** Rendering
- **200+:** UI & Post-Processing

#### Creating Custom Systems
Team members should create systems in their respective subsystem folders:
- **Physics:** `Engine/Physics/PhysicsSystem.h`
- **Rendering:** `Engine/Graphics/RenderSystem.h`
- **Audio:** `Engine/Audio/AudioSystem.h`

Example system structure:
```cpp
// Engine/Physics/PhysicsSystem.h
#pragma once
#include "ECS/System.h"
#include "ECS/Components.h"

namespace Engine {

class PhysicsSystem : public System {
public:
    void OnUpdate(Scene* scene, Timestep ts) override {
        auto view = scene->GetRegistry().view<TransformComponent, RigidbodyComponent>();
        for (auto entity : view) {
            // Physics logic here
        }
    }
    int GetPriority() const override { return 10; }
    const char* GetName() const override { return "PhysicsSystem"; }
};

} // namespace Engine
```

### **Serialization System**

#### Property Reflection
- Type-safe property access with metadata
- Supports: Bool, Int, Float, String, Vec2, Vec3, Vec4
- Automatic component registration system

#### Scene Serialization
- JSON-based format (human-readable)
- Save/Load entire scenes to/from files
- RapidJSON for fast parsing
- Version-controlled format

#### Key Classes
- `Property<T, V>` - Typed property wrapper
- `ComponentMetadata` - Stores component properties
- `ReflectionRegistry` - Singleton registry for all components
- `SceneSerializer` - Handles JSON serialization/deserialization
- `ComponentRegistry` - Central component registration

### **Graphics Pipeline** (Future)
- OpenGL 4.3 Core Profile
- Forward rendering initially
- Batch rendering for 2D sprites
- 3D mesh rendering with materials

### **Build Configuration**

#### CMake Structure
1. **Root CMakeLists.txt** - Sets up project, C++ standard, output dirs
2. **External/CMakeLists.txt** - Configures all third-party libraries
3. **Engine/CMakeLists.txt** - Builds static library `EngineLib`
4. **Game/CMakeLists.txt** - Builds executable, links to `EngineLib`, copies resources

#### Build Commands
```bash
# Windows (Visual Studio)
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Debug

# Linux/Mac
mkdir build && cd build
cmake .. -G "Unix Makefiles"
make -j8
```

---

## Code Style Guidelines

### Component Design
```cpp
// CORRECT - POD struct with explicit constructors
struct TagComponent {
    std::string Tag;
    
    TagComponent() : Tag("Entity") {}
    TagComponent(const std::string& tag) : Tag(tag) {}
};

// WRONG - Methods in components
struct TagComponent {
    std::string Tag;
    void SetTag(const std::string& tag) { Tag = tag; } // NO!
};
```

### System Design
```cpp
// CORRECT - Logic in systems, data in components
class MovementSystem : public System {
public:
    void OnUpdate(Scene* scene, Timestep ts) override {
        auto view = scene->GetRegistry().view<TransformComponent, RigidbodyComponent>();
        for (auto entity : view) {
            auto& transform = view.get<TransformComponent>(entity);
            auto& rb = view.get<RigidbodyComponent>(entity);
            transform.Position += rb.Velocity * ts;
        }
    }
    int GetPriority() const override { return 50; }
};

// WRONG - Logic in components
struct TransformComponent {
    glm::vec3 Position;
    void Move(glm::vec3 delta) { Position += delta; } // NO!
};
```

### Serialization Registration
```cpp
// In ComponentRegistry::RegisterAllComponents()
auto& meta = REGISTER_COMPONENT(TagComponent);
meta.AddProperty<TagComponent, std::string>(
    "Tag",
    PropertyType::String,
    [](const TagComponent& c) { return c.Tag; },
    [](TagComponent& c, const std::string& v) { c.Tag = v; }
);
```

### Scene Management with Systems
```cpp
// In Game::OnInit()
void Game::OnInit() {
    // Register components
    Engine::ComponentRegistry::RegisterAllComponents();
    
    // Create scene
    m_Scene = std::make_unique<Engine::Scene>("Main Scene");
    
    // Add systems (team members will add their systems here)
    // m_Scene->AddSystem<Engine::PhysicsSystem>();
    // m_Scene->AddSystem<Engine::RenderSystem>(GetWidth(), GetHeight());
    // m_Scene->AddSystem<Engine::AudioSystem>();
    
    // Initialize all systems
    m_Scene->InitializeSystems();
    
    // Load or create scene content
    if (!m_Scene->LoadFromFile("Resources/Scenes/ExampleScene.json")) {
        CreateDefaultScene();
    }
}

// In Game::OnUpdate()
void Game::OnUpdate(Engine::Timestep ts) {
    // Scene automatically calls all systems in priority order
    m_Scene->OnUpdate(ts);
}

// In Game::OnShutdown()
void Game::OnShutdown() {
    // Systems are automatically shut down when scene is destroyed
    m_Scene->ShutdownSystems();
    m_Scene.reset();
}
```

### Scene Save/Load
```cpp
// Save scene
m_Scene->SaveToFile("Resources/Scenes/MyScene.json");

// Load scene (with system re-initialization)
m_Scene->ShutdownSystems();
if (m_Scene->LoadFromFile("Resources/Scenes/MyScene.json")) {
    m_Scene->InitializeSystems();
}
```

---

## Common Issues & Solutions

### System Not Running
**Symptom:** System's OnUpdate never called
**Cause:** Forgot to initialize systems
**Solution:**
1. Call `m_Scene->InitializeSystems()` after adding all systems
2. Verify system is enabled with `system->IsEnabled()`
3. Check logs for "System execution order"

### Circular Dependency Errors
**Symptom:** Compiler errors about undefined `Scene`
**Cause:** Including Scene.h in System headers
**Solution:**
1. Use forward declaration: `class Scene;` in System.h and SystemRegistry.h
2. Only include Scene.h in .cpp files
3. Never include Scene.h in system header files

### Input Not Working
**Symptom:** Key presses or mouse input not detected
**Cause:** Event polling order incorrect
**Solution:**
1. Ensure `glfwPollEvents()` is called BEFORE `Input::Update()`
2. Check Application.cpp game loop order
3. Verify Input system is initialized

### Mouse Scroll Not Working
**Symptom:** Scroll events not detected
**Cause:** Scroll delta reset timing issue
**Solution:**
1. Reset scroll delta at START of `Input::Update()`
2. Accumulate in callback using `+=`
3. Poll events before updating input

### Serialization Crashes
**Symptom:** 0xc0000005 access violation when saving/loading
**Cause:** Components not properly initialized or registered
**Solution:**
1. Ensure all components have explicit default constructors
2. Call `ComponentRegistry::RegisterAllComponents()` at startup
3. Check that Resources/Scenes/ folder exists in build directory

### Scene is Null
**Symptom:** "Scene is null in OnUpdate!" error
**Cause:** OnInit() failed or virtual dispatch issue
**Solution:**
1. Verify OnInit() is called in Application::Run() (not in constructor)
2. Check for exceptions during scene creation
3. Enable trace logging to track scene lifetime

### Missing Scene Files
**Symptom:** "Failed to open file for reading"
**Cause:** Resources folder not copied to build directory
**Solution:**
1. Manually create `build/bin/Resources/Scenes/`
2. Copy scene JSON files to this directory
3. CMakeLists.txt should copy resources automatically

### Component Registration Errors
**Symptom:** Tuple construction error or type_index error
**Cause:** Incorrect macro usage or missing includes
**Solution:**
1. Use explicit lambda syntax in AddProperty
2. Include `<memory>`, `<typeindex>`, `<typeinfo>`
3. Avoid complex template deduction in macros

---

## Key Features

### Implemented ✅
- ECS architecture with EnTT
- Scene management with entity creation/destruction
- **Systems architecture with priority-based execution**
- **SystemRegistry for managing systems**
- Input system (keyboard, mouse, scroll) - **FIXED event timing**
- Logging system (console + file)
- Property-based reflection system
- JSON scene serialization/deserialization
- Component registration system
- Window management with GLFW
- OpenGL 4.3 rendering context
- **Fixed virtual function dispatch in Application**

### In Progress 🚧
- Rendering system (RenderSystem)
- Physics integration (PhysicsSystem with Jolt)
- Audio system (AudioSystem with FMOD)

### Planned 📋
- Editor with ImGui
- Event system (EventBus for system communication)
- Resource manager (Texture, Mesh, Shader, Audio)
- Prefab system
- Asset management
- Binary serialization
- Scene hierarchies
- Material system
- Advanced shader system

---

## Team Integration Guide

### For Physics Team
**Create:** `Engine/Physics/PhysicsSystem.h`
**Requirements:**
- `#include "ECS/System.h"`
- `#include "ECS/Components.h"`
- Inherit from `Engine::System`
- Priority: 10 (run early)
- Integrate Jolt Physics library
- Sync physics to TransformComponent
- Handle collision events

**Example:**
```cpp
#pragma once
#include "ECS/System.h"
#include "ECS/Components.h"

namespace Engine {

class PhysicsSystem : public System {
public:
    void OnUpdate(Scene* scene, Timestep ts) override {
        // Step Jolt physics world
        // Sync results to transforms
    }
    int GetPriority() const override { return 10; }
    const char* GetName() const override { return "PhysicsSystem"; }
};

} // namespace Engine
```

### For Rendering Team
**Create:** `Engine/Graphics/RenderSystem.h`
**Requirements:**
- `#include "ECS/System.h"`
- `#include "ECS/Components.h"`
- Inherit from `Engine::System`
- Priority: 100 (run late, after all transforms updated)
- Render entities with MeshRendererComponent
- Use CameraComponent for view/projection
- Manage shaders, textures, materials

**Example:**
```cpp
#pragma once
#include "ECS/System.h"
#include "ECS/Components.h"

namespace Engine {

class RenderSystem : public System {
public:
    RenderSystem(int width, int height);
    void OnUpdate(Scene* scene, Timestep ts) override {
        // Render all visible entities
    }
    int GetPriority() const override { return 100; }
    const char* GetName() const override { return "RenderSystem"; }
};

} // namespace Engine
```

### For Audio Team
**Create:** `Engine/Audio/AudioSystem.h`
**Requirements:**
- `#include "ECS/System.h"`
- `#include "ECS/Components.h"`
- Inherit from `Engine::System`
- Priority: 80 (after physics/movement, before rendering)
- Integrate FMOD library
- Play 2D and 3D audio
- Update listener from camera

**Example:**
```cpp
#pragma once
#include "ECS/System.h"
#include "ECS/Components.h"

namespace Engine {

class AudioSystem : public System {
public:
    void OnUpdate(Scene* scene, Timestep ts) override {
        // Update FMOD, play sounds
    }
    int GetPriority() const override { return 80; }
    const char* GetName() const override { return "AudioSystem"; }
};

} // namespace Engine
```

### For UI/Editor Team
**Create:** `Engine/Editor/EditorSystem.h`
**Requirements:**
- `#include "ECS/System.h"`
- Inherit from `Engine::System`
- Priority: 200 (run last, for UI overlay)
- Create ImGui panels (hierarchy, inspector, console)
- Edit entities and components
- Scene management UI

---

## Dependencies

### Core
- **GLFW 3.x** - Window and input
- **GLAD** - OpenGL loader
- **GLM** - Mathematics
- **EnTT** - ECS framework

### Serialization
- **RapidJSON** - JSON parsing (MIT License)

### Future
- **Jolt Physics** - Physics simulation
- **FMOD** - Audio
- **ImGui** - UI
- **Tracy** - Profiling

---

## File Naming Conventions
- Headers: `.h`
- Implementation: `.cpp`
- Scene files: `.json`
- Shaders: `.glsl`, `.vert`, `.frag`

## Namespace Structure
```cpp
namespace Engine {
    // All engine code (Core, ECS, Systems, Serialization, etc.)
}

// Game code uses global namespace or custom namespace
class Game : public Engine::Application { };
```

---

## Important Notes

### Forward Declarations
To avoid circular dependencies, use forward declarations in headers:
```cpp
// In System.h and SystemRegistry.h
namespace Engine {
    class Scene;  // Forward declaration
    
    class System {
        virtual void OnUpdate(Scene* scene, Timestep ts) = 0;
    };
}
```

### Include Order
1. System.h and SystemRegistry.h should NOT include Scene.h
2. Scene.h includes SystemRegistry.h
3. Only .cpp files should include Scene.h when needed
4. System headers in subsystem folders (Physics/, Graphics/, Audio/) should include `ECS/System.h`

### Virtual Function Dispatch
- `OnInit()` is called in `Application::Run()`, NOT in constructor
- This ensures derived class vtable is fully constructed
- Fixes virtual function dispatch issues

---

*Last Updated: 2025*
*Version: 2.0 with Systems Architecture*