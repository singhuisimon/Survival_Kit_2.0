using Engine;
using System;
using System.Collections.Generic;
using static Engine.Physics;
using static Engine.Tag;
using static Engine.Logger;

namespace Game
{
    /// <summary>
    /// Advanced Collision Manager with category-based collision detection.
    /// Handles multiple collision types: bullets, entities, environment, etc.
    /// 
    /// Collision Categories:
    /// - PLAYER_PROJECTILE: Player bullets, missiles, etc.
    /// - ENEMY_PROJECTILE: Enemy bullets, missiles, etc.
    /// - ALLY_PROJECTILE: Ally/sentry bullets, missiles, etc.
    /// - PLAYER: Player ship
    /// - ALLY: Allied ships, sentries
    /// - ENEMY: Enemy ships (botnets, etc.)
    /// - CORE: Player base/core
    /// - CORE_BARRIER: Protective barrier around core
    /// - ENVIRONMENT: Boundaries, obstacles, asteroids
    /// </summary>
    public class CollisionManager : ScriptBehaviour
    {
        // ============================================================
        // COLLISION CATEGORIES - Define what each entity type is
        // ============================================================
        
        public enum CollisionCategory
        {
            PLAYER_PROJECTILE,
            ENEMY_PROJECTILE,
            ALLY_PROJECTILE,
            PLAYER,
            ALLY,
            ENEMY,
            CORE,
            CORE_BARRIER,
            ENVIRONMENT,
            UNKNOWN
        }

        // ============================================================
        // TAG TO CATEGORY MAPPING
        // ============================================================
        
        private static readonly Dictionary<string, CollisionCategory> TagToCategory = 
            new Dictionary<string, CollisionCategory>()
        {
            // Player projectiles (all lowercase keys)
            { "primarybullet", CollisionCategory.PLAYER_PROJECTILE },
            { "primaryultbullet", CollisionCategory.PLAYER_PROJECTILE },
            { "primaryultaoe", CollisionCategory.PLAYER_PROJECTILE },
            { "secondarybullet", CollisionCategory.PLAYER_PROJECTILE },
            { "missile", CollisionCategory.PLAYER_PROJECTILE },
            
            // Enemy projectiles
            { "enemybullet", CollisionCategory.ENEMY_PROJECTILE },
            { "enemymissile", CollisionCategory.ENEMY_PROJECTILE },
            
            // Ally projectiles
            { "sentrybullet", CollisionCategory.ALLY_PROJECTILE },
            { "allybullet", CollisionCategory.ALLY_PROJECTILE },
            
            // Entities
            { "player", CollisionCategory.PLAYER },
            { "ally", CollisionCategory.ALLY },
            { "sentry", CollisionCategory.ALLY },
            
            // Enemies
            { "botnet", CollisionCategory.ENEMY },
            { "loveletter", CollisionCategory.ENEMY },
            { "adware", CollisionCategory.ENEMY },
            { "wormhost", CollisionCategory.ENEMY },
            { "wormchild", CollisionCategory.ENEMY },
            
            // Objectives
            { "semiconductor", CollisionCategory.CORE },
            { "corebarrier", CollisionCategory.CORE_BARRIER },
            
            // Environment
            { "oob", CollisionCategory.ENVIRONMENT }
        };

        // ============================================================
        // COLLISION STORAGE - Organized by collision type
        // ============================================================
        
        // Projectile hits entity
        private Dictionary<uint, List<uint>> playerProjectileHits = new Dictionary<uint, List<uint>>();
        private Dictionary<uint, List<uint>> enemyProjectileHits = new Dictionary<uint, List<uint>>();
        private Dictionary<uint, List<uint>> allyProjectileHits = new Dictionary<uint, List<uint>>();
        
        // Entity collisions
        private Dictionary<uint, List<uint>> playerCollisions = new Dictionary<uint, List<uint>>();
        private Dictionary<uint, List<uint>> allyCollisions = new Dictionary<uint, List<uint>>();
        private Dictionary<uint, List<uint>> enemyCollisions = new Dictionary<uint, List<uint>>();
        private Dictionary<uint, List<uint>> coreCollisions = new Dictionary<uint, List<uint>>();
        private Dictionary<uint, List<uint>> barrierCollisions = new Dictionary<uint, List<uint>>();
        
        // Environment collisions (everything that hit environment)
        private Dictionary<uint, List<uint>> environmentCollisions = new Dictionary<uint, List<uint>>();
        
        // ============================================================
        // COLLISION RULES - Define what can collide with what
        // ============================================================
        
        private struct CollisionRule
        {
            public CollisionCategory CategoryA;
            public CollisionCategory CategoryB;
            public bool ShouldRecord;

            public CollisionRule(CollisionCategory a, CollisionCategory b, bool record = true)
            {
                CategoryA = a;
                CategoryB = b;
                ShouldRecord = record;
            }
        }

