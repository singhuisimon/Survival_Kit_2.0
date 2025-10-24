#pragma once
namespace ScriptAPI
{
    public ref class Script abstract
    {
    public:
        virtual void Update(float dt) {}
        void CreateNativeException();
        void CreateSEHException();

    internal:
        void SetEntityId(int id) { entityId = id; }
        property int EntityId{ int get() { return entityId; } }

    private:
        int entityId{ 0 };
    };
}
