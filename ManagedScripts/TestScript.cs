using ScriptAPI;


public class TestScript : Script
{
    // ========== PUBLIC FIELDS (Always serialized) ==========
    public float moveSpeed = 5.0f;
    public int health = 100;
    public string playerName = "spaceship";
    public bool isActive = true;

    // ========== PRIVATE FIELDS (NOT serialized by default) ==========
    private float internalCooldown = 1.0f;  // This will NOT show in editor

    // ========== PRIVATE WITH [SerializeField] (WILL be serialized) ==========
    [SerializeField]
    private float jumpForce = 10.0f;

    [SerializeField]
    private int lives = 3;

    [SerializeField]
    private string secretCode = "ABC123";

    [SerializeField]
    private bool canDoubleJump = false;

    // ========== WITH RANGE ATTRIBUTE ==========
    [SerializeField]
    [Range(0f, 100f)]
    private float attackPower = 50f;

    // ========== WITH TOOLTIP ==========
    [SerializeField("The speed at which the player dashes")]
    private float dashSpeed = 15.0f;

    // ========== HIDDEN PUBLIC FIELD ==========
    [HideInInspector]
    public float debugValue = 99.9f;  // Public but hidden in editor

    private int frameCount = 0;

    public override void Update()
    {
        frameCount++;

        // Print values every 120 frames (~2 seconds)
        if (frameCount % 120 == 0)
        {
            Console.WriteLine("=== TestSerializeScript Values ===");
            Console.WriteLine($"Public - moveSpeed: {moveSpeed}");
            Console.WriteLine($"Public - health: {health}");
            Console.WriteLine($"Public - playerName: {playerName}");
            Console.WriteLine($"Public - isActive: {isActive}");
            Console.WriteLine($"[SerializeField] - jumpForce: {jumpForce}");
            Console.WriteLine($"[SerializeField] - lives: {lives}");
            Console.WriteLine($"[SerializeField] - secretCode: {secretCode}");
            Console.WriteLine($"[SerializeField] - canDoubleJump: {canDoubleJump}");
            Console.WriteLine($"[SerializeField][Range] - attackPower: {attackPower}");
            Console.WriteLine($"[SerializeField(tooltip)] - dashSpeed: {dashSpeed}");
            Console.WriteLine($"Private - internalCooldown: {internalCooldown} (not serialized)");
            Console.WriteLine($"[HideInInspector] - debugValue: {debugValue} (hidden)");
            Console.WriteLine("==================================\n");
        }
    }
}