        private static readonly List<CollisionRule> CollisionRules = new List<CollisionRule>
        {
            // Player projectiles can hit:
            new CollisionRule(CollisionCategory.PLAYER_PROJECTILE, CollisionCategory.ENEMY),
            
            // Enemy projectiles can hit:
            new CollisionRule(CollisionCategory.ENEMY_PROJECTILE, CollisionCategory.PLAYER),
            new CollisionRule(CollisionCategory.ENEMY_PROJECTILE, CollisionCategory.ALLY),
            new CollisionRule(CollisionCategory.ENEMY_PROJECTILE, CollisionCategory.CORE),
            new CollisionRule(CollisionCategory.ENEMY_PROJECTILE, CollisionCategory.CORE_BARRIER),
            
            // Ally projectiles can hit:
            new CollisionRule(CollisionCategory.ALLY_PROJECTILE, CollisionCategory.ENEMY),
            
            // Player can hit:
            new CollisionRule(CollisionCategory.PLAYER, CollisionCategory.ENVIRONMENT),
            new CollisionRule(CollisionCategory.PLAYER, CollisionCategory.ENEMY),
            
            // Enemies can hit:
            new CollisionRule(CollisionCategory.ENEMY, CollisionCategory.PLAYER),
            new CollisionRule(CollisionCategory.ENEMY, CollisionCategory.CORE),
            new CollisionRule(CollisionCategory.ENEMY, CollisionCategory.CORE_BARRIER),
            
            // Allies can hit:
            new CollisionRule(CollisionCategory.ALLY, CollisionCategory.ENEMY),
            
            // Core barrier can be hit by anything (it's defensive)
            new CollisionRule(CollisionCategory.CORE_BARRIER, CollisionCategory.ENEMY_PROJECTILE),
            new CollisionRule(CollisionCategory.CORE_BARRIER, CollisionCategory.ENEMY),
            
            new CollisionRule(CollisionCategory.CORE, CollisionCategory.ENEMY_PROJECTILE),
            new CollisionRule(CollisionCategory.CORE, CollisionCategory.ENEMY),
        };

        // Singleton
        private static CollisionManager instance;

        public override void OnStart()
        {
            instance = this;
            LogMessage("[CollisionManager] Initialized with category-based collision system");
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            // Clear all collision storage
            ClearAllCollisions();
            
            // Process all physics collisions
            int collisionCount = PhysicsGetCollisionCount();
            
            for (int i = 0; i < collisionCount; i++)
            {
                PhysicsGetCollisionPair(i, out uint entityA, out uint entityB);
                
                // Get tags and categories
                string tagA = TagGetTag(entityA);
                string tagB = TagGetTag(entityB);
                
                CollisionCategory categoryA = GetCategory(tagA);
                CollisionCategory categoryB = GetCategory(tagB);
                
                // Check if this collision should be recorded
                if (ShouldRecordCollision(categoryA, categoryB))
                {
                    RecordCollision(entityA, categoryA, entityB, categoryB);
                }
            }
        }

        // ============================================================
        // COLLISION PROCESSING
        // ============================================================

        private void ClearAllCollisions()
        {
            playerProjectileHits.Clear();
            enemyProjectileHits.Clear();
            allyProjectileHits.Clear();
            playerCollisions.Clear();
            allyCollisions.Clear();
            enemyCollisions.Clear();
            coreCollisions.Clear();
            barrierCollisions.Clear();
            environmentCollisions.Clear();
        }

        private CollisionCategory GetCategory(string tag)
        {
            if (string.IsNullOrEmpty(tag))
                return CollisionCategory.UNKNOWN;
                
            // Convert to lowercase for case-insensitive lookup
            string tagLower = tag.ToLower();
            if (TagToCategory.TryGetValue(tagLower, out CollisionCategory category))
                return category;
                
            return CollisionCategory.UNKNOWN;
        }

        private bool ShouldRecordCollision(CollisionCategory catA, CollisionCategory catB)
        {
            // Check both directions (A->B and B->A)
            foreach (var rule in CollisionRules)
            {
                if ((rule.CategoryA == catA && rule.CategoryB == catB) ||
                    (rule.CategoryA == catB && rule.CategoryB == catA))
                {
                    return rule.ShouldRecord;
                }
            }
            return false;
        }

