using ScriptAPI;

public class TestScript : Script
{
    public override void Update(float dt)
    {
        var tf = new TransformComponent(EntityId);
        var rb = new RigidbodyComponent(EntityId);

        var p = tf.Position;
        p.X += 1.0f * dt;
        tf.Position = p;

        if (p.X < 0.05f)
            rb.AddImpulse(new float3(0, 2.0f, 0));
    }
}
