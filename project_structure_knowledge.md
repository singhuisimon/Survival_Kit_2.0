# Game Engine Project Structure & Architecture

## Project Overview
**Name:** Property-Based ECS Game Engine  
**Architecture:** Entity Component System (ECS) using EnTT  
**Build System:** CMake  
**C++ Standard:** C++17  
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
│   │   ├── Input.h              # Input system (keyboard, mouse)
│   │   └── Input.cpp            # Non-singleton input handling
│   │
│   ├── ECS/                      # Entity Component System
│   │   ├── Entity.h             # Entity wrapper around EnTT
│   │   ├── Scene.h              # Scene management (entity collections)
│   │   ├── Scene.cpp            # Scene implementation with serialization
│   │   ├── Components.h         # All component definitions (POD structs)
│   │   └── Systems.h            # System definitions (future)
│   │
│   ├── Serialization/            # Property-Based Serialization System
│   │   ├── Property.h           # Property reflection base classes
│   │   ├── ReflectionRegistry.h # Component metadata registry
│   │   ├── ComponentRegistry.h  # Component registration header
│   │   ├── ComponentRegistry.cpp # Register all components
│   │   ├── SceneSerializer.h    # Scene serialization interface
│   │   └── SceneSerializer.cpp  # JSON-based scene serialization
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
│       ├── EditorCamera.h       # Editor-specific camera
│       ├── EditorCamera.cpp
│       ├── Gizmos.h             # Transform gizmos
│       └── Gizmos.cpp
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
│
└── Resources/                     # GAME ASSETS
    ├── Shaders/                  # GLSL shaders
    ├── Models/                   # 3D models
    ├── Textures/                 # Image files
    ├── Audio/                    # Sound files
    └── Scenes/                   # Scene JSON files (NEW)
        ├── ExampleScene.json     # Example game scene
        ├── TestScene.json        # Arena test scene
        └── SavedScene.json       # Runtime save location

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

#### Input System  
- **Purpose:** Handle keyboard and mouse input
- **Design:** Direct class (not singleton/manager pattern)
- **Features:**
  - Key/button states (pressed, just pressed, just released)
  - Mouse position and delta tracking
  - Scroll wheel support
  - Cursor visibility control

### **ECS (Entity Component System)**

#### Design Philosophy
- **Entities:** Simple IDs (managed by EnTT)
- **Components:** Pure data structs (no logic)
- **Systems:** Functions that operate on components (future)

#### Current Components
- `TagComponent` - Human-readable names
- `TransformComponent` - Position, rotation, scale
- `CameraComponent` - Camera settings
- `MeshRendererComponent` - Rendering data (future)
- `RigidbodyComponent` - Physics data (future)

#### Entity ID Uniqueness
- EnTT guarantees unique entity IDs within a registry
- IDs are versioned internally to prevent reuse conflicts
- Each `Scene` has its own registry (isolated ID space)

### **Serialization System** (NEW)

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

### Scene Save/Load
```cpp
// Save scene
m_Scene->SaveToFile("Resources/Scenes/MyScene.json");

// Load scene
m_Scene->LoadFromFile("Resources/Scenes/MyScene.json");
```

---

## Common Issues & Solutions

### Serialization Crashes
**Symptom:** 0xc0000005 access violation when saving/loading
**Cause:** Components not properly initialized or registered
**Solution:**
1. Ensure all components have explicit default constructors
2. Call `ComponentRegistry::RegisterAllComponents()` at startup
3. Check that Resources/Scenes/ folder exists in build directory

### Scene is Null
**Symptom:** "Scene is null in OnUpdate!" error
**Cause:** Scene destruction or failed creation
**Solution:**
1. Verify scene is created in OnInit()
2. Check for exceptions during scene creation
3. Don't use std::move on scene pointer
4. Enable trace logging to track scene lifetime

### Missing Scene Files
**Symptom:** "Failed to open file for reading"
**Cause:** Resources folder not copied to build directory
**Solution:**
1. Manually create `build/bin/Resources/Scenes/`
2. Copy scene JSON files to this directory
3. Update CMakeLists.txt to copy resources automatically

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
- Input system (keyboard, mouse, scroll)
- Logging system (console + file)
- Property-based reflection system
- JSON scene serialization/deserialization
- Component registration system
- Window management with GLFW
- OpenGL 4.3 rendering context

### In Progress 🚧
- Rendering system
- Physics integration (Jolt)
- Audio system (FMOD)

### Planned 📋
- Editor with ImGui
- Prefab system
- Asset management
- Binary serialization
- Scene hierarchies
- Material system
- Shader system

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
    // All engine code
}

// Game code uses global namespace or custom namespace
class Game : public Engine::Application { };
```

---

*Last Updated: 2025*
*Version: 1.0 with Serialization System*