        private void RecordCollision(uint entityA, CollisionCategory catA, uint entityB, CollisionCategory catB)
        {
            // Record based on category combinations
            
            // PROJECTILE HITS
            if (catA == CollisionCategory.PLAYER_PROJECTILE)
            {
                RecordHit(playerProjectileHits, entityA, entityB);
            }
            else if (catB == CollisionCategory.PLAYER_PROJECTILE)
            {
                RecordHit(playerProjectileHits, entityB, entityA);
            }
            
            if (catA == CollisionCategory.ENEMY_PROJECTILE)
            {
                RecordHit(enemyProjectileHits, entityA, entityB);
            }
            else if (catB == CollisionCategory.ENEMY_PROJECTILE)
            {
                RecordHit(enemyProjectileHits, entityB, entityA);
            }
            
            if (catA == CollisionCategory.ALLY_PROJECTILE)
            {
                RecordHit(allyProjectileHits, entityA, entityB);
            }
            else if (catB == CollisionCategory.ALLY_PROJECTILE)
            {
                RecordHit(allyProjectileHits, entityB, entityA);
            }
            
            // ENTITY COLLISIONS
            if (catA == CollisionCategory.PLAYER)
            {
                RecordHit(playerCollisions, entityA, entityB);
            }
            else if (catB == CollisionCategory.PLAYER)
            {
                RecordHit(playerCollisions, entityB, entityA);
            }
            
            if (catA == CollisionCategory.ALLY)
            {
                RecordHit(allyCollisions, entityA, entityB);
            }
            else if (catB == CollisionCategory.ALLY)
            {
                RecordHit(allyCollisions, entityB, entityA);
            }
            
            if (catA == CollisionCategory.ENEMY)
            {
                RecordHit(enemyCollisions, entityA, entityB);
            }
            else if (catB == CollisionCategory.ENEMY)
            {
                RecordHit(enemyCollisions, entityB, entityA);
            }
            
            if (catA == CollisionCategory.CORE)
            {
                RecordHit(coreCollisions, entityA, entityB);
            }
            else if (catB == CollisionCategory.CORE)
            {
                RecordHit(coreCollisions, entityB, entityA);
            }
            
            if (catA == CollisionCategory.CORE_BARRIER)
            {
                RecordHit(barrierCollisions, entityA, entityB);
            }
            else if (catB == CollisionCategory.CORE_BARRIER)
            {
                RecordHit(barrierCollisions, entityB, entityA);
            }
            
            // ENVIRONMENT COLLISIONS
            if (catA == CollisionCategory.ENVIRONMENT)
            {
                RecordHit(environmentCollisions, entityB, entityA);
            }
            else if (catB == CollisionCategory.ENVIRONMENT)
            {
                RecordHit(environmentCollisions, entityA, entityB);
            }
        }

        private void RecordHit(Dictionary<uint, List<uint>> storage, uint source, uint target)
        {
            if (!storage.ContainsKey(source))
            {
                storage[source] = new List<uint>();
            }
            storage[source].Add(target);
        }

        // ============================================================
        // PUBLIC QUERY API
        // ============================================================

        /// <summary>
        /// Get all entities that a player projectile hit
        /// </summary>
        public static List<uint> GetPlayerProjectileHits(uint projectileEntityId)
        {
            return GetFromDictionary(instance?.playerProjectileHits, projectileEntityId);
        }

        /// <summary>
        /// Get all entities that an enemy projectile hit
        /// </summary>
        public static List<uint> GetEnemyProjectileHits(uint projectileEntityId)
        {
            return GetFromDictionary(instance?.enemyProjectileHits, projectileEntityId);
        }

        /// <summary>
        /// Get all entities that an ally projectile hit
        /// </summary>
        public static List<uint> GetAllyProjectileHits(uint projectileEntityId)
        {
            return GetFromDictionary(instance?.allyProjectileHits, projectileEntityId);
        }

        /// <summary>
        /// Get all entities that the player collided with (boundaries, enemies, etc.)
        /// </summary>
        public static List<uint> GetPlayerCollisions(uint playerEntityId)
        {
            return GetFromDictionary(instance?.playerCollisions, playerEntityId);
        }

        /// <summary>
        /// Get all entities that an ally collided with
        /// </summary>
        public static List<uint> GetAllyCollisions(uint allyEntityId)
        {
            return GetFromDictionary(instance?.allyCollisions, allyEntityId);
        }

        /// <summary>
        /// Get all entities that an enemy collided with (player, core, etc.)
        /// </summary>
        public static List<uint> GetEnemyCollisions(uint enemyEntityId)
        {
            return GetFromDictionary(instance?.enemyCollisions, enemyEntityId);
        }

        /// <summary>
        /// Get all entities that hit the core
        /// </summary>
        public static List<uint> GetCoreCollisions(uint coreEntityId)
        {
            return GetFromDictionary(instance?.coreCollisions, coreEntityId);
        }

        /// <summary>
        /// Get all entities that hit the core barrier
        /// </summary>
        public static List<uint> GetBarrierCollisions(uint barrierEntityId)
        {
            return GetFromDictionary(instance?.barrierCollisions, barrierEntityId);
        }

        /// <summary>
        /// Get all environment objects (boundaries, walls) that an entity hit
        /// </summary>
        public static List<uint> GetEnvironmentCollisions(uint entityId)
        {
            return GetFromDictionary(instance?.environmentCollisions, entityId);
        }

        // ============================================================
        // HELPER METHODS
        // ============================================================

        private static List<uint> GetFromDictionary(Dictionary<uint, List<uint>> dict, uint key)
        {
            if (dict == null || instance == null)
            {
                LogMessage("[CollisionManager] ERROR: Instance is null!");
                return null;
            }
            
            if (dict.TryGetValue(key, out var targets))
            {
                return targets;
            }
            
            return null;
        }

        public override void OnDestroy()
        {
            if (instance == this)
            {
                instance = null;
            }
        }
    }
}